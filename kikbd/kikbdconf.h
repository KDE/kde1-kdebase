#ifndef KIKBDCONF_H
#define KIKBDCONF_H

#include <kapp.h>
#include "kobjconf.h"

class KiKbdMapConfig: public QObject {
  Q_OBJECT
 protected:
  QList<QStrList> keysyms, keycodes;
  QStrList  capssyms;
  QString   name, label, comment, longcomment, locale, authors;
  bool      hasAltKeys;
  bool      userData;
  int       numberOfSyms;
 public:
  KiKbdMapConfig(const char*);
  friend class KiKbdConfig;
  /*--- public information ---*/
  const QString getName     () const {return name;}
  const QString getAuthors  () const {return authors;}
  const QString getLabel    () const {return label;}
  const QString getLocale   () const {return locale;}
  const QString getComment  () const {return comment;}
  const QString getLongComment() const {
    return (longcomment.isNull())?comment:longcomment;
  }
  const QString getGoodLabel() const;
  const bool getHasAltKeys  () const {return hasAltKeys;}
  const bool getUserData    () const {return userData;}
  const unsigned getNumberOfSyms() const {return numberOfSyms;}
  QList<QStrList>& getKeysyms() {return keysyms;}
  QList<QStrList>& getKeycodes(){return keycodes;}
  QStrList&        getCapssyms(){return capssyms;}
 public slots:
  void noUserDataFile(const char*){userData = false;}
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
};

#endif
