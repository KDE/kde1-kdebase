#ifndef _KWMPAGER_PAGER_H
#define _KWMPAGER_PAGER_H

/*
 *   kwmpager 0.1 - a pager for kwm (by Matthias Ettrich)
 *   Copyright (C) 1997  Stephan Kulow
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */        

#include <kapp.h>
#include <kwmmapp.h>
#include "desktop.h"

class Pager: public QWidget {
  Q_OBJECT

public:
  Pager(KWMModuleApplication*);

private:
  KWMModuleApplication* kwmmapp;
  QList<Desktop> desktops;
  Desktop *activeDesktop;

public slots:
  void initDesktops();

private slots:
  void changeDesktop(int);
  void changeNumber(int);
  void addWindow(Window);
  void removeWindow(Window);
  void windowChange(Window);
  void windowActivate(Window);
  
protected:
  virtual void resizeEvent ( QResizeEvent * );  
};

#endif
