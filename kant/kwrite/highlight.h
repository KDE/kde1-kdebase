#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include <qlist.h>

#include <kcolorbtn.h>

#include "kwdoc.h"



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
    virtual bool startEnable(char);
    virtual bool endEnable(char c) {return startEnable(c);};
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

class HlCStringCont : public HlItem {
  public:
    HlCStringCont(int attribute, int context);
    virtual bool endEnable(char);
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
    virtual bool startEnable(char);
    virtual const char *checkHgl(const char *);
};

class HlHtmlChar : public HlItem {
  public:
    HlHtmlChar(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlHtmlTag : public HlItem {
  public:
    HlHtmlTag(int attribute, int context);
    virtual bool startEnable(char);
    virtual const char *checkHgl(const char *);
};

class HlHtmlValue : public HlItem {
  public:
    HlHtmlValue(int attribute, int context);
    virtual bool startEnable(char);
    virtual const char *checkHgl(const char *);
};

class HlShellComment : public HlItemWw {
  public:
    HlShellComment(int attribute, int context);
    virtual bool endEnable(char) {return true;};
    virtual const char *checkHgl(const char *);
};

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
    Attribute **attrList();
    virtual void getItemList(QStrList &);
    virtual bool isInWord(char);
    virtual void doHighlight(KWriteDoc &, int startLine, int endLine);
    virtual void readConfig();
    virtual void writeConfig();
  protected:
    virtual void makeDefAttribs() = 0;
    virtual void makeContextList() = 0;
    Attribute *attribs[nAttribs];
    HlContext *contextList[nAttribs];
    QString name;
};


class NoHighlight : public Highlight {
  public:
    NoHighlight(const char *hName);
    virtual ~NoHighlight();
    virtual void doHighlight(KWriteDoc &, int startLine, int endLine);
  protected:
    virtual void makeDefAttribs();
    virtual void makeContextList();
};

class CHighlight : public Highlight {
  public:
    CHighlight(const char *hName);
    virtual ~CHighlight();
  protected:
    virtual void makeDefAttribs();
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
    virtual void makeDefAttribs();
    virtual void makeContextList();
};

class BashHighlight : public Highlight {
  public:
    BashHighlight(const char *hName);
    virtual ~BashHighlight();
  protected:
    virtual void makeDefAttribs();
    virtual void makeContextList();
};


class HighlightDialog : public QDialog {
    Q_OBJECT
  public:
    HighlightDialog(QStrList &types, QWidget *parent, const char *newHlSlot);

    void newHl(Highlight *);

    static Highlight *getHighlight(QStrList &types,
      QWidget *parent, const char *newHlSlot);
  signals:
    void newHl(int index);
  protected slots:
    void newItem(int);
    void newCol(const QColor &);
    void newSelCol(const QColor &);
    void newFont();

  protected:
    Highlight *highlight;
    Attribute *a;
    QListBox *typeLB;
    QListBox *itemLB;
    KColorButton *col;
    KColorButton *selCol;
};

#endif //HIGHLIGHT_H
