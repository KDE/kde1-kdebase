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
#include <stream.h>
#include <qfileinf.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qradiobt.h>

#include <kapp.h>
#include "kobjconf.h"
#include "kconfobjs.h"
#include <kcolordlg.h>
#include <kfontdialog.h>
#include <kcolorbtn.h>


const char *configGroup   = "KObjectConfig";
const char *configVersion = "Version";

/********************************************************************
 * Configuration objects
 */
KConfigObject::KConfigObject(void* pData, bool pDeleteData, const char* key)
{
  group = 0L;  data  = pData;
  deleteData = pDeleteData;
  keys.setAutoDelete(TRUE);
  if(key) keys.append(key);
  configure();
}
KConfigObject::~KConfigObject()
{
  if(deleteData) delete data;
}






/*********************************************************************
 * regexp matched keys Config Object
 */
KConfigMatchKeysObject::KConfigMatchKeysObject(const QRegExp& match,
					       QStrList& list)
  :KConfigObject(&list, FALSE), regexp(match)
{
}
void KConfigMatchKeysObject::readObject(KObjectConfig* config)
{
  QStrList &list = *((QStrList*)data);
  list.clear();
  keys.clear();
  KEntryIterator* it = config->getConfig()->entryIterator(config->group());
  if(it) {
    for(it->toFirst(); it->current(); ++(*it)) {
      if(regexp.match(it->currentKey()) != -1)
	if(it->current()->aValue) {
	  keys.append(it->currentKey());
	  list.append(it->current()->aValue);
	}
    }
  }
}
void KConfigMatchKeysObject::writeObject(KObjectConfig* config)
{
  QStrList &list = *((QStrList*)data);
  unsigned i;for(i=0; i<keys.count(); i++)
    config->getConfig()->writeEntry(keys.at(i), list.at(i));
}
QStrList KConfigMatchKeysObject::separate(const char* s, char sep=',')
{
  QString  string(s);
  QStrList list;
  int i, j;for(i=0;j=string.find(sep, i), j != -1; i=j+1)
    list.append(string.mid(i, j-i));
  QString last = string.mid(i, 1000);
  if(!last.isNull() && last != "") list.append(last);
  return list;
}





/**********************************************************************
 * numbered keys Config Object
 */
KConfigNumberedKeysObject::KConfigNumberedKeysObject(const char* pKeybase,
						     unsigned pFrom,
						     unsigned pTo,
						     QStrList& list)
  :KConfigObject(&list, FALSE), keybase(pKeybase)
{
  from = pFrom, to = pTo;
}
void KConfigNumberedKeysObject::readObject(KObjectConfig* config)
{
  QStrList &list = *((QStrList*)data);
  list.clear();
  keys.clear();
  unsigned i;for(i=from; i<to; i++) {
    QString num;
    QString key = keybase + num.setNum(i);
    QString entry = config->getConfig()->readEntry(key);
    if(entry.isNull() || entry == "") break;
    keys.append(key);
    list.append(entry);
  }
}
void KConfigNumberedKeysObject::writeObject(KObjectConfig* config)
{
  QStrList &list = *((QStrList*)data);
  unsigned i;for(i=0; i<=list.count(); i++) {
    QString num;
    QString key = keybase + num.setNum(i);
    config->getConfig()->writeEntry(key, i>=list.count()?"":list.at(i));
  }
}





/**********************************************************************
 * Bool Object
 */
void KConfigBoolObject::readObject(KObjectConfig* config)
{
  *((bool*)data) = config->getConfig()->readBoolEntry(keys.current(), *((bool*)data));
}
void KConfigBoolObject::writeObject(KObjectConfig* config)
{
  config->getConfig()->writeEntry(keys.current(), *((bool*)data));
}
void KConfigBoolObject::toggled(bool val)
{
  *((bool*)data) = val;
}
QWidget* KConfigBoolObject::createWidget(QWidget* parent, const char* label)
{
  QCheckBox* box = new QCheckBox(label, parent);
  box->setChecked(*((bool*)data));
  box->setMinimumSize(box->sizeHint());
  connect(box, SIGNAL(toggled(bool)), SLOT(toggled(bool)));
  return box;
}





/***************************************************************************
 * Int Object
 */
void KConfigIntObject::readObject(KObjectConfig* config)
{
  *((int*)data) = config->getConfig()->readNumEntry(keys.current(), *((int*)data));
}
void KConfigIntObject::writeObject(KObjectConfig* config)
{
  config->getConfig()->writeEntry(keys.current(), *((int*)data));
}





/*****************************************************************************
 * String Object
 */
void KConfigStringObject::readObject(KObjectConfig* config)
{
  *((QString*)data) = config->getConfig()->readEntry(keys.current(), *((QString*)data));
}
void KConfigStringObject::writeObject(KObjectConfig* config)
{
  config->getConfig()->writeEntry(keys.current(), *((QString*)data));
}





/******************************************************************************
 * String List Object
 */
void KConfigStrListObject::readObject(KObjectConfig* config)
{
  config->getConfig()->readListEntry(keys.current(), *((QStrList*)data), sep);
}
void KConfigStrListObject::writeObject(KObjectConfig* config)
{
  config->getConfig()->writeEntry(keys.current(), *((QStrList*)data), sep);
}





/******************************************************************************
 * Combo Object
 */
KConfigComboObject::KConfigComboObject(const char* key, QString& val,
				       const char** list,
				       unsigned num)
  :KConfigStringObject(key, val)
{
  unsigned i;for(i=0; i<num; combo.append(list[i++]));
}
QWidget* KConfigComboObject::createWidget(QWidget* parent,
					  const char** list)
{
  QComboBox* box = new QComboBox(parent);
  unsigned i, j;for(i=j=0; i<combo.count(); i++) {
    if(*((QString*)data) == combo.at(i)) j = i;
    if(list && list[i]) box->insertItem(klocale->translate(list[i]));
    else box->insertItem(combo.at(i));
  }
  box->setCurrentItem(j);
  box->setMinimumSize(box->sizeHint());
  connect(box, SIGNAL(activated(int)), SLOT(activated(int)));
  return box;
}
QWidget* KConfigComboObject::createWidget2(QWidget* parent,
					   const char** list,
					   const char* name)
{
  QButtonGroup* box = new QButtonGroup(name, parent);
  int height = 0;
  unsigned i, j;for(i=j=0; i<combo.count(); i++) {
    if(*((QString*)data) == combo.at(i)) j = i;
    QRadioButton *but = new QRadioButton((list && list[i])?klocale->translate(list[i])
					 :combo.at(i), box);
    but->setMinimumSize(but->sizeHint());
    height = but->height();
  }
  ((QRadioButton*)box->find(j))->setChecked(TRUE);
  box->setMinimumHeight(2*height);
  connect(box, SIGNAL(clicked(int)), SLOT(activated(int)));
  return box;
}





/***********************************************************************
 * Color List Object
 */
void KConfigColorObject::readObject(KObjectConfig* config)
{
  *((QColor*)data) = config->getConfig()->readColorEntry(keys.current(), ((QColor*)data));
}
void KConfigColorObject::writeObject(KObjectConfig* config)
{
  config->getConfig()->writeEntry(keys.current(), *((QColor*)data));
}
QWidget* KConfigColorObject::createWidget(QWidget* parent)
{
  KColorButton *button = new KColorButton(*((const QColor*)data), parent);
  connect(button, SIGNAL(changed(const QColor&)),
	  SLOT(changed(const QColor&)));
  return button;
}
void KConfigColorObject::changed(const QColor& newColor)
{
  *((QColor*)data) = newColor;
}





/*************************************************************************
 * Font Object
 */
void KConfigFontObject::readObject(KObjectConfig* config)
{
  *((QFont*)data) = config->getConfig()->readFontEntry(keys.current(), ((QFont*)data));
}
void KConfigFontObject::writeObject(KObjectConfig* config)
{
  config->getConfig()->writeEntry(keys.current(), *((QFont*)data));
}
QWidget* KConfigFontObject::createWidget(QWidget* parent)
{
  QPushButton *button = new QPushButton(parent);
  connect(button, SIGNAL(clicked()), SLOT(activated()));
  return button;
}
void KConfigFontObject::activated()
{
  QFont font = *((QFont*)data);
  if(KFontDialog::getFont(font)) {
    *((QFont*)data) = font;
  }
}



/*************************************************************************
   Object Configuration 
   

**************************************************************************/
KObjectConfig::KObjectConfig(KConfigBase* config, bool autoDelete)
{
  init();
  deleteConfig = autoDelete;
  config = config;
  configFile = 0L;
  configType = External;
  version = -1.0;
}
KObjectConfig::KObjectConfig(int type, const char* name)
{
  init();
  configType   = type;
  configFile   = name;
}
KObjectConfig::~KObjectConfig()
{
  if(deleteConfig && config) delete config;
}
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
  readOnly = FALSE;
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
	    readOnly = ! info.isWritable();
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
void KObjectConfig::setGroup(const char* pGroup)
{
  if(groups.find(pGroup) == -1) groups.append(pGroup);
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
  if(version >= 0.0) {
    config->setGroup(configGroup);
    double newversion = config->readDoubleNumEntry(configVersion, 0.0);
    if(int(newversion) > int(version)) emit newerVersion();
    else if(int(newversion) < int(version)) emit olderVersion();
  }
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
  if(version >= 0.0) {
    QString str(6);
    str.sprintf("%2.2f", version);
    config->setGroup(configGroup);
    config->writeEntry(configVersion, str);
  }
  config->sync();
}
KConfigObject* KObjectConfig::find(void* data)
{
  unsigned i;for(i=0; i<entries.count(); i++)
    if(entries.at(i)->data == data)
      return entries.at(i);
  return 0L;
}
/**
   supported in core objects
*/
void KObjectConfig::registerBool(const char* key, bool& val)
{
  registerObject(new KConfigBoolObject(key, val));
}
void KObjectConfig::registerInt(const char* key, int& val)
{
  registerObject(new KConfigIntObject(key, val));
}
void KObjectConfig::registerString(const char* key, QString& val)
{
  registerObject(new KConfigStringObject(key, val));
}
void KObjectConfig::registerStrList(const char* key, QStrList& val,
				    char pSep)
{
  registerObject(new KConfigStrListObject(key, val, pSep));
}
void KObjectConfig::registerColor(const char* key, QColor& val)
{
  registerObject(new KConfigColorObject(key, val));
}
void KObjectConfig::registerFont(const char* key, QFont& val)
{
  registerObject(new KConfigFontObject(key, val));
}
/**
   supported widgets
*/
QWidget* KObjectConfig::createBoolWidget(bool& val, const char* label,
					 QWidget* parent)
{
  KConfigObject *obj = find(&val);
  if(obj)
    return ((KConfigBoolObject*)obj)->createWidget(parent, label);
  return 0L;
}
QWidget* KObjectConfig::createComboWidget(QString& val, const char** list,
					  QWidget* parent)
{
  KConfigObject *obj = find(&val);
  if(obj)
    return ((KConfigComboObject*)obj)->createWidget(parent, list);
  return 0L;
}
QWidget* KObjectConfig::createComboWidget2(QString& val, const char** list,
					   const char* name, QWidget* parent)
{
  KConfigObject *obj = find(&val);
  if(obj)
    return ((KConfigComboObject*)obj)->createWidget2(parent, list, name);
  return 0L;
}
QWidget* KObjectConfig::createColorWidget(QColor& val, QWidget* parent)
{
  KConfigObject *obj = find(&val);
  if(obj)
    return ((KConfigColorObject*)obj)->createWidget(parent);
  return 0L;
}
QWidget* KObjectConfig::createFontWidget(QFont& val, QWidget* parent)
{
  KConfigObject *obj = find(&val);
  if(obj)
    return ((KConfigFontObject*)obj)->createWidget(parent);
  return 0L;
}

