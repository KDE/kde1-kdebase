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
  "class","const","delete","friend","inline","new","private","protected",
  "public","this","virtual",0L};
char *bashKeywords[] = {
  "break","case","done","do","elif","else","esac","exit","export","fi","for",
  "function","if","in","return","select","then","until","while",".",0L};


HlItem::HlItem(int attribute, int context)
  : attr(attribute), ctx(context) {
}

HlItemWw::HlItemWw(int attribute, int context)
  : HlItem(attribute,context) {
}

bool HlItemWw::startEnable(char ch) {
  static char data[] = {0,0,0,0,0,64,255,3,254,255,255,135,254,255,255,7};
  if (ch & 128) return false;
  return !(data[ch >> 3] & (1 << (ch & 7)));
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



KeywordData::KeywordData(const char *str) {

  len = strlen(str);
  s = new char(len);
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
  bool b1, b2;

  b1 = false;
  while (*s >= '0' && *s <= '9') {
    s++;
    b1 = true;
  }
  if (*s == '.') s++; else return 0L;
  b2 = false;
  while (*s >= '0' && *s <= '9') {
    s++;
    b2 = true;
  }
  if (!b1 && !b2) return 0L;
  if (*s == 'E' || *s == 'e') s++; else return s;
  if (*s == '-') s++;
  b1 = false;
  while (*s >= '0' && *s <= '9') {
    s++;
    b1 = true;
  }
  if (b1) return s; else return 0L;
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

HlCStringCont::HlCStringCont(int attribute, int context)
  : HlItem(attribute,context) {
}

bool HlCStringCont::endEnable(char c) {
  return c == '\0';
}

const char *HlCStringCont::checkHgl(const char *s) {
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

bool HlCPrep::startEnable(char c) {
  return c == '\0';
}

const char *HlCPrep::checkHgl(const char *s) {

  while (*s == ' ' || *s == '\t') s++;
  if (*s == '#') {
    s++;
    return s;
  }
  return 0L;
}

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

HlHtmlTag::HlHtmlTag(int attribute, int context)
  : HlItem(attribute,context) {
}

bool HlHtmlTag::startEnable(char c) {
  return c == '<';
}

const char *HlHtmlTag::checkHgl(const char *s) {
  while (*s == ' ' || *s == '\t') s++;
  while (*s != ' ' && *s != '\t' && *s != '>' && *s != '\0') s++;
  return s;
}

HlHtmlValue::HlHtmlValue(int attribute, int context)
  : HlItem(attribute,context) {
}

bool HlHtmlValue::startEnable(char c) {
  return c == '=';
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
  : HlItemWw(attribute,context) {
}

const char *HlShellComment::checkHgl(const char *s) {
  if (*s == '#') {
    do s++; while (*s);
    return s;
  }
  return 0L;
}

HlContext::HlContext(int attribute, int lineEndContext)
  : attr(attribute), ctx(lineEndContext) {
  items.setAutoDelete(true);
}


Highlight::Highlight(const char *hName) : name(hName) {
  int z;

  for (z = 0; z < nAttribs; z++) {
    contextList[z] = 0;
    attribs[z] = 0;
  }
}

Highlight::~Highlight() {
  int z;

  for (z = 0; z < nAttribs; z++) {
    delete contextList[z];
    delete attribs[z];
  }
}

void Highlight::init() {
  makeDefAttribs();
  makeContextList();
  readConfig();
}

Attribute **Highlight::attrList() {
  return attribs;
}

void Highlight::getItemList(QStrList &list) {
  int z;
  Attribute *a;

  for (z = 0; z < nAttribs; z++) {
    a = attribs[z];
    if (a) {
      list.append(i18n(a->name));
    }
  }
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

void Highlight::readConfig() {
  KConfig *config;
  int z;
  Attribute *a;
  const char *s;

  config = kapp->getConfig();
  config->setGroup(name);
  for (z = 0; z < nAttribs; z++) {
    a = attribs[z];
    if (a) {
      s = a->name;
      a->col = config->readColorEntry(QString(s) + "_Col",&a->col);
      a->selCol = config->readColorEntry(QString(s) + "_SelCol",&a->selCol);
      a->setFont(config->readFontEntry(QString(s) + "_Font",&a->font));
    }
  }
}

void Highlight::writeConfig() {
  KConfig *config;
  int z;
  Attribute *a;
  const char *s;

  config = kapp->getConfig();
  config->setGroup(name);
  for (z = 0; z < nAttribs; z++) {
    a = attribs[z];
    if (a) {
      s = a->name;
      config->writeEntry(QString(s) + "_Col",a->col);
      config->writeEntry(QString(s) + "_SelCol",a->selCol);
      config->writeEntry(QString(s) + "_Font",a->font);
    }
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

void NoHighlight::makeDefAttribs() {

  attribs[0] = new Attribute("Normal Text",black,white,
    QFont("courier",12,QFont::Normal,false));
}

void NoHighlight::makeContextList() {
}


CHighlight::CHighlight(const char *hName) : Highlight(hName) {
}

CHighlight::~CHighlight() {
}

void CHighlight::makeDefAttribs() {
  QFont font1("courier",12,QFont::Normal,false);
  QFont font2("courier",12,QFont::Bold,false);
  QFont font3("courier",12,QFont::Normal,true);

  attribs[0] = new Attribute("Normal Text",black,white,font1);
  attribs[1] = new Attribute("Keyword",black,white,font2);
  attribs[2] = new Attribute("Decimal",blue,cyan,font1);
  attribs[3] = new Attribute("Octal",darkCyan,cyan,font1);
  attribs[4] = new Attribute("Hex",darkCyan,cyan,font1);
  attribs[5] = new Attribute("Float",darkMagenta,cyan,font1);
  attribs[6] = new Attribute("Char",magenta,magenta,font1);
  attribs[7] = new Attribute("String",red,red,font1);
  attribs[8] = new Attribute("String Char",magenta,magenta,font1);
  attribs[9] = new Attribute("Comment",darkGray,gray,font3);
  attribs[10] = new Attribute("Preprocessor",darkGreen,green,font1);
  attribs[11] = new Attribute("Prep. Lib",darkYellow,yellow,font1);
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
    c->items.append(new HlCStringCont(7,8));
    c->items.append(new HlCStringChar(8,1));
    c->items.append(new HlCharDetect(7,0,'"'));
  contextList[2] = new HlContext(9,0);
  contextList[3] = c = new HlContext(9,3);
    c->items.append(new Hl2CharDetect(9,0,"*/"));
  contextList[4] = c = new HlContext(10,0);
    c->items.append(new HlCharDetect(11,5,'"'));
    c->items.append(new HlCharDetect(11,6,'<'));
    c->items.append(new Hl2CharDetect(9,2,"//"));
    c->items.append(new Hl2CharDetect(9,7,"/*"));
  contextList[5] = c = new HlContext(11,0);
    c->items.append(new HlCharDetect(11,4,'"'));
  contextList[6] = c = new HlContext(11,0);
    c->items.append(new HlCharDetect(11,4,'>'));
  contextList[7] = c = new HlContext(9,7);
    c->items.append(new Hl2CharDetect(9,4,"*/"));
  contextList[8] = new HlContext(0,1);

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

void HtmlHighlight::makeDefAttribs() {
  QFont font1("courier",12,QFont::Normal,false);
  QFont font2("courier",12,QFont::Bold,false);
  QFont font3("courier",12,QFont::Normal,true);

  attribs[0] = new Attribute("Normal Text",black,white,font1);
  attribs[1] = new Attribute("Char",darkGreen,green,font1);
  attribs[2] = new Attribute("Comment",darkGray,gray,font3);
  attribs[3] = new Attribute("Tag Text",black,white,font2);
  attribs[4] = new Attribute("Tag",darkMagenta,magenta,font2);
  attribs[5] = new Attribute("Tag Value",darkCyan,cyan,font1);
}

void HtmlHighlight::makeContextList() {
  HlContext *c;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(new HlHtmlChar(1,0));
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

BashHighlight::BashHighlight(const char *hName) : Highlight(hName) {
}

BashHighlight::~BashHighlight() {
}

void BashHighlight::makeDefAttribs() {
  QFont font1("courier",12,QFont::Normal,false);
  QFont font2("courier",12,QFont::Bold,false);
  QFont font3("courier",12,QFont::Normal,true);

  attribs[0] = new Attribute("Normal Text",black,white,font1);
  attribs[1] = new Attribute("Keyword",black,white,font2);
  attribs[2] = new Attribute("Integer",blue,cyan,font1);
  attribs[3] = new Attribute("String",red,magenta,font1);
  attribs[4] = new Attribute("Substitution",darkCyan,cyan,font1);
  attribs[5] = new Attribute("Comment",darkGray,gray,font3);
}

void BashHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;
  char **w;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlCharDetect(3,1,'"'));
    c->items.append(new HlCharDetect(4,2,'`'));
    c->items.append(new HlShellComment(5,0));
  contextList[1] = c = new HlContext(3,0);
    c->items.append(new HlCharDetect(3,0,'"'));
  contextList[2] = c = new HlContext(4,0);
    c->items.append(new HlCharDetect(4,0,'`'));

  w = bashKeywords;
  while (*w) {
    keyword->addWord(*w);
    w++;
  }
}



HighlightDialog::HighlightDialog(QStrList &types, QWidget *parent,
  const char *newHlSlot)
  : QDialog(parent,0L,true) {
  QPushButton *button;
  QRect r;

  highlight = 0L;
  connect(this,SIGNAL(newHl(int)),parent,newHlSlot);

  typeLB = new QListBox(this);
  typeLB->insertStrList(&types);
  connect(typeLB,SIGNAL(highlighted(int)),this,SIGNAL(newHl(int)));
  r.setRect(10,10,120,220);
  typeLB->setGeometry(r);

  itemLB = new QListBox(this);
  connect(itemLB,SIGNAL(highlighted(int)),this,SLOT(newItem(int)));
  r.setRect(r.right() + 10,r.y(),160,r.height());
  itemLB->setGeometry(r);


  col = new KColorButton(this);
  connect(col,SIGNAL(changed(const QColor &)),this,SLOT(newCol(const QColor &)));
  r.setRect(r.right() + 10,r.y(),70,25);
  col->setGeometry(r);

  selCol = new KColorButton(this);
  connect(selCol,SIGNAL(changed(const QColor &)),this,SLOT(newSelCol(const QColor &)));
  r.moveBy(0,30);
  selCol->setGeometry(r);

  button = new QPushButton(i18n("&New Font"),this);
  button->setDefault(true);
  r.moveBy(0,30);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(newFont()));

  button = new QPushButton(i18n("&OK"),this);
  button->setDefault(true);
  r.moveBy(0,30);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(accept()));

  typeLB->setCurrentItem(2);
}

void HighlightDialog::newHl(Highlight *hl) {
  QStrList items;
printf("HighlightDialog::newHl()\n");

  if (highlight) {
    highlight->writeConfig();
printf("HighlightDialog::newHl() del\n");
    delete highlight;
  }
  highlight = hl;
  hl->getItemList(items);
  itemLB->clear();
  itemLB->insertStrList(&items);
}

Highlight *HighlightDialog::getHighlight(QStrList &types,
  QWidget *parent, const char *newHlSlot) {

  HighlightDialog *dlg;
  Highlight *highlight;

  dlg = new HighlightDialog(types,parent,newHlSlot);
  dlg->exec();
  highlight = dlg->highlight;

  delete dlg;
  if (highlight) highlight->writeConfig();
  return highlight;
}

void HighlightDialog::newItem(int index) {
  a = highlight->attrList()[index];
  col->setColor(a->col);
  selCol->setColor(a->selCol);
}

void HighlightDialog::newCol(const QColor &c) {
  a->col = c;
}

void HighlightDialog::newSelCol(const QColor &c) {
  a->selCol = c;
}

void HighlightDialog::newFont() {
  if (a) {
    QFont font = a->font;
    if (KFontDialog::getFont(font)) a->setFont(font);
  }
}
