// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de
#include "kioslave_ipc.h"

void KIOSlaveIPC::hello()
{
	int len = 0;
	len += len_string("hello");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "hello" );
}

void KIOSlaveIPC::progress(int _percent)
{
	int len = 0;
	len += len_int( _percent );
	len += len_string("progress");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "progress" );
	write_int( sock->socket(), _percent );
}

void KIOSlaveIPC::info(const char* _text)
{
	int len = 0;
	len += len_string( _text );
	len += len_string("info");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "info" );
	write_string( sock->socket(), _text );
}

void KIOSlaveIPC::dirEntry(const char* _dir, const char* _name, bool _isDir, int _size, const char* _date, const char* _access, const char* _owner, const char* _group)
{
	int len = 0;
	len += len_string( _dir );
	len += len_string( _name );
	len += len_bool( _isDir );
	len += len_int( _size );
	len += len_string( _date );
	len += len_string( _access );
	len += len_string( _owner );
	len += len_string( _group );
	len += len_string("dirEntry");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "dirEntry" );
	write_string( sock->socket(), _dir );
	write_string( sock->socket(), _name );
	write_bool( sock->socket(), _isDir );
	write_int( sock->socket(), _size );
	write_string( sock->socket(), _date );
	write_string( sock->socket(), _access );
	write_string( sock->socket(), _owner );
	write_string( sock->socket(), _group );
}

void KIOSlaveIPC::data(IPCMemory _text)
{
	int len = 0;
	len += len_mem( _text );
	len += len_string("data");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "data" );
	write_mem( sock->socket(), _text );
}

void KIOSlaveIPC::flushDir(const char* _dir)
{
	int len = 0;
	len += len_string( _dir );
	len += len_string("flushDir");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "flushDir" );
	write_string( sock->socket(), _dir );
}

void KIOSlaveIPC::done()
{
	int len = 0;
	len += len_string("done");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "done" );
}

void KIOSlaveIPC::fatalError(int _error, const char* _text, int _errno)
{
	int len = 0;
	len += len_int( _error );
	len += len_string( _text );
	len += len_int( _errno );
	len += len_string("fatalError");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "fatalError" );
	write_int( sock->socket(), _error );
	write_string( sock->socket(), _text );
	write_int( sock->socket(), _errno );
}

void KIOSlaveIPC::setPID(int _pid)
{
	int len = 0;
	len += len_int( _pid );
	len += len_string("setPID");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "setPID" );
	write_int( sock->socket(), _pid );
}

void KIOSlaveIPC::redirection(const char* _url)
{
	int len = 0;
	len += len_string( _url );
	len += len_string("redirection");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "redirection" );
	write_string( sock->socket(), _url );
}

void KIOSlaveIPC::mimeType(const char* _type)
{
	int len = 0;
	len += len_string( _type );
	len += len_string("mimeType");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "mimeType" );
	write_string( sock->socket(), _type );
}

void KIOSlaveIPC::parse_mount( char *_data, int _len )
{
	int pos = 0;

	// Parsing bool
	bool _ro;
	_ro = read_bool( _data, pos, _len );
	// Parsing string
	const char* _fstype;
	_fstype = read_string( _data, pos, _len );
	// Parsing string
	const char* _dev;
	_dev = read_string( _data, pos, _len );
	// Parsing string
	const char* _point;
	_point = read_string( _data, pos, _len );
	// Calling function
	emit mount( _ro, _fstype, _dev, _point );
	free( (void*)_fstype );
	free( (void*)_dev );
	free( (void*)_point );
}

void KIOSlaveIPC::parse_unmount( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _point;
	_point = read_string( _data, pos, _len );
	// Calling function
	emit unmount( _point );
	free( (void*)_point );
}

void KIOSlaveIPC::parse_copy( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _from_url;
	_from_url = read_string( _data, pos, _len );
	// Parsing string
	const char* _to_url;
	_to_url = read_string( _data, pos, _len );
	// Parsing bool
	bool _overwrite;
	_overwrite = read_bool( _data, pos, _len );
	// Calling function
	emit copy( _from_url, _to_url, _overwrite );
	free( (void*)_from_url );
	free( (void*)_to_url );
}

void KIOSlaveIPC::parse_get( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit get( _url );
	free( (void*)_url );
}

void KIOSlaveIPC::parse_reload( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit reload( _url );
	free( (void*)_url );
}

void KIOSlaveIPC::parse_del( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit del( _url );
	free( (void*)_url );
}

void KIOSlaveIPC::parse_mkdir( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit mkdir( _url );
	free( (void*)_url );
}

void KIOSlaveIPC::parse_list( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Parsing bool
	bool _bHTML;
	_bHTML = read_bool( _data, pos, _len );
	// Calling function
	emit list( _url, _bHTML );
	free( (void*)_url );
}

void KIOSlaveIPC::parse_getPID( char *, int  )
{
	// int pos = 0;

	// Calling function
	emit getPID(  );
}

void KIOSlaveIPC::parse_cleanUp( char *, int )
{
	// int pos = 0;

	// Calling function
	emit cleanUp(  );
}

