/****************************************
 *
 *   desktop.cpp  - The Desktop widget
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
 *   Send comments and bug fixes to larrosa@kde.org or antlarr@arrakis.es
 *   or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain
 *
 */
#include "desktop.moc"
#include <qpainter.h>
#include <stdio.h>

//#define DESKTOPDEBUG

Desktop::Desktop(int _id,int swidth, int sheight,QWidget *parent, char *_name)
    : QWidget(parent,_name) , QDropSite(this)
{
    id=_id;
    window_list=new QList<WindowProperties>();
    screen_width=swidth;
    screen_height=sheight;
    desktopActived=false;
    //    setBackgroundColor(QColor(192,192,192));
    //    setBackgroundColor(QColor(0,0,195));
    setBackgroundMode(NoBackground);

    QPainter *qp=new QPainter(this);
    headerHeight=qp->fontMetrics().height()+5;
    delete qp;

    backgroundPixmap=0L;
    bigBackgroundPixmap=0L;
    useBackgroundInfoFromKbgndwm=false;
    useWallpaper=false;

    readBackgroundSettings();

}

Desktop::~Desktop()
{
#ifdef DESKTOPDEBUG
    printf("~Desktop\n");
#endif
    delete window_list;
    if (backgroundPixmap!=0L) delete backgroundPixmap;
    if (bigBackgroundPixmap!=0L) delete bigBackgroundPixmap;
}

void Desktop::setDesktopActived(bool status)
{
    desktopActived=status;
    if (status==true)
    {
        WindowProperties *wp=window_list->first();
        while ((wp!=NULL)&&(wp->active==false))
        {
            wp=window_list->next();
        }
        if ((wp!=NULL)&&(wp->active==true)) wp->active=false;
    }
    
    update();
}

void Desktop::clearWindowList(void)
{
    window_list->clear();
    update();
}

void Desktop::addWindow(Window w,int pos)
{
    WindowProperties *wp=new WindowProperties;
#ifdef DESKTOPDEBUG
    printf("[%d]window Added : %ld\n",id,w);
#endif
    wp->id=w;
    wp->name=KWM::title(w);
    wp->active=(KWM::activeWindow()==w)? true: false;
    wp->geometry=KWM::geometry(w,FALSE);
    wp->framegeometry=KWM::geometry(w,TRUE);
    if (pos==-1) window_list->append(wp);
     else window_list->insert(pos,wp);
    update();
}

int Desktop::getIndexOfWindow(Window w)
{
    WindowProperties *wp=window_list->first();
    while ((wp!=NULL)&&(wp->id!=w))
    {
        wp=window_list->next();
    }
    if (wp==NULL) return -1;
    return window_list->at();
}


void Desktop::removeWindow(Window w)
{
    window_list->remove(getIndexOfWindow(w));
    update();
}

void Desktop::changeWindow(Window w)
{
#ifdef DESKTOPDEBUG
    printf("[%d]Change window %ld\n",id,w);
#endif
    uint wid=getIndexOfWindow(w);
    window_list->remove(wid);
    WindowProperties *wp=new WindowProperties;
    wp->id=w;
    wp->name=KWM::title(w);
    wp->active=(KWM::activeWindow()==w)? true: false;
    wp->geometry=KWM::geometry(w,FALSE);
    wp->framegeometry=KWM::geometry(w,TRUE);
    window_list->insert(wid,wp);
    update();
}

void Desktop::raiseWindow(Window w)
{
    uint wid=getIndexOfWindow(w);
    window_list->remove(wid);
    WindowProperties *wp=new WindowProperties;
    wp->id=w;
    wp->name=KWM::title(w);
    wp->active=(KWM::activeWindow()==w)? true: false;
    wp->geometry=KWM::geometry(w,FALSE);
    wp->framegeometry=KWM::geometry(w,TRUE);
    window_list->append(wp);
    update();
}

void Desktop::lowerWindow(Window w)
{
    uint wid=getIndexOfWindow(w);
    window_list->remove(wid);
    WindowProperties *wp=new WindowProperties;
    wp->id=w;
    wp->name=KWM::title(w);
    wp->active=(KWM::activeWindow()==w)? true: false;
    wp->geometry=KWM::geometry(w,FALSE);
    wp->framegeometry=KWM::geometry(w,TRUE);
    window_list->insert(0,wp);
    update();
}

void Desktop::activateWindow(Window w)
{
    WindowProperties *wp=window_list->first();
    int end=0;
#ifdef DESKTOPDEBUG
    printf("activate Window\n");
#endif
    while ((wp!=NULL)&&(end<2))
    {
        if (wp->active==true)
        {
            wp->active=false;
            end++;
        }
        if (wp->id==w)
        {
            wp->active=true;
            end++;
        }
        wp=window_list->next();
    }
    update();
}

void Desktop::drawSunkRect(QPainter *p,int x,int y,int w,int h,bool highlighted)
{
    if (highlighted)
        p->setPen(QColor(255,255,180));
    else
        p->setPen(QColor(236,236,236));
    
    p->drawLine(x+w,y,x+w,y+h);
    p->drawLine(x,y+h,x+w,y+h);

    if (highlighted)
        p->setPen(QColor(255,255,0));
    else
        p->setPen(QColor(192,192,192));
    
    p->drawLine(x+w-1,y+1,x+w-1,y+h-1);
    p->drawLine(x+1,y+h-1,x+w-1,y+h-1);

    if (highlighted)
        p->setPen(QColor(192,192,0));
    else
        p->setPen(QColor(145,145,145));
    
    p->drawLine(x,y,x+w,y);
    p->drawLine(x,y,x,y+h);

    if (highlighted)
        p->setPen(QColor(127,127,0));
    else
        p->setPen(QColor(96,96,96));
    
    p->drawLine(x+1,y+1,x+w-1,y+1);
    p->drawLine(x+1,y+1,x+1,y+h-1);

    p->setPen(QColor(0,0,0));
    p->drawRect(x+2,y+2,w-3,h-3);

    if (highlighted)
       p->fillRect(x+3,y+3,w-5,h-5,QColor(255,255,0));
    else
       p->fillRect(x+3,y+3,w-5,h-5,QColor(192,192,192));

}

int Desktop::getHeaderHeight(void)
{
    return headerHeight;
}
int Desktop::headerHeight=25;


void Desktop::paintEvent(QPaintEvent *)
{
    QPainter *painter=new QPainter(this);
    painter->setPen(QColor(0,0,0));
    QRect tmp,tmp2;
    int x=0;
    int y=getHeaderHeight();
    painter->fillRect(0,0,width(),y,QColor(192,192,192));

    painter->drawText(20,getHeaderHeight()*3/4,name(),strlen(name()));

    drawSunkRect(painter,2,2,10,16,desktopActived ? true : false);
    
    WindowProperties *wp=window_list->first();
    painter->setClipRect(x,y,width()-x,height()-y);
    painter->setClipping(TRUE);
    if ((useBackgroundInfoFromKbgndwm)&&(backgroundPixmap!=0L))
    {
        painter->drawPixmap(x,y,*backgroundPixmap);
    }
    else
    {
        painter->fillRect(x,y,width()-x,height()-y,QColor(46,135,180));
    }

    double ratiox=(double)width()/(double)screen_width;
    double ratioy=(double)(height()-y)/(double)screen_height;
    while (wp!=NULL)
    {
        tmp.setRect((int)(wp->framegeometry.x()*ratiox),(int)(y+wp->framegeometry.y()*ratioy),(int)(wp->framegeometry.width()*ratiox),(int)(wp->framegeometry.height()*ratioy));
        
        if ((wp->active)&&(desktopActived)) painter->fillRect(tmp,QColor(255,255,0));
        else painter->fillRect(tmp,QColor(200,200,200));

// The next line causes oclock to behave very strange, I better don't call KWM::getDecoration
//        if (KWM::getDecoration(wp->id)==KWM::normalDecoration)
        {
//        tmp2.setRect((int)wp->geometry.x()*ratiox,(int)y+wp->geometry.y()*ratioy,(int)wp->geometry.width()*ratiox,(int)wp->geometry.height()*ratioy);
            tmp2.setRect(tmp.x()+2,tmp.y()+6,tmp.width()-4,tmp.height()-8);

            painter->fillRect(tmp2,QColor(145,145,145));
        }
        painter->drawRect(tmp);
        wp=window_list->next();
    }
    painter->setClipping(FALSE);
    painter->setPen(QColor(0,0,0));
    painter->drawRect(0,0,width(),height());

    delete painter;
}

bool Desktop::contains(Window w)
{
#ifdef DESKTOPDEBUG
    if (getIndexOfWindow(w)==-1)
        printf("[%d]Window %ld doesn't exist\n",id,w);
    else
        printf("[%d]Window %ld does exist\n",id,w);
#endif
   return (getIndexOfWindow(w)==-1)? false : true;
}

WindowProperties *Desktop::windowAtPosition(const QPoint *p,bool *ok,QPoint *pos=0L)
{
    WindowProperties *wp=window_list->last();
    if (wp==0L) 
    {
        if (ok!=0L) *ok=false;
        return 0L;
    }
    int y=getHeaderHeight();
    double ratiox=(double)width()/(double)screen_width;
    double ratioy=(double)(height()-y)/(double)screen_height;
    wp->minigeometry.setRect((int)(wp->framegeometry.x()*ratiox),(int)(y+wp->framegeometry.y()*ratioy),(int)(wp->framegeometry.width()*ratiox),(int)(wp->framegeometry.height()*ratioy));
#ifdef DESKTOPDEBUG
    printf("pos : x %d   y %d\n",p->x(),p->y());
#endif
    while ((wp!=0L)&&(!wp->minigeometry.contains(*p)))
    {
#ifdef DESKTOPDEBUG
        printf("minigeom : x %d   y %d    w %d   h %d\n",wp->minigeometry.x(),wp->minigeometry.y(),wp->minigeometry.width(),wp->minigeometry.height());
#endif
        wp=window_list->prev();
        if (wp!=NULL) wp->minigeometry.setRect((int)(wp->framegeometry.x()*ratiox),(int)(y+wp->framegeometry.y()*ratioy),(int)(wp->framegeometry.width()*ratiox),(int)(wp->framegeometry.height()*ratioy));
    }
    
    if (wp==0L)
    {
        if (ok!=0L) *ok=false;
        return 0L;
    }
    if (ok!=0L) *ok=true;
    if (pos!=0L)
    {
        pos->setX(p->x()-(int)wp->minigeometry.x());
        pos->setY(p->y()-(int)wp->minigeometry.y());
    }
    return wp;
}


void Desktop::mousePressEvent (QMouseEvent *e)
{
    QPoint dragpos;
    bool ok;
    WindowProperties *wp=windowAtPosition(&e->pos(),&ok,&dragpos);
    if (wp==0L) return;

    QPixmap *qxpm=new QPixmap(wp->minigeometry.width(),wp->minigeometry.height());
    QPainter *painter=new QPainter(qxpm);
    QRect tmp(0,0,qxpm->width(),qxpm->height());

    if ((wp->active)&&(desktopActived)) painter->fillRect(tmp,QColor(255,255,0));
    else painter->fillRect(tmp,QColor(200,200,200));
    if (KWM::getDecoration(wp->id)==KWM::normalDecoration)
    {
        QRect tmp2(tmp.x()+2,tmp.y()+6,tmp.width()-4,tmp.height()-8);
        painter->fillRect(tmp2,QColor(145,145,145));
    }
    painter->drawRect(tmp);
    delete painter;

    int deltax=dragpos.x();
    int deltay=dragpos.y();
#ifdef DESKTOPDEBUG
    printf("Window %ld , dx %d , dy %d\n",wp->id,deltax,deltay);
#endif
    windowDrag *wd= new windowDrag(wp->id,deltax,deltay,id,this);
    wd->setPixmap(*qxpm,QPoint(deltax,deltay));
    delete qxpm;
    wd->dragCopy();
}

void Desktop::dragEnterEvent(QDragEnterEvent *e)
{
    if (windowDrag::canDecode( e )) e->accept();
}

void Desktop::dragMoveEvent(QDragMoveEvent *)
{
}

void Desktop::dropEvent(QDropEvent *e)
{
    Window w=0;
    int deltax,deltay;
    int origdesk;
    if (windowDrag::decode(e,w,deltax,deltay,origdesk))
    {
#ifdef DESKTOPDEBUG
        printf("Window %ld , dx %d , dy %d\n",w,deltax,deltay);
        printf("e->x %d e->y %d\n",e->pos().x(),e->pos().y());
#endif
        int x=e->pos().x()-deltax;
        int y=e->pos().y()-deltay-getHeaderHeight();
        double ratiox=(double)width()/(double)screen_width;
        double ratioy=(double)(height()-getHeaderHeight())/(double)screen_height;
        x=(int)(x/ratiox);
        y=(int)(y/ratioy);
        emit moveWindow(w,id,x,y,origdesk);
    }
    
}

void Desktop::mouseDoubleClickEvent( QMouseEvent *)
{
    emit switchToDesktop(id);
}


void Desktop::readBackgroundSettings(void)
{
    KConfig config(KApplication::kde_configdir() + "/kdisplayrc",
                   KApplication::localconfigdir() + "/kdisplayrc");
    char group[50];
    sprintf(group,"Desktop%d",id);
    
    config.setGroup ( group );

    QString str;
    
    str = config.readEntry( "Color1", "#4682B4");
    color1.setNamedColor(str);
    str = config.readEntry( "Color2", "#4682B4");
    color2.setNamedColor(str);
    
    gfMode=Flat;
    uint pattern[8];
    str = config.readEntry("ColorMode");
    if (str == "Gradient")
    {
        gfMode=Gradient;
    }
    else if (str == "Pattern" )
    {
	printf("Pattern background are not supported (yet)\n");
        gfMode=Pattern;
        QStrList strl;
        config.readListEntry("Pattern", strl);
        uint size = strl.count();
        if (size > 8) size = 8;
        uint i = 0;
        for (i = 0; i < 8; i++)
            pattern[i] = (i < size) ? QString(strl.at(i)).toUInt() : 255;
    }
    
    orMode=Portrait;
    str = config.readEntry( "OrientationMode" );
    if (str == "Landscape" ) orMode=Landscape;
    
    wpMode = Tiled;
    str = config.readEntry( "WallpaperMode" );
    if ( !str.isEmpty() )
    {
        if ( str == "Centred" )
            wpMode = Centred;
        else if ( str == "Scaled" )
            wpMode = Scaled;
    }
    
    useWallpaper = config.readBoolEntry( "UseWallpaper", false );
    QString wallpaper;
    if ( useWallpaper )
    {
        QPixmap *qxpm;
        wallpaper = config.readEntry( "Wallpaper", "" );
        qxpm=loadWallpaper(wallpaper);
        backPixmapWidth=qxpm->width();
        backPixmapHeight=qxpm->height();
        if (qxpm==NULL) printf("isNULL !!!\n");
        if (bigBackgroundPixmap!=0L) delete bigBackgroundPixmap;
        if (backgroundPixmap!=0L) delete backgroundPixmap;
        QWMatrix matrix;
        matrix.scale((double)120/backPixmapWidth,(double)120/backPixmapWidth);
        bigBackgroundPixmap = new QPixmap(qxpm->xForm(matrix));
        delete qxpm;
        matrix.scale(width()/bigBackgroundPixmap->width(),(height()-getHeaderHeight())/bigBackgroundPixmap->height());
        backgroundPixmap = new QPixmap(bigBackgroundPixmap->xForm(matrix));
        useBackgroundInfoFromKbgndwm=true;
#ifdef DESKTOPDEBUG
        printf("[%d]Use wallpaper\n",id);
#endif
    }
    if ((!useWallpaper)||(wallpaper.isEmpty()))
    {
        switch (gfMode)
        {
        case Gradient:
            {
                int numColors=4;
                if (QColor::numBitPlanes() > 8)
                    numColors=16;
                KPixmap *kbigBackgroundPixmap = new KPixmap;
                kbigBackgroundPixmap->resize(width(),height());
                kbigBackgroundPixmap->gradientFill(color2,color1,(orMode==Portrait)?true:false,numColors);
                backgroundPixmap=new QPixmap;
                *backgroundPixmap=*kbigBackgroundPixmap;
                delete kbigBackgroundPixmap;
                useBackgroundInfoFromKbgndwm=true;
#ifdef DESKTOPDEBUG
                printf("[%d]Use background\n",id);
#endif
            }
        }
    };
};

QPixmap *Desktop::loadWallpaper(QString wallpaper)
{
    QString filename;

    if ( wallpaper[0] != '/' )
    {
        filename = KApplication::kde_wallpaperdir().copy() + "/";
        filename += wallpaper;
    }
    else
        filename = wallpaper;

    KPixmap *wpPixmap = new KPixmap;

    if ( wpPixmap->load( filename, 0, KPixmap::LowColor ) == FALSE )
    {
        delete wpPixmap;
        wpPixmap = 0;
    }
    return wpPixmap;
}

void Desktop::resizeEvent (QResizeEvent *)
{
    if (!useBackgroundInfoFromKbgndwm) return;
    if ((!useWallpaper)||(wpMode==Centred))
    {
        if(gfMode==Gradient)
        {
            int numColors=4;
            if (QColor::numBitPlanes() > 8)
                numColors=16;
            KPixmap *kBackgroundPixmap = new KPixmap;
            kBackgroundPixmap->resize(width(),height()-getHeaderHeight());
#ifdef DESKTOPDEBUG
	    printf("size %d x %d\n",width(),height());
#endif
            if (width()>1)
                kBackgroundPixmap->gradientFill(color2,color1,(orMode==Portrait)?true:false,numColors);
	    else
	        kBackgroundPixmap->fill(QColor(0,0,192));
            if (backgroundPixmap!=0L) delete backgroundPixmap;
            backgroundPixmap=new QPixmap;
            *backgroundPixmap=*kBackgroundPixmap;
            delete kBackgroundPixmap;
        }
    }
    if (useWallpaper)
    {
        switch (wpMode)
            
        {
        case (Scaled) :
            {
                if (backgroundPixmap!=0L) delete backgroundPixmap;
                QWMatrix matrix;
                matrix.scale((double)width()/bigBackgroundPixmap->width(),((double)height()-getHeaderHeight())/bigBackgroundPixmap->height());
                backgroundPixmap = new QPixmap(bigBackgroundPixmap->xForm(matrix));
            } break;
        case (Tiled) :
            {
                if (backgroundPixmap!=0L) delete backgroundPixmap;
                backgroundPixmap=new QPixmap();
                backgroundPixmap->resize(width(),height()-getHeaderHeight());
                QWMatrix matrix;
                matrix.scale(
                             ((double)backPixmapWidth*width()/screen_width)/bigBackgroundPixmap->width(),
                             (((double)backPixmapWidth*(height()-getHeaderHeight())/screen_height)/bigBackgroundPixmap->height())
                             );
                QPixmap *tmp = new QPixmap(bigBackgroundPixmap->xForm(matrix));
                int x,y=0;
#ifdef DESKTOPDEBUG
                printf("backPixmap %d x %d\n",backPixmapWidth,backPixmapHeight);
                printf("size  %d x %d\n",width(),height()-getHeaderHeight());
                printf("tmp %d  x  %d\n",tmp->width(),tmp->height());
                printf("ratio %g  x  %g\n",((double)backPixmapWidth*width()/screen_width)/*/bigBackgroundPixmap->width()*/,(((double)backPixmapWidth*(height()-getHeaderHeight())/screen_height)/*/bigBackgroundPixmap->height()*/));
#endif
                if ((tmp->height()>2)&&(tmp->width()>2))
                {
                while (y<tmp->height()+height()-getHeaderHeight())
                {
                    x=0;
                    while (x<width()+tmp->width())
                    {
                        bitBlt(backgroundPixmap,x,y,tmp,0,0,tmp->width(),tmp->height(),CopyROP);
                        x+=tmp->width();
                    }
                    y+=tmp->height();
                };
                } else
                    backgroundPixmap->fill(QColor(0,0,195));
                delete tmp;

            } break;
        case (Centred) :
            {
                QWMatrix matrix;
                matrix.scale(
                             ((double)backPixmapWidth*width()/screen_width)/bigBackgroundPixmap->width(),
                             (((double)backPixmapWidth*(height()-getHeaderHeight())/screen_height)/bigBackgroundPixmap->height())
                             );
                QPixmap *tmp = new QPixmap(bigBackgroundPixmap->xForm(matrix));
                bitBlt(backgroundPixmap,(width()-tmp->width())/2,((height()-getHeaderHeight())-tmp->height())/2,tmp,0,0,tmp->width(),tmp->height(),CopyROP);

                delete tmp;
            } break;
        }
        
    }

};
