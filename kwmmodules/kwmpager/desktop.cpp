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

#include "desktop.moc"
#include "pager.h"
#include <qpainter.h>
#include <stdlib.h>


Desktop::Desktop(KWMModuleApplication *a, int id, Pager *parent) :
  QFrame(parent)
{
    kwmmapp = a;
    Id = id;
    setFrameStyle(Panel | Raised);
    root_size = KWM::geometry( qt_xrootwin() ).size();
    pixmap_size = contentsRect().size() - QSize(2,2);
    windows.setAutoDelete( true );
    activeWindow = 0L;
    fillPixmap();
    repaint();
}

void Desktop::fillPixmap()
{
    PagerWindow *win;
    pixmap.resize(pixmap_size);
    pixmap.fill(backgroundColor());
    QPainter p(&pixmap);
    p.setPen(QColor(0,0,0));
    
    for (win = windows.first(); win != 0L; win = windows.next()) {
	int x = win->rect.x() * pixmap_size.width() / root_size.width() + 1;
	int y = win->rect.y() * pixmap_size.height() / root_size.height() + 1;
	int w = win->rect.width() * pixmap_size.width() / root_size.width();
	int h = win->rect.height() * pixmap_size.height() / root_size.height();
	win->prect = QRect(x,y,w,h);
	QColor col = 
	    (win == activeWindow) ? 
	    kapp->activeTitleColor : 
	    kapp->inactiveTitleColor;
	p.fillRect(x, y, w, h,QBrush(col));
	p.drawRect(x, y, w, h);
    }
    
}

void Desktop::init()
{
    windows.clear();
    pixmap.fill(backgroundColor());
}

void Desktop::addWindow(Window w)
{
    PagerWindow *win = new PagerWindow;
    if (w == KWM::activeWindow())
	activeWindow = win;
    
    win->id = w;
    win->rect = KWM::geometry(w);
    
    windows.append(win);
    fillPixmap();
}

void Desktop::windowActivate(Window w)
{
    for (PagerWindow *win = windows.first(); win != 0L; win = windows.next()) {
	if (win->id == w) {
	    activeWindow = win;
	    break;
	}
    }
    fillPixmap();
}

void Desktop::activate(bool flag)
{
    if (!flag) {
	activeWindow = NULL;
	setFrameStyle(Panel | Raised);
    } else 
	setFrameStyle(Panel | Sunken);
    
    repaint(false);
}

void Desktop::mousePressEvent( QMouseEvent *e )
{
    KWM::switchToDesktop(Id);
    for (PagerWindow *win = windows.last(); win; win = windows.prev()) {
	if (win->prect.contains(e->pos())) {
	    KWM::activate(win->id);
	    break;
	}
    }
}

void Desktop::resizeEvent ( QResizeEvent * )
{
    pixmap_size = contentsRect().size();
    fillPixmap();
    repaint();
}

void Desktop::drawContents ( QPainter *p )
{
    if (!pixmap.isNull())
	p->drawPixmap(0, 0, pixmap);
}
