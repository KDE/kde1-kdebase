// -*- C++ -*-
//
//  ktaskbar
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@bonn.netsurf.de
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef CONFMENU_H
#define CONFMENU_H

#include <qpopmenu.h>
#include <qlist.h>
#include <qbttngrp.h>
#include <qframe.h>

#include <drag.h>

#include "pmenu.h"
#include "button.h"

class ConfMenuDialog;
class EntryDialog;

class MenuButton : public EditButton
{
  Q_OBJECT
public:
  MenuButton ( PMenuItem *p_it, int i, PMenu *p_parent, QWidget *parent=0, const char* name=0 );
  ~MenuButton ();
  
  void setId(int i) { id = i; }
  int  getId() { return id; }
  void parentMoved() { moveEvent( NULL ); }

signals:
  void clicked(int);
  void Mclicked(int);
  void Rclicked(int);
  void pressed(int);
  void Mpressed(int);
  void Rpressed(int);
  void released(int);
  void Mreleased(int);
  void Rreleased(int);
  void posChanged( int, QPoint );
  void newButton( int );
  void delButton( int );
  void pasteButton( int );

public slots:
  void change_item();

protected slots:
  void open();
  void sOpen(int) { open(); }
  void new_item() { emit newButton( id ); }
  void cutItem() { copyItem(); emit delButton( id ); }
  void copyItem();
  void pasteItem() { emit pasteButton( id); }
  void delete_item();
  void change_accept();
  void change_reject();
  void popupMenu( int );
  void childRepos() { moveEvent( NULL ); }
  
protected:
  virtual void mousePressEvent(QMouseEvent *e);
  virtual void dndMouseReleaseEvent(QMouseEvent *e);
  virtual void dndMouseMoveEvent(QMouseEvent *e);
  virtual void dragEndEvent() { left_pressed = FALSE; setDown(FALSE); }
  virtual void moveEvent(QMoveEvent *e);
  virtual void drawButton(QPainter *_painter);
  virtual void drawButtonLabel(QPainter *_painter);
  virtual void leaveEvent( QEvent *_ev );
  virtual void enterEvent( QEvent *_ev );
  virtual void focusOutEvent( QFocusEvent * );
  void paint( QPainter *_painter );
  
  int             id;
  EntryType       type;
  QPopupMenu      popmenu;
  EntryDialog    *dialog;
  bool            dialog_open;
  PMenuItem      *pmenu_item;
  PMenu          *pmenu_parent;
  bool            move_button;
  bool            submenu_open;
  QPoint          move_offset;
  bool            move_group;
  bool            left_pressed;
  int             press_x;
  int             press_y;
};

class ConfigureMenu;

class ConfMenuItem : public QObject
{
  Q_OBJECT
  friend ConfigureMenu;
public:
  ConfMenuItem (MenuButton *but, PMenuItem *item);
  ~ConfMenuItem () { if(button) delete button; }
protected:
  MenuButton *button;
  PMenuItem  *pmenu_item;
};

class ConfigureMenu : public QFrame
{
  Q_OBJECT
public:
  ConfigureMenu( PMenu *m, QWidget *parent=0, const char *name=0 );
  ~ConfigureMenu();

  void append (PMenuItem *item);
  void update ();
  int  getHeight () { return height; }
  int  getWidth () { return width; }
  void moveRequest( int x, int y ) { if( move_enable ) move(x, y); }
  void moveEnable( bool b ) { move_enable = b; }

protected slots:
  void buttonMoved( int id, QPoint p );
  void newButton( int id );
  void delButton( int id );
  void pasteButton( int id );
  virtual void moveEvent( QMoveEvent *e);
  void urlDroped(KDNDDropZone *);

protected:
  short             but_nr;
  short             width;
  short             height;
  short             group_height;
  PMenu            *pmenu;
  QList<ConfMenuItem> but_list;
  QButtonGroup     *but_group;
  QButtonGroup     *prot_group;
  bool              move_enable;
};


#endif // CONFMENU_H
