/*
  main.cpp - The KDE control center

  written 1997 by Matthias Hoelzer
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */


#include <qapp.h>
#include <qpushbt.h>
#include <qstring.h>
#include <kmsgbox.h>

#include "configlist.h"
#include "toplevel.h"

#include <kapp.h>
#include <kwmmapp.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


// run the control center application

int standalone(ConfigList *cl)
{
  TopLevel *toplevel;
  
  toplevel = new TopLevel(cl);
  toplevel->setCaption(klocale->translate("KDE Control Center"));
  
  kapp->setMainWidget(toplevel);
  
  return kapp->exec();
}


// The main programm

int main(int argc, char **argv)
{
  KWMModuleApplication app(argc, argv, "kcontrol");
  ConfigList configList;

  app.connectToKWM();
  
  if (argc >= 2)
    {
      // init mode
      if (strcmp(argv[1], "-init") == 0)
	{
	  configList.doInit();
	  return 0;
	}
      
      // setup mode (for compatibility with kdisplay)
      if (strcmp(argv[1], "-setup") == 0)
	return standalone(&configList);
    }
  
  // run the control center
  return standalone(&configList);
}
