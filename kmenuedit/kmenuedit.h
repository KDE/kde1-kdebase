// -*- C++ -*-

//
//  kmenuedit
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

#ifndef kmenuedit_included
#define kmenuedit_included

#include <qlayout.h>

#include <ktopwidget.h>

class QScrollBar;
class QPushButton;
class QFrame;
class KToolBar;
class PMenu;

class KMenuEdit : public KTopLevelWidget
{
  Q_OBJECT
public:
  KMenuEdit( const char* name = NULL );
  virtual ~KMenuEdit();

signals:

public slots:

protected slots:
  void move_h( int x );
  void move_v( int y );
  void startHelp();
  void about();
  void reload();
  void save();
  void reloadFileTypes();
  void changeMenuName();
  
protected:
  void loadMenus();
  void saveMenus();
  void resizeEvent( QResizeEvent *e );

  QFrame          *f_main;
  QFrame          *f_mask;
  QFrame          *f_move;
  QScrollBar      *scrollx;
  QScrollBar      *scrolly;
  PMenu           *pers_menu_data;
  PMenu           *glob_menu_data;
  QString          pers_menu_name;
  QString          glob_menu_name;
  bool             glob_menu_writable;
  QGridLayout     *top2bottom;
  KToolBar        *toolbar;
  KMenuBar        *menubar;
  KStatusBar      *statusbar;
};
#endif // kmenuedit_included



