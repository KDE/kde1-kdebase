#ifndef _KWMPAGER_DESKTOP_H
#define _KWMPAGER_DESKTOP_H

/*
 *   kwmpager - a pager for kwm (by Matthias Ettrich)
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

#include <kwmmapp.h>
#include <X11/X.h>
#include <qframe.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <qdatetm.h>

class Pager;

struct PagerWindow {
    Window id; // Window ID for kwm
    QRect rect; // window geometry
    QRect prect; // geometry in pager
    QRect mrect; // geometry while moved
    bool icony; // flag for icon state
    QString name; // title
};

class Desktop : public QFrame {
    Q_OBJECT
    
public:
    Desktop(KWMModuleApplication *a, int id, Pager *parent);
    ~Desktop() {};
    void activate(bool flag);
    void addWindow(Window);
    void addWindow(PagerWindow *win);
    PagerWindow *removeWindow(Window);
    PagerWindow *getWindow(Window);
    void changeWindow(Window w);
    void raiseWindow(Window w);
    void activateWindow(Window w);
    void lowerWindow(Window);
    void init();

protected:
    void fillPixmap();
    void calculate(PagerWindow* win);
    virtual void mousePressEvent ( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void mouseDoubleClickEvent ( QMouseEvent *);
    virtual void drawContents ( QPainter * );
    virtual void resizeEvent ( QResizeEvent * );  
    virtual void paletteChange ( const QPalette &);
    PagerWindow *activeWindow;

private:
    KWMModuleApplication* kwmmapp;
    int Id;
    QList<PagerWindow> windows;
    QPixmap pixmap;
    QSize root_size;
    QSize pixmap_size;
    QPoint dragStart;
    PagerWindow *dragWindow;
    QTime startTime;
    bool cursor_set;

signals:
    void activated(int);
    void doubleClick();
};

#endif
