// -*- C++ -*-

//
//  This file is part of KPanel
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@bonn.netsurf.de
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

/*
 * Several modifications and extensions by Matthias Ettrich <ettrich@kde.org>
 */

#include <qdstream.h>
#include <qtstream.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qapp.h>
#include <qfont.h>
#include <qdir.h>
#include <qpixmap.h>

#include <kapp.h>
#include <kiconloader.h>
#include <kfm.h>
#include <kmsgbox.h>

#include <ksimpleconfig.h>

#include "pmenu.h"
#include "pmenu.moc"

extern "C" {
#include <stdlib.h>
}

static int global_id = 1;


bool ready_for_event_loop = false;

QString personal;
QString kde_apps;

static void getPaths(){
  if (!personal) {
    KConfig* config = kapp->getConfig();
    config->setGroup("KDE Desktop Entries");
    QString temp = KApplication::localkdedir()+"/share/applnk";
    personal = config->readEntry("PersonalPath", temp.data() );
    temp = KApplication::kde_appsdir();
    kde_apps = config->readEntry("Path", temp.data() );
  }
}


bool isKdelnkFile(const char* filename){

  QFile file(filename);
  if (file.open(IO_ReadOnly)){
    char s[65];
    int r = file.readBlock(s, 64);
    file.close();

    if(r == -1)
      return false;

    // terminate string
    s[r] = '\0';

    return ( QString(s).find("[KDE Desktop Entry]", 0, false) != -1 );
  }

  return false;
}

PMenuItem::PMenuItem()
{
  initMetaObject();
  entry_type = empty;
  sub_menu = 0;
  cmenu = 0;
  recv = 0;
  memb = 0;
  id = global_id++;
}

PMenuItem::PMenuItem( EntryType e, QString t, QString c, QString n,
		      PMenu *menu, QObject *receiver, char *member,
		      QPopupMenu *cm, bool /* ro */, QString d, QString co )
{
  initMetaObject();
  entry_type = e;
  text_name = t;
  command_name = c;
  pixmap_name = n;
  if( !pixmap_name.isEmpty() )
    {
      pixmap = KApplication::getKApplication()->getIconLoader()->loadApplicationMiniIcon( pixmap_name , 16, 16);
    }
  else
    {
	// pixmap = 0;
	pixmap = QPixmap();
    }
  sub_menu = menu;
  cmenu = (myPopupMenu *) cm;
  recv = receiver;
  memb = member;
  dir_path = d.copy();
  comment = co;
  if (comment.isEmpty())
    comment = text_name;
  id = global_id++;
}

PMenuItem::PMenuItem( PMenuItem &item )
{
  initMetaObject();
  text_name       = item.text_name;
  pixmap_name     = item.pixmap_name;
  big_pixmap_name = item.big_pixmap_name;
  pixmap          = item.pixmap;
  entry_type      = item.entry_type;
  command_name    = item.command_name;
  comment         = item.comment;
  dir_path        = item.dir_path;
  real_name = item.real_name;
  if( entry_type == submenu )
    {
      sub_menu = new PMenu( *(item.sub_menu) );
      cmenu    = new myPopupMenu;
    }
  else
    {
      sub_menu = 0;
      cmenu    = 0;
    }
  recv         = item.cmenu;
  memb         = item.memb;
  if( entry_type == prog_com )
    {
      entry_type = unix_com;
      recv = 0;
      memb = 0;
    }
  id = global_id++;
}

PMenuItem::~PMenuItem()
{
  if( cmenu )
    delete cmenu;
  if( sub_menu )
    {
      delete sub_menu;
    }
}

short PMenuItem::parse( QFileInfo *fi, PMenu *menu)
{
  if (ready_for_event_loop)
    KApplication::getKApplication()->processEvents();

  real_name = fi->fileName().copy();
  int pos = fi->fileName().find(".kdelnk");
  if( pos >= 0 )
    text_name = fi->fileName().left(pos);
  else
    text_name = fi->fileName();
  if( menu != 0 )
    {
      QString file = fi->absFilePath();
      dir_path = fi->dirPath();
      file += "/.directory";
      QFile config(file);
      if( config.exists() )
	{
	  KSimpleConfig kconfig(file, true);
	  kconfig.setGroup("KDE Desktop Entry");
	  pixmap_name = kconfig.readEntry("MiniIcon");
	  big_pixmap_name = kconfig.readEntry("Icon");
	  comment = kconfig.readEntry("Comment");
	  text_name = kconfig.readEntry("Name", text_name);
	}
      entry_type  = submenu;
      sub_menu    = menu;
      cmenu       = new myPopupMenu;
    }
  else
    {
      KSimpleConfig kconfig(fi->absFilePath(), true);
      kconfig.setGroup("KDE Desktop Entry");
      command_name = kconfig.readEntry("Exec");
      entry_type = unix_com;
      pixmap_name     = kconfig.readEntry("MiniIcon");
      big_pixmap_name = kconfig.readEntry("Icon");
      comment         = kconfig.readEntry("Comment");
      text_name = kconfig.readEntry("Name", text_name);
      dir_path        = fi->dirPath(true);
    }
  QPixmap tmppix;
  pixmap = tmppix;
  if( !pixmap_name.isEmpty() ){
    pixmap = KApplication::getKApplication()->getIconLoader()->loadApplicationMiniIcon(pixmap_name, 16, 16);
  }
  if (pixmap.isNull() && !big_pixmap_name.isEmpty()){
    pixmap = KApplication::getKApplication()->getIconLoader()->loadApplicationMiniIcon(big_pixmap_name, 16, 16);
  }
  if (pixmap.isNull() && getType() == unix_com){
    QString tmp = real_name.copy();
    int pos = tmp.find(".kdelnk");
    if( pos >= 0 )
      tmp = tmp.left(pos);
    tmp.append(".xpm");
    pixmap = KApplication::getKApplication()->getIconLoader()->loadApplicationMiniIcon(tmp, 16, 16);
  }

  if (pixmap.isNull())
    pixmap = KApplication::getKApplication()->getIconLoader()->loadApplicationMiniIcon("mini-default.xpm", 16, 16);

  if (comment.isEmpty())
    comment = text_name;
  if (big_pixmap_name.isEmpty()){
    QString tmp = real_name.copy();
    int pos = tmp.find(".kdelnk");
    if( pos >= 0 )
      tmp = tmp.left(pos);
    tmp.append(".xpm");
    big_pixmap_name = tmp.copy();
  }

  return 0;
}

short PMenuItem::parse( QString abs_file_path )
{
  QFileInfo fi(abs_file_path);
  return parse(&fi);
}

void PMenuItem::addFolderToRecentList()
{
  PFileMenu::updateRecentFolders(getDirPath());
}


void PMenuItem::exec()
{
  QDir d(getDirPath());
  QString cpath = d.canonicalPath() + "/" + real_name;

  if (! ready_for_event_loop){ // hack: avoid doing this when the menu is not fully parsed.
      // otherwise PFileMenu::updateRecentFiles will segfault (Matthias)
      PFileMenu::updateRecentFiles(cpath);
  }

  KFM* kfm = new KFM;
  QString com = "file:";
  com.append(cpath);
  kfm->exec(com.data(),0L);
  delete kfm;
}

QString PMenuItem::getSaveName()
{
  getPaths();
  QString temp = fullPathName();
  if( temp == personal || temp == personal + "/" )
    {
      temp = "$$PERSONAL";
    }
  else if( temp.left(personal.length()) ==  personal )
    {
      // kdelnk file is in $HOME/Personal
      temp = temp.right(temp.length()-personal.length());
      temp.prepend("$$PERSONAL");
    }
  else if( temp.left(kde_apps.length()) == kde_apps )
    {
      // kdelnk file is in $KDEDIR/apps
      temp = temp.right(temp.length()-kde_apps.length());
      temp.prepend("$$KDEAPPS");
    }

  return temp;
}


//--------------------------------------------------------------------------------------

PMenu::PMenu()
{
  initMetaObject();
  list.setAutoDelete(true);
  cmenu = 0;
  altSort = false;
}

PMenu::PMenu( PMenu &menu )
{
  initMetaObject();
  cmenu = 0;
  list.setAutoDelete(true);
  PMenuItem *item, *new_item;
  for( item = menu.list.first(); item != 0; item = menu.list.next() )
    {
      new_item = new PMenuItem( *item );
      list.append(new_item);
    }
  altSort = false;
}

PMenu::~PMenu()
{
  list.clear();
}

void PMenu::createMenu( QPopupMenu *menu, kPanel *panel, bool add_button)
{
  QPixmap buffer;
  PMenuItem *item;
  EntryType et;
  cmenu = (myPopupMenu *) menu;
  menu->installEventFilter((QObject *) panel);
  connect( menu, SIGNAL(highlighted(int)), this, SLOT(highlighted(int)) );
  for( item = list.first(); item != 0; item = list.next() )
    {
      et = item->entry_type;
      switch ( et ) {
      case separator:
	menu->insertSeparator();
	continue;
      case submenu:
	if( add_button )
	  {
	    item->cmenu->setFont(menu->font());
	    item->cmenu->insertItem(item->pixmap, item->text_name, item->getId());
	    item->cmenu->connectItem(item->getId(), item, SLOT(execAddButton()) );
	    connect( item, SIGNAL(addButton(PMenuItem*)),
	    	     (QObject *) panel, SLOT(addButton(PMenuItem*)) );
	    connect( item->cmenu, SIGNAL(highlighted(int)), this, SLOT(highlighted(int)) );
	    item->cmenu->insertSeparator();
	    // create submenu
	    item->sub_menu->createMenu( item->cmenu, panel, true );
	    menu->insertItem(item->pixmap, item->text_name, item->cmenu, item->getId());
	    connect( item, SIGNAL(showToolTip(QString)), (QObject *) panel,
		     SLOT(showToolTip(QString)) );
	  }
	else
	  {
	    item->cmenu->setFont(menu->font());
	    item->sub_menu->createMenu( item->cmenu, panel );
	    menu->insertItem(item->pixmap, item->text_name, item->cmenu, item->getId());
	    connect( item, SIGNAL(showToolTip(QString)), (QObject *) panel,
		     SLOT(showToolTip(QString)) );
	  }
	continue;

      case dirbrowser:
        item->cmenu->setFont(menu->font());
        item->sub_menu->createMenu( item->cmenu, panel );
        menu->insertItem(item->pixmap, item->text_name, item->cmenu, item->getId());
        ((PFileMenu*)(item->sub_menu))->id = item->getId();

        item->cmenu->id = item->getId();

        ((PFileMenu*)(item->sub_menu))->parentItem = item;

	continue;

      case url:
	menu->insertItem(item->pixmap, item->text_name, item->getId());
	connect( item, SIGNAL(showToolTip(QString)), (QObject *) panel,
		 SLOT(showToolTip(QString)) );

	menu->connectItem(item->getId(), item, SLOT(exec()) );

	// NOTE: only files accessed through DiskNavigator menus update the
	// recent folders.

	menu->connectItem(item->getId(), item, SLOT(addFolderToRecentList()));

	continue;

      case label:
	menu->insertItem(item->pixmap, item->text_name, item->getId());
	connect( item, SIGNAL(showToolTip(QString)), (QObject *) panel,
		 SLOT(showToolTip(QString)) );
	continue;
      case unix_com:
	menu->insertItem(item->pixmap, item->text_name, item->getId());
	if( add_button )
	  {
	    menu->connectItem( item->getId(), item, SLOT(execAddButton()) );
	    connect( item, SIGNAL(addButton(PMenuItem *)), (QObject *) panel,
		     SLOT(addButton(PMenuItem*)) );
	  }
	else
	  {
	    if( !item->command_name.isEmpty() )
	      menu->connectItem( item->getId(), item, SLOT(exec()) );
	  }
	connect( item, SIGNAL(showToolTip(QString)), (QObject *) panel,
		 SLOT(showToolTip(QString)) );
	continue;
      case prog_com:
	menu->insertItem(item->pixmap, item->text_name, item->getId());
	if( add_button )
	  {
	    menu->connectItem( item->getId(), item, SLOT(execAddButton()) );
	    connect( item, SIGNAL(addButton(PMenuItem *)), (QObject *) panel,
		     SLOT(addButton(PMenuItem*)) );
	  }
	else
	  menu->connectItem(item->getId(), item->recv, item->memb);
	connect( item, SIGNAL(showToolTip(QString)), (QObject *) panel,
		 SLOT(showToolTip(QString)) );
	continue;
      case add_but:
	item->cmenu->setFont(menu->font());
	// create submenu
	item->sub_menu->createMenu( item->cmenu, panel, true );
	menu->insertItem(item->pixmap, item->text_name, item->cmenu, item->getId());
	item->cmenu->installEventFilter((QObject *) panel);
	connect( item, SIGNAL(showToolTip(QString)), (QObject *) panel,
		 SLOT(showToolTip(QString)) );
	connect( item->cmenu, SIGNAL(highlighted(int)), this,
		 SLOT(highlighted(int)) );
	continue;
      case empty:
	continue;
      };
    }
  return;
}

void PMenu::add (PMenuItem *item)
{
  list.append(item);
}

void PMenu::insert( PMenuItem *item, int index )
{
  if( index > (int) list.count() )
    list.append(item);
  else
    list.insert(index, item);
}

short PMenu::parse( QDir d )
{
  if( !d.exists() )
    return -1;

  PMenuItem *new_item;
  PMenu *new_menu;
  QDir new_dir;
  QStrList sort_order;
  QList<PMenuItem> item_list;
  item_list.setAutoDelete(false);
  int pos;
  sort_order.setAutoDelete(true);

  if (altSort == true)
    d.setSorting(QDir::DirsFirst | QDir::Name);

  QString file = d.path();
  QFileInfo dir_info(file);
  file += "/.directory";
  QFile config(file);
  if( config.exists())
    {
      KSimpleConfig kconfig(file, true); // kalle
      kconfig.setGroup("KDE Desktop Entry");
      QString order = kconfig.readEntry("SortOrder");
      int len = order.length();
      int j,i;
      QString temp;
      for( i = 0; i < len; )
	{
	  j = order.find(',', i);
	  if( j == -1 )
	    j = len;
	  temp = order.mid(i, j-i);
	  temp.stripWhiteSpace();
	  sort_order.append(temp);
	  i = j+1;
	}
    }

  const QFileInfoList *list = d.entryInfoList();
  QFileInfoListIterator it( *list );
  QFileInfo *fi;
  if( it.count() < 3 )
    return -1;
  while ( (fi=it.current()) )
    {
      if( fi->fileName() == "." || fi->fileName() == ".." )
	{ ++it; continue; }
      if( fi->isDir() && fi->isReadable() )
	{
	  new_menu = new PMenu;
	  new_menu->setAltSort(altSort);
	  new_item = new PMenuItem;
          new_dir.setPath( fi->filePath() );
	  if( new_menu->parse( new_dir ) < 0 || new_item->parse( fi, new_menu ) < 0 )
	    {
	      delete new_menu;
	      delete new_item;
	      new_item = 0;
	      ++it;
	      continue;
	    }
	}
      else
	{
	  if( !isKdelnkFile(fi->absFilePath()))
	    { ++it; continue; }
	  new_item = new PMenuItem;
	  if( new_item->parse(fi) < 0 ){
	    delete new_item;
	    new_item = 0;
	  }
	  else {
	    if (fi->fileName() == "KMenuEdit.kdelnk")
	      menu_editor_item = new_item;
	  }
	}

      if ( new_item )
      {

        PMenuItem *tmp = item_list.first();

        if ( altSort == true && int(new_item->getType() ) > submenu)
          for (;
            tmp && int(tmp->getType()) <= submenu;
            tmp = item_list.next());

        for (;
          tmp && stricmp( tmp->text_name, new_item->text_name.data() ) <= 0;
          tmp = item_list.next());

        if ( !tmp )
          item_list.append( new_item );
        else
          item_list.insert( item_list.at(), new_item );
      }
      ++it;
    }
  // sort items
  QString item_name;
  PMenuItem *item;
  for( item_name = sort_order.first(); !item_name.isEmpty(); item_name = sort_order.next() )
    {
      for( item = item_list.first(); item != 0; item = item_list.next() )
	{
	  if( item->real_name == item_name )
	    {
	      add(item);
	      item_list.removeRef(item);
	      break;
	    }
	}
    }
  if( item_list.count() != 0 )
    {
      for( item = item_list.first(); item != 0; item = item_list.next() )
	{ add(item); }
    }
  item_list.clear();
  // insert separators
  sort_order.first();
  while( (pos = sort_order.findNext((QString) "SEPARATOR")) >= 0 )
    {
      sort_order.next();
      new_item = new PMenuItem;
      new_item->entry_type = separator;
      insert(new_item, pos);
    }
  return 0;
}


void PMenu::move(short item_id, short new_pos)
{
  PMenuItem *item;
  if( item_id > new_pos )
    {
      item = list.take(item_id);
      list.insert( new_pos, item);
    }
  else
    {
      item = list.take(item_id);
      list.insert( new_pos - 1, item);
    }
}

void PMenu::remove( short item_id )
{
  list.remove( item_id );
}

void PMenu::create_pixmap( QPixmap &buf, PMenuItem *item, QPopupMenu *menu)
{
  int w, h;
  QPainter p;
  QFontMetrics fm = menu->fontMetrics();   // size of font set for this widget

  w  = 2 + item->pixmap.width() + 4 + fm.width( item->text_name ) + 2;
  h = ( item->pixmap.height() > fm.height() ? item->pixmap.height() : fm.height() ) + 4;

  buf.resize( w, h );                    // resize pixmap
  buf.fill( menu->backgroundColor() );   // clear it

  p.begin( &buf );
  p.drawPixmap( 2, 2, item->pixmap );              // use 2x2 border
  p.setFont( menu->font() );
  /*
  p.drawText( 2 + item->pixmap.width() + 4,        // center text in item
	      (h + fm.height()) / 2 - fm.descent(),
	      item->text_name );
	      */
  p.drawText( 2 + item->pixmap.width() + 4,        // center text in item
	      0, w, h,
	      AlignVCenter | ShowPrefix | DontClip | SingleLine,
	      item->text_name );
  p.end();
}

PMenuItem * PMenu::searchItem(QString name)
{
  // search for kdelnk-file as a PMenuItem inside this PMenu hierarchy
  // if it can't find the file it will return a new created PMenuItem

  getPaths();
  PMenuItem *found_item = 0L;
  PMenuItem *item;
  QString path;
  static PMenu* hack = 0;
  if (!hack)
    hack = this;
  //debug("searchName = %s", (const char *) name );
  name = name.stripWhiteSpace();
  if( name.left(9) == "$PERSONAL" )
    {
      // in $HOME/Personal
      name.remove(0, 9);
      name.prepend(personal);
    }
  else if( name.left(8) == "$KDEAPPS" )
    {
      // in $KDEDIR/apps
      name.remove(0, 8);
      name.prepend(kde_apps);
    }
  path = name.left( name.findRev('/') );

  if( list.first() && list.first()->dir_path != path )
    {
      // can't be inside of this menu, so search only for submenus
      for( item = list.first(); item != 0L; item = list.next() )
	{
	  if( item->fullPathName() == name)
	    return item;
	  if( item->cmenu != 0L )
	    {
	     if( ( found_item = item->sub_menu->searchItem(name) ) )
	       return found_item;
	    }
	}
    }
  else
    {
      // should be inside of this menu
      for( item = list.first(); item != 0L; item = list.next() )
	{
	  if( item->fullPathName() == name )
	    return item;
	}
    }

  if (hack == this)
    hack = 0;

  PMenuItem *pmi = 0;

  if (!hack){
    if (isKdelnkFile(name)){
      // generate a free entry
      pmi = new PMenuItem(unix_com);
      pmi->parse(name);
    } else {
      QFileInfo i(name);
      if (i.isDir()) {
	PMenu *dir = new PMenu();
	dir->parse(QDir(name));
	pmi = new PMenuItem(submenu);
	pmi->parse(&i, dir);
      }
    }
  }
  return pmi;
}

PMenuItem * PMenu::searchItem(int id)
{
  PMenuItem *item;
  for( item = list.first(); item != 0 && item->getId() != id;
       item = list.next() );

  return item;
}

#ifdef DISKNAV_DEBUG

PMenuItem * PMenu::searchItem(PMenu* _item)
{
  PMenuItem *item;
  for( item = list.first(); item != 0 && item->sub_menu != _item;
       item = list.next() );

  return item;
}
#endif

void PMenu::highlighted( int id )
{
  PMenuItem *item;
  for( item = list.first(); item != 0 && item->getId() != id;
       item = list.next() );
  if (item && item->getId() == id)
    item->highlighted();
}

PMenuItem*PMenu:: menu_editor_item = 0;
