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
#include "kfmgui.h"
#include "kioserver.h"
#include "kstrlist.h"
#include "kiojob.h"

#include "config-kfm.h"

class KRootWidget;
class KFMManger;
/**
 * This class is used to store the position of files, which are not on the
 * file system yet. For example if a file is dropped on the desktop, you know
 * the position, but the file itself is not present yet. Until this happens,
 * we store the stuff here.
 */
class KRootLayout
{
protected:
    int x,y;
    QString url;


public:
    KRootLayout( const char *_url, int _x, int _y ) { url = _url; x = _x; y = _y; }

    int getX() { return x; }
    int getY() { return y; }
    const char* getURL() { return url.data(); }
};

class KRootIcon : public KDNDWidget
{
    Q_OBJECT
public:
    KRootIcon( const char* _url, int _x, int _y );
    virtual ~KRootIcon();

    /// Updates the pixmap and the tool tip ( if required )
    /**
      This function is usually called from KRootWidget::update().
      */
    virtual void updatePixmap();
    virtual void init();
    virtual void initToolTip();

    const char* getURL() { return url.data(); }

    /**
     * Call this to reinit/redraw the icon (e.g. if the iconstyle
     * has changed)
     */
    void update();
    /**
     * Selects/Unselects the icon.
     */
    void select( bool _select );
    /**
     * @return TRUE if the icon is selected ( highlighted )
     */
    bool isSelected() { return bSelected; }
 
    bool isbeingmoved() { return beingmoved; }
    void startmove() { beingmoved = true; } 
    void endmove() { beingmoved = false; } 

    /// Call to rename an icon
    /**
      _new_name is not a complete URL. It is only the filename.
      */
    virtual void rename( const char *_new_name );

    int gridX() { return grid_x; }
    int gridY() { return grid_y; }
    void setGridX( int _x ) { grid_x = _x; }
    void setGridY( int _y ) { grid_y = _y; }
    
public slots:
    /**
     * Called when the user drops something over the icon.
     */
    void slotDropEvent( KDNDDropZone *_zone );
    
    void slotDropCopy();
    void slotDropMove();
    void slotDropLink();

    void slotFontChanged();

protected:
    virtual void resizeEvent( QResizeEvent * );
    virtual void paintEvent( QPaintEvent *_event );
    virtual void mouseDoubleClickEvent( QMouseEvent *_mouse );
    virtual void mousePressEvent( QMouseEvent *_mouse );
    virtual void dndMouseMoveEvent( QMouseEvent *_mouse );
    virtual void dndMouseReleaseEvent( QMouseEvent *_mouse );
    virtual void dragEndEvent();

    /**
     * Called from within @ref #slotDropEvent
     */
    void dropPopupMenu( KDNDDropZone *_zone, const char *_dest, const QPoint *_p );

    /**
     * This is the URL of the file represented by this icon.
     */
    QString url;
    /**
     * This is only the filename with no path in front of it.
     */
    QString file;
    /**
      * help function to find out filename for a directory
      */
    void initFilename();
    /**
     * Dont delete this pixmap. It is cached in @ref KMimeType::pixmapCache
     */
    QPixmap *pixmap;
    QBitmap mask;

    int pixmapXOffset;
    int pixmapYOffset;
    int textYOffset;
    int textXOffset;

    bool pressed;
    int press_x, press_y;
    
    int width;
    int height;

    /**
     * The DropZone bound to this icon
     */
    KDNDDropZone *dropZone;

    /**
     * Tells us wether this icon is currently selected.
     *
     * @see #select
     */
    bool bSelected;

    QString dropDestination;
    QPopupMenu *popupMenu;

    int grid_x;
    int grid_y;

    /*
     * whether the icon currently occupies space or is in a state where a
     * new position for it is to be calculated
     */
    bool beingmoved;

    /**
     * Tells, wether this icons represents a link in the UNIX sense of
     * a link. If yes, then we have to draw the label with an italic font.
     */
    bool bIsLink;

    static QPixmap *link_pixmap;
    static QPixmap *ro_pixmap;
};

class KRootWidget : public QWidget
{
    friend KRootIcon;
    friend KfmView;
    friend KFMManager;
    
    Q_OBJECT
public:
    KRootWidget( QWidget *parent=0, const char *name=0 );
    virtual ~KRootWidget() { }

    void openURL( const char *_url );
    void openPopupMenu( QStrList &_urls, const QPoint &_point );
    bool isBindingHardcoded( const char *_txt );

    void setRootGridParameters(int gridwidth ,int gridheight);
	void setRootIconStyle(int newiconstyle);
	void setRootIconColors(QColor &, QColor &);//CT 12Nov1998

    /**
     * Takes all icons corresponding to the given URLs and moves them.
     * This function uses 'icon->dndStartX/Y()' and 'p' to determine the amount of
     * pixels the icons have to move.
     */
    void moveIcons( QStrList & _urls, QPoint &p );

    /**
     * Unselects all icons.
     */
    void unselectAllIcons();
    /**
     * Selects all root icons in the given rectangle.
     */
    void selectIcons( QRect &rect );
    /**
     * Fills '_list' with the URLs of all selected icons.
     */
    void getSelectedURLs( QStrList &_list );
      
    /**
     * Returns a list of selected icons or a null string if
     * no icon was currently selected.
     */
    QString getSelectedURLs();

    /**
     * Since there is only one root widget, this method can be used to
     * get its pointer. This is better than using global variables since this
     * would not be object oriented.
     */
    static KRootWidget* getKRootWidget() { return pKRootWidget; }

    /**
     * The URL of the desktop. This string always ends with "/".
     */
    QString desktopDir;
        
    /**
     * Rearranges all icons on the desktop. The algorithm tries to fit all icons
     * in a certain grid. Starting in the upper left corner of the screen.
     */
    void rearrangeIcons();
    
    /**
     * color of the icon label text
     */
    const QColor& labelForeground( void ) const { return labelColor; }

    /**
     * color of the icon background
     */
    const QColor& iconBackground( void ) const { return iconBgColor; }

    /**
     * Style of the root icons.
     *
     * @return 0=shaped with unshaped label, 1=shaped with shaped label
     */
    int iconStyle( void ) const { return iconstyle; }


public slots:
    void slotPopupActivated( int _id );

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
    
    /**
     * If the 'rootDropZone' receives a drop action, this slot is signaled.
     */
    void slotDropEvent( KDNDDropZone *_zone );

    /// Open a properties dialog
    /**
      The URLs belonging to the popup menu is stored in 'popupFiles'.
      */
    void slotPopupProperties();
    void slotPopupCopy();
    void slotPopupPaste();
    void slotPopupDelete();
    void slotPopupTrash();
    void slotPopupNewView();
    void slotPopupOpenWith();
    /**
     * Called if the user wants to empty the trash bin.
     */
    void slotPopupEmptyTrash();
    
    /**
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
    void slotPropertiesCancel();

protected:
    /**
     * The instance of the root widget ( there is only one! ) is
     * stored here. Used by the function 'getKRootWidget'.
     */
    static KRootWidget* pKRootWidget;
    
    /**
     * Finds a icon searching for its URL.
     */
    KRootIcon* findIcon( const char *_url );
    /**
     * Saves all layout information of the root widget in the file
     * ~/.kde/share/apps/kfm/desktop.
     */
    void saveLayout();

    /**
     * @return a pointer to the layout object for the given
     *         URL if there is any. It is a good idea to remove
     *         the layout of the list of layouts once its corresponding
     *         file showed up in the file system.
     */
    KRootLayout* findLayout( const char *_url );

    /**
     * @return true if the position in the grid is already used by some icon.
     */
    bool isPlaceUsed( int _gridx, int _gridy );

    /**
     * @return the position next to (_gridx,_gridy) that is not yet used.
     */
    QPoint findFreePlace( int _gridx, int _gridy );
    
    /**
     * @return a point where we can place a new icon. The coordinates
     *         returned are positions in the root widgets grid.
     */
    QPoint findFreePlace();
    
    /**
     * Contains a list of all visible icons on the root window.
     */
    QList<KRootIcon> icon_list;

    /**
     * Contains a layout object for every icon on the root window.
     * This list is created at startup time. It is used to position
     * all icons on the root window for the first time.
     * Do not use this list after startup since it may be outdated
     * or call loadLayout() to update this list.
     */
    QList<KRootLayout> layoutList;

    /**
     * When the user hits the right mouse button over an icon, then
     * this is are the URLs of this icon or all marked icons.
     */
    KStrList popupFiles;

    /**
     * When opening a popup menu, a pointer to it is stored here.
     * Before assigning another value to it delete the old one if
     * it is not already 0L.
     */
    QPopupMenu *popupMenu;

    /**
     * When a drop occured this is the corresponding drop zone
     */
    KDNDDropZone *dropZone;
    /**
     * When a drop occured this is position of the drop
     */
     int dropFileX, dropFileY;

    /**
     * When you copy lost of files to the desktop, KIOManager will
     * force an update after every operation. You can avoid this by setting
     * this falg.
     */
     bool noUpdate;

    /**
     * This drop zone is used for root drops.
     */
     KDNDDropZone *rootDropZone;

    /**
     * The color used for the text on the label.
     */
    QColor labelColor;
    /**
     * The color used for the background of the label.
     */
    QColor iconBgColor;
    /**
     * If this value is 0, then the label is not shaped. A value of 1 means
     * that the value is shaped.
     */
    int iconstyle;

    /**
     * The mouseposition when opening the popup Menu.
     *
     * @see #popupMenu
     */
    QPoint popupMenuPosition;

    /**
     * The start of am icon-drag in global coordinates
     */
    QPoint dndStartPos;
    /** 
      *  Hold the rood window icon grid parameters.
      */
    int gridwidth, gridheight;
    int oldgridwidth, oldgridheight;
};

#endif





