#ifndef KEYMAP_H
#define KEYMAP_H
#include <qstring.h>
#include <qobject.h>
#include "kikbdconf.h"
#include <X11/Xlib.h>

/**
   class to get and to write back symbols to X server
*/
class KeySyms {
 protected:
  KeySym *syms;
  unsigned  minKeyCode, maxKeyCode, kcodes;
  void allocSyms(int, int, int);
  friend class KeyMap;
 public:
  KeySyms();
  void read();
  void write();
  int  findSym(const char*);
  int  findCode(const char*);
  void change(int, const char*, int);
  void changeSym(const char* f, const char* s, int o) {
    change(findSym(f), s, o);
  }
  void changeCode(const char* f, const char* s, int o) {
    change(findCode(f), s, o);
  }
  void expandCodes(unsigned);
  bool isNull() {return syms == 0L;}
  KeySyms& operator=(KeySyms&);
 public:
  static KeySym stringToSym(const char*);
};

/**
   class to hold information and symbols for each language
*/
class KeyMap: public QObject {
  Q_OBJECT
 protected:
  static  bool isToggleCaps;
  QString name, label, comment;
  bool    altKeys;
  KeySyms keySyms, capsKeySyms;
 public:
  KeyMap(KiKbdMapConfig&, KeySyms& initSyms);
  bool  changeKeySym(const char*, const char*, int);
  const QString getName()    const { return name;    }
  const QString getLabel()   const { return label;   }
  const QString getComment() const { return comment; }
  bool  hasAltKeys() const {return altKeys;}
  void  toggle();
  void  toggleCaps(bool);
 signals:
  void infoChanged(QString, QString);
};

#endif
