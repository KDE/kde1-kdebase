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


void MySplitter::resizeEvent(QResizeEvent *event)
{
  QSplitter::resizeEvent(event);

  emit resized();
}


TopLevel::TopLevel (ConfigList *cl)
  : KTopLevelWidget(), ID_GENERAL(1)
{
  configList = cl;
  current = 0;

  setupMenuBar();
  setupStatusBar();

  splitter = new MySplitter(this);
  connect(splitter, SIGNAL(resized()), this, SLOT(doResize()));

  treelist = new KTreeList(splitter);
  configList->fillTreeList(treelist);
  treelist->setMinimumWidth(200);
  splitter->setResizeMode(treelist,QSplitter::KeepSize);

  mwidget = new mainWidget(splitter);
  connect(mwidget, SIGNAL(resized()), this, SLOT(doResize()));

  connect(treelist, SIGNAL(selected(int)), this, SLOT(item_selected(int)));
  connect(treelist, SIGNAL(singleSelected(int)), this, SLOT(item_singleSelected(int)));

  setView(splitter);
  setMinimumSize(450,200);

  resize(700,600);

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

    file->insertItem(klocale->translate("E&xit"),
		     KApplication::getKApplication(), SLOT(quit()));

    options->setCheckable(TRUE);
    swallowID = options->insertItem(klocale->translate("&Swallow modules"),
                        this, SLOT(swallowChanged()));

    QPopupMenu *helpMenu = kapp->getHelpMenu(true, klocale->translate("KDE Control Center - "
					"Version 1.0\n\n"
					"Written by Matthias Hölzer\n"
					"(hoelzer@physik.uni-wuerzburg.de)\n\n"
					"Thanks to:\n"
					"S. Kulow, P. Dowler, M. Wuebben & M. Jones."));

    menubar = new KMenuBar(this);
    menubar->insertItem(klocale->translate("&File"), file);
    menubar->insertItem(klocale->translate("&Options"), options);
    menubar->insertSeparator(-1);
    menubar->insertItem(klocale->translate("&Help"), helpMenu);

    setMenu(menubar);
}


void TopLevel::setupStatusBar()
{
  statusbar = new KStatusBar(this);
  statusbar->insertItem("", ID_GENERAL);
  statusbar->setInsertOrder(KStatusBar::LeftToRight);
  setStatusBar(statusbar);
}


void TopLevel::resizeEvent(QResizeEvent *)
{
  updateRects();

  doResize();
}


void TopLevel::doResize()
{
  if (KModuleListEntry::visibleWidget)
    KModuleListEntry::visibleWidget->resize(mwidget->width(), mwidget->height());
}


void TopLevel::item_selected(int item)
{
  KModuleListEntry *listEntry = getListEntry(item);

  if (listEntry)
    if (listEntry->isDirectory())
      treelist->expandOrCollapseItem(item);
    else
      listEntry->execute(mwidget);
}


void TopLevel::item_singleSelected(int item)
{
  KModuleListEntry *listEntry = getListEntry(item);
  QString          hint;

  if (listEntry)
    hint = listEntry->getComment();

  statusbar->changeItem(hint.data(), ID_GENERAL);

  if (listEntry && !listEntry->isDirectory())
    listEntry->execute(mwidget);
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


void TopLevel::swallowChanged()
{
  KModuleListEntry::swallowingEnabled = !KModuleListEntry::swallowingEnabled;

  KConfig *config = kapp->getConfig();
	
  config->setGroup("Options");
  config->writeEntry("SwallowEnabled", KModuleListEntry::swallowingEnabled);

  options->setItemChecked(swallowID, KModuleListEntry::swallowingEnabled);
  config->sync();
}


void TopLevel::ensureSize(int w, int h)
{
  int width=w, height=h;

  if (w < mwidget->width())
    width = mwidget->width();
  if (h < mwidget->height())
    height = mwidget->height();

  width += treelist->width() + 6; // 6 ~= width of QSplitter slider
  height += menubar->height()+statusbar->height();

  if (width < this->width())
    width = this->width();
  if (height < this->height())
    height = this->height();

  resize(width, height);
}