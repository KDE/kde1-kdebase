#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include <qlist.h>
#include <qlistbox.h>

#include <kcolorbtn.h>

#include "kwdoc.h"

bool testWw(char c); //whole word check: false for 'A'-'Z','a'-'z','0'-'9','_'

class HlItem {
  public:
    HlItem(int attribute, int context);
    virtual bool startEnable(char) {return true;};
    virtual bool endEnable(char) {return true;};
    virtual const char *checkHgl(const char *) = 0;
    int attr;
    int ctx;
};

class HlItemWw : public HlItem {
  public:
    HlItemWw(int attribute, int context);
    virtual bool startEnable(char c) {return testWw(c);};
    virtual bool endEnable(char c) {return testWw(c);};
};


class HlCharDetect : public HlItem {
  public:
    HlCharDetect(int attribute, int context, char);
    virtual const char *checkHgl(const char *);
  protected:
    char sChar;
};

class Hl2CharDetect : public HlItem {
  public:
    Hl2CharDetect(int attribute, int context, const char *);
    virtual const char *checkHgl(const char *);
  protected:
    char sChar[2];
};

class HlStringDetect : public HlItem {
  public:
    HlStringDetect(int attribute, int context, const char *);
    virtual ~HlStringDetect();
    virtual const char *checkHgl(const char *);
  protected:
    char *str;
    int len;
};

class HlRangeDetect : public HlItem {
  public:
    HlRangeDetect(int attribute, int context, const char *);
    virtual const char *checkHgl(const char *);
  protected:
    char sChar[2];
};


class KeywordData {
  public:
    KeywordData(const char *);
    ~KeywordData();
    char *s;
    int len;
};

class HlKeyword : public HlItemWw {
  public:
    HlKeyword(int attribute, int context);
    virtual ~HlKeyword();
    void addWord(const char *);
    void addList(char **);
    virtual const char *checkHgl(const char *);
  protected:
    QList<KeywordData> words;
};

class HlInt : public HlItemWw {
  public:
    HlInt(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlFloat : public HlItemWw {
  public:
    HlFloat(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCInt : public HlInt {
  public:
    HlCInt(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCOct : public HlItemWw {
  public:
    HlCOct(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCHex : public HlItemWw {
  public:
    HlCHex(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCFloat : public HlFloat {
  public:
    HlCFloat(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlLineContinue : public HlItem {
  public:
    HlLineContinue(int attribute, int context);
    virtual bool endEnable(char c) {return c == '\0';};
    virtual const char *checkHgl(const char *);
};

class HlCStringChar : public HlItem {
  public:
    HlCStringChar(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCChar : public HlItemWw {
  public:
    HlCChar(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCPrep : public HlItem {
  public:
    HlCPrep(int attribute, int context);
    virtual bool startEnable(char c) {return c == '\0';};
    virtual const char *checkHgl(const char *);
};

class HlHtmlTag : public HlItem {
  public:
    HlHtmlTag(int attribute, int context);
    virtual bool startEnable(char c) {return c == '<';};
    virtual const char *checkHgl(const char *);
};

class HlHtmlValue : public HlItem {
  public:
    HlHtmlValue(int attribute, int context);
    virtual bool startEnable(char c) {return c == '=';};
    virtual const char *checkHgl(const char *);
};

class HlShellComment : public HlCharDetect {
  public:
    HlShellComment(int attribute, int context);
    virtual bool startEnable(char c) {return testWw(c);};
};

//modula 2 hex
class HlMHex : public HlItemWw {
  public:
    HlMHex(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

//context
class HlContext {
  public:
    HlContext(int attribute, int lineEndContext);
    QList<HlItem> items;
    int attr;
    int ctx;
};

class Highlight {
  public:
    Highlight(const char *hName);
    virtual ~Highlight();
    virtual void init();
    virtual void getItemList(QStrList &);
    virtual bool isInWord(char);
    virtual void doHighlight(int ctxNum, TextLine *);
    virtual void readConfig();
    virtual void writeConfig();

  Attribute **attrList();
  
  static QList<Attribute> attList;
  static QList<HlContext> ctxList;
  
  protected:
    virtual void makeContextList() {};
    HlContext *contextList[nAttribs];
    QString name;
  
    Attribute *attribs[nAttribs];
  
};


class NoHighlight : public Highlight {
  public:
    NoHighlight(const char *hName);
    virtual ~NoHighlight();
    virtual void doHighlight(int ctxNum, TextLine *);
  protected:
    virtual void makeContextList();
};

class CHighlight : public Highlight {
  public:
    CHighlight(const char *hName);
    virtual ~CHighlight();
  protected:
    virtual void makeContextList();
    virtual void setKeywords(HlKeyword *);
};

class CppHighlight : public CHighlight {
  public:
    CppHighlight(const char *hName);
    virtual ~CppHighlight();
  protected:
    virtual void setKeywords(HlKeyword *);
};

class HtmlHighlight : public Highlight {
  public:
    HtmlHighlight(const char *hName);
    virtual ~HtmlHighlight();
  protected:
    virtual void makeContextList();
};

class BashHighlight : public Highlight {
  public:
    BashHighlight(const char *hName);
    virtual ~BashHighlight();
  protected:
    virtual void makeContextList();
};

class ModulaHighlight : public Highlight {
  public:
    ModulaHighlight(const char *hName);
    virtual ~ModulaHighlight();
  protected:
    virtual void makeContextList();
};


#endif //HIGHLIGHT_H
