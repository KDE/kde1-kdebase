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
  QList<QStrList> keysyms[4], keycodes[4];
  QStrList  capssyms[4];
  QString   name;     // the name of the layout, alwais defined
  QString   label;    // layout label, must be set
  QString   locale;   // layout locale, must be set
  QStrList  authors;  // authors of the layout, must be set. How create this?
  QString   language; // language of layout, must be set (or "unknown")
  QString   comment;  // layout comment, may be unset (will be "No comment")
  QString   charset;  // layout charset, may be unset
  bool      hasAltKeys, hasCompose;
  bool      userData, noFile;
  int       numberOfSyms;
 public:
  KiKbdMapConfig(const char*);
  ~KiKbdMapConfig(){}
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
  const bool getHasCompose  () const {return hasCompose;}
  const bool getUserData    () const {return userData;}
  const bool getNoFile      () const {return noFile;}
  const unsigned getNumberOfSyms() const {return numberOfSyms;}
  const QColor  getColor() const;
  const QPixmap getIcon () const;
  QList<QStrList>& getKeysyms(unsigned i) {return keysyms[i];}
  QList<QStrList>& getKeycodes(unsigned i){return keycodes[i];}
  QStrList&        getCapssyms(unsigned i){return capssyms[i];}
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
  QString         switchComb, altSwitchComb, codes;
  QColor          capsColor, altColor, forColor, bakColor;
  QFont           font;
 public:
  KiKbdConfig(bool readOnly = TRUE);
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
  const QColor&  getBakColor    () const {return bakColor;     }
  const QFont&   getFont        () const {return font;         }
  const int& getAutoStartPlace  () const {return autoStartPlace;}
  const QString& getCodes       () const {return codes;}
  const QString& getSwitchRef   () const {return switchComb;}
  const QString& getAltSwitchRef() const {return altSwitchComb;}
  QStrList getSwitch() const {
    return KObjectConfig::separate(switchComb, '+');
  }
  QStrList& getMaps() {return maps;}
  KiKbdMapConfig* getMap(const char* name);
  bool            hasAltKeys();
  bool            hasCompose();
  bool            oneKeySwitch() const;
  void            setDefaults();
  virtual void loadConfig();
 public:
  enum {Config_Normal=0, Config_StartKikbd};
  static bool     readAutoStart();
  static QStrList availableMaps(const char* subdir = "");
  static void     startConfigProgram(int opt=Config_Normal);
  static bool     isConfigProgram();
 public slots:
  void newUserRc();
 //CT 17Jan1999
 //  void olderVersion(float);
 //  void newerVersion(float);
 void wrongVersion(int);
};
extern KiKbdConfig *kikbdConfig;

class KiKbdMsgBox {
 public:
  static void error(const char* form, const char* s1=0L, const char *s2=0L);
  static void warning(const char* form, const char* s1=0L, const char *s2=0L);
  static int  yesNo(const char* form, const char* s1=0L, const char *s2=0L);
  static void ask(const char* form, const char* s1=0L, const char *s2=0L);
  static void askContinue(const char* form, const char* s1=0L, const char *s2=0L);
};

#define gettext(text) text

#endif
