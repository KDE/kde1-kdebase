/*
 * main.cpp. Part of the KDE project.
 *
 * Copyright (C) 1997 Martin Jones
 *
 */

#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <qimage.h>
#include <qwidcoll.h>
#include <kwmmapp.h>
#include <kimgio.h>

#include "kbgndwm.h"
#include "version.h"

#include <X11/Xlib.h>

int main( int argc, char *argv[] )
{
    KWMModuleApplication a (argc, argv);

    kimgioRegister();

    if ( argc > 1 )
    {
	if ( QString("-version") == argv[1] )
	{
	    printf( KDISPLAYWM_VERSION );
	    printf("\n");
	    printf("Copyright (C) 1997 Martin Jones (mjones@kde.org)\n");
	    printf("              1998 Matej  Koss  (koss@napri.sk)\n");
	    ::exit(0);
	}
	else
	{
	    printf("Usage:");
	    printf("%s [-version]\n", argv[0]);
	}
	::exit(1); 
    }

    // Initialize random generator
    time_t now = time(NULL);
    srandom((unsigned int)now);

    KBGndManager kbgnd( &a );

    kbgnd.connect(&a, SIGNAL(desktopChange(int)), SLOT(desktopChange(int)));
    kbgnd.connect(&a, SIGNAL(commandReceived(QString)), SLOT(commandReceived(QString)));

    a.connectToKWM();
    return a.exec();
}

//----------------------------------------------------------------------------

