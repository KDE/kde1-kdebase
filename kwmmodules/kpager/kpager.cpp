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
#include "kpager.moc"

#include <stdio.h>
#include <qstring.h>
#include <unistd.h>
#include <qkeycode.h>
#include <qmessagebox.h>
#include <kaccel.h>
#include "version.h"
//#include <X11/Xlib.h>
    
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

    kKeysAccel->readSettings();

    m_file = new QPopupMenu;
    m_file->insertItem( i18n("&Quit"), kapp, SLOT(quit()), CTRL+Key_Q );
    m_options = new QPopupMenu;
    if (kpagerclient->isVisibleGlobalDesktop())
        m_options->insertItem(i18n("Hide &Global Desktop"), this, SLOT(options_toggleGlobalDesktop()), Key_0 , 1 );
    else
        m_options->insertItem(i18n("Show &Global Desktop"), this, SLOT(options_toggleGlobalDesktop()), Key_0 , 1 );

    kKeysAccel->changeMenuAccel(m_options,1, "Toggle Global Desktop");
        
    m_options->insertItem(i18n("Hide &Menubar"), this, SLOT(options_toggleMenuBar()), Key_Space , 2 );
    kKeysAccel->changeMenuAccel(m_options,2, "Toggle Menubar");
    m_options->insertSeparator();
    m_drawmode = new QPopupMenu;
    m_drawmode->setCheckable( TRUE );
    m_drawmode->insertItem(i18n("Plain"),this,SLOT(options_drawPlain()));
    m_drawmode->setId(0,1);
    m_drawmode->setItemChecked(1,TRUE);
    m_drawmode->insertItem(i18n("Icon"),this,SLOT(options_drawIcon()));
    m_drawmode->setId(1,2);
    m_drawmode->setItemChecked(2,FALSE);
    m_drawmode->insertItem(i18n("Pixmap"),this,SLOT(options_drawPixmap()));
    m_drawmode->setId(2,3);
    m_drawmode->setItemChecked(3,FALSE);

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
//    setGeometry(x(),y(),380,140);
};

KPager::~KPager()
{
#ifdef KPAGERDEBUG
    printf("KPager:: destructor\n");
#endif

    delete m_file;
    delete m_options;
    delete m_drawmode;
    delete m_help;
    delete menu;
#ifdef KPAGERDEBUG
    printf("KPager:: destructor(end)\n");
#endif

};

void KPager::options_toggleGlobalDesktop()
{
    if (kpagerclient->isVisibleGlobalDesktop())
    {
        m_options->changeItem(i18n("Show &Global Desktop"), 1);
        kpagerclient->setVisibleGlobalDesktop(false);
    }
    else
    {
        kpagerclient->setVisibleGlobalDesktop(true);
        m_options->changeItem(i18n("Hide &Global Desktop"), 1);
    }
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

}

void KPager::options_drawPlain()
{
    m_drawmode->setItemChecked(1, TRUE);
    m_drawmode->setItemChecked(2, FALSE);
    m_drawmode->setItemChecked(3, FALSE);
    
    kpagerclient->setDrawMode(0);
}

void KPager::options_drawIcon()
{
    m_drawmode->setItemChecked(1, FALSE);
    m_drawmode->setItemChecked(2, TRUE);
    m_drawmode->setItemChecked(3, FALSE);
    
    kpagerclient->setDrawMode(1);
}

void KPager::options_drawPixmap()
{
    m_drawmode->setItemChecked(1, FALSE);
    m_drawmode->setItemChecked(2, FALSE);
    m_drawmode->setItemChecked(3, TRUE);
    
    kpagerclient->setDrawMode(2);

}


void KPager::saveProperties(KConfig *kcfg)
{
    kcfg->writeEntry("visibleMenuBar",menubar_visible);
    kcfg->writeEntry("visibleGlobalDesktop",kpagerclient->isVisibleGlobalDesktop());
    kcfg->writeEntry("drawMode",kpagerclient->getDrawMode());
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
}
