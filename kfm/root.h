#ifndef ROOT_H
#define ROOT_H

#include <qwidget.h>
#include <qbitmap.h>
#include <qpopmenu.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/extensions/shape.h>

#include "kbind.h"
#include "html.h"
#include "kfmwin.h"
#include "kioserver.h"
#include "kstrlist.h"
#include "kiojob.h"

class KRootWidget;

/// IO job for root drops
/**
  This special job handles notify signals different. If a notify
  signal affects the desktop, the KRootWidget is not informed the
  usual way. Instead the function 'dropUpdateIcons' is called with
  the coordinates of the drop.
  */
class KIORootJob : public KIOJob
{
    Q_OBJECT
public:
    /// Creates a new IO job
    /**
      '_x' and '_y' are the coordinates of the drop.
      */
    KIORootJob( KRootWidget * _root, int _x, int _y );
    ~KIORootJob() {};
 
public slots:
    /// Gets notify events
    /**
      All drop IO has turned global notifies off. This slot gets all notifies
      and must decide what to do with them.
      */
    void slotDropNotify( int _id, const char *_url );

protected:
    int dropFileX;
    int dropFileY;
    KRootWidget *rootWidget;
};

/// A 'KFileManager' for the desktop
/**
  This one differs from KFileManager in the way, a popup menu
  is handled.
  */
class KRootManager : public KFileManager
{
    Q_OBJECT
public:
    KRootManager( KRootWidget * );
    virtual ~KRootManager() { }
   
    virtual void openPopupMenu( QStrList & _urls, const QPoint &_point );
    
protected:
    KRootWidget *rootWidget;
};

class KRootLayout
{
protected:
    int x,y;
    QString url;
    int dndType;

public:
    KRootLayout( const char *_url, int _t, int _x, int _y ) { url = _url; x = _x; y = _y; dndType = _t; }

    int getX() { return x; }
    int getY() { return y; }
    const char* getURL() { return url.data(); }
};

class KRootIcon : public KDNDWidget
{
    Q_OBJECT
public:
    KRootIcon( const char * _pixmap_file, QString & _url, int _x, int _y );
    virtual ~KRootIcon();

    /// Updates the pixmap and the tool tip ( if required )
    /**
      This function is usually called from KRootWidget::update().
      */
    virtual void updatePixmap();
    virtual void init();
    virtual void initToolTip();

    const char* getURL() { return url.data(); }
    int getType() { return dndType; }

    /// Returns the X-Position for saving in the file ~./desktop
    virtual int saveX() { return x() + pixmapXOffset; }
    /// Returns the Y-Position for saving in the file ~./desktop
    virtual int saveY() { return y(); }

    /// Returns the coordinate on which the dragginh of this icon started
    virtual QPoint& getDndStartPos() { return dndStartPos; }

    /// Call to rename an icon
    /**
      _new_name is not a complete URL. It is only the filename.
      */
    virtual void rename( const char *_new_name );
    
public slots:
    /// Called when the user drops something over the icon
    void slotDropEvent( KDNDDropZone *_zone );
    
protected:
    virtual void resizeEvent( QResizeEvent * );
    virtual void paintEvent( QPaintEvent *_event );
    virtual void mouseDoubleClickEvent( QMouseEvent *_mouse );
    virtual void mousePressEvent( QMouseEvent *_mouse );
    virtual void dndMouseMoveEvent( QMouseEvent *_mouse );
    virtual void dndMouseReleaseEvent( QMouseEvent *_mouse );
    // virtual void rootDropEvent( int, int );
    virtual void dragEndEvent();

    /// This is the URL of the file represented by this icon.
    QString url;
    /// This is only the filename with no path in front of it.
    QString file;
    QPixmap pixmap;
    /// The filename of the currently displayed pixmap.
    QString pixmapFile;
    QBitmap mask;

    int pixmapXOffset;
    int pixmapYOffset;
    int textYOffset;
    int textXOffset;

    bool pressed;
    int press_x, press_y;

    /// The start of a drag in global coordinates
    QPoint dndStartPos;
    
    int width;
    int height;

    /// The DropZone bound to this icon
    KDNDDropZone *dropZone;
};

class KRootWidget : public QWidget
{
    Q_OBJECT
public:
    KRootWidget( QWidget *parent=0, const char *name=0 );
    virtual ~KRootWidget() { }

    /// Takes all icons corresponding to the given URLs and moves them.
    /** 
      This function used 'icon->dndStartX/Y()' and 'p' to determine the amount of
      pixels the icons have to move.
     */
    void moveIcons( QStrList & _urls, QPoint &p );
      
    /******************************************************
     * Returns a list of selected icons or a null string if
     * no icon was currently selected.
     */
    QString getSelectedURLs();

    /******************************************************
     * Since there is only one root widget, this method can be used to
     * get its pointer. This is better than using global variables since this
     * would not be object oriented.
     */
    static KRootWidget* getKRootWidget() { return pKRootWidget; }

    /// Return pointer to KRootManager.
    /**
      Root icons use this manager to perform various actions like double
      clicks ans popup menus.
      */
    KRootManager *getManager() { return manager; }

    /// Set the URLs belonging to the opened popup menu
    /**
      This function is used by KRootManager when a popup file is opened since
      some of the menu items are connected to slots in the root widget.
      */
    void setPopupFiles( QStrList & _urls );

    /// Updates the icons after a drop
    /**
      After a drop, we assume that every new file in the desktop directory
      is a result of the drop. So we arrange all new icons around the position of the 
      drop. The x|y coordinates passed to this function are the point of the drop.
      The new icons will be placed there.
      */
    void dropUpdateIcons( int _x, int _y );

    /// The URL of the desktop.
    /**
      This string always ends with "/".
      */
    QString desktopDir;
    
public slots:
    /// Updates the icons
    /**
      This function is used to synchronize the filesystem with
      the icons. For every file without an icon, an icon will be created
      and for every icon without a file the icon will be deleted.
      */
    void update();

    void slotDropCopy();
    void slotDropMove();
    void slotDropLink();
    
    /******************************************************
     * If the 'rootDropZone' receives a drop action, this slot is signaled.
     */
    void slotDropEvent( KDNDDropZone *_zone );

    /// Open a properties dialog
    /**
      The URLs belonging to the popup menu is stored in 'popupFiles'.
      */
    void slotPopupProperties();
    void slotPopupCopy();
    void slotPopupDelete();
    void slotPopupNewView();

    /******************************************************
     * When this slot is signaled then we know that something
     * changed in the directory given by the argument.
     */
    void slotFilesChanged( const char *_url );

    /// This slot is called when the Properties dialog changed something.
    /**
      We only look at this to get informed about renamings, since we must rename
      the icons by hand. Otherwise KRootWidget would give the renamed icon another
      position on the screen.
      */
    void slotPropertiesChanged( const char *_url, const char *_new_name );

protected:
    /******************************************************
     * The instance of the root widget ( there is only one! ) is
     * stored here. Used by the function 'getKRootWidget'.
     */
    static KRootWidget* pKRootWidget;
    
    /******************************************************
     * Finds a icon searching for its URL.
     */
    KRootIcon* findIcon( const char *_url );
    /******************************************************
     * Updates the 'layoutList' from the file ~/.desktop. This does
     * not cause any icon to movee its place. Use update for doing that.
     */
    void loadLayout();
    /******************************************************
     * Saves all layout information of the root widget in the file
     * .desktop in the users home directory.
     */
    void saveLayout();

    /******************************************************
     * Returns a position for displaying the icon with the given URL.
     * If there is no entry for this URL ( the file may be new )
     * a default position is returned.
     */
    QPoint findLayout( const char * );

    /******************************************************
     * Contains a list of all visible icons on the root window.
     */
    QList<KRootIcon> icon_list;

    /******************************************************
     * Contains a layout object for every icon on the root window.
     * This list is created at startup time. It is used to position
     * all icons on the root window for the first time.
     * Do not use this list after startup since it may be outdated
     * or call loadLayout() to update this list.
     */
    QList<KRootLayout> layoutList;

    /******************************************************
     * When the user hits the right mouse button over an icon, then
     * this is are the URLs of this icon or all marked icons.
     */
    KStrList popupFiles;

    /******************************************************
     * When opening a popup menu, a pointer to it is stored here.
     * Before assigning another value to it delete the old one if
     * it is not already 0L.
     */
    QPopupMenu *popupMenu;

    /******************************************************
     * When a drop occured this is the corresponding drop zone
     */
    KDNDDropZone *dropZone;
    /******************************************************
     * When a drop occured this is position of the drop
     */
     int dropFileX, dropFileY;

    /******************************************************
     * When you copy lost of files to the desktop, KIOManager will
     * force an update after every operation. You can avoid this by setting
     * this falg.
     */
     bool noUpdate;

    /******************************************************
     * This drop zone is used for root drops.
     */
     KDNDDropZone *rootDropZone;

    /// The manager for the ~/Desktop file system
    /**
      This object is responsible for popup menus on desktop icons,
      for double clicks on desktop icons and so on.
      */
    KRootManager* manager;
};

#endif





