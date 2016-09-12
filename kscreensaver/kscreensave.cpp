//-----------------------------------------------------------------------------
// $Id: kscreensave.cpp,v 1.10 1998/09/09 09:47:02 jones Exp $
// KDE screen saver
//-----------------------------------------------------------------------------

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <kapp.h>

#include "kscreensave.h"

int sendSignal()
{
	QString buffer(getenv("HOME"));

	buffer.append("/.kss-install.pid.");
        char ksshostname[200];
        gethostname(ksshostname, 200);
        buffer.append(ksshostname);
	int r = -1;
	FILE *fp;

	if ( (fp = fopen( buffer, "r" ) ) != NULL )
	{
		// a screen saver is running - just tell it to lock
		int pid;

		fscanf( fp, "%d", &pid );
		fclose( fp );

                // But only kill it if the pid isn't -1!
                if (pid > 0) {
                  if( kill( pid, SIGUSR1 ) == 0 )
			  r = 0;
		}
	}

	return r;
}

void kForceLocker()
{
	if( sendSignal() != 0 )
	{
		KConfig *kdisplayConfig = new KConfig( kapp->kde_configdir() + "/kdisplayrc",
		                                       kapp->localconfigdir() + "/kdisplayrc" );
		kdisplayConfig->setGroup("ScreenSaver");
		bool allowRoot = kdisplayConfig->readBoolEntry( "allowRoot", false );
		delete kdisplayConfig;
		char *root = "-allow-root";
		if( !allowRoot )
			root = 0;

		// either no saver is running or an old pidFile was not removed
		QString buffer = QString(KApplication::kde_bindir().data());
		buffer.append("/kblankscrn.kss");
	    
		if ( fork() == 0 )
		{
			execlp( buffer, buffer, "-install", "-delay", "0", "-lock", root, 0 );

			// if we make it here then try again using default path
			execlp("kblankscrn.kss","kblankscrn.kss", "-install", "-delay", "0", "-lock", root, 0);
		    
			// uh oh - failed
			fprintf( stderr, "Could not invoke kblankscrn.kss in $PATH or"
			         " %s/bin\n" , KApplication::kde_bindir().data());
			exit (1);
		}
	}
}
