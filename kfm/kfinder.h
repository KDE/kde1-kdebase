#ifndef _kfinder_h
#define _kfinder_h

#include <qpushbt.h>
#include <qtablevw.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qlist.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <drag.h>

#define CELL_HEIGHT 24
#define PIXMAP_HEIGHT 16
#define PIXMAP_WIDTH 16

class KFinder;
class KFinderButton;
class KFinderWin;
class KFinderItem;
class KFinderNode;

class KFinderButton : public QPushButton
{
    Q_OBJECT
public:
    KFinderButton( const char *_text, KFinder *_finder );

signals:
    void pressed( KFinderButton* );
    
public slots:
    void slotPressed();
};

class KFinderItem : public QObject
{
    friend KFinderWin;
    friend KFinderNode;
    
    Q_OBJECT
public:
    ~KFinderItem();

    virtual KFinderNode* node() { return finderNode; }

    virtual bool isOpen() { return bOpened; }
    virtual void setOpen( bool _open ) = 0;
    
protected:
    KFinderItem( KFinder *_finder );

    virtual void setLevel( int _level ) { level = _level; }
    virtual void fillItemList( QList<KFinderItem>& _list, int _level );

    virtual void paintCell( QPainter *_painter, int _col ) = 0;
    virtual void mousePressEvent( QMouseEvent *, const QPoint &_globalPoint );

    virtual void pressed( QMouseEvent *, const QPoint &_globalPoint ) = 0;
    virtual void dropEvent( QStrList &, const QPoint & ) { }
    
    KFinder *finder;
    KFinderNode *finderNode;
    bool bOpened;
    int level;
};

class KFinderNode
{
public:
    KFinderNode();

    void append( KFinderItem *_item ) { itemList.append( _item ); }
    void clear() { itemList.clear(); }
    void fillItemList( QList<KFinderItem>& _list, int _level );

    KFinderItem* first() { return itemList.first(); }
    KFinderItem* next() { return itemList.next(); }
    
    /**
     * By default the auto delete option is turned on.
     */
    void setAutoDelete( bool _d );
    bool isAutoDelete() { return bAutoDelete; }
    
protected:
    QList<KFinderItem> itemList;
    bool bAutoDelete;
};

class KFinderWin : public QTableView
{
    friend KFinder;
    
    Q_OBJECT
public:
    KFinderWin( KFinder *_finder );
    ~KFinderWin();

    void offsets( int &_xpos, int &_ypos );
    void setOffsets( int _xpos, int _ypos );
    
    KFinderItem* first() { return itemList.first(); }
    KFinderItem* next() { return itemList.next(); }

    void changeTree( KFinderNode *_node );
    void updateTree( bool _repaint = TRUE );

public slots:
    void slotDropEvent( KDNDDropZone *_zone );
    
protected:
    void paintCell( QPainter *, int, int );
    int cellWidth( int _column );

    void mousePressEvent( QMouseEvent *_ev );
    
    QColor  textColor;
    QColor  bgColor;
    KFinder *finder;
    KFinderNode *node;
    QList<KFinderItem> itemList;

    KDNDDropZone *dropZone;
};

class KFinder : public QWidget
{
    Q_OBJECT

    friend KFinderWin;
public:
    KFinder( QWidget *_parent = 0L, QStrList *_headers = 0L, QArray<int> *_sizes = 0L );
    ~KFinder();

    virtual void changeTree( KFinderNode *node );
    virtual void updateTree( bool _repaint = TRUE );

    virtual KFinderItem* first();
    virtual KFinderItem* next();

    static QPixmap& getOpenPixmap();
    static QPixmap& getClosePixmap();

    virtual void itemList( QList<KFinderItem> &_list );

    QColor getTextColor();
    void setColors(QColor bgcolor,QColor textcolor);
    void initColors();

signals:
    void buttonPressed( int );
    void drop( QStrList &_urls, const QPoint &_point );
    
public slots:
    void slotButtonPressed( KFinderButton *_but );
    void slotScrolled( int );
    
protected:
    virtual void resizeEvent( QResizeEvent *_ev );

    virtual void emitDrop( QStrList &_urls, const QPoint & _point );
    
    QList<KFinderButton> buttonList;
    unsigned int *sizeList;
    unsigned int *currentSizeList;
    KFinderWin *finderWin;
    int sort;
    int xOffset;
    QColor textColor;    
    QColor bgColor;

    static QPixmap* openPixmap;
    static QPixmap* closePixmap;
};

#endif
