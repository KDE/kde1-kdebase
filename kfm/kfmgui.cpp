#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>

#include <qpopmenu.h>
#include <qpainter.h>
#include <qapp.h>
#include <qkeycode.h>
#include <qaccel.h>
#include <qpushbt.h>
#include <qdir.h>
#include <qstrlist.h>
#include <qregexp.h>
#include <qmsgbox.h>
#include <qtooltip.h>

#include <Kconfig.h>
#include <kapp.h>

#include "kfmpaths.h"
#include "kfmgui.h"
#include "kfmdlg.h"
#include "kfmprops.h"
#include "kbutton.h"
#include "root.h"
#include "kbind.h"
#include <config-kfm.h>

#include <klocale.h>

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

KBookmarkManager KfmGui::bookmarkManager;

QList<KfmGui> KfmGui::windowList;

QList<QPixmap> KfmGui::animatedLogo;

KfmGui::KfmGui( QWidget *, const char *name, const char * _url)
    : KTopLevelWidget( name )
{
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

    initGUI();

    windowList.setAutoDelete( false );
    windowList.append( this );

    if ( _url )
	view->openURL( _url );
}

KfmGui* KfmGui::findWindow( const char *_url )
{
    KfmGui *w;
    for ( w = windowList.first(); w != 0L; w = windowList.next() )
	if ( strcmp( _url, w->getURL() ) == 0 )
	    return w;
    
    return 0L;
}

void KfmGui::initGUI()
{
    setMinimumSize( 100, 100 );

    initAccel();
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

void KfmGui::initAccel()
{
    accel = new QAccel( this );  
  
    accel->insertItem (Key_Up, UP);
    accel->insertItem (Key_Down, DOWN);
    accel->insertItem (Key_Prior, PGUP);
    accel->insertItem (Key_Next, PGDOWN);

    accel->connectItem(UP, this, SLOT (slotKeyUp ()));
    accel->connectItem(DOWN, this, SLOT (slotKeyDown ()));
    accel->connectItem(PGUP, this, SLOT (slotPageUp ()));
    accel->connectItem(PGDOWN, this, SLOT (slotPageDown ()));
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
    treeView = new KFMTreeView( pannerChild0 );
    connect( treeView, SIGNAL( showDir( const char * ) ), this, SLOT( slotOpenURL( const char * ) ) );
    connect( treeView, SIGNAL( popupMenu( const char *, const QPoint & )),
	     this, SLOT( slotTreeViewPopupMenu( const char *, const QPoint &)) );
    treeView->show();
}

void KfmGui::initStatusBar()
{
    statusBar = new KStatusBar( this );
 
    statusBar->insertItem( (char*)klocale->translate("KFM"), 1 );
    // statusBar->insertItem("Some long comment", 2 );
    
    statusBar->show();
    setStatusBar( statusBar );
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
    file->insertItem( klocale->translate("&Run"), 
		      this, SLOT(slotRun()) );
    file->insertItem( klocale->translate("Open &Terminal"), 
		      this, SLOT(slotTerminal()) );
    file->insertSeparator();
    file->insertItem( klocale->translate("&Open Location"),
		      this, SLOT(slotOpenLocation()), CTRL+Key_L );
    file->insertSeparator();
    file->insertItem( klocale->translate("&Print..."), 
		      this, SLOT(slotPrint()) );
    file->insertSeparator();        
    file->insertItem( klocale->translate("&Close"), 
		      this, SLOT(slotClose()), CTRL+Key_W );
    file->insertItem( klocale->translate("&Quit"),  
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
    mview->insertSeparator();
    mview->insertItem( klocale->translate("&Save Settings"),
		       this, SLOT(slotSaveSettings()) );
    /* mview->insertSeparator();
    mview->insertItem( klocale->translate("Split &window"),
		       this, SLOT(slotSplitWindow()) ); */
    mview->insertSeparator();
    mview->insertItem( klocale->translate("&Reload Document"),
		       view, SLOT(slotReload()) );
    mview->insertItem( klocale->translate("Rescan &bindings"),
		       this, SLOT(slotRescanBindings()) );
    mview->insertSeparator();
    mview->insertItem( klocale->translate("&View Frame Source"),
		       this, SLOT(slotViewFrameSource()) );
    
    mview->setItemChecked( mview->idAt( 0 ), showDot );
    mview->setItemChecked( mview->idAt( 1 ), bTreeView );
    mview->setItemChecked( mview->idAt( 2 ), visualSchnauzer );
    mview->setItemChecked( mview->idAt( 3 ), bViewHTML );

    switch (viewMode) 
      {
      case ICON_VIEW:
	mview->setItemChecked( mview->idAt( 5 ), true );
	break;
      case LONG_VIEW:
	mview->setItemChecked( mview->idAt( 6 ), true );
	break;
      case TEXT_VIEW:
	mview->setItemChecked( mview->idAt( 7 ), true );
	break;
      }

    QPopupMenu *nav = new QPopupMenu;
    CHECK_PTR( nav );

    QPopupMenu *tool = new QPopupMenu;
    CHECK_PTR( tool );
    tool->insertItem( klocale->translate("&Find"), this, 
		      SLOT(slotToolFind()), ALT+Key_S );

    bookmarkMenu = new QPopupMenu;
    CHECK_PTR( bookmarkMenu );
    connect( &bookmarkManager, SIGNAL( changed() ), 
	     this, SLOT( slotBookmarksChanged() ) );
    QString p = getenv( "HOME" );
    QString bmFile = p + "/.kde/kfm.bookmarks.html";
    bookmarkMenu->insertItem( klocale->translate("&Add Bookmark"), 
			      this, SLOT(slotAddBookmark()) );
    bookmarkManager.read( bmFile );
    
    QPopupMenu *help = new QPopupMenu;
    CHECK_PTR( help );
    help->insertItem( klocale->translate("&About"), this, SLOT(slotAbout()) );
    help->insertItem( klocale->translate("&How can I ..."), 
		      this, SLOT(slotHelp()) );

    menu = new KMenuBar( this );
    CHECK_PTR( menu );
    menu->insertItem( klocale->translate("&File"), file );
    menu->insertItem( klocale->translate("&Edit"), edit );
    menu->insertItem( klocale->translate("&View"), mview );
    menu->insertItem( klocale->translate("&Bookmarks"), bookmarkMenu );
    menu->insertItem( klocale->translate("&Tool"), tool );
    menu->insertSeparator();
    menu->insertItem( klocale->translate("&Help"), help );
    menu->show();
    
    setMenu( menu );
}

void KfmGui::enableToolbarButton( int id, bool enable )
{
    if ( toolbar == 0L )
	return;
    
    toolbar->setItemEnabled( id, enable );
}

void KfmGui::initToolBar()
{
    QString file, path;
    QPixmap pixmap;
    toolbar = new KToolBar(this, "kfmwin-toolbar");
    path = kapp->kdedir() + "/share/toolbar/";
    
    pixmap.load(path + "back.xpm");
    toolbar->insertButton(pixmap, 0, SIGNAL( clicked() ), view, 
			  SLOT( slotBack() ), false, 
			  klocale->translate("Back"));
    
    pixmap.load(path + "forward.xpm");
    toolbar->insertButton(pixmap, 1, SIGNAL( clicked() ), view, 
			  SLOT( slotForward() ), false, 
			  klocale->translate("Forward"));
    
    pixmap.load(path + "home.xpm");
    toolbar->insertButton(pixmap, 2, SIGNAL( clicked() ), this, 
			  SLOT( slotHome() ), true, 
			  klocale->translate("Home") );
    
    toolbar->insertSeparator();
    
    pixmap.load(path + "reload.xpm");
    toolbar->insertButton(pixmap, 3, SIGNAL( clicked() ), view, 
			  SLOT( slotReload() ), true, 
			  klocale->translate("Reload") );

    toolbar->insertSeparator();
    
    pixmap.load(path + "editcopy.xpm");
    toolbar->insertButton(pixmap, 4, SIGNAL( clicked() ), this, 
			  SLOT( slotCopy() ), true, 
			  klocale->translate("Copy") );
    
    pixmap.load(path + "editpaste.xpm");
    toolbar->insertButton(pixmap, 5, SIGNAL( clicked() ), this, 
			  SLOT( slotPaste() ), true, 
			  klocale->translate("Paste") );
    
    toolbar->insertSeparator();
    
    pixmap.load(path + "help.xpm");
    toolbar->insertButton(pixmap, 6, SIGNAL( clicked() ), this, 
			  SLOT( slotHelp() ), true, 
			  klocale->translate("Help"));
    
    toolbar->insertSeparator();
    
    pixmap.load(path + "exit.xpm");
    toolbar->insertButton(pixmap, 7, SIGNAL( clicked() ), this, 
			  SLOT( slotStop() ), false, 
			  klocale->translate("Stop"));

    path = kapp->kdedir() + "/share/apps/kfm/pics/";

    pixmap.load( path + "/kde1.xpm" );
    
    toolbar->insertButton(pixmap, 8, SIGNAL( clicked() ), this, 
			  SLOT( slotNewWindow() ), false );
    toolbar->setItemEnabled( 8, true );
    
    // Load animated logo
    if ( animatedLogo.count() == 0 )
    {
	animatedLogo.setAutoDelete( true );
	for ( int i = 1; i <= 9; i++ )
	{
	    QString n;
	    n.sprintf( "/kde%i.xpm", i );
	    QPixmap *p = new QPixmap();
	    p->load( path + n );
	    if ( p->isNull() )
	    {
		QString e;
		e.sprintf( klocale->translate( "Could not load icon\n\r%s" ), n.data() );
		QMessageBox::message( klocale->translate( "KFM Error" ), e.data() );
	    }
	    animatedLogo.append( p );
	}
    }
	    
    addToolBar( toolbar );
    toolbar->setBarPos(KToolBar::Top);
    toolbar->show();                
}

void KfmGui::initView()
{
    view = new KfmView( this, pannerChild1, "" );
    CHECK_PTR( view );
    
    connect( view, SIGNAL( setTitle(const char *)), this, SLOT( slotTitle(const char *)) );
    connect( view, SIGNAL( historyUpdate( bool, bool ) ), this, SLOT( slotUpdateHistory( bool, bool ) ) );
    connect( view, SIGNAL( documentStarted( KHTMLView * ) ), 
	     this, SLOT( slotAddWaitingWidget( KHTMLView * ) ) );
    connect( view, SIGNAL( documentDone( KHTMLView * ) ), 
	     this, SLOT( slotRemoveWaitingWidget( KHTMLView * ) ) );
    
    view->show();
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

void KfmGui::slotRescanBindings()
{
    KMimeType::clearAll();
    KMimeType::init();
    KRootWidget::getKRootWidget()->update();

    KfmGui *win;
    for ( win = windowList.first(); win != 0L; win = windowList.next() )
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

void KfmGui::slotPrint()
{
    if ( view->getActiveView() )
	view->getActiveView()->print();
}   

void KfmGui::slotPannerChanged()
{
    if ( !bTreeViewInitialized )
	treeView->initializeTree();  

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
    debugT("VIEW is '%i' %d\n",(int)bViewHTML, mview->idAt(3));
    // viewMode = ICON_VIEW;
    mview->setItemChecked( mview->idAt( 3 ), bViewHTML);
    view->slotUpdateView();
}

void KfmGui::slotIconView()
{
    viewMode = ICON_VIEW;
    // bViewHTML = false;
    mview->setItemChecked( mview->idAt( 5 ), true );
    mview->setItemChecked( mview->idAt( 6 ), false );
    mview->setItemChecked( mview->idAt( 7 ), false );
    view->slotUpdateView();
}

void KfmGui::slotLongView()
{
    viewMode = LONG_VIEW;
    // bViewHTML = false;
    mview->setItemChecked( mview->idAt( 5 ), false );
    mview->setItemChecked( mview->idAt( 6 ), false);
    mview->setItemChecked( mview->idAt( 7 ), true );
    view->slotUpdateView();
}

void KfmGui::slotTextView()
{
    viewMode = TEXT_VIEW;
    // bViewHTML = false;
    mview->setItemChecked( mview->idAt( 5 ), false );
    mview->setItemChecked( mview->idAt( 6 ), true );
    mview->setItemChecked( mview->idAt( 7 ), false );
    view->slotUpdateView();
}

void KfmGui::slotKeyUp()
{
    view->getActiveView()->slotVertSubtractLine();
}

void KfmGui::slotKeyDown()
{
    view->getActiveView()->slotVertAddLine();
}

void KfmGui::slotPageUp()
{
    view->getActiveView()->slotVertSubtractPage();
}

void KfmGui::slotPageDown()
{
    view->getActiveView()->slotVertAddPage();
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
    bookmarkMenu->insertItem( klocale->translate("Add Bookmark"), 
			      this, SLOT(slotAddBookmark()) );
    bookmarkMenu->insertSeparator();
    int idStart = BOOKMARK_ID_BASE;
    fillBookmarkMenu( bookmarkManager.getRoot(), bookmarkMenu, idStart );
}

void KfmGui::fillBookmarkMenu( KBookmark *parent, QPopupMenu *menu, int &id )
{
    KBookmark *bm;
    
    connect( menu, SIGNAL( activated( int ) ),
	     SLOT( slotBookmarkSelected( int ) ) );
    
    for ( bm = parent->getChildren().first(); bm != NULL;
	  bm = parent->getChildren().next() )
    {
	if ( bm->getType() == KBookmark::URL )
	{
	    menu->insertItem( bm->getText(), id );
	    id++;
	}
	else
	{
	    QPopupMenu *subMenu = new QPopupMenu;
	    menu->insertItem( bm->getText(), subMenu );
	    fillBookmarkMenu( bm, subMenu, id );
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
	    debugT("Command copy '%s' '%s'\n",src.data(),dest.data());
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
    enableToolbarButton( 0, _back );
    enableToolbarButton( 1, _forward );
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
    QString bmFile = p + "/.kde/kfm.bookmarks.html";
    bookmarkManager.add( _title, _url );
    bookmarkManager.write( bmFile );
}

void KfmGui::slotAddBookmark()
{
    addBookmark( title.data(), view->getURL() );
}

void KfmGui::slotBookmarkSelected( int id )
{
    id -= BOOKMARK_ID_BASE;
    
    debugT( "Bookmark selected : %i\n",id );
    
    KBookmark *bm = bookmarkManager.getBookmark( id );
    
    if ( bm )
    {
	KURL u( bm->getURL() );
	if ( u.isMalformed() )
	{
	      warning(klocale->translate("ERROR: Malformed URL"));
	      return;
	}
	
	view->openURL( bm->getURL() );
    }
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

void KfmGui::slotShowDot()
{
    showDot = !showDot;
    mview->setItemChecked( mview->idAt( 0 ), showDot );
    view->slotUpdateView();
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
	treeView->initializeTree();  
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
    if ( !QMessageBox::query( klocale->translate("KFM"), 
			      klocale->translate("Do you really want to quit"),
			      klocale->translate("Yes"),
			      klocale->translate("No") ) )
	return;
    
    QString file = QDir::homeDirPath();
    file += "/.kfm.run";
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
    QString tmp;
    tmp.sprintf("KFM %s\n\n%s", kfm_getrev(), klocale->translate("(c) by Torben Weis\nweis@kde.org\n\nand the KFM team") );
    QMessageBox::message( klocale->translate("About"), 
			  tmp, klocale->translate("Ok") );
}

void KfmGui::slotHelp()
{
  kapp->invokeHTMLHelp( "kfm/kfm.html", "" );
}

void KfmGui::slotTreeViewPopupMenu( const char *_url, const QPoint & )
{
    debugT("slotPopupMenu single '%s'\n",_url);

    // TODO: viewManager->openPopupMenu( _url, _point );
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
    if ( animatedLogoCounter == animatedLogo.count() )
	animatedLogoCounter = 0;
    toolbar->setButtonPixmap( 8, *( animatedLogo.at( animatedLogoCounter ) ) );
}

void KfmGui::slotAddWaitingWidget( KHTMLView *_w )
{
    if ( waitingWidgetList.findRef( _w ) != -1 )
	return;
    waitingWidgetList.append( _w );
    if ( !animatedLogoTimer->isActive() )
    {
	animatedLogoTimer->start( 50 );
	toolbar->setItemEnabled( 7, true );
    }
}

void KfmGui::slotRemoveWaitingWidget( KHTMLView *_w )
{
    waitingWidgetList.removeRef( _w );
    
    if ( waitingWidgetList.count() == 0 )
    {
	animatedLogoTimer->stop();
	toolbar->setButtonPixmap( 8, *( animatedLogo.at( 0 ) ) );
	toolbar->setItemEnabled( 7, false );
	slotSetStatusBar( klocale->translate("Document: Done") );
    }

   KURL u( view -> getURL());
   _w->gotoAnchor(u.reference());
}

void KfmGui::slotSaveSettings()
{
  KConfig *config = kapp->getConfig();
  config->setGroup( "Settings" );
    
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
  config->sync();
}

void KfmGui::slotViewFrameSource()
{
    if ( !view->getActiveView() )
	return;
    
    QString cmd;
    cmd.sprintf("kedit \"%s\"", view->getActiveView()->getURL() );
    debugT("RUNNING '%s'\n",cmd.data());
    KMimeBind::runCmd( cmd );
}
    
KfmGui::~KfmGui()
{
    delete view;
    windowList.remove( this );
    // if ( windowList.isEmpty() )   the last window closed?
    // saveSettings();
}

#include "kfmgui.moc"



