// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include <kdebug.h>

#include "ipc.h"
#include "kioslave_ipc.h"

KIOSlaveIPC::KIOSlaveIPC( char * _path )
{
    bHeader = TRUE;
    cHeader = 0;
    pBody = 0L;

    sock = new KSocket( _path );
    connect( sock, SIGNAL( readEvent(KSocket*) ), this, SLOT( readEvent(KSocket*) ) );
    connect( sock, SIGNAL( closeEvent(KSocket*) ), this, SLOT( closeEvent(KSocket*) ) );
    sock->enableRead( TRUE );
    connected = TRUE;
}

KIOSlaveIPC::~KIOSlaveIPC()
{
    delete sock;
}

bool KIOSlaveIPC::isConnected()
{
    return connected;
}

void KIOSlaveIPC::closeEvent( KSocket * )
{
    //kdebug(KDEBUG_INFO, 0, "******** EXIT ********\n");
    exit(1);
    connected = FALSE;
}

void KIOSlaveIPC::readEvent( KSocket * )
{
    if ( bHeader )
    {
	int n;
	n = read( sock->socket(), headerBuffer + cHeader, 1 );
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
    n = read( sock->socket(), pBody + cBody, bodyLen - cBody );
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

	if ( strcmp( name, "mount" ) == 0 ) { parse_mount( _data, _len ); } else
	if ( strcmp( name, "unmount" ) == 0 ) { parse_unmount( _data, _len ); } else
	if ( strcmp( name, "copy" ) == 0 ) { parse_copy( _data, _len ); } else
	if ( strcmp( name, "get" ) == 0 ) { parse_get( _data, _len ); } else
	if ( strcmp( name, "del" ) == 0 ) { parse_del( _data, _len ); } else
	if ( strcmp( name, "mkdir" ) == 0 ) { parse_mkdir( _data, _len ); } else
	if ( strcmp( name, "list" ) == 0 ) { parse_list( _data, _len ); } else
	if ( strcmp( name, "getPID" ) == 0 ) { parse_getPID( _data, _len ); } else
	if ( strcmp( name, "cleanUp" ) == 0 ) { parse_cleanUp( _data, _len ); } else
    { printf("Unknown command '%s'\n",name); }
   free(name);
}


#include "kioslave_ipc.moc"
