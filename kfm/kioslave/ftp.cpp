#include "ftp.h"
#include <errno.h>
#include <kstring.h>

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
int KProtocolFTP::ftpOpen(const char *host, unsigned short int port)
{
    struct sockaddr_in sin;
    struct hostent *phe;
    struct servent *pse;
    int on=1;

    ftplib_lastresp[0] = '\0';
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;

    if (port)
      sin.sin_port = htons(port);
    else
    {
      if ((pse = getservbyname("ftp","tcp")) == NULL)
      {
		perror("getservbyname");
		return 0;
      }
      sin.sin_port = pse->s_port;
    }

    if ((phe = gethostbyname(host)) == NULL)
    {
		perror("gethostbyname");
		return 0;
    }

    memcpy((char *)&sin.sin_addr, phe->h_addr, phe->h_length);
    sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sControl == -1)
    {
		perror("socket");
		return 0;
    }
    if (setsockopt(sControl,SOL_SOCKET,SO_REUSEADDR,
		   (char*)&on, sizeof(on)) == -1)
    {
		perror("setsockopt");
		close(sControl);
		return 0;
    }

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

    if (readresp('2') == 0)
    {
		close(sControl);
		free(nControl);
		return 0;
    }

    return 1;
}

/*
 * ftpSendCmd - send a command and wait for expected response
 *
 * return 1 if proper response received, 0 otherwise
 */
int KProtocolFTP::ftpSendCmd(const char *cmd, char expresp)
{
    if (ftplib_debug > 2)
		fprintf(stderr,"%s\n",cmd);
    QString buf (cmd);
    buf += "\r\n";
    if (write(sControl,buf.data(),buf.length()) <= 0)
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
int KProtocolFTP::ftpLogin( const char *user, const char *pass, QString *redirect)
{
    QString tempbuf("user ");

    ftplib_lastresp[0] = '\0';

    tempbuf += user;

    rspbuf[0]='\0';

    if (!ftpSendCmd(tempbuf,'3'))
    {
	if (rspbuf[0]=='2') return 1; /* no password required */
	return 0;
    }      
    tempbuf = "pass ";
    tempbuf += pass;

    if ( redirect == 0L )
	return ftpSendCmd(tempbuf,'2');

    if ( ftpSendCmd( tempbuf, '2' ) == 1 )
    {
	tempbuf = "pwd";
	if ( ftpSendCmd( tempbuf, '2' ) == 0 )
	    return 0;
	
	char *p = rspbuf;
	while ( isdigit( *p ) ) p++;
	while ( *p == ' ' || *p == '\t' ) p++;
	if ( *p != '"' )
	    return 1;
	char *p2 = strchr( p + 1, '"' );
	if ( p2 == 0L )
	    return 1;
	*p2 = 0;
	*redirect = p + 1;
	return 1;
    }
    else
	return 0;
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
    pasv=0;

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
		   (char*)&on,sizeof(on)) == -1)
    {
		perror("setsockopt");
		close(sDatal);
		return 0;
    }
    if (setsockopt(sDatal,SOL_SOCKET,SO_LINGER,
		   (char*)&lng,sizeof(lng)) == -1)
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

/** Use the SIZE command to get the file size. David.
    Warning : the size depends on the transfer mode, hence the second arg. */
long KProtocolFTP::ftpSize( const char *path, char mode)
{
    QString buf;
    ftplib_lastresp[0] = '\0';
    buf.sprintf("type %c",mode);
    if ( !ftpSendCmd( buf, '2' ) ) return Error(KIO_ERROR_CouldNotConnect,
				"Could not set ftp to correct mode for transmission");

    buf = "SIZE ";
    buf += path;
    if (!ftpSendCmd(buf,'2'))
	return FAIL;
    return atol(rspbuf+4); // skip leading "213 " (response code)
}

int KProtocolFTP::ftpPasv(void)
{
    int i[6], j;
    unsigned char n[6];
    int on=1;
    union {
	       struct sockaddr sa;
	       struct sockaddr_in in;
    } sin;
    struct linger lng = { 0, 0 };

    pasv=1;
    if (sDatal != -1)
       close(sDatal);
    sDatal = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(sDatal,SOL_SOCKET,SO_REUSEADDR,(char*)&on, sizeof(on)) == -1)
    {
	       perror("setsockopt");
	       close(sData);
	       return 0;
    }
    if (sDatal < 0)
    {
	       perror("socket");
	       close(sDatal);
	       return 0;
    }

    /* Let's PASsiVe*/
    if (!(ftpSendCmd("PASV",'2')))
    {
	       if (ftplib_debug>1) debug("server does not support PASV.");
	       close(sDatal);
	       return ftpPort(); // use old method if PASV not supported
    }

    if (sscanf(rspbuf, "%*[^(](%d,%d,%d,%d,%d,%d)",&i[0], &i[1], &i[2], &i[3], &i[4], &i[5]) != 6)
    {
	       perror("cannot parse PASV message.");
	       close(sDatal);
	       return ftpPort();	       
    }

    for (j=0; j<6; j++)
    {
		n[j] = (unsigned char) (i[j] & 0xff);
    }
       
    memset(&sin,0, sizeof(sin));
    sin.in.sin_family = AF_INET;
    memcpy(&sin.in.sin_addr, &n[0], (size_t) 4);
    memcpy(&sin.in.sin_port, &n[4], (size_t) 2);

    if( ::connect(sDatal, &sin.sa, sizeof(sin)) == -1)
    {
	       perror("connect");
	       close(sDatal);
	       return 0;
    }

    if (setsockopt(sDatal, SOL_SOCKET,SO_LINGER, (char *) &lng,(int) sizeof (lng)) < 0)
	       perror("Linger mode was not allowed.");
    return 1;
}

/*
 * ftpMkdir - create a directory at server
 *
 * return 1 if successful, 0 otherwise
 */
int KProtocolFTP::ftpMkdir( const char *path )
{
    QString buf("mkd ");
    buf += path;
    ftplib_lastresp[0] = '\0';
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
    QString buf("cwd ");
    buf += path;
    ftplib_lastresp[0] = '\0';
    if (!ftpSendCmd(buf,'2'))
	return 0;
    return 1;
}

 int KProtocolFTP::ftpRmdir( const char *path)
{
    QString buf("RMD ");
    buf += path;
    ftplib_lastresp[0] = '\0';
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
    struct sockaddr addr;
    int sData;
    ksize_t l;
    fd_set mask;

    FD_ZERO(&mask);
    FD_SET(sDatal,&mask);

    if (select( sDatal + 1, &mask,NULL,NULL, 0L) == 0)
    {
		close(sDatal);
		return -2;
    }
    if (pasv == 1)
    {
	    return sDatal;
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
    QString cmd("RNFR ");
    cmd += src;
    ftplib_lastresp[0] = '\0';
    if (!ftpSendCmd(cmd,'3'))
	return 0;
    cmd = "RNTO ";
    cmd += dst;
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
    QString cmd("DELE ");
    cmd += fnm;
    ftplib_lastresp[0] = '\0';
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
    ftplib_debug = 9; // set to 9 for maximum debugging output
}

KProtocolFTP::~KProtocolFTP()
{
    if( dirfile )
	CloseDir();
    Close();
}

int KProtocolFTP::OpenConnection( const char *command, const char *path, char mode )
{
    QString buf;
    ftplib_lastresp[0] = '\0';
    buf.sprintf("type %c",mode);
    if ( !ftpSendCmd( buf, '2' ) ) return Error(KIO_ERROR_CouldNotConnect,
				"Could not set ftp to correct mode for transmission");
#ifdef DONT_TRY_PASV // never defined - define if you don't want to try PASV first
    if ( !ftpPort() ) return Error(KIO_ERROR_CouldNotConnect,
				"Could not setup ftp data port", errno);
#else
    if ( !ftpPasv() ) return Error(KIO_ERROR_CouldNotConnect,
				"Could not setup ftp data port", errno);
#endif
    
    // Special case for the list command. We try to change to this
    // directory first to see whether it really is a directory.
    if ( strcmp( command, "list" ) == 0 )
    {
      QString tmp(command);
      if ( path != NULL ) {
        tmp = "cwd ";
	tmp += path;
        path = NULL; // Once cwd is done, we don't need <path> in the list
                     // command (this allows to follow links in the FTP site)
      } // otherwise use command
      
      if ( !ftpSendCmd( tmp.data(), '2' ) )
	return Error(KIO_ERROR_NotADirectory, "Error requested URL is not a directory");
    }

    QString com(command);
    if ( path != NULL ) {
      com += " ";
      com += path;
    }
    
    if ( !ftpSendCmd( com.data(), '1' ) )
      return Error(KIO_ERROR_CouldNotConnect, "Error requesting file/dir from server");

    if ( ( sData = accept_connect() ) < 0 )
    {
	if (sData == -2) perror("accept");
	else fprintf( stderr, "ERROR!: %s",rspbuf);
	return Error(KIO_ERROR_CouldNotConnect, "Could not establish data connection",errno);
    }

    return SUCCESS;
}

int KProtocolFTP::CloseConnection()
{
    /** readresp('2') ?? gibt an ob Transmission erfolgreich war! **/
    if( sData != -1 )
    {
	shutdown( sData, 2 );
	close( sData );
	sData = -1;
    }

    if( sDatal != -1 )
    {
	close( sDatal );
	sDatal = -1;
    }
    return 1;
}

int KProtocolFTP::OpenDir( KURL *url )
{
    QString path( url->path() );
    bool haspath = url->hasPath();
    
    if( Connect( url ) == FAIL )
	return (FAIL);

    // Did we get a redirect ?
    if ( path != url->path() || haspath != url->hasPath() )
    {
      // printf("Emit Redirection in kioslave\n");
	// Remove password
	KURL u( *url );
	u.setPassword("");
	emit redirection( u.url().data() );
    }
    
    if( OpenConnection( "list", url->path(), 'A' ) == FAIL )
	return (FAIL);

    dirfile = fdopen( sData, "r" );
    if( !dirfile )
	return (FAIL);
    return (SUCCESS);
}

KProtocolDirEntry *KProtocolFTP::ReadDir()
{
	char buffer[1024];
	static KProtocolDirEntry de;

	while( fgets( buffer, 1024, dirfile ) != 0 ) {

		char *p_access, *p_junk, *p_owner, *p_group;
		char *p_size, *p_date_1, *p_date_2, *p_date_3, *p_name;

		if ( (p_access = strtok(buffer," ")) != 0
				&& (p_junk   = strtok(NULL," ")) != 0 
				&& (p_owner  = strtok(NULL," ")) != 0
				&& (p_group  = strtok(NULL," ")) != 0
				&& (p_size   = strtok(NULL," ")) != 0
				&& (p_date_1 = strtok(NULL," ")) != 0
				&& (p_date_2 = strtok(NULL," ")) != 0
				&& (p_date_3 = strtok(NULL," ")) != 0
				&& (p_name   = strtok(NULL,"\r\n")) != 0 )  {

			if ( p_access[0] == 'l' ) {

				QString tmp( p_name );
				int i = tmp.findRev( " -> " );

				if ( i != -1 ) {
					tmp.truncate( i );
				}

				strcpy( p_name, tmp.data() );
			}

			de.access	= p_access;
			de.owner	= p_owner;
			de.group	= p_group;
			de.size		= atoi( p_size );
			de.isdir	= p_access[0]=='d';

			QString tmp(p_name);
			de.name	= tmp.stripWhiteSpace();

			if(de.isdir) {
				de.name += "/";
			}

			de.date.sprintf("%s %s %s",p_date_1, p_date_2, p_date_3);

			return(&de);
		}
	} // while

	return(NULL);
}

int KProtocolFTP::CloseDir()
{
    if( dirfile )
    {
	fclose( dirfile );
	dirfile = NULL;
	CloseConnection();
	ftpQuit();
    }
    return(SUCCESS);
}

int KProtocolFTP::Connect(KURL *url)
{
    QString user, passwd;

    if (!ftpOpen(url->host(), url->port()))
	return(Error(KIO_ERROR_CouldNotConnect, "Could not connect", 0));
    
    if(strlen(url->user()))
    {
	user = url->user();
	passwd = url->passwd();
        KURL::decodeURL(passwd);
    }
    else
    {
	user = FTP_LOGIN;
	passwd = FTP_PASSWD;
    }

    QString redirect;
    // Do we have the root directory and a user ?
    // ( for example ftp://weis@localhost )
    // So we can expect redirection to the home directory of user weis
    // if ( strcmp( url->path(), "/" ) == 0 && url->user() != 0L && url->user()[0] != 0 && !url->hasPath() )
    // printf("PATH=%s\n",url->path());
    if ( !url->hasPath() )
    {
      // printf("NO PATH\n");
	int ret = ftpLogin( user, passwd, &redirect );
	// We could login and got a redirect ?
	if ( ret && !redirect.isEmpty() )
	{
	    if ( redirect.right(1) != "/" )
		redirect += "/";
	    
	    // printf("REDIRECTION '%s'\n",redirect.data());
	    url->cd( redirect.data() );
	    // printf("Now URL is '%s'\n",url->url().data());
	}
	if ( ret == 1 )
	    return (SUCCESS);
	return( Error( KIO_ERROR_CouldNotLogin, "invalid passwd or username") );
    }
    
    if( !ftpLogin( user, passwd ) )
	return( Error( KIO_ERROR_CouldNotLogin, "invalid passwd or username") );
    
    return (SUCCESS);
}

// Patch from Alessandro Mirone <alex@greco2.polytechnique.fr>
// Little helper function
const char* strnextchr( const char * p , char c )
{
    while( *p != c && *p != 0L )
    {
	p++;
    }
    return p;
}         
// end patch

int KProtocolFTP::Open(KURL *url, int mode)
{
    if(Connect(url) == FAIL) return(FAIL);
    if(mode & READ)
    {
        size = ftpSize(url->path(),'I'); // try to find the size of the file
        if (ftplib_debug > 1)
            fprintf(stderr,"size set to %ld\n",size);
	int rc = OpenConnection("retr",url->path(),'I');
	if(rc == FAIL)
	    return Error(KIO_ERROR_FileDoesNotExist,"Could not retrieve file");
    
    	// Read the size from the response string, if the "size" command failed
    	if ( strlen( rspbuf ) > 4 && size == FAIL )
    	{
	    // char *p = strchr( rspbuf, '(' );
	    // Patch from Alessandro Mirone <alex@greco2.polytechnique.fr>
	    const char *p=rspbuf;
	    const char *oldp=0L;
	    while (*(p= strnextchr( p , '(' ))=='(')
	    {
		oldp=p;
		p++;
	    }
	    p = oldp;      
	    // end patch
	    if ( p != 0L ) size = atol( p + 1 );
    	}
	
	bytesleft = size;
	return(SUCCESS);
    }
    else if(mode & WRITE)
    {
	if(OpenConnection("stor",url->path(),'I') == SUCCESS)
	    return(SUCCESS);
    }
    return(FAIL);
}

int KProtocolFTP::Close()
{
    bool b = CloseConnection();
    ftpQuit();
    
    if ( b )
	return(SUCCESS);
    return(FAIL);
}

int KProtocolFTP::Delete(KURL *url)
{
    int result; 
    
    if(Connect(url) == FAIL) return(FAIL);

    if ( url->path()[ strlen( url->path() ) - 1 ] == '/' )
        result = ftpRmdir( url->path() );
    else
        result = ftpDelete( url->path() );
    
    Close();

    if (result == 1)
        return (SUCCESS);
    return(FAIL);
}

long KProtocolFTP::Size()
{
    return(size);
}

int KProtocolFTP::atEOF()
{
    return(bytesleft <= 0);
}

long KProtocolFTP::Read(void *buffer, long len)
{
    int n = read(sData,buffer,len);
    bytesleft -= n;
    return(n);
}

long KProtocolFTP::Write(void *buffer, long len)
{
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
