// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include <errno.h>
#include <unistd.h>

#include "kfmserver_ipc.h"
#include "utils.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

KfmIpcServer::KfmIpcServer()
{
    // Keep in sync with the same in main.cpp!
    QString idir;
    idir.sprintf(_PATH_TMP"/kfm_%i_%i%s",(int)getuid(),(int)getpid(),displayName().data());
    
    serv_sock = new KServerSocket( idir.data() );
    if ( serv_sock->socket() < 0 )
    {
	fprintf( stderr, "ERROR: Could not establish server socket\n");
	exit(1);
    }
    connect( serv_sock, SIGNAL( accepted(KSocket*) ), 
	    this, SLOT( slotAccept(KSocket*) ) );
}

void KfmIpcServer::slotAccept( KSocket *_sock )
{
    KfmIpc * p = new KfmIpc( _sock );
    emit newClient( p );
}

KfmIpcServer::~KfmIpcServer()
{
    delete serv_sock;
}

KfmIpc::KfmIpc( KSocket *_sock )
{
    bHeader = TRUE;
    cHeader = 0;
    pBody = 0L;

    connect( _sock, SIGNAL( readEvent(KSocket*) ), this, SLOT( readEvent(KSocket*) ) );
    connect( _sock, SIGNAL( closeEvent(KSocket*) ), this, SLOT( closeEvent(KSocket*) ) );
    _sock->enableRead( TRUE );
    data_sock = _sock;
}

KfmIpc::~KfmIpc()
{
    data_sock->enableRead( FALSE );
    delete data_sock;
    if ( pBody != 0L )
	free( pBody );
}

void KfmIpc::closeEvent( KSocket * )
{
    delete this;
    return;
}

void KfmIpc::readEvent( KSocket * )
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

void KfmIpc::parse( char *_data, int _len )
{
    int pos = 0;
    char *name = read_string( _data, pos, _len );
    if ( name == 0L )
	return;
    _data += pos;
    _len -= pos;
	if ( strcmp( name, "refreshDesktop" ) == 0 ) { parse_refreshDesktop( _data, _len ); } else
	if ( strcmp( name, "refreshDirectory" ) == 0 ) { parse_refreshDirectory( _data, _len ); } else
	if ( strcmp( name, "openURL" ) == 0 ) { parse_openURL( _data, _len ); } else
	if ( strcmp( name, "openProperties" ) == 0 ) { parse_openProperties( _data, _len ); } else
	if ( strcmp( name, "list" ) == 0 ) { parse_list( _data, _len ); } else
	if ( strcmp( name, "exec" ) == 0 ) { parse_exec( _data, _len ); } else
	if ( strcmp( name, "copy" ) == 0 ) { parse_copy( _data, _len ); } else
	if ( strcmp( name, "move" ) == 0 ) { parse_move( _data, _len ); } else
	if ( strcmp( name, "moveClient" ) == 0 ) { parse_moveClient( _data, _len ); } else
	if ( strcmp( name, "copyClient" ) == 0 ) { parse_copyClient( _data, _len ); } else
	if ( strcmp( name, "sortDesktop" ) == 0 ) { parse_sortDesktop( _data, _len ); } else
        if ( strcmp( name, "configure" ) == 0 ) { parse_configure( _data, _len ); } else
	if ( strcmp( name, "auth" ) == 0 ) { parse_auth( _data, _len ); } else
	if ( strcmp( name, "selectRootIcons" ) == 0 ) { parse_selectRootIcons( _data, _len ); } else
		return;
	free_string( name );
}

#include "kfmserver_ipc.moc"
