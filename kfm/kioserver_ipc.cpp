// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include "kioserver_ipc.h"

KIOSlaveIPCServer::KIOSlaveIPCServer()
{
    serv_sock = new KServerSocket( 0 ); /* 0: choose free port */
    if ( serv_sock->socket() < 0 )
    {
	printf("ERROR: Could not establish server\n");
	exit(1);
    }
    printf( "SOCK=%i\n",serv_sock->getPort());
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

int KIOSlaveIPCServer::getPort()
{
    return serv_sock->getPort();
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
    data_sock->enableRead( FALSE );
    delete data_sock;
    if ( pBody != 0L )
	free( pBody );
}

void KIOSlaveIPC::closeEvent( KSocket *_sock )
{
    delete this;
    return;
}

void KIOSlaveIPC::readEvent( KSocket *_sock )
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
		printf("ERROR: Invalid header\n");
		delete this;
		return;
	    }
	    if ( pBody != 0L )
		free( pBody );
	    pBody = (char*)malloc( bodyLen + 1 );
	}
	else if ( cHeader + n == 10 )
	{
	    printf("ERROR: Too long header\n");
	    delete this;
	    return;
	}
	else
	{
	    if ( !isdigit( headerBuffer[ cHeader ] ) )
	    {
		printf("ERROR: Header must be an int\n");
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
	printf(">>'%s'\n",pBody);
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
	if ( strcmp( name, "dirEntry" ) == 0 ) { parse_dirEntry( _data, _len ); } else
	if ( strcmp( name, "flushDir" ) == 0 ) { parse_flushDir( _data, _len ); } else
	if ( strcmp( name, "done" ) == 0 ) { parse_done( _data, _len ); } else
	if ( strcmp( name, "fatalError" ) == 0 ) { parse_fatalError( _data, _len ); } else
	if ( strcmp( name, "setPID" ) == 0 ) { parse_setPID( _data, _len ); } else
		return;
	free_string( name );
}

#include "kioserver_ipc.moc"
