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

int o_argc;
char ** o_argv;


void restart_the_panel(){
  QApplication::exit();
  execvp(o_argv[0],  o_argv);
}

#include "kpanel.moc"

int main( int argc, char ** argv ){

  o_argc = argc;
  o_argv = new char*[o_argc + 2];
  int v;

  for (v=0; v<o_argc; v++) o_argv[v] = argv[v];
  o_argv[v] = NULL;

  KWMModuleApplication myapp( argc, argv, "kpanel" );

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
  }
  
  kPanel* the_panel = new kPanel(&myapp);
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
  return myapp.exec();
}
