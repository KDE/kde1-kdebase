/****************************************
 *
 *   kpager.cpp  - The KPager main widget
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

#include <stdio.h>
#include <qstring.h>
#include <unistd.h>
#include <qkeycode.h>
#include <qmessagebox.h>
#include <kaccel.h>
#include <kpopmenu.h>
#include "version.h"
#include "kpager.h"

//#define KPAGERDEBUG

KPager::KPager(KWMModuleApplication *kwmmapp,const char *name)
    :KTMainWindow(name)
{
    kpagerclient=new KPagerClient(kwmmapp,this,"KPagerClient");
    kpagerclient->show();
    setView(kpagerclient,FALSE);
    
    kwmmapp->connectToKWM();
    KWM::setSticky(winId(), true);
    KWM::setDecoration(winId(), KWM::tinyDecoration);
    KWM::setWmCommand(winId(), "kpager");
    KWM::setUnsavedDataHint(winId(),false);
    
    QObject::connect(kwmmapp,SIGNAL(windowAdd(Window)),
                     kpagerclient,SLOT(windowAdded(Window)));
    QObject::connect(kwmmapp,SIGNAL(windowRemove(Window)),
                     kpagerclient,SLOT(windowRemoved(Window)));
    QObject::connect(kwmmapp,SIGNAL(windowChange(Window)),
                     kpagerclient,SLOT(windowChanged(Window)));
    QObject::connect(kwmmapp,SIGNAL(windowRaise(Window)),
                     kpagerclient,SLOT(windowRaised(Window)));
    QObject::connect(kwmmapp,SIGNAL(windowLower(Window)),
                     kpagerclient,SLOT(windowLowered(Window)));
    QObject::connect(kwmmapp,SIGNAL(windowActivate(Window)),
                     kpagerclient,SLOT(windowActivated(Window)));
    QObject::connect(kwmmapp,SIGNAL(desktopNameChange(int,QString)),
                     kpagerclient,SLOT(desktopNameChanged(int,QString)));
    QObject::connect(kwmmapp,SIGNAL(desktopNumberChange(int)),
                     kpagerclient,SLOT(desktopNumberChanged(int)));
    QObject::connect(kwmmapp,SIGNAL(desktopChange(int)),
                     kpagerclient,SLOT(desktopChanged(int)));
    QObject::connect(kwmmapp,SIGNAL(commandReceived(QString)),
                     kpagerclient,SLOT(commandReceived(QString)));
    
    kKeysAccel=new KAccel(this);
    kKeysAccel->insertItem(i18n("Toggle Global Desktop"),"Toggle Global Desktop", Key_0);
    kKeysAccel->connectItem("Toggle Global Desktop", this, SLOT(options_toggleGlobalDesktop()));
    kKeysAccel->insertItem(i18n("Toggle Menubar"), "Toggle Menubar", Key_Space );
    kKeysAccel->connectItem("Toggle Menubar", this, SLOT(options_toggleMenuBar()));
    kKeysAccel->connectItem(KAccel::Quit, this, SLOT( file_quit() ) );  
    kKeysAccel->insertItem(i18n("Move Right"), "Move Right", Key_Right );  
    kKeysAccel->connectItem("Move Right", kpagerclient, SLOT(singleClickR()));
    kKeysAccel->insertItem(i18n("Move Left"), "Move Left", Key_Left );  
    kKeysAccel->connectItem("Move Left", kpagerclient, SLOT(singleClickL()));
    kKeysAccel->readSettings();
    
    m_file = new QPopupMenu;
    m_file->setCheckable( TRUE );
    m_file->insertItem( i18n("&Quit"), this, SLOT(file_quit()) );
    kKeysAccel->changeMenuAccel(m_file, 1, KAccel::Quit ); 
    
    m_options = new QPopupMenu;
    if (kpagerclient->isVisibleGlobalDesktop())
        m_options->insertItem(i18n("Hide &Global Desktop"), this, SLOT(options_toggleGlobalDesktop()), Key_0 , 1 );
    else
        m_options->insertItem(i18n("Show &Global Desktop"), this, SLOT(options_toggleGlobalDesktop()), Key_0 , 1 );
    m_options->setId(0,0);
    
    kKeysAccel->changeMenuAccel(m_options,1, "Toggle Global Desktop");
    
    m_options->insertItem(i18n("Hide &Menubar"), this, SLOT(options_toggleMenuBar()), Key_Space , 2 );
    m_options->setId(1,1);
    
    kKeysAccel->changeMenuAccel(m_options,2, "Toggle Menubar");
    m_options->insertItem(i18n("&2 Rows"), this, SLOT(options_2Rows()));
    m_options->setId(2,2);
    m_options->setItemChecked(2,FALSE);
    m_options->insertItem(i18n("&One Click change Desktop"), this, SLOT(options_oneClickMode()));
    m_options->setId(3,3);
    m_options->setItemChecked(3,TRUE);
    if (kpagerclient->isShowingDesktopName())
        m_options->insertItem(i18n("Hide Desktop &Names"), this, SLOT(options_showDesktopName()) );
    else
        m_options->insertItem(i18n("Show Desktop &Names"), this, SLOT(options_showDesktopName()) );
    m_options->setId(4,4);
    m_options->insertSeparator();
    m_drawmode = new QPopupMenu;
    m_drawmode->setCheckable( TRUE );
    m_drawmode->insertItem(i18n("Plain"),this,SLOT(options_drawPlain()));
    m_drawmode->setId(0,1);
    m_drawmode->setItemChecked(1,FALSE);
    m_drawmode->insertItem(i18n("Icon"),this,SLOT(options_drawIcon()));
    m_drawmode->setId(1,2);
    m_drawmode->setItemChecked(2,FALSE);
    m_drawmode->insertItem(i18n("Pixmap"),this,SLOT(options_drawPixmap()));
    m_drawmode->setId(2,3);
    m_drawmode->setItemChecked(3,TRUE);
    
    m_options->insertItem(i18n("Draw Mode"),m_drawmode);
    
    char aboutstring[500];
    sprintf(aboutstring,
            i18n("%s\n\n" \
                 "(C) 1998 Antonio Larrosa Jimenez\n"
                 "larrosa@kde.org\t\tantlarr@arrakis.es\n" \
                 "Malaga (Spain)\n\n" \
                 "KPager's homepage is at : http://www.arrakis.es/~rlarrosa/kpager.html\n\n" \
                 "KPager comes with ABSOLUTELY NO WARRANTY; for details view file COPYING\n" \
                 "This is free software, and you are welcome to redistribute it\n" \
                 "under certain conditions\n"), VERSION_TXT );
    m_help = (KApplication::getKApplication())->getHelpMenu(true, aboutstring);
    
    
    menu = new KMenuBar(this);
    menu->insertItem(i18n("&File"),m_file);
    menu->insertItem(i18n("&Options"),m_options);
    menu->insertSeparator();
    menu->insertItem(i18n("&Help"),m_help);
    menu->show();
    menubar_visible=true;
    setMenu(menu);
    
    // Let's read the configuration
    
    KConfig *kcfg=kapp->getConfig();
    
    QRect r=KWM::getWindowRegion(KWM::currentDesktop());
    r.setTop(r.bottom()-140);
    r.setRight(r.left()+400);
    setGeometry(kapp->getConfig()->readRectEntry("Geometry",&r));
    
    readProperties(kcfg);
    
}

KPager::~KPager()
{
#ifdef KPAGERDEBUG
    printf("KPager::destructor\n");
#endif
}

void KPager::closeEvent( QCloseEvent *)
{
    kapp->getConfig()->writeEntry("Geometry",geometry());
    kapp->quit();
}

void KPager::file_quit()
{
    kapp->getConfig()->writeEntry("Geometry",geometry());
    kapp->quit();
}

void KPager::options_toggleGlobalDesktop()
{
    if (kpagerclient->isVisibleGlobalDesktop())
    {
        m_options->changeItem(i18n("Show &Global Desktop"), 0);
        kpagerclient->setVisibleGlobalDesktop(false);
    }
    else
    {
        kpagerclient->setVisibleGlobalDesktop(true);
        m_options->changeItem(i18n("Hide &Global Desktop"), 0);
    }
    
    kapp->getConfig()->writeEntry("visibleGlobalDesktop",kpagerclient->isVisibleGlobalDesktop());
    kapp->getConfig()->sync();
}

void KPager::options_toggleMenuBar()
{
    if (menubar_visible)
    {
        menubar_visible=false;
        menu->hide();
        updateRects();
    } else
    {
        menubar_visible=true;
        menu->show();
        if (menu->menuBarPos() != KMenuBar::Floating)
        {
            resize(width(),height());
        }
        updateRects();        
    }
    
    kapp->getConfig()->writeEntry("visibleMenuBar",menubar_visible);
    kapp->getConfig()->sync();
    
}

void KPager::options_2Rows()
{
    kpagerclient->toggle2Rows();
    if (kpagerclient->is2Rows()) m_options->setItemChecked(2,TRUE);
    else m_options->setItemChecked(2,FALSE);
    kapp->getConfig()->writeEntry("use2Rows",kpagerclient->is2Rows());
    kapp->getConfig()->sync();
};

void KPager::options_oneClickMode()
{
    kpagerclient->toggle1ClickMode();
    if (kpagerclient->is1ClickMode()) m_options->setItemChecked(3,TRUE);
    else m_options->setItemChecked(3,FALSE);
    kapp->getConfig()->writeEntry("use1ClickToChangeDesktop",kpagerclient->is1ClickMode());
    kapp->getConfig()->sync();
};

void KPager::options_showDesktopName()
{
    kpagerclient->toggleShowName();
    
    if (kpagerclient->isShowingDesktopName()) 
    {
        m_options->changeItem(i18n("Hide Desktop &Names"), 4); 
    }
    else 
    {
        m_options->changeItem(i18n("Show Desktop &Names"), 4); 
    }
    kapp->getConfig()->writeEntry("showDesktopName",kpagerclient->isShowingDesktopName());
    kapp->getConfig()->sync();
}; 

void KPager::options_drawPlain()
{
    m_drawmode->setItemChecked(1, TRUE);
    m_drawmode->setItemChecked(2, FALSE);
    m_drawmode->setItemChecked(3, FALSE);
    
    kpagerclient->setDrawMode(0);
    
    kapp->getConfig()->writeEntry("drawMode",kpagerclient->getDrawMode());
    kapp->getConfig()->sync();
}

void KPager::options_drawIcon()
{
    m_drawmode->setItemChecked(1, FALSE);
    m_drawmode->setItemChecked(2, TRUE);
    m_drawmode->setItemChecked(3, FALSE);
    
    kpagerclient->setDrawMode(1);
    
    kapp->getConfig()->writeEntry("drawMode",kpagerclient->getDrawMode());
    kapp->getConfig()->sync();
}

void KPager::options_drawPixmap()
{
    m_drawmode->setItemChecked(1, FALSE);
    m_drawmode->setItemChecked(2, FALSE);
    m_drawmode->setItemChecked(3, TRUE);
    
    kpagerclient->setDrawMode(2);
    
    kapp->getConfig()->writeEntry("drawMode",kpagerclient->getDrawMode());
    kapp->getConfig()->sync();
}


void KPager::saveProperties(KConfig *kcfg)
{
    kcfg->writeEntry("visibleMenuBar",menubar_visible);
    kcfg->writeEntry("visibleGlobalDesktop",kpagerclient->isVisibleGlobalDesktop());
    kcfg->writeEntry("drawMode",kpagerclient->getDrawMode());
    kcfg->writeEntry("use2Rows",kpagerclient->is2Rows());
    kcfg->writeEntry("use1ClickToChangeDesktop",kpagerclient->is1ClickMode());
    kcfg->writeEntry("showDesktopName",kpagerclient->isShowingDesktopName());
}

void KPager::readProperties(KConfig *kcfg)
{
    if (kcfg->readBoolEntry("visibleMenuBar"))
    {
        if (!menubar_visible)
            options_toggleMenuBar();
    }
    else
    {
        if (menubar_visible)
            options_toggleMenuBar();
        
    }
    if (kcfg->readBoolEntry("visibleGlobalDesktop"))
    {
        if (!kpagerclient->isVisibleGlobalDesktop())
            options_toggleGlobalDesktop();
    }
    else
    {
        if (kpagerclient->isVisibleGlobalDesktop())
            options_toggleGlobalDesktop();
        
    }
    
    int i=kcfg->readNumEntry("drawMode");
    switch (i)
    {
    case 0 : options_drawPlain();break;
    case 1 : options_drawIcon();break;
    case 2 : options_drawPixmap();break;
    }
    
    if (kcfg->readBoolEntry("use2Rows"))
    {
        if (!kpagerclient->is2Rows())
            options_2Rows();
    }
    else
    {
        if (kpagerclient->is2Rows())
            options_2Rows();
    }
    
    if (kcfg->readBoolEntry("use1ClickToChangeDesktop"))
    {
        if (!kpagerclient->is1ClickMode())
            options_oneClickMode();
    }
    else
    {
        if (kpagerclient->is1ClickMode())
            options_oneClickMode();
    }
    
    if (kcfg->readBoolEntry("showDesktopName"))
    {
        if (!kpagerclient->isShowingDesktopName())
            options_showDesktopName();
    }
    else
    {
        if (kpagerclient->isShowingDesktopName())
            options_showDesktopName();
    }
    
}

QPopupMenu *KPager::getOptionlikeMenu(void)
{
    QPopupMenu *m_options=new QPopupMenu;
    if (kpagerclient->isVisibleGlobalDesktop())
        m_options->insertItem(i18n("Hide &Global Desktop"), this, SLOT(options_toggleGlobalDesktop()), Key_0 , 1 );
    else
        m_options->insertItem(i18n("Show &Global Desktop"), this, SLOT(options_toggleGlobalDesktop()), Key_0 , 1 );
    m_options->setId(0,0);
    
    kKeysAccel->changeMenuAccel(m_options,1, "Toggle Global Desktop");
    
    if (menubar_visible)
        m_options->insertItem(i18n("Hide &Menubar"), this, SLOT(options_toggleMenuBar()), Key_Space , 2 );
    else
        m_options->insertItem(i18n("Show &Menubar"), this, SLOT(options_toggleMenuBar()), Key_Space , 2 );
    m_options->setId(1,1);
    
    kKeysAccel->changeMenuAccel(m_options,2, "Toggle Menubar");
    m_options->insertItem(i18n("&2 Rows"), this, SLOT(options_2Rows()));
    m_options->setId(2,2);
    m_options->setItemChecked(2,(kpagerclient->is2Rows())? TRUE : FALSE);
    m_options->insertItem(i18n("One Click change Desktop"), this, SLOT(options_oneClickMode()));
    m_options->setId(3,3);
    m_options->setItemChecked(3,(kpagerclient->is1ClickMode())? TRUE : FALSE);
    //    m_options->insertItem(i18n("Show Desktop Names"), this, SLOT(options_showDesktopName()));
    if (kpagerclient->isShowingDesktopName())
        m_options->insertItem(i18n("Hide Desktop &Names"), this, SLOT(options_showDesktopName()) );
    else
        m_options->insertItem(i18n("Show Desktop &Names"), this, SLOT(options_showDesktopName()) );
    m_options->setId(4,4);
    //    m_options->setItemChecked(4,(kpagerclient->isShowingDesktopName())? TRUE : FALSE);
    m_options->insertSeparator();
    QPopupMenu *m_drawmode = new QPopupMenu;
    m_drawmode->setCheckable( TRUE );
    m_drawmode->insertItem(i18n("Plain"),this,SLOT(options_drawPlain()));
    m_drawmode->setId(0,1);
    m_drawmode->insertItem(i18n("Icon"),this,SLOT(options_drawIcon()));
    m_drawmode->setId(1,2);
    m_drawmode->insertItem(i18n("Pixmap"),this,SLOT(options_drawPixmap()));
    m_drawmode->setId(2,3);
    m_drawmode->setItemChecked(1,(kpagerclient->getDrawMode()==0)?TRUE:FALSE);
    m_drawmode->setItemChecked(2,(kpagerclient->getDrawMode()==1)?TRUE:FALSE);
    m_drawmode->setItemChecked(3,(kpagerclient->getDrawMode()==2)?TRUE:FALSE);
    
    m_options->insertItem(i18n("Draw Mode"),m_drawmode);
    
    return m_options;
    
}

QPopupMenu *KPager::getToDesktoplikeMenu(int mark)
{
    QPopupMenu *menu=new QPopupMenu;
    menu->setCheckable( TRUE );
    for (int i=1;i<=kpagerclient->getNumberOfDesktops();i++)
    {
        menu->insertItem(kpagerclient->getDesktopName(i));
        if (i!=mark)
            menu->setItemChecked(i-1, FALSE);
        else
            menu->setItemChecked(i-1, TRUE);
    }
    connect(menu,SIGNAL(activated(int)),this,SLOT(toDesktop(int)));
    return menu;
}

void KPager::showPopupMenu(Window w,QPoint p)
{
    QPopupMenu *menu;
    QPopupMenu *moptions=getOptionlikeMenu();
    selectedWindow=w;
    if (w==0) 
    {
        moptions->exec(p);
    }
    else 
    {
        menu=new KPopupMenu(KWM::title(w),NULL, "KPagerPopupMenu");
        menu->insertItem(i18n("&Maximize"));    
        menu->insertItem(i18n("&Iconify"));    
        if (KWM::isSticky(w))
            menu->insertItem(i18n("&UnSticky"));    
        else
            menu->insertItem(i18n("&Sticky"));    
        
        menu->insertItem(i18n("&To Desktop"),getToDesktoplikeMenu(KWM::desktop(w)));
        menu->insertItem(i18n("&Close"));    
        menu->insertSeparator();
        menu->insertItem(i18n("&Options"),moptions);
        connect(menu,SIGNAL(activated(int)),this,SLOT(windowOperations(int)));
        menu->exec(p,KPM_FirstItem);
        delete menu;
    }
    delete moptions;
}

void KPager::windowOperations(int id)
{
    switch (id)
    {
    case (3) : KWM::setMaximize(selectedWindow, true);break;
    case (4) : KWM::setIconify(selectedWindow, true);break;
    case (5) :
        if (!KWM::isSticky(selectedWindow))
        {
            KWM::moveToDesktop(selectedWindow,KWM::currentDesktop());
            KWM::setSticky(selectedWindow, true);break;
        } else {
            KWM::setSticky(selectedWindow, false);break;
        };
    case (7) : KWM::close(selectedWindow);break;
    };    
}

void KPager::toDesktop(int id)
{
    KWM::moveToDesktop(selectedWindow,id+1);
}
