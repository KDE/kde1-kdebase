#include <stdio.h>
#include <qobject.h>
#include <qapp.h>
#include <qclipbrd.h>
#include <qfont.h>
#include <qpainter.h>
#include "kwview.h"
#include "kwdoc.h"
#include "highlight.h"


const taSelected = 0x40;
const taFound = 0x80;
const taSelectMask = taSelected | taFound;
const taAttrMask = ~taSelectMask;
const taShift = 6;

TextLine::TextLine(int attribute) : attr(attribute) {
  len = 0;
  size = 0;
  text = 0L;
  attribs = 0L;
  ctx = 0;
}

TextLine::~TextLine() {
  delete text;
  delete attribs;
}

void TextLine::move(int pos, int n) {

  if (pos >= len) {
    resize(pos + n);
    memset(&text[len],' ',pos - len);
    memset(&attribs[len],attr,pos - len + n);
    len = pos;
  } else {
    resize(len + n);
    memmove(&text[pos + n],&text[pos],len - pos);
    memmove(&attribs[pos + n],&attribs[pos],len - pos);
    memset(&attribs[pos],attribs[pos],n);
  }
  len += n;
}

void TextLine::ins(int pos, char c, int n) {

  move(pos,n);
  memset(&text[pos],c,n);
}

/*
void TextLine::ins(int pos, const char *s, int n) {

  move(pos,n);
  memcpy(&text[pos],s,n);
}
*/

void TextLine::overwrite(int pos, const char *s, int n) {

  if (n == -1) n = strlen(s);
  if (pos + n > len) {
    resize(pos + n);
    if (pos > len) memset(&text[len],' ',pos - len);
    memset(&attribs[len],attr,pos + n - len);
    len = pos + n;
  }
  memcpy(&text[pos],s,n);
}

void TextLine::append(char c, int n) {

  resize(len + n);
  memset(&text[len],c,n);
  memset(&attribs[len],attr,n);
  len += n;
}

void TextLine::del(int pos, int n) {
  int count;

  count = len - (pos + n);
  if (count <= 0) {
    if (len > pos) len = pos;
  } else {
    memmove(&text[pos],&text[pos + n],count);
    memmove(&attribs[pos],&attribs[pos + n],count);
    len -= n;
  }
}

int TextLine::length() {
  return len;
}

void TextLine::setLength(int l) {
  if (l >= len) {
    resize(l);
    memset(&text[len],' ',l - len);
    memset(&attribs[len],attr,l - len);
  } else attr = attribs[l];
  len = l;
}

void TextLine::truncate(int l) {
  if (l < len) {
    attr = attribs[l];
    len = l;
  }
}

void TextLine::append(TextLine *textLine, int pos) {
  int n;

  n = textLine->len - pos;
  if (n > 0) {
    resize(len + n);
    memcpy(&text[len],&textLine->text[pos],n);
    memcpy(&attribs[len],&textLine->attribs[pos],n);
    len += n;
  }
  attr = textLine->attr;
}


void TextLine::copy(TextLine *textLine) {

  resize(textLine->len);
  len = textLine->len;
  memcpy(text,textLine->text,len);
  memcpy(attribs,textLine->attribs,len);
  attr = textLine->attr;
}

void TextLine::insEnd(TextLine *textLine, int pos) {
  int l;

  l = textLine->len - pos;
  if (l > 0) {
    move(0,l);
    memcpy(text,&textLine->text[pos],l);
    memcpy(attribs,&textLine->attribs[pos],l);
  }
}

void TextLine::removeSpaces() {

  while (len > 0 && text[len - 1] == ' ') len--;
}

int TextLine::firstChar() {
  int z;

  z = 0;
  while (z < len && text[z] <= 32) z++;
  return (z < len) ? z : -1;
}

char TextLine::getChar( int pos ) const {
  if (pos < len) return text[pos];
  return ' ';
}

void TextLine::setAttribs(int attribute, int start, int end) {
  int z;

  if (end > len) end = len;
  for (z = start; z < end; z++) attribs[z] = (attribs[z] & taSelectMask) | attribute;
}

void TextLine::setAttr(int attribute) {
  attr = (attr & taSelectMask) | attribute;
}

int TextLine::getAttr(int pos) {
  if (pos < len) return attribs[pos] & taAttrMask;
  return attr & taAttrMask;
}

int TextLine::getAttr() {
  return attr & taAttrMask;
}

int TextLine::getRawAttr(int pos) {
  if (pos < len) return attribs[pos];
  return attr;
}

int TextLine::getRawAttr() {
  return attr;
}

void TextLine::setContext(int context) {
  ctx = context;
}

int TextLine::getContext() {
  return ctx;
}

const char* TextLine::getString() {
  resize(len+1);
  text[len] = '\0';
  return text;
}

const char* TextLine::getText() {
  return text;
}


void TextLine::select(bool sel, int start, int end) {
  int z;

  if (end > len) end = len;
  if (sel) {
    for (z = start; z < end; z++) attribs[z] |= taSelected;
  } else {
    for (z = start; z < end; z++) attribs[z] &= ~taSelected;
  }
}

void TextLine::selectEol(bool sel, int pos) {
  int z;

  if (sel) {
    for (z = pos; z < len; z++) attribs[z] |= taSelected;
    attr |= taSelected;
  } else {
    for (z = pos; z < len; z++) attribs[z] &= ~taSelected;
    attr &= ~taSelected;
  }
}


void TextLine::toggleSelect(int start, int end) {
  int z;

  if (end > len) end = len;
  for (z = start; z < end; z++) attribs[z] = attribs[z] ^ taSelected;
}


void TextLine::toggleSelectEol(int pos) {
  int z;

  for (z = pos; z < len; z++) attribs[z] = attribs[z] ^ taSelected;
  attr = attr ^ taSelected;
}


int TextLine::numSelected() {
  int z, n;

  n = 0;
  for (z = 0; z < len; z++) if (attribs[z] & taSelected) n++;
  return n;
}

bool TextLine::isSelected(int pos) {
  if (pos < len) return (attribs[pos] & taSelected);
  return (attr & taSelected);
}

bool TextLine::isSelected() {
  return (attr & taSelected);
}
/*
void TextLine::delSelected() {
  int s, e;

  s = -1;
  while (true) {
    do {
      s++;
      if (s >= len) return;
    } while (!(attribs[s] & taSelected));
    e = s+1;
    while (e < len && attribs[e] & taSelected) e++;
    del(s,e-s);
  }
}
*/

int TextLine::findSelected(bool sel, int pos) {
  if (sel) {
    while (pos < len && attribs[pos] & taSelected) pos++;
  } else {
    while (pos < len && !(attribs[pos] & taSelected)) pos++;
  }
  return pos;
}


int TextLine::cursorX(int pos, int tabChars) {
  int l, x, z;

  l = (pos < len) ? pos : len;
  x = 0;
  for (z = 0; z < l; z++) {
    if (text[z] == '\t') x += tabChars - (x % tabChars); else x++;
  }
  if (pos > len) x += pos - len;
  return x;
}
/*
int TextLine::find(const char *searchFor, int pos) {
  int slen, count;
  const char *s;
  char *t;

  slen = strlen(searchFor);
  count = len - slen - pos;

  while (count >= 0) {
    s = searchFor;
    t = &text[pos];
    while (*s && *s == *t) {
      s++;
      t++;
    }
    if (!*s) return pos + slen;
    pos++;
    count--;
  }
  return -1;
}
*/

void TextLine::markFound(int pos, int l) {
  int z;

  l += pos;
  if (l > len) l = len;
  for (z = pos; z < l; z++) attribs[z] |= taFound;
}

void TextLine::unmarkFound() {
  int z;
  for (z = 0; z < len; z++) attribs[z] &= ~taFound;
}


void TextLine::resize(int newsize) {
  char *newtext;
  unsigned char *newattribs;

  if (newsize > size) {
    if (newsize*2 < size*3) newsize = (size*3)/2;
    size = (newsize + 15) & (~15);

    newtext = new char[size];
    newattribs = new unsigned char[size];
if (!newtext || !newattribs) {
  printf("error resizing textline\n");
  exit(1);
}
    memcpy(newtext,text,len);
    memcpy(newattribs,attribs,len);
    delete text;
    delete attribs;
    text = newtext;
    attribs = newattribs;
  }
}


Attribute::Attribute() : font(), fm(font) {
}

Attribute::Attribute(const char *aName, const QColor &aCol,
  const QColor &aSelCol, const QFont &aFont)
  : name(aName), col(aCol), selCol(aSelCol), font(aFont), fm(font) {
}


void Attribute::setFont(const QFont &f) {
  font = f;
  fm = QFontMetrics(f);
}

KWriteDoc::KWriteDoc() : QObject(0L) {

  contents.setAutoDelete(true);
  contents.append(new TextLine());
/*
  for (int z = 0; z < nAttribs; z++) attribs[z] = 0;

  Attribute *a;
  QFont font1("courier",12,QFont::Normal,false);
  QFont font2("courier",12,QFont::Normal,true);
  QFont font3("courier",12,QFont::Bold,false);

  //normal
  a = new Attribute();
  a->col = black;
  a->selCol = white;
  a->setFont(font1);
  attribs[0] = a;

  //keyword
  a = new Attribute();
  a->col = black;
  a->selCol = white;
  a->setFont(font3);
  attribs[1] = a;

  //int
  a = new Attribute();
  a->col = blue;
  a->selCol = cyan;
  a->setFont(font1);
  attribs[2] = a;

  //octal
  a = new Attribute();
  a->col = darkCyan;
  a->selCol = cyan;
  a->setFont(font1);
  attribs[3] = a;

  //hex
  a = new Attribute();
  a->col = darkCyan;
  a->selCol = cyan;
  a->setFont(font1);
  attribs[4] = a;

  //float
  a = new Attribute();
  a->col = darkMagenta;
  a->selCol = cyan;
  a->setFont(font1);
  attribs[5] = a;

  //char
  a = new Attribute();
  a->col = magenta;
  a->selCol = magenta;
  a->setFont(font1);
  attribs[6] = a;

  //string
  a = new Attribute();
  a->col = red;
  a->selCol = magenta;
  a->setFont(font1);
  attribs[7] = a;

  //string char
  a = new Attribute();
  a->col = magenta;
  a->selCol = magenta;
  a->setFont(font1);
  attribs[8] = a;

  //line comment, comment
  a = new Attribute();
  a->col = darkGray;
  a->selCol = gray;
  a->setFont(font2);
  attribs[9] = a;
  attribs[10] = a;
  attribs[14] = a;
  attribs[15] = a;

  //preprocessor
  a = new Attribute();
  a->col = darkGreen;
  a->selCol = green;
  a->setFont(font1);
  attribs[11] = a;

  //prep string, prep lib
  a = new Attribute();
  a->col = darkYellow;
  a->selCol = yellow;
  a->setFont(font1);
  attribs[12] = a;
  attribs[13] = a;
*/
  selCols[0] = white;
  selCols[1] = darkBlue;
  selCols[2] = black;
  selCols[3] = black;

  highlight = new CppHighlight("C++ Highlight");
  highlight->init();
  attribs = highlight->attrList();
  tabChars = 8;
  updateFontData();


  bufferLine = new TextLine();

  modified = false;
  clear();
}

KWriteDoc::~KWriteDoc() {
}

int KWriteDoc::numLines() const {
  return contents.count();
}

TextLine *KWriteDoc::textLine(int line) {
  return contents.at(line);
/*
  TextLine *textLine;

  if (line < 0 || line >= (int) contents.count()) line = contents.count() -1;
  textLine = contents.at(line);

  return textLine->getString();
*/
}

void KWriteDoc::tagLines(int start, int end) {
  int z;

  for (z = 0; z < (int) views.count(); z++) {
    views.at(z)->tagLines(start,end);
  }
}
/*
void KWriteDoc::tagAll() {
  int z;

  for (z = 0; z < (int) views.count(); z++) {
    views.at(z)->tagAll();
  }
}
*/

void KWriteDoc::readSessionConfig(KConfig *config) {
   fName = config->readEntry("URL");
   setTabWidth(config->readNumEntry("TabWidth",8));
}

void KWriteDoc::writeSessionConfig(KConfig *config) {
  config->writeEntry("URL",fName);
  config->writeEntry("TabWidth",tabChars);
}

void KWriteDoc::registerView(KWriteView *view) {
  views.append(view);
}

void KWriteDoc::removeView(KWriteView *view) {
  views.remove(view);
}


int KWriteDoc::currentColumn(PointStruc &cursor) {
  return contents.at(cursor.y)->cursorX(cursor.x,tabChars);
}

void KWriteDoc::insert(VConfig &c, const char *s) {
  TextLine *textLine;

  if (!s || !*s) return;
  if (!(c.flags & cfVerticalSelect)) {
    int x, l;

    textLine = contents.at(c.cursor.y);

    bufferLine->truncate(0);
    bufferLine->append(textLine,c.cursor.x);
    textLine->setLength(c.cursor.x);
    x = textLine->cursorX(c.cursor.x,tabChars);

    while (*s != 0) {
      if ((unsigned char) *s >= 32) {
        textLine->append(*s);
        x++;
      } else if (*s == '\t') {
        if (c.flags & cfReplaceTabs) {
          l = tabChars - (x % tabChars);
          textLine->append(' ',l);
          x += l;
        } else textLine->append('\t');
      } else if (*s == '\n') {
        x = 0;
  //      updateMaxLength(textLine);
        c.cursor.y++;
        textLine = new TextLine(textLine->getRawAttr());
        contents.insert(c.cursor.y,textLine);
      }
      s++;
    }

    c.cursor.x = textLine->length();
    textLine->append(bufferLine);

  //  updateMaxLength(textLine);

    update(c);
  } else {
    int yStart;
    int xPos, pos, l;

    yStart = c.cursor.y;
    xPos = textWidth(c.cursor);
    textLine = contents.at(c.cursor.y);
    pos = c.cursor.x;
    while (*s != 0) {
      if ((unsigned char) *s >= 32) {
        textLine->ins(pos,*s);
        pos++;
      } else if (*s == '\t') {
        if (c.flags & cfReplaceTabs) {
          l = tabChars - (textLine->cursorX(pos,tabChars) % tabChars);
          textLine->ins(pos,' ',l);
          pos += l;
        } else {
          textLine->ins(pos,'\t');
          pos++;
        }
      } else if (*s == '\n') {
        c.cursor.y++;
        if (c.cursor.y >= (int) contents.count()) {
          textLine = new TextLine();
          contents.append(textLine);
        } else textLine = contents.at(c.cursor.y);
        pos = textPos(textLine,xPos);
      }
      s++;
    }
//    tagLines(yStart,c.cursor.y);
    updateLines(c.flags,yStart,c.cursor.y);
  }
}

void KWriteDoc::insertFile(VConfig &c, QIODevice &dev) {
  TextLine *textLine;
  int x, l, len;
  char buf[512];
  char *s;


  textLine = contents.at(c.cursor.y);

  bufferLine->truncate(0);
  bufferLine->append(textLine,c.cursor.x);
  textLine->setLength(c.cursor.x);
  x = textLine->cursorX(c.cursor.x,tabChars);

  do {
    len = dev.readBlock(buf,512);
    s = buf;
    while (len > 0) {
      if ((unsigned char) *s >= 32) {
        textLine->append(*s);
        x++;
      } else if (*s == '\t') {
        if (c.flags & cfReplaceTabs) {
          l = tabChars - (x % tabChars);
          textLine->append(' ',l);
          x += l;
        } else textLine->append('\t');
      } else if (*s == '\n') {
        x = 0;
//        updateMaxLength(textLine);
        c.cursor.y++;
        textLine = new TextLine(textLine->getRawAttr());
        contents.insert(c.cursor.y,textLine);
      } else if (*s == '\r') {
        printf("dos text\n");
      }
      s++;
      len--;
    }
  } while (s != buf);

  c.startCursor.x++;
  c.cursor.x = textLine->length() +1;
  textLine->append(bufferLine);

//  updateMaxLength(textLine);

  update(c);
}

void KWriteDoc::writeFile(QIODevice &dev) {
  int lines, line;
  TextLine *textLine;

  lines = contents.count();

  line = 0;
  do {
    textLine = contents.at(line);
    dev.writeBlock(textLine->getText(),textLine->length());
    line++;
    if (line >= lines) break;
    dev.putch('\n');
  } while (true);
}

void KWriteDoc::insertChar(VConfig &c, char ch) {
  TextLine *textLine;
  int l;

  textLine = contents.at(c.cursor.y);

  if (ch == '\t' && c.flags & cfReplaceTabs) {
    l = tabChars - (textLine->cursorX(c.cursor.x,tabChars) % tabChars);
    ch = ' ';
  } else l = 1;
  if (c.flags & cfOvr) textLine->del(c.cursor.x,l);
  textLine->ins(c.cursor.x,ch,l);
  c.cursor.x += l;

  if (c.flags & cfAutoBrackets) {
    if (c.flags & cfOvr) textLine->del(c.cursor.x,1);
    if (ch == '(') textLine->ins(c.cursor.x,')');
    if (ch == '[') textLine->ins(c.cursor.x,']');
    if (ch == '{') textLine->ins(c.cursor.x,'}');
  }


  if (c.flags & cfWordWrap) {
    int startLine, line;
    const char *s;
    int z, pos;
    TextLine *nextLine;
    PointStruc start, end;

    if (!(c.flags & cfPersistent)) deselectAll();
    updateCursors(c.startCursor,c.cursor);

    startLine = line = c.cursor.y;
    do {
      s = textLine->getText();
      l = textLine->length();
      for (z = c.wrapAt; z < l; z++) if (s[z] > 32) break;
      if (z >= l) break; // nothing more to wrap
      pos = c.wrapAt;
      for (; z >= 0; z--) {
        if (s[z] <= 32) {
          pos = z + 1;
          break;
        }
      }
      // do indent
      start.x = pos;
      start.y = line;

      line++;
      if (line >= (int) contents.count()) {
        nextLine = new TextLine;
        contents.append(nextLine);
      } else nextLine = contents.at(line);

      nextLine->insEnd(textLine,pos);
      textLine->truncate(pos);

      end.x = 0;
      end.y = line;
      updateCursors(start,end,false);

      textLine = nextLine;
    } while (true);

    updateLines(c.flags,startLine,line);
  } else update(c);
}

void KWriteDoc::newLine(VConfig &c) {
  TextLine *textLine, *newLine;
  int pos;

  newLine = new TextLine();
  textLine = contents.at(c.cursor.y);

  if (!(c.flags & cfAutoIndent)) {
    newLine->append(textLine,c.cursor.x);
    pos = 0;
  } else {
    TextLine *indentLine;
    int z;

    pos = textLine->firstChar();
    if (c.cursor.x > pos) pos = c.cursor.x;
    newLine->append(textLine,pos);

    z = c.cursor.y;
    do {
      indentLine = contents.at(z);
      pos = indentLine->firstChar();
      if (pos >= 0) break;
      z--;
    } while (z >= 0);
    if (pos > 0) {
      newLine->move(0,pos);
      newLine->overwrite(0,indentLine->getText(),pos);
    } else pos = 0;
  }
  textLine->truncate(c.cursor.x);
  c.cursor.x = pos;
  c.cursor.y++;
  contents.insert(c.cursor.y,newLine);

//  updateMaxLength(newLine);
//  updateMaxLength(textLine);

  update(c);
}

void KWriteDoc::killLine(VConfig &c) {
  TextLine *textLine;

  textLine = contents.at(c.cursor.y);

  textLine->truncate(0);
  updateMaxLength(textLine);

  c.cursor.x = 0;
  if ((int) contents.count() -1 > c.cursor.y) {
    contents.remove(c.cursor.y);
  }

  c.startCursor.x = 0;
  c.startCursor.y = c.cursor.y + 1;

  update(c);
}

void KWriteDoc::backspace(VConfig &c) {
  TextLine *textLine, *upLine;

  textLine = contents.at(c.cursor.y);

  if (c.cursor.x > 0) {
    if (!(c.flags & cfBackspaceIndent)) {
      c.cursor.x--;
      textLine->del(c.cursor.x);
    } else {
      TextLine *indentLine;
      int z, pos;

      z = c.cursor.y - 1;
      pos = textLine->firstChar();
      if (pos >= 0 && pos < c.cursor.x) pos = 0;
      while (z >= 0 && pos != 0) {
        indentLine = contents.at(z);
        pos = indentLine->firstChar();
        if (pos >= 0 && pos < c.cursor.x) goto found;
        z--;
      };
      pos = c.cursor.x - 1;
      found:
      textLine->del(pos,c.cursor.x - pos);
      c.cursor.x = pos;
    }
//    updateMaxLength(textLine);

    update(c);
  } else {
    if (c.cursor.y > 0) {

      upLine = contents.at(c.cursor.y-1);
      c.cursor.x = upLine->length();
      upLine->append(textLine);
      textLine->truncate(0);

//      updateMaxLength(upLine);
      updateMaxLength(textLine);

      contents.remove(c.cursor.y);
      c.cursor.y--;

      update(c);
    }
  }
}


void KWriteDoc::del(VConfig &c) {
  TextLine *textLine, *upLine;

  textLine = contents.at(c.cursor.y);

  if (c.cursor.x < textLine->length()) {
    c.startCursor.x++;
    textLine->del(c.cursor.x);
//    updateMaxLength(textLine);
    update(c);
  } else {
    if (c.cursor.y < (int) contents.count() -1) {
      c.startCursor.y++;
      c.startCursor.x = 0;

      upLine = contents.at(c.startCursor.y);
      textLine->setLength(c.cursor.x);
      textLine->append(upLine);
      upLine->truncate(0);

//      updateMaxLength(textLine);
      updateMaxLength(upLine);
      contents.remove(c.startCursor.y);
      update(c);
    }
  }
}

void KWriteDoc::clipboardChanged() {
#if defined(_WS_X11_)
  disconnect(QApplication::clipboard(),SIGNAL(dataChanged()),
    this,SLOT(clipboardChanged()));
  deselectAll();
  updateViews();
#endif
}

void KWriteDoc::updateFontData() {
  int maxAscent, maxDescent;
  int i, midTabWidth;
  int z;
  Attribute *a;

  maxAscent = 0;
  maxDescent = 0;
  midTabWidth = 0;

  i = 0;
  for (z = 0; z < nAttribs; z++) {
    a = attribs[z];
    if (a) {
      if (a->fm.ascent() > maxAscent) maxAscent = a->fm.ascent();
      if (a->fm.descent() > maxDescent) maxDescent = a->fm.descent();
      midTabWidth += a->fm.width('x');
      i++;
    }
  }

  fontHeight = maxAscent + maxDescent + 1;
  fontAscent = maxAscent;
  tabWidth = tabChars*midTabWidth/i;

  for (z = 0; z < (int) views.count(); z++) {
    KWriteView *view = views.at(z);
    resizeBuffer(view,view->width(),fontHeight);
  }
}

void KWriteDoc::setHighlight(Highlight *hl) {

  delete highlight;
  highlight = hl;
  attribs = hl->attrList();
  updateFontData();
  hl->doHighlight(*this,0,(int) contents.count() -1);
}

void KWriteDoc::setTabWidth(int chars) {

  if (chars < 1) chars = 1;
  if (chars > 16) chars = 16;
  tabChars = chars;
  updateFontData();
}


void KWriteDoc::update(VConfig &c) {
  int dy;

  if (c.flags & cfPersistent) {
    dy = c.cursor.y - c.startCursor.y;
    if (selectStart > c.startCursor.y) selectStart += dy;
      else if (selectStart > c.cursor.y) selectStart = c.cursor.y;
    if (selectEnd >= c.startCursor.y) selectEnd += dy;
      else if (selectEnd > c.cursor.y) selectEnd = c.cursor.y;
  } else deselectAll();

  if (c.cursor.y > c.startCursor.y) {
    updateLines(c.flags,c.startCursor.y,c.cursor.y);
  } else {
    updateLines(c.flags,c.cursor.y,c.startCursor.y);
  }

  updateCursors(c.startCursor,c.cursor);
}


void KWriteDoc::updateLines(int flags, int startLine, int endLine) {
  int line;
  TextLine *textLine;

  if (endLine >= (int) contents.count()) endLine = (int) contents.count() -1;
  for (line = startLine; line <= endLine; line++) {
    textLine = contents.at(line);
    updateMaxLength(textLine);
    if (flags & cfRemoveSpaces) textLine->removeSpaces();
  }
  highlight->doHighlight(*this,startLine,endLine);
  setModified(true);
}


void KWriteDoc::updateMaxLength(TextLine *textLine) {
  int len, z;

  len = textWidth(textLine,textLine->length());

  if (len > maxLength) {
    longestLine = textLine;
    maxLength = len;
  } else {
    if (textLine == longestLine && len <= maxLength*3/4) {
      maxLength = -1;
      for (z = 0; z < (int) contents.count(); z++) {
        textLine = contents.at(z);
        len = textWidth(textLine,textLine->length());
        if (len > maxLength) {
          maxLength = len;
          longestLine = textLine;
        }
      }
    }
  }
}


void KWriteDoc::updateCursors(PointStruc &start, PointStruc &end, bool insert) {
  int z;

  for (z = 0; z < (int) views.count(); z++) {
    views.at(z)->updateCursor(start,end,insert);
  }
}

void KWriteDoc::updateViews(int flags) {
  int z;

  for (z = 0; z < (int) views.count(); z++) {
    views.at(z)->updateView(flags);
  }
}


int KWriteDoc::textLength(int line) {
  return contents.at(line)->length();
}

int KWriteDoc::textWidth(TextLine *textLine, int cursorX) {
  int x;
  int z;
  char ch;
  Attribute *a;

  x = 0;
  for (z = 0; z < cursorX; z++) {
    ch = textLine->getChar(z);
    a = attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->fm.width(&ch,1);
  }
  return x;
}

int KWriteDoc::textWidth(PointStruc &cursor) {
  if (cursor.x < 0) cursor.x = 0;
  if (cursor.y < 0) cursor.y = 0;
  if (cursor.y >= (int) contents.count()) cursor.y = (int) contents.count() -1;
  return textWidth(contents.at(cursor.y),cursor.x);
}

int KWriteDoc::textWidth(bool wrapCursor, PointStruc &cursor, int xPos) {
  TextLine *textLine;
  int len;
  int x, oldX;
  int z;
  char ch;
  Attribute *a;

  if (cursor.y < 0) cursor.y = 0;
  if (cursor.y >= (int) contents.count()) cursor.y = (int) contents.count() -1;
  textLine = contents.at(cursor.y);
  len = textLine->length();

  x = oldX = z = 0;
  while (x < xPos && (!wrapCursor || z < len)) {
    oldX = x;
    ch = textLine->getChar(z);
    a = attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->fm.width(&ch,1);
    z++;
  }
  if (xPos - oldX < x - xPos && z > 0) {
    z--;
    x = oldX;
  }
  cursor.x = z;
  return x;
}

int KWriteDoc::textPos(TextLine *textLine, int xPos) {
  int len;
  int x, oldX;
  int z;
  char ch;
  Attribute *a;

  len = textLine->length();

  x = oldX = z = 0;
  while (x < xPos) { // && z < len) {
    oldX = x;
    ch = textLine->getChar(z);
    a = attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->fm.width(&ch,1);
    z++;
  }
  if (xPos - oldX < x - xPos && z > 0) z--;
  return z;
}

int KWriteDoc::textWidth() {
  return maxLength + 8;
}

int KWriteDoc::textHeight() {
  return contents.count()*fontHeight;
}

void KWriteDoc::toggleRect(int x1, int y1, int x2, int y2) {
  int z;
  bool t;
  int start, end;
  TextLine *textLine;

  if (x1 > x2) {
    z = x1;
    x1 = x2;
    x2 = z;
  }
  if (y1 > y2) {
    z = y1;
    y1 = y2;
    y2 = z;
  }

  t = false;
  for (z = y1; z < y2; z++) {
    textLine = contents.at(z);
    start = textPos(textLine,x1);
    end = textPos(textLine,x2);
    if (end > start) {
      textLine->toggleSelect(start,end);
      t = true;
    }
  }
  if (t) {
    y2--;
    tagLines(y1,y2);
    if (selectEnd < selectStart || y1 < selectStart) selectStart = y1;
    if (y2 > selectEnd) selectEnd = y2;
  }
}

void KWriteDoc::selectTo(PointStruc &start, PointStruc &end, int flags) {

  if (start.x != select.x || start.y != select.y) {
    /* new selection */
    if (!(flags & cfKeepSelection)) deselectAll();
    anchor = start;
  }

  if (!(flags & cfVerticalSelect)) {
    //horizontal selections
    TextLine *textLine;
    int x, y;
    int xe, ye;
    bool sel;

    if (end.y > start.y || (end.y == start.y && end.x > start.x)) {
      x = start.x;
      y = start.y;
      xe = end.x;
      ye = end.y;
      sel = true;
    } else {
      x = end.x;
      y = end.y;
      xe = start.x;
      ye = start.y;
      sel = false;
    }
    tagLines(y,ye);

    if (selectEnd < selectStart || y < selectStart) selectStart = y;
    if (ye > selectEnd) selectEnd = ye;

    textLine = contents.at(y);
//    bufferLine->copy(textLine);

    if (flags & cfXorSelect) {
      //xor selection with old selection
      while (y < ye) {
        textLine->toggleSelectEol(x);
//        optimizedDrawLine(paint,fm,*textLine,*bufferLine,y);
        x = 0;
        y++;
        textLine = contents.at(y);
        bufferLine->copy(textLine);
      }
      textLine->toggleSelect(x,xe);
//      optimizedDrawLine(paint,fm,*textLine,*bufferLine,y);
    } else {
      //set selection over old selection

      if (anchor.y > y || (anchor.y == y && anchor.x > x)) {
        if (anchor.y < ye || (anchor.y == ye && anchor.x < xe)) {
          sel = !sel;
          while (y < anchor.y) {
            textLine->selectEol(sel,x);
//            optimizedDrawLine(paint,fm,*textLine,*bufferLine,y);
            x = 0;
            y++;
            textLine = contents.at(y);
            bufferLine->copy(textLine);
          }
          textLine->select(sel,x,anchor.x);
          x = anchor.x;
        }
        sel = !sel;
      }
      while (y < ye) {
        textLine->selectEol(sel,x);
//        optimizedDrawLine(paint,fm,*textLine,*bufferLine,y);
        x = 0;
        y++;
        textLine = contents.at(y);
        bufferLine->copy(textLine);
      }
      textLine->select(sel,x,xe);
//      optimizedDrawLine(paint,fm,*textLine,*bufferLine,y);
    }
  } else {
    //vertical (block) selections
    int ax, sx, ex;

    ax = textWidth(anchor);
    sx = textWidth(start);
    ex = textWidth(end);

    toggleRect(ax,start.y + 1,sx,end.y + 1);
    toggleRect(sx,anchor.y,ex,end.y + 1);
//    setLine(start.y, end.y);
  }
  select = end;
}

void KWriteDoc::clear() {
  PointStruc start, end;
  TextLine *textLine;

  start.x = 0;
  start.y = contents.count();
  end.x = 0;
  end.y = 0;

  updateCursors(start,end);

  contents.clear();
  textLine = new TextLine();
  contents.append(textLine);
  longestLine = textLine;
  maxLength = 0;

  select.x = -1;

  selectStart = 0;
  selectEnd = -1;

  foundLine = -1;

  setModified(false);
}


void KWriteDoc::copy(int flags) {

  if (selectEnd < selectStart) return;

  QString s = markedText(flags);
  if (!s.isEmpty()) {
#if defined(_WS_X11_)
    disconnect(QApplication::clipboard(),SIGNAL(dataChanged()),this,0);
#endif
    QApplication::clipboard()->setText(s);
#if defined(_WS_X11_)
    connect(QApplication::clipboard(),SIGNAL(dataChanged()),
      this,SLOT(clipboardChanged()));
#endif
  }
}

void KWriteDoc::paste(VConfig &c) {
  QString s = QApplication::clipboard()->text();
  if (!s.isEmpty()) {
    unmarkFound();
    insert(c,s);
  }
}


void KWriteDoc::cut(int flags) {

  if (selectEnd < selectStart) return;

  unmarkFound();
  copy(flags);
  delMarkedText(flags);
}

void KWriteDoc::selectAll() {
  int z;
  TextLine *textLine;

  select.x = -1;

  unmarkFound();
  selectStart = 0;
  selectEnd = contents.count() -1;

  tagLines(selectStart,selectEnd);

  for (z = selectStart; z < selectEnd; z++) {
    textLine = contents.at(z);
    textLine->selectEol(true,0);
  }
  textLine = contents.at(z);
  textLine->select(true,0,textLine->length());
}

void KWriteDoc::deselectAll() {
  int z;
  TextLine *textLine;

  select.x = -1;
  if (selectEnd < selectStart) return;

  unmarkFound();
  tagLines(selectStart,selectEnd);

  for (z = selectStart; z <= selectEnd; z++) {
    textLine = contents.at(z);
    textLine->selectEol(false,0);
  }
  selectEnd = -1;
}



void KWriteDoc::invertSelection() {
  int z;
  TextLine *textLine;

  select.x = -1;

  unmarkFound();
  selectStart = 0;
  selectEnd = contents.count() -1;

  tagLines(selectStart,selectEnd);

  for (z = selectStart; z < selectEnd; z++) {
    textLine = contents.at(z);
    textLine->toggleSelectEol(0);
  }
  textLine = contents.at(z);
  textLine->toggleSelect(0,textLine->length());
}


QString KWriteDoc::markedText(int flags) {
  TextLine *textLine;
  int len, z, start, end, i;

  len = 0;

  if (!(flags & cfVerticalSelect)) {
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = contents.at(z);
      len += textLine->numSelected();
      if (textLine->isSelected()) len++;
    }
    QString s(len + 1);
    len = 0;
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = contents.at(z);
      end = 0;
      do {
        start = textLine->findSelected(false,end);
        end = textLine->findSelected(true,start);
        for (i = start; i < end; i++) {
          s[len] = textLine->getChar(i);
          len++;
        }
      } while (start < end);
      if (textLine->isSelected()) {
        s[len] = '\n';
        len++;
      }
    }
    s[len] = '\0';
    return s;
  } else {
    while (selectStart <= selectEnd) {
      textLine = contents.at(selectStart);
      if (textLine->isSelected() || textLine->numSelected() > 0) break;
      selectStart++;
    }
    while (selectEnd >= selectStart) {
      textLine = contents.at(selectEnd);
      if (textLine->numSelected() > 0) break;
      selectEnd--;
    }
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = contents.at(z);
      len += textLine->numSelected() + 1;
    }
    QString s(len + 1);
    len = 0;
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = contents.at(z);
      end = 0;
      do {
        start = textLine->findSelected(false,end);
        end = textLine->findSelected(true,start);
        for (i = start; i < end; i++) {
          s[len] = textLine->getChar(i);
          len++;
        }
      } while (start < end);
      s[len] = '\n';
      len++;
    }
    s[len] = '\0';
    return s;
  }
}

void KWriteDoc::delMarkedText(int flags) {
  int z;
  TextLine *textLine, *nextLine;
  PointStruc start, end;

  if (selectEnd < selectStart) return;

  for (z = selectEnd; z >= selectStart; z--) {
    textLine = contents.at(z);
    start.y = z;
    end.y = z;

    end.x = 0;
    do {
      end.x = textLine->findSelected(false,end.x);
      start.x = textLine->findSelected(true,end.x);
      if (start.x == end.x) break;
      updateCursors(start,end);
      textLine->del(end.x,start.x - end.x);
    } while (true);

    if (textLine->isSelected()) {
      start.x = 0;
      start.y++;
      updateCursors(start,end);
      nextLine = contents.at(z+1);
      textLine->append(nextLine);
      contents.remove(z+1);
      selectEnd--;
    }

  }
//  tagLines(selectStart,selectEnd);
  updateLines(flags,selectStart,selectEnd);

  selectEnd = -1;
  select.x = -1;
}

QColor &KWriteDoc::cursorCol(int x, int y) {
  TextLine *textLine;
  int attr;
  Attribute *a;

//  if (x > 0) x--;
  textLine = contents.at(y);
  attr = textLine->getRawAttr(x);
  a = attribs[attr & taAttrMask];
  if (attr & taSelectMask) return a->selCol; else return a->col;
}

/*
void KWriteDoc::paintTextLine(QPainter &paint, int line,
                              int xPos, int xStart, int xEnd, int yPos) {
  int y;
  TextLine *textLine;
  int z, x;
  char ch;
  Attribute *a = 0;
  int attr, nextAttr;
  int xs;
  int xc, zc;

  y = line*fontHeight - yPos;
  if (line >= (int) contents.count()) {
    paint.fillRect(xStart - xPos,y,xEnd - xStart,fontHeight,selCols[0]);
    return;
  }
//printf("xStart = %d, xEnd = %d, line = %d\n",xStart,xEnd,line);
//printf("text = ");
  textLine = contents.at(line);

  z = 0;
  x = 0;
  do {
    xc = x;
    ch = textLine->getChar(z);
    if (ch == '\t') {
      x += tabWidth - (x % tabWidth);
    } else {
      a = attribs[textLine->getAttr(z)];
      x += a->fm.width(&ch,1);
    }
    z++;
  } while (x <= xStart);
  zc = z - 1;

  xs = xStart;
  attr = textLine->getRawAttr(zc);
  while (x < xEnd) {
    nextAttr = textLine->getRawAttr(z);
    if ((nextAttr ^ attr) & taSelectMask) {
      paint.fillRect(xs - xPos,y,x - xs,fontHeight,selCols[attr >> taShift]);
      xs = x;
      attr = nextAttr;
    }
    ch = textLine->getChar(z);
    if (ch == '\t') {
      x += tabWidth - (x % tabWidth);
    } else {
      a = attribs[attr & taAttrMask];
      x += a->fm.width(&ch,1);
    }
    z++;
  }
  paint.fillRect(xs - xPos,y,xEnd - xs,fontHeight,selCols[attr >> taShift]);
//int len = textLine->length();
  y += fontAscent -1;
  attr = -1;
  while (xc < xEnd) {
    ch = textLine->getChar(zc);
    if (ch == '\t') {
      xc += tabWidth - (xc % tabWidth);
    } else {
      nextAttr = textLine->getRawAttr(zc);
      if (nextAttr != attr) {
        attr = nextAttr;
        a = attribs[attr & taAttrMask];
        if (attr & taSelectMask) paint.setPen(a->selCol); else paint.setPen(a->col);
        paint.setFont(a->font);
      }
      paint.drawText(xc - xPos,y,&ch,1);
      xc += a->fm.width(&ch,1);
//if (zc < len) printf("%c",ch);
    }
    zc++;
  }

//printf("\n");
}
*/

void KWriteDoc::paintTextLine(QPainter &paint, int line,
                              int xStart, int xEnd) {
  int y;
  TextLine *textLine;
  int z, x;
  char ch;
  Attribute *a = 0;
  int attr, nextAttr;
  int xs;
  int xc, zc;

  y = 0;//line*fontHeight - yPos;
  if (line >= (int) contents.count()) {
    paint.fillRect(0,y,xEnd - xStart,fontHeight,selCols[0]);
    return;
  }
//printf("xStart = %d, xEnd = %d, line = %d\n",xStart,xEnd,line);
//printf("text = ");
  textLine = contents.at(line);

  z = 0;
  x = 0;
  do {
    xc = x;
    ch = textLine->getChar(z);
    if (ch == '\t') {
      x += tabWidth - (x % tabWidth);
    } else {
      a = attribs[textLine->getAttr(z)];
      x += a->fm.width(&ch,1);
    }
    z++;
  } while (x <= xStart);
  zc = z - 1;

  xs = xStart;
  attr = textLine->getRawAttr(zc);
  while (x < xEnd) {
    nextAttr = textLine->getRawAttr(z);
    if ((nextAttr ^ attr) & taSelectMask) {
      paint.fillRect(xs - xStart,y,x - xs,fontHeight,selCols[attr >> taShift]);       xs = x;
      attr = nextAttr;
    }
    ch = textLine->getChar(z);
    if (ch == '\t') {
      x += tabWidth - (x % tabWidth);
    } else {
      a = attribs[attr & taAttrMask];
      x += a->fm.width(&ch,1);
    }
    z++;
  }
  paint.fillRect(xs - xStart,y,xEnd - xs,fontHeight,selCols[attr >> taShift]);
//int len = textLine->length();
  y += fontAscent -1;
  attr = -1;
  while (xc < xEnd) {
    ch = textLine->getChar(zc);
    if (ch == '\t') {
      xc += tabWidth - (xc % tabWidth);
    } else {
      nextAttr = textLine->getRawAttr(zc);
      if (nextAttr != attr) {
        attr = nextAttr;
        a = attribs[attr & taAttrMask];
        if (attr & taSelectMask) paint.setPen(a->selCol); else paint.setPen(a->col);
        paint.setFont(a->font);
      }
      paint.drawText(xc - xStart,y,&ch,1);
      xc += a->fm.width(&ch,1);
//if (zc < len) printf("%c",ch);
    }
    zc++;
  }

//printf("\n");
}

void KWriteDoc::setModified(bool m) {
  KWriteView *view;

  if (m != modified) {
    modified = m;
    for (view = views.first(); view != 0L; view = views.next()) {
      emit view->kWrite->newStatus();
    }
  }
}

bool KWriteDoc::isModified() {
  return modified;
}

bool KWriteDoc::isLastView(int numViews) {
  return ((int) views.count() == numViews);
}

void KWriteDoc::setFileName(const char *s) {
  int z;

  fName = s;
  for (z = 0; z < (int) views.count(); z++) {
    emit views.at(z)->kWrite->newCaption();
  }
}

bool KWriteDoc::hasFileName() {
  return !fName.isEmpty();
}

const char *KWriteDoc::fileName() {
  return fName;
}

void downcase(char *s, int len) {
  while (len > 0) {
    if (*s >= 'A' && *s <= 'Z') *s += 32;
    s++;
    len--;
  }
}

int KWriteDoc::doSearch(const char *searchFor, int flags, PointStruc &cursor) {
  int line, col;
  int searchEnd;
  int slen, blen, tlen;
  char *s, *b, *t;
  TextLine *textLine;
  int pos, newPos;

  slen = strlen(searchFor);
  if (slen == 0) return 0;
  s = new char[slen];
  memcpy(s,searchFor,slen);
  if (!(flags & sfCaseSensitive)) downcase(s,slen);

  blen = -2;
  b = 0;
  t = 0;

  if (!(flags & sfBackward)) {
    //forward search
    if (flags & sfFromCursor) {
      line = cursor.y;
      col = cursor.x;
    } else {
      line = 0;
      col = 0;
    }

    if (flags & sfSelected) {
      if (line < selectStart) {
        line = selectStart;
        col = 0;
      }
      searchEnd = selectEnd;
    } else searchEnd = (int) contents.count() -1;

    while (line <= searchEnd) {
      textLine = contents.at(line);
      tlen = textLine->length();
      if (tlen > blen) {
        delete b;
        blen = (tlen + 257) & (~255);
        b = new char[blen];
        blen -= 2;
        b[0] = 0;
        t = &b[1];
      }
      memcpy(t,textLine->getText(),tlen);
      t[tlen] = 0;
      if (flags & sfSelected) {
        pos = 0;
        do {
          pos = textLine->findSelected(true,pos);
          newPos = textLine->findSelected(false,pos);
          memset(&t[pos],0,newPos - pos);
          pos = newPos;
        } while (pos < tlen);
      }
      if (!(flags & sfCaseSensitive)) downcase(t,tlen);

      tlen -= slen;
      if (flags & sfWholeWords) {
        while (col <= tlen) {
          if (!highlight->isInWord(t[col-1]) && !highlight->isInWord(t[col+slen])) {
            if (memcmp(&t[col],s,slen) == 0) goto found;
          }
          col++;
        }
      } else {
        while (col <= tlen) {
          if (memcmp(&t[col],s,slen) == 0) goto found;
          col++;
        }
      }
      col = 0;
      line++;
    }
  } else {
    // backward search
    if (flags & sfFromCursor) {
      line = cursor.y;
      col = cursor.x;
    } else {
      line = (int) contents.count() -1;
      col = -1;
    }

    if (flags & sfSelected) {
      if (line > selectEnd) {
        line = selectEnd;
        col = -1;
      }
      searchEnd = selectStart;
    } else searchEnd = 0;

    while (line >= searchEnd) {
      textLine = contents.at(line);
      tlen = textLine->length();
      if (tlen > blen) {
        delete b;
        blen = (tlen + 257) & (~255);
        b = new char[blen];
        blen -= 2;
        b[0] = 0;
        t = &b[1];
      }
      memcpy(t,textLine->getText(),tlen);
      t[tlen] = 0;
      if (flags & sfSelected) {
        pos = 0;
        do {
          pos = textLine->findSelected(true,pos);
          newPos = textLine->findSelected(false,pos);
          memset(&t[pos],0,newPos - pos);
          pos = newPos;
        } while (pos < tlen);
      }
      if (!(flags & sfCaseSensitive)) downcase(t,tlen);

      if (col < 0 || col > tlen) col = tlen;
      col -= slen;
      if (flags & sfWholeWords) {
        while (col >= 0) {
          if (!highlight->isInWord(t[col-1]) && !highlight->isInWord(t[col+slen])) {
            if (memcmp(&t[col],s,slen) == 0) goto found;
          }
          col--;
        }
      } else {
        while (col >= 0) {
          if (memcmp(&t[col],s,slen) == 0) goto found;
          col--;
        }
      }
      line--;
    }
  }
  delete s;
  delete b;
  return 0;

found:
  delete s;
  delete b;

  unmarkFound();
  textLine->markFound(col,slen);

  if (!(flags & sfBackward)) col += slen;
  cursor.x = col;
  cursor.y = line;
  foundLine = line;
  tagLines(foundLine,foundLine);
  return 1;
}

void KWriteDoc::replace(PointStruc &cursor, int slen, const char *replaceWith, int flags) {
  TextLine *textLine;
  int pos, rlen;
  PointStruc start, end;

  unmarkFound();

  rlen = strlen(replaceWith);
  pos = cursor.x;
  if (!(flags & sfBackward)) {
    pos -= slen;
    cursor.x = pos + rlen;
  }

  textLine = contents.at(cursor.y);
  if (rlen > slen) {
    textLine->ins(pos + slen - 1,' ',rlen - slen);
  } else {
    textLine->del(pos + rlen,slen - rlen);
  }
  textLine->overwrite(pos,replaceWith);

  start.y = end.y = cursor.y;
  start.x = pos + slen;
  end.x = pos + rlen;
  updateCursors(start,end);

//  tagLines(cursor.y,cursor.y);
//  highlight->doHighlight(*this,cursor.y,cursor.y);
//  setModified(true);

  updateLines(flags,cursor.y,cursor.y);
}

void KWriteDoc::unmarkFound() {
  if (foundLine != -1) {
    contents.at(foundLine)->unmarkFound();
    tagLines(foundLine,foundLine);
    foundLine = -1;
  }
}

