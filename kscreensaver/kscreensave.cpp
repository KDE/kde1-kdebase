//-----------------------------------------------------------------------------
//
// KDE screen saver
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "kscreensave.h"
#include <kapp.h>

void kForceLocker()
{
	char buffer[256];

	strcpy( buffer, getenv( "HOME" ) );
	strcat( buffer, "/.kss.pid" );

	FILE *fp;

	if ( (fp = fopen( buffer, "r" ) ) != NULL )
	{
		// a screen saver is running - just tell it to lock
		int pid;

		fscanf( fp, "%d", &pid );
		fclose( fp );
		kill( pid, SIGUSR1 );
	}
	else
	{
	    strcpy( buffer, KApplication::kde_bindir() );
	    strcat( buffer, "/kblankscrn.kss" );
	    
	    if ( fork() == 0 )
		{
		    execlp( buffer, buffer, "-test", "-lock", 0 );
		    
				// if we make it here then try again using default path
		    execlp("kblankscrn.kss","kblankscrn.kss","-test","-lock",0);
		    
				// uh oh - failed
		    fprintf( stderr, "Could not invoke kblankscrn.kss in $PATH or"
			     " %s/bin\n" , KApplication::kde_bindir().data());
		    exit( 1 );
		}
	}
}

