#ifndef KWDOC_H
#define KWDOC_H

#include <qobject.h>
#include <qlist.h>
#include <qcolor.h>
#include <qfont.h>
#include <qfontmet.h>

#include "kwview.h"

class Highlight;

class TextLine {
  public:
    TextLine(int attribute = 0);
    ~TextLine();
    void insert(int pos, const char *, int l);
    void overwrite(int pos, const char *, int l);
    void append(char, int n = 1);
    void del(int pos,int l = 1);
    int length();
    void setLength(int l);

    void wrap(TextLine *nextLine, int pos);
    void unWrap(TextLine *nextLine, int pos);

    void removeSpaces();
    int firstChar();

    char getChar(int pos) const;

    void setAttribs(int attribute, int start, int end);
    void setAttr(int attribute);
    int getAttr(int pos);
    int getAttr();
    int getRawAttr(int pos);
    int getRawAttr();

    void setContext(int context);
    int getContext();

    const char *getString();
    const char *getText();

    void select(bool sel, int start, int end);
    void selectEol(bool sel, int pos);
    void toggleSelect(int start, int end);
    void toggleSelectEol(int pos);
    int numSelected();
    bool isSelected(int pos);
    bool isSelected();

    int findSelected(int pos);
    int findUnSelected(int pos);
    int findRevSelected(int pos);
    int findRevUnSelected(int pos);

    int cursorX(int pos, int tabChars);

    void markFound(int pos, int l);
    void unmarkFound();

    void move(int pos, int offs);
  protected:
    void resize(int);

    int len;
    int size;
    char *text;
    unsigned char *attribs;
    unsigned char attr;
    unsigned char ctx;
};



const nAttribs = 32;

class Attribute {
  public:
    Attribute();
    Attribute(const char *aName, const QColor &, const QColor &, const QFont &);
    QString name;
    QColor col;
    QColor selCol;
    void setFont(const QFont &);
    QFont font;
    QFontMetrics fm;
};

class KWAction {
  public:
    enum Action {replace, wordWrap, wordUnWrap, newLine, delLine,
      insLine, killLine};//, doubleLine, removeLine};

    KWAction(Action, PointStruc &aCursor);
    ~KWAction();
    void setData(int aLen, const char *aText, int aTextLen);
    Action action;
    PointStruc cursor;
    int len;
    const char *text;
    int textLen;
    KWAction *next;
};

class KWActionGroup {
  public:
    KWActionGroup(PointStruc &aStart);
    ~KWActionGroup();
    void insertAction(KWAction *);

    PointStruc start;
    PointStruc end;
    KWAction *action;
};

class KWriteDoc : QObject {
    Q_OBJECT
    friend KWriteView;
    friend KWrite;
  public:
    KWriteDoc();
    ~KWriteDoc();

    int lastLine() const;
    TextLine *textLine(int line);
    void tagLines(int start, int end);
    void tagAll();
    void readSessionConfig(KConfig *);
    void writeSessionConfig(KConfig *);
  protected:
    void registerView(KWriteView *);
    void removeView(KWriteView *);

    int currentColumn(PointStruc &cursor);

    void insert(KWriteView *, VConfig &, const char *);
    void insertFile(KWriteView *, VConfig &, QIODevice &);
    void loadFile(QIODevice &);
    void writeFile(QIODevice &);

    void insertChar(KWriteView *, VConfig &, char);
    void newLine(KWriteView *, VConfig &);
    void killLine(KWriteView *, VConfig &);
    void backspace(KWriteView *, VConfig &);
    void del(KWriteView *, VConfig &);


  protected slots:
    void clipboardChanged();

  protected:
    void updateFontData();
    void setHighlight(Highlight *);
    void setTabWidth(int);
//    void update(VConfig &);
    void updateLines(int flags, int startLine, int endLine);
    void updateMaxLength(TextLine *);
//    void updateCursors(PointStruc &start, PointStruc &end, bool insert = true);
    void updateViews(int flags = 0);

    int textLength(int line);
    int textWidth(TextLine *, int cursorX);
    int textWidth(PointStruc &cursor);
    int textWidth(bool wrapCursor, PointStruc &cursor, int xPos);
    int textPos(TextLine *, int xPos);

    int textWidth();
    int textHeight();

    void toggleRect(int, int, int, int);
    void selectTo(PointStruc &start, PointStruc &end, int flags);
    void clear();
    void copy(int flags);
    void paste(KWriteView *,VConfig &);
    void cut(KWriteView *, VConfig &);
    void selectAll();
    void deselectAll();
    void invertSelection();

    QString markedText(int flags);
    void delMarkedText(KWriteView *, VConfig &);

    QColor &cursorCol(int x, int y);
//    void paintTextLine(QPainter &, int line, int xPos, int xStart, int xEnd, int yPos);
    void paintTextLine(QPainter &, int line, int xStart, int xEnd);

    void setModified(bool);
//    bool isModified();
    bool isLastView(int numViews);

    void setFileName(const char *);
    bool hasFileName();
    const char *fileName();

    bool doSearch(PointStruc &cursor, const char *searchFor, int flags);
    void unmarkFound();
    void markFound(PointStruc &cursor, int len);

    void tagLine(int line);
    void insLine(int line);
    void delLine(int line);
    void optimizeSelection();
    
    void doAction(KWAction *);
    const char *doReplace(KWAction *);
    void doWordWrap(KWAction *);
    void doWordUnWrap(KWAction *);
    void doNewLine(KWAction *);
    void doDelLine(KWAction *);
    void doInsLine(KWAction *);
    void doKillLine(KWAction *);
    void newUndo();
    void recordStart(PointStruc &);
    void recordAction(KWAction::Action, PointStruc &);
    void recordReplace(PointStruc &, int len, const
      char *text = 0L, int textLen = 0);
    void recordEnd(KWriteView *, VConfig &);
    void doActionGroup(KWActionGroup *, int flags);
    void undo(KWriteView *, int flags);
    void redo(KWriteView *, int flags);

    QList<TextLine> contents;
    Attribute **attribs;//[nAttribs];
    int fontHeight;
    int fontAscent;
    QColor selCols[4];

    QList<KWriteView> views;

    int tabChars;
    int tabWidth;

//    TextLine *bufferLine;

    TextLine *longestLine;
    int maxLength;

    PointStruc select;
    PointStruc anchor;
    int selectStart;
    int selectEnd;

    bool modified;
    QString fName;

    int foundLine;

    Highlight *highlight;

    QList<KWActionGroup> undoList;
    int currentUndo;
    int undoState;
    int tagStart;
    int tagEnd;
};

#endif //KWDOC_H
