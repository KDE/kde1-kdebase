#ifndef kfmtree_h
#define kfmtree_h

#undef index

#include <qapp.h>
#include <qdir.h>
#include <qlistbox.h>
#include <qpushbt.h>
#include <ktreelist.h>

class KFMTree : public KTreeList
{
    Q_OBJECT
public:
    KFMTree( QWidget *_parent );
    ~KFMTree();

signals:
    void showPopup( int _index, QPoint &_point );
    
protected:
    virtual void mousePressEvent( QMouseEvent *_e );

};

class KFMTreeView : public QWidget
{
    Q_OBJECT
public:
    KFMTreeView( QWidget *parent = 0 );
    ~KFMTreeView();

    void initializeTree();

signals:
    void showDir( const char *_dir );
    void popupMenu( const char *_url, const QPoint &_point );
    
public slots:
    void slotDirExpanded( int );
    void slotDirHighlighted( int );
    void slotDirSelected( int );
    void slotShowPopup( int, QPoint &_point );

protected:
    virtual void resizeEvent(QResizeEvent *e);

    void advanceReadDirectories( int treeIndex );

    KFMTree *dirTree;
    QPixmap *openPixmap;
    QPixmap *closedPixmap;
    KPath oldPath;
};

#endif
