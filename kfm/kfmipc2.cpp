// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include "kfmipc.h"


#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void write_int( int _fd, int _value )
{
    char buffer[10];
    sprintf( buffer, "%i ", _value );
    write( _fd, (const char*)buffer, strlen(buffer) );
}

void write_double( int _fd, double _value )
{
    char buffer[10];
    sprintf( buffer, "%f ", _value );
    write( _fd, (const char*)buffer, strlen(buffer) );
}

void write_char( int _fd, char _value )
{
    write( _fd, (const char*)&_value, sizeof(char) );
}

void write_string( int _fd, char* _value )
{
    int len = strlen( _value );
    write_int( _fd, len );
    write( _fd, (const char*)_value, strlen(_value) );
}

void write_intList( int _fd, intList* _list )
{
    int len = _list->elements * sizeof(int);
    write( _fd, (const char*)&(_list->elements), sizeof(int) );
    write( _fd, (const char*)(_list->list), len );
}

void write_doubleList( int _fd, doubleList* _list )
{
    int len = _list->elements * sizeof(double);
    write( _fd, (const char*)&(_list->elements), sizeof(int) );
    write( _fd, (const char*)(_list->list), len );
}

void write_charList( int _fd, charList* _list )
{
    int len = _list->elements * sizeof(char);
    write( _fd, (const char*)&(_list->elements), sizeof(int) );
    write( _fd, (const char*)(_list->list), len );
}

void write_stringList( int _fd, stringList* _list )
{
    int len = _list->elements * sizeof(int);
    write( _fd, (const char*)&(_list->elements), sizeof(int) );
    for( int i = 0; i < _list->elements; i++ )
    {
	write_string( _fd, _list->list[i] );
    }
}

char* read_string( char *_data, int &_pos, int _len )
{
    int tmp = read_int( _data, _pos, _len );
    char *str = (char*)malloc(tmp + 1);
    strncpy( str, _data + _pos, tmp);
    _pos += tmp;
    str[tmp] = 0;
    return str;
}

int read_int( char *_data, int &_pos, int _len )
{
    int i = _pos;
    while ( _data[ _pos ] != ' ' )
    {
	_pos++;
	if ( _pos == _len )
	    return 0;
    }
    _pos++;
    
    return atoi( _data + i );
}

char read_char( char *_data, int &_pos, int _len )
{
    char tmp = *((char*)_data + _pos);
    _pos += sizeof(char);
    return tmp;
}

double read_double( char *_data, int &_pos, int _len )
{
    int i = _pos;
    while ( _data[ _pos ] != ' ' )
    {
	_pos++;
	if ( _pos == _len )
	    return 0;
    }
    _pos++;
    
    return atof( _data + i );
}

void read_stringList( char *_data, int &_pos, int _len, stringList *_list )
{
    int tmp = *((int*)_data + _pos);
    _list->elements = tmp;
    _pos += sizeof(int);
    _list->list = (char**)malloc( tmp * sizeof(char*) );
    for( int i = 0; i < tmp; i++ )
    {
	_list->list[i] = read_string( _data, _pos, _len );
    }
}

void read_intList( char *_data, int &_pos, int _len, intList *_list )
{
    int tmp = *((int*)_data + _pos);
    _list->elements = tmp;
    _pos += sizeof(int);
    _list->list = (int*)malloc( tmp * sizeof(int) );
    for( int i = 0; i < tmp; i++ )
    {
	_list->list[i] = read_int( _data, _pos, _len );
    }
}

void read_doubleList( char *_data, int &_pos, int _len, doubleList *_list )
{
    int tmp = *((int*)_data + _pos);
    _list->elements = tmp;
    _pos += sizeof(int);
    _list->list = (double*)malloc( tmp * sizeof(double) );
    for( int i = 0; i < tmp; i++ )
    {
	_list->list[i] = read_double( _data, _pos, _len );
    }
}

void read_charList( char *_data, int &_pos, int _len, charList *_list )
{
    int tmp = *((int*)_data + _pos);
    _list->elements = tmp;
    _pos += sizeof(int);
    _list->list = (char*)malloc( tmp * sizeof(char) );
    for( int i = 0; i < tmp; i++ )
    {
	_list->list[i] = read_char( _data, _pos, _len );
    }
}

void free_string( char *_str )
{
    free( _str );
}

void free_stringList( stringList *_list )
{
    for( int i = 0; i < _list->elements; i++ )
    {
	free ( _list->list[i] );
    }
    free ( _list->list );
}

void free_intList( intList *_list )
{
    free ( _list->list );
}

void free_doubleList( doubleList *_list )
{
    free ( _list->list );
}

void free_charList( charList *_list )
{
    free ( _list->list );
}

int len_int( int _value )
{
    char buffer[ 20 ];
    sprintf( buffer, "%i", _value );
    return strlen(buffer) + 1;
}

int len_double( double _value )
{
    char buffer[ 20 ];
    sprintf( buffer, "%f", _value );
    return strlen(buffer) + 1;
}

int len_char( char _value )
{
    return 1;
}

int len_string( char *_str )
{
    int len = strlen( _str );
    return len + len_int( len );
}

int len_stringList( stringList *_list )
{
    int len = sizeof(int);
    for( int i = 0; i < _list->elements; i++ )
    {
	len += len_string( _list->list[i] );
    }
    return len;
}

int len_intList( intList *_list )
{
    return ( sizeof(int) + _list->elements * sizeof(int) );
}

int len_doubleList( doubleList *_list )
{
    return ( sizeof(int) + _list->elements * sizeof(double) );
}

int len_charList( charList *_list )
{
    return ( sizeof(int) + _list->elements * sizeof(char) );
}

IPC::IPC()
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

void IPC::slotAccept( KSocket *_sock )
{
    new ipcKFM( _sock );
}

IPC::~IPC()
{
    delete serv_sock;
}

int IPC::getPort()
{
    return serv_sock->getPort();
}

ipcKFM::ipcKFM( KSocket *_sock )
{
    bHeader = TRUE;
    cHeader = 0;
    pBody = 0L;

    connect( _sock, SIGNAL( readEvent(KSocket*) ), this, SLOT( readEvent(KSocket*) ) );
    connect( _sock, SIGNAL( closeEvent(KSocket*) ), this, SLOT( closeEvent(KSocket*) ) );
    _sock->enableRead( TRUE );
    data_sock = _sock;
}

ipcKFM::~ipcKFM()
{
    data_sock->enableRead( FALSE );
    delete data_sock;
    if ( pBody != 0L )
	free( pBody );
}

void ipcKFM::closeEvent( KSocket *_sock )
{
    delete this;
    return;
}

void ipcKFM::readEvent( KSocket *_sock )
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

void ipcKFM::parse( char *_data, int _len )
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
		return;
	free_string( name );
}

#include "kfmipc.moc"
