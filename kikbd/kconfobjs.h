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
#ifndef K_CONF_OBJS
#define K_CONF_OBJS

#include <qregexp.h>
#include "kobjconf.h"

/**
   This objects can handle a group of objects with keys matched
   regular expression. As data used QStrList - list of string
   for matched keys. As example if you want to read keys
   
       keyAB=...
       keyBC=..
       ...
       keyAZ=..

   Use this object:

       registerObject(new KConfigMatchKeysObject(QRegExp("key[A-Z][A-Z]",
           list, ',');
*/
class KConfigMatchKeysObject: public KConfigObject {
 protected:
  QRegExp  regexp;
  /**
     KConfigObject reimplemented read data method
  */
  virtual void readObject(KObjectConfig*);
  /**
     KConfigObject reimplemented write data method
  */
  virtual void writeObject(KObjectConfig*);
 public:
  static QStrList separate(const char* string, char sep=',');
  /**
     Create new object to read all keys matched regular expression.
     @param match The Regular Expression
     @param list  QStrList variable to read/write data. Each matched key
     readed as QString item of list.
  */
  KConfigMatchKeysObject(const QRegExp& match, QStrList& list);
  QStrList separate(int index, char sep=','){
    return separate(((QStrList*)data)->at(index), sep);
  }
};





/**
   This objects can handle a group of objects with keys numbered
   without holes from start number to end. As data used QStrList 
   - list of string for matched keys. As example if you want to read keys
   
       key0=...
       key1=..
       ...
       key9=..

   Use this object:

       registerObject(new KConfigNumberedKeysObject("key", 0, 9, list, ',');
*/
class KConfigNumberedKeysObject: public KConfigObject {
 protected:
  unsigned from, to;
  QString  keybase;
  virtual  void readObject(KObjectConfig*);
  virtual  void writeObject(KObjectConfig*);
 public:
  KConfigNumberedKeysObject(const char* pKeybase, unsigned pFrom, unsigned pTo,
			    QStrList& list);
};





/**
  * Boolean Config Object
  */
class KConfigBoolObject: public KConfigObject {
  Q_OBJECT
protected:
  /**
     KConfigObject reimplemented read data method
  */
  void readObject(KObjectConfig*);
  /**
     KConfigObject reimplemented write data method
  */
  void writeObject(KObjectConfig*);
public:
  KConfigBoolObject(const char* key, bool& val)
    :KConfigObject(&val, FALSE, key){}
  QWidget* createWidget(QWidget* parent=0L, const char* label=0L);
public slots:
  void toggled(bool);
};





/**
  * Int Config Object
  */
class KConfigIntObject: public KConfigObject {
  Q_OBJECT
protected:
  /**
     KConfigObject reimplemented read data method
  */
  void readObject(KObjectConfig*);
  /**
     KConfigObject reimplemented write data method
  */
  void writeObject(KObjectConfig*);
public:
  KConfigIntObject(const char* key, int& val)
    :KConfigObject(&val, FALSE, key){}
};





/**
  OString Config object. 
 */
class KConfigStringObject: public KConfigObject {
  Q_OBJECT
protected:
  /**
     KConfigObject reimplemented write data method
  */
  void readObject(KObjectConfig*);
  /**
     KConfigObject reimplemented write data method
  */
  void writeObject(KObjectConfig*);
public:
  KConfigStringObject(const char* key, QString& val)
    :KConfigObject(&val, FALSE, key){}
};





/**
  * String List Config Object
  */
class KConfigStrListObject: public KConfigObject {
  Q_OBJECT
protected:
  char sep;
  /**
     KConfigObject reimplemented write data method
  */
  void readObject(KObjectConfig*);
  /**
     KConfigObject reimplemented write data method
  */
  void writeObject(KObjectConfig*);
public:
  KConfigStrListObject(const char* key, QStrList& val, char pSep=',')
    :KConfigObject(&val, FALSE, key){sep = pSep;}
};





/**
   Object of data type QString witch can have as value one of the
   given string from the list.
*/
class KConfigComboObject: public KConfigStringObject {
  Q_OBJECT
protected:
  QStrList combo;
public:
  /**
     Construct new object with list of available values given by QStrList.
  */
  KConfigComboObject(const char* key, QString& val, const QStrList& pCombo)
    :KConfigStringObject(key, val), combo(pCombo) {
  }
  /**
     Construct new object with list of available values given by array
     of NULL terminated character strings.
     @param list array of NULL terminated strings
     @param num size of the list array
  */
  KConfigComboObject(const char* key, QString& val, const char** list,
		     unsigned num);
  /**
     Create QComboBox widget to change value of the KConfigComboObject
     data.
     @param list if not null, used as strings to show in QComboBox instead
     of ones used as values. Must be of size given in constructor.
  */
  QWidget* createWidget(QWidget* parent=0L, const char** list=0L);
  /**
     Create QButtonGroup widget to change value of the KConfigComboObject
     data.
     @param list if not null, used as strings to show in QComboBox instead
     of ones used as values. Must be of size given in constructor.
     @param name The name of QButtonGroup.
  */
  QWidget* createWidget2(QWidget* parent=0L, const char** list=0L,
			 const char *name=0L);
public slots:
    /**
       Slot used to connect widgets.
    */
  void activated(int index){
    *((QString*)data) = combo.at(index);
  }
};




/**
   OColor Object
 */
class KConfigColorObject: public KConfigObject {
  Q_OBJECT
protected:
  /**
     KConfigObject reimplemented write data method
  */
  void readObject(KObjectConfig*);
  /**
     KConfigObject reimplemented write data method
  */
  void writeObject(KObjectConfig*);
public:
  /**
     Construct new Color object.
  */
  KConfigColorObject(const char* key, QColor& val)
    :KConfigObject(&val, FALSE, key){}
  /**
     Create colored button connected to KColorDialog.
  */
  virtual QWidget* createWidget(QWidget* parent=0L);
public slots:
  void changed(const QColor&);
};




/**
  * QFont object
  */
class KConfigFontObject: public KConfigObject {
  Q_OBJECT
protected:
  /**
     KConfigObject reimplemented write data method
  */
  void readObject(KObjectConfig*);
  /**
     KConfigObject reimplemented write data method
  */
  void writeObject(KObjectConfig*);
public:
  /**
     Construct new Font object.
  */
  KConfigFontObject(const char* key, QFont& val)
    :KConfigObject(&val, FALSE, key){}
  /**
     Create unlabeled QPushButton connected to KFontDialog.
  */
  virtual QWidget* createWidget(QWidget* parent=0L);
public slots:
  void activated();
};

#endif
