#ifndef KFMGUI_H
#define KFMGUI_H

#define BOOKMARK_ID_BASE	200

// accelerator ids
#define UP                  1
#define DOWN                2
#define PGUP                3
#define PGDOWN              4     

class KfmGui;

#include <qwidget.h>
#include <qlabel.h>
#include <qlist.h>
#include <qdstream.h>
#include <kpanner.h>
#include <ktopwidget.h>
#include <kstatusbar.h>

#include "kfmview.h"
#include "bookmark.h"
#include "kioserver.h"
#include "kstrlist.h"

#include "kfmman.h"
#include "kfmtree.h"

class KfmGui : public KTopLevelWidget
{
    Q_OBJECT
public:
    /**
     * Constructs a new KFM toplevel window.
     */
    KfmGui( QWidget *parent=0, const char *name=0, const char *_url = 0L );
    /**
     * Deletes the window.
     */
    virtual ~KfmGui();

    /**
     * The values for the variable @ref #viewMode .
     */
    enum ViewMode { ICON_VIEW, TEXT_VIEW, LONG_VIEW };    
    /**
     * For example KFileManager use this to determine how to display the
     * directory listing.
     *
     * @return the view mode.
     *
     * @see #ViewMode
     */
    ViewMode getViewMode() { return viewMode; }
    /**
     * @return TRUE if we want to display .kde.html or index.html files.
     */
    bool isViewHTML() { return bViewHTML; }
    /**
     * @return TRUE if the user wants to see the .dot files.
     */
    bool isShowDot() { return showDot; }
    /**
     * @return TRUE if the user enabled the visual schnauzer.
     */
    bool isVisualSchnauzer() { return visualSchnauzer; }

    /**
     * @return the URL that the @ref #view is currently displaying.
     */
    const char* getURL();

    /**
     * Finds an window that displays a certain URL.
     * Uses the @ref #windowList variable to find all open windows.
     */
    static KfmGui* findWindow( const char *_url );

    /**
     * @eturn the list of all open windows.
     */
    static QList<KfmGui>& getWindowList() { return windowList; }
    
    /**
     * Enables a button in the toolbar if _enable is TRUE.
     */
    void enableToolbarButton( int _id, bool _enable );

    /**
     * Builds new HTML code for the current URL. The function does
     * not force a reload of the directory information. This is not 
     * a reload.
     *
     * @see KfmView::slotUpdateView
     * @see KfmView::slotReload
     */
    void updateView();

    /**
     * Adds '_url' to the bookmarks with '_title' as description.
     */    
    void addBookmark( const char *_title, const char *_url );
    
public slots:
    /**
     * Menu "File->Close"
     */
    void slotClose();
    /**
     * Menu "File->Quit"
     */
    void slotQuit();
    /**
     * Menu "File->New Window"
     */
    void slotNewWindow();
    /**
     * Menu "File->Open Location"
     */
    void slotOpenLocation( );
    /**
     * Menu "File->New->*"
     */
    void slotNewFile( int _id );
    /**
     * Menu "File->Run"
     */
    void slotRun();
    /**
     * Menu "File->Terminal"
     */
    void slotTerminal();
    /**
     * Menu "File->Print"
     */
    void slotPrint();          

    /**
     * Menu "Edit->Select"
     */
    void slotSelect();
    /**
     * Menu "Edit->Copy"
     */
    void slotCopy();
    /**
     * Menu "Edit->Delete"
     */
    void slotDelete();
    /**
     * Menu "Edit->Move to Trash"
     */
    void slotTrash();
    /**
     * Menu "Edit->Paste"
     */
    void slotPaste();

    /**
     * Menu "Help->About"
     */
    void slotAbout();
    /**
     * Menu "Help->How can I..."
     */
    void slotHelp();

    /**
     * Menu "Tool->Find"
     */
    void slotToolFind();

    /**
     * Menu "View->Toggle HTML view"
     */
    void slotViewHTML();
    /**
     * Menu "View->Show/Hide Dot Files"
     */
    void slotShowDot();
    /**
     * Menu "View->Visual Schnauzer ON/OFF"
     */
    void slotShowSchnauzer();
    /**
     * Menu "View->Tree View ON/OFF"
     */
    void slotShowTreeView();
    /**
     * Menu "View->Icon View"
     */
    void slotIconView();
    /**
     * Menu "View->Long View"
     */
    void slotLongView();
    /**
     * Menu "View->Text View"
     */
    void slotTextView();
    /**
     * Menu "View->Rescan Bindings"
     */
    void slotRescanBindings();
    /**
     * Menu "View->Split Window"
     */
    void slotSplitWindow();
    /**
     * Menu "View->Frame Source"
     */
    void slotViewFrameSource();
    
    /**
     * Menu "Bookmarks->Add Bookmark"
     */
    void slotAddBookmark();
    /**
     * Bound to all menu items in "Bookmarks"
     */
    void slotBookmarkSelected( int id );
    /**
     * Called when the bookmarks changed
     */
    void slotBookmarksChanged();

    /**
     * Used for @ref QAccel
     *
     * @see #initAccel
     */
    void slotKeyUp ();
    /**
     * Used for @ref QAccel
     *
     * @see #initAccel
     */
    void slotKeyDown ();
    /**
     * Used for @ref QAccel
     *
     * @see #initAccel
     */
    void slotPageUp ();
    /**
     * Used for @ref QAccel
     *
     * @see #initAccel
     */
    void slotPageDown ();       

    /**
     * Sets the windows title.
     */
    void slotTitle( const char *_title );
   
    /**
     * Toolbar "Home"
     */
    void slotHome();

    /**
     * Toolbar "Stop"
     */
    void slotStop();
    
    /**
     * Called if the panner changes its position.
     */
    void slotPannerChanged();

    /**
     * This slot is usually called if the user selects an URL in the tree view
     */
    void slotOpenURL( const char *_url );
    
    /**
     * Called if the user presses the right mouse button over the tree view.
     */
    void slotTreeViewPopupMenu( const char *_url, const QPoint &_point );

    /**
     * Sets a new text for the status bar.
     */
    void slotSetStatusBar( const char *_url );

    /**
     * If the user opens a new URL, this function is called to update
     * the history toolbar buttons.
     *
     * @see forwardStack
     * @see backStack
     * @see stackLock
     */
    void slotUpdateHistory( bool _back, bool _forward );

    /**
     * Call this slot if a @ref KHTMLView widget is waiting for net 
     * ressources. This will start an animated logo.
     * Call @ref #slotRemoveWaitingWidget if the widget got the stuff completely.
     * If no widget remains in the list, the animated logo is stopped.
     *
     * @see #waitingImageList
     */
    void slotAddWaitingWidget( KHTMLView * _widget );
    /**
     * @see #slotAddWaitingWidget
     */
    void slotRemoveWaitingWidget( KHTMLView * _waiting );

    /** 
     * Saves the current GUI settings
     */
    void slotSaveSettings();

protected slots:    
    void slotAnimatedLogoTimeout();

protected:

    /**
     * Closes the window and deletes it.
     */
    virtual void closeEvent( QCloseEvent *e );

    /**
     * Initializes the GUI.
     * Calls a list of functions that fo the job. So you can for
     * example dervice from this class and override 'initMenu' if you
     * want to have another menu.
     */
    virtual void initGUI();
    virtual void initToolBar();
    virtual void initMenu();
    virtual void initView();
    virtual void initStatusBar();
    virtual void initPanner();
    virtual void initTreeView();
    virtual void initAccel();
   
    /**
     * Fill the 'menu' with all boomarks in 'parent'.
     */
    void fillBookmarkMenu( KBookmark *parent, QPopupMenu *menu, int &id );
    /**
     * Manager for the bookmarks.
     */
    static KBookmarkManager bookmarkManager;
    /**
     * Menu containing the bookmarks.
     */
    QPopupMenu *bookmarkMenu;
    
    /**
     * List of all open windows.
     */
    static QList<KfmGui> windowList;
        
    /**
     * Contains the windows title.
     */
    QString title;
    
    /**
     * This flag tells us wether we should show .kde.html files.
     * When showing a directory that contains a file called .kde.html or index.html this
     * HTML file may be displayed instead of the normal icon view. This flag
     * tells us wether the user wants this or not. Default is TRUE.
     */
    bool bViewHTML;
    /**
     * Contains LongView, IconView, TextView.
     * With this variable you can switch the view mode.
     * The default is ICON_VIEW.
     */
    ViewMode viewMode;
    /**
     * This flag is set if we display the .* files, too.
     */
    bool showDot;

    /**
     * This flag is set, if kfm should work like a visual schnauzer.
     */
    bool visualSchnauzer;
    
    /**
     * The toolbar.
     */
    KToolBar *toolbar;
    KMenuBar *menu;

    KPanner *panner;
    QWidget *pannerChild0;
    QWidget *pannerChild1;

    KStatusBar *statusBar;

    QPopupMenu *mview;
    /**
     * The menu "New" in the "File" menu.
     * Since the items of this menu are not connected themselves
     * we need a pointer to this menu to get information about the
     * selected menu item.
     */
    QPopupMenu *menuNew;

    /**
     * The widget that contains the HTML stuff and the ( optional ) scrollbars.
     */
    KfmView *view;

    /**
     * Contains all URLs you can reach with the back button.
     */
    QStack<QString>backStack;
    /**
     * Contains all URLs you can reach with the forward button.
     */
    QStack<QString>forwardStack;
    /**
     * Lock 'backStack' and 'forwardStack'.
     * This is used by @ref #slotBack for example. When calling @ref #slotBack we dont
     * like the history stacks to be manipulated. In slotBack we call @ref KfmView::openURL.
     * This function in turn tries to tell us about the new URL but we dont want to modify
     * the history stack.
     */
    bool stackLock;

    /**
     * The path where the templates are stored.
     * This string always ends with "/".
     */
    QString templatePath;

    KFMTreeView *treeView;
    bool bTreeViewInitialized;
    bool bTreeView;

    /**
     * The Keyboard accelerator.
     */
    QAccel *accel;

    /**
     * List of all busy widgets.
     *
     * @see #slotAddWaitingWidget
     */
    QList<QWidget> waitingWidgetList;

    /**
     * Holds all images for the animated logo.
     * 
     * @see #slotAddWaitingWidget
     */
    static QList<QPixmap> animatedLogo;
    /**
     * The image from 0...animatedLogo.count()-1 we are currently
     * displaying.
     * 
     * @see #animatedLogo
     */
    uint animatedLogoCounter;
    /**
     * Times used to display the animated logo.
     *
     * @see #animatedLogo
     */
    QTimer *animatedLogoTimer;

    /**
     * List of all template files. It is important that they are in
     * the same order as the 'New' menu.
     */
    QStrList templatesList;
};

#endif

