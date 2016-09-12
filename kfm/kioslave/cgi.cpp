// $Id: cgi.cpp,v 1.7 1999/01/09 23:03:40 garbanzo Exp $

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

#include <kapp.h>

#include "cgi.h"

KProtocolCGI::KProtocolCGI()
{
    fsocket = 0L;
}

KProtocolCGI::~KProtocolCGI()
{
    Close();
}

int KProtocolCGI::Close()
{
    connected = FALSE;
    
    if ( fsocket )
	pclose( fsocket );

    fsocket = 0L;
    
    return SUCCESS;
}

/* int KProtocolCGI::atEOF()
{
    return( bytesleft == 0 || feof( fsocket ) );
} */

long KProtocolCGI::Read( void *buffer, long len )
{
    if ( atEOF() )
	return( 0 );
    
    long nbytes = fread( buffer, 1, len, fsocket );
    
    bytesleft -= nbytes;
    if ( nbytes >= 0 )
	return nbytes;
    
    Error(KIO_ERROR_CouldNotRead,"Reading from CGI failed", errno);
    return(FAIL);
}

int KProtocolCGI::Open( KURL *url, int mode )
{
    currentMode = mode;

    if( mode != READ )
	return( Error(KIO_ERROR_NotImplemented, "CGI only supports reading", 0 ) );

    if ( fsocket ) 
	Close();
    
    QString u = url->path();

    // extract query
    int qPos = u.find( '?' );
    
    QString query = "";
    if ( qPos > 0 )
	query = u.right( u.length() - qPos - 1 );
    
    QString script;
    
    if ( qPos > 0 )
	script = u.left( qPos );
    else
	script = u.data();
    
    // extract path info
    int pathPos = script.find( '/', 9 );
    
    QString pathInfo;
    
    if ( pathPos >= 0 )
    {
	pathInfo = script.right( script.length() - pathPos - 1 );
	script.truncate( pathPos );
    }
    
    QString command;	
    
    if (!strncmp(script.data(), "/cgi-bin", 8))
	command = KApplication::kde_cgidir() + script.right(script.length() - 8);
    else
	command = KApplication::kde_cgidir() + script;

    setenv( "QUERY_STRING", query.data(), true );
    // setenv( "PATH_INFO", pathInfo.data(), true );
	
    fsocket = popen( command, "r" );
	
    if ( fsocket == NULL )
    {
	printf("Failed\n");
	return FAIL;
    }

    connected = TRUE;
    
    return ProcessHeader();    
}

#include "cgi.moc"
