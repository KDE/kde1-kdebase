/****************************************
 *
 *   kpagerclient.cpp  - The KPager view, which does all the work
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
#include "kpagerclient.moc"

#include <stdio.h>
#include <qstring.h>
#include <qpainter.h>
#include <unistd.h>
//#include <X11/Xlib.h>

//#define KPAGERCLIENTDEBUG


KPagerClient::KPagerClient(KWMModuleApplication *_kwmmapp,QWidget *parent,const char *name)
    :QWidget(parent,name)
{
/*    client=new QWidget(this,"ClientWindow");
    kmidclient->songType(1);
    kmidclient->show();
    setView(kmidclient,FALSE);
*/

    kwmmapp=_kwmmapp;
    activedesktop=1;
    move_window_w=0;
    move_window_dsk=-1;
    deltax=0;
    maxdeltax=0;
    visibleGlobalDesktop=true;
    for (int i=0;i<MAXDESKTOPS;i++) desktop[i]=0L;
    numberofDesktops=0;
    screenwidth=KWM::geometry( qt_xrootwin()).width();
    screenheight=KWM::geometry( qt_xrootwin()).height();

    desktop[0]=new Desktop(0,screenwidth,screenheight,this,"Global Desktop");
    connect(desktop[0],SIGNAL(moveWindow(Window,int,int,int,int)),this,SLOT(moveWindow(Window,int,int,int,int)));

    right=new KTriangleButton(KTriangleButton::Right,this,"Right");
    left=new KTriangleButton(KTriangleButton::Left,this,"Left");
    connect(right,SIGNAL(clickedQuickly()),this,SLOT(moveRight()));
    connect(left,SIGNAL(clickedQuickly()),this,SLOT(moveLeft()));
    connect(right,SIGNAL(pressed()),this,SLOT(pressedButtonLR()));
    connect(left,SIGNAL(pressed()),this,SLOT(pressedButtonLR()));
    
    desktopContainer=new QWidget(this,"desktopContainer");
    desktopContainer->setBackgroundColor(QColor(96,129,137));
    setBackgroundMode(NoBackground);
    //    setBackgroundColor(QColor(96,129,137));

    initDesktops();

};

KPagerClient::~KPagerClient()
{
#ifdef KPAGERCLIENTDEBUG
    printf("KPagerClient:: destructor\n");
#endif
    /*
    for (int i=0;i<MAXDESKTOPS;i++)
        if (desktop[i]!=0L)
        {
            delete desktop[i];
            desktop[i]=0L;
        }
*/
#ifdef KPAGERCLIENTDEBUG
    printf("KPagerClient:: destructor(end)\n");
#endif
    
    
};

void KPagerClient::desktopChanged(int i)
{
#ifdef KPAGERCLIENTDEBUG
    printf("desktop changed to %d\n",i);
#endif
    desktop[activedesktop]->setDesktopActived(false);
    activedesktop=i;
    
    desktop[activedesktop]->setDesktopActived(true);

}

int KPagerClient::desktopContaining(Window w)
{
    int d=-1;
    int i=0;
    while ((d==-1)&&(i<MAXDESKTOPS))
    {
        if (desktop[i]->contains(w)) d=i;
        i++;
    }
    return d;
};


void KPagerClient::windowAdded(Window w)
{
#ifdef KPAGERCLIENTDEBUG
    printf("window added\n");
#endif
    if (KWM::isSticky(w))
    {
        for (int i=0;i<MAXDESKTOPS;i++)
        {
            if ((desktop[i]!=0L)&&(!desktop[i]->contains(w))) desktop[i]->addWindow(w);
        }
    } else
        desktop[KWM::desktop(w)]->addWindow(w);
    
}
void KPagerClient::windowRemoved(Window w)
{
#ifdef KPAGERCLIENTDEBUG
    printf("window removed\n");
#endif
    if (desktop[0]->contains(w))
    {
        for (int i=0;i<MAXDESKTOPS;i++)
        {
            if (desktop[i]!=0L) desktop[i]->removeWindow(w);
        }
    } else
        desktop[KWM::desktop(w)]->removeWindow(w);
    
}
void KPagerClient::windowChanged(Window w)
{
#ifdef KPAGERCLIENTDEBUG
    printf("window changed\n");
#endif

    int dsk=KWM::desktop(w);
    
    if (KWM::isSticky(w))
    {
        if (!desktop[0]->contains(w))
        {
            for (int i=0;i<MAXDESKTOPS;i++)
            {
                if ((desktop[i]!=0L)&&(dsk!=i)) desktop[i]->addWindow(w);
            }
            desktop[dsk]->changeWindow(w);
        }
        else
        for (int i=0;i<MAXDESKTOPS;i++)
        {
            if (desktop[i]!=0L) desktop[i]->changeWindow(w);
        }
    } else
    {
        if ((move_window_dsk!=-1)&&(move_window_w=w)) // This is done
            // because the wm doesn't support to unsticky and move desktop at a time
        {
            KWM::moveToDesktop(move_window_w,move_window_dsk);
            move_window_dsk=-1;
        };

        if (desktop[0]->contains(w))
        {
            for (int i=0;i<MAXDESKTOPS;i++)
            {
                if ((desktop[i]!=0L)&&(dsk!=i)) desktop[i]->removeWindow(w);
            }
            desktop[dsk]->changeWindow(w);
        }
        else
        {
            int d=desktopContaining(w);
            if (d!=dsk)  // This window has been moved across desktops
            {
                desktop[d]->removeWindow(w);
                int i=0;
                QList <Window> *windowlist=new QList<Window>(kwmmapp->windows_sorted);
                Window *win=windowlist->first();
                while ((win!=0L)&&(*win!=w))
                {
                    if (KWM::desktop(*win)==dsk) i++;
                    win=windowlist->next();
                }
                delete windowlist;
                
                desktop[dsk]->addWindow(w,i);
                

            }
            else
                desktop[dsk]->changeWindow(w);
            
        };
        
    }
    
}
void KPagerClient::windowRaised(Window w)
{
#ifdef KPAGERCLIENTDEBUG
    printf("window raised\n");
#endif
    if (KWM::isSticky(w))
    {
        for (int i=0;i<MAXDESKTOPS;i++)
        {
            if (desktop[i]!=0L) desktop[i]->raiseWindow(w);
        }
    } else
        desktop[KWM::desktop(w)]->raiseWindow(w);
    
}
void KPagerClient::windowLowered(Window w)
{
#ifdef KPAGERCLIENTDEBUG
    printf("window lowered\n");
#endif
    if (KWM::isSticky(w))
    {
        for (int i=0;i<MAXDESKTOPS;i++)
        {
            if (desktop[i]!=0L) desktop[i]->lowerWindow(w);
        }
    } else
        desktop[KWM::desktop(w)]->lowerWindow(w);
    
}
void KPagerClient::windowActivated(Window w)
{
#ifdef KPAGERCLIENTDEBUG
    printf("window activated\n");
#endif
    desktop[KWM::desktop(w)]->activateWindow(w);
}

void KPagerClient::desktopNameChanged(int,QString)
{
#ifdef KPAGERCLIENTDEBUG
    printf("desktop name changed\n");
#endif
}

void KPagerClient::desktopNumberChanged(int)
{
#ifdef KPAGERCLIENTDEBUG
    printf("desktop number changed\n");
#endif
}

void KPagerClient::initDesktops(void)
{
    numberofDesktops=KWM::numberOfDesktops();
    int x=5;
    int y=5;
    int h=height()-10;
    int w=(h-Desktop::getHeaderHeight())*screenwidth/screenheight;

    for (int i=1;i<=numberofDesktops;i++)
    {
//        desktop[i]=new Desktop(i+1,screenwidth,screenheight,this,KWM::getDesktopName(i).data());
        desktop[i]=new Desktop(i,screenwidth,screenheight,desktopContainer,KWM::getDesktopName(i).data());
        connect(desktop[i],SIGNAL(moveWindow(Window,int,int,int,int)),this,SLOT(moveWindow(Window,int,int,int,int)));
        connect(desktop[i],SIGNAL(switchToDesktop(int)),this,SLOT(switchToDesktop(int)));
        desktop[i]->setGeometry(x,y,w,h);
        x+=w+5;
    };

    activedesktop=KWM::currentDesktop();
    desktop[activedesktop]->setDesktopActived(true);

    QList <Window> *windowlist=new QList<Window>(kwmmapp->windows_sorted);
    Window *win=windowlist->first();
    while (win!=0L)
    {
        windowAdded(*win);
        win=windowlist->next();
    }
    delete windowlist;


}
void KPagerClient::moveRight()
{
    velocity+=0.5;
    deltax+=(int)velocity;
    if (deltax>=maxdeltax)
    {
        velocity=0;
        deltax=maxdeltax;
        updateRects(true);
    }
    else
    desktopContainer->scroll(-(int)velocity,0);
    //    updateRects(true);
}
void KPagerClient::moveLeft()
{
    velocity+=0.5;
    deltax-=(int)velocity;
    if (deltax<=0)
    {
        velocity=0;
        deltax=0;
        updateRects(true);
    }
    else
    desktopContainer->scroll((int)velocity,0);
    //    updateRects(true);
}

void KPagerClient::pressedButtonLR()
{
    velocity=2;
}


void KPagerClient::updateRects(bool onlydesktops)
{
    int x=2;
    int y=0;
    int h=height()-10;
    int w=(h-Desktop::getHeaderHeight())*screenwidth/screenheight;
    if (visibleGlobalDesktop)
        desktopContainer->setGeometry(3,5,width()-w-36,h);
    else
        desktopContainer->setGeometry(3,5,width()-26,h);
    
    if ((desktopContainer->width()==1)||(desktopContainer->height()==1))
        desktopContainer->hide();
    else if (!desktopContainer->isVisible())
        desktopContainer->show();

        
    for (int i=1;i<=numberofDesktops;i++)
    {
        if (desktop[i]!=NULL) desktop[i]->setGeometry(x-deltax,y,w,h);
        x+=w+5;
    }
    maxdeltax=x-5-desktopContainer->width()+2;
    
    if (!onlydesktops)
        if (visibleGlobalDesktop)
        {
            desktop[0]->setGeometry(width()-w-5,5,w,h);
            right->setGeometry(width()-w-30,0,20,height()/2);
            left->setGeometry(width()-w-30,height()/2,20,height()/2+1);
        }
        else
        {
            right->setGeometry(width()-20,0,20,height()/2);
            left->setGeometry(width()-20,height()/2,20,height()-height()/2);
        };
}

void KPagerClient::resizeEvent(QResizeEvent *)
{
    updateRects(false);
}

void KPagerClient::moveWindow(Window w,int dsk,int x,int y, int origdesk)
{
#ifdef KPAGERCLIENTDEBUG
    printf("move from %d to %d\n",origdesk,dsk);
#endif
    if (dsk==0)
    {
        KWM::setSticky(w,true);
    }
    else
    {
        if (origdesk==0)
        {
            KWM::setSticky(w,false);
            move_window_w=w;
            move_window_dsk=dsk;
        };
        KWM::moveToDesktop(w,dsk);
    }

    QPoint p(x,y);
    KWM::move(w,p);
}

void KPagerClient::switchToDesktop(int i)
{
    KWM::switchToDesktop(i);
}

void KPagerClient::setVisibleGlobalDesktop(bool status)
{
    if (status==visibleGlobalDesktop) return;
    if (status==true)
        desktop[0]->show();
    else
        desktop[0]->hide();
    visibleGlobalDesktop=status;

    updateRects();
    update();
}

void KPagerClient::paintEvent(QPaintEvent *)
{
    if (right->x()<6) return;
    QPainter *painter=new QPainter(this);
    // I suppose painting four small bars is faster than painting a big rect which
    // is mostly behind another widget
//    painter->fillRect(0,0,right->x(),height(),QColor(96,129,137));
    QColorGroup qcg;
    qcg=colorGroup();
    if (visibleGlobalDesktop)
    {
        painter->fillRect(right->x()+right->width(),0,desktop[0]->x(),height(),qcg.background());
        painter->fillRect(right->x()+right->width(),0,width()-right->x()-right->width(),desktop[0]->y(),qcg.background());
        painter->fillRect(desktop[0]->x()+desktop[0]->width(),0,width()-desktop[0]->x()-desktop[0]->width(),height(),qcg.background());
        painter->fillRect(right->x()+right->width(),desktop[0]->y()+desktop[0]->height(),width()-right->x()-right->width(),height()-desktop[0]->y()-desktop[0]->height(),qcg.background());
    }

    //    painter->setPen(QColor(236,236,236));
    painter->setPen(qcg.light());
    painter->drawLine(0,height()-1,right->x()-1,height()-1);
    painter->drawLine(right->x()-1,0,right->x()-1,height()-1);
    //    painter->setPen(QColor(193,193,193));
    painter->setPen(qcg.background());
    painter->drawLine(1,height()-2,right->x()-2,height()-2);
    painter->drawLine(right->x()-2,1,right->x()-2,height()-2);
    //    painter->setPen(QColor(145,145,145));
    painter->setPen(qcg.mid());
    painter->drawLine(0,0,0,height()-1);
    painter->drawLine(0,0,right->x()-1,0);
    //    painter->setPen(QColor(96,96,96));
    painter->setPen(qcg.dark());
    painter->drawLine(1,1,1,height()-2);
    painter->drawLine(1,1,right->x()-2,1);
    painter->setPen(QColor(0,0,0));
    painter->drawRect(2,2,right->x()-4,height()-4);
    painter->setPen(QColor(96,129,137));
    painter->drawLine(3,3,right->x()-4,3);
    painter->drawLine(3,4,right->x()-4,4);
    painter->drawLine(3,height()-4,right->x()-4,height()-4);
    painter->drawLine(3,height()-5,right->x()-4,height()-5);
    delete painter;
}

void KPagerClient::commandReceived(QString s)
{
    if (strcmp(s.data(),"kbgwm_reconfigure")==0)
    {
        for (int i=1;i<=numberofDesktops;i++)
        {
            if (desktop[i]!=NULL) desktop[i]->reconfigure();
        }
    }

#ifdef KPAGERCLIENTDEBUG
    else
        printf("KPager: Command (%s) is not for me\n",s.data());
#endif
};

void KPagerClient::setDrawMode(int mode)
{
    for (int i=0;i<=numberofDesktops;i++)
    {
        if (desktop[i]!=NULL) desktop[i]->setDrawMode(mode);
    }
}