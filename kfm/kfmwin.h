#ifndef KFMWIN_H
#define KFMWIN_H

// accelerator ids
#define COPY                1
#define PASTE               2
#define UP                  3
#define DOWN                4
#define PGUP                5
#define PGDOWN              6     
#define QUIT                13
#define HELP                50

#define BOOKMARK_ID_BASE	200

#include <qwidget.h>
#include <qmenubar.h>
#include <qlabel.h>
#include <qscrbar.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qdstream.h>
#include <qbttngrp.h>
#include <qgrpbox.h>
#include <kpanner.h>
#include <ktopwidget.h>

#include "kfmview.h"
#include "bookmark.h"
#include "kioserver.h"
#include "kstrlist.h"

class KFileWindow;

#include "kfmman.h"
#include "kfmtree.h"

class KFileWindow : public KTopLevelWidget
{
    Q_OBJECT
public:
    /******************************************************
     * This constructor can be used by classes derived from this one. Just
     * set _dir to 0L. This causes the 'init' function not to be called. You
     * MUST do that in your constructor!
     */
    KFileWindow( QWidget *parent=0, const char *name=0, const char *_dir=0 );
    virtual ~KFileWindow();

    /// The values for the variable 'viewMode'.
    enum ViewMode { ICON_VIEW, TEXT_VIEW, LONG_VIEW };    

    void refresh( KAbstractManager * );
    /// The URLs belonging to the currently opened popup menu.
    /**
      The 'actualManager' opens the popup menu and tells the window using
      this function that he opened a popup menu for this URL.
      */
    void setPopupFiles( QStrList &_files );
    /// Returns a list of all URLs belonging to the opened popup menu
    QStrList& getPopupFiles() { return popupFiles; }
    
    /// Display .kde.html files ?
    /** 
      Returns the flag 'bViewHTML'. Tells wether the user wants to see
      the .kde.html files instead of the normal view.
      */
    bool isViewHTML() {	return bViewHTML; }

    /// Open the URL
    /**
      The function tries to find the appropriate manager for the URL
      and tells him to open the URL. If there is no manager it tries to
      run the default binding. _url may be a directory, but it can be an
      executable or data file, too.
      */
    virtual void openURL( const char *_url );

    /// Return the currently opened URL
    const char* getURL();
    
    /// Find an window that displays a certain URL
    /**
      Uses the 'windowList' variable to find all open windows.
      */
    static KFileWindow* findWindow( const char *_url );

    /// Returns the list of all windows
    static QList<KFileWindow>& getWindowList() { return windowList; }
    
    /// Return the view mode.
    /**
      For example KFileManager use this to determine how to display the
      directory listing.
      */
    ViewMode getViewMode() { return viewMode; }
    
    /// Are the .dot files visible?    
    bool getShowDot() { return showDot; }

    /// Is the visual schnauzer turned on ? 
    bool isVisualSchnauzer() { return visualSchnauzer; }

    /// Return a pointer to the KTarManager
    /**
      Used by KFileManager!
      */
    KTarManager *getTarManager() { return tarManager; }

    /// Holds a list of all files copied or cut
    /**
      This variable is public, because it is used by KRootWidget, too.
      */
    static QStrList clipboard;

    /// Sets the appropriate pixmaps for the toolbars buttons
    void enableToolbarButton( int id, bool enable );
    
public slots:
    /// Menu "File->Close"
    void slotClose();
    /// Menu "File->Quit"
    void slotQuit();
    /// Menu "File->New Window"
    void slotNewWindow();
    /// Menu "File->Open Location"
    void slotOpenLocation( );
    /// Menu "File->New->*"
    void slotNewFile( int _id );
    /// Menu "File->Run"
    void slotRun();
    /// Menu "File->Terminal"
    void slotTerminal();
    
    /// Menu "Edit->Select"
    void slotSelect();
    void slotCopy();
    void slotDelete();
    void slotPaste();
    /**
     * This slot is called if the user selected 'Open With' in the context sensitive popup
     * menu. The URL belonging to the popup menu is stored in 'popup_file'.
     */
    void slotPopupOpenWith();   
    /// Menu "Help->About"
    void slotAbout();
    /// Menu "Help->How can I..."
    void slotHelp();
    
    /// Set the window title.
    void slotTitle( const char *_title );
    /// Called when the user double clicks on an URL.
    void slotDoubleClick( const char *_url, int _button );
    /// Called if the document in the HTML Widget changes
    void slotDocumentChanged();
   
    /// Menu "Tool->Find"
    void slotToolFind();

    /// Menu "View->Update"
    void slotViewUpdate();
    /// Menu "View->Toggle HTML view"
    void slotViewHTML();
    /// Menu "View->Show/Hide Dot Files"
    void slotShowDot();
    /// Menu "View->Visual Schnauzer ON/OFF"
    void slotShowSchnauzer();
    /// Menu "View->Tree View ON/OFF"
    void slotShowTreeView();
    /// Menu "View->Icon View"
    void slotIconView();
    /// Menu "View->Long View"
    void slotLongView();
    /// Menu "View->Text View"
    void slotTextView();
    /// Menu "View->Rescan Bindings"
    void slotRescanBindings();
    
    /// Menu "Bookmarks->Add Bookmark"
    void slotAddBookmark();
    
    /// Bound to all menu items in "Bookmarks"
    void slotBookmarkSelected( int id );
    /// Calles when the bookmarks changed
    void slotBookmarksChanged();

    /// Toolbar "Forward"
    void slotForward();
    /// Toolbar "Back"
    void slotBack();
    /// Toolbar "Home"
    void slotHome();

    /// Toolbar "Stop"
    void slotStop();
    
    /// Connected to the HTML widget
    void slotScrollVert( int );
    /// Connected to the HTML widget
    void slotScrollHorz( int );

    void slotKeyUp ();
    void slotKeyDown ();
    void slotPageUp ();
    void slotPageDown ();    

    /// Called if the panner changes its position
    void slotPannerChanged();

    /// Called if the user selects an URL in the tree view
    void slotOpenURL( const char *_url );
    
    /// The user pressed the right mouse button over an URL
    /**
      _point is already in global coordinates.
      Since 'actualManager' has to created the popup menu, this function will
      just call 'actualManager'.
      */
    void slotPopupMenu( QStrList & _urls, const QPoint &_point );
    /// A convenience function that takes only one URL as argument
    void slotPopupMenu( const char *_url, const QPoint &_point );
    /// Called if the user presses the right mouse button over the tree view
    void slotTreeViewPopupMenu( const char *_url, const QPoint &_point );

    /// The user selected 'Copy' in the context sensitive popup menu.
    /**
      The URL belonging to the popup menu is stored in 'popup_file'.
      */
    void slotPopupCopy();
    /// The user selected 'Paste' in the context sensitive popup menu.
    /**
      The URL belonging to the popup menu is stored in 'popup_file'.
      */
    void slotPopupPaste();
    /// The user selected 'Cd' in the context sensitive popup menu.
    /**
      The URL belonging to the popup menu is stored in 'popup_file'.
      */
    void slotPopupCd();
    /// The user selected 'Delete' in the context sensitive popup menu.
    /**
      The URL belonging to the popup menu is stored in 'popup_file'.
      */
    void slotPopupDelete();
    /// The user selected 'NewView' in the context sensitive popup menu.
    /**
      The URL belonging to the popup menu is stored in 'popup_file'.
      */
    void slotPopupNewView();
    /// The user selected 'Properties' in the context sensitive popup menu.
    /**
      The URL belonging to the popup menu is stored in 'popup_file'.
      */
    void slotPopupProperties();
    
    /******************************************************
     * Called when someone dropped something over the drop zone
     * associated withe FileView widget.
     */
    void slotDropEvent( KDNDDropZone * );
    /******************************************************
     * Called when a drag ( no matter who initiated it ) enters
     * the drop zone or is moved inside the drop zone.
     */
    void slotDropEnterEvent( KDNDDropZone * );
    /******************************************************
     * Called when a drag ( no matter who initiated it ) leaves
     * the drop zone. You can be shure that you had one or more
     * slotDropEvent calls before that.
     */
    void slotDropLeaveEvent( KDNDDropZone * );

    /******************************************************
     * If a change occured in a directory this event is signaled to this
     * slot. The URL of the directory is the argument given to this function.
     * If this directory is displayed in this FileWindow we will have to
     * update the contents of the FileView and call refresh. _last indicates
     * wether more updates are expected and _event is thee kind of event that
     * happend to _url. This slot is bound to KIOManager's notify signal.
     * KIOHttp sends this signal for example after copying a HTML page to you
     * hard disk. _event is not used yet.
     */
    void slotFilesChanged( const char *_url );

    /// Handles mount notifies
    /**
      If we display something of the lokal filesystem, it might be, that we display
      the icon of a device (for example: CDROM). If this device is mounted/unmounted this
      would cause the icon to change => Refresh the contents of the window.
      */
    void slotMountNotify();

    /// Called if the mouse pointer moved to another URL
    /**
      '_url' may be 0L if the pointer is over no URL at all.
      */
    void slotOnURL( const char *_url );
    
protected:

    /******************************************************
     * Calls 'initGUI' and 'initFileManagers'.
     * This is the main init function.
     */
    virtual void init( const char * );
    /******************************************************
     * Initializes the GUI.
     * Calls a list of functions that fo the job. So you can for
     * example dervice from this class and override 'initMenu' if you
     * want to have another menu.
     */
    virtual void initGUI();
    virtual void initToolBar();
    virtual void initMenu();
    virtual void initView();
    virtual void initAccel();
    virtual void initStatusBar();
    virtual void initPanner();
    virtual void initTreeView();
    virtual void initLayout();
    
    /******************************************************
     * Creates all file managers and opens the directory _dir
     * with a suitable manager. If _dir is 0L all managers are
     * created, a default manager is made active but no directory
     * is opened. This can be used if you want to use your own
     * manager for default but KFileWindow depens on having the other
     * managers.
     * Call only after the GUI ( in special 'view' ) is initialized.
     */
    virtual void initFileManagers();

    virtual void closeEvent( QCloseEvent *e );
    virtual void resizeEvent( QResizeEvent * );

    /// Fill the 'menu' with all boomarks in 'parent'.
    void fillBookmarkMenu( KBookmark *parent, QPopupMenu *menu, int &id );

    /// Add a bookmark
    /**
      The bookmark is added to KBookmarkManager and saved to disk.
      The menu becomes updated, too.
      */
    void addBookmark( const char *_title, const char *url );

    /// Manager for the bookmarks
    static KBookmarkManager bookmarkManager;

    /// List of all open windows
    static QList<KFileWindow> windowList;
    
    /// Menu containing the bookmarks
    QPopupMenu *bookmarkMenu;
    
    /******************************************************
     * The file view widgets offset to the upper border. This offset
     * depends on the height of the menu bar and wether a toolbar is
     * displayed or not.
     */
    int topOffset;

    /// Contains the windows title.
    QString title;
    
    /// Show .kde.html files ?
    /**
      When showing a directory that contains a file called .kde.html this
      HTML file may be displayed instead of the normal icon view. This flag
      tells us wether the user wants this or not. Default is TRUE.
      */
    bool bViewHTML;
    
    QAccel *accel;

    /// The toolbar
    KToolBar *toolbar;
    
    KMenuBar *menu;
    QPopupMenu *mview;
    QScrollBar *horz;
    QScrollBar *vert;

    /// The HTML Widget
    KFileView *view;

    KFileManager *manager;
    KTarManager *tarManager;
    KFtpManager *ftpManager;
    KHttpManager *httpManager;
    /******************************************************
     * The manager that is currently responsible for the contents
     * of the file view widget.
     */
    KAbstractManager *actualManager;
    /******************************************************
     * When the user pressed the right mouse button over an URL a popup menu
     * is displayed. The URL belonging to this popup menu is stored here.
     */
    KStrList popupFiles;

    /******************************************************
     * Sometimes a drop forces the program to display a popup menu. A pointer
     * to this menu is stored here. Before you assign a new popup menu to this
     * pointer delete the old popup menu associated to it.
     */
    QPopupMenu *popupMenu;
    /// The drop zone for the KViewWindow widget.
    KDNDDropZone *dropZone;

    /// Contains all URLs you can reach with the back button
    QStack<QString>backStack;
    /// Contains all URLs you can reach with the forward button
    QStack<QString>forwardStack;

    /// Lock 'backStack' and 'forwardStack'
    /**
      This is used by 'slotBack' for example. When calling 'openURL' we dont
      like the history stacks to be manipulated. This flag can turn that off.
      */
    bool stackLock;

    /// The path where the templates are stored
    /**
      This string always ends with "/".
      */
    QString templatePath;

    /// The menu "New" in the "File" menu
    /**
      Since the items of this menu are not connected themselves
      we need a pointer to this menu to get information about the
      selected menu item.
      */
    QPopupMenu *menuNew;

    /// LongView, IconView, TextView
    /**
      With this variable you can switch the view mode. Valid values
      are the enums TEX_VIEW etc.
      The default is ICON_VIEW.
      */
    ViewMode viewMode;

    /// This flag is set if we display the .* files, too
    bool showDot;

    /// This flag is set, if kfm should work like a visual schnauzer
    bool visualSchnauzer;

    QLabel *statusBar;
    QLabel *statusBar2;

    KPanner *panner;
    QWidget *pannerChild0;
    QWidget *pannerChild1;

    KFMTreeView *treeView;
    bool bTreeViewInitialized;
    bool bTreeView;
};

#endif

