//-----------------------------------------------------------------------------
//
// KDE Help main
//
// (c) Martin R. Jones 1996
//

#include <signal.h>
#include <sys/stat.h>
#include <kapp.h>
#include "khelp.h"

static int msgqid = -1;


void kHelp::openWindow( const char *url )
{
	QString pidFile = getenv( "HOME" );
	pidFile += "/.kde/kdehelp/kdehelp.pid";

	// if there is a pidFile then there is an instance of kdehelp running
	if ( ( fp = fopen( pidFile, "r" ) ) != NULL )
	{
		printf( "found PID file\n" );
		KHelpMsg buf;
		int pid;
		buf.setType( 1L );
		buf.setMsg( url );
		fscanf( fp, "%d %d", &pid, &msgqid );
		// if this fails I assume that the pid file is left over from bad exit
		// and continue on
		//
		if ( buf.send( msgqid ) != -1)
		{
			// if we don't receive a reply within 5secs assume previous
			// instance of kdehelp died an unatural death.
			// How should this stuff be handled properly? File locking?
			//
			time_t start = time(NULL);
			while ( time(NULL) - start < 5 )
			{
				if (buf.recv( msgqid, 2L, IPC_NOWAIT ) != -1)
				{
					return;
				}
			}
		}
		fclose( fp );
	}

	if ( fork() == 0 )
	{
		execlp( "kdehelp", "kdehelp", url, 0 );
		exit(1);
	}
}

