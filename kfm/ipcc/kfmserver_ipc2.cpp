// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include "kfmserver_ipc.h"

void KfmIpc::parse_refreshDesktop( char *_data, int _len )
{
	int pos = 0;

	// Calling function
	emit refreshDesktop(  );
}

void KfmIpc::parse_refreshDirectory( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit refreshDirectory( _url );
}

void KfmIpc::parse_openURL( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit openURL( _url );
}

void KfmIpc::parse_openProperties( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit openProperties( _url );
}

void KfmIpc::parse_exec( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Parsing string
	const char* _binding;
	_binding = read_string( _data, pos, _len );
	// Calling function
	emit exec( _url, _binding );
}

void KfmIpc::parse_copy( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _src;
	_src = read_string( _data, pos, _len );
	// Parsing string
	const char* _dest;
	_dest = read_string( _data, pos, _len );
	// Calling function
	emit copy( _src, _dest );
}

void KfmIpc::parse_move( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _src;
	_src = read_string( _data, pos, _len );
	// Parsing string
	const char* _dest;
	_dest = read_string( _data, pos, _len );
	// Calling function
	emit move( _src, _dest );
}

void KfmIpc::parse_moveClient( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _src;
	_src = read_string( _data, pos, _len );
	// Parsing string
	const char* _dest;
	_dest = read_string( _data, pos, _len );
	// Calling function
	emit moveClient( _src, _dest );
}

void KfmIpc::parse_ask( char *_data, int _len )
{
	int pos = 0;

	// Parsing int
	int _x;
	_x = read_int( _data, pos, _len );
	// Parsing int
	int _y;
	_y = read_int( _data, pos, _len );
	// Parsing string
	const char* _src;
	_src = read_string( _data, pos, _len );
	// Parsing string
	const char* _dest;
	_dest = read_string( _data, pos, _len );
	// Calling function
	emit ask( _x, _y, _src, _dest );
}

void KfmIpc::parse_sortDesktop( char *_data, int _len )
{
	int pos = 0;

	// Calling function
	emit sortDesktop(  );
}

void KfmIpc::finished()
{
	int len = 0;
	len += len_string("finished");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "finished" );
}

