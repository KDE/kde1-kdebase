// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include "ipc.h"
#include "kfmclient_ipc.h"
#include <config-kfm.h>

KfmIpc::KfmIpc( int _port )
{
    bHeader = true;
    cHeader = 0;
    pBody = 0L;

    port = _port;
    sock = new KSocket( "localhost", port );
    connect( sock, SIGNAL( readEvent(KSocket*) ), this, SLOT( readEvent(KSocket*) ) );
    connect( sock, SIGNAL( closeEvent(KSocket*) ), this, SLOT( closeEvent(KSocket*) ) );
    sock->enableRead( true );
    connected = true;
}

KfmIpc::~KfmIpc()
{
    delete sock;
}

bool KfmIpc::isConnected()
{
    return connected;
}

void KfmIpc::closeEvent( KSocket * )
{
    connected = false;
}

void KfmIpc::readEvent( KSocket * )
{
    if ( bHeader )
    {
	int n;
	n = read( sock->socket(), headerBuffer + cHeader, 1 );
	if ( headerBuffer[ cHeader ] == ' ' )
	{
	    bHeader = false;
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
    n = read( sock->socket(), pBody + cBody, bodyLen - cBody );
    if ( n + cBody == bodyLen )
    {
	pBody[bodyLen] = 0;
	debugT(">>'%s'\n",pBody);
	bHeader = true;
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

	if ( strcmp( name, "finished" ) == 0 ) { parse_finished( _data, _len ); } else
    { debugT("Unknown command '%s'\n",name); }
}


#include "kfmclient_ipc.moc"
