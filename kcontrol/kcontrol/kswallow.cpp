/*
  kswallow.cpp - a widget to swallow a program

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


#include <kwm.h>
#include <kwmmapp.h>

#include "kswallow.h"
#include "kswallow.moc"


KSwallowWidget::KSwallowWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  window = 0;
  resize(480,480);
}


void KSwallowWidget::resizeEvent(QResizeEvent*)
{
  if (window != 0)
    XResizeWindow(qt_xdisplay(), window, width(), height());
}


void KSwallowWidget::swallowWindow(Window w)
{
  KWM::prepareForSwallowing(w);

  XReparentWindow(qt_xdisplay(), w, winId(), 0, 0);
  XMapRaised(qt_xdisplay(), w);
  XResizeWindow(qt_xdisplay(), window, parentWidget()->width(), parentWidget()->height());

  show();

  window = w;
}
