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
#ifndef KIKBDCONF_H
#define KIKBDCONF_H

#include <kapp.h>
#include "kobjconf.h"

class KiKbdMapConfig: public QObject {
  Q_OBJECT
 protected:
  QList<QStrList> keysyms, keycodes;
  QStrList  capssyms;
  QString   name;     // the name of the layout, alwais defined
  QString   label;    // layout label, must be set
  QString   locale;   // layout locale, must be set
  QStrList  authors;  // authors of the layout, must be set. How create this?
  QString   language; // language of layout, must be set (or "unknown")
  QString   comment;  // layout comment, may be unset (will be "No comment")
  QString   charset;  // layout charset, may be unset (will be "unknown")
  bool      hasAltKeys;
  bool      userData, noFile;
  int       numberOfSyms;
 public:
  KiKbdMapConfig(const char*);
  friend class KiKbdConfig;
  /*--- public information ---*/
  const QString getName     () const {return name;}
  const QStrList getAuthors () const {return authors;}
  const QString getLabel    () const {return label;}
  const QString getLocale   () const {return locale;}
  const QString getComment  () const {return comment;}
  const QString getLanguage () const {return language;}
  const QString getCharset  () const {return charset;}
  const QString getGoodLabel() const;
  const QString getInfo     ();
  const bool getHasAltKeys  () const {return hasAltKeys;}
  const bool getUserData    () const {return userData;}
  const bool getNoFile      () const {return noFile;}
  const unsigned getNumberOfSyms() const {return numberOfSyms;}
  const QColor  getColor() const;
  const QPixmap getIcon () const;
  QList<QStrList>& getKeysyms() {return keysyms;}
  QList<QStrList>& getKeycodes(){return keycodes;}
  QStrList&        getCapssyms(){return capssyms;}
 public slots:
  void noUserDataFile(const char*){userData = FALSE;}
  void noSystemDataFile(const char*){noFile = TRUE;}
};

class KiKbdConfig: public KObjectConfig {
  Q_OBJECT
 public:
  enum {Input_Global, Input_Window, Input_Class};
 protected:
  QList<KiKbdMapConfig> allMaps;
  QStrList        maps;
  bool            keyboardBeep, hotList;
  bool            autoMenu, emuCapsLock;
  bool            custFont, saveClasses;
  bool            autoStart, docking;
  QString         input;
  QString         switchComb, altSwitchComb;
  QString         autoStartPlace;
  QColor          capsColor, altColor, forColor;
  QFont           font;
 public:
  KiKbdConfig();
  ~KiKbdConfig(){}
  /*--- bool values ---*/
  bool& getKeyboardBeep() {return keyboardBeep; }
  bool& getAutoMenu    () {return autoMenu;     }
  bool& getEmuCapsLock () {return emuCapsLock;  }
  bool& getCustFont    () {return custFont;     }
  bool& getSaveClasses () {return saveClasses;  }
  bool& getAutoStart   () {return autoStart;    }
  bool& getDocking     () {return docking;      }
  bool& getHotList     () {return hotList;      }
  const QColor  getCapsColor  () const {return capsColor;}
  const QColor  getAltColor   () const {return altColor;}
  const QColor  getForColor   () const {return forColor;}
  const QFont   getFont       () const {return font;}
  int   getInput       ();
  const QString getAutoStartPlace () const {return autoStartPlace;}
  /*--- switches ---*/
  QStrList getSwitch();
  QStrList getAltSwitch();
  /*--- bool widgets ---*/
  QWidget* keyboardBeepWidget(const char* label, QWidget* parent){
    return createBoolWidget(keyboardBeep, label, parent);
  }
  QWidget* hotListWidget(const char* label, QWidget* parent){
    return createBoolWidget(hotList, label, parent);
  }
  QWidget* autoMenuWidget(const char* label, QWidget* parent){
    return createBoolWidget(autoMenu, label, parent);
  }
  QWidget* emuCapsLockWidget(const char* label, QWidget* parent){
    return createBoolWidget(emuCapsLock, label, parent);
  }
  QWidget* custFontWidget(const char* label, QWidget* parent){
    return createBoolWidget(custFont, label, parent);
  }
  QWidget* saveClassesWidget(const char* label, QWidget* parent){
    return createBoolWidget(saveClasses, label, parent);
  }
  QWidget* autoStartWidget(const char* label, QWidget* parent){
    return createBoolWidget(autoStart, label, parent);
  }
  QWidget* dockingWidget(const char* label, QWidget* parent){
    return createBoolWidget(docking, label, parent);
  }
  /*--- combo values ---*/
  QWidget* switchWidget(const char** list, QWidget* parent) {
    return createComboWidget(switchComb, list, parent);
  }
  QWidget* altSwitchWidget(const char** list, QWidget* parent) {
    return createComboWidget(altSwitchComb, list, parent);
  }
  QWidget* autoStartPlaceWidget(const char** list, QWidget* parent) {
    return createComboWidget(autoStartPlace, list, parent);
  }
  QWidget* inputWidget(const char** list, const char* name, QWidget* parent) {
    return createComboWidget2(input, list, name, parent);
  }
  /**
     color values
  */
  QWidget* capsColorWidget(QWidget* parent){
    return createColorWidget(capsColor, parent);
  }
  QWidget* altColorWidget(QWidget* parent){
    return createColorWidget(altColor, parent);
  }
  QWidget* forColorWidget(QWidget* parent){
    return createColorWidget(forColor, parent);
  }
  QWidget* fontWidget(QWidget* parent){
    return createFontWidget(font, parent);
  }
 public:
  static void error(const char*, const char* str=0, const char* str2=0);
  static void message(const char*, const char* str=0);
  /*--- global maps ---*/
  QStrList availableMaps();
  QStrList& getMaps() {return maps;}
  KiKbdMapConfig* getMap(const char* name);
  bool hasAltKeys();
  bool oneKeySwitch();
 public slots:
  void newUserRc();
  void olderVersion();
  void newerVersion();
};

#endif
