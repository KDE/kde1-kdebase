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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pager.moc"
#include "version.h"
#include "desktop.h"
#include <kwm.h>
#include <unistd.h>

const char *Pager::PosStrings[] = {"TopRight", "TopLeft", "BottomRight",
				   "BottomLeft" };

Pager::Pager(KWMModuleApplication *a) : QFrame(NULL, "kwmpager",
						WStyle_Customize | 
						WStyle_NoBorder | 
						WStyle_Tool)
{
    kwmmapp = a;
    kwmmapp -> connectToKWM();
    KWM::setSticky(winId(), true);

    int count = KWM::numberOfDesktops();
    desktops.setAutoDelete(true);

    for (int i = 0; i < count; i++) {
        Desktop *desk = new Desktop(a, i + 1, this);
        desktops.append(desk);
    }
    
    activeDesktop = desktops.at(KWM::currentDesktop() - 1);
    activeDesktop->activate(true);
    
    connect(kwmmapp, SIGNAL(desktopChange(int)) , SLOT(changeDesktop(int)));
    connect(kwmmapp, SIGNAL(init()), SLOT(initDesktops()));
    connect(kwmmapp, SIGNAL( desktopNumberChange(int)), 
	    SLOT(changeNumber(int)));
    connect(kwmmapp, SIGNAL( windowAdd(Window)), 
	    SLOT(addWindow(Window)));
    connect(kwmmapp, SIGNAL( windowRemove(Window)), 
	    SLOT(removeWindow(Window)));
    connect(kwmmapp, SIGNAL( windowChange(Window)), 
	    SLOT(windowChange(Window)));
    connect(kwmmapp, SIGNAL( windowRaise(Window)), 
	    SLOT(raiseWindow(Window)));
    connect(kwmmapp, SIGNAL( windowLower(Window)), 
	    SLOT(lowerWindow(Window)));
    connect(kwmmapp, SIGNAL( windowActivate(Window)), 
	    SLOT(windowActivate(Window)));
    
    readSettings();
    initDesktops();
    show();
    placeIt();
}

void Pager::readSettings()
{
    KConfig *config = kapp->getConfig();
    config->setGroup("Geometry");
    QString entry = config->readEntry("Size", "100 100");
    QTextStream str(entry, IO_ReadOnly);
    int w, h;
    str >> w >> h;
    entry.sprintf("%d %d",w,h);
    config->writeEntry("Size", entry);
    entry = config->readEntry("Geometry", PosStrings[0]);
    position = Costumized;
    for (int i=0; i<5;i++)
      if (entry == PosStrings[i])
        position = (Position)i;
    
    if (position == Costumized) {
      QTextStream s(entry, IO_ReadOnly);
      s >> posx >> posy;
      entry.sprintf("%d %d",pos, posy);
    }
    config->writeEntry("Geometry", entry);
    config->sync();
    resize(w,h);
}

void Pager::placeIt()
{
    QRect rect = KWM::getWindowRegion(KWM::currentDesktop());
    switch (position) {
    case TopRight:
	KWM::move(winId(), rect.topRight() - QPoint(width(), 0));
	break;
    case TopLeft:
	KWM::move(winId(), rect.topLeft());
	break;
    case BottomRight:
	KWM::move(winId(), rect.bottomRight() - QPoint(width(), height()));
	break;
    case BottomLeft:
	KWM::move(winId(), rect.bottomLeft() - QPoint(0, height()));
	break;
    case Costumized:
	move(posx ,posy);
    }
}

void Pager::changeDesktop(int index) 
{
    activeDesktop->activate(false);
    activeDesktop = desktops.at(index - 1);
    activeDesktop->activate(true);
}

void Pager::resizeEvent ( QResizeEvent * )  
{
    int count = KWM::numberOfDesktops();
    QSize desktop_size(width() / (count / 2), height() / 2);
    
    Desktop *desk;
    int i;
    for (desk = desktops.first(), i=0; desk ; desk = desktops.next(), i++) {
        desk->resize(desktop_size);
        desk->move(desktop_size.width() * (i / 2), 
		   (i % 2) ? height() / 2 : 0);
    }
}

void Pager::initDesktops()
{
    for (Desktop *desk = desktops.first(); desk != 0L;
         desk = desktops.next())
	desk->init();
    for (Window *w = kwmmapp->windows_sorted.first(); w != 0L; 
	 w = kwmmapp->windows_sorted.next()) {
	if (!KWM::isIconified(*w))
	    desktops.at(KWM::desktop(*w) - 1) -> addWindow(*w);
    }
    for (Desktop *desk = desktops.first(); desk != 0L;
	 desk = desktops.next())
	desk->repaint(false);
}

void Pager::changeNumber(int)
{
    desktops.clear();
    int count = KWM::numberOfDesktops();
    
    for (int i = 0; i < count; i++) {
	Desktop *desk = new Desktop(kwmmapp, i + 1, this);
	desktops.append(desk);
	desk->show();
    }
    
    activeDesktop = desktops.at(KWM::currentDesktop() - 1);
    activeDesktop->activate(true);
    
    resizeEvent(NULL);
    initDesktops();
    repaint(true);
}

void Pager::addWindow(Window w)
{ 
    // debug("add %lx",w);
    desktops.at(KWM::desktop(w) - 1 ) -> addWindow(w);
}

void Pager::removeWindow(Window w)
{
    // debug("remove %lx",w);
    desktops.at(KWM::desktop(w) - 1) -> removeWindow(w);
}

void Pager::windowChange(Window w)
{
    // debug("change %lx",w);
    desktops.at(KWM::desktop(w) - 1)->changeWindow(w);
}

void Pager::raiseWindow(Window w)
{
    // debug("raise %lx",w);
    desktops.at(KWM::desktop(w) - 1)->raiseWindow(w);
}

void Pager::lowerWindow(Window w)
{
    // debug("lower %lx",w);
    desktops.at(KWM::desktop(w) - 1)->lowerWindow(w);
}

void Pager::windowActivate(Window w)
{
    // debug("activate %lx",w);
    desktops.at(KWM::desktop(w) - 1) -> activateWindow(w);
}

int main( int argc, char *argv[] )
{
    KWMModuleApplication a (argc, argv);
    if (!KWM::isKWMInitialized()){
	printf("kwmpager: waiting for windowmanager\n");
	while (!KWM::isKWMInitialized()) sleep(1);
    }   
    if (argc > 1){
	if (QString("-version") == argv[1]){
	    printf(KWMPAGER_VERSION);
	    printf("\n");
	    printf("Copyright (C) 1997 Stephan Kulow (coolo@kde.org)\n");
	    ::exit(0);
	}
	else {
	    printf("Usage:");
	    printf("%s [-version]\n", argv[0]);
	}
	::exit(1); 
    }
    Pager p(&a);
    return a.exec();
}
