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
  setFocusPolicy(StrongFocus);
  setMinimumSize(480,480);
}


void KSwallowWidget::resizeEvent(QResizeEvent*)
{
  if (window != 0)
    XResizeWindow(qt_xdisplay(), window, width(), height());
}

void KSwallowWidget::focusInEvent( QFocusEvent * ){
  // workarund: put the focus onto the swallowed widget (ettrich)
  // TODO: When we switch to a newer qt than qt-1.33 this hack should
  // be replaced with my new kswallow widget!
  if (isActiveWindow() && isVisible()){ // isActiveWindow is important here!
    // verify wether the window still belongs to us
    unsigned int nwins;
    Window dw1, dw2, *wins;
    XQueryTree(qt_xdisplay(), winId(), 
	       &dw1, &dw2, &wins, &nwins);
    if (nwins)
      XSetInputFocus(qt_xdisplay(), window, RevertToParent, CurrentTime);
  }
}

void KSwallowWidget::swallowWindow(Window w)
{
  window = w;

  KWM::prepareForSwallowing(w);

  XReparentWindow(qt_xdisplay(), w, winId(), 0, 0);
  XMapRaised(qt_xdisplay(), w);
 
  // let the swallowd window set the size
  int x, y;
  unsigned int width, height, dummy;
  Window root;

  XGetGeometry(qt_xdisplay(), window, &root, &x, &y, &width, &height, &dummy, &dummy);
  if (width < parentWidget()->width())
    width = parentWidget()->width();
  if (height < parentWidget()->height())
    height = parentWidget()->height();
  parentWidget()->resize(width,height);
  resize(width,height);
  XResizeWindow(qt_xdisplay(), window, width, height);

  orig = QSize(width,height);
 
  show();
}
