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
#include <iostream>
#include <stdio.h>
#include <qkeycode.h>
#include <qregexp.h>
#include <X11/keysymdef.h>
#include "keytrans.h"

using namespace std;

KeySym KeyTranslate::stringOrNumToCode(const char* s)
{
  unsigned code = 0;
  if(sscanf(s, "%d", &code) != 1)
    return (KeySym)-1;
  return code;
}
KeySym KeyTranslate::stringToSym(const char* s)
{
  return XStringToKeysym(s);
}
KeySym KeyTranslate::stringOrHexToSym(const char* s)
{
  QRegExp r = QRegExp("0x[a-f0-9]+", FALSE); // hexdecimal number
  QString str = s;
  if(str.contains(r) == 1) {
    unsigned int hex;
    sscanf(str, "%x", &hex);
    return hex;
  }
  return XStringToKeysym(str);
}
KeySym KeyTranslate::tolower(KeySym s)
{
  return ((s>='A')&&(s<='Z'))?(s-'A'+'a'):s;
}
