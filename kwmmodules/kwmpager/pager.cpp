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
#include <qfont.h>
#include "desktop.h"
#include <kwm.h>
#include <unistd.h>
#include <stdlib.h>

const char *Pager::PosStrings[] = {"TopRight", "TopLeft", "BottomRight",
				   "BottomLeft" };

Pager::Pager(KWMModuleApplication *a, char *name) : QWidget(NULL,  "kwmpager")
{
    kwmmapp = a;
    a->setMainWidget(this);
    kwmmapp -> connectToKWM();
    KWM::setSticky(winId(), true);
    KWM::setDecoration(winId(), KWM::tinyDecoration | KWM::noFocus);
    KWM::setWmCommand(winId(), name);
    KWM::setUnsavedDataHint(winId(), false);

    int count = KWM::numberOfDesktops();
    desktop_font = new QFont();
    Desktop *desk;

    desktops.resize(count);
    for (int i = 0; i < count; i++) {
        desk = new Desktop(a, i + 1, this);
        desktops[i] = desk;
    }
    
    activeDesktop = desktops.at(KWM::currentDesktop() - 1);
    activeDesktop->activate(true);
    
    connect(kwmmapp, SIGNAL(desktopChange(int)) , 
	    SLOT(changeDesktop(int)));
    connect(kwmmapp, SIGNAL(init()), 
	    SLOT(initDesktops()));
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
    connect(kwmmapp, SIGNAL( commandReceived(QString)),
	    SLOT(receiveCommand(QString)));

    moved = false;
    readSettings();
    initDesktops();
    show();
    placeIt();
}

Pager::~Pager()
{
    delete desktop_font;
}

void Pager::receiveCommand(QString command)
{
    if (command == "pager:config") {
	readSettings();
	initDesktops();
	placeIt();
    }
}

void Pager::readSettings()
{
    KConfig *config = kapp->getConfig();
    config->reparseConfiguration();
    config->setGroup("GUI Settings");

    // Size
    QString entry = config->readEntry("Size", "100 100");
    QTextStream str(entry, IO_ReadOnly);
    int w, h;
    str >> w >> h;
    entry.sprintf("%d %d",w,h);
    config->writeEntry("Size", entry);
    resize(w,h);

    // Geometry
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
    
    // Font
    entry = config->readEntry("Font", 
       // don't blame me, I've played with xfontsel ;)
       "-bitstream-courier-medium-i-normal-*-10-*-75-75-m-54-iso8859-*");
    desktop_font->setRawMode(TRUE);
    desktop_font->setFamily(entry);
}

void Pager::placeIt()
{
    QRect rect = KWM::getWindowRegion(KWM::currentDesktop());
    switch (position) {
    case TopRight:
	KWM::move(winId(), rect.topRight() - QPoint(width(), 0) + QPoint(-4,0));
	break;
    case TopLeft:
	KWM::move(winId(), rect.topLeft());
	break;
    case BottomRight:
	KWM::move(winId(), rect.bottomRight() - QPoint(width(), height()) + QPoint(-4,-4));
	break;
    case BottomLeft:
	KWM::move(winId(), rect.bottomLeft() - QPoint(0, height()) + QPoint(0,-4));
	break;
    case Costumized:
	move(posx ,posy);
    }
}

void Pager::changeDesktop(int index) 
{
    Desktop *old = activeDesktop;
    activeDesktop->activate(false);
    activeDesktop = desktops.at(index - 1);
    activeDesktop->activate(true);
    PagerWindow *win;
    for (win = stickys.first(); win ; win = stickys.next()) {
	old -> removeWindow(win->id);
	activeDesktop -> addWindow(win);
    }	
}

void Pager::resizeEvent ( QResizeEvent * )  
{
    int count = KWM::numberOfDesktops();
    QSize desktop_size(width() / (count / 2), height() / 2);
    
    Desktop *desk;
    for (uint i=0; i < desktops.size() ; i++) {
	desk = desktops[i];
        desk->resize(desktop_size);
        desk->move(desktop_size.width() * (i / 2), 
		   (i % 2) ? height() / 2 : 0);
    }
    QString size;
    size.sprintf("%d %d",width(), height());
    KConfig *config = kapp->getConfig();
    config->setGroup("GUI Settings");
    config->writeEntry("Size",size);
    config->sync();
}
 
void Pager::initDesktops()
{
    uint i;
    for (i=0; i < desktops.size(); i++) {
	desktops[i]->init();
	desktops[i]->setFont(*desktop_font);
    }
    for (Window *w = kwmmapp->windows_sorted.first(); w != 0L; 
	 w = kwmmapp->windows_sorted.next()) {
	desktops[KWM::desktop(*w) - 1] -> addWindow(*w);
    }
    for (i=0; i < desktops.size(); i++) {
	desktops[i]->repaint(false);
	int l = i - 2;
	uint r = i + 2;
	int t = i - 1;
	uint b = i + 1;
	desktops[i]->setNeighbours(l >= 0 ? desktops[l] : 0,
				   r < desktops.size() ? desktops[r] : 0,
				   t >= 0 ? desktops[t] : 0,
				   b < desktops.size() ? desktops[b] : 0);
    }
}

void Pager::changeNumber(int)
{
    uint i;
    windowActivate(0); // clear Desktop With Active Window
    for (i = 0; i < desktops.size(); i++)
	delete desktops[i];

    uint count = KWM::numberOfDesktops();
    desktops.resize(count);
    for (i = 0; i < count; i++) {
	desktops[i] = new Desktop(kwmmapp, i + 1, this);
	desktops[i]->show();
    }
    
    activeDesktop = desktops[KWM::currentDesktop() - 1];
    activeDesktop->activate(true);
    
    resizeEvent(0);
    initDesktops();
    repaint(true);
}

void Pager::moveEvent ( QMoveEvent *) {
    if (moved) {
	QString pos;
	QPoint position = KWM::geometry(winId(), false).topLeft();
	pos.sprintf("%d %d",position.x(), position.y());
	KConfig *config = kapp->getConfig();
	config->setGroup("GUI Settings");
	config->writeEntry("Geometry",pos);
	config->sync();
    }
    moved = true;
}

void Pager::closeEvent( QCloseEvent *)
{
    kapp->quit();
}

void Pager::addWindow(Window w)
{ 
    // debug("add %lx",w);
    PagerWindow *win = new PagerWindow;
    win->id = w;
    
    if (KWM::isSticky(w)) {
	stickys.append(win);
	desktops.at(KWM::desktop(w) - 1 )->addWindow(win);
    } else
	desktops.at(KWM::desktop(w) - 1 ) -> addWindow(w);
}

void Pager::removeWindow(Window w)
{
    // debug("remove %lx",w);
    PagerWindow *win;
    for (win = stickys.first(); win && win->id != w; win = stickys.next())
	continue;

    if (win)
	stickys.remove();

    for (uint i = 0; i < desktops.size(); i++)
	desktops[i]->removeWindow( w );

    if (win)
	delete win;
}

void Pager::windowChange(Window w)
{
    Desktop *current = desktops.at(KWM::desktop(w) - 1);
    // debug("change %lx",w);
    PagerWindow *win;
    for (win = stickys.first(); win && win->id != w; win = stickys.next())
	continue;

    if (!KWM::isSticky(w)) 
	stickys.remove();
    else {
	if (!win) {
	    win = current->getWindow(w);
	    if (win)
		stickys.append(win);
	}
    }
	
    if ( current->getWindow(w) == 0L ) {
	for (uint i = 0; i < desktops.size(); i++)
	    desktops[i]->removeWindow(w);
	current->addWindow(w);
    } 
    desktops[KWM::desktop(w) - 1]->changeWindow(w);
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

    static Desktop *desktopWithActiveWindow = 0L;
 
    if (w == 0L) {
	desktopWithActiveWindow = 0L;
	return;
    }

    if (desktopWithActiveWindow)
	desktopWithActiveWindow -> activateWindow( 0L );

    desktopWithActiveWindow = desktops.at(KWM::desktop(w) - 1);
    desktopWithActiveWindow -> activateWindow( w );
}

int main( int argc, char *argv[] )
{
    KWMModuleApplication a (argc, argv);
    if (!KWM::isKWMInitialized()){
	printf(klocale->translate("kwmpager: waiting for windowmanager\n"));
	while (!KWM::isKWMInitialized()) sleep(1);
    }   
    if (argc > 1){
	if (QString("-version") == argv[1]){
	    printf(KWMPAGER_VERSION);
	    printf("\n");
	    printf(klocale->translate("Copyright (C) 1997 Stephan Kulow (coolo@kde.org)\n"));
	    ::exit(0);
	}
	else {
	    printf(klocale->translate("Usage:"));
	    printf("%s [-version]\n", argv[0]);
	}
	::exit(1); 
    }
    Pager p(&a, argv[0]);
    return a.exec();
}
