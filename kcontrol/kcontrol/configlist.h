/*
  configlist.h - internally used by the KDE control center

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


#ifndef _CONFIGLIST_
#define _CONFIGLIST_


#include <qobject.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qtabdlg.h>
#include <ktreelist.h>
#include <qlist.h>
#include <kprocess.h>
#include <kwm.h>

#include "kswallow.h"


class KModuleListEntry : QObject
{
  Q_OBJECT

public:

  KModuleListEntry(const QString &fn);
  ~KModuleListEntry();
  
  QList<KModuleListEntry> *getChildren() { return children; };

  QString &getFilename() { return filename; };
  QString &getExec()     { return exec; };
  QPixmap getIcon();
  QString &getDocPath()  { return docPath; };
  QString &getComment()  { if (!comment.isEmpty()) return comment; else return name; };
  QString &getName()     { return name; };
  QString &getInit()     { return init; };
  QString &getSwallowTitle() { return swallowTitle; };

  bool    isDirectory()  { return (children != 0) && (children->count() > 0); };
  bool    isSwallow()    { return swallowingEnabled && !swallowTitle.isEmpty(); };
  
  bool    execute(QWidget *parent);

  KSwallowWidget *getSwallowWidget() { return swallowWidget; };

  static KSwallowWidget *visibleWidget;
	
  static bool swallowingEnabled;
  
  void insertInit(QStrList *list);

private:

  QList<KModuleListEntry> *children;

  QString filename;
  QString exec;
  QString icon;
  QString miniIcon;
  QString docPath;
  QString comment;
  QString name;
  QString init;
  QString swallowTitle;

  KProcess       *process;
  KSwallowWidget *swallowWidget;
  QWidget        *swallowParent;

  void parseKdelnkFile(const QString &fn);

private slots:

  void processExit(KProcess *proc);

  void addWindow(Window w);
 
};


// -----------------------------------


class ConfigTreeItem : public KTreeListItem
{
public:

  ConfigTreeItem(KModuleListEntry *e = NULL) { moduleListEntry = e; };
  
  KModuleListEntry *moduleListEntry;
};


class ConfigList
{
public:
  
  ConfigList(); 
  ~ConfigList();
    
  void loadConfigModules();

  bool select(QString name);

  void fillTreeList(KTreeList *list);

  void doInit();

  void raiseWidget(KSwallowWidget *widget);
  
private:
  
  KModuleListEntry *modules;

  void insertEntry(KTreeList *list, KPath *path, KModuleListEntry *entry, bool root);

};

#endif
