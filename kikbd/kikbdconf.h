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
  QString   charset;  // layout charset, may be unset
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
  const QString getInfo     () const;
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
  enum {Place_TopLeft, Place_TopRight, Place_BottomLeft, Place_Bottom_Right};
 protected:
  QList<KiKbdMapConfig> allMaps;
  QStrList        maps;
  bool            keyboardBeep, hotList, autoStart, docking;
  bool            autoMenu, emuCapsLock, custFont, saveClasses;
  int             input, autoStartPlace;
  QString         switchComb, altSwitchComb;
  QColor          capsColor, altColor, forColor;
  QFont           font;
 public:
  KiKbdConfig();
  ~KiKbdConfig(){}
  /*--- bool values ---*/
  const bool&    getKeyboardBeep() const {return keyboardBeep; }
  const bool&    getAutoMenu    () const {return autoMenu;     }
  const bool&    getEmuCapsLock () const {return emuCapsLock;  }
  const bool&    getCustFont    () const {return custFont;     }
  const bool&    getSaveClasses () const {return saveClasses;  }
  const bool&    getAutoStart   () const {return autoStart;    }
  const bool&    getDocking     () const {return docking;      }
  const bool&    getHotList     () const {return hotList;      }
  const int&     getInput       () const {return input;        }
  const QColor&  getCapsColor   () const {return capsColor;    }
  const QColor&  getAltColor    () const {return altColor;     }
  const QColor&  getForColor    () const {return forColor;     }
  const QFont&   getFont        () const {return font;         }
  const int& getAutoStartPlace  () const {return autoStartPlace;}
  QStrList& getMaps() {return maps;}
  /*--- switches ---*/
  QStrList getSwitch()    const {
    return KObjectConfig::separate(switchComb, '+');
  }
  QStrList getAltSwitch() const {
    return KObjectConfig::separate(altSwitchComb, '+');
  }
  /*--- combo values ---*/
  QWidget* switchWidget(QWidget* parent, const char* tip) {
    return createWidget(&switchComb, parent, 0L, tip);
  }
  QWidget* altSwitchWidget(QWidget* parent, const char* tip) {
    return createWidget(&altSwitchComb, parent, 0L, tip);
  }
  KiKbdMapConfig* getMap(const char* name);
  bool            hasAltKeys();
  bool            oneKeySwitch() const;
  void            setDefaults();
  virtual void loadConfig();
 public:
  static bool     readAutoStart();
  static QStrList availableMaps();
 public slots:
  void newUserRc();
  void olderVersion(float);
  void newerVersion(float);
};

class KiKbdMsgBox {
 public:
  static void error(const char* form, const char* s1=0L, const char *s2=0L);
  static void warning(const char* form, const char* s1=0L, const char *s2=0L);
  static int  yesNo(const char* form, const char* s1=0L, const char *s2=0L);
};

#define gettext(text) text

#endif
