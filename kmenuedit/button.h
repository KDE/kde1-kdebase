// -*- C++ -*-

//
//  kmenuedit
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

#ifndef button_included
#define button_included

#include <qbutton.h>
#include <qpixmap.h>

class EditButton : public QButton
{
  Q_OBJECT;
public:
  EditButton(QWidget *parent = NULL, const char *name = NULL);
  ~EditButton();

  void  setText( const char *t );
  void  setPixmap( const QPixmap &p );
  void  setGreyed( bool b ) { greyed = b; }
  QSize sizeHint();

  virtual void startDrag( KDNDIcon *_icon, char *_data, int _size, int _type, int _dx, int _dy );

public slots:
  void slotPressed();
  void slotReleased();

protected:
  
  QPixmap bpixmap;
  QString btext;
  int     raised;
  bool    greyed;

  // drag and drop stuff
  virtual Window EditButton::findRootWindow( QPoint & p );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void mouseReleaseEvent( QMouseEvent * );
  virtual void rootDropEvent( int _x, int _y );
  virtual void rootDropEvent();
  virtual void dragEndEvent() { }

  virtual void dndMouseMoveEvent( QMouseEvent * ) { }
  virtual void dndMouseReleaseEvent( QMouseEvent * ) { }

  bool drag;
  char *dndData;
  int dndSize;
  int dndType;
  int dndOffsetX;
  int dndOffsetY;
  KDNDIcon *dndIcon;
  Window dndLastWindow;
};


#endif /* button_included */



