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
