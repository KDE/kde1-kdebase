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

void kForceLocker()
{
	char *p, buffer[256];

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
		// no screen saver - start a temporary saver
		p = getenv( "KDEDIR" );

		if ( p )
		{
			strcpy( buffer, p );
			strcat( buffer, "/bin/kblankscrn.kss" );

			if ( fork() == 0 )
			{
				execlp( buffer, buffer, "-test", "-lock", 0 );
				
				// if we make it here then try again using default path
				execlp("kblankscrn.kss","kblankscrn.kss","-test","-lock",0);

				// uh oh - failed
				fprintf( stderr, "Could not invoke kblankscrn.kss in $PATH or"
					" $KDEDIR/bin\n" );
				exit( 1 );
			}
		}
	}
}

