//----------------------------------------------------------------------------
// Very basic CGI server for kdehelp
//
// This module implements only a fraction of the functionality required
// by a CGI server.  It is really only meant for help searches currently.
//
// Only a fraction of expected env. variables set
// Lots of other shortcomings
//
// Copyright (c) Martin R. Jones 1997


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <kurl.h>
#include "cgi.h"

#include "cgi.moc"


KCGI::KCGI()
{
	query = "";
	script = "";
	pathInfo = "";

	scriptPID = 0;

	connect( &timer, SIGNAL( timeout() ), SLOT( checkScript() ) );
}

bool KCGI::get( const char *_url, const char *_dest, const char *_method )
{
	method = _method;
	method = method.upper();

	QString u = _url;

	// extract query
	int qPos = u.find( '?' );

	if ( qPos > 0 )
		query = u.right( u.length() - qPos - 1 );

	// extract script
	int scriptPos = u.find( "/cgi-bin" );

	if ( scriptPos < 0 )
		return false;

	if ( qPos > 0 )
		script = u.mid( scriptPos, qPos - scriptPos );
	else
		script = u.right( u.length() - scriptPos );

	// extract path info
	int pathPos = script.find( '/', 9 );

	if ( pathPos >= 0 )
	{
		pathInfo = script.right( script.length() - pathPos - 1 );
		script.truncate( pathPos );
	}

	printf( "Script: %s\n", script.data() );
	printf( "Query: %s\n", query.data() );
	printf( "Path Info: %s\n", pathInfo.data() );

	KURL url( _dest );

	if ( url.isMalformed() )
	{
		printf( "CGI: Destination URL malformed: %s\n", _dest );
		return false;
	}

	destFile = url.path();

	return runScript();
}

bool KCGI::runScript()
{
	char *kdePath = getenv( "KDEDIR" );

	if ( !kdePath )
	{
		printf( "KDEDIR not set\n" );
		return false;
	}

	QString command = kdePath + script;
	command += " > " + destFile;

	if ( ( scriptPID = fork() ) == 0 )
	{
		QString tmp;

		if ( method == "GET" )
			setenv( "QUERY_STRING", query.data(), TRUE );
		setenv( "PATH_INFO", pathInfo.data(), TRUE );

		printf( "Running: %s\n", command.data() );

		FILE *fp = popen( command, "w" );

		if ( fp == NULL )
		{
			fp = fopen( destFile, "w" );
			if ( fp )
			{
				fprintf( fp, "<HTML><HEAD><TITLE>Error 404</TITLE></HEAD>" );
				fprintf( fp, "<BODY><h2>Error 404</h2>" );
				fprintf( fp, "URL not found</BODY></HTML>" );
				fclose( fp );
			}
		}
		else
		{
			if ( method == "POST" )
				fputs( query, fp );
			pclose( fp );
		}

		exit(0);
	}

	timer.start( 0 );

	return true;
}

void KCGI::checkScript()
{
	int status;

	if ( waitpid( scriptPID, &status, WNOHANG ) > 0 )
	{
		timer.stop();
		scriptPID = 0;
		emit finished();
	}
}

KCGI::~KCGI()
{
	if ( scriptPID )
		kill( scriptPID, SIGKILL );
}

