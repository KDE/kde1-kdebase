// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include "kfmserver_ipc.h"

KfmIpcServer::KfmIpcServer()
{
    serv_sock = new KServerSocket( 0 ); /* 0: choose free port */
    if ( serv_sock->socket() < 0 )
    {
	debugT("ERROR: Could not establish server\n");
	exit(1);
    }
    debugT( "SOCK=%i\n",serv_sock->getPort());
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

int KfmIpcServer::getPort()
{
    return serv_sock->getPort();
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

void KfmIpc::closeEvent( KSocket *_sock )
{
    delete this;
    return;
}

void KfmIpc::readEvent( KSocket *_sock )
{
    if ( bHeader )
    {
	int n;
	n = read( data_sock->socket(), headerBuffer + cHeader, 1 );
	if ( headerBuffer[ cHeader ] == ' ' )
	{
	    bHeader = FALSE;
	    cHeader = 0;
	    bodyLen = atoi( headerBuffer );
	    cBody = 0;
	    if ( bodyLen <= 0 )
	    {
		debugT("ERROR: Invalid header\n");
		delete this;
		return;
	    }
	    if ( pBody != 0L )
		free( pBody );
	    pBody = (char*)malloc( bodyLen + 1 );
	}
	else if ( cHeader + n == 10 )
	{
	    debugT("ERROR: Too long header\n");
	    delete this;
	    return;
	}
	else
	{
	    if ( !isdigit( headerBuffer[ cHeader ] ) )
	    {
		debugT("ERROR: Header must be an int\n");
		delete this;
		return;
	    }

	    cHeader += n;
	    return;
	}
    }
	
    int n;
    n = read( data_sock->socket(), pBody + cBody, bodyLen - cBody );
    if ( n + cBody == bodyLen )
    {
	pBody[bodyLen] = 0;
	debugT(">>'%s'\n",pBody);
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
	if ( strcmp( name, "exec" ) == 0 ) { parse_exec( _data, _len ); } else
	if ( strcmp( name, "copy" ) == 0 ) { parse_copy( _data, _len ); } else
	if ( strcmp( name, "move" ) == 0 ) { parse_move( _data, _len ); } else
	if ( strcmp( name, "moveClient" ) == 0 ) { parse_moveClient( _data, _len ); } else
	if ( strcmp( name, "ask" ) == 0 ) { parse_ask( _data, _len ); } else
	if ( strcmp( name, "sortDesktop" ) == 0 ) { parse_sortDesktop( _data, _len ); } else
	if ( strcmp( name, "auth" ) == 0 ) { parse_auth( _data, _len ); } else
		return;
	free_string( name );
}

#include "kfmserver_ipc.moc"
