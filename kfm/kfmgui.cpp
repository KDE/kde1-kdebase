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

#include "kfmgui.h"
#include "kfmdlg.h"
#include "kfmprops.h"
#include "kbutton.h"
#include "root.h"
#include <config-kfm.h>

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
    
    debugT("Hi 1\n");
    
    bTreeViewInitialized = FALSE;
    bTreeView = FALSE;
        
    debugT("Hi 2\n");

    viewMode = ICON_VIEW;
    showDot = FALSE;
    visualSchnauzer = FALSE;
    bViewHTML = TRUE;

    debugT("Hi 3\n");

    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup( "Templates" );
    templatePath = config->readEntry( "Path" );
    if ( templatePath.isNull() )
    {
	debugT("ERROR: No path for templates specified in config files\n");
	exit(1);
    }

    debugT("Hi 4\n");
    initGUI();
    debugT("Hi 5\n");

    windowList.setAutoDelete( FALSE );
    windowList.append( this );

    debugT("Hi 6\n");
    if ( _url )
	view->openURL( _url );
    debugT("Hi 7\n");
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

    debugT("GUI 2\n");
    initAccel();
    debugT("GUI 3\n");
    initPanner();
    debugT("GUI 4\n");
    initTreeView();
    debugT("GUI 5\n");
    initStatusBar();
    debugT("GUI 6\n");
    initView();
    debugT("GUI 7\n");
    initMenu();
    debugT("GUI 1\n");
    initToolBar();
    QGridLayout *gl = new QGridLayout( pannerChild0, 1, 1 );
    gl->addWidget( treeView, 0, 0 );
    debugT("GUI 8\n");
    gl = new QGridLayout( pannerChild1, 1, 1 );
    gl->addWidget( view, 0, 0 );
    debugT("GUI 9\n");
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
 
    statusBar->insertItem("KFM", 1 );
    // statusBar->insertItem("Some long comment", 2 );
    
    statusBar->show();
    setStatusBar( statusBar );
}

void KfmGui::initMenu()
{
    debugT("Menu 1\n");
    
    menuNew = new QPopupMenu;
    CHECK_PTR( menuNew );
    menuNew->insertItem( "Folder" );

    debugT("Menu 2\n");

    connect( menuNew, SIGNAL( activated( int ) ), this, SLOT( slotNewFile( int ) ) );

    debugT("Menu 3\n");

    QDir d( templatePath );
    const QFileInfoList *list = d.entryInfoList();
    if ( list == 0L )
	debugT("ERROR: Template does not exist '%s'\n",templatePath.data());
    else
    {
	QFileInfoListIterator it( *list );      // create list iterator
	QFileInfo *fi;                          // pointer for traversing

	while ( ( fi = it.current() ) != 0L )
	{
	    if ( strcmp( fi->fileName().data(), "." ) != 0 && strcmp( fi->fileName().data(), ".." ) != 0 )
		menuNew->insertItem( fi->fileName().data() );
	    ++it;                               // goto next list element
	}
    }

    debugT("Menu 4\n");    

    QPopupMenu *file = new QPopupMenu;
    CHECK_PTR( file );
    file->insertItem( "New", menuNew );
    file->insertSeparator();
    file->insertItem( "New Window", this, SLOT(slotNewWindow()), ALT+Key_N );
    file->insertSeparator();
    file->insertItem( "Run", this, SLOT(slotRun()), ALT+Key_R );
    file->insertItem( "Open Terminal", this, SLOT(slotTerminal()), ALT+Key_E );
    file->insertSeparator();
    file->insertItem( "Open Location", this, SLOT(slotOpenLocation()), ALT+Key_L );
    file->insertSeparator();
    file->insertItem( "Print...", this, SLOT(slotPrint()), ALT+Key_P );
    file->insertSeparator();        
    file->insertItem( "Close", this, SLOT(slotClose()), ALT+Key_C );
    file->insertItem( "Quit",  this, SLOT(slotQuit()), ALT+Key_Q );

    QPopupMenu *edit = new QPopupMenu;
    CHECK_PTR( edit );
    edit->insertItem( "Copy", this, SLOT(slotCopy()), CTRL+Key_C );
    edit->insertItem( "Paste", this, SLOT(slotPaste()), CTRL+Key_V );
    edit->insertItem( "Move to Trash", this, SLOT(slotTrash()),  ALT+Key_T );
    edit->insertItem( "Delete", this, SLOT(slotDelete()) );
    edit->insertSeparator();
    edit->insertItem( "Select", this, SLOT(slotSelect()), CTRL+Key_S );

    mview = new QPopupMenu;
    CHECK_PTR( mview );
    mview->setCheckable(TRUE);
    mview->insertItem( "Show Dot Files", this, SLOT(slotShowDot()), ALT+Key_D );
    mview->insertItem( "Show Tree", this, SLOT(slotShowTreeView()) );
    mview->insertItem( "Visual Schnauzer", this, SLOT(slotShowSchnauzer()) );
    mview->insertSeparator();
    mview->insertItem( "HTML View", this, SLOT(slotViewHTML()), ALT+Key_H );
    mview->insertItem( "Icon View", this, SLOT(slotIconView()), ALT+Key_I );
    mview->insertItem( "Text View", this, SLOT(slotTextView()) );
    mview->insertItem( "Long View", this, SLOT(slotLongView()), ALT+Key_O );
    mview->insertSeparator();
    mview->insertItem( "Split window", this, SLOT(slotSplitWindow()) );
    mview->insertSeparator();
    mview->insertItem( "Reload Document", view, SLOT(slotReload()), ALT+Key_R );
    mview->insertItem( "Rescan bindings", this, SLOT(slotRescanBindings()) );

    mview->setItemChecked( mview->idAt( 4 ), TRUE );
    
    QPopupMenu *nav = new QPopupMenu;
    CHECK_PTR( nav );

    QPopupMenu *tool = new QPopupMenu;
    CHECK_PTR( tool );
    tool->insertItem( "Find", this, SLOT(slotToolFind()), ALT+Key_S );

    bookmarkMenu = new QPopupMenu;
    CHECK_PTR( bookmarkMenu );
    connect( &bookmarkManager, SIGNAL( changed() ), this, SLOT( slotBookmarksChanged() ) );
    QString p = getenv( "HOME" );
    QString bmFile = p + "/.kfm.bookmarks.html";
    bookmarkMenu->insertItem( "Add Bookmark", this, SLOT(slotAddBookmark()) );
    bookmarkManager.read( bmFile );
    
    QPopupMenu *help = new QPopupMenu;
    CHECK_PTR( help );
    help->insertItem( "About", this, SLOT(slotAbout()) );
    help->insertItem( "How can I ...", this, SLOT(slotHelp()) );

    menu = new KMenuBar( this );
    CHECK_PTR( menu );
    menu->insertItem( "File", file );
    menu->insertItem( "Edit", edit );
    menu->insertItem( "View", mview );
    menu->insertItem( "Bookmarks", bookmarkMenu );
    menu->insertItem( "Tool", tool );
    menu->insertSeparator();
    menu->insertItem( "Help", help );
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
    path = KMimeType::getIconPath() + QString("/toolbar/");
    
    pixmap.load(path + "back.xpm");
    toolbar->insertButton(pixmap, 0, SIGNAL( clicked() ), view, SLOT( slotBack() ), FALSE, "Back");
    
    pixmap.load(path + "forward.xpm");
    toolbar->insertButton(pixmap, 1, SIGNAL( clicked() ), view, SLOT( slotForward() ), FALSE, "Forward");
    
    pixmap.load(path + "home.xpm");
    toolbar->insertButton(pixmap, 2, SIGNAL( clicked() ), this, SLOT( slotHome() ), TRUE, "Home");
    
    toolbar->insertSeparator();
    
    pixmap.load(path + "reload.xpm");
    toolbar->insertButton(pixmap, 3, SIGNAL( clicked() ), view, SLOT( slotReload() ), TRUE, "Reload");

    toolbar->insertSeparator();
    
    pixmap.load(path + "editcopy.xpm");
    toolbar->insertButton(pixmap, 4, SIGNAL( clicked() ), this, SLOT( slotCopy() ), TRUE, "Copy");
    
    pixmap.load(path + "editpaste.xpm");
    toolbar->insertButton(pixmap, 5, SIGNAL( clicked() ), this, SLOT( slotPaste() ), TRUE, "Paste");
    
    toolbar->insertSeparator();
    
    pixmap.load(path + "help.xpm");
    toolbar->insertButton(pixmap, 6, SIGNAL( clicked() ), this, SLOT( slotHelp() ), TRUE, "Help");
    
    toolbar->insertSeparator();
    
    pixmap.load(path + "exit.xpm");
    toolbar->insertButton(pixmap, 7, SIGNAL( clicked() ), this, SLOT( slotStop() ), FALSE, "Stop");

    path = KMimeType::getIconPath();
    pixmap.load( path + "/kde1.gif" );
    
    toolbar->insertButton(pixmap, 8, SIGNAL( clicked() ), this, SLOT( slotNewWindow() ), FALSE );
    toolbar->setItemEnabled( 8, TRUE );
    
    // Load animated logo
    if ( animatedLogo.count() == 0 )
    {
	animatedLogo.setAutoDelete( TRUE );
	for ( int i = 1; i <= 30; i++ )
	{
	    QString n;
	    n.sprintf( "/kde%i.gif", i );
	    QPixmap *p = new QPixmap();
	    p->load( path + n );
	    if ( p->isNull() )
	      debugT("????????????????????? Could not load '%s' ????????????\n",n.data());
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
    debugT("Closing\n");
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
      mview->setItemChecked( mview->idAt( 1 ), FALSE );
    else
      mview->setItemChecked( mview->idAt( 1 ), TRUE );
    
    resizeEvent( 0L );
}

void KfmGui::slotSplitWindow()
{
    view->getActiveView()->splitWindow();
}

void KfmGui::slotIconView()
{
    viewMode = ICON_VIEW;
    bViewHTML = FALSE;
    mview->setItemChecked( mview->idAt( 4 ), FALSE );
    mview->setItemChecked( mview->idAt( 5 ), TRUE );
    mview->setItemChecked( mview->idAt( 6 ), FALSE );
    mview->setItemChecked( mview->idAt( 7 ), FALSE );
    view->slotUpdateView();
}

void KfmGui::slotLongView()
{
    viewMode = LONG_VIEW;
    bViewHTML = FALSE;
    mview->setItemChecked( mview->idAt( 4 ), FALSE );
    mview->setItemChecked( mview->idAt( 5 ), FALSE );
    mview->setItemChecked( mview->idAt( 6 ), FALSE);
    mview->setItemChecked( mview->idAt( 7 ), TRUE );
    view->slotUpdateView();
}

void KfmGui::slotTextView()
{
    viewMode = TEXT_VIEW;
    bViewHTML = FALSE;
    mview->setItemChecked( mview->idAt( 4 ), FALSE );
    mview->setItemChecked( mview->idAt( 5 ), FALSE );
    mview->setItemChecked( mview->idAt( 6 ), TRUE );
    mview->setItemChecked( mview->idAt( 7 ), FALSE );
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
    DlgLineEntry l( "Select files:", "", this );
    if ( l.exec() )
    {
	QString pattern = l.getText();
	if ( pattern.length() == 0 )
	    return;

	QRegExp re( pattern, TRUE, TRUE );
	
	view->getActiveView()->select( re, TRUE );
    }
}

void KfmGui::slotBookmarksChanged()
{
    bookmarkMenu->clear();
    bookmarkMenu->disconnect( this );
    bookmarkMenu->insertItem( "Add Bookmark", this, SLOT(slotAddBookmark()) );
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
    
    QString p =  menuNew->text( _id );    

    QString text = "New ";
    text += p.data();
    text += ":";

    const char *value = "";
    if ( strcmp( p.data(), "Folder" ) != 0 )
	value = p.data();

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
	    QString src = templatePath.data();
	    if ( src.right( 1 ) != "/" )
		src += "/";
	    src += p.data();
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
    QString bmFile = p + "/.kfm.bookmarks.html";
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
		debugT("ERROR: Malformed URL\n");
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
	f = new KfmGui( 0L, 0L, view->getURL() );
    }
    
    f->show();
}

void KfmGui::slotViewHTML( )
{
    bViewHTML = TRUE;
    viewMode = ICON_VIEW;
    mview->setItemChecked( mview->idAt( 4 ), TRUE );
    mview->setItemChecked( mview->idAt( 5 ), FALSE );
    mview->setItemChecked( mview->idAt( 6 ), FALSE );
    mview->setItemChecked( mview->idAt( 7 ), FALSE );
    view->slotUpdateView();
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
	bTreeViewInitialized = TRUE;
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
    
    DlgLineEntry l( "Open Location:", url.data(), this, TRUE );
    if ( l.exec() )
    {
	QString url = l.getText();
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
	
	KURL u( url.data() );
	if ( u.isMalformed() )
	{
	    debugT("ERROR: Malformed URL\n");
	    return;
	}
	
	view->openURL( url.data() );
    }
}

void KfmGui::slotQuit()
{
    if ( !QMessageBox::query( "KFM", "Do you really want to quit" ) )
	return;
    
    QString file = QDir::homeDirPath();
    file += "/.kfm.run";
    unlink( file.data() );

    // Clean up IO stuff
    KIOJob::deleteAllJobs();
    delete ( KIOServer::getKIOServer() );
    
    exit(0);
}

void KfmGui::slotClose()
{
    close();
}

void KfmGui::slotAbout()
{
    QMessageBox::message( "About", "KFM 0.8.1\n\r(c) by Torben Weis\n\rweis@kde.org\n\r\n\rand the KFM team", "Ok" );
}

void KfmGui::slotHelp()
{
    if ( fork() == 0 )
    {
	QString arg = "file:";
	arg += kapp->kdedir();
	arg += "/doc/HTML/kfm/kfm.html";
        execlp( "kdehelp", "kdehelp", arg.data(), 0 );
        exit( 1 );
    }
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
	toolbar->setItemEnabled( 7, TRUE );
    }
}

void KfmGui::slotRemoveWaitingWidget( KHTMLView *_w )
{
    waitingWidgetList.removeRef( _w );
    
    if ( waitingWidgetList.count() == 0 )
    {
	animatedLogoTimer->stop();
	toolbar->setButtonPixmap( 8, *( animatedLogo.at( 0 ) ) );
	toolbar->setItemEnabled( 7, FALSE );
	slotSetStatusBar( "Document: Done" );
    }

   KURL u( view -> getURL());
   _w->gotoAnchor(u.reference());
}

KfmGui::~KfmGui()
{
    debugT("Deleting KfmGui\n");
    
    debugT("->View\n");
    delete view;
    debugT("<-View\n");

    windowList.remove( this );
    debugT("Deleted\n");
}

#include "kfmgui.moc"



