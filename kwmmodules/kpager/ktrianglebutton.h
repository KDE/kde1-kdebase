/****************************************
 *
 *   ktrianglebutton.h  - The KTriangleButton widget (button with an arrow)
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
 *   Note: This widget was copied from KButton as found in the kdelibs/kdeui
 *   and then it was modified to increase funcionality.
 *   KButton was originally copyrighted by Torben Weis (weis@kde.org)
 *   and Matthias Ettrich (ettrich@kde.org) on 1997
 */

#include <qbutton.h>

class KTriangleButton : public QButton
{
    Q_OBJECT;

public:
    enum Direction {Left , Right, Up, Down};
    /*
     Up and Down are not implemented, feel free to implement them yourself
     if you need them :-)
     */
protected:
     
     Direction dir;
     bool usingTimer;
     int timeCount;
     
public:
    KTriangleButton( Direction d,QWidget *_parent = 0L, const char *name = 0L );
    virtual ~KTriangleButton();

    virtual void leaveEvent( QEvent *_ev );
    virtual void enterEvent( QEvent *_ev );
        
    virtual void drawButton( QPainter *_painter );
    virtual void drawButtonLabel( QPainter *_painter );

    void paint( QPainter *_painter );

signals:
    void clickedQuickly();

protected:
    bool raised;

    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void timerEvent(QTimerEvent *);

};