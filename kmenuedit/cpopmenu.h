//
//  ktaskbar
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@mail.bonn.netsurf.de
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


#ifndef CPOPMENU_H
#define CPOPMENU_H

#include <qpopmenu.h>

class CPopupMenu: public QPopupMenu
{
  Q_OBJECT
public:
  CPopupMenu( QWidget *parent=0, const char *name=0 );
  ~CPopupMenu() {}

  int getRightHeight();
  int getRightWidth();
  void hide();
  void setActItem( int it );

protected:
  void mousePressEvent( QMouseEvent *e );

private:	// Disabled copy constructor and operator=
  CPopupMenu( const CPopupMenu & ) {}
  CPopupMenu &operator=( const CPopupMenu & ) { return *this; }
};


#endif // CPOPMENU_H




