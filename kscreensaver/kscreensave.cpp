//-----------------------------------------------------------------------------
// $Id$
// KDE screen saver
//-----------------------------------------------------------------------------

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <kapp.h>

#include "kscreensave.h"

void kForceLocker()
{
	QString buffer(getenv("HOME"));

	buffer.append("/.kss.pid.");
        char ksshostname[200];
        gethostname(ksshostname, 200);
        buffer.append(ksshostname);

	FILE *fp;

	if ( (fp = fopen( buffer, "r" ) ) != NULL )
	{
		// a screen saver is running - just tell it to lock
		int pid;

		fscanf( fp, "%d", &pid );
		fclose( fp );

                // But only kill it if the pid isn't -1!
                if (pid > 0)
                  kill( pid, SIGUSR1 );
	}
	else
	{
            buffer = QString(KApplication::kde_bindir().data());
	    buffer.append("/kblankscrn.kss");
	    
	    if ( fork() == 0 )
		{
		    execlp( buffer, buffer, "-test", "-lock", 0 );

                    // if we make it here then try again using default path
		    execlp("kblankscrn.kss","kblankscrn.kss","-test","-lock",0);
		    
                    // uh oh - failed
		    fprintf( stderr, "Could not invoke kblankscrn.kss in $PATH or"
                             " %s/bin\n" , KApplication::kde_bindir().data());
		    exit (1);
		}
	}
}
