#ifndef _KWMPAGER_PAGER_H
#define _KWMPAGER_PAGER_H

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

#include <kapp.h>
#include <kwmmapp.h>
#include <desktop.h>
#include <qframe.h>

class QFont;

class Pager: public QFrame {
    Q_OBJECT
    
public:
    Pager(KWMModuleApplication*);
    
private:
    KWMModuleApplication* kwmmapp;
    QList<Desktop> desktops;
    QList<PagerWindow> stickys;
    Desktop *activeDesktop;
    enum Position { TopRight, TopLeft, BottomRight, BottomLeft, Costumized } position;
    static const char *PosStrings[5];
    int posx, posy;
    QFont *desktop_font;

public slots:
    void initDesktops();
    
private slots:
    void changeDesktop(int);
    void changeNumber(int);
    void addWindow(Window);
    void removeWindow(Window);
    void windowChange(Window);
    void windowActivate(Window);
    void raiseWindow(Window);
    void lowerWindow(Window);
    void placeIt();
    void receiveCommand(QString command);
    
protected:
    virtual void resizeEvent ( QResizeEvent * );  
    void readSettings();
};

#endif
