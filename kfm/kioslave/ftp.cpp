#include "ftp.h"
#include <errno.h>

#define SUCCESS 0
#define FAIL -1

#define FTP_LOGIN "anonymous"
#define FTP_PASSWD "kfm-user@nowhere.org"

/*
 * read a line of text
 *
 * return -1 on error or bytecount
 */
int KProtocolFTP::readline(char *buf,int max,netbuf *ctl)
{
    int x,retval = 0;
    char *end;
    int eof = 0;

    if (max == 0)
	return 0;
    do
    {
    	if (ctl->cavail > 0)
    	{
	    	x = (max >= ctl->cavail) ? ctl->cavail : max-1;
	    	end = (char*)memccpy(buf,ctl->cget,'\n',x);
	    	if (end != NULL)
				x = end - buf;
	    	retval += x;
	    	buf += x;
	    	*buf = '\0';
	    	max -= x;
	    	ctl->cget += x;
	    	ctl->cavail -= x;
	    	if (end != NULL)
	    		break;
    	}
    	if (max == 1)
    	{
	    	*buf = '\0';
	    	break;
    	}
    	if (ctl->cput == ctl->cget)
    	{
	    	ctl->cput = ctl->cget = ctl->buf;
	    	ctl->cavail = 0;
	    	ctl->cleft = FTP_BUFSIZ;
    	}
		if (eof)
		{
	    	if (retval == 0)
			retval = -1;
	    	break;
		}
    	if ((x = read(ctl->handle,ctl->cput,ctl->cleft)) == -1)
    	{
	    	perror("read");
	    	retval = -1;
	    	break;
    	}
		if (x == 0)
	    	eof = 1;
    	ctl->cleft -= x;
    	ctl->cavail += x;
    	ctl->cput += x;
    }
    while (1);
    return retval;
}

/*
 * read a response from the server
 *
 * return 0 if first char doesn't match
 * return 1 if first char matches
 */
int KProtocolFTP::readresp(char c)
{
    char match[5];
    if (readline(rspbuf,256,nControl) == -1)
    {
		perror("Control socket read failed");
		return 0;
    }
    if (ftplib_debug > 1)
	fprintf(stderr,"resp> %s",rspbuf);
    if (rspbuf[3] == '-')
    {
		strncpy(match,rspbuf,3);
		match[3] = ' ';
		match[4] = '\0';
		do
		{
	    	if (readline(rspbuf,256,nControl) == -1)
	    	{
				perror("Control socket read failed");
				return 0;
	    	}
	    	if (ftplib_debug > 1)
				fprintf(stderr,"%s",rspbuf);
		}
		while (strncmp(rspbuf,match,4));
    }
    	
    if (rspbuf[0] == c)
		return 1;

    return 0;
}

/*
 * ftpOpen - connect to remote server
 *
 * return 1 if connected, 0 if not
 */
int KProtocolFTP::ftpOpen(const char *host)
{
    struct sockaddr_in sin;
    struct hostent *phe;
    struct servent *pse;
    int on=1;

	printf("A\n");
    ftplib_lastresp[0] = '\0';
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;

    if ((pse = getservbyname("ftp","tcp")) == NULL)
    {
		perror("getservbyname");
		return 0;
    }
    sin.sin_port = pse->s_port;

    if ((phe = gethostbyname(host)) == NULL)
    {
		perror("gethostbyname");
		return 0;
    }
	printf("B\n");
    memcpy((char *)&sin.sin_addr, phe->h_addr, phe->h_length);
    sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sControl == -1)
    {
		perror("socket");
		return 0;
    }
    if (setsockopt(sControl,SOL_SOCKET,SO_REUSEADDR,
		   SETSOCKOPT_OPTVAL_TYPE &on, sizeof(on)) == -1)
    {
		perror("setsockopt");
		close(sControl);
		return 0;
    }
	printf("C\n");
    if ( ::connect(sControl, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
		perror("connect");
		close(sControl);
		return 0;
    }
    nControl = (netbuf*)calloc(1,sizeof(netbuf));
    if (nControl == NULL)
    {
		perror("calloc");
		close(sControl);
		return 0;
    }
    nControl->handle = sControl;
	printf("D\n");
    if (readresp('2') == 0)
    {
		close(sControl);
		free(nControl);
		return 0;
    }
	printf("E\n");
    return 1;
}

/*
 * ftpSendCmd - send a command and wait for expected response
 *
 * return 1 if proper response received, 0 otherwise
 */
int KProtocolFTP::ftpSendCmd(char *cmd, char expresp)
{
    char buf[256];
    if (ftplib_debug > 2)
		fprintf(stderr,"%s\n",cmd);
    sprintf(buf,"%s\r\n",cmd);
    if (write(sControl,buf,strlen(buf)) <= 0)
    {
		perror("write");
		return 0;
    }    
    return readresp(expresp);
}

/*
 * ftpLogin - log in to remote server
 *
 * return 1 if logged in, 0 otherwise
 */
int KProtocolFTP::ftpLogin( const char *user, const char *pass)
{
    char tempbuf[64];

	printf("LOGIN!\n");
    printf("ftplib_lastresp[0] = '\\0';\n");
    ftplib_lastresp[0] = '\0';
	printf("sprintf(tempbuf,...)\n");
    sprintf(tempbuf,"user %s",user);
	printf("init rspbuf\n");
    rspbuf[0]='\0';
	printf("Sending login %s\n",user);
    if (!ftpSendCmd(tempbuf,'3'))
    {
		printf("accepted w/o pass\n");
		if (rspbuf[0]=='2') return 1; /* no password required */
		return 0;
    }      
    sprintf(tempbuf,"pass %s",pass);
	printf("Sending pass\n");
    return ftpSendCmd(tempbuf,'2');
}

/*
 * ftpPort - set up date connection
 *
 * return 1 if successful, 0 otherwise
 */
int KProtocolFTP::ftpPort(void)
{
    union {
		struct sockaddr sa;
		struct sockaddr_in in;
    } sin;
    struct linger lng = { 0, 0 };
    ksize_t l;
    char buf[64];
    int on=1;

    l = sizeof(sin);
    if (getsockname(sControl,&sin.sa,&l) < 0)
	return 0;
    sDatal = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sDatal == -1)
    {
		perror("socket");
		return 0;
    }
    if (setsockopt(sDatal,SOL_SOCKET,SO_REUSEADDR,
		   SETSOCKOPT_OPTVAL_TYPE &on,sizeof(on)) == -1)
    {
		perror("setsockopt");
		close(sDatal);
		return 0;
    }
    if (setsockopt(sDatal,SOL_SOCKET,SO_LINGER,
		   SETSOCKOPT_OPTVAL_TYPE &lng,sizeof(lng)) == -1)
    {
		perror("setsockopt");
		close(sDatal);
		return 0;
    }
    sin.in.sin_port = 0;
    if (bind(sDatal,&sin.sa,sizeof(sin)) == -1)
    {
		perror("bind");
		close(sDatal);
		sDatal = -1;
		return 0;
    }
    if (listen(sDatal,1) < 0)
    {
		perror("listen");
		close(sDatal);
		sDatal = -1;
		return 0;
    }
    if (getsockname(sDatal,&sin.sa,&l) < 0)
	return 0;
    sprintf(buf,"port %d,%d,%d,%d,%d,%d",
	    (unsigned char)sin.sa.sa_data[2],(unsigned char)sin.sa.sa_data[3],
	    (unsigned char)sin.sa.sa_data[4],(unsigned char)sin.sa.sa_data[5],
	    (unsigned char)sin.sa.sa_data[0],(unsigned char)sin.sa.sa_data[1]);
    if (!ftpSendCmd(buf,'2'))
	return 0;
    return 1;
}

/*
 * ftpMkdir - create a directory at server
 *
 * return 1 if successful, 0 otherwise
 */
int KProtocolFTP::ftpMkdir( const char *path )
{
    char buf[256];
    ftplib_lastresp[0] = '\0';
    sprintf(buf,"mkd %s",path);
    if (!ftpSendCmd(buf,'2')) return 0;
    return 1;
}

/*
 * ftpChdir - change path at remote
 *
 * return 1 if successful, 0 otherwise
 */
 int KProtocolFTP::ftpChdir( const char *path)
{
    char buf[256];
    ftplib_lastresp[0] = '\0';
    sprintf(buf,"cwd %s",path);
    if (!ftpSendCmd(buf,'2'))
	return 0;
    return 1;
}

 int KProtocolFTP::ftpRmdir( const char *path)
{
    char buf[256];
    ftplib_lastresp[0] = '\0';
    sprintf(buf,"RMD %s",path);
    if (!ftpSendCmd(buf,'2'))
	return 0;
    return 1;
}

/*
 * accept_connect - wait for incoming connection
 *
 * return -2 on error or timeout
 * otherwise returns socket descriptor
 */
 int KProtocolFTP::accept_connect(void)
{
    int sData;
    struct sockaddr addr;
    ksize_t l;
    fd_set mask;

    FD_ZERO(&mask);
    FD_SET(sDatal,&mask);

    if (select( sDatal + 1, &mask,NULL,NULL, 0L) == 0)
    {
		close(sDatal);
		return -2;
    }

    l = sizeof(addr);
    if ((sData = accept(sDatal,&addr,&l)) > 0)
	return sData;

    close(sDatal);
    return -2;
}

/*
 * ftpRename - rename a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
int KProtocolFTP::ftpRename( const char *src, const char *dst)
{
    char cmd[256];
    ftplib_lastresp[0] = '\0';
    sprintf(cmd,"RNFR %s",src);
    if (!ftpSendCmd(cmd,'3'))
	return 0;
    sprintf(cmd,"RNTO %s",dst);
    if (!ftpSendCmd(cmd,'2'))
	return 0;
    return 1;
}

/*
 * ftpDelete - delete a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
int KProtocolFTP::ftpDelete( const char *fnm )
{
    char cmd[256];
    ftplib_lastresp[0] = '\0';
    sprintf(cmd,"DELE %s",fnm);
    if (!ftpSendCmd(cmd,'2'))
	return 0;
    return 1;
}

/*
 * ftpQuit - disconnect from remote
 */
void KProtocolFTP::ftpQuit(void)
{
	if(sControl >= 0)
	{
    	ftplib_lastresp[0] = '\0';
    	ftpSendCmd("quit",'2');
    	free(nControl);
    	close(sControl);
		sControl = -1;
	}
}

KProtocolFTP::KProtocolFTP()
{
	dirfile = NULL;
	sControl = sData = sDatal = -1;
	ftplib_lastresp = rspbuf;
	ftplib_debug = 9;
}

KProtocolFTP::~KProtocolFTP()
{
	printf("entering destructor...\n");
	if(dirfile) CloseDir();
	printf("dirfile done.\n");
	Close();
	printf("leaving destructor\n");
}

int KProtocolFTP::OpenConnection(const char *command, const char *path, char mode)
{
    char buf[256];

    ftplib_lastresp[0] = '\0';
	sprintf(buf,"type %c",mode);
    if (!ftpSendCmd(buf,'2')) return Error(KIO_ERROR_CouldNotConnect,
				"Could not set ftp to correct mode for transmission");
    if (!ftpPort()) return Error(KIO_ERROR_CouldNotConnect,
				"Could not setup ftp data port", errno);

    if (path != NULL)
		sprintf(buf,"%s %s",command,path);
	else
    	strcpy(buf,command);

	printf("sending command '%s'\n",buf);
    if (!ftpSendCmd(buf,'1')) return Error(KIO_ERROR_CouldNotConnect,
				"Error requesting file/dir from server");
    if ((sData = accept_connect()) < 0)
    {
		if (sData == -2) perror("accept");
		else printf("ERROR!: %s",rspbuf);
		return Error(KIO_ERROR_CouldNotConnect,
				"Could not establish data connection",errno);
    }
	printf("very good, socket is %d\n",sData);
	return SUCCESS;
}

int KProtocolFTP::CloseConnection()
{
	/** readresp('2') ?? gibt an ob Transmission erfolgreich war! **/
	if(sData != -1)
	{
		printf("sData:A\n");
    	shutdown(sData,2);
		printf("sData:B\n");
    	close(sData);
		sData = -1;
	}
    if(sDatal != -1)
	{
		printf("sDatal:A\n");
		close(sDatal);
		sDatal = -1;
	}
	return 1;
}

int KProtocolFTP::OpenDir(KURL *url)
{
	if(Connect(url) == FAIL) return(FAIL);
	if(OpenConnection("list",url->path(),'A') == FAIL) return(FAIL);
	dirfile = fdopen(sData,"r");
	if(!dirfile) return(FAIL);
	return(SUCCESS);
}

KProtocolDirEntry *KProtocolFTP::ReadDir()
{
	char buffer[1024];
	static KProtocolDirEntry de;

	while(fgets(buffer,1024,dirfile) != NULL)
	{
		if(char *p_access = strtok(buffer," "))
		if(char *p_junk = strtok(NULL," "))
        if(char *p_owner = strtok(NULL," "))
        if(char *p_group = strtok(NULL," "))
        if(char *p_size = strtok(NULL," "))
        if(char *p_date_1 = strtok(NULL," "))
        if(char *p_date_2 = strtok(NULL," "))
        if(char *p_date_3 = strtok(NULL," "))
        if(char *p_name = strtok(NULL," \r\n"))
		{
			de.access	= p_access;
			de.owner	= p_owner;
			de.group	= p_group;
			de.size		= atoi(p_size);
			de.isdir	= p_access[0]=='d';
			de.name		= p_name;
			if(de.isdir) de.name += "/";
			de.date.sprintf("%s %s %s",p_date_1, p_date_2, p_date_3);
			return(&de);
		}
	}
	return(NULL);
}

int KProtocolFTP::CloseDir()
{
	if(dirfile)
	{
		fclose(dirfile);
		dirfile = NULL;
		CloseConnection();
		ftpQuit();
	}
	return(SUCCESS);
}

int KProtocolFTP::Connect(KURL *url)
{
	QString user, passwd;
	printf("connecting %s\n",url->host());
    if (!ftpOpen(url->host()))
		return(Error(KIO_ERROR_CouldNotConnect, "Could not connect", 0));

	if(strlen(url->user()))
	{
		user = url->user();
		passwd = url->passwd();
	}
	else
	{
		user = FTP_LOGIN;
		passwd = FTP_PASSWD;
	}
	printf("Login: \n");
	if(!ftpLogin(user,passwd))
		return(Error(KIO_ERROR_CouldNotLogin, "invalid passwd or username"));

	printf("... connection established!\n");
	return(SUCCESS);
}

int KProtocolFTP::Open(KURL *url, int mode)
{
	if(Connect(url) == FAIL) return(FAIL);
	if(mode & READ)
	{
		int rc = OpenConnection("retr",url->path(),'I');
		if(rc == FAIL)
			return Error(KIO_ERROR_CouldNotConnect,"Error building connection");
    	printf("Calculating size\n");
    
    	// Read the size from the response string
    	if ( strlen( rspbuf ) > 4 )
    	{
			char *p = strchr( rspbuf, '(' );
			if ( p != 0L ) size = atol( p + 1 );
    	}

    	printf("Size is %ld (rc is %d)\n",size ,rc);
		bytesleft = size;
		return(SUCCESS);
	}
	if(mode & WRITE)
	{
		if(OpenConnection("stor",url->path(),'I') == SUCCESS)
			return(SUCCESS);
	}
	return(FAIL);
}

int KProtocolFTP::Close()
{
	if(CloseConnection()) return(SUCCESS);
	return(FAIL);
}

long KProtocolFTP::Size()
{
	return(size);
}

int KProtocolFTP::atEOF()
{
	printf("atEOF? ... %s! bytesleft =  %ld\n",(bytesleft<=0)?"yes":"no",bytesleft);
	return(bytesleft <= 0);
}

long KProtocolFTP::Read(void *buffer, long len)
{
	printf("Trying to read...\n");
	int n = read(sData,buffer,len);
	bytesleft -= n;
	return(n);
}

long KProtocolFTP::Write(void *buffer, long len)
{
	printf("Trying to write...\n");
	return(write(sData,buffer,len));
}

int KProtocolFTP::MkDir(KURL *url)
{
	if(Connect(url) == FAIL) return(FAIL);
	if(!ftpMkdir(url->path()))
		return(Error(KIO_ERROR_CouldNotMkdir,"Can't create dir on ftp server"));
	ftpQuit();
	return(SUCCESS);
}

#include "ftp.moc"
