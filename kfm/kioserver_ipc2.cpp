// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include "kioserver_ipc.h"

void KIOSlaveIPC::parse_hello( char *, int )
{
	// int pos = 0;

	// Calling function
	emit hello(  );
}

void KIOSlaveIPC::parse_progress( char *_data, int _len )
{
	int pos = 0;

	// Parsing int
	int _percent;
	_percent = read_int( _data, pos, _len );
	// Calling function
	emit progress( _percent );
}

void KIOSlaveIPC::parse_info( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _text;
	_text = read_string( _data, pos, _len );
	// Calling function
	emit info( _text );
	free( (void*)_text );
}

void KIOSlaveIPC::parse_dirEntry( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _dir;
	_dir = read_string( _data, pos, _len );
	// Parsing string
	const char* _name;
	_name = read_string( _data, pos, _len );
	// Parsing bool
	bool _isDir;
	_isDir = read_bool( _data, pos, _len );
	// Parsing int
	int _size;
	_size = read_int( _data, pos, _len );
	// Parsing string
	const char* _date;
	_date = read_string( _data, pos, _len );
	// Parsing string
	const char* _access;
	_access = read_string( _data, pos, _len );
	// Parsing string
	const char* _owner;
	_owner = read_string( _data, pos, _len );
	// Parsing string
	const char* _group;
	_group = read_string( _data, pos, _len );
	// Calling function
	emit dirEntry( _dir, _name, _isDir, _size, _date, _access, _owner, _group );
	free( (void*)_dir );
	free( (void*)_name );
	free( (void*)_date );
	free( (void*)_access );
	free( (void*)_owner );
	free( (void*)_group );
}

void KIOSlaveIPC::parse_data( char *_data, int _len )
{
	int pos = 0;

	// Parsing mem
	IPCMemory _text;
	_text = read_mem( _data, pos, _len );
	// Calling function
	emit data( _text );
	free( (void*)_text.data );
}

void KIOSlaveIPC::parse_flushDir( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _dir;
	_dir = read_string( _data, pos, _len );
	// Calling function
	emit flushDir( _dir );
	free( (void*)_dir );
}

void KIOSlaveIPC::parse_done( char *, int )
{
	// int pos = 0;

	// Calling function
	emit done(  );
}

void KIOSlaveIPC::parse_fatalError( char *_data, int _len )
{
	int pos = 0;

	// Parsing int
	int _error;
	_error = read_int( _data, pos, _len );
	// Parsing string
	const char* _text;
	_text = read_string( _data, pos, _len );
	// Parsing int
	int _errno;
	_errno = read_int( _data, pos, _len );
	// Calling function
	emit fatalError( _error, _text, _errno );
	free( (void*)_text );
}

void KIOSlaveIPC::parse_setPID( char *_data, int _len )
{
	int pos = 0;

	// Parsing int
	int _pid;
	_pid = read_int( _data, pos, _len );
	// Calling function
	emit setPID( _pid );
}

void KIOSlaveIPC::parse_redirection( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	emit redirection( _url );
	free( (void*)_url );
}

void KIOSlaveIPC::parse_mimeType( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	const char* _type;
	_type = read_string( _data, pos, _len );
	// Calling function
	emit mimeType( _type );
	free( (void*)_type );
}

void KIOSlaveIPC::mount(bool _ro, const char* _fstype, const char* _dev, const char* _point)
{
	int len = 0;
	len += len_bool( _ro );
	len += len_string( _fstype );
	len += len_string( _dev );
	len += len_string( _point );
	len += len_string("mount");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "mount" );
	write_bool( data_sock->socket(), _ro );
	write_string( data_sock->socket(), _fstype );
	write_string( data_sock->socket(), _dev );
	write_string( data_sock->socket(), _point );
}

void KIOSlaveIPC::unmount(const char* _point)
{
	int len = 0;
	len += len_string( _point );
	len += len_string("unmount");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "unmount" );
	write_string( data_sock->socket(), _point );
}

void KIOSlaveIPC::copy(const char* _from_url, const char* _to_url, bool _overwrite)
{
	int len = 0;
	len += len_string( _from_url );
	len += len_string( _to_url );
	len += len_bool( _overwrite );
	len += len_string("copy");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "copy" );
	write_string( data_sock->socket(), _from_url );
	write_string( data_sock->socket(), _to_url );
	write_bool( data_sock->socket(), _overwrite );
}

void KIOSlaveIPC::get(const char* _url, const char* _data)
{
        if (!_data)
        {
            _data = "";
        }

	int len = 0;
	len += len_string( _url );
	len += len_string( _data );
	len += len_string("get");

	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "get" );
	write_string( data_sock->socket(), _url );
        write_string( data_sock->socket(), _data );
}

void KIOSlaveIPC::reload(const char* _url)
{
	int len = 0;
	len += len_string( _url );
	len += len_string("reload");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "reload" );
	write_string( data_sock->socket(), _url );
}

void KIOSlaveIPC::del(const char* _url)
{
	int len = 0;
	len += len_string( _url );
	len += len_string("del");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "del" );
	write_string( data_sock->socket(), _url );
}

void KIOSlaveIPC::mkdir(const char* _url)
{
	int len = 0;
	len += len_string( _url );
	len += len_string("mkdir");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "mkdir" );
	write_string( data_sock->socket(), _url );
}

void KIOSlaveIPC::list(const char* _url, bool _bHTML)
{
	int len = 0;
	len += len_string( _url );
	len += len_bool( _bHTML );
	len += len_string("list");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "list" );
	write_string( data_sock->socket(), _url );
	write_bool( data_sock->socket(), _bHTML );
}

void KIOSlaveIPC::getPID()
{
	int len = 0;
	len += len_string("getPID");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "getPID" );
}

void KIOSlaveIPC::cleanUp()
{
	int len = 0;
	len += len_string("cleanUp");
	write_int( data_sock->socket(), len );
	write_string( data_sock->socket(), "cleanUp" );
}

