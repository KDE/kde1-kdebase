#ifndef KFMVIEW_H
#define KFMVIEW_H

class KfmView;

#include <qlist.h>
#include <qdstream.h>
#include <qstring.h>
#include <qstack.h>

#include <ktopwidget.h>
#include <htmlview.h>

#include "kioserver.h"
#include "kstrlist.h"
#include "kfmman.h"
#include "kfmgui.h"
#include "htmlcache.h"

class KfmView : public KHTMLView
{
    Q_OBJECT
public:
    KfmView( KfmGui *_gui, QWidget *parent=0, const char *name=0, KHTMLView *_parent_view = 0L );
    virtual ~KfmView();

    /**
     * Overrides @ref KHTMLView::newView. It just creates a new instance
     * of KfmView. These instances are used as frames.
     */
    virtual KHTMLView* newView( QWidget *_parent, const char *_name, int _flags );

    void begin( const char *_url = 0L, int _x_offset = 0, int _y_offset = 0 );
    
    /**
     * @return the currently selected view ( the one with the black
     *         frame around it ) or 'this' if we dont have frames.
     *         Use this function only with the topmost KfmView.
     *         This function will never return 0L.
     */
    KfmView* getActiveView();
	
    /**
     * Stores the URLs belonging to the currently opened popup menu.
     * The @ref #manager opens the popup menu and tells the window using
     * this function that he opened a popup menu for this URL.
     */
    void setPopupFiles( QStrList &_files );
    /**
     * @return a list of all URLs belonging to the opened popup menu.
     */
    QStrList& getPopupFiles() { return popupFiles; }
    
    /**
     * Open the URL.
     * The function asks the @ref #manager to open the URL.
     */
    virtual void openURL( const char *_url );

    /**
     * Return the currently opened URL.
     */
    const char* getURL();

    KfmGui* getGUI() { return gui; }
    HTMLCache* getHTMLCache() { return htmlCache; }
    
    /**
     * Splits the view into 2 frames of equal size. Every frame loads the
     * current URL. Use this function for example to get a "Norton Commander"
     * line interface.
     */
    void splitWindow();
    
    /**
     * Holds a list of all files copied or cut.
     * This variable is public, because it is used by @ref KRootWidget, too.
     */
    static QStrList clipboard;
  
    /**
     * This function is hooked into the event processing of the @ref KHTMLWidget.
     *
     * @see KHTMLView::mouseMoveHook
     */
    virtual bool mouseMoveHook( QMouseEvent *_ev );
    /**
     * This function is hooked into the event processing of the @ref KHTMLWidget.
     *
     * @see KHTMLView::mouseReleasedHook
     */
    virtual bool mouseReleaseHook( QMouseEvent *_ev );
    /**
     * This function is hooked into the event processing of the @ref KHTMLWidget.
     *
     * @see KHTMLView::mousePressedHook
     */
    virtual bool mousePressedHook( const char* _url, const char *_target, QMouseEvent *_ev,
				   bool _isselected);
    /**
     * This function is hooked into the event processing of the @ref KHTMLWidget.
     *
     * @see KHTMLView::dndHook
     */
    virtual bool dndHook( const char *_url, QPoint &_p );

    /**
     * This function returns the default font of the HTML widget
     */
    const QFont defaultFont( void ) { return QFont("times"); } 

    bool isHistoryStackLocked() { return stackLock; }
    
signals:

    /**
     * This signal is emitted if the user changed the history
     * stack of this view @ref KfmGui connects
     * to this signal to update the toolbar buttons.
     *
     * @see backStack
     * @see forwardStack
     */
    void historyUpdate( bool _back, bool _forward );
    
    /**
     * Emitted whenever a new URL is visited.
     *
     * @see KfmGui::slotNewURL
     */
    void newURL( const char *_url );
    
public slots:

    /**
     * Called for example from the toolbars "Forward" button
     */
    void slotForward();
    /**
     * Called for example from the toolbars "Back" button
     */
    void slotBack();
    /**
     * Pushes '_url' on the stack. This function is called if the
     * view displays a new URL and wants to store the old
     * URL on the stack.
     */
    void slotURLToStack( const char *_url );
    
    /**
     * Start an new terminal. Usually called from the menu.
     */
    void slotTerminal();
    /**
     * Start acli. Usually called from the menu.
     */
    void slotRun();
    /**
     * Stop all downloading activities. Usually called if the user
     * presses the stop button on the toolbar.
     */
    void slotStop();
    /**
     * Reloads the current directory. Information in the cache is flushed
     * to make shure that we dont get the old stuff again.
     */
    void slotReload();
    
    /**
     * Copy the currently selected URLs to the @ref #clipboard.
     */
    void slotCopy();
    /**
     * Delete the currently selected URLs
     */
    void slotDelete();
    /**
     * Moves all currently selected URLs to the Trash-Bin
     */
    void slotTrash();
    /**
     * Insert all URLs in the @ref #clipboard in the current directory/URL.
     */
    void slotPaste();

    /**
     * This slot is called if the user selected 'Open With' in the context sensitive popup
     * menu. The URL belonging to the popup menu is stored in @ref popupFiles .
     */
    void slotPopupOpenWith();   
    
    /**
     * The user pressed the right mouse button over an URL.
     * Since @ref KFMManager has to create the popup menu, this function will
     * just call @ref #manager to do this.
     *
     * @param _point is already in global coordinates.
     */
    void slotPopupMenu( QStrList & _urls, const QPoint &_point );

    /**
     * The user selected 'Copy' in the context sensitive popup menu.
     * The URL belonging to the popup menu is stored in @ref popupFiles.
     */
    void slotPopupCopy();
    /**
     * The user selected 'Paste' in the context sensitive popup menu.
     * The URL belonging to the popup menu is stored in @ref popupFiles.
     */
    void slotPopupPaste();
    /**
     * The user selected 'Cd' in the context sensitive popup menu.
     * The URL belonging to the popup menu is stored in @ref popupFiles.
     */
    void slotPopupCd();
    /**
     * The user selected 'Delete' in the context sensitive popup menu.
     * The URL belonging to the popup menu is stored in @ref popupFiles.
     */
    void slotPopupDelete();
    /**
     * The user selected 'Move to Trash' in the context sensitive popup menu.
     * The URL belonging to the popup menu is stored in @ref popupFiles.
     */
    void slotPopupTrash();
    /**
     * The user selected 'New View' in the context sensitive popup menu.
     * The URL belonging to the popup menu is stored in @ref popupFiles.
     */
    void slotPopupNewView();
    /**
     * The user selected 'Properties' in the context sensitive popup menu.
     * The URL belonging to the popup menu is stored in @ref popupFiles.
     */
    void slotPopupProperties();
    /**
     * The user wants to add the selected URLs to his bookmarks.
     */
    void slotPopupBookmarks();
    /**
     * The user opened the popup menu for the trashbin and wants to
     * empty it.
     */
    void slotPopupEmptyTrashBin();
    
    /**
     * Called when someone dropped something over the drop zone
     * associated withe FileView widget.
     */
    void slotDropEvent( KDNDDropZone * );
    /**
     * Called when a drag ( no matter who initiated it ) enters
     * the drop zone or is moved inside the drop zone.
     */
    void slotDropEnterEvent( KDNDDropZone * );
    /**
     * Called when a drag ( no matter who initiated it ) leaves
     * the drop zone. You can be shure that you had one or more
     * slotDropEvent calls before that.
     */
    void slotDropLeaveEvent( KDNDDropZone * );

    /**
     * If a change occured in a directory this event is signaled to this
     * slot. The URL of the directory is the argument given to this function.
     * If this directory is displayed in this KfmView we will have to
     * update the contents of the HTML widget. _last indicates
     * wether more updates are expected and _event is the kind of event that
     * happend to _url.
     */
    void slotFilesChanged( const char *_url );

    /**
     * Handles mount notifies.
     * If we display something of the lokal filesystem, it might be, that we display
     * the icon of a device (for example: CDROM). If this device is mounted/unmounted this
     * would cause the icon to change => Refresh the contents of the window.
     */
    void slotMountNotify();

    /**
     * Updates the view. This does not mean that for example the directory is going
     * to be rescaned. This may happen but dont expect it to. Use this function if
     * for example the view mode changed. The function wont reload framesets. Only
     * the content of the single frames is updated.
     */
    void slotUpdateView( );

protected slots:
    /**
     * Called when the user clicks on an URL.
     */
    virtual void slotURLSelected( const char *_url, int _button, const char *_target );

    /**
     * Called if the mouse pointer moved to another URL.
     *
     * @param _url may be 0L if the pointer is over no URL at all.
     */
    virtual void slotOnURL( const char *_url );

    /**
     * Called if the user presses the submit button.
     *
     * @param _url is the <form action=...> value
     * @param _method is the <form method=...> value
     */
    virtual void slotFormSubmitted( const char *_method, const char *_url );
    
protected:
    KFMManager *manager;
    
    /**
     * When the user pressed the right mouse button over an URL a popup menu
     * is displayed. The URL belonging to this popup menu is stored here.
     */
    KStrList popupFiles;

    /**
     * Sometimes a drop forces the program to display a popup menu. A pointer
     * to this menu is stored here. Before you assign a new popup menu to this
     * pointer delete the old popup menu associated to it.
     */
    QPopupMenu *popupMenu;
    /**
     * The drop zone for the KViewWindow widget.
     */
    KDNDDropZone *dropZone;

    /**
     * Contains all URLs you can reach with the back button.
     */
    QStack<QString>backStack;
    /**
     * Contains all URLs you can reach with the forward button.
     */
    QStack<QString>forwardStack;
    /**
     * Lock @ref #backStack and @ref #forwardStack .
     */
    bool stackLock;

    KfmGui *gui;
    
    /**
     * A list containing all direct child views. Usually every frame in a frameset becomes
     * a child view. This list is used to update the contents of all children ( see @ref #slotUpdateView ).
     *
     * @see newView
     */
    QList<KfmView> childViewList;

    /**
     * Used to download and cache images from the web.
     */
    HTMLCache *htmlCache;

    /**
     * Upper left corner of a rectangular selection.
     */
    int rectX1, rectY1;
    /**
     * Lower right corner of a rectangular selection.
     */
    int rectX2, rectY2;
    /**
     * This flag is TRUE if we are in the middle of a selection using a rectangular
     * rubber band.
     */
    bool rectStart;
    /**
     * Painter used for the rubber band.
     */
    QPainter *dPainter;    

    /**
     * If this flag is set to TRUE, then the next call to @ref #mouseReleaseHook will
     * be ignored and TRUE returned.
     *
     * @see #mousePressedHook
     */
    bool ignoreMouseRelease;
  
};

#endif

