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
#include <ksimpleconfig.h>


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
    if(entry.isEmpty()) break;
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
  *((bool*)data) = config->getConfig()->
    readBoolEntry(keys.current(), *((bool*)data));
}
void KConfigBoolObject::writeObject(KObjectConfig* config)
{
  config->getConfig()->writeEntry(keys.current(), *((bool*)data));
}
void KConfigBoolObject::toggled(bool val){*((bool*)data) = val;}
QWidget* KConfigBoolObject::createWidget(QWidget* parent,
					 const char* label) const
{
  QCheckBox* box = new QCheckBox(i18n(label), parent);
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
  *((int*)data) = config->getConfig()->
    readNumEntry(keys.current(), *((int*)data));
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
  *((QString*)data) = config->getConfig()->
    readEntry(keys.current(), *((QString*)data));
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
KConfigComboObject::KConfigComboObject(const char* key, int& val,
				       const char** list,
				       unsigned num, const char** labels,
				       int type)
  :KConfigObject(&val, FALSE, key)
{
  this->list   = list;
  this->labels = labels;
  this->type   = type;
  this->num    = num;
  pindex  = &val;
  pstring = 0L;
}
KConfigComboObject::KConfigComboObject(const char* key, QString& val,
				       const char** list,
				       unsigned num, const char** labels,
				       int type)
  :KConfigObject(&val, FALSE, key)
{
  this->list   = list;
  this->labels = labels;
  this->type   = type;
  this->num    = num;
  pstring = &val;
  pindex  = 0L;
}
int KConfigComboObject::getIndex(const QString& string) const
{
  unsigned i;for(i=num; --i>0 && string!=list[i];);
  return i;
}
QWidget* KConfigComboObject::createWidget(QWidget* parent, 
					  const char* name) const
{
  QWidget *w = 0L;

  switch(type) {
  case Combo : 
    {
      QComboBox* box = new QComboBox(parent);
      unsigned i;for(i=0; i<num; i++)
	box->insertItem(klocale->
			translate(labels&&labels[i]?labels[i]:list[i]));
      box->setCurrentItem(getIndex());
      box->setMinimumSize(box->sizeHint());
      connect(box, SIGNAL(activated(int)), SLOT(activated(int)));
      w = box;
    }
    break;
  case ButtonGroup :
    {
      QButtonGroup* box = new QButtonGroup(i18n(name), parent);
      int height = 0;
      unsigned i;for(i=0; i<num; i++) {
	QRadioButton *but = 
	  new QRadioButton(i18n(labels && labels[i]?labels[i]
					      :list[i]), box);
	but->setMinimumSize(but->sizeHint());
	height = but->height();
      }
      ((QRadioButton*)box->find(getIndex()))->setChecked(TRUE);
      box->setMinimumHeight(2*height);
      connect(box, SIGNAL(clicked(int)), SLOT(activated(int)));
      w = box;
    }
    break;
  }
  return w;
}
void KConfigComboObject::readObject(KObjectConfig* config)
{
  QString tmp = config->getConfig()->readEntry(keys.current(), getString());
  if(pindex) *pindex = getIndex(tmp); else *pstring = tmp;
}
void KConfigComboObject::writeObject(KObjectConfig* config)
{
  config->getConfig()->writeEntry(keys.current(), getString());
}

/***********************************************************************
 * Color List Object
 */
void KConfigColorObject::readObject(KObjectConfig* config)
{
  *((QColor*)data) = config->getConfig()->
    readColorEntry(keys.current(), ((QColor*)data));
}
void KConfigColorObject::writeObject(KObjectConfig* config)
{
  config->getConfig()->writeEntry(keys.current(), *((QColor*)data));
}
QWidget* KConfigColorObject::createWidget(QWidget* parent, const char*) const
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
  *((QFont*)data) = config->getConfig()->
    readFontEntry(keys.current(), ((QFont*)data));
}
void KConfigFontObject::writeObject(KObjectConfig* config)
{
  config->getConfig()->writeEntry(keys.current(), *((QFont*)data));
}
QWidget* KConfigFontObject::createWidget(QWidget* parent, 
					 const char* label) const
{
  QPushButton *button = new QPushButton(i18n(label), parent);
  connect(button, SIGNAL(clicked()), SLOT(activated()));
  return button;
}
void KConfigFontObject::activated()
{
  QFont font = *((QFont*)data);
  if(KFontDialog::getFont(font)) *((QFont*)data) = font;
}

/*************************************************************************
   Object Configuration 
   
**************************************************************************/
QStrList KObjectConfig::separate(const char* s, char sep)
{
  QString  string(s);
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
  version      = -1.0;
}
KObjectConfig::KObjectConfig(int type, const char* name)
{
  init();
  configType = type;
  configFile = name;
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
  if(version >= 0.0) {
    config->setGroup(configGroup);
    double newversion = config->readDoubleNumEntry(configVersion, 0.0);
    if(int(newversion) > int(version)) emit newerVersion(newversion);
    else if(int(newversion) < int(version)) emit olderVersion(newversion);
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
				     const char* label, const char* tip) const
{
  KConfigObject *obj = find(data);
  if(obj) {
    QWidget* widget = obj->createWidget(parent, label);
    if(tip) QToolTip::add(widget, i18n(tip));
    return widget;
  }
  return 0L;
}
