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
#include <qchkbox.h>
#include <qcombo.h>
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qradiobt.h>

#include <kcolordlg.h>
#include <kfontdialog.h>
#include <kcolorbtn.h>
#include <ksimpleconfig.h>
#include <kapp.h>

#include "kobjconf.h"
#include "kconfobjs.h"
#include "kconfobjs.moc"

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
QWidget* KConfigBoolObject::createWidget(QWidget* parent,
					 const char* label) 
{
  QWidget *wid = new QCheckBox(i18n(label), parent);
  wid->setMinimumSize(wid->sizeHint());
  connect(wid, SIGNAL(toggled(bool)), SLOT(setData(bool)));
  return controlWidget(wid), wid;
}
void KConfigBoolObject::setWidget(QWidget* wid)
{
  ((QCheckBox*)wid)->setChecked(*(bool*)data);  
}
void KConfigBoolObject::setData(bool f) 
{
  *(bool*)data = f;
  emit dataChanged();
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
				       const char** vlist,
				       unsigned num, const char** vlabels,
				       int type)
  :KConfigObject(&val, FALSE, key),
   labels(vlabels ? new QStrList() : 0L),
   pstring(0L), pindex(&val), num(num), type(type)
   
{
  for(unsigned i=0; i<num; i++) {
    if(vlist  ) list.append(vlist[i]);
    if(vlabels) labels->append(vlabels[i]);
  }
}
KConfigComboObject::KConfigComboObject(const char* key, int& val,
				       const QStrList& vlist,
				       unsigned num, const QStrList* vlabels,
				       int type)
  :KConfigObject(&val, FALSE, key), list(vlist),
   labels(vlabels ? new QStrList(*vlabels) : 0L),
   pstring(0L), pindex(&val), num(num), type(type)
{
}
KConfigComboObject::KConfigComboObject(const char* key, QString& val,
				       const char** vlist,
				       unsigned num, const char** vlabels,
				       int type)
  :KConfigObject(&val, FALSE, key),
   labels(vlabels ? new QStrList() : 0L),
   pstring(&val), pindex(0L), num(num), type(type)
{
  for(unsigned i=0; i<num; i++) {
    if(vlist  ) list.append(vlist[i]);
    if(vlabels) labels->append(vlabels[i]);
  }
}
KConfigComboObject::KConfigComboObject(const char* key, QString& val,
				       const QStrList& vlist,
				       unsigned num, const QStrList* vlabels,
				       int type)
  :KConfigObject(&val, FALSE, key), list(vlist),
   labels(vlabels ? new QStrList(*vlabels) : 0L),
   pstring(&val), pindex(0L), num(num), type(type)
{
}
KConfigComboObject::~KConfigComboObject()
{
  if(labels) delete labels;
}
int KConfigComboObject::getIndex(const QString& string) const
{
  return ((QStrList*)&list)->find(string);
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
QWidget* KConfigComboObject::createWidget(QWidget* parent, 
					  const char* name) 
{
  QWidget *wid = 0L;
  unsigned i;
  switch(type) {
  case Combo : 
    wid = new QComboBox(parent);
    for(i=0; i<num; i++)
      ((QComboBox*)wid)->
	insertItem(i18n(labels?labels->at(i):list.at(i)));
    wid->setMinimumSize(wid->sizeHint());
    connect(wid, SIGNAL(activated(int)), SLOT(setData(int)));
    break;
  case ButtonGroup :
    wid = new QButtonGroup(i18n(name), parent);
    int height = 0;
    unsigned i;for(i=0; i<num; i++) {
      QRadioButton *but = 
	new QRadioButton(i18n(labels?labels->at(i):list.at(i)),
			 (QButtonGroup*)wid);
      but->setMinimumSize(but->sizeHint());
      height = but->height();
    }
    wid->setMinimumHeight(2*height);
    connect(wid, SIGNAL(clicked(int)), SLOT(setData(int)));
    break;
  }
  return controlWidget(wid), wid;
}
void KConfigComboObject::setWidget(QWidget* wid)
{
  switch(type) {
  case Combo : 
    ((QComboBox*)wid)->setCurrentItem(getIndex());
    break;
  case ButtonGroup :
    ((QButtonGroup*)wid)->setButton(getIndex());
    break;
  }
}
void KConfigComboObject::setData(int i)
{
  setIndex(i);
  emit dataChanged();
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
QWidget* KConfigColorObject::createWidget(QWidget* parent, const char*)
{
  QWidget *wid = new KColorButton(parent);
  connect(wid, SIGNAL(changed(const QColor&)),
	  SLOT(setData(const QColor&)));
  return controlWidget(wid), wid;
}
void KConfigColorObject::setWidget(QWidget* wid)
{
  ((KColorButton*)wid)->setColor(*(QColor*)data);
}
void KConfigColorObject::setData(const QColor& color)
{
  *((QColor*)data) = color;
  emit dataChanged();
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
					 const char* label) 
{
  QWidget *wid = new QPushButton(i18n(label), parent);
  connect(wid, SIGNAL(clicked()), SLOT(activated()));
  return controlWidget(wid), wid;
}
void KConfigFontObject::activated()
{
  QFont font = *(QFont*)data;
  if(KFontDialog::getFont(font)) {
    *(QFont*)data = font;
    emit dataChanged();
  }
}
void KConfigFontObject::setWidget(QWidget* wid)
{
  wid->setFont(*(QFont*)data);
  wid->setMinimumSize(wid->sizeHint());
}
