#include <string.h>

//#include <qcombo.h>
#include <qgrpbox.h>
#include <qtstream.h>
#include <qregexp.h>

#include <kapp.h>
#include <kfontdialog.h>
#include <kcharsets.h>

#include <X11/Xlib.h>

#include "highlight.h"
#include "kwdoc.h"
#include "kmimemagic.h"

char *cTypes[] = {
  "char","double","float","int","long","short","signed","unsigned","void",0L};

char *cKeywords[] = {
  "break","case","continue","default","do","else","enum","extern","for",
  "goto","if","interrupt","register","return","static","struct","switch",
  "typedef","union","volatile","while",0L};

char *cppTypes[] = {
  "bool",0L};

char *cppKeywords[] = {
  "class","const","delete","friend","inline","new","operator","private",
  "protected","public","this","virtual",0L};

char *javaKeywords[] = {
  "abstract", "boolean", "break", "byte", "case", "cast",
  "catch", "char", "class", "const", "continue", "default",
  "do", "double", "else", "extends", "false", "final", 
  "finally", "float", "for", "future", "generic", "goto",
  "if", "implements", "import", "inner", "instanceof", "int",
  "interface", "long", "native", "new", "null", "operator",
  "outer", "package", "private", "protected", "public", "rest",
  "return", "short", "static", "super", "switch", "synchronized",
  "this", "throw", "throws", "transient", "true", "try", 
  "var", "void", "volatile", "while", 0L};

char *bashKeywords[] = {
  "break","case","done","do","elif","else","esac","exit","export","fi","for",
  "function","if","in","return","select","then","until","while",".",0L};

char *modulaKeywords[] = {
  "BEGIN","CONST","DEFINITION","DIV","DO","ELSE","ELSIF","END","FOR","FROM",
  "IF","IMPLEMENTATION","IMPORT","MODULE","MOD","PROCEDURE","RECORD","REPEAT",
  "RETURN","THEN","TYPE","VAR","WHILE","WITH","|",0L};

char *adaKeywords[] = {
  "abort","abs","accept","access","all","and","array","at","begin","body",
  "case","constant","declare","delay","delta","digits","do","else","elsif",
  "end","entry","exception","exit","for", "function","generic","goto","if",
  "in","is","limited","loop","mod","new", "not","null","of","or","others",
  "out","package","pragma","private","procedure", "raise","range","rem",
  "record","renames","return","reverse","select","separate","subtype", "task",
  "terminate","then","type","use","when","while","with","xor",0L};

char fontSizes[] = {4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,24,26,28,32,48,64,0};

bool testWw(char c) {
  static char data[] = {0,0,0,0,0,64,255,3,254,255,255,135,254,255,255,7};
  if (c & 128) return false;
  return !(data[c >> 3] & (1 << (c & 7)));
}


HlItem::HlItem(int attribute, int context)
  : attr(attribute), ctx(context) {
}

HlItemWw::HlItemWw(int attribute, int context)
  : HlItem(attribute,context) {
}


HlCharDetect::HlCharDetect(int attribute, int context, char c)
  : HlItem(attribute,context), sChar(c) {
}

const char *HlCharDetect::checkHgl(const char *str) {
  if (*str == sChar) return str + 1;
  return 0L;
}

Hl2CharDetect::Hl2CharDetect(int attribute, int context, const char *s)
  : HlItem(attribute,context) {
  sChar[0] = s[0];
  sChar[1] = s[1];
}

const char *Hl2CharDetect::checkHgl(const char *str) {
  if (str[0] == sChar[0] && str[1] == sChar[1]) return str + 2;
  return 0L;
}

HlStringDetect::HlStringDetect(int attribute, int context, const char *s)
  : HlItem(attribute,context) {
  len = strlen(s);
  str = new char[len];
  memcpy(str,s,len);
}

HlStringDetect::~HlStringDetect() {
  delete str;
}

const char *HlStringDetect::checkHgl(const char *s) {
  if (memcmp(s,str,len) == 0) return s + len;
  return 0L;
}

HlRangeDetect::HlRangeDetect(int attribute, int context, const char *s)
  : HlItem(attribute,context) {
  sChar[0] = s[0];
  sChar[1] = s[1];
}

const char *HlRangeDetect::checkHgl(const char *s) {
  if (*s == sChar[0]) {
    do {
      s++;
      if (!*s) return 0L;
    } while (*s != sChar[1]);
    return s + 1;
  }
  return 0L;
}


KeywordData::KeywordData(const char *str) {
  len = strlen(str);
  s = new char[len];
  memcpy(s,str,len);
}

KeywordData::~KeywordData() {
  delete s;
}

HlKeyword::HlKeyword(int attribute, int context)
  : HlItemWw(attribute,context) {
  words.setAutoDelete(true);
}

HlKeyword::~HlKeyword() {
}


void HlKeyword::addWord(const char *s) {
  KeywordData *word;
  word = new KeywordData(s);
  words.append(word);
}

void HlKeyword::addList(char **list) {

  while (*list) {
    addWord(*list);
    list++;
  }
}

const char *HlKeyword::checkHgl(const char *s) {
  int z, count;
  KeywordData *word;

  count = words.count();
  for (z = 0; z < count; z++) {
    word = words.at(z);
    if (memcmp(s,word->s,word->len) == 0) {
      return s + word->len;
    }
  }
  return 0L;
}


HlInt::HlInt(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlInt::checkHgl(const char *str) {
  const char *s;

  s = str;
  while (*s >= '0' && *s <= '9') s++;
  if (s > str) return s;
  return 0L;
}

HlFloat::HlFloat(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlFloat::checkHgl(const char *s) {
  bool b, p;

  b = false;
  while (*s >= '0' && *s <= '9') {
    s++;
    b = true;
  }
  if (p = (*s == '.')) {
    s++;
    while (*s >= '0' && *s <= '9') {
      s++;
      b = true;
    }
  }
  if (!b) return 0L;
  if (*s == 'E' || *s == 'e') s++; else return (p) ? s : 0L;
  if (*s == '-') s++;
  b = false;
  while (*s >= '0' && *s <= '9') {
    s++;
    b = true;
  }
  if (b) return s; else return 0L;
}


HlCInt::HlCInt(int attribute, int context)
  : HlInt(attribute,context) {
}

const char *HlCInt::checkHgl(const char *s) {

  if (*s == '0') s++; else s = HlInt::checkHgl(s);
  if (s && (*s == 'L' || *s == 'l' || *s == 'U' || *s == 'u')) s++;
  return s;
}

HlCOct::HlCOct(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlCOct::checkHgl(const char *str) {
  const char *s;

  if (*str == '0') {
    str++;
    s = str;
    while (*s >= '0' && *s <= '7') s++;
    if (s > str) {
      if (*s == 'L' || *s == 'l' || *s == 'U' || *s == 'u') s++;
      return s;
    }
  }
  return 0L;
}

HlCHex::HlCHex(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlCHex::checkHgl(const char *str) {
  const char *s;

  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
    str += 2;
    s = str;
    while ((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F') || (*s >= 'a' && *s <= 'f')) s++;
    if (s > str) {
      if (*s == 'L' || *s == 'l' || *s == 'U' || *s == 'u') s++;
      return s;
    }
  }
  return 0L;
}

HlCFloat::HlCFloat(int attribute, int context)
  : HlFloat(attribute,context) {
}

const char *HlCFloat::checkHgl(const char *s) {

  s = HlFloat::checkHgl(s);
  if (s && (*s == 'F' || *s == 'f')) s++;
  return s;
}

HlLineContinue::HlLineContinue(int attribute, int context)
  : HlItem(attribute,context) {
}

const char *HlLineContinue::checkHgl(const char *s) {
  if (*s == '\\') return s + 1;
  return 0L;
}


HlCStringChar::HlCStringChar(int attribute, int context)
  : HlItem(attribute,context) {
}

const char *checkCStringChar(const char *str) {
  static char echars[] = "abefnrtv\"\'\\";
  const char *s;
  int n;

  if (str[0] == '\\' && str[1] != 0) {
    s = str + 1;
    if (strchr(echars,*s)) {
      s++;
    } else if (*s == 'x') {
      n = 0;
      do {
        s++;
        n *= 16;
        if (*s >= '0' && *s <= '9') n += *s - '0';
        else if (*s >= 'A' && *s <= 'F') n += *s - 'A' + 10;
        else if (*s >= 'a' && *s <= 'f') n += *s - 'a' + 10;
        else break;
        if (n >= 256) return 0L;
      } while (true);
      if (s - str == 2) return 0L;
    } else {
      if (!(*s >= '0' && *s <= '7')) return 0L;
      n = *s - '0';
      do {
        s++;
        n *= 8;
        if (*s >= '0' && *s <= '7') n += *s - '0'; else break;
        if (n >= 256) return s;
      } while (s - str < 4);
    }
    return s;
  }
  return 0L;
}

const char *HlCStringChar::checkHgl(const char *str) {
  return checkCStringChar(str);
}


HlCChar::HlCChar(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlCChar::checkHgl(const char *str) {
  const char *s;

  if (str[0] == '\'' && str[1] != 0) {
    s = checkCStringChar(&str[1]); //try to match escaped char
    if (!s) s = &str[2];           //match single non-escaped char
    if (*s == '\'') return s + 1;
  }
  return 0L;
}

HlCPrep::HlCPrep(int attribute, int context)
  : HlItem(attribute,context) {
}

const char *HlCPrep::checkHgl(const char *s) {

  while (*s == ' ' || *s == '\t') s++;
  if (*s == '#') {
    s++;
    return s;
  }
  return 0L;
}

HlHtmlTag::HlHtmlTag(int attribute, int context)
  : HlItem(attribute,context) {
}

const char *HlHtmlTag::checkHgl(const char *s) {
  while (*s == ' ' || *s == '\t') s++;
  while (*s != ' ' && *s != '\t' && *s != '>' && *s != '\0') s++;
  return s;
}

HlHtmlValue::HlHtmlValue(int attribute, int context)
  : HlItem(attribute,context) {
}

const char *HlHtmlValue::checkHgl(const char *s) {
  while (*s == ' ' || *s == '\t') s++;
  if (*s == '\"') {
    do {
      s++;
      if (!*s) return 0L;
    } while (*s != '\"');
    s++;
  } else {
    while (*s != ' ' && *s != '\t' && *s != '>' && *s != '\0') s++;
  }
  return s;
}

HlShellComment::HlShellComment(int attribute, int context)
  : HlCharDetect(attribute,context,'#') {
}

HlMHex::HlMHex(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlMHex::checkHgl(const char *s) {

  if (*s >= '0' && *s <= '9') {
    s++;
    while ((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F')) s++;
    if (*s == 'H') return s + 1;
  }
  return 0L;
}

HlAdaDec::HlAdaDec(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlAdaDec::checkHgl(const char *s) {
  const char *str;

  if (*s >= '0' && *s <= '9') {
    s++;
    while ((*s >= '0' && *s <= '9') || *s == '_') s++;
    if (*s != 'e' && *s != 'E') return s;
    s++;
    str = s;
    while ((*s >= '0' && *s <= '9') || *s == '_') s++;
    if (s > str) return s;
  }
  return 0L;
}

HlAdaBaseN::HlAdaBaseN(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlAdaBaseN::checkHgl(const char *s) {
  int base;
  char c1, c2, c3;
  const char *str;

  base = 0;
  while (*s >= '0' && *s <= '9') {
    base *= 10;
    base += *s - '0';
    if (base > 16) return 0L;
    s++;
  }
  if (base >= 2 && *s == '#') {
    s++;
    c1 = '0' + ((base <= 10) ? base : 10);
    c2 = 'A' + base - 10;
    c3 = 'a' + base - 10;
    while ((*s >= '0' && *s < c1) || (*s >= 'A' && *s < c2)
      || (*s >= 'a' && *s < c3)) {
      s++;
    }
    if (*s == '#') {
      s++;
      if (*s != 'e' && *s != 'E') return s;
      s++;
      str = s;
      while ((*s >= '0' && *s <= '9') || *s == '_') s++;
      if (s > str) return s;
    }
  }
  return 0L;
}

HlAdaFloat::HlAdaFloat(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlAdaFloat::checkHgl(const char *s) {
  const char *str;

  str = s;
  while (*s >= '0' && *s <= '9') s++;
  if (s > str && *s == '.') {
    s++;
    str = s;
    while (*s >= '0' && *s <= '9') s++;
    if (s > str) {
      if (*s != 'e' && *s != 'E') return s;
      s++;
      if (*s == '-') s++;
      str = s;
      while ((*s >= '0' && *s <= '9') || *s == '_') s++;
      if (s > str) return s;
    }
  }
  return 0L;
}

HlAdaChar::HlAdaChar(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlAdaChar::checkHgl(const char *s) {
  if (s[0] == '\'' && s[1] && s[2] == '\'') return s + 3;
  return 0L;
}

//--------
ItemStyle::ItemStyle() : selCol(white), bold(false), italic(false) {
}

ItemStyle::ItemStyle(const QColor &col, const QColor &selCol,
  bool bold, bool italic)
  : col(col), selCol(selCol), bold(bold), italic(italic) {
}

ItemFont::ItemFont() : family("courier"), size(12), charset("") {
}

ItemData::ItemData(const char *name, int defStyleNum)
  : name(name), defStyleNum(defStyleNum), defStyle(true), defFont(true) {
}

ItemData::ItemData(const char *name, int defStyleNum,
  const QColor &col, const QColor &selCol, bool bold, bool italic)
  : name(name), defStyleNum(defStyleNum), ItemStyle(col,selCol,bold,italic),
  defStyle(false), defFont(true) {
}

HlData::HlData(const char *wildcards, const char *mimetypes)
  : wildcards(wildcards), mimetypes(mimetypes) {

  itemDataList.setAutoDelete(true);
}

Highlight::Highlight(const char *name) : iName(name), refCount(0) {
}

Highlight::~Highlight() {
}

KConfig *Highlight::getKConfig() {
  KConfig *config;

  config = kapp->getConfig();
  config->setGroup((QString) iName + " Highlight");
  return config;
}

void Highlight::getWildcards(QString &w) {
  KConfig *config;

  config = getKConfig();

//  iWildcards
  w = config->readEntry("Wildcards",dw);
//  iMimetypes = config->readEntry("Mimetypes");
}


void Highlight::getMimetypes(QString &w) {
  KConfig *config;

  config = getKConfig();

  w = config->readEntry("Mimetypes",dm);
}


HlData *Highlight::getData() {
  KConfig *config;
  HlData *hlData;

  config = getKConfig();

//  iWildcards = config->readEntry("Wildcards");
//  iMimetypes = config->readEntry("Mimetypes");
//  hlData = new HlData(iWildcards,iMimetypes);
  hlData = new HlData(
    config->readEntry("Wildcards",dw),config->readEntry("Mimetypes",dm));
  getItemDataList(hlData->itemDataList,config);
  return hlData;
}

void Highlight::setData(HlData *hlData) {
  KConfig *config;

  config = getKConfig();

//  iWildcards = hlData->wildcards;
//  iMimetypes = hlData->mimetypes;

  config->writeEntry("Wildcards",hlData->wildcards);
  config->writeEntry("Mimetypes",hlData->mimetypes);

  setItemDataList(hlData->itemDataList,config);
}

void Highlight::getItemDataList(ItemDataList &list) {
  KConfig *config;

  config = getKConfig();
  getItemDataList(list,config);
}

void Highlight::getItemDataList(ItemDataList &list, KConfig *config) {
  ItemData *p;
  QString s;
  QRgb col, selCol;
  char family[96];
  char charset[48];

  list.clear();
  list.setAutoDelete(true);
  createItemData(list);

  for (p = list.first(); p != 0L; p = list.next()) {
    s = config->readEntry(p->name);
    if (!s.isEmpty()) {
      sscanf(s,"%d,%X,%X,%d,%d,%d,%95[^,],%d,%47[^,]",
        &p->defStyle,&col,&selCol,&p->bold,&p->italic,
        &p->defFont,family,&p->size,charset);
      p->col.setRgb(col);
      p->selCol.setRgb(selCol);
      p->family = family;
      p->charset = charset;
    }
  }
}

void Highlight::setItemDataList(ItemDataList &list, KConfig *config) {
  ItemData *p;
  char s[200];

  for (p = list.first(); p != 0L; p = list.next()) {
    sprintf(s,"%d,%X,%X,%d,%d,%d,%1.95s,%d,%1.47s",
      p->defStyle,p->col.rgb(),p->selCol.rgb(),p->bold,p->italic,
      p->defFont,p->family.data(),p->size,p->charset.data());
    config->writeEntry(p->name,s);
  }
}

const char *Highlight::name() {
  return iName;
}

void Highlight::use() {
  if (refCount == 0) init();
  refCount++;
}

void Highlight::release() {
  refCount--;
  if (refCount == 0) done();
}


bool Highlight::isInWord(char ch) {
  static char data[] = {0,0,0,0,0,0,255,3,254,255,255,135,254,255,255,7};
  if (ch & 128) return true;
  return data[ch >> 3] & (1 << (ch & 7));
}

void Highlight::doHighlight(int, TextLine *textLine) {

  textLine->setAttribs(0,0,textLine->length());
  textLine->setAttr(0);
}

void Highlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",0));
}


void Highlight::init() {
}

void Highlight::done() {
}


HlContext::HlContext(int attribute, int lineEndContext)
  : attr(attribute), ctx(lineEndContext) {
  items.setAutoDelete(true);
}


GenHighlight::GenHighlight(const char *name) : Highlight(name) {
}


void GenHighlight::doHighlight(int ctxNum, TextLine *textLine) {
  HlContext *context;
  const char *str, *s1, *s2;
  char lastChar;
  HlItem *item;

  context = contextList[ctxNum];

  str = textLine->getString();
  lastChar = 0;

  s1 = str;
  while (*s1) {
    for (item = context->items.first(); item != 0L; item = context->items.next()) {
      if (item->startEnable(lastChar)) {
        s2 = item->checkHgl(s1);
        if (s2 > s1) {
          if (item->endEnable(*s2)) {
            textLine->setAttribs(item->attr,s1 - str,s2 - str);
            ctxNum = item->ctx;
            context = contextList[ctxNum];
            s1 = s2 - 1;
            goto found;
          }
        }
      }
    }
    // nothing found: set attribute of one char
    textLine->setAttribs(context->attr,s1 - str,s1 - str + 1);

    found:
    lastChar = *s1;
    s1++;
  }
  //set "end of line"-properties of actual context
  textLine->setAttr(context->attr);
  textLine->setContext(context->ctx);
}


void GenHighlight::init() {
  int z;

  for (z = 0; z < nAttribs; z++) contextList[z] = 0L;
  makeContextList();
}

void GenHighlight::done() {
  int z;

  for (z = 0; z < nAttribs; z++) delete contextList[z];
}


CHighlight::CHighlight(const char *name) : GenHighlight(name) {
  dw = "*.c";
  dm = "text/x-c-src";
}

CHighlight::~CHighlight() {
}

void CHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",0));
  list.append(new ItemData("Keyword",1));
  list.append(new ItemData("Decimal",2));
  list.append(new ItemData("Octal",3));
  list.append(new ItemData("Hex",3));
  list.append(new ItemData("Float",4));
  list.append(new ItemData("Char",5));
  list.append(new ItemData("String",6));
  list.append(new ItemData("String Char",5));
  list.append(new ItemData("Comment",7));
  list.append(new ItemData("Preprocessor",8,darkGreen,green,false,false));
  list.append(new ItemData("Prep. Lib",8,darkYellow,yellow,false,false));
}

void CHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlCInt(2,0));
    c->items.append(new HlCOct(3,0));
    c->items.append(new HlCHex(4,0));
    c->items.append(new HlCFloat(5,0));
    c->items.append(new HlCChar(6,0));
    c->items.append(new HlCharDetect(7,1,'"'));
    c->items.append(new Hl2CharDetect(9,2,"//"));
    c->items.append(new Hl2CharDetect(9,3,"/*"));
    c->items.append(new HlCPrep(10,4));
  contextList[1] = c = new HlContext(7,0);
    c->items.append(new HlLineContinue(7,6));
    c->items.append(new HlCStringChar(8,1));
    c->items.append(new HlCharDetect(7,0,'"'));
  contextList[2] = new HlContext(9,0);
  contextList[3] = c = new HlContext(9,3);
    c->items.append(new Hl2CharDetect(9,0,"*/"));
  contextList[4] = c = new HlContext(10,0);
    c->items.append(new HlLineContinue(10,7));
    c->items.append(new HlRangeDetect(11,4,"\"\""));
    c->items.append(new HlRangeDetect(11,4,"<>"));
//    c->items.append(new HlCharDetect(11,5,'"'));
//    c->items.append(new HlCharDetect(11,6,'<'));
    c->items.append(new Hl2CharDetect(9,2,"//"));
    c->items.append(new Hl2CharDetect(9,5,"/*"));
//  contextList[5] = c = new HlContext(11,0);
//    c->items.append(new HlCharDetect(11,4,'"'));
//  contextList[6] = c = new HlContext(11,0);
//    c->items.append(new HlCharDetect(11,4,'>'));
  contextList[5] = c = new HlContext(9,5);
    c->items.append(new Hl2CharDetect(9,4,"*/"));
  contextList[6] = new HlContext(0,1);
  contextList[7] = new HlContext(0,4);

  setKeywords(keyword);
}

void CHighlight::setKeywords(HlKeyword *keyword) {

//  keyword->addList(cTypes);
  keyword->addList(cKeywords);
}


CppHighlight::CppHighlight(const char *name) : CHighlight(name) {
  dw = "*.cpp;*.h;*.C";
  dm = "text/x-c++-src;text/x-c++-hdr;text/x-c-hdr";
}

CppHighlight::~CppHighlight() {
}

void CppHighlight::setKeywords(HlKeyword *keyword) {

//  keyword->addList(cTypes);
//  keyword->addList(cppTypes);
  keyword->addList(cKeywords);
  keyword->addList(cppKeywords);
}


JavaHighlight::JavaHighlight(const char *name) : CHighlight(name) {
  dw = "*.java";
  dm = "text/x-java-src";
}

JavaHighlight::~JavaHighlight() {
}

void JavaHighlight::setKeywords(HlKeyword *keyword) {

  keyword->addList(javaKeywords);
}


HtmlHighlight::HtmlHighlight(const char *name) : GenHighlight(name) {
  dw = "*.html;*.htm";
  dm = "text/html";
}

HtmlHighlight::~HtmlHighlight() {
}

void HtmlHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",0));
  list.append(new ItemData("Char",5,darkGreen,green,false,false));
  list.append(new ItemData("Comment",7));
  list.append(new ItemData("Tag Text",8,black,white,true,false));
  list.append(new ItemData("Tag",1,darkMagenta,magenta,true,false));
  list.append(new ItemData("Tag Value",2,darkCyan,cyan,false,false));
}

void HtmlHighlight::makeContextList() {
  HlContext *c;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(new HlRangeDetect(1,0,"&;"));
    c->items.append(new HlStringDetect(2,1,"<!--"));
    c->items.append(new HlStringDetect(2,2,"<COMMENT>"));
    c->items.append(new HlCharDetect(3,3,'<'));
  contextList[1] = c = new HlContext(2,1);
    c->items.append(new HlStringDetect(2,0,"-->"));
  contextList[2] = c = new HlContext(2,2);
    c->items.append(new HlStringDetect(2,0,"</COMMENT>"));
  contextList[3] = c = new HlContext(3,3);
    c->items.append(new HlHtmlTag(4,3));
    c->items.append(new HlHtmlValue(5,3));
    c->items.append(new HlCharDetect(3,0,'>'));
}


BashHighlight::BashHighlight(const char *name) : GenHighlight(name) {
  dm = "text/x-shellscript";
}


BashHighlight::~BashHighlight() {
}

void BashHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",0));
  list.append(new ItemData("Keyword",1));
  list.append(new ItemData("Integer",2));
  list.append(new ItemData("String",6));
  list.append(new ItemData("Substitution",8));//darkCyan,cyan,false,false);
  list.append(new ItemData("Comment",7));
}

void BashHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlCharDetect(3,1,'"'));
    c->items.append(new HlCharDetect(4,2,'`'));
    c->items.append(new HlShellComment(5,3));
  contextList[1] = c = new HlContext(3,0);
    c->items.append(new HlCharDetect(3,0,'"'));
  contextList[2] = c = new HlContext(4,0);
    c->items.append(new HlCharDetect(4,0,'`'));
  contextList[3] = new HlContext(5,0);

  keyword->addList(bashKeywords);
}


ModulaHighlight::ModulaHighlight(const char *name) : GenHighlight(name) {
  dw = "*.md;*.mi";
  dm = "text/x-modula-2-src";
}

ModulaHighlight::~ModulaHighlight() {
}

void ModulaHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",0));
  list.append(new ItemData("Keyword",1));
  list.append(new ItemData("Decimal",2));
  list.append(new ItemData("Hex",3));
  list.append(new ItemData("Float",4));
  list.append(new ItemData("String",6));
  list.append(new ItemData("Comment",7));
}

void ModulaHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlMHex(3,0));
    c->items.append(new HlFloat(4,0));
    c->items.append(new HlCharDetect(5,1,'"'));
    c->items.append(new Hl2CharDetect(6,2,"(*"));
  contextList[1] = c = new HlContext(5,0);
    c->items.append(new HlCharDetect(5,0,'"'));
  contextList[2] = c = new HlContext(6,2);
    c->items.append(new Hl2CharDetect(6,0,"*)"));

  keyword->addList(modulaKeywords);
}


AdaHighlight::AdaHighlight(const char *name) : GenHighlight(name) {
  dw = "*.a";
  dm = "text/x-ada-src";
}

AdaHighlight::~AdaHighlight() {
}

void AdaHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",0));
  list.append(new ItemData("Keyword",1));
  list.append(new ItemData("Decimal",2));
  list.append(new ItemData("Octal",3));
  list.append(new ItemData("Float",4));
  list.append(new ItemData("String",6));
  list.append(new ItemData("Comment",7));
}

void AdaHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlAdaBaseN(3,0));
    c->items.append(new HlAdaDec(2,0));
    c->items.append(new HlAdaFloat(4,0));
    c->items.append(new HlAdaChar(5,0));
    c->items.append(new HlCharDetect(6,1,'"'));
    c->items.append(new Hl2CharDetect(7,2,"--"));
  contextList[1] = c = new HlContext(6,0);
    c->items.append(new HlCharDetect(6,0,'"'));
  contextList[2] = c = new HlContext(7,0);

  keyword->addList(adaKeywords);
}


HlManager::HlManager() : QObject(0L) {

  hlList.setAutoDelete(true);
  hlList.append(new Highlight("Normal"));
  hlList.append(new CHighlight("C"));
  hlList.append(new CppHighlight("C++"));
  hlList.append(new JavaHighlight("Java"));
  hlList.append(new HtmlHighlight("HTML"));
  hlList.append(new BashHighlight("Bash"));
  hlList.append(new ModulaHighlight("Modula 2"));
  hlList.append(new AdaHighlight("Ada"));
}

HlManager::~HlManager() {
}

Highlight *HlManager::getHl(int n) {
  if (n < 0 || n >= (int) hlList.count()) n = 0;
  return hlList.at(n);
}

int HlManager::defaultHl() {
  KConfig *config;

  config = kapp->getConfig();
  config->setGroup("General Options");
  return nameFind(config->readEntry("Highlight"));
}


int HlManager::highlightFind(KWriteDoc *doc)
{
  int hl = -1;

  if (doc->hasFileName())
    hl = wildcardFind(doc->fileName());

  if (hl == -1)
    hl = mimeFind(doc);

  return hl;
}


int HlManager::nameFind(const char *name) {
  int z;

  for (z = hlList.count() - 1; z > 0; z--) {
    if (hlList.at(z)->iName == name) break;
  }
  return z;
}

int HlManager::wildcardFind(const char *fileName) {
  Highlight *highlight;
  int p1, p2;
  QString w;
printf("file name %s\n",fileName);
  for (highlight = hlList.first(); highlight != 0L; highlight = hlList.next()) {
printf("highlight %s\n",highlight->name());
    p1 = 0;
//    w = highlight->iWildcards;
    highlight->getWildcards(w);
    while (p1 < (int) w.length()) {
      p2 = w.find(';',p1);
      if (p2 == -1) p2 = w.length();
      if (p1 < p2) {
printf("wildcard %s\n",w.mid(p1,p2 - p1).data());
        QRegExp regExp(w.mid(p1,p2 - p1),true,true);
        if (regExp.match(fileName) == 0) return hlList.at();
      }
      p1 = p2 + 1;
    }
  }
  return -1;
}

int HlManager::mimeFind(KWriteDoc *doc) 
{
  // Magic file detection init (from kfm/kbind.cpp)    
  QString mimefile = kapp->kde_mimedir().copy();    
  mimefile += "/magic";    
  KMimeMagic magic(mimefile);    
  magic.setFollowLinks(true);      

  // fill the detection buffer with the contents of the text  
  const int HOWMANY = 1024;
  char buffer[HOWMANY];
  int number=0, len;
  
  for (int index=0; index<doc->lastLine(); index++)
  {
    len = doc->textLength(index);

    if (number+len > HOWMANY)
      break;

    memcpy(&buffer[number], doc->textLine(index)->getText(), len);
    number += len;
  }

  // detect the mime type
  KMimeMagicResult *result;
  if (doc->hasFileName())
    result = magic.findBufferFileType(buffer,number,doc->fileName());
  else
    result = magic.findBufferType(buffer,number);

printf("MIMETYPE: %s\n", result->getContent().data());

  Highlight *highlight;
  int p1, p2;
  QString w;

  for (highlight = hlList.first(); highlight != 0L; highlight = hlList.next()) 
  {
    highlight->getMimetypes(w);

    p1 = 0;
    while (p1 < (int) w.length()) {
      p2 = w.find(';',p1);
      if (p2 == -1) p2 = w.length();
      if (p1 < p2) {
        QRegExp regExp(w.mid(p1,p2 - p1),true,true);
        if (regExp.match(result->getContent()) == 0) return hlList.at();
      }
      p1 = p2 + 1;
    }
  }

  return -1;
}

void HlManager::makeAttribs(Highlight *highlight, Attribute *a, int n) {
  ItemStyleList defaultStyleList;
  ItemStyle *defaultStyle;
  ItemFont defaultFont;
  ItemDataList itemDataList;
  ItemData *itemData;
  int z;
  QFont font;

  defaultStyleList.setAutoDelete(true);
  getDefaults(defaultStyleList,defaultFont);

  itemDataList.setAutoDelete(true);
  highlight->getItemDataList(itemDataList);
  for (z = 0; z < (int) itemDataList.count(); z++) {
    itemData = itemDataList.at(z);
    if (itemData->defStyle) {
      defaultStyle = defaultStyleList.at(itemData->defStyleNum);
      a[z].col = defaultStyle->col;
      a[z].selCol = defaultStyle->selCol;
      font.setBold(defaultStyle->bold);
      font.setItalic(defaultStyle->italic);
    } else {
      a[z].col = itemData->col;
      a[z].selCol = itemData->selCol;
      font.setBold(itemData->bold);
      font.setItalic(itemData->italic);
    }
    if (itemData->defFont) {
      font.setFamily(defaultFont.family);
      font.setPointSize(defaultFont.size);
      KCharset(defaultFont.charset).setQFont(font);
    } else {
      font.setFamily(itemData->family);
      font.setPointSize(itemData->size);
      KCharset(itemData->charset).setQFont(font);
    }
    a[z].setFont(font);
  }
  for (; z < n; z++) {
    a[z].col = black;
    a[z].selCol = black;
    a[z].setFont(font);
  }
}

int HlManager::defaultStyles() {
  return 9;
}

const char *HlManager::defaultStyleName(int n) {
  static const char *names[] = {
    "Normal","Keyword","Decimal/Value","Base-N Integer","Floating Point",
    "Character","String","Comment","Others"};

  return names[n];
}

void HlManager::getDefaults(ItemStyleList &list, ItemFont &font) {
  KConfig *config;
  int z;
  ItemStyle *i;
  QString s;
  QRgb col, selCol;

  list.setAutoDelete(true);
  list.append(new ItemStyle(black,white,false,false));
  list.append(new ItemStyle(black,white,true,false));
  list.append(new ItemStyle(blue,cyan,false,false));
  list.append(new ItemStyle(darkCyan,cyan,false,false));
  list.append(new ItemStyle(darkMagenta,cyan,false,false));
  list.append(new ItemStyle(magenta,magenta,false,false));
  list.append(new ItemStyle(red,red,false,false));
  list.append(new ItemStyle(darkGray,gray,false,true));
  list.append(new ItemStyle(darkBlue,blue,false,false));

  config = kapp->getConfig();
  config->setGroup("Default Item Styles");
  for (z = 0; z < defaultStyles(); z++) {
    i = list.at(z);
    s = config->readEntry(defaultStyleName(z));
    if (!s.isEmpty()) {
      sscanf(s,"%X,%X,%d,%d",&col,&selCol,&i->bold,&i->italic);
      i->col.setRgb(col);
      i->selCol.setRgb(selCol);
    }
  }

  config->setGroup("Default Font");
  font.family = config->readEntry("Family","courier");
  font.size = config->readNumEntry("Size",12);
  font.charset = config->readEntry("Charset","ISO-8859-1");
}

void HlManager::setDefaults(ItemStyleList &list, ItemFont &font) {
  KConfig *config;
  int z;
  ItemStyle *i;
  char s[64];

  config = kapp->getConfig();
  config->setGroup("Default Item Styles");
  for (z = 0; z < defaultStyles(); z++) {
    i = list.at(z);
    sprintf(s,"%X,%X,%d,%d",i->col.rgb(),i->selCol.rgb(),i->bold, i->italic);
    config->writeEntry(defaultStyleName(z),s);
  }

  config->setGroup("Default Font");
  config->writeEntry("Family",font.family);
  config->writeEntry("Size",font.size);
  config->writeEntry("Charset",font.charset);

  emit changed();
}


int HlManager::highlights() {
  return (int) hlList.count();
}

const char *HlManager::hlName(int n) {
  return hlList.at(n)->iName;
}

void HlManager::getHlDataList(HlDataList &list) {
  int z;

  for (z = 0; z < (int) hlList.count(); z++) {
    list.append(hlList.at(z)->getData());
  }
}

void HlManager::setHlDataList(HlDataList &list) {
  int z;

  for (z = 0; z < (int) hlList.count(); z++) {
    hlList.at(z)->setData(list.at(z));
  }

  emit changed();
}


//-----

//"ripped" from kfontdialog
bool getKDEFontList(QStrList &fontList) {
  QString s;

  //TODO replace by QDir::homePath();
  s = KApplication::localkdedir() + "/share/config/kdefonts";
  QFile fontfile(s);
//  if (!fontfile.exists()) return false;
  if(!fontfile.open(IO_ReadOnly)) return false;
//  if (!fontfile.isReadable()) return false;

  QTextStream t(&fontfile);
  while (!t.eof()) {
    s = t.readLine();
    s = s.stripWhiteSpace();
    if (!s.isEmpty()) fontList.append(s);
  }
  fontfile.close();
  return true;
}

void getXFontList(QStrList &fontList) {
  Display *kde_display;
  int numFonts;
  char** fontNames;
  char* fontName;
  QString qfontname;
  int i, dash, dash_two;

  kde_display = XOpenDisplay( 0L );
  fontNames = XListFonts(kde_display, "*", 32767, &numFonts);

  for(i = 0; i < numFonts; i++) {
    fontName = fontNames[i];
    if (*fontName != '-') {
      // The font name doesn't start with a dash -- an alias
      // so we ignore it. It is debatable whether this is the right
      // behaviour so I leave the following snippet of code around.
      // Just uncomment it if you want those aliases to be inserted as well.

      /*
      qfontname = fontName;
      if(fontlist.find(qfontname) == -1)
          fontlist.inSort(qfontname);
      */
      continue;
    }

    qfontname = fontName;
    dash = qfontname.find ('-', 1); // find next dash
    if (dash == -1) continue; // No dash such next dash -- this shouldn't happen.
                              // but what do I care -- lets skip it.

    // the font name is between the second and third dash so:
    // let's find the third dash:
    dash_two = qfontname.find ('-', dash + 1);
    if (dash == -1) continue; // No such next dash -- this shouldn't happen.
                              // but what do I care -- lets skip it.

    // fish the name of the font info string
    qfontname = qfontname.mid(dash +1, dash_two - dash -1);
    if (!qfontname.contains("open look", TRUE)) {
      if (qfontname != "nil") {
        if (fontList.find(qfontname) == -1) fontList.inSort(qfontname);
      }
    }
  }

  XFreeFontNames(fontNames);
  XCloseDisplay(kde_display);
}

void getFontList(QStrList &fontList) {

  //try to get KDE fonts
  if (getKDEFontList(fontList)) return;
  //not successful: get X fonts
  getXFontList(fontList);
}


StyleChanger::StyleChanger(QWidget *parent, int x, int y) : QObject(parent) {
  QLabel *label;
  QRect r;

  col = new KColorButton(parent);
  label = new QLabel(col,i18n("Normal:"),parent);
  connect(col,SIGNAL(changed(const QColor &)),this,SLOT(changed()));

  r.setRect(x,y,80,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  col->setGeometry(r);
//  label = new QLabel(col,i18n("Color:"),parent);

  selCol = new KColorButton(parent);
  label = new QLabel(selCol,i18n("Selected:"),parent);
  connect(selCol,SIGNAL(changed(const QColor &)),SLOT(changed()));
  r.moveBy(0,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  selCol->setGeometry(r);

  bold = new QCheckBox(i18n("Bold"),parent);
  connect(bold,SIGNAL(clicked()),SLOT(changed()));
  r.setRect(r.right() + 20,y + 25,70,25);
  bold->setGeometry(r);

  italic = new QCheckBox(i18n("Italic"),parent);
  connect(italic,SIGNAL(clicked()),SLOT(changed()));
  r.moveBy(0,25);
  italic->setGeometry(r);
}

void StyleChanger::setRef(ItemStyle *s) {

  style = s;
  col->setColor(style->col);
  selCol->setColor(style->selCol);
  bold->setChecked(style->bold);
  italic->setChecked(style->italic);

}

void StyleChanger::setEnabled(bool enable) {

  col->setEnabled(enable);
  selCol->setEnabled(enable);
  bold->setEnabled(enable);
  italic->setEnabled(enable);
}

void StyleChanger::changed() {

  if (style) {
    style->col = col->color();
    style->selCol = selCol->color();
    style->bold = bold->isChecked();
    style->italic = italic->isChecked();
  }
}

FontChanger::FontChanger(QWidget *parent, int x, int y)
  : QObject(parent) {

  QStrList fontList(true);
  QRect r;
  QLabel *label;
  int z;
  char s[4];

  getFontList(fontList);

  familyCombo = new QComboBox(true,parent);
  label = new QLabel(familyCombo,i18n("Family:"),parent);
  connect(familyCombo,SIGNAL(activated(const char *)),SLOT(familyChanged(const char *)));
  familyCombo->insertStrList(&fontList);

  r.setRect(x,y,160,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  familyCombo->setGeometry(r);

  sizeCombo = new QComboBox(true,parent);
  label = new QLabel(sizeCombo,i18n("Size:"),parent);
  connect(sizeCombo,SIGNAL(activated(int)),SLOT(sizeChanged(int)));
  z = 0;
  while (fontSizes[z]) {
    sprintf(s,"%d",fontSizes[z]);
    sizeCombo->insertItem(s);
    z++;
  }

  r.moveBy(0,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  sizeCombo->setGeometry(r);

  charsetCombo = new QComboBox(true,parent);
  label = new QLabel(charsetCombo,i18n("Charset:"),parent);
  connect(charsetCombo,SIGNAL(activated(const char *)),SLOT(charsetChanged(const char *)));

//  KCharsets *charsets=KApplication::getKApplication()->getCharsets();
//  QStrList lst = charsets->displayable(selFont.family());

  r.moveBy(0,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  charsetCombo->setGeometry(r);

}

void FontChanger::setRef(ItemFont *f) {
  int z;

  font = f;
  for (z = 0; z < (int) familyCombo->count(); z++) {
    if (font->family == familyCombo->text(z)) {
      familyCombo->setCurrentItem(z);
      goto found;
    }
  }
  font->family = familyCombo->text(0);
found:

  for (z = 0; fontSizes[z] > 0; z++) {
    if (font->size == fontSizes[z]) {
      sizeCombo->setCurrentItem(z);
      break;
    }
  }
  displayCharsets();
}

void FontChanger::familyChanged(const char *family) {

  font->family = family;
  displayCharsets();
}

void FontChanger::sizeChanged(int n) {

  font->size = fontSizes[n];;
}

void FontChanger::charsetChanged(const char *charset) {

  font->charset = charset;
  //KCharset(chset).setQFont(font);
}

void FontChanger::displayCharsets() {
  int z;
  const char *charset;
  KCharsets *charsets;

  charsets = kapp->getCharsets();
  QStrList lst = charsets->displayable(font->family);
  charsetCombo->clear();
  for(z = 0; z < (int) lst.count(); z++) {
    charset = lst.at(z);
    charsetCombo->insertItem(charset);
    if ((QString) font->charset == charset) charsetCombo->setCurrentItem(z);
  }
  charset = "any";
  charsetCombo->insertItem(charset);
  if ((QString) font->charset == charset) charsetCombo->setCurrentItem(z);
}

//---------

DefaultsDialog::DefaultsDialog(HlManager *hlManager, ItemStyleList *styleList,
  ItemFont *font, QWidget *parent) : QDialog(parent,0L,true) {

  QGroupBox *group;
  QComboBox *styleCombo;
  QLabel *label;
  FontChanger *fontChanger;
  QPushButton *button;
  QRect r, gr;
  int z;

  group = new QGroupBox(i18n("Default Item Styles"),this);
  gr.setRect(10,10,200,180);
  group->setGeometry(gr);
  styleCombo = new QComboBox(false,group);
  label = new QLabel(styleCombo,i18n("Item:"),group);
  connect(styleCombo,SIGNAL(activated(int)),this,SLOT(changed(int)));
  r.setRect(10,15,160,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  styleCombo->setGeometry(r);
  styleChanger = new StyleChanger(group,r.x(),r.y() + 25);
  connect(styleCombo,SIGNAL(activated(int)),this,SLOT(changed(int)));

  for (z = 0; z < hlManager->defaultStyles(); z++) {
    styleCombo->insertItem(hlManager->defaultStyleName(z));
  }
//  for (defStyle = defStyleList->first(); defStyle != 0L;
//    defStyle = defStyleList->next()) {

//    styleCombo->insertItem(defStyle->name);
//  }

  group = new QGroupBox(i18n("Default Font"),this);
  gr.moveBy(gr.width() + 10,0);
  group->setGeometry(gr);
  fontChanger = new FontChanger(group,10,15);
  fontChanger->setRef(font);

  itemStyleList = styleList;
  changed(0);

  button = new QPushButton(i18n("&OK"),this);
  button->setDefault(true);
  r.setRect(10,210,70,25);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(accept()));

  button = new QPushButton(i18n("&Cancel"),this);
  r.moveBy(gr.right() - r.width() -5,0);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));
}

void DefaultsDialog::changed(int z) {

  styleChanger->setRef(itemStyleList->at(z));
}


HighlightDialog::HighlightDialog(HlManager *hlManager,
  HlDataList *highlightDataList, QWidget *parent)
  : QDialog(parent,0L,true), hlData(0L) {

  QPushButton *button;
  QGroupBox *group;
  QLabel *label;
  QRect r, gr;
  int z;

  group = new QGroupBox(i18n("Config Select"),this);
  gr.setRect(10,10,200,130);
  group->setGeometry(gr);
  hlCombo = new QComboBox(false,group);
  label = new QLabel(hlCombo,i18n("Highlight:"),group);
  connect(hlCombo,SIGNAL(activated(int)),SLOT(hlChanged(int)));
  r.setRect(10,15,180,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  hlCombo->setGeometry(r);
  for (z = 0; z < hlManager->highlights(); z++) {
    hlCombo->insertItem(hlManager->hlName(z));
  }
//  for (highlight = hlList->first(); highlight != 0L; highlight = hlList->next()) {
//    hlCombo->insertItem(highlight->name());
//  }

  itemCombo = new QComboBox(false,group);
  label = new QLabel(itemCombo,i18n("Item:"),group);
  connect(itemCombo,SIGNAL(activated(int)),SLOT(itemChanged(int)));
  r.moveBy(0,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  itemCombo->setGeometry(r);

  group = new QGroupBox(i18n("Highlight Auto Select"),this);
  gr.moveBy(gr.width() + 10,0);
  group->setGeometry(gr);
  wildcards = new QLineEdit(group);
  label = new QLabel(wildcards,i18n("File Extensions:"),group);
  r.setRect(10,15,180,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  wildcards->setGeometry(r);

  mimetypes = new QLineEdit(group);
  label = new QLabel(mimetypes,i18n("Mime Types:"),group);
  r.moveBy(0,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  mimetypes->setGeometry(r);


  group = new QGroupBox(i18n("Item Style"),this);
  gr.setRect(10,gr.bottom()+1 + 15,200,205);
  group->setGeometry(gr);
  styleDefault = new QCheckBox(i18n("Default"),group);
  connect(styleDefault,SIGNAL(clicked()),SLOT(changed()));
  r.setRect(10,15,160,25);
  styleDefault->setGeometry(r);
  styleChanger = new StyleChanger(group,r.x(),r.y() + 25);

  group = new QGroupBox(i18n("Item Font"),this);
  gr.moveBy(gr.width() + 10,0);
  group->setGeometry(gr);
  fontDefault = new QCheckBox(i18n("Default"),group);
  connect(fontDefault,SIGNAL(clicked()),SLOT(changed()));
  fontDefault->setGeometry(r);
  fontChanger = new FontChanger(group,r.x(),r.y() + 25);


  hlDataList = highlightDataList;
  hlChanged(0);

  button = new QPushButton(i18n("&OK"),this);
  button->setDefault(true);
  r.setRect(10,370,70,25);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(accept()));

  button = new QPushButton(i18n("&Cancel"),this);
  r.moveBy(gr.right() - r.width() -5,0);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));
}

void HighlightDialog::hlChanged(int z) {
  ItemData *itemData;

  writeback();

  hlData = hlDataList->at(z);

  wildcards->setText(hlData->wildcards);
  mimetypes->setText(hlData->mimetypes);

  itemCombo->clear();
  for (itemData = hlData->itemDataList.first(); itemData != 0L;
    itemData = hlData->itemDataList.next()) {
    itemCombo->insertItem(itemData->name);
  }

  itemChanged(0);

  if (hlCombo->currentItem() != z)
    hlCombo->setCurrentItem(z);
}

void HighlightDialog::itemChanged(int z) {

  itemData = hlData->itemDataList.at(z);

  styleDefault->setChecked(itemData->defStyle);
  styleChanger->setRef(itemData);

  fontDefault->setChecked(itemData->defFont);
  fontChanger->setRef(itemData);
}

void HighlightDialog::changed() {
  itemData->defStyle = styleDefault->isChecked();
  itemData->defFont = fontDefault->isChecked();
}

void HighlightDialog::writeback() {
  if (hlData) {
    hlData->wildcards = wildcards->text();
    hlData->mimetypes = mimetypes->text();
  }
}

void HighlightDialog::done(int r) {
  writeback();
  QDialog::done(r);
}

