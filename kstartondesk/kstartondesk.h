/*
 * kstartondesk.h Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <qapp.h>
#include <qcursor.h>
#include <qlist.h>
#include <qstring.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <qwidget.h>
#include <qpopmenu.h>
#include <qstrlist.h>
#include <kwmmapp.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qpushbt.h>


class KStartOnDesk: public QObject {
  Q_OBJECT

public:
  KStartOnDesk(KWMModuleApplication* kwmmapp_arg, 
	       const char* command_arg,
	       const char*  desktop_arg,
	       const char* title_arg,
	       bool activate_arg = false);
  ~KStartOnDesk(){};

public slots:

  void windowAdd(Window);
  void timerDone();

private:
  KWMModuleApplication* kwmmapp;
  const char* command;
  const char* title;
  int desktop;
  bool activate;
};



