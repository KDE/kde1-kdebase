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
    
    QObject::connect(kwmmapp,SIGNAL(desktopChange(int)),
                     kpagerclient,SLOT(desktopChanged(int)));
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


    m_file = new QPopupMenu;
    m_file->insertItem(i18n("&Open..."), this, SLOT(file_Open()), CTRL+Key_O );
    m_file->insertSeparator();
    m_file->insertItem( i18n("&Quit"), kapp, SLOT(quit()), CTRL+Key_Q );
    m_options = new QPopupMenu;
    if (kpagerclient->isVisibleGlobalDesktop())
        m_options->insertItem(i18n("Hide &Global Desktop"), this, SLOT(options_toggleGlobalDesktop()), Key_0 , 1 );
    else
        m_options->insertItem(i18n("Show &Global Desktop"), this, SLOT(options_toggleGlobalDesktop()), Key_0 , 1 );
        
    m_options->insertItem(i18n("Hide &Menubar"), this, SLOT(options_toggleMenuBar()), Key_Space , 2 );

    char aboutstring[500];
    sprintf(aboutstring,
            i18n("%s\n\n" \
                 "(C) 1998 Antonio Larrosa Jimenez\n"
                 "larrosa@kde.org\n"
                 "antlarr@arrakis.es\n" \
                 "Malaga (Spain)\n\n" \
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
    setGeometry(x(),y(),380,140);
};

KPager::~KPager()
{
#ifdef KPAGERDEBUG
    printf("KPager:: destructor\n");
#endif

    delete m_file;
    delete m_options;
    delete m_help;
    delete menu;
#ifdef KPAGERDEBUG
    printf("KPager:: destructor(end)\n");
#endif

};

void KPager::file_Open()
{
//    QMessageBox::about( this , kapp->getCaption(), "Hello");
    printf("Damnit Jim, I'm a pager, not a text editor ! :-)\n");
}

void KPager::options_toggleGlobalDesktop()
{
    if (kpagerclient->isVisibleGlobalDesktop())
    {
        printf("a");
        m_options->changeItem(i18n("Show &Global Desktop"), 1);
        kpagerclient->setVisibleGlobalDesktop(false);
    }
    else
    {
        printf("b");
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
