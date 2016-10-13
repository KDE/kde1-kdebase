/*
   - 

  written 1998 by Alexander Budnik <budnik@linserv.jinr.ru>
  
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
#include <stdlib.h>	
#include <iostream>
#include <qfileinf.h>
#include <qtooltip.h>

#include <kapp.h>
#include <ksimpleconfig.h>

#include "kobjconf.h"
#include "kobjconf.moc"

const char *configGroup   = "KObjectConfig";
const char *configVersion = "Version";

/********************************************************************
 * Configuration objects
 */
KConfigObject::KConfigObject(void* pData, bool pDeleteData, const char* key)
{
  widgets = 0L;
  group   = 0L;
  data    = pData;
  deleteData = pDeleteData;
  keys.setAutoDelete(TRUE);
  if(key) keys.append(key);
  configure();
  connect(this, SIGNAL(dataChanged()), SLOT(setWidgets()));
}
KConfigObject::~KConfigObject()
{
  if(deleteData) delete data;
  if(widgets) delete widgets;
}
void KConfigObject::configure(bool bPersistent, bool bGlobal, bool bNLS) 
{
  persistent = bPersistent;
  global = bGlobal;
  NLS = bNLS;
}
void KConfigObject::controlWidget(QWidget* wid)
{
  if(!widgets) widgets = new QList<QWidget>();
  widgets->append(wid);
  connect(wid, SIGNAL(destroyed()), SLOT(deleteWidget()));
  setWidget(wid);
}
void KConfigObject::deleteWidget()
{
  if(sender()) widgets->remove((QWidget*)sender());
}
void KConfigObject::setWidgets()
{
  for(unsigned i=0; widgets && i<widgets->count(); i++)
    setWidget(widgets->at(i));
}
void KConfigObject::markDataChanged() {
  static int flag = 0;
  if(flag) return;
  flag++;
  emit dataChanged();
  flag--;
}
QWidget* KConfigObject::createWidget(QWidget* parent, const char* label) 
{
  return 0L;
}
/*************************************************************************
   Object Configuration 
   
**************************************************************************/
QStrList KObjectConfig::separate(const QString& string, char sep)
{
  QStrList list;
  int i, j;for(i=0;j=string.find(sep, i), j != -1; i=j+1)
    list.append(string.mid(i, j-i));
  QString last = string.mid(i, 1000);
  if(!last.isEmpty()) list.append(last);
  return list;
}
KObjectConfig::KObjectConfig(KConfigBase* config, bool autoDelete)
{
  init();
  deleteConfig = autoDelete;
  config       = config;
  configFile   = 0L;
  configType   = External;
  version      = -1;
  readOnly     = FALSE;
}
KObjectConfig::KObjectConfig(int type, const char* name, bool rdOnly)
{
  init();
  configType = type;
  configFile = name;
  //version      = -1; //CT 07Jan1999 - fix for segfault on Alphas
  readOnly   = rdOnly;
}
KObjectConfig::~KObjectConfig(){if(deleteConfig && config) delete config;}
void KObjectConfig::init()
{
  groups.setAutoDelete(TRUE);
  entries.setAutoDelete(TRUE);
  config = 0L;
}

bool nonzeroFile(QString& file) {
    QFileInfo info(file);
    return info.exists() && (info.size() > 0);
}
void KObjectConfig::initConfig()
{
  if(config == 0L && configType != External) {
    QString file;
    switch(configType) {
      case AppRc :
	configFile = kapp->localconfigdir() + "/" + kapp->appName() + "rc";
	if(!nonzeroFile(configFile)) {
	  emit noUserRcFile(configFile);
	  file = kapp->kde_configdir() + "/" + kapp->appName() + "rc";
	  if(nonzeroFile(file)) configFile = file; else emit noSystemRcFile(file);
	}
	break;
      case UserFromSystemRc :
	configFile = kapp->localconfigdir() + "/" + kapp->appName() + "rc";
	if(!nonzeroFile(configFile)) {
	  file = kapp->kde_configdir() + "/" + kapp->appName() + "rc";
	  if(nonzeroFile(file)) {
	    //--- system file exists, reading
	    config = new KSimpleConfig(file, TRUE);
	    loadConfig();
	    delete config;
	  }
	  //--- save new one
	  config = new KSimpleConfig(configFile);
	  saveConfig();
	  emit newUserRcFile();
	}
	break;
      case AppData :
        file = configFile;
	configFile = kapp->localkdedir() + "/share/apps/" + kapp->appName() + "/" + file;
	if(!QFile::exists(configFile)) {
	  emit noUserDataFile(configFile);
	  file = kapp->kde_datadir() + "/" + kapp->appName() + "/" + file;
	  QFileInfo info(file);
	  if(info.isFile()) {
	    configFile = file;
	    if(!readOnly) readOnly = ! info.isWritable();
	  } else {
	    configFile = 0L;
	    emit noSystemDataFile(file);
	  }
	}
	break;    
      case File : break;
      default : ASSERT("KObjectConfig: bad type in constructor");
    }
    if(config == 0L && !configFile.isNull())
      config = new KSimpleConfig(configFile, readOnly);
    deleteConfig = TRUE;
  }
}

/*
 * set current configuration group
 */
int KObjectConfig::setGroup(const char* pGroup)
{
  if(groups.find(pGroup) == -1) groups.append(pGroup);
  return 0;
}
const char* KObjectConfig::group()
{
  return groups.current();
}
void KObjectConfig::registerObject(KConfigObject* obj)
{
  obj->group = groups.current();
  entries.append(obj);
}
void KObjectConfig::loadConfig()
{
  initConfig();
  if(config == 0L) return;
  unsigned i;for(i=0; i<groups.count(); i++) {
    config->setGroup(groups.at(i));
    unsigned j;for(j=0; j<entries.count(); j++)
      if(groups.at(i) == entries.at(j)->group) {
	entries.at(j)->readObject(this);
      }
  }
  /*CT 17Jan1999 - completely unneeded for the moment and very cumbersome
                   take it out
  if(version >= 0) {
    config->setGroup(configGroup);
    int newversion = config->readNumEntry(configVersion, 1);
    if (newversion != version) 
      emit wrongVersion(newversion - version);
  }
  */
}
void KObjectConfig::saveConfig()
{
  initConfig();
  if(config == 0L) return;
  if(readOnly) {
    ASSERT("KObjectConfig: object read only");
    return;
  }
  unsigned i;for(i=0; i<groups.count(); i++) {
    config->setGroup(groups.at(i));
    unsigned j;for(j=0; j<entries.count(); j++)
      if(groups.at(i) == entries.at(j)->group)
	entries.at(j)->writeObject(this);
  }
  if(version >= 0) {
    QString str;
    str.sprintf("%2.1f", version);
    config->setGroup(configGroup);
    config->writeEntry(configVersion, str);
  }
  config->sync();
}
KConfigObject* KObjectConfig::find(const void* data) const
{
  // we need this complicated expression because we know that
  // find does not make sensible changes
  QList<KConfigObject> &entries = *(QList<KConfigObject>*)&this->entries;
  unsigned i;for(i=0; i<entries.count(); i++)
    if(entries.at(i)->data == data)
      return entries.at(i);
  return 0L;
}
/**
   supported widgets
*/
QWidget* KObjectConfig::createWidget(const void* data, QWidget* parent,
				     const char* label, const char* tip)
{
  KConfigObject *obj = find(data);
  if(obj) {
    QWidget* widget = obj->createWidget(parent, label);
    if(tip) QToolTip::add(widget, i18n(tip));
    return widget;
  }
  return 0L;
}
void KObjectConfig::markDataChanged()
{
  unsigned i;for(i=0; i<entries.count(); i++)
    emit entries.at(i)->dataChanged();
  emit dataChanged();
}
void KObjectConfig::objectChanged()
{
}
