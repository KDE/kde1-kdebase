#ifndef KFMGUI_H
#define KFMGUI_H

#define TOOLBAR_URL_ID 1000

class KfmGui;

#include <qwidget.h>
#include <qlabel.h>
#include <qlist.h>
#include <qdstream.h>
#include <kpanner.h>
#include <ktopwidget.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kmenubar.h>
#include <kapp.h>

#include "kfmview.h"
#include "bookmark.h"
#include "kioserver.h"
#include "kstrlist.h"

#include "kfmman.h"
#include "kfmtree.h"
#include "kURLcompletion.h"
#include "config-kfm.h"
#include "finddlg.h"

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
    enum ViewMode { ICON_VIEW, TEXT_VIEW, LONG_VIEW, SHORT_VIEW };    
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
    static QList<KfmGui>& getWindowList() { return (*windowList); }
    
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


    // session management
    void readProperties(int number)
    {
	KTopLevelWidget::readPropertiesInternal(kapp->getConfig(), number);
    }

    void saveProperties(int number)
    {
	KTopLevelWidget::savePropertiesInternal(kapp->getConfig(), number);
    }
    
    /**
     * Changes the URL displayed in the toolbar. This function is called
     * for example if we get a redirection message from kioslave.
     */
    void setToolbarURL( const char *_text );
    
    static bool sumode;
    static bool rooticons;
    
    /*
     * hack to get static classes up and running even with C++-Compilers/
     * Systems where a constructor of a class element declared static
     * would never get called (Aix, Alpha,...). In the next versions these
     * elements should disappear.
     */
    static void InitStatic() {
    	animatedLogo = new QList<QPixmap>;
	windowList = new QList<KfmGui>;
	bookmarkManager = new KBookmarkManager;
    }
    
    /**
     * Sets view charset
     * Called from kfmexec, when mimeType contains charset information
     */
    void setCharset(const char *_c);

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
    * Menu "Edit->SelectAll"
    */
    void slotSelectAll();
    /**
    * Menu "Edit->Find in page"
    */
    void slotFind();
    /**
    * Menu "Edit->Find next"
    */
    void slotFindNext();
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
     * Menu "Edit->Global Mime Types"
     */
    void slotEditSUMimeTypes();
    /**
     * Menu "Edit->Global Applications"
     */
    void slotEditSUApplications();
    /**
     * Menu "Edit->Mime Types"
     */
    void slotEditMimeTypes();
    /**
     * Menu "Edit->Applications"
     */
    void slotEditApplications();

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
     * Menu "View->Short View"
     */
    void slotShortView();
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
     * Menu "View->Document Source"
     */
    void slotViewDocumentSource();
    /**
     * Menu "View->Reload Tree"
     */
    void slotReloadTree();
    
    /**
     * Cache "View->Show History"
     */
    void slotShowHistory();
    /**
     * Cache "View->Show Cache"
     */
    void slotShowCache();
    /**
     * Cache "View->Clear Cache"
     */
    void slotClearCache();
    /**
     * Cache "View->Alway look in cache"
     */
    void slotCacheOn();
    /**
     * Cache "View->Never look in cache"
     */
    void slotCacheOff();
 
    /**
     * Cache "View->Always save cache"
     */
    void slotSaveCacheOn();
    /**
     * Cache "View->Never save cache"
     */
    void slotSaveCacheOff();
    
    /**
     * Menu "Bookmarks->Edit Bookmarks"
     */
    void slotEditBookmarks();
    /**
     * Bound to all menu items in "Bookmarks"
     */
    void slotBookmarkSelected( int id );
    /**
     * Called when the bookmarks changed
     */
    void slotBookmarksChanged();

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
     * Called when Find button in find dialog is pressed.
     */
    void slotFindNext( const QRegExp &regExp );
    
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
    void slotTreeUrlSelected( const char *, int );

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
     * This slot is called whenever a new URL is opened. This URL is
     * then appended to the history list.
     *
     * @see #historyList
     * @see #toolbarURL
     */
    void slotNewURL( const char *_url );
    
    /**
     * This slot is called whenever the user entered a new URL in the toolbar.
     *
     * @see #initToolbar
     */
    void slotURLEntered();
    
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

  /**
	* Configure the built-in browser
	*/
  void slotConfigureBrowser();

protected slots:    
    void slotAnimatedLogoTimeout();

    void slotShowToolbar();
    void slotShowStatusbar();
    void slotShowMenubar();
    void slotShowLocationBar();
    void slotTextSelected( KHTMLView *, bool );
    
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
   
    /**
     * Fill the 'menu' with all boomarks in 'parent'.
     */
    void fillBookmarkMenu( KBookmark *parent, QPopupMenu *menu );
    /**
     * Manager for the bookmarks.
     */
    static KBookmarkManager *bookmarkManager;
    /**
     * Menu containing the bookmarks.
     */
    QPopupMenu *bookmarkMenu;
    
    /**
     * List of all open windows.
     */
    static QList<KfmGui> *windowList;
        
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
    
    KMenuBar *menu;

    KPanner *panner;
    QWidget *pannerChild0;
    QWidget *pannerChild1;

    KStatusBar *statusBar;

    QPopupMenu *mview;
    QPopupMenu *mcache;
    QPopupMenu *moptions;
    
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
     * The Find text dialog
     */
    KFindTextDialog *findDialog;

    /**
     * The path where the templates are stored.
     * This string always ends with "/".
     */
    QString templatePath;

    KFMDirTree *treeView;
    bool bTreeViewInitialized;
    bool bTreeView;

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
    static QList<QPixmap> *animatedLogo;
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
    
    /**
     * The Toolbar that holds the history line.xb
     */
    KToolBar* toolbarURL;
    KToolBar* toolbarButtons;
    
    /**
     * This list contains all previuosly visited URLs.
     */
    QStrList historyList;

    /**
      * Completion Object, connected to URL-Line
      */
    KURLCompletion *completion;

    bool showToolbar;
    KToolBar::BarPosition toolbarPos;
    bool showStatusbar;
    KStatusBar::Position statusbarPos;
    bool showMenubar;
    KMenuBar::menuPosition menubarPos;
    bool showLocationBar;
    KToolBar::BarPosition locationBarPos;

    int kfmgui_height;
    int kfmgui_width;
};

#endif

