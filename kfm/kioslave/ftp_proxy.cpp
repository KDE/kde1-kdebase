// File: ftp_proxy.cpp by Martin Zumkley (uc2n@rzstud1.rz.uni-karlsruhe.de)
// additions to work with KFM Proxy Manager by Lars Hoss ( Lars.Hoss@munich.netsurf.de )
//
// $Id$

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <qstring.h>
#include <qmsgbox.h>

#include <kapp.h>
#include <ksimpleconfig.h>

#include "ftp_proxy.h"
#include "kio_errors.h"

/*****************************************************************************/

KProtocolProxyFTP::KProtocolProxyFTP () {

    use_proxy = 0;
    int port = 80;

    QString proxyStr;
    QString tmp;
    KURL proxyURL;
    
    // All right. Now read the proxy settings
    // KSimpleConfig prxcnf(KApplication::localconfigdir() + "/kfmrc");
    // Read Only
    KSimpleConfig prxcnf(KApplication::localconfigdir() + "/kfmrc", true );
    prxcnf.setGroup("Browser Settings/Proxy");

    noProxyForStr = prxcnf.readEntry("NoProxyFor");
    
    tmp = prxcnf.readEntry( "UseProxy" );
    if ( tmp == "Yes" ) { // Do we need proxy?
        proxyStr = prxcnf.readEntry( "FTP-Proxy" );
        proxyURL = proxyStr.data();
        // printf( "Using ftp proxy %s on port %d\n", proxyURL.host(), proxyURL.port() );
        port = proxyURL.port();
	if ( port == 0 )
	    port = 80;

	proxy_user = prxcnf.readEntry( "Proxy-User" );
	proxy_pass = prxcnf.readEntry( "Proxy-Pass" );

	init_sockaddr(&proxy_name, proxyURL.host(), port);
	use_proxy = 1;
    }
}

KProtocolProxyFTP::~KProtocolProxyFTP() {}

/* EmitData is called when the retrieved Data is of mimetype HTML
   Normaly it's after a Directory request
*/
void KProtocolProxyFTP::EmitData( KIOSlaveIPC *_ipc )
{

    char buffer[ 1024 ];
    while ( fgets( buffer, 1024, fsocket ) )
    {
	int n = strlen(buffer);
	if ( n > 0 )
	{
	    buffer[n] = 0;
	    _ipc->data( IPCMemory( buffer, n ) );
	}
    }
}

int KProtocolProxyFTP::OpenDir(KURL *_url)
{
  return Open(_url, KProtocol::READ);
}


int KProtocolProxyFTP::Open(KURL *_url, int mode){
  return OpenProxy(_url,mode,false);
}

int KProtocolProxyFTP::ReOpen(KURL *_url, int mode){
  return OpenProxy(_url,mode,true);
}

int KProtocolProxyFTP::OpenProxy(KURL *_url, int mode, bool _reload)
{
  url = _url->url().data();

  bytesRead = 0;
  startTime.start();
  currentTime.start();
  
    // Save the parameter, we could need it if we get HTTP redirection
    // See ::ProcessHeader
    currentMode = mode;
    
	if(mode != READ) return(Error(KIO_ERROR_NotImplemented,
				              "FTP Proxy currently only supports reading",0));
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
//            char *p;
//	    if ( ( p = getenv("no_proxy") ) ) {
//	         do_proxy = !revmatch(_url->host(), p);
//	    }
       	    if ( ! noProxyForStr.isEmpty() ) 
	    {
	      // printf( "host: %s\n", _url->host() );
	      // printf( "nplist: %s\n", noProxyForStr.data() );
	        do_proxy = !revmatch( _url->host(), noProxyForStr.data() );    
	    }
	}

	if(do_proxy)
	{
	  // printf("FTP::Open: connecting to proxy %s:%d\n",
	  // inet_ntoa(proxy_name.sin_addr),
	  // ntohs(proxy_name.sin_port));
		if(::connect(sock,(struct sockaddr*)(&proxy_name),sizeof(proxy_name)))
		{
	    	Error(KIO_ERROR_CouldNotConnect,"Could not connect to proxy",errno);
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

                // printf("ProxyFTP::Open: connecting to %s:%d\n",
		// inet_ntoa(server_name.sin_addr),
		// ntohs(server_name.sin_port));
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

	QString command(15              // for the constant characters
                        + strlen(_url->user())
                        + strlen(_url->host())
                        + 20            // for the port number
                        );

	if(do_proxy)
	{
		/** add hostname when using proxy **/
		int port = _url->port();
		if (! port &&  strcmp(_url->protocol(),"ftp") == 0)  // use default one
			port = 21;

		if( strlen(_url->user()) != 0 )
		    command.sprintf("GET ftp://%s@%s:%d",
		                     _url->user(), _url->host(), port);
		else
		    command.sprintf("GET ftp://%s:%d", _url->host(), port);

	} else {
		command = "GET ";
	}
	
	if ( _url->path()[0] != '/' ) command += "/";
	command += _url->httpPath(); // keep path encoded. Thanks to Josef.Weidendorfer@bmw.de

	command += " HTTP/1.0\n"; /* start header */
	command += "User-Agent: Konqueror/1.0\r\n"; /* User agent */

	if ( _reload ){ /* No caching for reload */
	  command += "Pragma: no-cache\r\n"; /* for HTTP/1.0 caches */
	  command += "Cache-control: no-cache\r\n"; /* for HTTP/>=1.1 caches */
        }
	
	command += "Host: "+QString(_url->host())+"\r\n"; /* support for HTTP 1.1 */
 
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

	command += "\n";  /* end header */
	// fprintf(stderr,"Command=%s\n",command.data());

	// write(0, command.data(), command.length());
	write(sock, command.data(), command.length());

	return(ProcessHeader());
}

int KProtocolProxyFTP::ProcessHeader()
{
  char buffer[1024];
  int len = 1;
  
  size = 0xFFFFFFF;
  
  while( len && fgets( buffer, 1024, fsocket ) )
    {
      len = strlen(buffer);
      while( len && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
	buffer[--len] = 0;
      
      if ( strncmp( buffer, "Content-length: ", 16 ) == 0 
	   ||   strncmp( buffer, "Content-Length: ", 16 ) == 0 )
	size = atol( buffer + 16 );
      else if ( strncmp( buffer, "Content-Type: ", 14 ) == 0 
		||   strncmp( buffer, "Content-type: ", 14 ) == 0 )
	{
	  emit mimeType( buffer + 14 );
	}
      else if ( strncmp( buffer, "HTTP/1.0 ", 9 ) == 0 )
	{
	  if ( (buffer[9] == '4') || (buffer[9] == '5'))
	    {
	      KURL u( url );
	      QString path = u.path();
	      /* When the requested URL is not a Directory, fail
                 with 'KIO_ERROR_NotADirectory'. Kfm will try again
                 as a file.
	      */
	      if ( (path.right(1) == "/") && (path.length() != 1 ) )
              {
		Close();
		Error(KIO_ERROR_NotADirectory,"Not a directory",errno);
		return FAIL;
	      } else { // A real error occured
		Close();
		Error(KIO_ERROR_CouldNotRead,buffer+9,errno);
		return FAIL;
	      }
	    }
	}      
      else if ( strncmp( buffer, "Location: ", 10 ) == 0 )
	{
	  Close();
	  KURL u( url );
	  KURL u2( u, buffer + 10 );
	  emit redirection( u2.url() );
	  return Open( &u2, currentMode );
	}
      
    }
  bytesleft = size;
  return(SUCCESS);
}

#include "ftp_proxy.moc"
