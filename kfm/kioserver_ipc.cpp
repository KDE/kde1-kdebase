// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include <errno.h>
#include <unistd.h>

#include "kioserver_ipc.h"
#include "utils.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

KIOSlaveIPCServer::KIOSlaveIPCServer()
{
    // Keep in sync with the same in kioserver.cpp!
    QString idir;
    idir.sprintf(_PATH_TMP"/kio_%i_%i%s",(int)getuid(), (int)getpid(),displayName().data());
    
    serv_sock = new KServerSocket( idir.data() );
    if ( serv_sock->socket() < 0 )
    {
	fprintf( stderr, "ERROR: Could not establish server socket!\n");
	exit(1);
    }
    connect( serv_sock, SIGNAL( accepted(KSocket*) ), 
	    this, SLOT( slotAccept(KSocket*) ) );
}

void KIOSlaveIPCServer::slotAccept( KSocket *_sock )
{
    KIOSlaveIPC * p = new KIOSlaveIPC( _sock );
    emit newClient( p );
}

KIOSlaveIPCServer::~KIOSlaveIPCServer()
{
    delete serv_sock;
}

KIOSlaveIPC::KIOSlaveIPC( KSocket *_sock )
{
    bHeader = TRUE;
    cHeader = 0;
    pBody = 0L;

    connect( _sock, SIGNAL( readEvent(KSocket*) ), this, SLOT( readEvent(KSocket*) ) );
    connect( _sock, SIGNAL( closeEvent(KSocket*) ), this, SLOT( closeEvent(KSocket*) ) );
    _sock->enableRead( TRUE );
    data_sock = _sock;
}

KIOSlaveIPC::~KIOSlaveIPC()
{
    /**
     * MODIFIED
     */
    emit closed( this );
   
    // data_sock->enableRead( FALSE );
    delete data_sock;
    if ( pBody != 0L )
	free( pBody );
}

void KIOSlaveIPC::closeEvent( KSocket * )
{
  // printf("CLOSE EVENT\n");
    delete this;
    return;
}

void KIOSlaveIPC::readEvent( KSocket * )
{
    if ( bHeader )
    {
	int n;
    next:
	if ( ( n = read( data_sock->socket(), headerBuffer + cHeader, 1 ) ) < 0 )
        {
	  if ( errno == EINTR )
	    goto next;
	  fprintf( stderr, "ERROR: KIOSlaveIPC::readEvent\n");
	  delete this;
	  return;
        }
	
	if ( headerBuffer[ cHeader ] == ' ' )
	{
	    bHeader = FALSE;
	    cHeader = 0;
	    bodyLen = atoi( headerBuffer );
	    cBody = 0;
	    if ( bodyLen <= 0 )
	    {
		fprintf( stderr, "ERROR: Invalid header\n");
		delete this;
		return;
	    }
	    if ( pBody != 0L )
		free( pBody );
	    pBody = (char*)malloc( bodyLen + 1 );
	}
	else if ( cHeader + n == 10 )
	  {
	    fprintf( stderr, "ERROR: Too long header\n");
	    delete this;
	    return;
	}
	else
	{
	    if ( !isdigit( headerBuffer[ cHeader ] ) )
	    {
		fprintf( stderr, "ERROR: Header must be an int\n");
		delete this;
		return;
	    }

	    cHeader += n;
	    return;
	}
    }
	
    int n;
next2:
    if ( ( n = read( data_sock->socket(), pBody + cBody, bodyLen - cBody ) ) < 0 )
    {
      if ( errno == EINTR )
	goto next2;
      fprintf( stderr, "ERROR: KIOSlaveIPC::readEvent\n");
      delete this;
      return;
    }

    if ( n + cBody == bodyLen )
    {
	pBody[bodyLen] = 0;
	bHeader = TRUE;
	parse( pBody, bodyLen );
	return;
    }
    cBody += n;
}

void KIOSlaveIPC::parse( char *_data, int _len )
{
    int pos = 0;
    char *name = read_string( _data, pos, _len );
    if ( name == 0L )
	return;
    _data += pos;
    _len -= pos;
	if ( strcmp( name, "hello" ) == 0 ) { parse_hello( _data, _len ); } else
	if ( strcmp( name, "progress" ) == 0 ) { parse_progress( _data, _len ); } else
	if ( strcmp( name, "info" ) == 0 ) { parse_info( _data, _len ); } else
	if ( strcmp( name, "dirEntry" ) == 0 ) { parse_dirEntry( _data, _len ); } else
	if ( strcmp( name, "data" ) == 0 ) { parse_data( _data, _len ); } else
	if ( strcmp( name, "flushDir" ) == 0 ) { parse_flushDir( _data, _len ); } else
	if ( strcmp( name, "done" ) == 0 ) { parse_done( _data, _len ); } else
	if ( strcmp( name, "fatalError" ) == 0 ) { parse_fatalError( _data, _len ); } else
	if ( strcmp( name, "setPID" ) == 0 ) { parse_setPID( _data, _len ); } else
	if ( strcmp( name, "redirection" ) == 0 ) { parse_redirection( _data, _len ); } else
	if ( strcmp( name, "mimeType" ) == 0 ) { parse_mimeType( _data, _len ); } else
	if ( strcmp( name, "cookie" ) == 0 ) { parse_cookie( _data, _len ); } else
		return;
	free_string( name );
}

#include "kioserver_ipc.moc"
