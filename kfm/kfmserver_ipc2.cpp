// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include "kfmserver_ipc.h"

void KfmServIpc::parse_refreshDesktop( char *, int )
{
	// int pos = 0;

	// Calling function
	emit refreshDesktop(  );
}

void KfmServIpc::parse_refreshDirectory( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit refreshDirectory( _url );
	free( (void*)_url );
}

void KfmServIpc::parse_openURL( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit openURL( _url );
	free( (void*)_url );
}

void KfmServIpc::parse_openProperties( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit openProperties( _url );
	free( (void*)_url );
}

void KfmServIpc::parse_list( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit list( _url );
	free( (void*)_url );
}

void KfmServIpc::parse_exec( char *_data, int _len )
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
	free( (void*)_url );
	free( (void*)_binding );
}

void KfmServIpc::parse_copy( char *_data, int _len )
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
	free( (void*)_src );
	free( (void*)_dest );
}

void KfmServIpc::parse_move( char *_data, int _len )
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
	free( (void*)_src );
	free( (void*)_dest );
}

void KfmServIpc::parse_moveClient( char *_data, int _len )
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
	free( (void*)_src );
	free( (void*)_dest );
}

void KfmServIpc::parse_copyClient( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _src;
	_src = read_string( _data, pos, _len );
	// Parsing string
	const char* _dest;
	_dest = read_string( _data, pos, _len );
	// Calling function
	emit copyClient( _src, _dest );
	free( (void*)_src );
	free( (void*)_dest );
}

void KfmServIpc::parse_sortDesktop( char *, int )
{
	// int pos = 0;

	// Calling function
	emit sortDesktop(  );
}

void KfmServIpc::parse_configure( char *, int )
{
	// int pos = 0;

	// Calling function
	emit configure(  );
}

void KfmServIpc::parse_auth( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _password;
	_password = read_string( _data, pos, _len );
	// Calling function
	emit auth( _password );
	free( (void*)_password );
}

void KfmServIpc::parse_selectRootIcons( char *_data, int _len )
{
	int pos = 0;

	// Parsing int
	int _x;
	_x = read_int( _data, pos, _len );
	// Parsing int
	int _y;
	_y = read_int( _data, pos, _len );
	// Parsing int
	int _w;
	_w = read_int( _data, pos, _len );
	// Parsing int
	int _h;
	_h = read_int( _data, pos, _len );
	// Parsing bool
	bool _add;
	_add = read_bool( _data, pos, _len );
	// Calling function
	emit selectRootIcons( _x, _y, _w, _h, _add );
}

void KfmServIpc::finished()
{
	int len = 0;
	len += len_string("finished");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "finished" );
}

void KfmServIpc::error(int _kerror, const char* _text)
{
	int len = 0;
	len += len_int( _kerror );
	len += len_string( _text );
	len += len_string("error");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "error" );
	write_int( data_sock->socket(), _kerror );
	write_string( data_sock->socket(), _text );
}

void KfmServIpc::dirEntry(const char* _name, const char* _access, const char* _owner, const char* _group, const char* _date, int _size)
{
	int len = 0;
	len += len_string( _name );
	len += len_string( _access );
	len += len_string( _owner );
	len += len_string( _group );
	len += len_string( _date );
	len += len_int( _size );
	len += len_string("dirEntry");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "dirEntry" );
	write_string( data_sock->socket(), _name );
	write_string( data_sock->socket(), _access );
	write_string( data_sock->socket(), _owner );
	write_string( data_sock->socket(), _group );
	write_string( data_sock->socket(), _date );
	write_int( data_sock->socket(), _size );
}

