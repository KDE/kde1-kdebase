#include "manage.h"
#include "http.h"
#include "file.h"
#include "ftp.h"

int ProtocolSupported(KURL *url)
{
	char *prot = url->protocol();
	if(strcmp(prot,"ftp") == 0) return(1);
	if(strcmp(prot,"file") == 0) return(1);
	if(strcmp(prot,"http") == 0) return(1);
	return(0);
}

KProtocol *CreateProtocol(KURL *url)
{
	char *prot = url->protocol();
	if(strcmp(prot,"file") == 0) return(new KProtocolFILE);
	if(strcmp(prot,"http") == 0) return(new KProtocolHTTP);
	if(strcmp(prot,"ftp") == 0) return(new KProtocolFTP);
	return(NULL);
}
