/****************************************
 *
 *   main.cpp  - The KPager main
 *   Copyright (C) 1998  Antonio Larrosa Jimenez
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Send comments and bug fixes to antlarr@arrakis.es
 *   or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain
 *
 */

#include "kpager.h"
#include <stdlib.h>
#include <string.h>
#include <qwidget.h>
#include <unistd.h>

#ifdef HAVE_LIBJPEG
#include <qimage.h>
#include "jpeg.h"
#endif

    
int main(int argc, char **argv)
{
    KWMModuleApplication *app=new KWMModuleApplication(argc,argv);

    while (!KWM::isKWMInitialized()) sleep(1);

    if (app==NULL) 
	{
	   return 1;
	}

    app->enableSessionManagement(TRUE);

#ifdef HAVE_LIBJPEG
    QImageIO::defineIOHandler("JFIF","^\377\330\377\340", 0, read_jpeg_jfif, NULL);
#endif

    KPager *kpager=new KPager(app,"KPager");

    if (kpager==NULL) 
    {
	   delete app;
	   return 1;
    }

    app->setMainWidget ( kpager );


    if (app->isRestored())
    {
        if (kpager->canBeRestored(1)) kpager->restore(1);
    }
    
    kpager->show();

    return app->exec();
};
