// -*- C++ -*-

//
//  kpanel
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
//

/*
 * Several modifications and extensions by Matthias Ettrich <ettrich@kde.org>
 */

#ifndef PMENU_H
#define PMENU_H

#include <cstdlib>

#include <qpixmap.h>
#include <qlist.h>
#include <qfileinf.h>
#include <qpopmenu.h>

#include <kapp.h>

class QDir;
class PMenu;
class PFileMenu;
class kPanel;

class myPopupMenu: public QPopupMenu
{
  Q_OBJECT
public:
  myPopupMenu( QWidget *parent=0, const char *name=0 );
  myPopupMenu( PFileMenu* _parentMenu );
  ~myPopupMenu() {}
  int height();
  int width();
  int id;        // if this object is a submenu, the submenu id

private:	// Disabled copy constructor and operator=
  myPopupMenu( const myPopupMenu & ) {}
  myPopupMenu &operator=( const myPopupMenu & ) { return *this; }

  virtual bool x11Event( XEvent * xe);
  virtual void mousePressEvent ( QMouseEvent * );

public:
  int maxEntriesOnScreen();
  int entryHeight();
  PFileMenu* parentMenu;
  static int keyStatus;   // ShiftButton | ControlButton | AltButton
};


enum EntryType { empty, separator, submenu, unix_com, prog_com, label, add_but, dirbrowser, url };

#define __cc0 (void*)0

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class PMenuItem : public QObject
{
  Q_OBJECT
  friend QString &operator<<(QString &, PMenuItem &);
  friend PMenu;
  friend PFileMenu;
public:

#ifdef DISKNAV_DEBUG
  virtual void dump();
#endif

  PMenuItem();
  PMenuItem( EntryType e, QString text=QString(), QString command=QString(),
	     QString pixmap=QString(), PMenu *submenu=0,
	     QObject *receiver=0, char *member=0, QPopupMenu *cm=0,
	     bool readonly = FALSE,
	     QString dir=QString(), QString comment=QString() );

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
  QString   getBigIconName(){ return big_pixmap_name;};
  QString   getText() { return text_name; }
  QString   getFullPathName(){return dir_path + "/" + real_name;}
  QString   getComment(){ return comment;}
  QString   getDirPath(){ return dir_path;}

  int getId(){return id;}
  void setRealName(QString _real_name) { real_name = _real_name; }

  bool writeConfig( QDir dir );

public slots:
  void exec();
  void addFolderToRecentList();

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

#ifdef DISKNAV_DEBUG
  virtual void dump();
#endif

  void       add ( PMenuItem *item);
  void       insert ( PMenuItem *item, int index );
  void       move( short item_id, short new_pos);
  void       remove( short item_id );
  short      parse ( QDir d );
  short      parseLazy ( QDir d );
  virtual void createMenu( QPopupMenu *menu, kPanel *panel, bool add_button = FALSE );
  void       create_pixmap( QPixmap &buf, PMenuItem *item, QPopupMenu *menu );
  void       set_net_recv( QObject *receiver, char *member );
  PMenuItem *searchItem( QString name );

  PMenuItem *searchItem( int id );
  bool       addFromDrop( QString a);
  bool       copyFiles( QString source, QString dest);
  int        count() { return list.count(); }

#ifdef DISKNAV_DEBUG
  PMenuItem *searchItem( PMenu* item );  // DEBUG
#endif

  QPopupMenu *getQPopupMenu() { return cmenu; }

  void       setAltSort( bool alternateSort ) { altSort = alternateSort; }
  bool       getAltSort() { return altSort; }
  void       clearSubmenus();

signals:

public slots:
  virtual void aboutToShow();
  virtual void aboutToShowAddMenu();

protected slots:
  virtual void highlighted(int id);

private:
  bool parsed;

protected:
  QString uniqueFileName(QString name, QString dir_name);
  void parseBeforeShowing(bool is_add_menu);

  QList<PMenuItem> list;
  myPopupMenu      *cmenu;
  PMenuItem*       parentItem;

  bool             altSort;
  static PMenuItem* menu_editor_item;
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


class PFileMenu : public PMenu
{
  Q_OBJECT
public:
  friend PMenu;
  friend PMenuItem;

#ifdef DISKNAV_DEBUG
  virtual void dump();
#endif

  PFileMenu(bool isRoot = false);
  PFileMenu(QString& _path);
  PFileMenu(const char* _path);
  virtual ~PFileMenu();

  void setId(int _id) { this->id = _id; }
  int parseDir(QDir d, bool addOpenFolderEntry = true);
  int parseTail();

  PMenuItem* newDirBrowserItem(const QFileInfo* fi, bool useCurrentPath);
  PMenuItem* newLinkItem(const QFileInfo* fi, bool useCurrentPath);
  PMenuItem* newFileItem(const QFileInfo* fi, bool useCurrentPath);
  void buildRootMenu();

  static void updateRecentFolders(QString _path);
  static void updateRecentFiles(QString _path);
  static void calculateMaxEntriesOnScreen(PMenuItem* menu);

public slots:
  virtual void aboutToShow();
  void deactivated(int _id);
  void optionsDlg();
  void openFolder();
  void end() { exit(0); }   // DMALLOC DEBUG

protected:

  static void updateRecentList(EntryType, QString _path, QStrList& recentlist,
                               int max_size);


  static void insertRecentItem(EntryType type, const char* _path,
                                const char* fn = 0);

  static void removeLessRecentItem(EntryType type, QStrList& recentlist);

  bool addFile(QFileInfo* fi, bool useDefaultPath = true);
  bool addFile(QString _path, bool useDefaultPath = true);

  void addTailMenu(QFileInfoListIterator* tail);
  void copyTailFileInfo(QFileInfoListIterator& it);
  void fixMenuPosition();

#ifdef DISKNAV_DEBUG
  void doSelfCheck();
#endif

private:
  int id;
  QFileInfoListIterator* tail;
  QFileInfoList finfos;
  QString path;
  bool isClean;
  static PFileMenu* root;
  static int maxEntriesOnScreen;
  static int entryHeight;
  PFileMenu* lastActivated;
};

#endif // PMENU_H
