#include <stream.h>
#include <stdio.h>
#include <qkeycode.h>
#include <qregexp.h>
#include <X11/keysymdef.h>
#include "keytrans.h"

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
