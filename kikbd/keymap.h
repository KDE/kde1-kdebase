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
  KeySym *syms, minKeyCode, maxKeyCode;
  unsigned  kcodes;
  void allocSyms(KeySym, KeySym, unsigned);
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
  KeySyms& operator=(KiKbdMapConfig*);
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
  bool    altKeys, compose;
  KeySyms keySyms[4], capsKeySyms[4];
 public:
  KeyMap(KiKbdMapConfig&, KeySyms& initSyms);
  bool  changeKeySym(const char*, const char*, int, unsigned);
  const QString getName()    const { return name;    }
  const QString getLabel()   const { return label;   }
  const QString getComment() const { return comment; }
  bool  hasAltKeys() const {return altKeys;}
  bool  hasCompose() const {return compose;}
  void  toggle(unsigned);
  void  toggleCaps(bool, unsigned);
 signals:
  void infoChanged(QString, QString);
};

#endif
