// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//

#include "kpanel.h"
#include <qapp.h>
#include <kwmmapp.h>
#include <qdir.h>
#include <qmsgbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "kpanel_version.h"
#include <X11/keysym.h>

#include <dirent.h>
#include <sys/stat.h>

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
  if (ev->xany.window != None &&
      the_panel->parentOfSwallowed(ev->xany.window)){
    if (ev->type == ButtonPressMask){
      XAllowEvents(qt_xdisplay(), SyncPointer, CurrentTime);
      the_panel->parentOfSwallowed(ev->xany.window)->grabMouse();
    }
    ev->xany.window = the_panel->parentOfSwallowed(ev->xany.window)->winId();
  }

  return KWMModuleApplication::x11EventFilter(ev);
}

void testDir( const char *_name )
{
  DIR *dp;
  QString c = KApplication::localkdedir();
  c += _name;
  dp = opendir( c.data() );
  if ( dp == NULL )
    ::mkdir( c.data(), S_IRWXU );
  else
    closedir( dp );
}

void copyFiles( QString source, QString dest )
{
  char data[1024];
  QFile in(source);
  QFile out(dest);
  if( !in.open(IO_ReadOnly) || !out.open(IO_WriteOnly) )
    return;
  int len;
  while( (len = in.readBlock(data, 1024)) > 0 ) {
    if( out.writeBlock(data,len) < len ) {
      len = -1;
      break;
    }
  }
  out.close();
  in.close();
  if( len == -1 ) { // abort and remove destination file
    QFileInfo fi(dest);
    fi.dir().remove(dest);
  }
}

int main( int argc, char ** argv ){

  o_argc = argc;
  o_argv = new char*[o_argc + 2];
  int v;

  for (v=0; v<o_argc; v++) o_argv[v] = argv[v];
  o_argv[v] = 0;

  MyApp myapp( argc, argv, "kpanel" );
  bool use_kwm = true;

  for (v=1; v<argc; v++){
    if (QString("-version")==argv[v]){
      printf(KPANEL_VERSION);
      printf("\n");
      printf("Copyright (C) 1997 Matthias Ettrich (ettrich@kde.org)\n\n");
      exit(0);
    }
    if (QString("-no-KDE-compliant-window-manager")==argv[v]){
      use_kwm = false;
      break;
    }

    printf("Desktop Panel for the KDE Desktop Environment\n");
    printf("Permitted arguments:\n");
    printf("-help : displays this message\n");
    printf("-version : displays the version number\n");
    printf("-no-KDE-compliant-window-manager : force startup without \n");
    printf("           initializing the module communication \n");
    printf("And all KApplication and Qt-toolkit arguments.\n");
    exit(0);
  }

  if (use_kwm){
    if (!KWM::isKWMInitialized()){
      printf("kpanel: waiting for windowmanager\n");
      while (!KWM::isKWMInitialized()) sleep(1);
      sleep(1);
    }
  }

  // create $HOME/.kde/share/apps/kpanel/applnk
  testDir( "" );
  testDir( "/share" );
  testDir( "/share/config" );
  testDir( "/share/apps" );
  testDir( "/share/apps/kpanel" );
  testDir( "/share/apps/kpanel/applnk" );
  // create $HOME/.kde/share/applnk
  testDir( "/share/applnk" );
  // create default $HOME/.kde/share/applnk/.directory file if there is none
  QString src_path = KApplication::kde_datadir().copy();
  src_path += "/kpanel/default/personal_directory";
  QString dest_path = KApplication::localkdedir().copy();
  dest_path += "/share/applnk/.directory";
  QFileInfo fi(dest_path);
  if( !fi.exists() ) {
    copyFiles(src_path, dest_path);
  }

  the_panel = new kPanel(&myapp);
  the_panel->connect(&myapp, SIGNAL(init()),
		     SLOT(kwmInit()));
  the_panel->connect(&myapp, SIGNAL(windowAdd(Window)),
		     SLOT(windowAdd(Window)));
  the_panel->connect(&myapp, SIGNAL(dialogWindowAdd(Window)),
		     SLOT(dialogWindowAdd(Window)));
  the_panel->connect(&myapp, SIGNAL(windowRemove(Window)),
		     SLOT(windowRemove(Window)));
  the_panel->connect(&myapp, SIGNAL(windowChange(Window)),
		     SLOT(windowChange(Window)));
  the_panel->connect(&myapp, SIGNAL(windowActivate(Window)),
		     SLOT(windowActivate(Window)));
  the_panel->connect(&myapp, SIGNAL(windowIconChanged(Window)),
		     SLOT(windowIconChanged(Window)));
  the_panel->connect(&myapp, SIGNAL(windowRaise(Window)),
		     SLOT(windowRaise(Window)));
  the_panel->connect(&myapp, SIGNAL(desktopChange(int)),
		     SLOT(kwmDesktopChange(int)));
  the_panel->connect(&myapp, SIGNAL(desktopNameChange(int, QString)),
		     SLOT(kwmDesktopNameChange(int, QString)));
  the_panel->connect(&myapp, SIGNAL(desktopNumberChange(int)),
		     SLOT(kwmDesktopNumberChange(int)));
  the_panel->connect(&myapp, SIGNAL(commandReceived(QString)),
		     SLOT(kwmCommandReceived(QString)));
  the_panel->connect(&myapp, SIGNAL(dockWindowAdd(Window)),
		     SLOT(dockWindowAdd(Window)));
  the_panel->connect(&myapp, SIGNAL(dockWindowRemove(Window)),
		     SLOT(dockWindowRemove(Window)));

//   the_panel->connect(&myapp, SIGNAL(playSound(QString)),
// 		     SLOT(playSound(QString)));

  the_panel->connect(&myapp, SIGNAL(kdisplayPaletteChanged()),
		     SLOT(kdisplayPaletteChanged()));
  
  the_panel->connect( &myapp, SIGNAL( kdisplayStyleChanged() ), SLOT( restart() ) );
  

  myapp.setMainWidget(the_panel);

  // connect to kwm as docking module
  myapp.connectToKWM(true);
  the_panel->launchSwallowedApplications();
  the_panel->show();
  myapp.syncX();
  myapp.processEvents();
  the_panel->parseMenus();
  XSelectInput(qt_xdisplay(), qt_xrootwin(),
	       KeyPressMask);
  while (1)
    myapp.exec();
  return 0;
}
