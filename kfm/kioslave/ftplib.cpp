#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <ctype.h>

#include "ftplib.h"

#define SETSOCKOPT_OPTVAL_TYPE (void *)

#define FTP_BUFSIZ 1024
#define ACCEPT_TIMEOUT 30

typedef struct {
    char *cput,*cget;
    int handle;
    int cavail,cleft;
    char buf[FTP_BUFSIZ];
} netbuf;

int sDatal = -1;
int sControl;
netbuf *nControl;
char rspbuf[256];
char *ftplib_lastresp = rspbuf;
int ftplib_debug = 2;

#define net_read read
#define net_write write
#define net_close close

/*
 * read a line of text
 *
 * return -1 on error or bytecount
 */
 int readline(char *buf,int max,netbuf *ctl)
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
    	if ((x = net_read(ctl->handle,ctl->cput,ctl->cleft)) == -1)
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
 int readresp(char c)
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
 int ftpOpen( const char *host)
{
    struct sockaddr_in sin;
    struct hostent *phe;
    struct servent *pse;
    int on=1;

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
    if (connect(sControl, (struct sockaddr *)&sin, sizeof(sin)) == -1)
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
 int ftpSendCmd(char *cmd, char expresp)
{
    char buf[256];
    if (ftplib_debug > 2)
	fprintf(stderr,"%s\n",cmd);
    sprintf(buf,"%s\r\n",cmd);
    if (net_write(sControl,buf,strlen(buf)) <= 0)
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
 int ftpLogin( const char *user, const char *pass)
{
    char tempbuf[64];
    int rc;
    ftplib_lastresp[0] = '\0';
    sprintf(tempbuf,"user %s",user);
    rspbuf[0]='\0';
    if (!ftpSendCmd(tempbuf,'3'))
    {
	if (rspbuf[0]=='2') return 1; /* no password required */
	return 0;
    }      
    sprintf(tempbuf,"pass %s",pass);
    return ftpSendCmd(tempbuf,'2');
}

/*
 * ftpPort - set up date connection
 *
 * return 1 if successful, 0 otherwise
 */
 int ftpPort(void)
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
	net_close(sDatal);
	return 0;
    }
    if (setsockopt(sDatal,SOL_SOCKET,SO_LINGER,
		   SETSOCKOPT_OPTVAL_TYPE &lng,sizeof(lng)) == -1)
    {
	perror("setsockopt");
	net_close(sDatal);
	return 0;
    }
    sin.in.sin_port = 0;
    if (bind(sDatal,&sin.sa,sizeof(sin)) == -1)
    {
	perror("bind");
	net_close(sDatal);
	sDatal = -1;
	return 0;
    }
    if (listen(sDatal,1) < 0)
    {
	perror("listen");
	net_close(sDatal);
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
 * ftpSite - send a SITE command
 *
 * return 1 if command successful, 0 otherwise
 */
 int ftpSite( const char *cmd)
{
    char buf[256];
    ftplib_lastresp[0] = '\0';
    sprintf(buf,"site %s",cmd);
    if (!ftpSendCmd(buf,'2'))
	return 0;
    return 1;
}

/*
 * ftpMkdir - create a directory at server
 *
 * return 1 if successful, 0 otherwise
 */
int ftpMkdir( const char *path )
{
    char buf[256];
    ftplib_lastresp[0] = '\0';
    sprintf(buf,"mkd %s",path);
    if (!ftpSendCmd(buf,'2'))
	return 0;
    return 1;
}

/*
 * ftpChdir - change path at remote
 *
 * return 1 if successful, 0 otherwise
 */
 int ftpChdir( const char *path)
{
    char buf[256];
    ftplib_lastresp[0] = '\0';
    sprintf(buf,"cwd %s",path);
    if (!ftpSendCmd(buf,'2'))
	return 0;
    return 1;
}

 int ftpRmdir( const char *path)
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
 int accept_connect(void)
{
    int sData;
    struct sockaddr addr;
    ksize_t l;
    fd_set mask;

    FD_ZERO(&mask);
    FD_SET(sDatal,&mask);

    if (select( sDatal + 1, &mask,NULL,NULL, 0L) == 0)
    {
	net_close(sDatal);
	return -2;
    }

    l = sizeof(addr);
    if ((sData = accept(sDatal,&addr,&l)) > 0)
	return sData;

    net_close(sDatal);
    return -2;
}

/*
 * ftpNlst - issue an NLST command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
 int ftpNlst( const char *outputfile, const char *path)
{
    int sData,l;
    char buf[256];
    char *dbuf;
    FILE *output;

    ftplib_lastresp[0] = '\0';
    if (!ftpSendCmd("type A",'2'))
	return 0;
    if (!ftpPort())
	return 0;
    strcpy(buf,"nlst");
    if (path != NULL)
	sprintf(buf+strlen(buf)," %s",path);
    if (!ftpSendCmd(buf,'1'))
	return 0;
    if ((sData = accept_connect()) < 0)
    {
	if (sData == -2)
	    perror("accept");
	else
	    printf("%s",rspbuf);
	return 0;
    }
    if (outputfile == NULL)
	output = stdout;
    else
    {
	output = fopen(outputfile,"w");
	if (output == NULL)
	{
	    perror(outputfile);
	    output = stdout;
	}
    }
    dbuf = (char*)malloc(FTP_BUFSIZ);
    while ((l = net_read(sData,dbuf,FTP_BUFSIZ)) > 0)
	fwrite(dbuf,1,l,output);
    if (outputfile != NULL)
	fclose(output);
    free(dbuf);
    shutdown(sData,2);
    net_close(sData);
    net_close(sDatal);
    return readresp('2');
}

/*
 * ftpDir - issue a LIST command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
int ftpDir( FILE *output, const char *path)
{
    int sData,l;
    char buf[256];
    char *dbuf;

    ftplib_lastresp[0] = '\0';
    if (!ftpSendCmd("type A",'2'))
	return 0;
    if (!ftpPort())
	return 0;
    strcpy(buf,"list");
    if (path != NULL)
	sprintf(buf+strlen(buf)," %s",path);
    if (!ftpSendCmd(buf,'1'))
	return 0;
    if ((sData = accept_connect()) < 0)
    {
	if (sData == -2)
	    perror("accept");
	else
	    printf("%s",rspbuf);
	return 0;
    }
    dbuf = (char*)malloc(FTP_BUFSIZ);
    while ((l = net_read(sData,dbuf,FTP_BUFSIZ)) > 0)
	fwrite(dbuf,1,l,output);
    free(dbuf);
    shutdown(sData,2);
    net_close(sData);
    net_close(sDatal);
    return readresp('2');
}

/*
 * ftpGet - issue a GET command and write received data to output
 *
 * return 1 if successful, 0 otherwise
 */
int ftpGet( FILE *output, const char *path, char mode, KIOSlaveIPC *ipc )
{
    int sData;
    int l;
    char buf[256];
    char *dbuf;
    int size = -1;

    ftplib_lastresp[0] = '\0';
    sprintf(buf,"type %c",mode);
    if (!ftpSendCmd(buf,'2'))
	return 0;
    if (!ftpPort())
	return 0;
    sprintf(buf,"retr %s",path);
    if (!ftpSendCmd(buf,'1'))
	return 0;

    printf("Calculating size\n");
    
    // Read the size from the response string
    if ( strlen( rspbuf ) > 4 )
    {
	char *p = strchr( rspbuf, '(' );
	if ( p != 0L )
	    size = atoi( p + 1 );
	/*
	char *p = rspbuf + 4;
	bool bend = FALSE;
	while ( !bend && *p != 0 )
	{
	    if ( isdigit( *p ) )
	    {
		size = atoi( p );
		bend = TRUE;
	    }
	    else
		p++;
	} */
    }
    
    printf("Size is '%i\n",size );
    
    if ((sData = accept_connect()) < 0)
    {
	if (sData == -2)
	    perror("accept");
	else
	    printf("%s",rspbuf);
	return 0;
    }
    
    printf("Accepted connection\n");
    
    int c = 0;
    int last = 0;
    
    dbuf = (char*)malloc(FTP_BUFSIZ);
    if (mode == 'A')
    {
	netbuf *nData;
	int dl;
	nData = (netbuf*)calloc(1,sizeof(netbuf));
	if (nData == NULL)
	{
	    perror("calloc");
	    net_close(sData);
	    net_close(sDatal);
	    return 0;
	}
	nData->handle = sData;
	while ((dl = readline(dbuf,FTP_BUFSIZ,nData))!= -1)
	{
	    if (strcmp(&dbuf[dl-2],"\r\n") == 0)
	    {
		dl -= 2;
		dbuf[dl++] = '\n';
		dbuf[dl] = '\0';
	    }
	    fwrite(dbuf,1,dl,output);
	    c += dl;
	    if ( ( c * 100 / size ) != last )
	    {
		last = ( c * 100 / size );
		ipc->progress( last );
	    }
	}
    }
    else
	while ((l = net_read(sData,dbuf,FTP_BUFSIZ)) > 0)
	{
	    printf("Read something\n");
	    fwrite(dbuf,1,l,output);
	    c += l;
	    if ( ( c * 100 / size ) != last )
	    {
		last = ( c * 100 / size );
		ipc->progress( last );
	    }
	}
    printf("Closing FTP\n");
    free(dbuf);
    shutdown(sData,2);
    net_close(sData);
    net_close(sDatal);
    
    return readresp('2');
}

/*
 * ftpPut - issue a PUT command and send data from input
 *
 * return 1 if successful, 0 otherwise
 */
int ftpPut( FILE *input, const char *path, char mode, KIOSlaveIPC *ipc, int size )
{
    int sData;
    int l;
    char buf[256];
    char *dbuf;

    ftplib_lastresp[0] = '\0';
    sprintf(buf,"type %c",mode);
    if (!ftpSendCmd(buf,'2'))
	return 0;
    if (!ftpPort())
	return 0;
    sprintf(buf,"stor %s",path);
    if (!ftpSendCmd(buf,'1'))
	return 0;
    if ((sData = accept_connect()) < 0)
    {
	if (sData == -2)
	    perror("accept");
	else
	    printf("%s",rspbuf);
	return 0;
    }

    int c = 0;
    int last = 0;
    
    dbuf = (char*)malloc(FTP_BUFSIZ);
    if (mode == 'A')
    {
	netbuf *nData;
	int dl;
	nData = (netbuf*)calloc(1,sizeof(netbuf));
	if (nData == NULL)
	{
	    perror("calloc");
	    net_close(sData);
	    net_close(sDatal);
	    fclose(input);
	    return 0;
	}
	nData->handle = fileno(input);
	while ((dl = readline(dbuf,FTP_BUFSIZ-1,nData))!= -1)
	{
	    if (dbuf[dl-1] == '\n')
	    {
		dbuf[dl-1] = '\r';
		dbuf[dl++] = '\n';
		dbuf[dl] = '\0';
	    }
	    if (net_write(sData,dbuf,dl) == -1)
	    {
		perror("write");
		break;
	    }
	    c += dl;
	    if ( ( c * 100 / size ) != last )
	    {
		last = ( c * 100 / size );
		ipc->progress( last );
	    }
	}
	free(nData);
    }
    else
	while ((l = fread(dbuf,1,FTP_BUFSIZ,input)) != 0)
	{
	    if (net_write(sData,dbuf,l) == -1)
	    {
		perror("write");
		break;
	    }
	    c += l;
	    if ( ( c * 100 / size ) != last )
	    {
		last = ( c * 100 / size );
		ipc->progress( last );
	    }
	}
   
    fclose(input);
    free(dbuf);
    if (shutdown(sData,2) == -1)
	perror("shutdown");
    if (net_close(sData) == -1)
	perror("close");
    net_close(sDatal);
    return readresp('2');
}

/*
 * ftpRename - rename a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
int ftpRename( const char *src, const char *dst)
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
int ftpDelete( const char *fnm )
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
 *
 * return 1 if successful, 0 otherwise
 */
void ftpQuit(void)
{
    ftplib_lastresp[0] = '\0';
    ftpSendCmd("quit",'2');
    free(nControl);
    net_close(sControl);
}
