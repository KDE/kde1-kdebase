// -*- C++ -*-

//
//  kpanel
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
//  but WITHOUT ANY WARRANTY; without even the7 implied warranty of
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

#ifndef PMENU_H
#define PMENU_H

#include <qpixmap.h>
#include <qlist.h>
#include <qfileinf.h>
#include <qpopmenu.h>

#include <kapp.h>

class QDir;
class PMenu;
class kPanel;

class myPopupMenu: public QPopupMenu
{
  Q_OBJECT
public:
  myPopupMenu( QWidget *parent=0, const char *name=0 );
  ~myPopupMenu() {}
  int height();
  int width();
private:	// Disabled copy constructor and operator=
  myPopupMenu( const myPopupMenu & ) {}
  myPopupMenu &operator=( const myPopupMenu & ) { return *this; }
};


enum EntryType { empty, separator, submenu, unix_com, prog_com, label, add_but };

#define __cc0 (void*)0

class PMenuItem : public QObject
{
  Q_OBJECT
  friend QString &operator<<(QString &, PMenuItem &);
  friend PMenu;
public:
  PMenuItem();
  PMenuItem( EntryType e, QString t=QString(), QString c=QString(), QString n=QString(), PMenu *menu=0,
	     QObject *receiver=0, char *member=0, QPopupMenu *cm=0, bool ro = FALSE,
	     QString d=QString(), QString co=QString() );
  PMenuItem( PMenuItem &item );
  ~PMenuItem ();

  short     parse( QFileInfo *fi, PMenu *menu = 0L );
  short     parse( QString abs_file_path );
  void      setType( EntryType e) { entry_type = e; }
  EntryType getType() { return entry_type; }
  void      setMenu( PMenu *menu );
  PMenu    *getMenu() { return sub_menu; }
  QPopupMenu *getQPopupMenu() { return cmenu; }
  void      removeMenu();
  QString   getSaveName();
  QString   bigIconName(){ return big_pixmap_name;};
  QString   text() { return text_name; }
  QString   fullPathName(){return dir_path + "/" + real_name;}
  QString   getComment(){ return comment;}
  QString   getDirPath(){ return dir_path;}

  int getId(){return id;}

public slots:
  void exec();

signals:
  void showToolTip(QString);
  void addButton(PMenuItem *);

protected slots:
  void highlighted() { emit showToolTip(comment); }
  void execAddButton() { emit addButton(this); }

protected:
  static bool  use_kfm;
  static bool  in_kwm_mode;
  QString      text_name;
  QString      real_name;
  QString      pixmap_name;
  QString      big_pixmap_name;
  QPixmap      pixmap;
  EntryType    entry_type;
  QString      command_name;
  QString      dir_path;
  PMenu       *sub_menu;
  myPopupMenu  *cmenu;
  QObject     *recv;
  char        *memb;
  QString      comment;

private:
  int id;
};

class PMenu : public QObject
{
  Q_OBJECT
public:
  PMenu ();
  PMenu ( PMenu &menu );
  ~PMenu ();

  void       add ( PMenuItem *item);
  void       insert ( PMenuItem *item, int index );
  void       move( short item_id, short new_pos);
  void       remove( short item_id );
  short      parse ( QDir d );
  void       createMenu( QPopupMenu *menu, kPanel *panel, bool add_button = FALSE );
  void       create_pixmap( QPixmap &buf, PMenuItem *item, QPopupMenu *menu );
  void       set_net_recv( QObject *receiver, char *member );
  PMenuItem *searchItem( QString name );
  QPopupMenu *getQPopupMenu() { return cmenu; }

  void       setAltSort( bool alternateSort ) { altSort = alternateSort; }
  bool       getAltSort() { return altSort; }

signals:

public slots:

protected slots:
  void highlighted(int id);

protected:
  QList<PMenuItem> list;
  myPopupMenu      *cmenu;

  bool             altSort;
};



inline void PMenuItem::setMenu( PMenu *menu )
{
  if( sub_menu )
    delete sub_menu; 
  sub_menu = menu; 
}

inline void PMenuItem::removeMenu()
{
  if( sub_menu )
    delete sub_menu;
  sub_menu = NULL; 
}

#endif // PMENU_H
