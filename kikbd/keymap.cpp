#include <stream.h>
#include <stdlib.h>
#include <qfile.h>
#include <qregexp.h>
#include <kapp.h>
#include <kconfig.h>

#include "kikbd.h"
#include "keymap.h"
#include "keytrans.h"

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
  comment = config.getComment();
  altKeys = config.getHasAltKeys();

  /**
     new main symbols
  */
  keySyms = initSyms;

  /**
     load symbols from symbols
  */
  QList<QStrList> &symmap = config.getKeysyms();
  unsigned i;for(i=0; i<symmap.count(); i++) {
    QStrList list = *(symmap.at(i));
    /**
       entry with less then 2 symbols are bad
    */
    if(list.count() < 2) continue;
    /**
       change symbols
    */
    int index = initSyms.findSym(list.at(0));
    if(index == -1)  continue;

    unsigned j;for(j=list.count()<=5?list.count():5; j-->1;)
      keySyms.change(index, list.at(j), j-1);
  }

  /**
     load symbols from codes
  */
  QList<QStrList> &codemap = config.getKeycodes();
  for(i=0; i<codemap.count(); i++) {
    QStrList list = *(codemap.at(i));
    /**
       entry with less then 2 symbols are bad
    */
    if(list.count() < 2) continue;
    /**
       change symbols
    */
    int index = initSyms.findCode(list.at(0));
    if(index == -1) {
      cout << "-1 code = " << list.at(0) << endl;
      continue;
    }
    unsigned j;for(j=list.count()<=5?list.count():5; j-->1;)
      keySyms.change(index, list.at(j), j-1);
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
void KeySyms::allocSyms(int min, int max, int codes)
{
  unsigned nsize = (max-min+1)*codes;
  if((!syms) || (nsize > ((maxKeyCode-minKeyCode+1)*kcodes))) {
    if(syms) delete syms;
    syms = new KeySym[nsize];
    if(syms == 0) {
      KiKbdApplication::error(klocale->translate("KeyMap: cannot allocate"
						 " memory"));
    }
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
  KiKbdApplication::error(klocale->translate("KeyMap: can not find "
					     "symbol %s"), ssym);  
  return -1;
}
int KeySyms::findCode(const char* scode)
{
  unsigned int code = 0;
  if(sscanf(scode, "%x", &code) != 1)
    if(sscanf(scode, "%d", &code) != 1)
      KiKbdApplication::error(klocale->translate("KeyMap: do not undestand "
						 "keycode %s"), scode);
  if(code < minKeyCode || code > maxKeyCode) {
    KiKbdApplication::error(klocale->translate("KeyMap: keycode %s go out "
					       "of range"), scode);
    return -1;
  }
  return (minKeyCode+code)*kcodes;
}
void KeySyms::change(int from, const char* to, int index)
{
  KeySym sto = KeyTranslate::stringOrHexToSym(to);
  if(from >= 0 && from <= (int)((maxKeyCode-minKeyCode+1)*kcodes))
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

