#ifndef MYKAPP_H
#define MYKAPP_H

#include <kapp.h>

#include "toplevel.h"

class MyKApplication : public KApplication
{
public:

  MyKApplication( int &argc, char **argv, char *name ) :
    KApplication( argc, argv, name ) { widget = 0L; }
  ~MyKApplication() {}

  void setGlobalKeyWidget( QWidget *w ) { widget = (TopLevel *) w; }

private:

  TopLevel *widget;

  inline bool MyKApplication::x11EventFilter( XEvent *e )
  {
    if ( widget )
      {
	if ( widget->globalKeys->x11EventFilter( e ) )
	  return true;
      }

    return KApplication::x11EventFilter( e );
  }


};

#endif
