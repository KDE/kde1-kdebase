/*
 * kstartondesk.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <qdir.h>

#include <kstartondesk.moc>
#include "version.h"

#include <fcntl.h>
#include <kprocess.h>
#include <qtimer.h>

void execute(const char* cmd){
  KShellProcess proc;
  proc << cmd;
  proc.start(KShellProcess::DontCare);
}

KStartOnDesk::KStartOnDesk(KWMModuleApplication* kwmmapp_arg, 
			   const char* command_arg,
			   const char* title_arg,
			   const char*  desktop_arg,
			   bool activate_arg)
  :QObject(){
    kwmmapp = kwmmapp_arg;
    command = command_arg;
    title = title_arg;
    QString s = desktop_arg;
    desktop = s.toInt();
    activate = activate_arg;
    kwmmapp->connectToKWM();
    QTimer::singleShot(300, this, SLOT(timerDone()));
}

void KStartOnDesk::timerDone(){
  connect(kwmmapp, SIGNAL(windowAdd(Window)), SLOT(windowAdd(Window)));
  KWM::doNotManage(title);
  execute(command);
}

void KStartOnDesk::windowAdd(Window w){
  QString t = KWM::titleWithState(w);
  if (t == title){
    if (KWM::desktop(w) != desktop){
      KWM::prepareForSwallowing(w);
      KWM::moveToDesktop(w, desktop);
      XSync(qt_xdisplay(), False);
    }
    XMapWindow(qt_xdisplay(), w);
    XSync(qt_xdisplay(), False);
    if (activate)
      KWM::activate(w);
    XSync(qt_xdisplay(), False);
    ::exit(0);
  }
}


int main( int argc, char *argv[] )
{
  KWMModuleApplication a (argc, argv);
  if (argc !=  4 && ! (argc == 5 && QString("-activate") == argv[1])){
    if (QString("-version") == argv[1]){
      printf(KSTARTONDESK_VERSION);
      printf("\n");
      printf(klocale->translate("Copyright (C) 1997 Matthias Ettrich (ettrich@kde.org)\n"));
      ::exit(0);
    }
    else {
      printf(klocale->translate("Usage:"));
      printf("%s [-version] [-activate] ", argv[0]);
      printf(klocale->translate("command "));
      printf(klocale->translate("title "));
      printf(klocale->translate("desktop"));
      printf("\n\n");
      printf(klocale->translate("Example"));
      printf(":  %s -activate \"kcalc -caption \\\"Calc\\\" \" \"Calc\" 3 \n", 
	     argv[0]);
    }
    ::exit(1); 
  }
  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);

  
  if (argc == 5)
    new KStartOnDesk(&a, argv[2], argv[3], argv[4], true );
  else
    new KStartOnDesk(&a, argv[1], argv[2], argv[3] );


  return a.exec();
}
