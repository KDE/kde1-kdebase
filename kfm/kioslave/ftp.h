#include "protocol.h"
#include "kio_errors.h"

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif    

class KProtocolFTP :public KProtocol
{
private:
	FILE *dirfile;
	int dirfd;
	int sData,sControl,sDatal;

	// Stephan: Aua
	// #define SETSOCKOPT_OPTVAL_TYPE (void *)

	#define FTP_BUFSIZ 1024
	#define ACCEPT_TIMEOUT 30
	int Connect(class KURL *);

	struct netbuf {
    	char *cput,*cget;
    	int handle;
    	int cavail,cleft;
    	char buf[FTP_BUFSIZ];
	};

	netbuf *nControl;
	char rspbuf[256];
	char *ftplib_lastresp;
	int ftplib_debug;

	int readline(char *buf,int max,netbuf *ctl);
	int readresp(char c);
	int ftpOpen(const char *host);
	int ftpSendCmd(char *cmd, char expresp);
	int ftpLogin( const char *user, const char *pass);
	int ftpPort();
	int ftpMkdir(const char *path);
	int ftpChdir( const char *path);
 	int ftpRmdir( const char *path);
 	int accept_connect(void);
	int ftpOpenConnection(const char *command, const char *path, char mode);
	int ftpCloseConnection(int sData);
	int ftpRename( const char *src, const char *dst);
	int ftpDelete( const char *fnm );
	void ftpQuit(void);

	long size,bytesleft;

	int OpenConnection(const char *command, const char *path, char mode);
	int CloseConnection();

public:
	KProtocolFTP();
	~KProtocolFTP();

	int OpenDir(KURL *url);
	KProtocolDirEntry *ReadDir();
	int CloseDir();

	int Open(KURL *url, int mode);
	int Close();
	long Size();

	int Read(void *buffer, int len);
	int Write(void *buffer, int len);

	int atEOF();

	int MkDir(KURL *url);
};
