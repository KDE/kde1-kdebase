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
#include <stream.h>
#include <string.h>
#include <stdlib.h>
#include <qfile.h>
#include <qregexp.h>
#include <kapp.h>
#include <kconfig.h>

#include "kikbd.h"
#include "keymap.h"
#include "keytrans.h"
#include "kikbdconf.h"
#include "keymap.moc.h"

/**
   constructors
*/
bool KeyMap::isToggleCaps = FALSE;
KeyMap::KeyMap(KiKbdMapConfig& config, KeySyms& initSyms)
{
  /**
     language information
  */
  name    = config.getName();
  label   = config.getLabel();
  comment = config.getGoodLabel();
  altKeys = config.getHasAltKeys();

  /**
     new main symbols
  */
  keySyms = initSyms;

  /**
     load symbols from symbols(k==0) and codes(k==1)
  */
  unsigned i;
  for(unsigned k=0; k<2; k++) {
    QList<QStrList> &symmap = k==0?config.getKeysyms():config.getKeycodes();

    for(i=0; i<symmap.count(); i++) {
      QStrList list = *(symmap.at(i));
      /**
	 entry with less then 2 symbols are bad
      */
      if(list.count() < 2) continue;
      /**
	 change symbols
      */
      int index = k==0?initSyms.findSym(list.at(0))
	:initSyms.findCode(list.at(0));
      if(index == -1)  continue;
      
      unsigned j;for(j=list.count()<=5?list.count():5; j-->1;)
	keySyms.change(index, list.at(j), j-1);
    }
  }

  /**
     create capslocked symbols
  */
  QStrList &capssyms = config.getCapssyms();
  capsKeySyms = keySyms;
  if(capssyms.count() > 0) {
    /**
       special capsed symbols
    */
    for(i=0; i<capssyms.count(); i++) {
      KeySym test = KeyTranslate::stringToSym(capssyms.at(i));
      if(test == NoSymbol) continue;
      /**
	 look for capsed symbol
      */
      unsigned j;for(j=0; j<(initSyms.maxKeyCode-initSyms.minKeyCode-1)*
		       initSyms.kcodes; j+=initSyms.kcodes) {
	if(KeyTranslate::tolower(initSyms.syms[j]) 
	   == KeyTranslate::tolower(test)) {
	  capsKeySyms.syms[j]   = keySyms.syms[j+1];
	  capsKeySyms.syms[j+1] = keySyms.syms[j];
	}
	if(KeyTranslate::tolower(keySyms.syms[j+2]) 
	   == KeyTranslate::tolower(test)) {
	  capsKeySyms.syms[j+2] = keySyms.syms[j+3];
	  capsKeySyms.syms[j+3] = keySyms.syms[j+2];
	}
      }
    }
  } else {
    /**
       default capsed symbols
    */
    for(i=0; i<(initSyms.maxKeyCode-initSyms.minKeyCode-1)*
	  initSyms.kcodes; i+=initSyms.kcodes) {
      if(KeyTranslate::tolower(initSyms.syms[i]) >= 'a' 
	 && KeyTranslate::tolower(initSyms.syms[i]) <= 'z') {
	capsKeySyms.syms[i]   = keySyms.syms[i+1];
	capsKeySyms.syms[i+1] = keySyms.syms[i];
      }
      if(KeyTranslate::tolower(keySyms.syms[i+2]) >= 'a' 
	 && KeyTranslate::tolower(initSyms.syms[i+2]) <= 'z') {
	capsKeySyms.syms[i+2] = keySyms.syms[i+3];
	capsKeySyms.syms[i+3] = keySyms.syms[i+2];
      }
    }
  }
}
/**
   change symbol by symbol in both normal and caps map
   return true if Ok
*/
bool KeyMap::changeKeySym(const char* from, const char* to, int index)
{
  int i = keySyms.findSym(from);
  if(i == -1) return FALSE;
  keySyms.change(i, to, index);
  capsKeySyms.change(i, to, index);
  return TRUE;
}

/**
   change X window keyboard mapping to this one
*/
void KeyMap::toggle()
{
  if(isToggleCaps && !capsKeySyms.isNull()) capsKeySyms.write();
  else keySyms.write();
}
/**
   activate capslocked keys
*/
void KeyMap::toggleCaps(bool on)
{
  if(isToggleCaps == on) return;
  isToggleCaps = on;
  if(isToggleCaps) {
    if(!capsKeySyms.isNull()) capsKeySyms.write();
  } else {
    keySyms.write();
  }
}

//=========================================================
//  KeySyms members
//=========================================================
KeySyms::KeySyms() 
{
  syms = 0;
}
void KeySyms::allocSyms(KeySym min, KeySym max, unsigned codes)
{
  unsigned nsize = (max-min+1)*codes;
  if((!syms) || (nsize > ((maxKeyCode-minKeyCode+1)*kcodes))) {
    if(syms) delete syms;
    syms = new KeySym[nsize];
    if(syms == 0) {
      KiKbdMsgBox::error(gettext("KeyMap: cannot allocate memory"));
    }
    memset(syms, 0, sizeof(KeySym)*nsize);
  }
  minKeyCode = min;
  maxKeyCode = max;
  kcodes = codes;
}
void KeySyms::expandCodes(unsigned n)
{
  if(n <= kcodes) return;
  KeySyms temp = *this;
  allocSyms(minKeyCode, maxKeyCode, n);
  unsigned i,j;for(i=j=0; i<((maxKeyCode-minKeyCode+1)*kcodes); 
		   i+=kcodes, j+=temp.kcodes) {
    unsigned k;for(k=0; k<temp.kcodes; syms[i+k] = temp.syms[j+k], k++);
    for(;k<kcodes; syms[i+k] = NoSymbol, k++);
  }
}
void KeySyms::read()
{
  int min, max, codes;
  XDisplayKeycodes(kapp->getDisplay(), &min, &max);
  KeySym* sym = XGetKeyboardMapping(kapp->getDisplay(), min,
				    max-min+1, &codes);
  allocSyms(min, max, codes);
  unsigned i; for(i=0; i<(maxKeyCode-minKeyCode+1)*kcodes; i++) {
    syms[i] = sym[i];
  }
  XFree(sym);
}
void KeySyms::write()
{
  if(syms)
    XChangeKeyboardMapping(kapp->getDisplay(), minKeyCode, kcodes, syms,
			   maxKeyCode-minKeyCode+1);
}
int KeySyms::findSym(const char* ssym)
{
  KeySym sym = KeyTranslate::stringOrHexToSym(ssym);
  unsigned i,f;for(i=f=0; i<(maxKeyCode-minKeyCode+1)*kcodes; i+=kcodes)
    {
      if(KeyTranslate::tolower(syms[i]) 
	 == KeyTranslate::tolower(sym)) return i;
    }
  if(kikbdConfig && kikbdConfig->getCodes() == "") {
    KiKbdMsgBox::ask(gettext("Can not find symbol \"%s\".\n"
			     "May be your default X codes does not match.\n"
			     "You can preset X codes in \"Advanced\" section\n"
			     "of configuration program."), ssym);
  } else
    KiKbdMsgBox::askContinue(gettext("KeyMap: can not find symbol \"%s\"."),
			     ssym);
  return -1;
}
int KeySyms::findCode(const char* scode)
{
  KeySym code = KeyTranslate::stringOrNumToCode(scode);
  if(code == (KeySym)-1) {
    KiKbdMsgBox::askContinue(gettext("KeyMap: do not undestand keycode \"%s\".")
			     , scode);
    return -1;
  }
  if(code < minKeyCode || code > maxKeyCode) {
    KiKbdMsgBox
       ::askContinue(gettext("KeyMap: keycode \"%s\" go out of range."),
		     scode);
    return -1;
  }
  return (code-minKeyCode)*kcodes;
}
void KeySyms::change(int from, const char* to, int index)
{
  KeySym sto = KeyTranslate::stringOrHexToSym(to);
  if(from >= 0 && from+index <= (int)((maxKeyCode-minKeyCode+1)*kcodes))
    syms[from+index] = sto;
}
KeySyms& KeySyms::operator=(KeySyms& s)
{
  allocSyms(s.minKeyCode, s.maxKeyCode, s.kcodes);

  unsigned i; for(i=0; i<(maxKeyCode-minKeyCode+1)*kcodes; i++) {
    syms[i] = s.syms[i];
  }
  return *this;
}
KeySyms& KeySyms::operator=(KiKbdMapConfig* map)
{
  QList<QStrList>& codes = map->getKeycodes();
  // find symbols per code and min max code
  KeySym min = (unsigned)-1, max = 0;
  unsigned ncodes = 0;
  for(unsigned i = 0; i < codes.count(); i++) {
    QStrList code = *codes.at(i);
    KeySym keycode = KeyTranslate::stringOrNumToCode(code.at(0));
    if(ncodes < code.count()-1) ncodes = code.count()-1;
    if(min > keycode) min = keycode;
    if(max < keycode) max = keycode;
  }
  // allocate space
  allocSyms(min, max, ncodes<4?4:ncodes);
  // set
  for(unsigned i = 0; i < codes.count(); i++) {
    QStrList code = *codes.at(i);
    int from  = (KeyTranslate::stringOrNumToCode(code.at(0)) - minKeyCode)
      * kcodes;
    for(unsigned j = 1; j < code.count(); j++) {
      change(from, code.at(j), j-1);
    }
  }
  /*cout << form("%d %d %d", minKeyCode, maxKeyCode, kcodes) << endl;
  for(int i = 0; i<(max-min+1)*kcodes; i+=kcodes) {
    cout << form("%4d : ", min+(i/kcodes));
    for(int j=0; j<kcodes; j++) {
      cout << form(" %6d", syms[i+j]);
    }
    cout << endl;
  }
  ::exit(0);*/
  return *this;
}

