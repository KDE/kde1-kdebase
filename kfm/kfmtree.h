#ifndef kfmtree_h
#define kfmtree_h

class KFMDirTree;
class KFMDirTreeItem;
class KfmGui;

#include <qpopmenu.h>

#include "kfinder.h"
#include "kstrlist.h"

class KFMDirTree : public KFinder
{
    friend KFMDirTreeItem;
    
    Q_OBJECT
public:
    KFMDirTree( QWidget *_parent, KfmGui *_gui );

    void fill();
    void update();

    KfmGui* getGui() { return gui; }


signals:
    void urlSelected( const char *_url, int _button );

public slots:
    void slotPopupNewView();
    void slotPopupCd();
    void slotPopupProperties();
    void slotPopupBookmarks();
    void slotPopupEmptyTrashBin();
    void slotPopupCopy();
    void slotPopupPaste();
    void slotPopupTrash();
    void slotPopupDelete();
    
    void slotDropCopy();
    void slotDropMove();
    void slotDropLink();

    void slotDirectoryChanged( const char *_url );
    
protected:
    void emitUrlSelected( const char *_url, int _button );

    void openPopupMenu( const char *_url, const QPoint &_point );
    void openDropMenu( const char *_dest, QStrList &_urls, const QPoint &_p );



    QString popupDir;
    QString dropDestination;
    KStrList dropSource;
    KFinderNode node;
    QPopupMenu *popupMenu;
    KfmGui *gui;
};

class KFMDirTreeItem : public KFinderItem
{
    Q_OBJECT
public:
    KFMDirTreeItem( KFMDirTree* _finder, const char *_url, bool _isfile = FALSE );
    ~KFMDirTreeItem();
    
    virtual void paintCell( QPainter *_painter, int _col );

    virtual void setOpen( bool _open );

    const char* getURL() { return url; }
    
protected:
    virtual void pressed( QMouseEvent *_ev, const QPoint &_globalPoint );
    virtual void dropEvent( QStrList &urls, const QPoint &_point );

    /**
     * Dont delete the pixmap in the destructor. It is cached in
     * the @ref KMimeType::pixmapCache.
     */
    QPixmap *pixmap;
    QString name;
    QString url;
    bool bFilled;
    bool bIsFile;
    KFMDirTree *dirTree;
};

#endif
