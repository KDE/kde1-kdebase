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
    void ins(int pos, char, int n = 1);
//    void ins(int pos, const char *, int n);
    void overwrite(int pos, const char *, int n = -1);
    void append(char, int n = 1);
    void del(int pos,int l = 1);
    int length();
    void setLength(int l);
    void truncate(int l);

    void append(TextLine *, int pos = 0);
    void copy(TextLine *);
    void insEnd(TextLine *, int pos);

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
    int findSelected(bool sel, int pos);

    int cursorX(int pos, int tabChars);

//    int find(const char *searchFor, int pos);
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

typedef QList<TextLine> TextContents;

class KWriteDoc : QObject {
    Q_OBJECT
    friend KWriteView;
    friend KWrite;
  public:
    KWriteDoc();
    ~KWriteDoc();

    int numLines() const;
    TextLine *textLine(int line);
    void tagLines(int start, int end);
//    void tagAll();
    void readSessionConfig(KConfig *);
    void writeSessionConfig(KConfig *);
  protected:
    void registerView(KWriteView *);
    void removeView(KWriteView *);

    int currentColumn(PointStruc &cursor);

    void insert(VConfig &, const char *);
    void insertFile(VConfig &, QIODevice &);
    void writeFile(QIODevice &);

    void insertChar(VConfig &, char);
    void newLine(VConfig &);
    void killLine(VConfig &);
    void backspace(VConfig &);
    void del(VConfig &);


  protected slots:
    void clipboardChanged();

  protected:
    void updateFontData();
    void setHighlight(Highlight *);
    void setTabWidth(int);
    void update(VConfig &);
    void updateLines(int flags, int startLine, int endLine);
    void updateMaxLength(TextLine *);
    void updateCursors(PointStruc &start, PointStruc &end, bool insert = true);
    void updateViews(int flags = 0);

    int textLength(int line);
    int textWidth(TextLine *, int cursorX);
    int textWidth(PointStruc &cursor);
    int textWidth(bool wrapCursor, PointStruc &cursor, int xPos);
    int textPos(TextLine *, int xPos);

    int textWidth();
    int textHeight();

 void toggleRect(int, int, int, int);
 void setLine(int, int);
    void selectTo(PointStruc &start, PointStruc &end, int flags);
    void clear();
    void copy(int flags);
    void paste(VConfig &);
    void cut(int flags);
    void selectAll();
    void deselectAll();
    void invertSelection();

    QString markedText(int flags);
    void delMarkedText(int flags);

    QColor &cursorCol(int x, int y);
//    void paintTextLine(QPainter &, int line, int xPos, int xStart, int xEnd, int yPos);
    void paintTextLine(QPainter &, int line, int xStart, int xEnd);

    void setModified(bool);
    bool isModified();
    bool isLastView(int numViews);

    void setFileName(const char *);
    bool hasFileName();
    const char *fileName();

    int doSearch(const char *searchFor, int flags, PointStruc &cursor);
    void replace(PointStruc &cursor, int slen, const char *replaceWith, int flags);
    void unmarkFound();


    TextContents contents;
    Attribute **attribs;//[nAttribs];
    int fontHeight;
    int fontAscent;
    QColor selCols[4];

    QList<KWriteView> views;

    int tabChars;
    int tabWidth;

    TextLine *bufferLine;

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
};

#endif //KWDOC_H
