#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>

#include <qpopmenu.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qapp.h>
#include <qkeycode.h>
#include <qpushbt.h>
#include <qdir.h>
#include <qstrlist.h>
#include <qregexp.h>
#include <qmsgbox.h>
#include <qtooltip.h>
#include <qclipbrd.h>

#include <kconfig.h>
#include <kapp.h>

#include "kfmpaths.h"
#include "kfmgui.h"
#include "kfmdlg.h"
#include "kfmprops.h"
#include "kbutton.h"
#include "root.h"
#include "kbind.h"
#include "config-kfm.h"
#include "utils.h"
#include "kkfmoptdlg.h"
#include "kbookmarkdlg.h"
#include "kfm.h"

//-> was: #include "useragentdlg.h" => moved to kkfmoptdlg.h

#include <klocale.h>
#include <kstring.h>

bool KfmGui::sumode = false;
bool KfmGui::rooticons = true;

char* kfm_getrev();

char* kfm_getrev()
{
    static char *revision = "$Revision$";
    static char *rev;
    static char *p;

    static bool flag = false;
    if ( flag )
	return rev;
    flag = true;

    char *dummy = strdup( revision );
    if ((p = strchr(dummy, ':'))) {
	rev = p + 2;
	p = strchr(rev, '$');
	*--p = 0;
    } else
	rev = "???";
    return rev;
}

KBookmarkManager *KfmGui::bookmarkManager;
QList<KfmGui> *KfmGui::windowList;
QList<QPixmap> *KfmGui::animatedLogo;

KfmGui::KfmGui( QWidget *, const char *name, const char * _url)
    : KTopLevelWidget( name )
{
    toolbarButtons = 0L;
    toolbarURL = 0L;
    
    // Timer used for animated logo
    animatedLogoTimer = new QTimer( this );
    animatedLogoCounter = 0;
    connect( animatedLogoTimer, SIGNAL( timeout() ),
	     this, SLOT( slotAnimatedLogoTimeout() ) );
    
    /* read the settings. We have some defaults, if the 
       settings are not defined or if there are unknown
       words in it. */

    KConfig *config = kapp->getConfig();
    config->setGroup( "Settings" );
    
    kfmgui_width = config->readNumEntry("kfmgui_width",  KFMGUI_WIDTH);
    kfmgui_height = config->readNumEntry("kfmgui_height",KFMGUI_HEIGHT);

    QString entry;

    entry = "Off"; // default
    if ( config->readEntry("TreeView", entry) == entry)
      bTreeView = false;
    else
      bTreeView = true;
    bTreeViewInitialized = false; // this is alway true
    
    entry = "IconView"; // default
    viewMode = ICON_VIEW;
    entry = config->readEntry("ViewMode", entry);
    if (entry == "LongView")
      viewMode = LONG_VIEW;
    if (entry == "TextView")
      viewMode = TEXT_VIEW;
    if (entry == "ShortView")
      viewMode = SHORT_VIEW;

    entry = "no";
    if ( config->readEntry("ShowDotFiles",entry) == "yes")
      showDot = true;
    else
      showDot = false;
    
    entry = "Off";
    if ( config->readEntry("VisualSchnauzer", entry) == "Off")
      visualSchnauzer = false;
    else
      visualSchnauzer = true;
    
    entry = "yes";
    if ( config->readEntry("HTMLView", entry) == entry)
      bViewHTML = true;
    else
      bViewHTML = false;

    entry = config->readEntry("Toolbar", "top");
    showToolbar = true;
    if ( entry == "top" )
	toolbarPos = KToolBar::Top;
    else if ( entry == "left" )
	toolbarPos = KToolBar::Left;
    else if ( entry == "right" )
	toolbarPos = KToolBar::Right;    
    else if ( entry == "bottom" )
	toolbarPos = KToolBar::Bottom;    
    else if ( entry == "floating" )
	toolbarPos = KToolBar::Floating;    
    else
	showToolbar = false;

    entry = config->readEntry("LocationBar", "top");
    showLocationBar = true;
    if ( entry == "top" )
	locationBarPos = KToolBar::Top;
    else if ( entry == "bottom" )
	locationBarPos = KToolBar::Bottom;    
    else if ( entry == "floating" )
	locationBarPos = KToolBar::Floating;    
    else
	showLocationBar = false;

    entry = config->readEntry("Menubar", "top");
    showMenubar = true;
    if ( entry == "top" )
	menubarPos = KMenuBar::Top;
    else if ( entry == "bottom" )
	menubarPos = KMenuBar::Bottom;    
    else if ( entry == "floating" )
	menubarPos = KMenuBar::Floating;    
    else
	showMenubar = false;

    entry = config->readEntry("Statusbar", "top");
    showStatusbar = true;
    if ( entry == "top" )
	statusbarPos = KStatusBar::Top;
    else if ( entry == "bottom" )
	statusbarPos = KStatusBar::Bottom;    
    else if ( entry == "floating" )
	statusbarPos = KStatusBar::Floating;    
    else
	showStatusbar = false;

    config->setGroup( "Cache" );
    bool on;
    on=config->readBoolEntry( "CacheEnabled", true );
    HTMLCache::enableCache(on);
    on=config->readBoolEntry( "SaveCacheEnabled", true );
    HTMLCache::enableSaveCache(on);
  
    initGUI();

    windowList->setAutoDelete( false );
    windowList->append( this );

    if ( _url )
	view->openURL( _url );
    this->resize(kfmgui_width,kfmgui_height);
}

KfmGui* KfmGui::findWindow( const char *_url )
{
    KfmGui *w;
    for ( w = windowList->first(); w != 0L; w = windowList->next() )
	if ( strcmp( _url, w->getURL() ) == 0 )
	    return w;
    
    return 0L;
}

void KfmGui::initGUI()
{
    setMinimumSize( 100, 100 );

    initPanner();
    initTreeView();
    initStatusBar();
    initView();
    initMenu();
    initToolBar();
    QGridLayout *gl = new QGridLayout( pannerChild0, 1, 1 );
    gl->addWidget( treeView, 0, 0 );
    gl = new QGridLayout( pannerChild1, 1, 1 );
    gl->addWidget( view, 0, 0 );
}

void KfmGui::initPanner()
{
    if ( bTreeView )
	panner = new KPanner( this, "_panner", KPanner::O_VERTICAL, 30 );
    else
	panner = new KPanner( this, "_panner", KPanner::O_VERTICAL, 0 );
    
    pannerChild0 = panner->child0();
    pannerChild1 = panner->child1();    
    connect( panner, SIGNAL( positionChanged() ), this, SLOT( slotPannerChanged() ) );

    setView( panner );
    panner->show();
}

void KfmGui::initTreeView()
{
    treeView = new KFMDirTree( pannerChild0, this );

    connect( treeView, SIGNAL( urlSelected( const char *, int ) ),
	     this, SLOT( slotTreeUrlSelected( const char *, int ) ) );

    if ( bTreeView )
    {
	bTreeViewInitialized = true;
	treeView->fill();  
    }

    treeView->show();
}

void KfmGui::initStatusBar()
{
    statusBar = new KStatusBar( this );
 
    statusBar->insertItem( (char*)klocale->translate("KFM"), 1 );
    
    statusBar->show();
    setStatusBar( statusBar );
    if ( !showStatusbar )
	statusBar->enable( KStatusBar::Hide );
}

void KfmGui::initMenu()
{
    menuNew = new QPopupMenu;
    CHECK_PTR( menuNew );
    menuNew->insertItem( klocale->translate("Folder") );

    connect( menuNew, SIGNAL( activated( int ) ), 
	     this, SLOT( slotNewFile( int ) ) );

    templatesList.clear();
    
    templatesList.append( QString( "Folder") );
    QDir d( KFMPaths::TemplatesPath() );
    const QFileInfoList *list = d.entryInfoList();
    if ( list == 0L )
        warning(klocale->translate("ERROR: Template does not exist '%s'"),
		KFMPaths::TemplatesPath().data());
    else
    {
	QFileInfoListIterator it( *list );      // create list iterator
	QFileInfo *fi;                          // pointer for traversing

	while ( ( fi = it.current() ) != 0L )
	{
	    if ( strcmp( fi->fileName().data(), "." ) != 0 && 
		 strcmp( fi->fileName().data(), ".." ) != 0 )
	    {
		QString tmp = fi->fileName().data();
		templatesList.append( tmp );
		if ( tmp.right(7) == ".kdelnk" )
		    tmp.truncate( tmp.length() - 7 );
		menuNew->insertItem( tmp );
	    }
	    ++it;                               // goto next list element
	}
    }

    QPopupMenu *file = new QPopupMenu;
    CHECK_PTR( file );
    file->insertItem( klocale->translate("&New"), menuNew );
    file->insertSeparator();
    file->insertItem( klocale->translate("&New Window"), 
		      this, SLOT(slotNewWindow()) );
    file->insertSeparator();
    file->insertItem( klocale->translate("&Run..."), 
		      this, SLOT(slotRun()) );
    file->insertItem( klocale->translate("Open &Terminal"), 
		      this, SLOT(slotTerminal()), CTRL+Key_T );
    file->insertSeparator();
    file->insertItem( klocale->translate("&Open Location..."),
		      this, SLOT(slotOpenLocation()), CTRL+Key_L );
    file->insertItem( klocale->translate("&Find"), this, 
		      SLOT(slotToolFind()), CTRL+Key_F );
    file->insertSeparator();
    file->insertItem( klocale->translate("&Print..."), 
		      this, SLOT(slotPrint()) );
    file->insertSeparator();        
    file->insertItem( klocale->translate("&Close"), 
		      this, SLOT(slotClose()), CTRL+Key_W );
    file->insertItem( klocale->translate("&Quit..."),  
		      this, SLOT(slotQuit()), CTRL+Key_Q );

    QPopupMenu *edit = new QPopupMenu;
    CHECK_PTR( edit );
    edit->insertItem( klocale->translate("&Copy"), this, 
		      SLOT(slotCopy()), CTRL+Key_C );
    edit->insertItem( klocale->translate("&Paste"), this, 
		      SLOT(slotPaste()), CTRL+Key_V );
    edit->insertItem( klocale->translate("&Move to Trash"), 
		      this, SLOT(slotTrash()) );
    edit->insertItem( klocale->translate("&Delete"), this, 
		      SLOT(slotDelete()) );
    edit->insertSeparator();
    edit->insertItem( klocale->translate("&Select"), this, 
		      SLOT(slotSelect()), CTRL+Key_S );

	edit->insertSeparator();
    edit->insertItem( klocale->translate("Mime Types"), this, 
		      SLOT(slotEditMimeTypes()) );
    edit->insertItem( klocale->translate("Applications"), this, 
		      SLOT(slotEditApplications()) );

    if ( sumode )
    {
	edit->insertSeparator();
	edit->insertItem( klocale->translate("Global Mime Types"), this, 
		      SLOT(slotEditSUMimeTypes()) );
	edit->insertItem( klocale->translate("Global Applications"), this, 
		      SLOT(slotEditSUApplications()) );
    }

    mview = new QPopupMenu;
    CHECK_PTR( mview );
    mview->setCheckable(true);
    mview->insertItem( klocale->translate("Show &Dot Files"),
		       this, SLOT(slotShowDot()) );
    mview->insertItem( klocale->translate("Show Tr&ee"), 
		       this, SLOT(slotShowTreeView()) );
    mview->insertItem( klocale->translate("&Visual Schnauzer"),
		       this, SLOT(slotShowSchnauzer()) );
    mview->insertItem( klocale->translate("&HTML View"),
		       this, SLOT(slotViewHTML()) );
    mview->insertSeparator();
    mview->insertItem( klocale->translate("&Icon View"),
		       this, SLOT(slotIconView()) );
    mview->insertItem( klocale->translate("&Text View"),
		       this, SLOT(slotTextView()) );
    mview->insertItem( klocale->translate("&Long View"),
		       this, SLOT(slotLongView()) );
    mview->insertItem( klocale->translate("&Short View"),
		       this, SLOT(slotShortView()) );
    /* mview->insertSeparator();
    mview->insertItem( klocale->translate("Split &window"),
		       this, SLOT(slotSplitWindow()) ); */
    mview->insertSeparator();
    mview->insertItem( klocale->translate("Rel&oad Tree"),
		       this, SLOT(slotReloadTree()) );
    mview->insertItem( klocale->translate("&Reload Document"),
		       view, SLOT(slotReload()) );
    mview->insertItem( klocale->translate("Rescan &bindings"),
		       this, SLOT(slotRescanBindings()) );
    mview->insertSeparator();
    mview->insertItem( klocale->translate("&View Frame Source"),
		       this, SLOT(slotViewFrameSource()) );
    mview->insertItem( klocale->translate("View Docu&ment Source"),
		       this, SLOT(slotViewDocumentSource()) );
    
    mview->setItemChecked( mview->idAt( 0 ), showDot );
    mview->setItemChecked( mview->idAt( 1 ), bTreeView );
    mview->setItemChecked( mview->idAt( 2 ), visualSchnauzer );
    mview->setItemChecked( mview->idAt( 3 ), bViewHTML );

    mcache = new QPopupMenu;
    CHECK_PTR( mcache );
    mcache->setCheckable(true);
    mcache->insertItem( klocale->translate( "Show History" ),
			this, SLOT( slotShowHistory() ) );
    mcache->insertItem( klocale->translate( "Show Cache" ),
			this, SLOT( slotShowCache() ) );
    mcache->insertItem( klocale->translate( "Clear Cache" ),
			this, SLOT( slotClearCache() ) );
    mcache->insertSeparator();
    mcache->insertItem( klocale->translate( "Always look in cache" ),
			this, SLOT( slotCacheOn() ) );
    mcache->insertItem( klocale->translate( "Never look in cache" ),
			this, SLOT( slotCacheOff() ) );
    mcache->insertItem( klocale->translate( "Always save cache" ),
			this, SLOT( slotSaveCacheOn() ) );
    mcache->insertItem( klocale->translate( "Never save cache" ),
			this, SLOT( slotSaveCacheOff() ) );
    
    mcache->setItemChecked( mcache->idAt( 4 ), HTMLCache::isEnabled() );
    mcache->setItemChecked( mcache->idAt( 5 ), !HTMLCache::isEnabled() );
    mcache->setItemChecked( mcache->idAt( 6 ), HTMLCache::isSaveEnabled() );
    mcache->setItemChecked( mcache->idAt( 7 ), !HTMLCache::isSaveEnabled() );

    moptions = new QPopupMenu;
    CHECK_PTR( moptions );
    moptions->setCheckable(true);
    moptions->insertItem( klocale->translate("Show &Menubar"),
			  this, SLOT(slotShowMenubar()) );
    moptions->insertItem( klocale->translate("Show &Statusbar"),
			  this, SLOT(slotShowStatusbar()) );
    moptions->insertItem( klocale->translate("Show &Toolbar"),
			  this, SLOT(slotShowToolbar()) );
    moptions->insertItem( klocale->translate("Show &Location bar"),
			  this, SLOT(slotShowLocationBar()) );
    moptions->insertSeparator();
    moptions->insertItem( klocale->translate("Sa&ve Settings"),
			  this, SLOT(slotSaveSettings()) );
	moptions->insertItem( klocale->translate("Configure Browser..."),
						  this, SLOT(slotConfigureBrowser()));

    moptions->setItemChecked( moptions->idAt( 0 ), showMenubar );
    moptions->setItemChecked( moptions->idAt( 1 ), showStatusbar );
    moptions->setItemChecked( moptions->idAt( 2 ), showToolbar );
    moptions->setItemChecked( moptions->idAt( 3 ), showLocationBar );

    switch (viewMode) 
      {
      case ICON_VIEW:
	mview->setItemChecked( mview->idAt( 5 ), true );
	break;
      case TEXT_VIEW:
	mview->setItemChecked( mview->idAt( 6 ), true );
	break;
      case LONG_VIEW:
	mview->setItemChecked( mview->idAt( 7 ), true );
	break;
      case SHORT_VIEW:
	mview->setItemChecked( mview->idAt( 8 ), true );
	break;
      }

    QPopupMenu *nav = new QPopupMenu;
    CHECK_PTR( nav );

    bookmarkMenu = new QPopupMenu;
    CHECK_PTR( bookmarkMenu );
    connect( bookmarkManager, SIGNAL( changed() ), 
	     this, SLOT( slotBookmarksChanged() ) );

    // bookmarkMenu->insertItem( klocale->translate("&Edit Bookmarks..."), 
    // this, SLOT(slotEditBookmarks()) );
    // fillBookmarkMenu( bookmarkManager->root(), bookmarkMenu );
    slotBookmarksChanged();
    
    QPopupMenu *help = new QPopupMenu;
    CHECK_PTR( help );
    // help->insertItem( "About &Qt...", this, SLOT(slotAboutQt()) );
    help->insertItem( klocale->translate("&Help"), 
		      this, SLOT(slotHelp()) );
    help->insertSeparator();
    help->insertItem( klocale->translate("&About..."), this, SLOT(slotAbout()) );

    menu = new KMenuBar( this );
    if ( sumode )
	menu->setBackgroundColor( red );
    
    CHECK_PTR( menu );
    menu->insertItem( klocale->translate("&File"), file );
    menu->insertItem( klocale->translate("&Edit"), edit );
    menu->insertItem( klocale->translate("&View"), mview );
    menu->insertItem( klocale->translate("&Bookmarks"), bookmarkMenu );
    menu->insertItem( klocale->translate("&Cache"), mcache );
    menu->insertItem( klocale->translate("&Options"), moptions );
    menu->insertSeparator();
    menu->insertItem( klocale->translate("&Help"), help );
    menu->show();
    
    setMenu( menu );
}

void KfmGui::enableToolbarButton( int id, bool enable )
{
    if ( toolbarButtons == 0L )
	return;
    
    toolbarButtons->setItemEnabled( id, enable );
}

void KfmGui::initToolBar()
{
    QString file, path;
    QPixmap pixmap;
    toolbarButtons = new KToolBar(this, "kfmwin-toolbar");
    path = kapp->kde_toolbardir() + "/";

    pixmap.load(path + "up.xpm");
    toolbarButtons->insertButton(pixmap, 0, SIGNAL( clicked() ), view, 
			  SLOT( slotUp() ), false, 
			  klocale->translate("Up"));
    
    pixmap.load(path + "back.xpm");
    toolbarButtons->insertButton(pixmap, 1, SIGNAL( clicked() ), view, 
			  SLOT( slotBack() ), false, 
			  klocale->translate("Back"));
    
    pixmap.load(path + "forward.xpm");
    toolbarButtons->insertButton(pixmap, 2, SIGNAL( clicked() ), view, 
			  SLOT( slotForward() ), false, 
			  klocale->translate("Forward"));
    
    pixmap.load(path + "home.xpm");
    toolbarButtons->insertButton(pixmap, 3, SIGNAL( clicked() ), this, 
			  SLOT( slotHome() ), true, 
			  klocale->translate("Home") );
    
    toolbarButtons->insertSeparator();
    
    pixmap.load(path + "reload.xpm");
    toolbarButtons->insertButton(pixmap, 4, SIGNAL( clicked() ), view, 
			  SLOT( slotReload() ), true, 
			  klocale->translate("Reload") );

    toolbarButtons->insertSeparator();
    
    pixmap.load(path + "editcopy.xpm");
    toolbarButtons->insertButton(pixmap, 5, SIGNAL( clicked() ), this, 
			  SLOT( slotCopy() ), true, 
			  klocale->translate("Copy") );
    
    pixmap.load(path + "editpaste.xpm");
    toolbarButtons->insertButton(pixmap, 6, SIGNAL( clicked() ), this, 
			  SLOT( slotPaste() ), true, 
			  klocale->translate("Paste") );
    
    toolbarButtons->insertSeparator();
    
    pixmap.load(path + "help.xpm");
    toolbarButtons->insertButton(pixmap, 7, SIGNAL( clicked() ), this, 
			  SLOT( slotHelp() ), true, 
			  klocale->translate("Help"));
    
    toolbarButtons->insertSeparator();
    
    pixmap.load(path + "stop.xpm");
    toolbarButtons->insertButton(pixmap, 8, SIGNAL( clicked() ), this, 
			  SLOT( slotStop() ), false, 
			  klocale->translate("Stop"));

    path = kapp->kde_datadir() + "/kfm/pics/";

    pixmap.load( path + "/kde1.xpm" );
    
    toolbarButtons->insertButton(pixmap, 9, SIGNAL( clicked() ), this, 
			  SLOT( slotNewWindow() ), false );
    toolbarButtons->setItemEnabled( 9, true );
    toolbarButtons->alignItemRight( 9, true );
    
    // Load animated logo
    if ( animatedLogo->count() == 0 )
    {
	animatedLogo->setAutoDelete( true );
	for ( int i = 1; i <= 9; i++ )
	{
	    QString n;
	    n.sprintf( "/kde%i.xpm", i );
	    QPixmap *p = new QPixmap();
	    p->load( path + n );
	    if ( p->isNull() )
	    {
		QString e;
		e << klocale->translate( "Could not load icon\n" ) << n.data();
		QMessageBox::warning( this, klocale->translate( "KFM Error" ), e.data() );
	    }
	    animatedLogo->append( p );
	}
    }
	    
    addToolBar( toolbarButtons );
    toolbarButtons->setBarPos( toolbarPos );
    toolbarButtons->show();                
    if ( !showToolbar )
	toolbarButtons->enable( KToolBar::Hide );

    toolbarURL = new KToolBar(this, "URL History");
    toolbarURL->insertLined( "", TOOLBAR_URL_ID,
			  SIGNAL( returnPressed() ), this, SLOT( slotURLEntered() ) );
    KLined *lined = toolbarURL->getLined (TOOLBAR_URL_ID); //Sven
    completion = new KURLCompletion();
    connect ( lined, SIGNAL (completion()),
	      completion, SLOT (make_completion()));
    connect ( lined, SIGNAL (rotation()),
	      completion, SLOT (make_rotation()));
    connect ( lined, SIGNAL (textChanged(const char *)),
	      completion, SLOT (edited(const char *)));
    connect ( completion, SIGNAL (setText (const char *)),
	      lined, SLOT (setText (const char *)));
    addToolBar( toolbarURL );
    toolbarURL->setFullWidth( TRUE );
    toolbarURL->setItemAutoSized( TOOLBAR_URL_ID, TRUE );
    toolbarURL->setBarPos( locationBarPos );
    toolbarURL->show();                
    if ( !showLocationBar )
	toolbarURL->enable( KToolBar::Hide );
}

void KfmGui::initView()
{
    view = new KfmView( this, pannerChild1, "" );
    CHECK_PTR( view );
    
    connect( view, SIGNAL( setTitle(const char *)), this, SLOT( slotTitle(const char *)) );
    connect( view, SIGNAL( historyUpdate( bool, bool ) ), this, SLOT( slotUpdateHistory( bool, bool ) ) );
    connect( view, SIGNAL( newURL( const char * ) ), this, SLOT( slotNewURL( const char * ) ) );
    connect( view, SIGNAL( documentStarted( KHTMLView * ) ), 
	     this, SLOT( slotAddWaitingWidget( KHTMLView * ) ) );
    connect( view, SIGNAL( documentDone( KHTMLView * ) ), 
	     this, SLOT( slotRemoveWaitingWidget( KHTMLView * ) ) );
    connect( view, SIGNAL( textSelected( KHTMLView *, bool ) ), 
	     this, SLOT( slotTextSelected( KHTMLView *, bool ) ) );
    view->show();
    view->setFocus();
}

void KfmGui::closeEvent( QCloseEvent *e )
{
    e->accept();

    delete this;
}

void KfmGui::updateView()
{
    view->slotUpdateView();
}

void KfmGui::slotReloadTree()
{
    if ( bTreeViewInitialized )
	treeView->update();
}

void KfmGui::slotShowCache()
{
  HTMLCache::save();
  
  QString s = KFMPaths::CachePath().data();
  s += "/index.html";
  view->openURL( s );
}

void KfmGui::slotShowHistory()
{
  QString s = KFMPaths::CachePath().data();
  s += "/history.html";
  KFM::saveHTMLHistory( s );

  QString tmp( "file:" );
  tmp += s;
  view->openURL( tmp );
}

void KfmGui::slotClearCache()
{
  HTMLCache::clear();
}

void KfmGui::slotCacheOn()
{
    mcache->setItemChecked( mcache->idAt( 4 ), true );
    mcache->setItemChecked( mcache->idAt( 5 ), false );

    HTMLCache::enableCache( true );
}

void KfmGui::slotCacheOff()
{
    mcache->setItemChecked( mcache->idAt( 4 ), false );
    mcache->setItemChecked( mcache->idAt( 5 ), true );

    HTMLCache::enableCache( false );
}

void KfmGui::slotSaveCacheOn()
{
    mcache->setItemChecked( mcache->idAt( 6 ), true );
    mcache->setItemChecked( mcache->idAt( 7 ), false );

    HTMLCache::enableSaveCache( true );
}

void KfmGui::slotSaveCacheOff()
{
    mcache->setItemChecked( mcache->idAt( 6 ), false );
    mcache->setItemChecked( mcache->idAt( 7 ), true );

    HTMLCache::enableSaveCache( false );
}

void KfmGui::slotURLEntered()
{
    if ( view->getActiveView() )
    {
	QString url = toolbarURL->getLinedText( TOOLBAR_URL_ID );

	// Exit if the user did not enter an URL
	if ( url.data()[0] == 0 )
	    return;
	// Root directory?
	if ( url.data()[0] == '/' )
	{
	  KURL u( toolbarURL->getLinedText( TOOLBAR_URL_ID ) );
	  url = u.url().data();
	}
	// Home directory?
        else if ( url.data()[0] == '~' )
        {
	  QString tmp( QDir::homeDirPath().data() );
	  tmp += toolbarURL->getLinedText( TOOLBAR_URL_ID ) + 1;
	  KURL u( tmp );
	  url = u.url().data();
	}

	KURL u( url.data() );
	if ( u.isMalformed() )
	{
	    QString tmp;
	    tmp << klocale->translate("Malformed URL\n") << toolbarURL->getLinedText( TOOLBAR_URL_ID );
	    QMessageBox::critical( (QWidget*)0L, klocale->translate( "KFM Error" ),
				   tmp );
	    return;
	}
	
	view->openURL( url.data() );             
    }
    // view->openURL( toolbarURL->getLinedText( TOOLBAR_URL_ID ) );
}

void KfmGui::setToolbarURL( const char *_url )
{
    toolbarURL->setLinedText( TOOLBAR_URL_ID, _url );
}

void KfmGui::slotNewURL( const char *_url )
{
    toolbarURL->setLinedText( TOOLBAR_URL_ID, _url );
    
    if ( historyList.find( _url ) == -1 )
	return;
    
    historyList.insert( 0, _url );
    /* toolbarURL->clearCombo( TOOLBAR_URL_ID );
    toolbarURL->insertComboList( TOOLBAR_URL_ID, &historyList, 0 ); */
}

void KfmGui::slotRescanBindings()
{
    KMimeType::clearAll();
    KMimeType::init();
    if ( KRootWidget::getKRootWidget() )
	KRootWidget::getKRootWidget()->update();

    KfmGui *win;
    for ( win = windowList->first(); win != 0L; win = windowList->next() )
	win->updateView();
}

void KfmGui::slotRun()
{
    if ( view->getActiveView() )
	view->getActiveView()->slotRun();
}

void KfmGui::slotTerminal()
{
    if ( view->getActiveView() )
	view->getActiveView()->slotTerminal();
}

void KfmGui::slotOpenURL( const char *_url )
{
    if ( view->getActiveView() )
	view->getActiveView()->openURL( _url );
}

void KfmGui::slotEditSUMimeTypes()
{
    QString tmp = KApplication::kde_mimedir().copy();
    view->openURL( tmp );
}

void KfmGui::slotEditSUApplications()
{
    QString tmp = KApplication::kde_appsdir().copy();
    view->openURL( tmp );
}

void KfmGui::slotEditMimeTypes()
{
    QString tmp = KApplication::localkdedir();
    tmp += "/share/mimelnk";
    view->openURL( tmp );
}

void KfmGui::slotEditApplications()
{
    QString tmp = KApplication::localkdedir();
    tmp += "/share/applnk";
    view->openURL( tmp );
}

void KfmGui::slotPrint()
{
    if ( view->getActiveView() )
	view->getActiveView()->print();
}   

void KfmGui::slotPannerChanged()
{
    if ( !bTreeViewInitialized )
    {
        bTreeViewInitialized = TRUE;
	treeView->fill();
    }

    if ( panner->getSeparator() == 0 )
      mview->setItemChecked( mview->idAt( 1 ), false );
    else
      mview->setItemChecked( mview->idAt( 1 ), true );
    
    resizeEvent( 0L );
}

void KfmGui::slotSplitWindow()
{
    view->getActiveView()->splitWindow();
}

void KfmGui::slotViewHTML( )
{
    bViewHTML = !mview->isItemChecked( mview->idAt(3) );
    mview->setItemChecked( mview->idAt( 3 ), bViewHTML);
    view->slotUpdateView();
}

void KfmGui::slotIconView()
{
    viewMode = ICON_VIEW;
    mview->setItemChecked( mview->idAt( 5 ), true );
    mview->setItemChecked( mview->idAt( 6 ), false );
    mview->setItemChecked( mview->idAt( 7 ), false );
    mview->setItemChecked( mview->idAt( 8 ), false );
    view->slotUpdateView( false );
}

void KfmGui::slotLongView()
{
    viewMode = LONG_VIEW;
    mview->setItemChecked( mview->idAt( 5 ), false );
    mview->setItemChecked( mview->idAt( 6 ), false);
    mview->setItemChecked( mview->idAt( 7 ), true );
    mview->setItemChecked( mview->idAt( 8 ), false );
    view->slotUpdateView( false );
}

void KfmGui::slotTextView()
{
    viewMode = TEXT_VIEW;
    mview->setItemChecked( mview->idAt( 5 ), false );
    mview->setItemChecked( mview->idAt( 6 ), true );
    mview->setItemChecked( mview->idAt( 7 ), false );
    mview->setItemChecked( mview->idAt( 8 ), false );
    view->slotUpdateView( false );
}

void KfmGui::slotShortView()
{
    viewMode = SHORT_VIEW;
    mview->setItemChecked( mview->idAt( 5 ), false );
    mview->setItemChecked( mview->idAt( 6 ), false );
    mview->setItemChecked( mview->idAt( 7 ), false );
    mview->setItemChecked( mview->idAt( 8 ), true );
    view->slotUpdateView( false );
}

void KfmGui::slotSelect()
{
    DlgLineEntry l( klocale->translate("Select files:"), "", this );
    if ( l.exec() )
    {
	QString pattern = l.getText();
	if ( pattern.length() == 0 )
	    return;

	QRegExp re( pattern, true, true );
	
	view->getActiveView()->select( re, true );
    }
}

void KfmGui::slotBookmarksChanged()
{
    bookmarkMenu->clear();
    bookmarkMenu->disconnect( this );
    bookmarkMenu->insertItem( klocale->translate("&Edit Bookmarks..."), 
			      this, SLOT(slotEditBookmarks()) );

    fillBookmarkMenu( bookmarkManager->root(), bookmarkMenu );
}

void KfmGui::fillBookmarkMenu( KBookmark *parent, QPopupMenu *menu )
{
    KBookmark *bm;
    
    menu->insertItem( i18n("&Add Bookmark"), parent->id() );
    menu->insertSeparator();

    connect( menu, SIGNAL( activated( int ) ),
	     SLOT( slotBookmarkSelected( int ) ) );
    
    for ( bm = parent->children()->first(); bm != NULL;
	  bm = parent->children()->next() )
    {
	if ( bm->type() == KBookmark::URL )
	{
	    menu->insertItem( *(bm->miniPixmap()), bm->text(), bm->id() );
	}
	else
	{	    
	    QPopupMenu *subMenu = new QPopupMenu;
	    menu->insertItem( *(bm->miniPixmap()), bm->text(), subMenu );
	    fillBookmarkMenu( bm, subMenu );
	}
    }
}

void KfmGui::slotNewFile( int _id )
{
    if ( menuNew->text( _id ) == 0)
	return;
    
    // QString p =  menuNew->text( _id );    
    QString p = templatesList.at( _id );
    
    QString text = klocale->translate("New ");
    text += p.data();
    text += ":";
    const char *value = p.data();

    if ( strcmp( p.data(), "Folder" ) == 0 ) {
	value = "";
	text = klocale->translate("New ");
	text += klocale->translate("Folder");
	text += ":";
    }
    
    DlgLineEntry l( text.data(), value, this );
    if ( l.exec() )
    {
	QString name = l.getText();
	if ( name.length() == 0 )
	    return;
	
	if ( strcmp( p.data(), "Folder" ) == 0 )
	{
	    KIOJob * job = new KIOJob;
	    QString u = view->getURL();
	    u.detach();
	    if ( u.right( 1 ) != "/" )
		u += "/";
	    u += name.data();
	    job->mkdir( u.data() );
	}
	else
	{
	    KIOJob * job = new KIOJob;
	    QString src = KFMPaths::TemplatesPath() + p.data();
	    QString dest = view->getURL();
	    dest.detach();
	    if ( dest.right( 1 ) != "/" )
	        dest += "/";
	    dest += name.data();
	    // debugT("Command copy '%s' '%s'\n",src.data(),dest.data());
	    job->copy( src.data(), dest.data() );
	}
    }
}

void KfmGui::slotStop()
{
    view->slotStop();
}

void KfmGui::slotUpdateHistory( bool _back, bool _forward )
{
    enableToolbarButton( 1, _back );
    enableToolbarButton( 2, _forward );
}

void KfmGui::slotHome()
{  
    QString url = "file:";
    url += QDir::homeDirPath().data();
    
    view->openURL( url.data() );
}

void KfmGui::addBookmark( const char *_title, const char *_url )
{
    QString p = getenv( "HOME" );
    QString bmFile = p + "/.kde/share/apps/kfm/bookmarks";

    KBookmark *root = bookmarkManager->root();
   
    (void)new KBookmark( bookmarkManager, root, _title, _url );
}

void KfmGui::slotEditBookmarks()
{
  QString p = getenv( "HOME" );
  p += "/.kde/share/apps/kfm/bookmarks";

  KfmGui *m = new KfmGui( 0L, 0L, p );
  m->show();
}

void KfmGui::slotBookmarkSelected( int _id )
{
    KBookmark *bm = bookmarkManager->findBookmark( _id );
    
    if ( bm )
    {
      if ( bm->type() == KBookmark::Folder )
      {
	(void)new KBookmark( bookmarkManager, bm, title.data(), view->getURL() );
	return;
      }

      KURL u( bm->url() );
      if ( u.isMalformed() )
      {
	warning(klocale->translate("ERROR: Malformed URL"));
	return;
      }
	
      view->openURL( bm->url() );
    }
    else
      warning("Internal: Could not find bookmark id\n");
}

void KfmGui::slotToolFind( )
{
    QString cmd = "kfind &";
    system( cmd.data() );
}

void KfmGui::slotNewWindow( )
{
    KfmGui *f;

    if ( view->getURL() )
	f = new KfmGui( 0L, 0L, view->getURL() );
    else
    {
	QString url;
	url.sprintf( "file:%s", getenv( "HOME" ) );
	f = new KfmGui( 0L, 0L, url );
    }
    
    f->show();
}

void KfmGui::slotShowMenubar()
{
    showMenubar = !showMenubar;
    moptions->setItemChecked( moptions->idAt( 0 ), showMenubar );
    if (showMenubar)
	menu->show();
    else
	menu->hide();
    updateRects();
    // TODO: does not work yet
}

void KfmGui::slotShowToolbar()
{
    showToolbar = !showToolbar;
    moptions->setItemChecked( moptions->idAt( 2 ), showToolbar );
    if ( !showToolbar )
	toolbarButtons->enable( KToolBar::Hide );
    else
	toolbarButtons->enable( KToolBar::Show );
    resizeEvent( 0L );
}

void KfmGui::slotShowLocationBar()
{
    showLocationBar = !showLocationBar;
    moptions->setItemChecked( moptions->idAt( 3 ), showLocationBar );
    if ( !showLocationBar )
	toolbarURL->enable( KToolBar::Hide );
    else
	toolbarURL->enable( KToolBar::Show );
    resizeEvent( 0L );
}

void KfmGui::slotShowStatusbar()
{
    showStatusbar = !showStatusbar;
    moptions->setItemChecked( moptions->idAt( 1 ), showStatusbar );
    if ( !showStatusbar )
	statusBar->enable( KStatusBar::Hide );
    else
	statusBar->enable( KStatusBar::Show );
    resizeEvent( 0L );
}

void KfmGui::slotShowDot()
{
    showDot = !showDot;
    mview->setItemChecked( mview->idAt( 0 ), showDot );
    view->slotUpdateView();
    if ( bTreeViewInitialized )
	treeView->update();
}

void KfmGui::slotShowSchnauzer()
{
    visualSchnauzer = !visualSchnauzer;
    mview->setItemChecked( mview->idAt( 2 ), visualSchnauzer );
    view->slotUpdateView();
}

void KfmGui::slotShowTreeView()
{
    bTreeView = !bTreeView;
    if ( !bTreeViewInitialized )
    {
	bTreeViewInitialized = true;
	treeView->fill();  
    }
    
    mview->setItemChecked( mview->idAt( 1 ), bTreeView );

    if ( bTreeView )
	panner->setSeparator( 30 );
    else
    	panner->setSeparator( 0 );
}

void KfmGui::slotOpenLocation( )
{
    QString url = "";
    if ( view->getActiveView()->getURL() )
	url = view->getActiveView()->getURL();
    
    DlgLineEntry l( klocale->translate("Open Location:"), url.data(), 
		    this, true );
    int x = l.exec();
    if ( x )
    {
	QString url = l.getText();
	url = url.stripWhiteSpace();
	// Exit if the user did not enter an URL
	if ( url.data()[0] == 0 )
	    return;
	// Root directory?
	if ( url.data()[0] == '/' )
	{
	    url = "file:";
	    url += l.getText();
	}
	// Home directory?
	else if ( url.data()[0] == '~' )
	{
	    url = "file:";
	    url += QDir::homeDirPath().data();
	    url += l.getText() + 1;
	}

	// Some kludge to add protocol specifier on
	// well known Locations
	if ( url.left(4) == "www." ) {
	    url = "http://";
	    url += l.getText();
	}
	if ( url.left(4) == "ftp." ) {
	    url = "ftp://";
	    url += l.getText();
	}
	/**
	 * Something for fun :-)
	 */
	if ( url == "about:kde" ) {
    		url = getenv( "KDEURL" );
		if ( url.isEmpty() )
			url = "http://www.kde.org";
	}
	
	KURL u( url.data() );
	if ( u.isMalformed() )
	{
	    warning(klocale->translate("ERROR: Malformed URL"));
	    return;
	}
	
	view->openURL( url.data() );
    }
}

void KfmGui::slotQuit()
{
    if ( QMessageBox::warning( 0, klocale->translate("KFM Confirm"), 
			      klocale->translate("Do you really want to quit?\nImportant desktop functionality will be lost"),
			      klocale->translate("Yes"),
			      klocale->translate("No") ) )
	return;
    
    // Session management
    pkfm->slotSave();
    
    QString file = QDir::homeDirPath();
    file += "/.kde/share/apps/kfm/pid";
    file += displayName();
    unlink( file.data() );

    // Clean up IO stuff
    KIOJob::deleteAllJobs();
    delete ( KIOServer::getKIOServer() );

    // saveSettings();

    exit(0);
}

void KfmGui::slotClose()
{
    close();
}

void KfmGui::slotAbout()
{
    QString about_title, about_text;
    about_title.sprintf( klocale->translate("About KFM") );
    about_text.sprintf("KFM Ver. %s\n\n%s", kfm_getrev(), klocale->translate("Author: Torben Weis\nweis@kde.org\n\nHTML widget by Martin Jones\nmjones@kde.org\n\nProxy Manager by Lars Hoss\nLars.Hoss@munich.netsurf.de") );
    QMessageBox::about( this, about_title, about_text );
}

void KfmGui::slotAboutQt()
{
    QMessageBox::aboutQt( this, "About Qt" );
}

void KfmGui::slotHelp()
{
  kapp->invokeHTMLHelp( "", "" );
}

void KfmGui::slotTreeUrlSelected( const char *_url , int _button )
{
    if ( _button == LeftButton )
    {
	slotOpenURL( _url );
	return;
    }
    
    KfmGui *f = new KfmGui( 0L, 0L, _url );
    f->show();
}

void KfmGui::slotTitle( const char *_title )
{
    setCaption( _title );
    title = _title;
    title.detach();
}

void KfmGui::slotSetStatusBar( const char *_url )
{
    statusBar->changeItem( (char*)_url, 1 );
}

void KfmGui::slotCopy()
{
    view->getActiveView()->slotCopy();
}

void KfmGui::slotPaste()
{
    view->getActiveView()->slotPaste();
}

void KfmGui::slotDelete()
{
    view->getActiveView()->slotDelete();
}

void KfmGui::slotTrash()
{
    view->getActiveView()->slotTrash();
}

const char* KfmGui::getURL() 
{
    return view->getURL();
}

void KfmGui::slotAnimatedLogoTimeout()
{
    animatedLogoCounter++;
    if ( animatedLogoCounter == animatedLogo->count() )
	animatedLogoCounter = 0;
    toolbarButtons->setButtonPixmap( 9, *( animatedLogo->at( animatedLogoCounter ) ) );
}

void KfmGui::slotAddWaitingWidget( KHTMLView *_w )
{
    debug( "Adding waiting: %ld, %d", (unsigned long)_w, waitingWidgetList.count() );
    if ( waitingWidgetList.findRef( _w ) != -1 )
	return;
    waitingWidgetList.append( _w );
    if ( !animatedLogoTimer->isActive() )
    {
	animatedLogoTimer->start( 50 );
	toolbarButtons->setItemEnabled( 8, true );
    }
}

void KfmGui::slotRemoveWaitingWidget( KHTMLView *_w )
{
    waitingWidgetList.removeRef( _w );
    
    if ( waitingWidgetList.count() == 0 )
    {
	animatedLogoTimer->stop();
	toolbarButtons->setButtonPixmap( 9, *( animatedLogo->at( 0 ) ) );
	toolbarButtons->setItemEnabled( 8, false );
	slotSetStatusBar( klocale->translate("Document: Done") );
    }

    debug( "Removed waiting: %ld, %d", (unsigned long)_w, waitingWidgetList.count() );

   KURL u( view->getURL());
   _w->gotoAnchor(u.reference());
}

void KfmGui::slotTextSelected( KHTMLView *v, bool s )
{
    if ( s )
    {
       QString sel;

       v->getSelectedText( sel );
       QClipboard *cb = KApplication::clipboard();
       cb->setText( sel );
    }
}

void KfmGui::slotSaveSettings()
{
  KConfig *config = kapp->getConfig();
  config->setGroup( "Settings" );
    
  config->writeEntry("kfmgui_width",this->width());
  config->writeEntry("kfmgui_height",this->height());

  QString entry;

  if ( bTreeView == false)
    entry = "Off";
  else
    entry = "On";
  
  config->writeEntry("TreeView", entry);
  
  switch (viewMode)
    {
    case ICON_VIEW:
      entry = "IconView";
      break;
    case LONG_VIEW:
      entry = "LongView";
      break;
    case TEXT_VIEW:
      entry = "TextView";
      break;
    case SHORT_VIEW:
      entry = "ShortView";
      break;
    }

  config->writeEntry("ViewMode", entry);

  if (showDot == true)
    entry = "yes";
  else
    entry = "no";
  
  config->writeEntry("ShowDotFiles", entry);

  if (visualSchnauzer == false)
    entry = "Off";
  else
    entry = "On";
  
  config->writeEntry("VisualSchnauzer", entry);

  if (bViewHTML == true)
    entry = "yes";
  else
    entry = "no";
  
  config->writeEntry("HTMLView", entry);

  if ( !showToolbar )
      config->writeEntry( "Toolbar", "hide" );
  else if ( toolbarButtons->barPos() == KToolBar::Top )
      config->writeEntry( "Toolbar", "top" );
  else if ( toolbarButtons->barPos() == KToolBar::Bottom )
      config->writeEntry( "Toolbar", "bottom" );
  else if ( toolbarButtons->barPos() == KToolBar::Left )
      config->writeEntry( "Toolbar", "left" );
  else if ( toolbarButtons->barPos() == KToolBar::Right )
      config->writeEntry( "Toolbar", "right" );
  else if ( toolbarButtons->barPos() == KToolBar::Floating )
      config->writeEntry( "Toolbar", "floating" );

  if ( !showLocationBar )
      config->writeEntry( "LocationBar", "hide" );
  else if ( toolbarURL->barPos() == KToolBar::Top )
      config->writeEntry( "LocationBar", "top" );
  else if ( toolbarURL->barPos() == KToolBar::Bottom )
      config->writeEntry( "LocationBar", "bottom" );
  else if ( toolbarURL->barPos() == KToolBar::Floating )
      config->writeEntry( "LocationBar", "floating" );

  if ( !showStatusbar )
      config->writeEntry( "Statusbar", "hide" );
  else
      config->writeEntry( "Statusbar", "bottom" );

  if ( !showMenubar )
      config->writeEntry( "Menubar", "hide" );
  else if ( menu->menuBarPos() == KMenuBar::Top )
      config->writeEntry( "Menubar", "top" );
  else if ( menu->menuBarPos() == KMenuBar::Bottom )
      config->writeEntry( "Menubar", "bottom" );
  else if ( menu->menuBarPos() == KMenuBar::Floating )
      config->writeEntry( "Menubar", "floating" );
  
  config->setGroup( "Cache" );
  config->writeEntry( "CacheEnabled", HTMLCache::isEnabled() );
  config->writeEntry( "SaveCacheEnabled", HTMLCache::isSaveEnabled() );

  config->sync();
}


void KfmGui::slotConfigureBrowser()
{
  //-> was:  UserAgentDialog dlg;
  KKFMOptDlg dlg;
  QStrList strlist( true );

  // read entries from config file
  KConfig* config = kapp->getConfig();
  QString oldgroup = config->group();

  // read entries for UserAgentDlg
  config->setGroup( "Browser Settings/UserAgent" );
  int entries = config->readNumEntry( "EntriesCount", 0 );
  for( int i = 0; i < entries; i++ )
	{
	  QString key;
	  key.sprintf( "Entry%d", i );
	  strlist.append( config->readEntry( key, "" ) );
	}
  // if there was no entry at all, we set at least a default
  if( entries == 0 )
	strlist.append( "*:Konqueror/1.0" );

  // transmit data to dialog
  dlg.setUsrAgentData( &strlist );

  // show the dialog
  int ret = dlg.exec();
  if( ret == QDialog::Accepted )
	{
	  // write back the entries from UserAgent
          config->setGroup("Browser Settings/UserAgent");
	  strlist = dlg.dataUsrAgent();
	  if( strlist.count() )
		{
		  config->writeEntry( "EntriesCount", strlist.count() );
		  for( uint i = 0; i < strlist.count(); i++ )
			{
			  QString key;
			  key.sprintf( "Entry%d", i );
			  config->writeEntry( key, strlist.at( i ) );
			}
		}
	  else
		{
		  // everything deleted -> write at least the Konqueror entry
		  config->writeEntry( "EntriesCount", 1 );
		  config->writeEntry( "Entry1", "*:Konqueror/1.0" );
		}

	  // write back the entries from KProxyDlg
          struct proxyoptions proxyopts;
          
	  dlg.proxyData( proxyopts );
          config->setGroup("Browser Settings/Proxy");
          config->writeEntry( "UseProxy", proxyopts.useProxy.data() );
	  config->writeEntry( "HTTP-Proxy", proxyopts.http_proxy.data() );
          config->writeEntry( "FTP-Proxy", proxyopts.ftp_proxy.data() );
	  config->writeEntry( "NoProxyFor", proxyopts.no_proxy_for.data() );


	  struct coloroptions coloropts;
	  struct fontoptions  fontopts;
	  struct rootoptions  rootopts;

	  dlg.fontData(fontopts);
	  dlg.colorData(coloropts);
	  dlg.miscData(rootopts);

	  if(fontopts.changed){

	    config->setGroup( "KFM HTML Defaults" );			
	    config->writeEntry( "BaseFontSize", fontopts.fontsize );
	    config->writeEntry( "StandardFont", fontopts.standardfont );
	    config->writeEntry( "FixedFont", fontopts.fixedfont );

	  }


	  if(coloropts.changed){

	    config->setGroup( "KFM HTML Defaults" );			
	    config->writeEntry( "BgColor", coloropts.bg );
	    config->writeEntry( "TextColor", coloropts.text);
	    config->writeEntry( "LinkColor", coloropts.link);
	    config->writeEntry( "VLinkColor", coloropts.vlink);
	    config->writeEntry( "ChangeCursor", coloropts.changeCursoroverLink);

	  }


	  if(rootopts.changed){
	    config->setGroup( "KFM Misc Defaults" );			
	    config->writeEntry( "GridWidth", rootopts.gridwidth);
	    config->writeEntry( "GridHeight", rootopts.gridheight);
	    config->setGroup( "KFM Root Icons" );			
	    config->writeEntry( "Style", rootopts.iconstyle);
	  }

	  

	  if(coloropts.changed || fontopts.changed){

	    KfmGui *w;
	    KHTMLWidget* htmlview;
	    for ( w = windowList->first(); w != 0L; w = windowList->next() ){

	      htmlview = view->getKHTMLWidget();
	      //htmlview now points to the underlying lowl leve html widget.
	      
	      htmlview->setFixedFont( fontopts.fixedfont.data() );
	      htmlview->setStandardFont( fontopts.standardfont.data() );
	      htmlview->setDefaultFontBase( fontopts.fontsize );

	      // w->view points to the kfmview in w  the kfmgui
	      w->view->setDefaultTextColors( 
					     coloropts.text, 
					     coloropts.link,
					     coloropts.vlink 
					     );

	      w->view->setDefaultBGColor( coloropts.bg );
	      if(coloropts.changeCursoroverLink)
		htmlview->setURLCursor( upArrowCursor);
	      else
		htmlview->setURLCursor( arrowCursor );

	      w->updateView();

	      if(w->treeView){
		w->treeView->setColors(coloropts.bg,coloropts.link);
		//		w->treeView->updateTree(true);
		w->treeView->repaint();
	      }
	      

	    }

	  }

	  if(rootopts.changed){
	    if ( KRootWidget::getKRootWidget() ){
	      KRootWidget::getKRootWidget()->setRootGridParameters(
								   rootopts.gridwidth ,
								   rootopts.gridheight
								   );
	      KRootWidget::getKRootWidget()->setRootIconStyle( rootopts.iconstyle );
	    }

	  }


	}

  // restore the group
  config->setGroup( oldgroup );
}


void KfmGui::slotViewFrameSource()
{
    if ( !view->getActiveView() )
	return;
    
    QString cmd;
    cmd << "kedit \"" << view->getActiveView()->getURL() << "\"";
    KMimeBind::runCmd( cmd );
}

void KfmGui::slotViewDocumentSource()
{
    QString cmd;
    cmd << "kedit \"" << view->getURL() << "\"";
    KMimeBind::runCmd( cmd );
}

void KfmGui::setCharset(const char *_c){

   view->setCharset(_c);
}
    
KfmGui::~KfmGui()
{
    if ( animatedLogoTimer )
    {
	animatedLogoTimer->stop();
	delete animatedLogoTimer;
    }
    
    if ( toolbarButtons )
	delete toolbarButtons;
    if ( toolbarURL )
	delete toolbarURL;
    if (menu)
       delete menu;
 
    delete view;
    delete completion;
    windowList->remove( this );

    // Last window and in window-only-mode ?
    if ( windowList->count() == 0 && !rooticons )
    {
	// remove pid file
	QString file = QDir::homeDirPath();
	file += "/.kde/share/apps/kfm/pid";
	file += displayName();
	unlink( file.data() );
	// quit
	exit(0);
    }
}

#include "kfmgui.moc"



