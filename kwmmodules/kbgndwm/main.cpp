/*
 * main.cpp. Part of the KDE project.
 *
 * Copyright (C) 1997 Martin Jones
 *
 */

#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <qimage.h>
#include <qwidcoll.h>
#include <kwmmapp.h>

#include "kbgndwm.h"
#include "version.h"

#include <X11/Xlib.h>

#ifdef HAVE_LIBGIF
#include "gif.h"
#endif

#ifdef HAVE_LIBJPEG
#include "jpeg.h"
#endif
 

int main( int argc, char *argv[] )
{
    KWMModuleApplication a (argc, argv);

    if ( argc > 1 )
    {
	if ( QString("-version") == argv[1] )
	{
	    printf( KDISPLAYWM_VERSION );
	    printf("\n");
	    printf("Copyright (C) 1997 Martin Jones (mjones@kde.org)\n");
	    ::exit(0);
	}
	else
	{
	    printf("Usage:");
	    printf("%s [-version]\n", argv[0]);
	}
	::exit(1); 
    }

#ifdef HAVE_LIBGIF
    QImageIO::defineIOHandler("GIF", "^GIF[0-9][0-9][a-z]", 0, read_gif_file, NULL);
#endif
#ifdef HAVE_LIBJPEG
    QImageIO::defineIOHandler("JFIF","^\377\330\377\340", 0, read_jpeg_jfif, NULL);
#endif

    KBGndManager kbgnd( &a );

    kbgnd.connect(&a, SIGNAL(desktopChange(int)), SLOT(desktopChange(int)));
    kbgnd.connect(&a, SIGNAL(commandReceived(int)), SLOT(commandReceived(int)));

    a.connectToKWM();

    return a.exec();
}

//----------------------------------------------------------------------------

