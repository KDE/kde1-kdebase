/*
 * krootwm.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <qapp.h>
#include <qcursor.h>
#include <qlist.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <qwidget.h>
#include <qpopmenu.h>

#include "kwmmapp.h"

enum {
  RMB_HELP=100,
  RMB_DISPLAY_PROPERTIES,
  RMB_REFRESH_DESKTOP,
  RMB_ARRANGE_ICONS,
  RMB_LOCK_SCREEN,
  RMB_LOGOUT
};

class KRootWm: public QObject {
  Q_OBJECT

public:
  KRootWm(KWMModuleApplication*);
  bool eventFilter( QObject *, QEvent * );

private:
  KWMModuleApplication* kwmmapp;

  GC gc;
  void draw_selection_rectangle(int x, int y, int dx, int dy);
  bool select_rectangle(int &x, int &y, int &dx, int &dy);
  
  void generateWindowlist(QPopupMenu* p);
  Window* callbacklist;

  QPopupMenu* mmb;
  QPopupMenu* rmb;

 private slots:
  void rmb_menu_activated(int);
  void mmb_menu_activated(int);
};


