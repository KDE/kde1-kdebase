// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//

#include "kpanel.h"
#include <qapp.h>
#include <kwmmapp.h>
#include <qmsgbox.h>
#include <stdio.h>
#include <unistd.h>
#include "kpanel_version.h"
#include <X11/keysym.h>

kPanel *the_panel;
int o_argc;
char ** o_argv;


void restart_the_panel(){
  QApplication::exit();
  execvp(o_argv[0],  o_argv);
  exit(1);
}

#include "kpanel.moc"


class MyApp:public KWMModuleApplication {
public:
  MyApp( int &argc, char **argv, const QString& rAppName );
  virtual bool x11EventFilter( XEvent * );
};

MyApp::MyApp(int &argc, char **argv , const QString& rAppName):
  KWMModuleApplication(argc, argv, rAppName){
}

bool MyApp::x11EventFilter( XEvent * ev){
  if (ev->type == KeyPress){
    XKeyEvent* e = &ev->xkey;
    int kc = XKeycodeToKeysym(qt_xdisplay(), e->keycode, 0);
    int km = e->state & (ControlMask | Mod1Mask | ShiftMask);
    if( (kc == XK_F1)  && (km == Mod1Mask) ){
      the_panel->showSystem();
      XAllowEvents(qt_xdisplay(), AsyncKeyboard, CurrentTime);
      return TRUE;
    }
  }
  return KWMModuleApplication::x11EventFilter(ev);
}

static void grabKey(KeySym keysym, unsigned int mod){
  static int NumLockMask = 0;
  if (!NumLockMask){
    XModifierKeymap* xmk = XGetModifierMapping(qt_xdisplay());
    int i;
    for (i=0; i<8; i++){
      if (xmk->modifiermap[xmk->max_keypermod * i] == 
	  XKeysymToKeycode(qt_xdisplay(), XK_Num_Lock))
	NumLockMask = (1<<i); 
    }
  }
  XGrabKey(qt_xdisplay(),
	   XKeysymToKeycode(qt_xdisplay(), keysym), mod,
	   qt_xrootwin(), True,
	   GrabModeAsync, GrabModeSync);
  XGrabKey(qt_xdisplay(),
	   XKeysymToKeycode(qt_xdisplay(), keysym), mod | LockMask,
	   qt_xrootwin(), True,
	   GrabModeAsync, GrabModeSync);
  XGrabKey(qt_xdisplay(),
	   XKeysymToKeycode(qt_xdisplay(), keysym), mod | NumLockMask,
	   qt_xrootwin(), True,
	   GrabModeAsync, GrabModeSync);
  XGrabKey(qt_xdisplay(),
	   XKeysymToKeycode(qt_xdisplay(), keysym), mod | LockMask | NumLockMask,
	   qt_xrootwin(), True,
	   GrabModeAsync, GrabModeSync);

  
}

int main( int argc, char ** argv ){

  o_argc = argc;
  o_argv = new char*[o_argc + 2];
  int v;

  for (v=0; v<o_argc; v++) o_argv[v] = argv[v];
  o_argv[v] = NULL;

  MyApp myapp( argc, argv, "kpanel" );

  for (v=1; v<argc; v++){
    printf(KPANEL_VERSION);
    printf("\n");
    printf("Copyright (C) 1997 Matthias Ettrich (ettrich@kde.org)\n\n");
    if (QString("-version")!=argv[v]){
      printf("Desktop Panel for the KDE Desktop Environment\n");
      printf("Permitted arguments:\n");
      printf("-help : displays this message\n");
      printf("-version : displays the version number\n");
      printf("And all KApplication and Qt-toolkit arguments.\n");
    }
    exit(0);
  }
  
  if (!KWM::isKWMInitialized()){
    printf("kpanel: waiting for windowmanager\n");
    while (!KWM::isKWMInitialized()) sleep(1);
    sleep(1);
  }
  
  the_panel = new kPanel(&myapp);
  the_panel->connect(&myapp, SIGNAL(init()), 
		     SLOT(kwmInit()));
  the_panel->connect(&myapp, SIGNAL(windowAdd(Window)), 
		     SLOT(windowAdd(Window)));
  the_panel->connect(&myapp, SIGNAL(windowRemove(Window)), 
		     SLOT(windowRemove(Window)));
  the_panel->connect(&myapp, SIGNAL(windowChange(Window)), 
		     SLOT(windowChange(Window)));
  the_panel->connect(&myapp, SIGNAL(windowActivate(Window)), 
		     SLOT(windowActivate(Window)));
  the_panel->connect(&myapp, SIGNAL(windowIconChanged(Window)), 
		     SLOT(windowIconChanged(Window)));
  the_panel->connect(&myapp, SIGNAL(desktopChange(int)), 
		     SLOT(kwmDesktopChange(int)));
  the_panel->connect(&myapp, SIGNAL(desktopNameChange(int, QString)), 
		     SLOT(kwmDesktopNameChange(int, QString)));
  the_panel->connect(&myapp, SIGNAL(desktopNumberChange(int)), 
		     SLOT(kwmDesktopNumberChange(int)));
  the_panel->connect(&myapp, SIGNAL(commandReceived(QString)), 
		     SLOT(kwmCommandReceived(QString)));

  the_panel->connect(&myapp, SIGNAL(kdisplayPaletteChanged()), 
		     SLOT(kdisplayPaletteChanged()));

  myapp.setMainWidget(the_panel);
  myapp.connectToKWM();
  the_panel->show();
  XSelectInput(qt_xdisplay(), qt_xrootwin(), 
	       KeyPressMask);
  grabKey(XK_F1, Mod1Mask);
  return myapp.exec();
}
