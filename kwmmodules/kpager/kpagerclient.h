/****************************************
 *
 *   kpagerclient.h  - The KPager view, which does all the work
 *   Copyright (C) 1998  Antonio Larrosa Jimenez
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
 *   Send comments and bug fixes to antlarr@arrakis.es
 *   or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain
 *
 */

#ifndef _KPAGERCLIENT_H
#define _KPAGERCLIENT_H

#include <ktmainwindow.h>
#include <kmenubar.h>
#include <kwmmapp.h>
#include <kapp.h>
#include <kwm.h>
#include "desktop.h"
#include "ktrianglebutton.h"
#include <X11/Xlib.h>

#define MAXDESKTOPS 33
// Note that the 0 index of the desktop array is used for the global Desktop,
// the normal desktops go from 1 to 32

class KApplication;


class KPagerClient : public QWidget
{
    Q_OBJECT
private:
    int screenwidth;
    int screenheight;
    int deltax;
    int maxdeltax;
    double velocity;
    Desktop *desktop[MAXDESKTOPS];
    KWMModuleApplication *kwmmapp;
    
    QWidget *desktopContainer;
    bool visibleGlobalDesktop;
    bool use2Rows;
    int drawMode;
    
    KTriangleButton *left;
    KTriangleButton *right;
    
    Window move_window_w;
    int move_window_dsk;

    int numberofDesktops;
    int activedesktop;
    
    int desktopContaining(Window w);
    void updateRects(bool onlydesktops=false);

    void initDesktops(void);
protected:
    virtual void paintEvent(QPaintEvent *);

    
public:
    KPagerClient(KWMModuleApplication *_kwmmapp,QWidget *parent,const char *name=0);
    virtual ~KPagerClient();

    virtual void resizeEvent(QResizeEvent *);

    void setVisibleGlobalDesktop(bool status);
    bool isVisibleGlobalDesktop(void) { return visibleGlobalDesktop; }

    void setDrawMode(int mode);
    int getDrawMode(void) { return drawMode; }

    bool areArrowsVisible(void) { return right->isVisible(); }
    bool is2Rows(void) { return use2Rows; }
    
public slots:
    void moveRight();
    void moveLeft();
    void pressedButtonLR();
    void singleClickR();
    void singleClickL();


    void desktopChanged(int);
    void windowAdded(Window);
    void windowRemoved(Window);
    void windowChanged(Window);
    void windowRaised(Window);
    void windowLowered(Window);
    void windowActivated(Window);
    void desktopNameChanged(int,QString);
    void desktopNumberChanged(int);
    void commandReceived(QString s);

    void moveWindow(Window w,int dsk,int x,int y, int origdesk);
    void switchToDesktop(int i);
    void updateDesk(int i);
    void toggle2Rows();

    
};

#endif
