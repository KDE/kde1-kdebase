#include "protocol.h"
#include "kio_errors.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>

#include <stdio.h>

class KProtocolHTTP :public KProtocol
{
private:

	//#define HTTP_PROXY_NAME "someproxy.somewhere.org"
	//#define HTTP_PROXY_PORT 80
	
	struct sockaddr_in proxy_name;
	int use_proxy, connected, sock;
	long size;
	long bytesleft;
	FILE *fsocket;

	int init_sockaddr(struct sockaddr_in *server_name, char *hostname,int port);
	int ProcessHeader();
public:
	KProtocolHTTP();
	~KProtocolHTTP();

	int Open(KURL *url, int mode);
	int Close();
	int Read(void *buffer, int len);

	long Size();

	int atEOF();
};
