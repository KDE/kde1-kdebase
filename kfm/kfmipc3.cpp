// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include "kfmipc.h"

void ipcKFM::parse_refreshDesktop( char *_data, int _len )
{
	int pos = 0;

	// Calling function
	refreshDesktop(  );
}

void ipcKFM::parse_refreshDirectory( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	refreshDirectory( _url );
}

void ipcKFM::parse_openURL( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	openURL( _url );
}

void ipcKFM::parse_openProperties( char *_data, int _len )
{
	int pos = 0;

	// Parsing string
	char* _url;
	_url = read_string( _data, pos, _len );
	// Calling function
	openProperties( _url );
}

