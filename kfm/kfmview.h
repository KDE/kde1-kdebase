#ifndef KFMVIEW_H
#define KFMVIEW_H

class KfmView;

#include <qlist.h>
#include <qdstream.h>
#include <qstring.h>
#include <qstack.h>

#include <ktopwidget.h>
#include <htmlview.h>
#include <html.h>

#include "kioserver.h"
#include "kstrlist.h"
#include "kfmman.h"
#include "kfmgui.h"
#include "htmlcache.h"

#include "config-kfm.h"

class KfmView : public KHTMLView
{
    Q_OBJECT
public:
    KfmView( KfmGui *_gui, QWidget *parent=0, const char *name=0, KfmView *_parent_view = 0L );
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
     * returns the backStack
     */
    QStack<SavedPage> *getBackStack();
    /**
     * returns the forwardStack
     */
    QStack<SavedPage> *getForwardStack();

    /**
     * @return write HTML-code to the widget (Hen). Quote HTML
     *   special characters, according to <b>RFC 1866</b>,
     *   Hypertext Markup Language 2.0,
     *   <i>9.7.1. Numeric and Special Graphic Entity Set</i>
     *   <table border=1>
     *   <tr><td>Character</td><td>Replacement</td></tr>
     *   <tr><td align=center>&quot;</td><td align=center>&amp;quot;</td></tr>
     *   <tr><td align=center>&amp;  </td><td align=center>&amp;amp;  </td></tr>
     *   <tr><td align=center>&lt;   </td><td align=center>&amp;lt;   </td></tr>
     *   <tr><td align=center>&gt;   </td><td align=center>&amp;gt;   </td></tr>
     *   </table>
     *   This is used to write filenames etc. which may have these special
     *   characters.<p>
     *   <b>Note:</b> This should move to KHTMLView someday ..<p>
     *   @param  text is the text to write to the widget
     */
    virtual void writeHTMLquoted (const char * text);

    /**
     * Open the URL.
     * The function asks the @ref #manager to open the URL.
     */
    virtual void openURL( const char *_url, bool _refresh, int _xoffset = 0, int _yoffset = 0 );

    virtual void openURL( const char *_url );

    virtual void openURL( const char *_url, const char *_data );

    /**
     * This function tries to read local properties (sven);
     */
    void checkLocalProperties (const char *_url);
    
    /**
     * Return the currently opened URL.
     */
    const char* getURL();
    
    const char* getJobURL();
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
    static QStrList *clipboard;
  
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
    const QFont defaultFont( void ) { return QFont(DEFAULT_VIEW_FONT); } 

    bool isHistoryStackLocked() { return stackLock; }

    /**
     * Pushes '_url' on the stack. This function is called if the
     * view displays a new URL and wants to store the old
     * URL on the stack.
     */
    void pushURLToHistory();

    /**
     * Set the URL the is going to be displayed if the user presses
     * the 'up' button.
     */
    void setUpURL( const char *_url );

    /**
     * Set the html mode to true (viewing a html file) or false (browsing a directory).
     */
    void setHTMLMode( bool bHtmlMode );

    bool hasBackHistory() { return !backStack->isEmpty(); }
    bool hasForwardHistory() { return !forwardStack->isEmpty(); }
    bool hasUpHistory() { return !m_strUpURL.isEmpty(); }
  
    /**
     * hack to get static classes up and running even with C++-Compilers/
     * Systems where a constructor of a class element declared static
     * would never get called (Aix, Alpha,...). In the next versions these
     * elements should disappear.
     */
    static void InitStatic() { clipboard = new QStrList; }

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
     * Called for example from the toolbars "Up" button
     */
    void slotUp();
  
    /**
     * Start an new terminal. Usually called from the menu.
     */
    void slotTerminal();
    /**
     * Invokes minicli . Usually called from the menu.
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
     * called, when khtml detects a redirect
     */
    void slotRedirect( int delay, const char * url );
    void slotDelayedRedirect(KHTMLView *);
    void slotDelayedRedirect2();

    /**
     * Copy the currently selected URLs to the @ref #clipboard.
     */
    void slotCopy();

    /**
     * Move the currently selected URLs to the @ref #clipboard.
     */
    void slotCut();

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
    void slotOpenWith();
    
    /**
     * This one catches signals from @ref HTMLView::popupMenu. The call is then piped thru
     * to @ref #slotPopupMenu.
     */
    void slotPopupMenu2( KHTMLView *_view, const char *_url, const QPoint &_point );
  
    /**
     * The user pressed the right mouse button over an URL.
     * Since @ref KFMManager has to create the popup menu, this function will
     * just call @ref #manager to do this.
     *
     * @param _point is already in global coordinates.
     * @param _current_dir inidcates that we want to open the context menu
     *                     for the current directory. In this case we dont want
     *                     to offer 'delete' for example.
     */
    void slotPopupMenu( QStrList & _urls, const QPoint &_point, bool _current_dir = false );
    /**
     * The user selected 'New View' in the context sensitive popup menu.
     * The URL belonging to the popup menu is stored in @ref popupFiles.
     */
    void slotNewView();
    /**
     * The user selected 'Properties' in the context sensitive popup menu.
     * The URL belonging to the popup menu is stored in @ref popupFiles.
     */
    void slotProperties();
    /**
     * The user wants to add the selected URLs to his bookmarks.
     */
    void slotBookmarks();
    /**
     * The user opened the popup menu for the trashbin and wants to
     * empty it.
     */
    void slotEmptyTrashBin();
    
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
     *
     * @param _reload tells wether just a refresh of the representation should be
     *                done ( false ) or wether the data is invalid and has to be
     *                refetched ( true ).
     */
    void slotUpdateView( bool _reload = true );

    void setDefaultTextColors( const QColor& textc,const QColor& linkc,
			       const QColor& vlink);

    void setDefaultBGColor( const QColor& bgcolor );

    void setUnderlineLinks( const bool uline );

    void setHTMLWidgetOptions();

    /**
     * This slot is called from popupmenu which is made in manager. It
     * saves properties for current url. (sven)
     */
    void slotSaveLocalProperties();

protected slots:
    void slotUpdateSelect(int);
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
    virtual void slotFormSubmitted( const char *_method, const char *_url, const char *_data );
    
protected:
    /**
     * This function determines wether we already visited a certain URL.
     * This in turn determines the color of the link.
     * The function is implemented in @ref KHTMLView and overloaded here.
     */
    virtual bool URLVisited( const char *_url );

    /**
     *  Copies currently selected content into the appropriate clipboard(s).
     *  Both SlotCopy & SlotCut Operations invoke this function to copy users
     *  selection(s) to clipboard. (Dawit A.)
     */
    void copySelection ();


    KFMManager *manager;
    
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
    QStack<SavedPage> *backStack;
    /**
     * Contains all URLs you can reach with the forward button.
     */
    QStack<SavedPage> *forwardStack;
    /**
     * Lock @ref #backStack and @ref #forwardStack .
     */
    static bool stackLock;

    /**
     * url for redirection and delay 
     */
    QString redirectURL;
    QTimer redirectTimer;
    int redirectDelay;

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
     * This flag is TRUE if we are in the middle of a selection using a
     * rectangular rubber band.
     */
    bool rectStart;
    /**
     * This flag is true if the rubber band is currently visible.
     */
    bool bandVisible;
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

    /**
     * This is just a temporary variable. It stores the URL the user clicked
     * on, until he releases the mouse again.
     *
     * @ref #mouseMoveHook
     * @ref #mousePressedHook
     */
    QString selectedURL;

    /**
     * @see #setUpURL
     */
    QString m_strUpURL;

    /**
     * This flag indicates whether a particular event
     * was generated by the popup menu or not. By default
     * it is set to false.
     */
    bool popupMenuEvent;

};

#endif

