/*
  toplevel.cpp - the main view of the KDE control center

  written 1997 by Matthias Hoelzer
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */


#include <unistd.h>
#include <qpixmap.h>
#include <qpushbt.h>
#include <kiconloader.h>
#include <kmsgbox.h>

#include "toplevel.moc"


TopLevel::TopLevel (ConfigList *cl)
  : KTopLevelWidget(), ID_GENERAL(1)
{
  configList = cl;
  current = 0;

  setupMenuBar();
  setupStatusBar();
  
  panner = new KPanner(this, "panner", KPanner::U_ABSOLUTE | KPanner::O_VERTICAL, 200);
  panner->resize(400,100);
  panner->setAbsSeparator(200);
  
  treelist = new KTreeList(panner->child0());
  configList->fillTreeList(treelist);

  mwidget = new mainWidget(panner->child1());

  connect(treelist, SIGNAL(selected(int)), this, SLOT(item_selected(int)));
  connect(treelist, SIGNAL(highlighted(int)), this, SLOT(item_highlighted(int)));
  connect(panner, SIGNAL(positionChanged()), this, SLOT(pannerChanged()));
  
  setView(panner);
  
  setMinimumSize(450,200);
  
  resize(680,530);
  show();
  resizeEvent(NULL);

  KConfig *config = kapp->getConfig();
  config->setGroup("Options");
  KModuleListEntry::swallowingEnabled = config->readNumEntry("SwallowEnabled", TRUE);
  options->setItemChecked(swallowID, KModuleListEntry::swallowingEnabled);
}


void TopLevel::setupMenuBar()
{
    file = new QPopupMenu();
    options = new QPopupMenu();
    helpMenu = new QPopupMenu();
      
    file->insertItem(klocale->translate("E&xit"), 
		     KApplication::getKApplication(), SLOT(quit()));

    options->setCheckable(TRUE);
    swallowID = options->insertItem(klocale->translate("&Swallow modules"),
                        this, SLOT(swallowChanged()));

    helpMenu->insertItem(klocale->translate("&About KControl..."), 
			 this, SLOT(about()));
    helpMenu->insertSeparator(-1);
    helpID = helpMenu->insertItem(klocale->translate("&Using KControl"), 
				  this, SLOT(help()));
    
    menubar = new KMenuBar(this);
    menubar->insertItem(klocale->translate("&File"), file);
    menubar->insertItem(klocale->translate("&Options"), options);
    menubar->insertSeparator(-1);
    menubar->insertItem(klocale->translate("&Help"), helpMenu);

    setMenu(menubar);
}
 

void TopLevel::about()
{
    KMsgBox::message(this, klocale->translate("About KControl"),
		     klocale->translate("KDE Control Center - "
					"Version 0.3\n\n"
					"Written by Matthias Hölzer\n"
					"(hoelzer@physik.uni-wuerzburg.de)\n\n"
					"Thanks to:\n"
					"S. Kulow, P. Dowler, M. Wuebben & M. Jones."),
		     KMsgBox::INFORMATION, klocale->translate("Close"));
}


void TopLevel::setupStatusBar()
{
  statusbar = new KStatusBar(this);
  statusbar->insertItem("", ID_GENERAL);
  statusbar->setInsertOrder(KStatusBar::LeftToRight);
  setStatusBar(statusbar);
}  


void TopLevel::pannerChanged()
{
  treelist->resize(panner->child0()->width(), panner->child0()->height());
  
  if (mwidget)
    mwidget->resize(width()-panner->getAbsSeparator(), height()); 

  if (KModuleListEntry::visibleWidget)
    KModuleListEntry::visibleWidget->resize(panner->child1()->width(), panner->child1()->height());
}


void TopLevel::resizeEvent(QResizeEvent *)
{
  updateRects();
  
  treelist->resize(panner->child0()->width(), panner->child0()->height());
  
  if (mwidget)
    mwidget->resize(width()-panner->getAbsSeparator(), height()); 

  if (KModuleListEntry::visibleWidget)
    KModuleListEntry::visibleWidget->resize(panner->child1()->width(), panner->child1()->height());
}


void TopLevel::item_selected(int item)
{
  KModuleListEntry *listEntry = getListEntry(item);
  
  if (listEntry)
    if (listEntry->isDirectory())
      treelist->expandOrCollapseItem(item);
    else
      listEntry->execute(panner->child1());
}


void TopLevel::item_highlighted(int item)
{
  KModuleListEntry *listEntry = getListEntry(item);
  QString          hint;
  
  if (listEntry)
    hint = listEntry->getComment();
  
  statusbar->changeItem(hint.data(), ID_GENERAL);

  if (listEntry && !listEntry->isDirectory())
    listEntry->execute(panner->child1());
}


KModuleListEntry *TopLevel::getListEntry(int item)
{
  ConfigTreeItem *list_item;

  list_item = (ConfigTreeItem *) treelist->itemAt(item);
  
  if (list_item == NULL)
    return NULL;
  else
    return list_item->moduleListEntry; 
}


void TopLevel::help()
{
  kapp->invokeHTMLHelp("kcontrol/index.html", "" );    
}


void TopLevel::swallowChanged()
{
  KModuleListEntry::swallowingEnabled = !KModuleListEntry::swallowingEnabled;

  KConfig *config = kapp->getConfig();
	
  config->setGroup("Options");
  config->writeEntry("SwallowEnabled", KModuleListEntry::swallowingEnabled);

  options->setItemChecked(swallowID, KModuleListEntry::swallowingEnabled);
  config->sync();
}

