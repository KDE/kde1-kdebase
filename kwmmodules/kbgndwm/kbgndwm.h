/*
 * kbgndwm.h.  Part of the KDE Project.
 *
 * Copyright (C) 1997 Martin Jones
 *               1998 Matej Koss
 *
 */

#ifndef __KBGNDWM_H__
#define __KBGNDWM_H__

//----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>

#include <kwmmapp.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include "bg.h"

//----------------------------------------------------------------------------

#define MAX_DESKTOPS 8

//----------------------------------------------------------------------------


class KBGndManager: public QWidget
{

  Q_OBJECT

public:
  KBGndManager( KWMModuleApplication * );

protected:
  void applyDesktop( int d );
  void cacheDesktop();
  void readSettings();

  void paintEvent(QPaintEvent *e);

public slots:
  void desktopChange( int );
  void commandReceived( QString );
  void toggleOneDesktop();

  void dock();
  void undock();
  void timeclick();

private slots:
  void mousePressEvent(QMouseEvent *e);
  void settings();

private:
  KWMModuleApplication* kwmmapp;
  QPopupMenu *popup_m;
  QPixmap pixmap;

  KBackground *desktops;
  int current;

  bool oneDesktopMode;
  int desktop;

  bool docked;

  int o_id;
};



//----------------------------------------------------------------------------

#endif
