/*
  kswallow.h - a widget to swallow a program

  written 1997 by Matthias Hoelzer
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */


#ifndef _SWALLOW_H_
#define _SWALLOW_H_


#include <X11/X.h>
#include <qwidget.h>
#include <qstring.h>


class KSwallowWidget : public QWidget
{
  Q_OBJECT
  
public:

  KSwallowWidget(QWidget *parent=0, const char *name=0);

  void swallowWindow(Window w);

protected: 

  void resizeEvent(QResizeEvent *);

private:

  Window  window;

};

#endif


