/*
 * kstart.h Part of the KDE project.
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


class KStart: public QObject {
  Q_OBJECT

public:
  KStart(KWMModuleApplication* kwmmapp_arg,
	 const char* command_arg,
	 const char* window_arg,
	 int desktop_arg,
	 bool activate_arg,
	 bool maximize_arg,
	 bool iconify_arg,
	 bool sticky_arg,
	 int decoration_arg);
  ~KStart(){};

public slots:

  void initialized();
  void windowAdd(Window);

private:
    
  void applyStyle(Window);
    
    
  KWMModuleApplication* kwmmapp;
  const char* command;
  const char* window;
  int desktop;
  bool activate;
  bool maximize;
  bool iconify;
  bool sticky;
  int decoration;
};



