/* -------------------------------------------------------------

   Klipper - Cut & paste history for KDE

   (C) by Andrew Stanley-Jones

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */


#include <mykapp.h>
#include <ktopwidget.h>
#include <kwm.h>

#include "toplevel.h"


int main(int argc, char *argv[])
{
  MyKApplication app(argc, argv, "klipper");

  TopLevel *toplevel=0;

  if (app.isRestored())
      RESTORE(TopLevel)
  else {
      // no session management: just create one window
      toplevel = new TopLevel();
  }

  // the widget handling the global keys
  app.setGlobalKeyWidget( toplevel );
  
  KWM::setDockWindow(toplevel->winId());
  toplevel->show();


  return app.exec();
}
