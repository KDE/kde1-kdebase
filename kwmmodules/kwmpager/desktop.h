#ifndef _KWMPAGER_DESKTOP_H
#define _KWMPAGER_DESKTOP_H

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

#include <kwmmapp.h>
#include <X11/X.h>
#include <qframe.h>

class Pager;

struct PagerWindow {
    Window id;
    QRect rect;
    QRect prect;
};

class Desktop : public QFrame {
    Q_OBJECT
    
public:
    Desktop(KWMModuleApplication *a, int id, Pager *parent);
    ~Desktop() {};
    void activate(bool flag);
    void addWindow(Window);
    void removeWindow(Window);
    void windowChange(Window);
    void windowRaise(Window);
    void windowLower(Window);
    void windowActivate(Window);
    void init();
    
protected:
    void fillPixmap();
    virtual void mousePressEvent ( QMouseEvent * );
    virtual void drawContents ( QPainter * );
    virtual void resizeEvent ( QResizeEvent * );  
    PagerWindow *activeWindow;
    
private:
    KWMModuleApplication* kwmmapp;
    int Id;
    QList<PagerWindow> windows;
    QPixmap pixmap;
    QSize root_size;
    QSize pixmap_size;
    
signals:
    void activated(int);
};

#endif
