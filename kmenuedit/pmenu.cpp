// -*- C++ -*-

//
//  kmenuedit
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

#include "pmenu.h"
#include "confmenu.h"

#include "pmenu.moc"

extern "C" {
#include <stdlib.h>
}

extern KIconLoader *global_pix_loader;

static bool isKdelnkFile(const char* name){
  QFile file(name);
  if (file.open(IO_ReadOnly)){
    char s[19];
    int r = file.readLine(s, 18);
    if(r > -1){
      s[r] = '\0';
      file.close();
      return (QString(s).left(17) == "# KDE Config File");
    }
    file.close();
  }
  return FALSE;
}


PMenuItem::PMenuItem() 
  : url_name(command_name), dev_name(command_name), mount_point(term_opt), fs_type(exec_path),
    dev_read_only(use_term), umount_pixmap_name(pattern)
{
  initMetaObject();
  entry_type = empty; 
  sub_menu = NULL;
  cmenu = NULL;
  recv = NULL;
  memb = NULL;
  read_only = FALSE;
}

PMenuItem::PMenuItem( EntryType e, QString t=0, QString c=0, QString n=0, PMenu *menu=0,
		      QObject *receiver=0, char *member=0, CPopupMenu *cm=0, bool ro = FALSE )
  : url_name(command_name), dev_name(command_name), mount_point(term_opt), fs_type(exec_path),
    dev_read_only(use_term), umount_pixmap_name(pattern)
{
  initMetaObject();
  entry_type = e;
  text_name = t;
  command_name = c;
  pixmap_name = n;
  if( !pixmap_name.isEmpty() )
    {
      pixmap = global_pix_loader->loadApplicationMiniIcon( pixmap_name, 14, 14 );
    }
  else
    {
      pixmap.resize(0,0);
    }
  sub_menu = menu;
  cmenu = cm;
  recv = receiver;
  memb = member;
  read_only = ro;
}

PMenuItem::PMenuItem( PMenuItem &item )
  : url_name(command_name), dev_name(command_name), mount_point(term_opt), fs_type(exec_path),
    dev_read_only(use_term), umount_pixmap_name(pattern)
{
  initMetaObject();
  text_name       = item.text_name;
  comment         = item.comment;
  pixmap_name     = item.pixmap_name;
  pixmap          = item.pixmap;
  entry_type      = item.entry_type; 
  command_name    = item.command_name;
  big_pixmap_name = item.big_pixmap_name;
  term_opt        = item.term_opt;
  exec_path       = item.exec_path;
  dir_path        = item.dir_path;
  use_term        = item.use_term;
  pattern         = item.pattern;
  protocols       = item.protocols;
  extensions      = item.extensions;
  if( entry_type == submenu )
    {
      sub_menu = new PMenu( *(item.sub_menu) );
      cmenu    = new CPopupMenu;
    }
  else
    {
      sub_menu = NULL;
      cmenu    = NULL;
    }
  recv         = item.cmenu;
  memb         = item.memb;
  if( item.read_only && entry_type == prog_com )
    {
      entry_type = unix_com;
      recv = NULL;
      memb = NULL;
    }
  read_only = FALSE; 
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

QPixmap PMenuItem::getPixmap()
{
  return pixmap;
}

QString &operator<<( QString &s, PMenuItem &item )
{
  if( item.read_only )
    {
      s = "";
      return s;
    }
  switch(item.entry_type) {
  case empty:
    s = "";
    return s;
  case submenu:
    s = "Menu ";
    break;
  case separator:
    s = "Separator ";
    break;
  case label:
    s = "Label ";
    break;
  case unix_com:
    s = "Unix_Com ";
    break;
  case fvwm_com:
    s = "Fvwm_Com ";
    break;
  case net_com:
    s = "Net_Com ";
    break;
  default:
    s = "";
    return s;
  };
  s += item.text_name + ", ";
  s += item.pixmap_name + ", ";
  s += item.command_name;
  return s;
}

short PMenuItem::parse( QString &s, PMenu *menu = NULL)
{
  s.simplifyWhiteSpace();
  short pos = s.find(' ');
  short npos;
  if ( pos < 0 )
    { pos = s.length(); }
  QString command = s.left( pos );
//debug ( "com = '%s'", (const char *) command);
  if ( command == "Menu" )
    {
      entry_type = submenu;
      sub_menu = menu;
      cmenu = new CPopupMenu;
    }
  else if ( command == "Separator" )
    {
      entry_type = separator;
      return 0;
    }
  else if ( command == "Label" )
    { entry_type = label; }
  else if ( command == "Unix_Com" )
    { entry_type = unix_com; }
  else if ( command == "Fvwm_Com" )
    { entry_type = fvwm_com; }
  else if ( command == "Net_Com" )
    { entry_type = net_com; }
  else
    { 
      entry_type = empty;
      return -1; 
    }
  if( (npos = s.find(',', pos)) < 0 )
    { entry_type = empty; return -1; }
  while( s[pos+1] == ' ' )  pos++;
  text_name = s.mid( pos+1, npos-pos-1 );
  pos = npos;
  if( (npos = s.find(',', pos+1)) < 0 )
    { entry_type = empty; return -1; }
  while( s[pos+1] == ' ' )  pos++;
  pixmap_name = s.mid( pos+1, npos-pos-1 );
  while( s[npos+1] == ' ' ) npos++;
  command_name = s.right( s.length()-npos-1 );
//debug ( "text = '%s'", (const char *) text_name);
//debug ( "pix = '%s'", (const char *) pixmap_name);
//debug ( "unix = '%s'", (const char *) command_name);
  if( !pixmap_name.isEmpty() )
    {
      pixmap = global_pix_loader->loadApplicationMiniIcon(pixmap_name, 14, 14);
    }
  return 0;
}

short PMenuItem::parse( QFileInfo *fi, PMenu *menu = NULL  )
{
  QString type_string;
  int pos = fi->fileName().find(".kdelnk");
  if( pos >= 0 )
    text_name = fi->fileName().left(pos);
  else
    text_name = fi->fileName();
  if( !(fi->isWritable()) )
    read_only = TRUE;
  if( menu != NULL )
    {
      QString file = fi->absFilePath();
      file += "/.directory";
      QFile config(file);
      if( config.open(IO_ReadOnly) ) 
	{
	  config.close(); // kalle
	  // kalle	  QTextStream st( (QIODevice *) &config);
	  KConfig kconfig(file);
	  kconfig.setGroup("KDE Desktop Entry");
	  pixmap_name = kconfig.readEntry("MiniIcon");
	  big_pixmap_name = kconfig.readEntry("Icon");
	  config.close();
	}
      entry_type  = submenu;
      sub_menu    = menu;
      cmenu       = new CPopupMenu;      
    }
  else
    {
      QFile config(fi->absFilePath());
      if( !config.open(IO_ReadOnly) ) 
	return -1;
	  config.close(); // kalle
	  // kalle      QTextStream st( (QIODevice *) &config);
      KConfig kconfig(fi->absFilePath());
      kconfig.setGroup("KDE Desktop Entry");
      command_name    = kconfig.readEntry("WmCommand");
      comment         = kconfig.readEntry("Comment");
      pixmap_name     = kconfig.readEntry("MiniIcon");
      big_pixmap_name = kconfig.readEntry("Icon");
      if( !command_name.isEmpty() )
	{
	  entry_type = fvwm_com;
	}
      else
	{
	  type_string = kconfig.readEntry("Type");
	  if( type_string == "Application" ) 
	    {
	      entry_type = unix_com;
	      command_name = kconfig.readEntry("Exec");
	      term_opt        = kconfig.readEntry("TerminalOptions");
	      exec_path       = kconfig.readEntry("Path");
	      dir_path        = fi->dirPath(TRUE);
	      use_term        = kconfig.readNumEntry("Terminal");
	      pattern         = kconfig.readEntry("BinaryPattern");
	      protocols       = kconfig.readEntry("Protocols");
	      extensions      = kconfig.readEntry("Extensions");
	    }
	  else if( type_string == "Link" )
	    {
	      entry_type = url;
	      url_name   = kconfig.readEntry("URL");
	    }
	  else if( type_string == "FSDevice" )
	    {
	      entry_type  = device;
	      dev_name    = kconfig.readEntry("Dev");
	      mount_point = kconfig.readEntry("MountPoint");
	      fs_type     = kconfig.readEntry("FSType");
	      umount_pixmap_name = kconfig.readEntry("UnmountIcon");
	      dev_read_only = kconfig.readNumEntry("ReadOnly");
	    }
	}
      config.close();
    }
  if( pixmap_name.isEmpty() )
    {
      pixmap_name = "mini-ball.xpm";
      pixmap = global_pix_loader->loadMiniIcon("mini-ball.xpm", 14, 14);
    }
  else
    {
      pixmap = global_pix_loader->loadApplicationMiniIcon(pixmap_name, 14, 14);
    }
  if( big_pixmap_name.isEmpty() )
    {
      if( entry_type == submenu )
	big_pixmap_name = "folder.xpm";
      else if( entry_type == unix_com )
	big_pixmap_name = "exec.xpm";
    }
  return 0;
}

void PMenuItem::writeConfig( QDir dir )
{
  if( read_only || entry_type == separator )
    return;
  QString file = dir.absPath();
  file += ( (QString) "/" + text_name ); //+ ".kdelnk" );
  QFile config(file);
  if( !config.open(IO_ReadWrite) ) 
    return;
  config.close(); // kalle
  // kalle  QTextStream st( (QIODevice *) &config);
  KConfig kconfig(file);
  kconfig.setGroup("KDE Desktop Entry");
  kconfig.writeEntry("Comment", comment );
  kconfig.writeEntry("Icon", big_pixmap_name );
  kconfig.writeEntry("MiniIcon", pixmap_name );
  switch( (int) entry_type ) {
  case (int) fvwm_com:
    kconfig.writeEntry("WmCommand", command_name );
    kconfig.writeEntry("Exec", "" );
    break;
  case (int) unix_com:
    kconfig.writeEntry("WmCommand", "" );
    kconfig.writeEntry("Exec", command_name );
    kconfig.writeEntry("TerminalOptions", term_opt );
    kconfig.writeEntry("Path", exec_path );
    kconfig.writeEntry("Terminal", use_term );
    kconfig.writeEntry("BinaryPattern", pattern);
    kconfig.writeEntry("Protocols", protocols);
    kconfig.writeEntry("Extensions", extensions);
    kconfig.writeEntry("Type", "Application");
    break;
  case (int) url:
    kconfig.writeEntry("URL", url_name);
    kconfig.writeEntry("Type", "Link");
    break;
  case (int) device:
    kconfig.writeEntry("Dev", dev_name);
    kconfig.writeEntry("MountPoint", mount_point);
    kconfig.writeEntry("FSType", fs_type);
    kconfig.writeEntry("UnmountIcon", umount_pixmap_name);
    kconfig.writeEntry("ReadOnly", dev_read_only);
    kconfig.writeEntry("Type", "FSDevice");
    break;
  };
  kconfig.sync();
  config.close();
}

void PMenuItem::exec_system()
{
  if( command_name.isNull() )
    return;
  QString name;
  if( command_name.right(1) != "&" )
    name = command_name + " &";
  else
    name = command_name;
  system( (const char *) name );
}

void PMenuItem::exec_fvwm()
{
  // ((TaskBar *) (main_app->mainWidget()))->send_fvwm_com(command_name);
}

//--------------------------------------------------------------------------------------

PMenu::PMenu()
{
  initMetaObject();
  menu_conf = NULL;
  list.setAutoDelete(TRUE);
}

PMenu::PMenu( PMenu &menu )
{
  initMetaObject();
  menu_conf = NULL;
  list.setAutoDelete(TRUE);
  PMenuItem *item, *new_item;
  for( item = menu.list.first(); item != 0; item = menu.list.next() )
    {
      new_item = new PMenuItem( *item );
      list.append(new_item);
    }
}

PMenu::~PMenu()
{
  hideConfig();
  list.clear();
}

short PMenu::create_cmenu( CPopupMenu *menu )
{
  QPixmap buffer;
  PMenuItem *item;
  //  int i;
  int id;
  EntryType et;
  for( item = list.first(); item != 0; item = list.next() )
    {
      et = item->entry_type;
      switch ( et ) {
      case separator:
	menu->insertSeparator();
	continue;
      case submenu:
	item->cmenu->setFont(menu->font());
	item->sub_menu->create_cmenu( item->cmenu );
	create_pixmap(buffer, item, menu);
	id = menu->insertItem(buffer, item->cmenu, -2);
	continue;
      case label:
	create_pixmap(buffer, item, menu);
	id = menu->insertItem( buffer, -2);
	continue;
      case unix_com:
	create_pixmap(buffer, item, menu);
	id = menu->insertItem( buffer, -2);
	if( !item->command_name.isEmpty() )
	  menu->connectItem( id, item, SLOT(exec_system()) );
	continue;
      case fvwm_com:
	create_pixmap(buffer, item, menu);
	id = menu->insertItem( buffer, -2);
	if( !item->command_name.isEmpty() )
	  menu->connectItem( id, item, SLOT(exec_fvwm()) );
	continue;
      case prog_com:
	create_pixmap(buffer, item, menu);
	id = menu->insertItem( buffer, -2);
	menu->connectItem(id, item->recv, item->memb);
	continue;
      case net_com:
	create_pixmap(buffer, item, menu);
	id = menu->insertItem(buffer, -2);
	menu->connectItem( id, item, SLOT(exec_system()) );
	menu->connectItem( id, item->recv, item->memb );
	continue;
      case empty:
	continue;
      default:
	return -1;
      };
    }
  return 0;
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

short PMenu::parse( QDataStream &s )
{
  QString buf;
  short pos;
  QString command;
  PMenuItem *new_item;
  PMenu *new_menu;
  char c;
  s >> c;
  while ( c != '\n' )
    {
      buf += c; 
      s >> c;
    }
  buf.simplifyWhiteSpace();
//  debug("%s", (const char *) buf);
  if( buf != "Begin_Menu" )
    return -1;
  while( !s.eof() )
    { 
      buf = "";
      s >> c;
      while ( c != '\n' && !s.eof() )
	{
	  buf += c; 
	  s >> c;
	}
      buf.simplifyWhiteSpace();
//debug ( "line = '%s'", (const char *) buf);
      if( buf == "End_Menu" )
	break;
      if( (pos = buf.find(' ')) < 0)
	{
	  if ( buf.length() == 0 )
	    continue;
	  else
	    pos = buf.length();
	}
      command = buf.left(pos);
      if( command == "Menu" )
	{
	  new_menu = new PMenu();
	  new_item = new PMenuItem;
	  new_item->parse(buf, new_menu);
	  if( new_menu->parse(s) < 0) 
	    {
	      delete new_menu;
	      delete new_item;
	      continue;
	    }
	  add(new_item);
	}
      else
	{
	  new_item = new PMenuItem;
	  new_item->parse(buf);
	  add(new_item);
	}
    }
  if ( buf != "End_Menu" )
    return -1;
  return 0;
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
  item_list.setAutoDelete(FALSE);
  int pos;
  bool read_only = FALSE;
  sort_order.setAutoDelete(TRUE);

  QString file = d.path();
  QFileInfo dir_info(file);
  if( !dir_info.isWritable() )
    read_only = TRUE;
  file += "/.directory";
  QFileInfo dir_fi(file);
  if( dir_fi.isReadable() )
    {
      KConfig kconfig(file);
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
      if( fi->isDir() )
	{
	  new_menu = new PMenu;
	  new_item = new PMenuItem;
          new_dir.setPath( fi->filePath() );
	  new_item->read_only = read_only;
	  if( new_menu->parse( new_dir ) < 0 || new_item->parse( fi, new_menu ) < 0 )
	    {
	      delete new_menu;
	      delete new_item;
	      ++it;
	      continue;
	    }
	  item_list.append(new_item);
	}
      else
	{
	  //if( !fi->extension().contains("kdelnk") )
	  if( !isKdelnkFile(fi->absFilePath()))
	    { ++it; continue; }
	  new_item = new PMenuItem;
	  new_item->read_only = read_only;
	  if( new_item->parse(fi) < 0 )
	    delete new_item;
	  else
	    item_list.append(new_item);
	}
      ++it;
    }
  // sort items
  QString item_name;
  PMenuItem *item;
  for( item_name = sort_order.first(); !item_name.isEmpty(); item_name = sort_order.next() )
    {
      for( item = item_list.first(); item != NULL; item = item_list.next() )
	{
	  if( item->text_name == item_name )
	    {
	      add(item);
	      item_list.removeRef(item);
	      break;
	    }
	}
    }
  if( item_list.count() != 0 )
    {
      for( item = item_list.first(); item != NULL; item = item_list.next() )
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
      new_item->read_only = read_only;
      insert(new_item, pos);
    }   
  return 0;
}

void PMenu::writeConfig( QTextStream &s )
{
  QString text = "";
  s << "Begin_Menu\n";
  PMenuItem *item;
  for( item = list.first(); item != 0; item = list.next() )
    {
      text << (*item);
      if( text.length() == 0 )
	continue;
      s << text << '\n';
      if( item->getType() == submenu )
	item->sub_menu->writeConfig(s);
    }
  s << "End_Menu\n";
}

void PMenu::writeConfig( QDir base_dir, PMenuItem *parent_item = NULL )
{
  if( parent_item )
    if( parent_item->read_only )
      return;
  if( !base_dir.exists() )
    {
      return;
    }
  QString name;
  const QStrList *temp_list = base_dir.entryList("*");
  QStrList file_list;
  file_list.setAutoDelete(TRUE);
  QStrListIterator temp_it( *temp_list );
  while( name = temp_it.current() )
    {
      file_list.append(name);
      ++temp_it;
    }
  temp_list = base_dir.entryList("*", QDir::Dirs);
  QStrList dir_list;
  dir_list.setAutoDelete(TRUE);
  temp_it.toFirst();
  while( name = temp_it.current() )
    {
      if(name != "." && name != "..")
	dir_list.append(name);
      ++temp_it;
    }

  QString sort_order;
  PMenuItem *item;
  for( item = list.first(); item != 0; item = list.next() )
    {
      if( item->read_only )
	continue;
      if( item->entry_type == separator )
	sort_order += ((QString) "SEPARATOR" + ',');
      else
	sort_order += (item->text_name + ',');
      if( item->getType() == submenu )
	{
	  if( item->read_only )
	    continue;
	  QDir sub_dir(base_dir);
	  if( !sub_dir.cd(item->text_name) )
	    {
	      base_dir.mkdir(item->text_name);
	      if( !sub_dir.cd(item->text_name) )
		continue;
	    }
	  item->sub_menu->writeConfig( sub_dir, item );
	  dir_list.remove(item->text_name);
	}
      else
	{
	  item->writeConfig(base_dir);
	  file_list.remove(item->text_name); // + ".kdelnk");
	}
    }
  // remove files not in pmenu
  for( name = file_list.first(); !name.isEmpty(); name = file_list.next() )
    {
      //debug("will remove file: %s", (const char *) name );
      base_dir.remove(name);
    }
  // remove dirs not in pmenu
  for( name = dir_list.first(); !name.isEmpty(); name = dir_list.next() )
    {
      //debug("will remove dir: %s", (const char *) name );
      QDir sub_dir(base_dir);
      if(sub_dir.cd(name))
	{
	  PMenu *new_menu = new PMenu;
	  new_menu->writeConfig(sub_dir);
	  delete new_menu;
	  sub_dir.remove(".directory");	  
	}
      base_dir.rmdir(name);
    }
  sort_order.truncate(sort_order.length()-1);
  QString file = base_dir.absPath();
  file += "/.directory";
  QFile config(file);
  if( !config.open(IO_ReadWrite) ) 
    return;
  config.close(); // kalle
  // kalle  QTextStream st( (QIODevice *) &config);
  KConfig kconfig(file);
  kconfig.setGroup("KDE Desktop Entry");
  kconfig.writeEntry("SortOrder", sort_order);
  if( parent_item )
    {
      kconfig.writeEntry("MiniIcon", parent_item->pixmap_name );
      if( parent_item->big_pixmap_name.isEmpty() )
	parent_item->big_pixmap_name = "folder.xpm";
      kconfig.writeEntry("Icon", parent_item->big_pixmap_name );
    }
  kconfig.sync();
  config.close();
}

void PMenu::move(short item_id, short new_pos)
{
  //  PMenuItem *current;
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
  //debug
  //debug("but_id = %i / new_pos = %i", item_id, new_pos);
  //int i = 0;
  //for( item = list.first(); item != 0; item = list.next(), i++ )
  //  {
  //    debug("PMenu:: name = %s / id = %i", (const char *) item->getText(), i);
  //  }
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

void PMenu::set_net_recv(QObject *receiver, char *member)
{
  PMenuItem *item;
  for( item = list.first(); item != 0; item = list.next() )
    {
      if( item->entry_type == net_com )
	{
	  item->recv = receiver;
	  item->memb = member;
	}
    }
}

void PMenu::popupConfig(QPoint p, QWidget *par_widg, bool bot = FALSE)
{
  if( !menu_conf )
    {
      menu_conf = new ConfigureMenu(this, par_widg, "menu_conf");
      PMenuItem *item;
      for( item = list.first(); item != 0; item = list.next() )
	{
	  menu_conf->append(item);
	}
      if( bot )
	{
	  QPoint np( 0, menu_conf->getHeight()+3 );
	  p -= np;
	}
      QWidget *desk = QApplication::desktop();
      int dw = desk->width();
      int dh = desk->height();
      int width = menu_conf->getWidth();
      int height = menu_conf->getHeight();
      int x = p.x();
      int y = p.y();
      if( (x+width) > dw )
	x = dw - width;
      if( (y+height+2) > dh )
	y = dh - height - 2;
      menu_conf->move(x, y);
      menu_conf->show();
    }
  else
    {
      hideConfig();
    }
}

void PMenu::moveConfig(QPoint p)
{
  if( menu_conf )
    {
      QWidget *desk = QApplication::desktop();
      int dw = desk->width();
      int dh = desk->height();
      int width = menu_conf->getWidth();
      int height = menu_conf->getHeight();
      int x = p.x();
      int y = p.y();
      if( (x+width) > dw )
	x = dw - width;
      if( (y+height+2) > dh )
	y = dh - height - 2;
      menu_conf->moveRequest(x, y);
    }
}

void PMenu::hideConfig()
{
  if( menu_conf )
    {
      menu_conf->hide();
      delete menu_conf;
      menu_conf = NULL;
    }
}

QPoint PMenu::configPos()
{
  return menu_conf->pos();
}
