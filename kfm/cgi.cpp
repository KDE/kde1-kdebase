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
#include <kmisc.h>
#include "cgi.h"
#include <kapp.h>
#include <config-kfm.h>

int idCounter = 0;

KCGI::KCGI()
{
    query = "";
    script = "";
    pathInfo = "";
    
    scriptPID = 0;
    
    connect( &timer, SIGNAL( timeout() ), SLOT( checkScript() ) );
}

QString KCGI::get( const char *_url, const char *_method )
{
    destFile.sprintf( "%s/.kfm/tmp/cgi%i", getenv( "HOME" ), idCounter++ );
    
    debugT("'%s' Writing to '%s'\n",_url, destFile.data() );
    
    method = _method;
    method = method.upper();
    
    QString u = _url;

    // extract query
    int qPos = u.find( '?' );
    
    if ( qPos > 0 )
	query = u.right( u.length() - qPos - 1 );
    
    if ( qPos > 0 )
	script = u.left( qPos );
    else
	script = u.data();
    
    // extract path info
    int pathPos = script.find( '/', 9 );
    
    if ( pathPos >= 0 )
    {
	pathInfo = script.right( script.length() - pathPos - 1 );
	script.truncate( pathPos );
    }
    
    debugT( "Script: %s\n", script.data() );
    debugT( "Query: %s\n", query.data() );
    debugT( "Path Info: %s\n", pathInfo.data() );
    
    if ( runScript() )
	return QString( destFile.data() );
    else
	return QString();
}

bool KCGI::runScript()
{
    QString command = kapp->kdedir() + script;
    command += " > " + destFile;
    
    if ( ( scriptPID = fork() ) == 0 )
    {
	QString tmp;
	
	if ( method == "GET" )
	    setenv( "QUERY_STRING", query.data(), true );
	setenv( "PATH_INFO", pathInfo.data(), true );
	
	debugT( "Running: %s\n", command.data() );
	
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

    debugT("((((((((((((((((( CHECKING %i )))))))))))))))))\n", scriptPID);
 
    if ( waitpid( scriptPID, &status, WNOHANG ) > 0 || kill( scriptPID, 0 ) != 0 )
    {   
	// if ( waitpid( scriptPID, &status, WNOHANG ) > 0 )
	debugT("((((((((((((((((( CHECKED )))))))))))))))))\n");
	timer.stop();
	scriptPID = 0;
	emit finished();
    }
}

void KCGI::stop()
{
    if ( scriptPID )
	kill( scriptPID, SIGKILL );
}

KCGI::~KCGI()
{
    if ( scriptPID )
	kill( scriptPID, SIGKILL );
}

#include "cgi.moc"
