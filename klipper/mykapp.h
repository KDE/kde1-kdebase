#ifndef MYKAPP_H
#define MYKAPP_H

#include <kapp.h>
#include <kglobalaccel.h>

class MyKApplication : public KApplication
{
public:

  MyKApplication( int &argc, char **argv, char *name ) :
    KApplication( argc, argv, name ) { accel = 0L; }
  ~MyKApplication() {}

  void setGlobalKeys( KGlobalAccel *a ) { accel = a; }

private:

  KGlobalAccel *accel;

  inline bool MyKApplication::x11EventFilter( XEvent *e )
  {
    if ( accel )
      {
	if ( accel->x11EventFilter( e ) )
	  return true;
      }

    return KApplication::x11EventFilter( e );
  }

};

#endif
