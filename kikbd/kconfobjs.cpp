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
#include "kconfobjsw.h"
#include "kconfobjs.moc.h"
#include "kconfobjsw.moc.h"

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
  return (new KConfigBoolWidget(this, label, parent))->getWidget();
}
// widget
KConfigBoolWidget::KConfigBoolWidget(KConfigBoolObject* obj, const char* label,
				     QWidget* parent=0L)
  :KConfigObjectWidget(parent, obj)
{
  widget = new QCheckBox(i18n(label), parent);
  widget->setMinimumSize(widget->sizeHint());
  connect(widget, SIGNAL(clicked()), SLOT(changeData()));
  dataChanged();
}
void KConfigBoolWidget::dataChanged()
{
  ((QCheckBox*)widget)->setChecked(*(bool*)(object->getData()));
}
void KConfigBoolWidget::changeData()
{
  *(bool*)(object->getData()) = ((QCheckBox*)widget)->isChecked();
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
  return (new KConfigComboWidget(this, name, parent))->getWidget();
}
// widget
KConfigComboWidget::KConfigComboWidget(KConfigComboObject* obj,
				       const char* name,
				       QWidget* parent=0L)
  :KConfigObjectWidget(parent, obj)
{
  KConfigComboObject* o = (KConfigComboObject*)object;
  switch(o->type) {
  case KConfigComboObject::Combo : 
    {
      QComboBox* box = new QComboBox(parent);
      unsigned i;for(i=0; i<o->num; i++)
	box->insertItem(i18n(o->labels&&o->labels[i]
			     ?o->labels[i]:o->list[i]));
      box->setMinimumSize(box->sizeHint());
      connect(box, SIGNAL(activated(int)), SLOT(changeData(int)));
      widget = box;
    }
    break;
  case KConfigComboObject::ButtonGroup :
    {
      QButtonGroup* box = new QButtonGroup(i18n(name), parent);
      int height = 0;
      unsigned i;for(i=0; i<o->num; i++) {
	QRadioButton *but = 
	  new QRadioButton(i18n(o->labels&&o->labels[i]?o->labels[i]
				:o->list[i]), box);
	but->setMinimumSize(but->sizeHint());
	height = but->height();
      }
      box->setMinimumHeight(2*height);
      connect(box, SIGNAL(clicked(int)), SLOT(changeData(int)));
      widget = box;
    }
    break;
  }
  dataChanged();
}
void KConfigComboWidget::dataChanged()
{
  KConfigComboObject* o = (KConfigComboObject*)object;
  switch(o->type) {
  case KConfigComboObject::Combo : 
    ((QComboBox*)widget)->setCurrentItem(o->getIndex());
    break;
  case KConfigComboObject::ButtonGroup :
    ((QButtonGroup*)widget)->setButton(o->getIndex());
    break;
  }
}
void KConfigComboWidget::changeData()
{
  int i = 0;
  switch(((KConfigComboObject*)object)->type) {
  case KConfigComboObject::Combo : 
    i = ((QComboBox*)widget)->currentItem();
    break;
  case KConfigComboObject::ButtonGroup :
    for(i=0; ((QButtonGroup*)widget)->find(i)
	  && !((QButtonGroup*)widget)->find(i)->isOn(); i++);
    break;
  }
  ((KConfigComboObject*)object)->setIndex(i);
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
  return (new KConfigColorWidget(this, parent))->getWidget();
}
void KConfigColorObject::changed(const QColor& newColor)
{
  *((QColor*)data) = newColor;
}
// widget
KConfigColorWidget::KConfigColorWidget(KConfigColorObject* obj,
				       QWidget* parent=0L)
  :KConfigObjectWidget(parent, obj)
{
  widget = new KColorButton(parent);
  connect(widget, SIGNAL(changed(const QColor&)),
	  SLOT(changeData(const QColor&)));
  dataChanged();
}
void KConfigColorWidget::dataChanged()
{
  ((KColorButton*)widget)->setColor(*(QColor*)object->getData());
}
void KConfigColorWidget::changeData()
{
  *(QColor*)object->getData() = ((KColorButton*)widget)->color();
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
  return (new KConfigFontWidget(this, label, parent))->getWidget();
}
// widget
KConfigFontWidget::KConfigFontWidget(KConfigFontObject* obj,
				     const char* label,
				     QWidget* parent=0L)
  :KConfigObjectWidget(parent, obj)
{
  widget = new QPushButton(i18n(label), parent);
  connect(widget, SIGNAL(clicked()), SLOT(activated()));
  dataChanged();
}
void KConfigFontWidget::dataChanged()
{
  widget->setFont(*((QFont*)object->getData()));
}
void KConfigFontWidget::changeData()
{
  *((QFont*)object->getData()) = widget->font();
}
void KConfigFontWidget::activated()
{
  QFont font = *((QFont*)object->getData());
  if(KFontDialog::getFont(font)) {
    widget->setFont(font);
    changeData();
  }
}

