/*
  configlist.cpp - internally used by the KDE control center

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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <qobject.h>
#include <qdir.h>
#include <kmsgbox.h>
#include <kiconloader.h>
#include <kapp.h>
#include <qfileinf.h> 

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "kdelnk.h"
#include "configlist.moc"
#include "configlist.h"


// ---------------------------------------------------------------
// class KModuleList


KSwallowWidget *KModuleListEntry::visibleWidget = 0;
bool KModuleListEntry::swallowingEnabled = TRUE;


KModuleListEntry::KModuleListEntry(const QString &fn)
  : filename(fn)
{
  QFileInfo info(fn);
  
  children = 0;
  process = 0;  
  swallowWidget = 0;
  swallowParent = 0;

  if (!info.isReadable())
    return;

  if (info.isDir())
    {
      // scan the directory information, if any
      QFileInfo desc(filename+"/.directory");
      
      if (desc.exists() && desc.isReadable())
	parseKdelnkFile(filename+"/.directory");

      // create a list of the children
      children = new QList<KModuleListEntry>();
      children->setAutoDelete(TRUE);

      // traverse the files in the directory
      QDir dir(filename);

      dir.setFilter(QDir::Dirs | QDir::Files);
      dir.setSorting(QDir::DirsFirst);
      
      for (unsigned int i=0; i<dir.count(); i++)
	{
	  // skip directories and hidden files
	  if (dir[i][0] == '.')
	    continue;

	  // add the file or directory
	  children->append(new KModuleListEntry(filename+"/"+dir[i])); 
	}
    }
  else
    {
      // parse the application information
      parseKdelnkFile(filename);
    }
}


KModuleListEntry::~KModuleListEntry()
{
  if (children)
    delete children;
  if (process)
    delete process;
}


void KModuleListEntry::parseKdelnkFile(const QString &fn)
{
  KKdelnk config(fn);

  exec =     config.readEntry("Exec");
  icon =     config.readEntry("Icon");
  miniIcon = config.readEntry("MiniIcon");
  docPath =  config.readEntry("DocPath");
  comment =  config.readEntry("Comment");
  name =     config.readEntry("Name", klocale->translate("Unknown module: ")+fn);
  init =     config.readEntry("Init");
  swallowTitle = config.readEntry("SwallowTitle");
}


QPixmap KModuleListEntry::getIcon()
{
  QPixmap result;

  if (!miniIcon.isEmpty())
    result = kapp->getIconLoader()->loadApplicationMiniIcon(miniIcon,16,16);

  if (result.isNull() && !icon.isEmpty())
    result = kapp->getIconLoader()->loadApplicationMiniIcon(icon,16,16);

  if (result.isNull())
    result = kapp->getMiniIcon();

  return result;
}


bool KModuleListEntry::execute(QWidget *parent)
{
  if (exec.isEmpty())
    return FALSE;
  if (isDirectory())
    return FALSE;

  swallowParent = parent;

  if (!process)
    {
      // Create process object
      process = new KProcess();
      if (!process)
	return FALSE;

      // split Exec entry 
      QString executable(exec.data());
      QString params;
      int pos = executable.find(' ');
      
      if (pos > 0)
	{
	  params = executable.right(executable.length()-pos-1);
	  executable.truncate(pos);
	}

      // set executable
      process->setExecutable(executable);

      // swallowing? then move out of screen, set unique title
      if (isSwallow())
	{
	  *process << "-swallow" << swallowTitle;
	  *process << "-geometry" << "480x480+10000+10000";

	  // connect to KWM events to get notification if window appears
	  connect(kapp, SIGNAL(windowAdd(Window)), this, SLOT(addWindow(Window)));
	}

      // set additional parameters
      //
      // Note: KProcess does not parse the arguments, so we have to
      // do it. This should be integrated into KProcess, and also
      // should be extended to escapes etc.
      //

      QString par;
      while (!params.isEmpty())
	{
          if (params[0]==' ')
          {
            *process << par;
            par = "";
          } 
          else
          {
            if (params[0]=='"')
            {
              params.remove(0,1);
              while (!params.isEmpty() && params[0] != '"')
              {
                par += params[0];
                params.remove(0,1);
              } 
            }
            else
            {
              par += params[0];
            }
          }
          params.remove(0,1);
	}
      *process << par;
       
/*      // set additional parameters
      if (!params.isEmpty())
	{
	  // Note: This will only handle 1 parameter correctly! 
	  *process << params;
	}
*/
      QObject::connect(process, SIGNAL(processExited(KProcess *)), this, SLOT(processExit(KProcess *)));

      // start process
      process->start();
    }
  else
    if (swallowWidget)
      {
	if (visibleWidget && visibleWidget != swallowWidget)
	  visibleWidget->hide();

	swallowWidget->resize(swallowParent->width(), swallowParent->height());
	swallowWidget->raise();
	swallowWidget->show();

	KModuleListEntry::visibleWidget = swallowWidget;
      }

  return TRUE;
}


void KModuleListEntry::processExit(KProcess *proc)
{
  if (proc == process)
    {
      delete process;
      process = 0;

      if (visibleWidget && visibleWidget == swallowWidget)
	visibleWidget = 0;

      delete swallowWidget;
      swallowWidget = 0;
    }
}


void KModuleListEntry::addWindow(Window w)
{
  XTextProperty titleProperty;

  XGetWMName(qt_xdisplay(), w, &titleProperty);

  if (strcmp(getSwallowTitle().data(), (char *) titleProperty.value) == 0)
    {
      if (!swallowWidget)
	{
	  swallowWidget = new KSwallowWidget(swallowParent);

	  if (visibleWidget && visibleWidget != swallowWidget)
	    visibleWidget->hide();
	  visibleWidget = swallowWidget;
	}

      swallowWidget->swallowWindow(w);
      
      // disconnect from KWM events
      disconnect(kapp, SIGNAL(windowAdd(Window)), this, SLOT(addWindow(Window)));
    }  
}


void KModuleListEntry::insertInit(QStrList *list)
{
  // insert init command
  if (!getInit().isEmpty())
    if (!list->contains(getInit()))
      list->append(getInit());

  // process children
  if (children)
    for (KModuleListEntry *current = children->first(); current != 0; current = children->next())
      current->insertInit(list);
}


// ---------------------------------------------------------------
// class ConfigList

ConfigList::ConfigList()
{
  modules = new KModuleListEntry(kapp->kde_appsdir()+"/Settings");
}

ConfigList::~ConfigList()
{
  delete modules;
}


void ConfigList::insertEntry(KTreeList *list, KPath *path, KModuleListEntry *entry, bool root)
{
  ConfigTreeItem *item;

  if (!entry)
    return;

  if (entry->isDirectory())
    {
      // add directory
      item = new ConfigTreeItem(entry);
      item->setText(entry->getName());
      item->setPixmap(&entry->getIcon());
     
      if (path->top())
	list->addChildItem(item, path);
      else 
	if (!root)
	  list->insertItem(item);

      // add children 
      if (!root)
	path->push(&entry->getName());
      for (KModuleListEntry *current = entry->getChildren()->first(); current != 0; current = entry->getChildren()->next())
	{
	  insertEntry(list, path, current, FALSE);
	}
      if (!root)
      path->pop();
    }
  else
    {
      // add application
      item = new ConfigTreeItem(entry);
      item->setText(entry->getName());
      item->setPixmap(&entry->getIcon());
      if (path->count() > 0)
        list->addChildItem(item, path);
      else
        list->insertItem(item);
    }
}


void ConfigList::fillTreeList(KTreeList *list)
{
  KPath path;

  if (modules)
    insertEntry(list, &path, modules, TRUE);
}


void ConfigList::doInit()
{
  QStrList initCommands;
  QString  commands;

  // get initialization commands
  if (modules)
    modules->insertInit(&initCommands);

  // compose command to execute
  for (char *current = initCommands.first(); current != 0; current = initCommands.next())
    {
      if (!commands.isEmpty())
	commands.append("; ");
      commands.append(current);
    }

  // execute initialization commands
  system(commands.data());
}
