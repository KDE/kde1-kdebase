#include "manage.h"
#include "subprotocol.h"
#include "http.h"
#include "file.h"
#include "ftp.h"
#include "tar.h"
#include "cgi.h"

int ProtocolSupported(const char *url)
{
	char *urlcp = strdup(url);
	char *pptr, *eptr;
	int supported = 1;

	pptr = urlcp;

	do
	{
		eptr = strchr(pptr,'#');
		if(eptr)
		{
			*eptr = 0;
			supported &= HaveProto(pptr);
			pptr = eptr+1;
		}
	}	while(eptr);
	supported &= HaveProto(pptr);

	free(urlcp);
	printf("url %s supported = %d\n",url, supported);
	return(supported);
}

int HaveProto(const char *url)
{
	printf("Checking for support: %s\n",url);
	// Something on the local file system ?
	if( url[0] == '/' ) return 1;
	if(strncmp(url,"ftp:",4) == 0) return(1);
	if(strncmp(url,"tar:",4) == 0) return(1);
	if(strncmp(url,"file:",5) == 0) return(1);
	if(strncmp(url,"http:",5) == 0) return(1);
	if(strncmp(url,"cgi:",4) == 0) return(1);
	return(0);
}

const char *TopLevelURL(const char *url)
{
	const char *lasturl;
	if( ( lasturl = strrchr(url,'#') ) ) return( lasturl+1 );
	return( url );
}

KProtocol *CreateProtocol(const char *url)
{
	const char *lasturl = TopLevelURL(url);

	if( lasturl[0] == '/' ) return(new KProtocolFILE);
	if(strncmp(lasturl,"file:",5) == 0) return(new KProtocolFILE);
	if(strncmp(lasturl,"http:",5) == 0) return(new KProtocolHTTP);
	if(strncmp(lasturl,"ftp:",4) == 0) return(new KProtocolFTP);
	if(strncmp(lasturl,"cgi:",4) == 0) return(new KProtocolCGI);
	if(strncmp(lasturl,"tar:",4) == 0)
	{
		KSubProtocol *sub = new KProtocolTAR;
		char *urlcp = strdup(url);
		char *urlstart;
		if( ( urlstart = strrchr(urlcp,'#') ) ) *urlstart = 0;
		char *parenturlname = strrchr(urlcp,'#');
		if(!parenturlname) parenturlname=urlcp;

		KURL parenturl(parenturlname);
		sub->InitSubProtocol(urlcp,parenturlname);
		free(urlcp);
		return(sub);
	}
	return(NULL);
}

/* char *ReformatURL(const char *url)
{
	QString reformattedurl;
	if(strncmp(url,"tar:",4) == 0)
	{
		printf("WARNING: Old KFM-style URL found: '%s'\n", url);
		char *urlcp = strdup(url);
		char *path=strrchr(urlcp,'#');
		*path = 0;
		path++;
		reformattedurl.sprintf("file:%s#tar:/%s",urlcp+4,path);
		url=reformattedurl.data();
		printf("WARNING: URL changed to '%s'\n",url);
		free(urlcp);
	}
	return(strdup(url));
} */
