/****************************************
 *
 *   ktrianglebutton.cpp  - The KTriangleButton widget (button with an arrow)
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
 *   Note: This widget was copied from KButton as found in kdelibs/kdeui
 *   and then it was modified to increase funcionality.
 *   KButton was originally copyrighted by Torben Weis (weis@kde.org)
 *   and Matthias Ettrich (ettrich@kde.org) on 1997
 */
     
#include "ktrianglebutton.h"
#include <qpainter.h>
#include <qdrawutil.h>

KTriangleButton::KTriangleButton( Direction d,QWidget *_parent, const char *name )
    : QButton( _parent , name)
{
    dir=d;
    raised = FALSE;
    setFocusPolicy( NoFocus );
}

KTriangleButton::~KTriangleButton()
{
}

void KTriangleButton::enterEvent( QEvent* )
{
    if ( isEnabled() )
    {
        raised = TRUE;
        repaint(FALSE);
    }
}

void KTriangleButton::leaveEvent( QEvent * )
{
    if( raised != FALSE )
    {
        raised = FALSE;
        repaint();
    }
}


void KTriangleButton::drawButton( QPainter *_painter )
{
    paint( _painter );
}

void KTriangleButton::drawButtonLabel( QPainter *_painter )
{
    paint( _painter );
}

void KTriangleButton::paint( QPainter *painter )
{
    if ( isDown() || isOn() )
    {
        if ( style() == WindowsStyle )
            qDrawWinButton( painter, 0, 0, width(), 
                            height(), colorGroup(), TRUE );
        else
            qDrawShadePanel( painter, 0, 0, width(), 
                             height(), colorGroup(), TRUE, 2, 0L );
    }
    else if ( raised )
    {
        if ( style() == WindowsStyle )
            qDrawWinButton( painter, 0, 0, width(), height(), 
                            colorGroup(), FALSE );
        else
            qDrawShadePanel( painter, 0, 0, width(), height(), 
                             colorGroup(), FALSE, 2, 0L );
    }
    
    if (dir==Right)
    {
        int x=width()/4;
        int y=height()*2/6;
        int l=height()-y*2;
        int i=0;
        int maxi=width()-2*x;
        double m=((double)(l/2))/maxi;
        while (i<=maxi)
        {
            painter->drawLine(x,y+(int)(i*m),x,y+l-(int)(i*m));
            x++;
            i++;
        };
    }
    else if (dir==Left)
    {
        int x=width()/4;
        int y=height()*2/6;
        int l=height()-y*2;
        int i=0;
        int maxi=width()-2*x;
        x=width()-x;
        double m=((double)(l/2))/maxi;
        while (i<=maxi)
        {
            painter->drawLine(x,y+(int)(i*m),x,y+l-(int)(i*m));
            x--;
            i++;
        };
        
    };
    
}

void KTriangleButton::mousePressEvent(QMouseEvent *e)
{
    QButton::mousePressEvent(e);
    //    tresholdTimer=true;
    usingTimer=true;
    startTimer(350);
    timeCount=0;
    
}

void KTriangleButton::mouseReleaseEvent(QMouseEvent *e)
{
    if ((usingTimer)&&(timeCount==0))       // It has been a single click
    {
        emit singleClick();
    };
    usingTimer=false;
    QButton::mouseReleaseEvent(e);
}

void KTriangleButton::timerEvent(QTimerEvent *)
{
    if (!usingTimer) {killTimers();return;};
    if (timeCount==0)
    {
        timeCount++;
        killTimers();
        startTimer(25);
    }
    emit clickedQuickly();
    
}
