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

#ifndef PMENU_H
#define PMENU_H

#include <qpixmap.h>
#include <qlist.h>
#include <qfileinf.h>

#include <kapp.h>
#include "cpopmenu.h"

class QDir;
class PMenu;
class ConfigureMenu;

enum EntryType { empty, separator, submenu, unix_com, fvwm_com, url, device, prog_com, net_com, label };

class PMenuItem : public QObject
{
  Q_OBJECT
  friend QString &operator<<(QString &, PMenuItem &);
  friend PMenu;
public:
  PMenuItem();
  PMenuItem( EntryType e, QString t=0, QString c=0, QString n=0, PMenu *menu=0,
	      QObject *receiver=0, char *member=0, CPopupMenu *cm=0, bool ro = FALSE );
  PMenuItem( PMenuItem &item );
  ~PMenuItem ();

  short     parse( QString &s, PMenu *menu = NULL);
  short     parse( QFileInfo *fi, PMenu *menu = NULL);
  void      writeConfig( QDir dir );
  void      setText( QString t )       { text_name = t; }
  QString   getText()                  { return text_name; }
  void      setName( QString t )       { real_name = t; }
  QString   getName()                  { return real_name; }
  void      setComment( QString t )    { comment = t; }
  QString   getComment()               { return comment; }
  void      setPixmap( QPixmap pix )   { pixmap = pix; }
  QPixmap   getPixmap();
  void      setPixmapName( QString t ) { pixmap_name = t; }
  QString   getPixmapName()            { return pixmap_name; }
  void      setUMPixmapName( QString t ) { umount_pixmap_name = t; }
  QString   getUMPixmapName()          { return umount_pixmap_name; }
  void      setType( EntryType e)      { entry_type = e; }
  EntryType getType()                  { return entry_type; }
  void      setCommand( QString t )    { command_name = t; }
  QString   getCommand()               { return command_name; }
  void      setPattern( QString t )    { pattern = t; }
  QString   getPattern()               { return pattern; }
  void      setProtocols( QString t )  { protocols = t; }
  QString   getProtocols()             { return protocols; }
  void      setExtensions( QString t ) { extensions = t; }
  QString   getExtensions()            { return extensions; }
  void      setUrl( QString t )        { url_name = t; }
  QString   getUrl()                   { return url_name; }
  void      setDevice( QString t )     { dev_name = t; }
  QString   getDevice()                { return dev_name; }
  void      setMountP( QString t )     { mount_point = t; }
  QString   getMountP()                { return mount_point; }
  void      setFSType( QString t )     { fs_type = t; }
  QString   getFSType()                { return fs_type; }
  void      setMenu( PMenu *menu );
  PMenu    *getMenu()                  { return sub_menu; }
  void      removeMenu();
  void      setRecv( QObject *receiver, char *member ) { recv = receiver; memb = member; }
  bool      isReadOnly()                  { return read_only; }
  void      setReadOnly( bool b )         { read_only = b; }
  void      setExecDir( QString dir )     { exec_path = dir; }
  QString   getExecDir()                  { return exec_path; }
  QString   getDirPath()                  { return dir_path; }
  QPixmap   getBigPixmap();
  void      setBigPixmapName( QString n ) { big_pixmap_name = n; }
  QString   getBigPixmapName()            { return big_pixmap_name; }
  void      setTermOpt( QString opt )     { term_opt = opt; }
  QString   getTermOpt()                  { return term_opt; }
  void      setUseTerm( bool b )          { use_term = b; }
  bool      useTerm()                     { return use_term; }
  void      setDevReadOnly( bool b )      { dev_read_only = b; }
  bool      isDevReadOnly()               { return dev_read_only; }

protected slots:
  void exec_system();
  void exec_fvwm();

protected:
  QString     text_name;
  QString     real_name;
  QString     old_name;
  QString     pixmap_name;
  QString     comment;
  QPixmap     pixmap;
  EntryType   entry_type;
  QString     command_name;
  QString     big_pixmap_name;
  QString     term_opt;
  QString     exec_path;
  QString     dir_path;
  bool        use_term;
  PMenu      *sub_menu;
  CPopupMenu *cmenu;
  QObject    *recv;
  char       *memb;
  bool        read_only;
  QString     pattern;
  QString     protocols;
  QString     extensions;
  QString    &url_name;      // reference to command_name
  QString    &dev_name;      // reference to command_name
  QString    &mount_point;   // reference to term_opt
  QString    &fs_type;       // reference to exec_path
  bool       &dev_read_only; // reference to use_term;
  QString    &umount_pixmap_name; // reference to pattern
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
  short      parse ( QDataStream &s );
  short      parse ( QDir d );
  void       writeConfig( QTextStream &s );
  void       writeConfig( QDir base_dir, PMenuItem *parent_item = NULL );
  void       copyLnkFiles(QDir base_dir);
  void       renameLnkFiles(QDir base_dir);
  PMenuItem *cut( short pos ) { return NULL; }
  PMenuItem *copy( short pos ) { return NULL; }
  void       paste( PMenuItem *item ) { item = NULL; }
  short      create_cmenu( CPopupMenu *menu );
  void       create_pixmap( QPixmap &buf, PMenuItem *item, QPopupMenu *menu );
  void       set_net_recv( QObject *receiver, char *member );
  void       popupConfig(QPoint p, QWidget * par_widg, bool bot = FALSE);
  void       posRequest() { emit reposition (); }
  int        count() { return list.count(); }
  QPoint     configPos();
  bool checkFilenames(QString name); // returns TRUE if kdelnk-file with this name allready exists
  QString    uniqueFileName(QString name);

signals:
  void reposition();

public slots:
  void moveConfig(QPoint p);
  void hideConfig();

protected:
  void copyFiles( QString source, QString dest );

  QList<PMenuItem> list;
  ConfigureMenu   *menu_conf;
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








