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
#ifndef KEYTRANS_H
#define KEYTRANS_H
#include <X11/Xlib.h>

class KeyTranslate {
 public:
  static KeySym tolower(KeySym);
  static KeySym stringToSym(const char*);
  static KeySym stringOrHexToSym(const char*);
  static int    stringOrNumToCode(const char*);
  static const char* stringFromSym(KeySym);
};

#endif
