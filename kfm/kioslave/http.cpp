// Additions to work with KFM Proxy Manager by Lars Hoss ( Lars.Hoss@munich.netsurf.de )

#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <unistd.h>
#include "kio_errors.h"
#include <qmsgbox.h>
#include <kstring.h>
#include <ksimpleconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <qregexp.h>

#include "../config-kfm.h"

#define MAX_HTTP_HEADER_SIZE  4096

/************************** Authorization stuff: copied from wget-source *****/

/* Encode a zero-terminated string in base64.  Returns the malloc-ed
   encoded line.  This is useful for HTTP only.

   Note that the string may not contain NUL characters.  */
char *base64_encode_line(const char *s)
{
   /* Conversion table. */
   static char tbl[64] = {
      'A','B','C','D','E','F','G','H',
      'I','J','K','L','M','N','O','P',
      'Q','R','S','T','U','V','W','X',
      'Y','Z','a','b','c','d','e','f',
      'g','h','i','j','k','l','m','n',
      'o','p','q','r','s','t','u','v',
      'w','x','y','z','0','1','2','3',
      '4','5','6','7','8','9','+','/'
   };
   int len, i;
   char *res;
   unsigned char *p;

   len = strlen(s);
    res = (char *)malloc(4 * ((len + 2) / 3) + 1);
   p = (unsigned char *)res;
   /* Transform the 3x8 bits to 4x6 bits, as required by
      base64.  */
   for (i = 0; i < len; i += 3)
   {
      *p++ = tbl[s[0] >> 2];
      *p++ = tbl[((s[0] & 3) << 4) + (s[1] >> 4)];
      *p++ = tbl[((s[1] & 0xf) << 2) + (s[2] >> 6)];
      *p++ = tbl[s[2] & 0x3f];
      s += 3;
   }
   /* Pad the result if necessary... */
   if (i == len + 1)
      *(p - 1) = '=';
   else if (i == len + 2)
      *(p - 1) = *(p - 2) = '=';
   /* ...and zero-teminate it.  */
   *p = '\0';
   return res;
}

char *create_generic_auth (const char *prefix, const char *user, const char *passwd )
{
   char *wwwauth;

   if (user && passwd)
   {
      char *t1, *t2;

      t1 = (char *)malloc(strlen(user) + 1 + 2 * strlen(passwd) + 1);
      sprintf(t1, "%s:%s", user, passwd);
      t2 = base64_encode_line(t1);
      free(t1);
      // wwwauth = (char *)malloc(strlen(t2) + 24);
      // sprintf(wwwauth, "Authorization: Basic %s\r\n", t2);
      wwwauth = (char *)malloc(strlen(t2) + strlen(prefix) + 11 /* UPDATE WHEN FORMAT BELOW CHANGES !!! */);
      sprintf(wwwauth, "%s: Basic %s\r\n", prefix, t2);
      free(t2);
   }
   else
      wwwauth = NULL;
   return(wwwauth);
}

char *create_www_auth(const char *user, const char *passwd)
{
  return create_generic_auth("Authorization",user, passwd);
}

/*****************************************************************************/

KProtocolHTTP::KProtocolHTTP()
{
    connected = 0;
    use_proxy = 0;
    

    int port = 80, i=0, count;

    QString proxyStr;
    QString tmp;
    KURL proxyURL;
    
    // All right. Now read the proxy settings
    // prxcnf changed to kfmcnf as not only proxy settings are to be read
    KSimpleConfig kfmcnf(KApplication::localconfigdir() + "/kfmrc", true );
    kfmcnf.setGroup("Browser Settings/Proxy");

    noProxyForStr = kfmcnf.readEntry("NoProxyFor");
    
    tmp = kfmcnf.readEntry( "UseProxy" );
    if ( tmp == "Yes" ) { // Do we need proxy?
        proxyStr = kfmcnf.readEntry( "HTTP-Proxy" );
        proxyURL = proxyStr.data();

        port = proxyURL.port();
	if ( port == 0 )
	    port = 80;

	init_sockaddr(&proxy_name, proxyURL.host(), port);

	proxy_user = kfmcnf.readEntry( "Proxy-User" );
	proxy_pass = kfmcnf.readEntry( "Proxy-Pass" );

	use_proxy = 1;
	
    }

    // Jacek:
    // All right. Now read the HTTP settings
    kfmcnf.setGroup("Browser Settings/HTTP");
    assumeHTML = kfmcnf.readBoolEntry( "AssumeHTML",false );
    tmp = kfmcnf.readEntry( "AcceptLanguages","" );
    PrepareLanguageList(tmp); // prepare list for use in HTTP header
    tmp = kfmcnf.readEntry( "AcceptCharsets","" );
    PrepareCharsetList(tmp); // prepare list for use in HTTP header
    kfmcnf.setGroup("Browser Settings/UserAgent");
    count = kfmcnf.readNumEntry("EntriesCount",-1);
    agent_strings = new QStrList();
    if (count==-1 || count==0) {
      agent_strings->append(DEFAULT_USERAGENT_STRING);
    } else {
       for (i=0; i<count; i++) {
         QString ent;
         ent.sprintf("Entry%d",i);
         agent_strings->append(kfmcnf.readEntry(ent, ":"));
       }
    }
}

KProtocolHTTP::~KProtocolHTTP()
{
    Close();
}

// Jacek:
void KProtocolHTTP::PrepareLanguageList(QString str){
 
  if (str.isEmpty()){
    languages="";
    return;
  }  
  
  QStrList lst;
  while(!str.isEmpty()){
    // find separators, we don't need to limit the user to any of them... :-)
    int f=str.find(QRegExp("[;,: ]"));
    QString lang;
    if (f<0){
      lang=str;
      str="";
    }
    else{
      lang=str.left(f);
      str=str.right(str.length()-f-1);
    }
 
    
    QString lng,ct,chset;
    
    // skip "C" as it is alway in locale languages list
    // english is the last accepted language anyway
    if (lang=="C") continue;
    // * means any other language (after it we can put languages we don't like :-) )
    else if (lang=="*" && !lst.contains("*")) lst.append("*"); 
    else{
      KLocale::splitLocale(lang,lng,ct,chset);
      // if language and country modifier is given use them
      if (!lng.isEmpty() && !ct.isEmpty() && !lst.contains(lng+"-"+ct))
          lst.append(lng+"-"+ct);
      // use language alone anyway
      // (if user requested US english he probably will understand british too)
      if (!lng.isEmpty() && !lst.contains(lng+"-"+ct)) lst.append(lng);
    }  
  }

  // if there is no language in the list we don't need Accept-Language
  if (lst.isEmpty()){
    languages="";
    return;
  }
  
  // and add the default language. Without this we would get nothing from most sites
  if (!lst.contains("en")) lst.append("en");

  // We start from the highest quality (default = 1)
  // and go down by 0.01 (I don't think anybody will choose 100 languages)
  // and RFC does not say if quality has to be positive :-)
  float quality=1;
  const char *l=lst.first();
  // Add first language with default quality (1)
  languages=l;
  for(l=lst.next();l!=0;l=lst.next()){
     quality-=0.01;
     // Add every next language with lower quality
     languages+=QString(", ")+l+";q="+QString().setNum(quality,'f',2);
  }
}

void KProtocolHTTP::PrepareCharsetList(QString str){
 
  if (str.isEmpty()){
    charsets="";
    return;
  }  
  
  QStrList lst;
  while(!str.isEmpty()){
    // find separators, we don't need to limit the user to any of them... :-)
    int f=str.find(QRegExp("[;,: ]"));
    QString chset;
    if (f<0){
      chset=str;
      str="";
    }
    else{
      chset=str.left(f);
      str=str.right(str.length()-f-1);
    }
    
    // * means any other charset (after it we can put charsets we don't like :-) )
    // RFC doesn't tell anything about it (it tells about "*" in  Accept-Language)
    // but Netscape uses it, so we use it too
    if (chset=="*" && !lst.contains("*")) lst.append("*"); 
    else if (!chset.isEmpty() && !lst.contains(chset)) lst.append(chset);
  }

  // if there is no charset in the list we don't need Accept-Charset
  if (lst.isEmpty()){
    charsets="";
    return;
  }
  
  // and add the default charset. Without this we would get nothing from most sites
  if (!lst.contains("iso-8859-1")) lst.append("iso-8859-1");

  // We start from the highest quality (default = 1)
  // and go down by 0.01 (I don't think anybody will choose 100 charsets)
  // and RFC does not say if quality has to be positive :-)
  float quality=1;
  const char *chs=lst.first();
  // Add first charset with default quality (1)
  charsets=chs;
  for(chs=lst.next();chs!=0;chs=lst.next()){
     quality-=0.01;
     // Add every next language with lower quality
     charsets+=QString(", ")+chs+";q="+QString().setNum(quality,'f',2);
  }
}

long KProtocolHTTP::Size()
{
    return(size);
}

int KProtocolHTTP::Close()
{
    if ( connected )
	close( sock );
    connected = 0;
    return SUCCESS;
}

int KProtocolHTTP::atEOF()
{
    return( bytesleft == 0 || feof( fsocket ) );
}

long KProtocolHTTP::Read(void *buffer, long len)
{
    if( atEOF() )
	return 0;

    long nbytes = fread(buffer,1,len,fsocket);
    bytesRead += nbytes;
    bytesleft -= nbytes;

    // Print only every second new status information
    if ( currentTime.elapsed() >= 1000 )
    {
	currentTime.start();
	int secs = startTime.secsTo( currentTime );
	if ( secs == 0 )
	    secs = 1;
	long bytesPerSec = bytesRead / secs;
	QString infoStr;
	if ( bytesPerSec < 1000 )
	    infoStr.sprintf( "%li bytes/s", bytesPerSec );
	else
	    infoStr.sprintf( "%.1f kb/s", (float)bytesPerSec / (float)1000 );
	
	// Do we know about the remaing bytes ?
	if ( size != 0xFFFFFFF && bytesPerSec > 0 )
	{
	    QTime leftTime( 0, 0 );
	    leftTime = leftTime.addSecs( bytesleft / bytesPerSec );
	    infoStr += "  ";
	    infoStr += leftTime.toString();
	}
	
	emit info( infoStr );
    }
    
    if ( ferror(fsocket))
    {
        Error( KIO_ERROR_CouldNotRead,"Reading from socket failed", errno);
        return FAIL;
    }
    return nbytes;
}

int KProtocolHTTP::init_sockaddr(struct sockaddr_in *server_name, const char *hostname, int port)
{
	struct hostent *hostinfo;
	server_name->sin_family = AF_INET;
	server_name->sin_port = htons( port );

	hostinfo = gethostbyname( hostname );

	if ( hostinfo == 0L ) {
	    // init_sockaddr failed, we should probably check errno
	    return(FAIL);
	}
	server_name->sin_addr = *(struct in_addr*) hostinfo->h_addr;
	return(SUCCESS);
}

/* Domain suffix match. E.g. return 1 if host is "cuzco.inka.de" and
   nplist is "inka.de,hadiko.de" or if host is "localhost" and
   nplist is "localhost" */
   
int revmatch(const char *host, const char *nplist)
{
    const char *hptr = host + qstrlen(host) - 1;
    const char *nptr = nplist + qstrlen(nplist) - 1;
    const char *shptr = hptr;
    
    while( nptr >= nplist ) {
        if ( *hptr != *nptr ) 
        {
            hptr = shptr; 
            // Try to find another domain or host in the list
            while ( --nptr>=nplist && *nptr!=',' && *nptr!=' ') 
                ;            
            while ( --nptr>=nplist && (*nptr==',' || *nptr==' '))
                ;
        } else {
            if ( nptr==nplist || nptr[-1]==',' || nptr[-1]==' ') 
            { 
                return 1;
            }
            hptr--;
            nptr--;
        }
    }
    
    return 0;
}

int KProtocolHTTP::SetData(const char *_data)
{
    post_data = _data;
    return SUCCESS;
}

int KProtocolHTTP::SetCookies(const char *_cookies)
{
    Cookies = _cookies;
    return SUCCESS;
}

int KProtocolHTTP::Open( KURL *_url, int mode )
{
  return OpenHTTP( _url, mode, false );
}

int KProtocolHTTP::ReOpen( KURL *_url, int mode )
{
  return OpenHTTP( _url, mode, true );
}

int KProtocolHTTP::OpenHTTP( KURL *_url, int mode,bool _reload )
{
    url = _url->url().data();
  
    bytesRead = 0;
    startTime.start();
    currentTime.start();

    // Save the parameter, we could need it if we get HTTP redirection
    // See ::ProcessHeader
    currentMode = mode;
    
	if(mode != READ) return(Error(KIO_ERROR_NotImplemented,
					              "HTTP currently only supports reading",0));
	if (connected) Close();

	sock = ::socket(PF_INET,SOCK_STREAM,0);

	if (sock < 0)
	{
	    Error(KIO_ERROR_CouldNotCreateSocket,"Could not create socket",errno);
	    return(FAIL);
	}

	int do_proxy = use_proxy;
	if (do_proxy)
	{
	    if ( ! noProxyForStr.isEmpty() ) 
	    {
	        do_proxy = !revmatch( _url->host(), noProxyForStr.data() );    
	    }
	}

	if(do_proxy)
	{
	  //HTTP::Open: connecting to proxy
		if(::connect(sock,(struct sockaddr*)(&proxy_name),sizeof(proxy_name)))
		{
		    	Error(KIO_ERROR_CouldNotConnect,"Could not connect to proxy", errno);
		    	return(FAIL);
		}
	}
	else
	{
		struct sockaddr_in server_name;
		int port = _url->port();
		if ( port == 0 )
			port = 80;

		if(init_sockaddr(&server_name, _url->host(), port) == FAIL)
		{
    		Error(KIO_ERROR_UnknownHost, "Unknown host", errno );
			return(FAIL);
		}

		if(::connect(sock,(struct sockaddr*)(&server_name),sizeof(server_name)))
		{
	    	Error(KIO_ERROR_CouldNotConnect, "Could not connect host", errno);
			return(FAIL);
		}
 	}
	connected = 1;

	fsocket = fdopen(sock,"r+");
	if(!fsocket)
	{
	    Error(KIO_ERROR_CouldNotConnect, "Could not fdopen socket", errno);
	    return(FAIL);
	}

	QString command;
	
	if (post_data.isEmpty())
	{
            command = "GET ";
        }
        else
        {
            _reload = TRUE;     /* no caching allowed */
            command = "POST ";
        }

	if(do_proxy)
	{
		int port = _url->port();
		if ( !port )  // use default one
			port = 80;
		command += "http://";
		command << _url->host() << ":" << port;
	}

	if ( _url->httpPath()[0] != '/' ) command += "/";
	command += _url->httpPath();
	if (_url->searchPart() != NULL)
	{
	    command += "?";
	    command += _url->searchPart();
	}
	command += " HTTP/1.0\r\n"; /* start header, we're HTTP/1.1 compliant, not! */
	command += "Connection: close\r\n"; /* Make sure we don't use persistant connections */

	QString _host = _url->host(); QString agent_string="";
	bool found=false;

	
	for (agent_strings->first(); agent_strings->current(); agent_strings->next()) {
          
          agent_string = QString(agent_strings->current());

	  if (!strcmp(agent_string.data(), ":")) // If it's a NULL marker.
	    continue;

	  if (agent_string.find(":") < 1)  // If it's got no :
	    continue;

          QRegExp agent_host(agent_string.mid(0, agent_string.find(":")), true, true);
	  if (agent_host.isValid() && !agent_host.match(_host)) {
	    agent_string = agent_string.mid(agent_string.find(":")+1,agent_string.length());
	    found=true;
	    break;
	  }
	}
	if (!found)
	  agent_string = QString(DEFAULT_USERAGENT_STRING).copy();

	command += "User-Agent: "+agent_string+"\r\n"; /* User agent */
	
	if ( _reload ){ /* No caching for reload */
	  command += "Pragma: no-cache\r\n"; /* for HTTP/1.0 caches */
	  command += "Cache-control: no-cache\r\n"; /* for HTTP/>=1.1 caches */
        }

	// Charset negotiation:
	if ( ! charsets.isEmpty() )
	   command += "Accept-Charset: "+charsets+"\r\n";
	   
	// Language negotiation:
	if ( ! languages.isEmpty() )
	   command += "Accept-Language: "+languages+"\r\n";
	
	command += "Host: "; /* support for virtual hosts, required by HTTP/1.1 */
	command += _url->host();
	if ( _url->port() != 0 )
	{
	    command += ":";
	    QString tmp;
	    tmp.setNum( _url->port() );
	    command += tmp;
	}
	command += "\r\n";

        if (!Cookies.isEmpty())
        {   
            command += Cookies;
        }

        if (!post_data.isEmpty())
        {
            command += "Content-Type: application/x-www-form-urlencoded\r\n"
                       "Content-Length: ";
            QString tmp;
            tmp.setNum( post_data.length());
            command += tmp;
	    command += "\r\n";
        }
 
	if( strlen(_url->user()) != 0 )
	{
		char *www_auth = create_www_auth(_url->user(),_url->passwd());
		command += www_auth;
		free(www_auth);
	}

	if( do_proxy )
	{
	  if( proxy_user != "" && proxy_pass != "" )
          {
	    char *www_auth = create_generic_auth("Proxy-authorization", proxy_user, proxy_pass);
	    command += www_auth;
	    free(www_auth);
	  }
	}

	command += "\r\n";  /* end header */
        if (!post_data.isEmpty())
        {
            command += post_data;
        }

	// write(0, command.data(), command.length());
	write(sock, command.data(), command.length());

	return(ProcessHeader());
}

int KProtocolHTTP::ProcessHeader()
{
	char buffer[MAX_HTTP_HEADER_SIZE];
	QString mType;
	QString Cookie;
	int len = 1;

	size = 0xFFFFFFF;
	
	// Jacek:
	// to get rid of those "Open with" dialogs...
	// however at least extensions should be checked
	if (assumeHTML) mType="text/html";
	
	while( len && fgets( buffer, MAX_HTTP_HEADER_SIZE, fsocket ) )
	{
	    len = strlen(buffer);
	    while( len && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
		buffer[--len] = 0;

	    if ( strncasecmp( buffer, "Set-Cookie", 10 ) == 0 )
	    {
	        Cookie += buffer;
	        Cookie += "\n";
	    } 
	    else if ( strncasecmp( buffer, "Content-Length: ", 16 ) == 0 )
	    {
		size = atol( buffer + 16 );
            }
	    else if ( strncasecmp( buffer, "Content-Type: ", 14 ) == 0 )
	    {
	    // Jacek: We can't send mimeType signal now,
	    //    because there may be another Content-Type to come
	      mType = buffer+14;
	    }
	    else if ( strncmp( buffer, "HTTP/1.0 ", 9 ) == 0 )
	    {
	        //Jacek: server error codes added (5xx)
		if ( buffer[9] == '4' ||  buffer[9] == '5' )
		{
		    Close();
		    Error(KIO_ERROR_CouldNotRead,buffer+9,errno);
		    // We have got problem here:
		    // If we fail we won't see error page,
		    // But the operation isn't successful either
		    // Let's try to continue - we callesd Error anyway
		  // return FAIL;
		}
	    }
	    // In fact we should do redirection only if we got redirection code
	    else if ( strncasecmp( buffer, "Location: ", 10 ) == 0 )
	    {
		Close();
		KURL u( url );
		KURL u2( u, buffer + 10 );
		if (!u2.isMalformed())
		{
		  if (!Cookie.isEmpty())
		  {
	            emit cookie( url.data(), Cookie.data() );
		  }
		  emit redirection( u2.url() );
//		  return Open( &u2, currentMode );
		  bytesleft = 0;
		  return (SUCCESS);
		}
		else
		{
		  Error(KIO_ERROR_MalformedURL, buffer+10, 0);
		  return FAIL;
		}
	    }
	    
	}
	emit mimeType( mType );
	if (!Cookie.isEmpty())
	{
	   emit cookie( url.data(), Cookie.data() );
	}
	bytesleft = size;
	return(SUCCESS);
}

#include "http.moc"
