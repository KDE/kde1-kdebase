#include <string.h>

#include <kapp.h>
#include <kfontdialog.h>

#include "highlight.h"

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
char *bashKeywords[] = {
  "break","case","done","do","elif","else","esac","exit","export","fi","for",
  "function","if","in","return","select","then","until","while",".",0L};

char *modulaKeywords[] = {
  "BEGIN","CONST","DEFINITION","DIV","DO","ELSE","ELSIF","END","FOR","FROM",
  "IF","IMPLEMENTATION","IMPORT","MODULE","MOD","PROCEDURE","RECORD","REPEAT",
  "RETURN","THEN","TYPE","VAR","WHILE","WITH","|",0L};


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
        if (*s >= '0' && *s <= '9') n += *s - '0'; else
        if (*s >= 'A' && *s <= 'F') n += *s - 'A' + 10; else
        if (*s >= 'a' && *s <= 'f') n += *s - 'a' + 10; else
        break;
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
        if (n >= 256) return 0L;
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
/*
HlHtmlChar::HlHtmlChar(int attribute, int context)
  : HlItem(attribute,context) {
}

const char *HlHtmlChar::checkHgl(const char *s) {
  if (*s == '&') {
    do {
      s++;
      if (!*s) return 0L;
    } while (*s != ';');
    return s + 1;
  }
  return 0L;
}
*/
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
/*
const char *HlShellComment::checkHgl(const char *s) {
  if (*s == '#') {
    do s++; while (*s);
    return s;
  }
  return 0L;
}
*/
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


HlContext::HlContext(int attribute, int lineEndContext)
  : attr(attribute), ctx(lineEndContext) {
  items.setAutoDelete(true);
}


QList<Attribute> Highlight::attList;
QList<HlContext> Highlight::ctxList;


Highlight::Highlight(const char *hName) : name(hName) {
  int z;

  for (z = 0; z < nAttribs; z++) {
    contextList[z] = 0;
  }
}

Highlight::~Highlight() {
  int z;

  for (z = 0; z < nAttribs; z++) {
    delete contextList[z];
  }
}

Attribute **Highlight::attrList()
{
printf("attrList()\n");
  uint i;

  for (i=0; i<nAttribs; i++)
    attribs[i] = 0;
  for (i=0; i<attList.count(); i++)
    attribs[i] = attList.at(i);

  return attribs;
}


void Highlight::init() {
  makeContextList();
  readConfig();
}

void Highlight::getItemList(QStrList &list) 
{
  for (Attribute *att = attList.first(); att != 0; att = attList.next())
    list.append(i18n(att->getName()));
}

bool Highlight::isInWord(char ch) {
  static char data[] = {0,0,0,0,0,0,255,3,254,255,255,135,254,255,255,7};
  if (ch & 128) return true;
  return data[ch >> 3] & (1 << (ch & 7));
}

void Highlight::doHighlight(int ctxNum, TextLine *textLine) {
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


void Highlight::readConfig() 
{
  QColor defaultSel, defaultFore, fore, back;
  QFont defaultFont, courierFont("courier",12), font;
  QString name, nrs;
  OverrideFlags flags;
  uint nr;

  KConfig *config = kapp->getConfig();

  // Clear the list, in case we are called more than once.
  attList.clear();

  // Read in the attributes. Note that the attributes are global to all
  // highlightings.

  // First the default font is read in.  
  config->setGroup("Attributes");
  defaultFont = config->readFontEntry("DefaultFont",&courierFont);
  defaultSel = config->readColorEntry("DefaultBackground",&white);
  defaultFore = config->readColorEntry("DefaultColor",&black);
  attList.append(new Attribute("Normal Text",defaultFore,defaultSel,defaultFont));
 
  // update the default attribute
  Attribute::DefaultFont = defaultFont;
  Attribute::DefaultColor = defaultFore;
  Attribute::DefaultSelColor = defaultSel;

  // create the default attribute set
  attList.append(new Attribute("Keyword",blue,white,courierFont,Color));
  attList.append(new Attribute("Number",darkGreen,white,courierFont,Color));
  attList.append(new Attribute("Char",red,white,courierFont,Color));
  attList.append(new Attribute("String",red,white,courierFont,Color));
  attList.append(new Attribute("String character",red,white,courierFont,Color));
  attList.append(new Attribute("Comment",gray,white,QFont("courier",12,QFont::Normal,true),(OverrideFlags)(FontStyle|Color)));
  attList.append(new Attribute("Preprocessor",darkYellow,white,courierFont,Color));
  attList.append(new Attribute("Preprocessor Library",darkYellow,white,courierFont,Color));
  attList.append(new Attribute("Tag text",blue,white,courierFont,Color));
  attList.append(new Attribute("Tag",blue,white,courierFont,Color));
  attList.append(new Attribute("Tag value",blue,white,courierFont,Color));
  attList.append(new Attribute("Substitution",blue,white,courierFont,Color));

  // Read in the attributes, if available.
  nr = config->readNumEntry("Number",1);
  for (uint index=1; index<nr; index++)
  {
    nrs.sprintf("Font_%d", index);
    font = config->readFontEntry(nrs.data(), &defaultFont);
    nrs.sprintf("Selection_%d", index);    
    back = config->readColorEntry(nrs.data(), &defaultSel);
    nrs.sprintf("Color_%d", index);    
    fore = config->readColorEntry(nrs.data(), &defaultFore);
    nrs.sprintf("Name_%d", index);    
    name = config->readEntry(nrs.data(), "unnamed");    
    nrs.sprintf("Override_%d", index);
    flags = (OverrideFlags) config->readNumEntry(nrs.data(), All);
    if (index < attList.count())
      attList.remove(index);
    attList.insert(index, new Attribute(name,fore,back,font,flags));
  }
}

void Highlight::writeConfig() 
{
  QString nrs;
  KConfig *config = kapp->getConfig();

  // First the default font is written.
  config->setGroup("Attributes");
  config->writeEntry("DefaultFont", Attribute::DefaultFont);
  config->writeEntry("DefaultColor", Attribute::DefaultColor);
  config->writeEntry("DefaultSelection", Attribute::DefaultSelColor);

  // Write out the rest of the attributes.
  int nr = attList.count();
  config->writeEntry("Number", nr);
  for (int index=1; index<nr; index++)
  {
    nrs.sprintf("Font_%d", index);
    config->writeEntry(nrs.data(), attList.at(index)->getFont());
    nrs.sprintf("Selection_%d", index);    
    config->writeEntry(nrs.data(), attList.at(index)->getSelColor());
    nrs.sprintf("Color_%d", index);    
    config->writeEntry(nrs.data(), attList.at(index)->getColor());
    nrs.sprintf("Name_%d", index);    
    config->writeEntry(nrs.data(), attList.at(index)->getName());
    nrs.sprintf("Override_%d", index);
    config->writeEntry(nrs.data(), (int)attList.at(index)->getOverrideFlags());
  }
}


NoHighlight::NoHighlight(const char *hName) : Highlight(hName) {
}

NoHighlight::~NoHighlight() {
}

void NoHighlight::doHighlight(int, TextLine *textLine) {

    textLine->setAttribs(0,0,textLine->length());
    textLine->setAttr(0);
}

void NoHighlight::makeContextList() {
}


CHighlight::CHighlight(const char *hName) : Highlight(hName) {
}

CHighlight::~CHighlight() {
}

void CHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlCInt(2,0));
    c->items.append(new HlCOct(2,0));
    c->items.append(new HlCHex(2,0));
    c->items.append(new HlCFloat(2,0));
    c->items.append(new HlCChar(3,0));
    c->items.append(new HlCharDetect(4,1,'"'));
    c->items.append(new Hl2CharDetect(6,2,"//"));
    c->items.append(new Hl2CharDetect(6,3,"/*"));
    c->items.append(new HlCPrep(7,4));
  contextList[1] = c = new HlContext(4,0);
    c->items.append(new HlLineContinue(4,6));
    c->items.append(new HlCStringChar(5,1));
    c->items.append(new HlCharDetect(4,0,'"'));
  contextList[2] = new HlContext(6,0);
  contextList[3] = c = new HlContext(6,3);
    c->items.append(new Hl2CharDetect(6,0,"*/"));
  contextList[4] = c = new HlContext(7,0);
    c->items.append(new HlLineContinue(7,7));
    c->items.append(new HlRangeDetect(8,4,"\"\""));
    c->items.append(new HlRangeDetect(8,4,"<>"));
//    c->items.append(new HlCharDetect(8,5,'"'));
//    c->items.append(new HlCharDetect(8,6,'<'));
    c->items.append(new Hl2CharDetect(6,2,"//"));
    c->items.append(new Hl2CharDetect(6,5,"/*"));
//  contextList[5] = c = new HlContext(8,0);
//    c->items.append(new HlCharDetect(8,4,'"'));
//  contextList[6] = c = new HlContext(8,0);
//    c->items.append(new HlCharDetect(8,4,'>'));
  contextList[5] = c = new HlContext(6,5);
    c->items.append(new Hl2CharDetect(6,4,"*/"));
  contextList[6] = new HlContext(0,1);
  contextList[7] = new HlContext(0,4);

  setKeywords(keyword);
}

void CHighlight::setKeywords(HlKeyword *keyword) {

//  keyword->addList(cTypes);
  keyword->addList(cKeywords);
}

CppHighlight::CppHighlight(const char *hName) : CHighlight(hName) {
}

CppHighlight::~CppHighlight() {
}

void CppHighlight::setKeywords(HlKeyword *keyword) {

//  keyword->addList(cTypes);
//  keyword->addList(cppTypes);
  keyword->addList(cKeywords);
  keyword->addList(cppKeywords);
}

HtmlHighlight::HtmlHighlight(const char *hName) : Highlight(hName) {
}

HtmlHighlight::~HtmlHighlight() {
}

void HtmlHighlight::makeContextList() {
  HlContext *c;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(new HlRangeDetect(3,0,"&;"));
    c->items.append(new HlStringDetect(6,1,"<!--"));
    c->items.append(new HlStringDetect(6,2,"<COMMENT>"));
    c->items.append(new HlCharDetect(9,3,'<'));
  contextList[1] = c = new HlContext(6,1);
    c->items.append(new HlStringDetect(6,0,"-->"));
  contextList[2] = c = new HlContext(6,2);
    c->items.append(new HlStringDetect(6,0,"</COMMENT>"));
  contextList[3] = c = new HlContext(9,3);
    c->items.append(new HlHtmlTag(10,3));
    c->items.append(new HlHtmlValue(11,3));
    c->items.append(new HlCharDetect(9,0,'>'));
}

BashHighlight::BashHighlight(const char *hName) : Highlight(hName) {
}

BashHighlight::~BashHighlight() {
}

void BashHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlCharDetect(4,1,'"'));
    c->items.append(new HlCharDetect(12,2,'`'));
    c->items.append(new HlShellComment(6,3));
  contextList[1] = c = new HlContext(4,0);
    c->items.append(new HlCharDetect(4,0,'"'));
  contextList[2] = c = new HlContext(12,0);
    c->items.append(new HlCharDetect(12,0,'`'));
  contextList[3] = new HlContext(6,0);

  keyword->addList(bashKeywords);
}

ModulaHighlight::ModulaHighlight(const char *hName) : Highlight(hName) {
}

ModulaHighlight::~ModulaHighlight() {
}

void ModulaHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlMHex(2,0));
    c->items.append(new HlFloat(2,0));
    c->items.append(new HlCharDetect(4,1,'"'));
    c->items.append(new Hl2CharDetect(6,2,"(*"));
  contextList[1] = c = new HlContext(4,0);
    c->items.append(new HlCharDetect(4,0,'"'));
  contextList[2] = c = new HlContext(6,2);
    c->items.append(new Hl2CharDetect(6,0,"*)"));

  keyword->addList(modulaKeywords);
}