#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

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

#include "kfmwin.h"
#include "kfmdlg.h"
#include "kfmprops.h"
#include "kbutton.h"
#include "root.h"

QStrList KFileWindow::clipboard;

KBookmarkManager KFileWindow::bookmarkManager;

QList<KFileWindow> KFileWindow::windowList;

KFileWindow* KFileWindow::findWindow( const char *_url )
{
    KFileWindow *w;
    for ( w = windowList.first(); w != 0L; w = windowList.next() )
	if ( strcmp( _url, w->getURL() ) == 0 )
	    return w;
    
    return 0L;
}

KFileWindow::KFileWindow( QWidget *parent, const char *name, const char* _url )
    : QWidget( parent, name )
{
    bTreeViewInitialized = FALSE;
    bTreeView = FALSE;
    
    if ( _url != 0L )
    {
	KURL u( _url );
	if ( u.isMalformed() )
	{
	    /// TODO: Eine fehlerseite anzeigen, denn dieses Fenster schmiert ab, wenn es
	    /// geschlossen wird.
	    printf("ERROR: Malformed URL '%s'\n",_url );
	    return;
	}
    }
    
    popupMenu = 0L;
    viewMode = ICON_VIEW;
    showDot = FALSE;
    visualSchnauzer = FALSE;
    
    bViewHTML = TRUE;
    actualManager = 0L;
    stackLock = FALSE;

    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup( "Templates" );
    templatePath = config->readEntry( "Path" );
    if ( templatePath.isNull() )
    {
	printf("ERROR: No path for templates specified in config files\n");
	exit(1);
    }
    
    if ( _url != 0L )
	init( _url );

    backStack.setAutoDelete( FALSE );
    forwardStack.setAutoDelete( FALSE );

    windowList.setAutoDelete( FALSE );
    windowList.append( this );
}

void KFileWindow::init( const char *_url )
{
    initGUI();
    initFileManagers();
    openURL( _url );    
    
    if ( view->docWidth() > view->width() )
	horz->setRange( 0, view->docWidth() - view->width() );

    if ( view->docHeight() > view->height() )
	vert->setRange( 0, view->docHeight() - view->height() );

    connect( KIOServer::getKIOServer(), SIGNAL( notify( const char * ) ), this,
    	     SLOT( slotFilesChanged( const char * ) ) );
    connect( KIOServer::getKIOServer(), SIGNAL( mountNotify() ), this, SLOT( slotMountNotify() ) );
}

void KFileWindow::initFileManagers( )
{
    manager = new KFileManager( this, view );
    tarManager = new KTarManager( this, view );
    ftpManager = new KFtpManager( this, view );
    httpManager = new KHttpManager( this, view );
}

void KFileWindow::initGUI()
{
    setMinimumSize( 80, 80 );

    initAccel();
    initMenu();
    initToolBar();
    initPanner();
    initTreeView();
    initView();
    initStatusBar();
    initLayout();
}

void KFileWindow::initLayout()
{
    QGridLayout *gl = new QGridLayout( pannerChild0, 1, 1 );
    gl->addWidget( treeView, 0, 0 );
    
    gl = new QGridLayout( pannerChild1, 2, 2 );
    gl->addWidget( view, 0, 0 );
    gl->addWidget( vert, 0, 1 );
    gl->addWidget( horz, 1, 0 );
    gl->setRowStretch( 0, 1 );
    gl->setColStretch( 0, 1 );
    gl->setRowStretch( 1, 0 );
    gl->setColStretch( 1, 0 );
}

void KFileWindow::initPanner()
{
    if ( bTreeView )
	panner = new KPanner( this, "_panner", KPanner::O_VERTICAL, 30 );
    else
	panner = new KPanner( this, "_panner", KPanner::O_VERTICAL, 0 );
    
    pannerChild0 = panner->child0();
    pannerChild1 = panner->child1();    
    connect( panner, SIGNAL( positionChanged() ), this, SLOT( slotPannerChanged() ) );
}

void KFileWindow::initTreeView()
{
    treeView = new KFMTreeView( pannerChild0 );
    connect( treeView, SIGNAL( showDir( const char * ) ), this, SLOT( slotOpenURL( const char * ) ) );
    connect( treeView, SIGNAL( popupMenu( const char *, const QPoint & )),
	     this, SLOT( slotTreeViewPopupMenu( const char *, const QPoint &)) );
}

void KFileWindow::initStatusBar()
{
    statusBar = new QLabel( this );
    statusBar->setAlignment( AlignLeft | AlignVCenter );
    statusBar->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    statusBar->setGeometry( 0, height() - 20, width() / 2, 20 );
    connect( view, SIGNAL( onURL( const char* ) ), this, SLOT( slotOnURL( const char* ) ) );

    statusBar2 = new QLabel( this );
    statusBar2->setAlignment( AlignLeft | AlignVCenter );
    statusBar2->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    statusBar2->setGeometry( width() / 2, height() - 20, width() / 2 - 1, 20 );
}

void KFileWindow::initAccel()
{
    accel = new QAccel( this );  
  
    accel->insertItem(CTRL  + Key_C, COPY);
    accel->insertItem(CTRL  + Key_V, PASTE);
    accel->insertItem(CTRL  + Key_Insert, COPY);
    accel->insertItem(SHIFT + Key_Insert, PASTE);
    accel->insertItem(ALT   + Key_Q, QUIT);
    accel->insertItem(ALT   + Key_H, HELP);
    
    accel->connectItem( COPY, this, SLOT(slotCopy()) );
    accel->connectItem( PASTE, this, SLOT(slotPaste()) );
    accel->connectItem( QUIT, this, SLOT(slotQuit()) );
}

void KFileWindow::initMenu()
{
    menuNew = new QPopupMenu;
    CHECK_PTR( menuNew );
    menuNew->insertItem( "Folder" );

    connect( menuNew, SIGNAL( activated( int ) ), this, SLOT( slotNewFile( int ) ) );

    QDir d( templatePath );
    const QFileInfoList *list = d.entryInfoList();
    if ( list == 0L )
	printf("ERROR: Template does not exist '%s'\n",templatePath.data());
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
    file->insertItem( "Close", this, SLOT(slotClose()), ALT+Key_C );
    file->insertItem( "Quit",  this, SLOT(slotQuit()), ALT+Key_Q );

    QPopupMenu *edit = new QPopupMenu;
    CHECK_PTR( edit );
    edit->insertItem( "Copy", this, SLOT(slotCopy()), CTRL+Key_C );
    edit->insertItem( "Paste", this, SLOT(slotPaste()), CTRL+Key_V );
    edit->insertItem( "Delete", this, SLOT(slotDelete()) );
    edit->insertSeparator();
    edit->insertItem( "Select", this, SLOT(slotSelect()), CTRL+Key_S );

    mview = new QPopupMenu;
    CHECK_PTR( mview );
    mview->insertItem( "Toggle HTML View", this, SLOT(slotViewHTML()), ALT+Key_H );
    mview->insertItem( "Show Dot Files", this, SLOT(slotShowDot()), ALT+Key_D );
    mview->insertItem( "Visual Schnauzer ON", this, SLOT(slotShowSchnauzer()) );
    mview->insertItem( "Tree View ON", this, SLOT(slotShowTreeView()), ALT+Key_T );
    mview->insertItem( "Update", this, SLOT(slotViewUpdate()), ALT+Key_U );
    mview->insertItem( "Rescan bindings", this, SLOT(slotRescanBindings()) );
    mview->insertSeparator();
    mview->insertItem( "Icon View", this, SLOT(slotIconView()), ALT+Key_I );
    mview->insertItem( "Text View", this, SLOT(slotTextView()) );
    mview->insertItem( "Long View", this, SLOT(slotLongView()), ALT+Key_O );

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
    bookmarkManager.read( bmFile );
    /* bookmarkMenu->insertItem( "Add Bookmark", this, SLOT(slotAddBookmark()) );
    bookmarkMenu->insertSeparator();
    int idStart = BOOKMARK_ID_BASE;
    fillBookmarkMenu( bookmarkManager.getRoot(), bookmarkMenu, idStart ); */

    QPopupMenu *help = new QPopupMenu;
    CHECK_PTR( help );
    help->insertItem( "About", this, SLOT(slotAbout()) );
    help->insertItem( "How can I ...", this, SLOT(slotHelp()) );

    menu = new QMenuBar( this );
    CHECK_PTR( menu );
    menu->insertItem( "File", file );
    menu->insertItem( "Edit", edit );
    menu->insertItem( "View", mview );
    menu->insertItem( "Bookmarks", bookmarkMenu );
    menu->insertItem( "Tool", tool );
    menu->insertSeparator();
    menu->insertItem( "Help", help );
}

void KFileWindow::enableToolbarButton( int id, bool enable )
{
    if ( toolbar == 0L )
	return;
    
    if ( !toolbar->isA( "QButtonGroup" ) )
	return;
    
    QButtonGroup *grp = (QButtonGroup*) toolbar;
    QButton *b = grp->find( id );
    
    b->setEnabled( enable );
    b->setPixmap( toolbarPixmaps[ id*2 + (enable ? 0 : 1) ] );
}

void KFileWindow::initToolBar()
{
    QString file;
    file = KFileType::getIconPath();
    file += "/toolbar/back.xpm";
    toolbarPixmaps[0].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/back_gray.xpm";
    toolbarPixmaps[1].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/forward.xpm";
    toolbarPixmaps[2].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/forward_gray.xpm";
    toolbarPixmaps[3].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/home.xpm";
    toolbarPixmaps[4].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/home.xpm";
    toolbarPixmaps[5].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/reload.xpm";
    toolbarPixmaps[6].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/reload.xpm";
    toolbarPixmaps[7].load( file.data() );

    file = KFileType::getIconPath();
    file += "/toolbar/editcopy.xpm";
    toolbarPixmaps[8].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/editcopy.xpm";
    toolbarPixmaps[9].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/editpaste.xpm";
    toolbarPixmaps[10].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/editpaste.xpm";
    toolbarPixmaps[11].load( file.data() );

    file = KFileType::getIconPath();
    file += "/toolbar/help.xpm";
    toolbarPixmaps[12].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/help.xpm";
    toolbarPixmaps[13].load( file.data() );

    file = KFileType::getIconPath();
    file += "/toolbar/exit.xpm";
    toolbarPixmaps[14].load( file.data() );
    file = KFileType::getIconPath();
    file += "/toolbar/exit_gray.xpm";
    toolbarPixmaps[15].load( file.data() );

    int	pos = 6, buttonWidth, buttonHeight;
    KButton *pb;

    buttonWidth = BUTTON_WIDTH;
    buttonHeight = BUTTON_HEIGHT;

    toolbar = new QButtonGroup( this );
    toolbar->setGeometry( 0, menu->frameGeometry().height(), width(), buttonHeight + 8 );
    
    pb = new KButton( toolbar ,"Back" );
    pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
    connect( pb, SIGNAL( clicked() ), SLOT( slotBack() ) );
    pos += buttonWidth;
    QToolTip::add( pb, "Back" );

    pb = new KButton( toolbar ,"Forward" );
    pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
    connect( pb, SIGNAL( clicked() ), SLOT( slotForward() ) );
    pos += buttonWidth;
    QToolTip::add( pb, "Forward" );

    pb = new KButton( toolbar , "Home" );
    pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
    connect( pb, SIGNAL( clicked() ), SLOT( slotHome() ) );
    pos += buttonWidth + BUTTON_SEPARATION;
    QToolTip::add( pb, "Home" );

    pb = new KButton( toolbar , "Reload" );
    pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
    connect( pb, SIGNAL( clicked() ), SLOT( slotViewUpdate() ) );
    pos += buttonWidth + BUTTON_SEPARATION;
    QToolTip::add( pb, "Reload" );

    pb = new KButton( toolbar , "Copy" );
    pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
    connect( pb, SIGNAL( clicked() ), SLOT( slotCopy() ) );
    pos += buttonWidth;
    QToolTip::add( pb, "Copy" );

    pb = new KButton( toolbar , "Paste" );
    pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
    connect( pb, SIGNAL( clicked() ), SLOT( slotPaste() ) );
    pos += buttonWidth + BUTTON_SEPARATION;
    QToolTip::add( pb, "Paste" );

    pb = new KButton( toolbar, "Help" );
    pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
    connect( pb, SIGNAL( clicked() ), SLOT( slotHelp() ) );
    pos += buttonWidth + BUTTON_SEPARATION;
    QToolTip::add( pb, "Help" );

    pb = new KButton( toolbar, "Stop" );
    pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
    connect( pb, SIGNAL( clicked() ), SLOT( slotStop() ) );
    pos += buttonWidth + BUTTON_SEPARATION;
    QToolTip::add( pb, "Stop" );

    enableToolbarButton( 0, FALSE );
    enableToolbarButton( 1, FALSE );
    enableToolbarButton( 2, TRUE );
    enableToolbarButton( 3, TRUE );
    enableToolbarButton( 4, TRUE );
    enableToolbarButton( 5, TRUE );
    enableToolbarButton( 6, TRUE );
    enableToolbarButton( 7, FALSE );

    topOffset = menu->frameGeometry().height() + toolbar->frameGeometry().height();
}

void KFileWindow::initView()
{
    horz = new QScrollBar( 0, 0, 10, 40, 0, QScrollBar::Horizontal, pannerChild1, "horz" );
    horz->setGeometry( 0, height() - 16 - 20, width() - 16, 16 );

    vert = new QScrollBar( 0, 0, 10, 40, 0, QScrollBar::Vertical, pannerChild1, "vert" );
    vert->setGeometry( width() - 16, topOffset, 16, height() - topOffset - 16- 20 );

    //    horz->setGeometry( 0, pannerChild1->height() - 16, pannerChild1->width() - 16, 16 );
    //    vert->setGeometry( pannerChild1->width() - 16, 0, 16, pannerChild1->height() - 16 );
    horz->setMinimumSize( 16, 16 );
    vert->setMinimumSize( 16, 16 );
    
    view = new KFileView( pannerChild1, "view" );
    CHECK_PTR( view );

    dropZone = new KDNDDropZone( view , DndURL);
    view->setGeometry( 0, topOffset , width() - vert->width() , height() - topOffset - horz->height() - 20 );

    connect( view, SIGNAL( scrollVert( int ) ), SLOT( slotScrollVert( int ) ) );
    connect( view, SIGNAL( scrollHorz( int ) ), SLOT( slotScrollHorz( int ) ) );

    connect( vert, SIGNAL( valueChanged(int) ), view, SLOT( slotScrollVert(int) ) );
    connect( horz, SIGNAL( valueChanged(int) ), view, SLOT( slotScrollHorz(int) ) );

    connect( view, SIGNAL( setTitle(const char *)), this, SLOT( slotTitle(const char *)) );
    connect( view, SIGNAL( doubleClick(const char *, int)), this, SLOT( slotDoubleClick(const char *, int)) );
    connect( view, SIGNAL( popupMenu(const char *, const QPoint & )), this, SLOT( slotPopupMenu(const char *, const QPoint &)) );
    connect( view, SIGNAL( popupMenu2( QStrList&, const QPoint & )), this, SLOT( slotPopupMenu( QStrList&, const QPoint &)) );
    connect( view, SIGNAL( documentChanged() ), SLOT( slotDocumentChanged() ) );

    connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ), this, SLOT( slotDropEvent( KDNDDropZone *) ) );
    connect( dropZone, SIGNAL( dropEnter( KDNDDropZone *) ), this, SLOT( slotDropEnterEvent( KDNDDropZone *) ) );
    connect( dropZone, SIGNAL( dropLeave( KDNDDropZone *) ), this, SLOT( slotDropLeaveEvent( KDNDDropZone *) ) );
}

void KFileWindow::closeEvent( QCloseEvent *e )
{
    printf("Closing\n");
    e->accept();

    delete this;
}

KFileWindow::~KFileWindow()
{
    printf("Deleting KFileWindow\n");
    
    if ( dropZone != 0L )
	delete dropZone;
    dropZone = 0L;
    
    delete manager;
    delete ftpManager;
    delete tarManager;
    delete httpManager;
    printf("->View\n");
    delete view;
    printf("<-View\n");

    printf("Closed\n");
    
    windowList.remove( this );
    printf("Deleted\n");
}

void KFileWindow::slotRescanBindings()
{
    KFileType::clearAll();
    KFileType::init();
    KRootWidget::getKRootWidget()->update();

    KFileWindow *win;
    for ( win = windowList.first(); win != 0L; win = windowList.next() )
	win->slotViewUpdate();
}

void KFileWindow::slotRun()
{
    QString dir = getenv( "HOME" );
    
    if ( actualManager == manager )
    {
	KURL u( manager->getURL() );
	dir = u.path();
    }
    
    QString cmd;
    cmd.sprintf( "cd %s; acli &", dir.data() );
    system( cmd.data() );
}

void KFileWindow::slotTerminal()
{
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup( "Terminal" );
    QString term = config->readEntry( "Terminal" );

    QString dir = getenv( "HOME" );
    
    if ( actualManager == manager )
    {
	KURL u( manager->getURL() );
	dir = u.path();
    }
    
    QString cmd;
    cmd.sprintf( "cd %s; %s &", dir.data(), term.data() );
    system( cmd.data() );
}

void KFileWindow::slotOpenURL( const char *_url )
{
    openURL( _url );
}

void KFileWindow::slotPannerChanged()
{
    if ( !bTreeViewInitialized )
	treeView->initializeTree();  

    printf("Panner has changed !!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    
    resizeEvent( 0L );
}

void KFileWindow::slotScrollVert( int _y )
{
    vert->setValue( _y );
}

void KFileWindow::slotScrollHorz( int _x )
{
    horz->setValue( _x );
}

void KFileWindow::slotIconView()
{
    if ( viewMode == ICON_VIEW )
	return;
    
    viewMode = ICON_VIEW;
    actualManager->openURL( actualManager->getURL(), FALSE );
    refresh( actualManager );
}

void KFileWindow::slotLongView()
{
    if ( viewMode == LONG_VIEW )
	return;
    
    viewMode = LONG_VIEW;
    actualManager->openURL( actualManager->getURL(), FALSE );
    refresh( actualManager );
}

void KFileWindow::slotTextView()
{
    if ( viewMode == TEXT_VIEW )
	return;
    
    viewMode = TEXT_VIEW;
    actualManager->openURL( actualManager->getURL(), FALSE );
    refresh( actualManager );
}

void KFileWindow::slotSelect()
{
    DlgLineEntry l( "Select files:", "", this );
    if ( l.exec() )
    {
	QString pattern = l.getText();
	if ( pattern.length() == 0 )
	    return;

	QRegExp re( pattern, TRUE, TRUE );
	
	view->select( 0L, re, TRUE );
    }
}

void KFileWindow::slotBookmarksChanged()
{
    bookmarkMenu->clear();
    bookmarkMenu->disconnect( this );
    bookmarkMenu->insertItem( "Add Bookmark", this, SLOT(slotAddBookmark()) );
    bookmarkMenu->insertSeparator();
    int idStart = BOOKMARK_ID_BASE;
    fillBookmarkMenu( bookmarkManager.getRoot(), bookmarkMenu, idStart );
}


void KFileWindow::fillBookmarkMenu( KBookmark *parent, QPopupMenu *menu, int &id )
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

void KFileWindow::addBookmark( const char *_title, const char *url )
{
    QString p = getenv( "HOME" );
    QString bmFile = p + "/.kfm.bookmarks.html";
    bookmarkManager.add( _title, url );
    bookmarkManager.write( bmFile );
    
    /* bookmarkMenu->clear();
    bookmarkMenu->disconnect( this );
    bookmarkMenu->insertItem( "Add Bookmark", this, SLOT(slotAddBookmark()) );
    bookmarkMenu->insertSeparator();
    int idStart = BOOKMARK_ID_BASE;
    fillBookmarkMenu( bookmarkManager.getRoot(), bookmarkMenu, idStart ); */
}

void KFileWindow::slotNewFile( int _id )
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
	    QString u = actualManager->getURL();
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
	    QString dest = actualManager->getURL();
	    dest.detach();
	    if ( dest.right( 1 ) != "/" )
		dest += "/";
	    dest += name.data();
	    printf("Command copy '%s' '%s'\n",src.data(),dest.data());
	    job->copy( src.data(), dest.data() );
	}
    }
}

void KFileWindow::slotStop()
{
    if ( actualManager )
	actualManager->stop();
}

void KFileWindow::slotForward()
{
    if ( forwardStack.isEmpty() )
	return;

    QString *s2 = new QString( actualManager->getURL() );
    s2->detach();
    backStack.push( s2 );
    enableToolbarButton( 0, TRUE );

    QString *s = forwardStack.pop();
    if ( forwardStack.isEmpty() )
	enableToolbarButton( 1, FALSE );
    
    stackLock = TRUE;
    openURL( s->data() );
    stackLock = FALSE;

    delete s;
}

void KFileWindow::slotBack()
{
    if ( backStack.isEmpty() )
	return;
    
    QString *s2 = new QString( actualManager->getURL() );
    s2->detach();
    forwardStack.push( s2 );
    enableToolbarButton( 1, TRUE );
    
    QString *s = backStack.pop();
    if ( backStack.isEmpty() )
	enableToolbarButton( 0, FALSE );
    
    stackLock = TRUE;
    openURL( s->data() );
    stackLock = FALSE;

    delete s;
}

void KFileWindow::slotHome()
{  
    QString url = "file:";
    url += QDir::homeDirPath().data();
    
    openURL( url.data() );
}

void KFileWindow::slotAddBookmark()
{
    addBookmark( title.data(), actualManager->getURL() );
}

void KFileWindow::slotBookmarkSelected( int id )
{
    id -= BOOKMARK_ID_BASE;
    
    printf( "Bookmark selected : %i\n",id );
    
    KBookmark *bm = bookmarkManager.getBookmark( id );
    
    if ( bm )
    {
	KURL u( bm->getURL() );
	if ( u.isMalformed() )
	{
		printf("ERROR: Malformed URL\n");
		return;
	}
	
	openURL( bm->getURL() );
    }
}

void KFileWindow::slotToolFind( )
{
    QString cmd = "kfind &";
    system( cmd.data() );
}

void KFileWindow::slotNewWindow( )
{
    KFileWindow *f = new KFileWindow( 0L, 0L, actualManager->getURL() );
    f->show();
}

void KFileWindow::slotViewHTML( )
{
    bViewHTML = !bViewHTML;
    // If KFileManager is active we will do a refresh, since toggeling
    // the above flag may cause the view to change from/to '.kde.html' view
    if ( actualManager == manager )
    {
       manager->openURL( manager->getURL() );
       refresh( manager );
    }
}

void KFileWindow::slotShowDot()
{
    showDot = !showDot;
    if (showDot)
        mview->changeItem("Hide Dot Files",mview->idAt(1));
    else
        mview->changeItem("Show Dot Files",mview->idAt(1));
    
    // If KFileManager is active we will do a refresh.
    if ( actualManager == manager )
    {
	manager->openURL( manager->getURL() );
	refresh( manager );
    }
}

void KFileWindow::slotShowSchnauzer()
{
    visualSchnauzer = !visualSchnauzer;
    if (visualSchnauzer)
        mview->changeItem("Visual Schnauzer OFF",mview->idAt(2));
    else
        mview->changeItem("Visual Schnauzer ON",mview->idAt(2));
    
    // If KFileManager is active we will do a refresh.
    if ( actualManager == manager )
    {
	manager->openURL( manager->getURL() );
	refresh( manager );
    }
}

void KFileWindow::slotShowTreeView()
{
    bTreeView = !bTreeView;
    if ( !bTreeViewInitialized )
    {
	bTreeViewInitialized = TRUE;
	treeView->initializeTree();  
    }
    
    if ( bTreeView )
    {
        mview->changeItem("Tree View OFF",mview->idAt(3));
	panner->setSeparator( 30 );
    }
    else
    {
        mview->changeItem("Tree View ON",mview->idAt(3));
    	panner->setSeparator( 0 );
    }
    
    // If KFileManager is active we will do a refresh.
    if ( actualManager == manager )
    {
	manager->openURL( manager->getURL() );
	refresh( manager );
    }
}

void KFileWindow::slotViewUpdate( )
{
    actualManager->openURL( actualManager->getURL(), TRUE );
    refresh( actualManager );
}

void KFileWindow::slotOpenLocation( )
{
    DlgLineEntry l( "Open Location:", actualManager->getURL(), this );
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
	    printf("ERROR: Malformed URL\n");
	    return;
	}
	
	openURL( url.data() );
    }
}

void KFileWindow::slotMountNotify()
{
    QString u = actualManager->getURL().data();
    
    if ( strncmp( u, "file:" , 5 ) == 0 )
	actualManager->openURL( u.data(), TRUE );
}

void KFileWindow::slotFilesChanged( const char *_url )
{
    printf("Slot called\n");
    
    QString u1 = _url;
    u1.detach();
    if ( u1.right( 1 ) != "/" )
	u1 += "/";
    
    QString u2 = actualManager->getURL().data();
    u2.detach();
    if ( u2.right( 1 ) != "/" )
	u2 += "/";

    printf("Comparing '%s' to '%s'\n",u1.data(), u2.data() );
    if ( u1 == u2 )
	actualManager->openURL( actualManager->getURL().data(), TRUE );
    printf("Changed\n");
}

void KFileWindow::slotDropEnterEvent( KDNDDropZone *_zone )
{
}

void KFileWindow::slotDropLeaveEvent( KDNDDropZone *_zone )
{
}

void KFileWindow::slotDropEvent( KDNDDropZone *_zone )
{
    QPoint p = view->mapFromGlobal( QPoint( _zone->getMouseX(), _zone->getMouseY() ) );
    const char *url = view->getURL( p );
 
    // Dropped over an object ?
    if ( url != 0L )
    {
	KURL u( url );
	if ( u.isMalformed() )
	{
	    printf("ERROR: Drop over malformed URL\n");
	    return;
	}
	
	printf(" Dropped over object\n");
		
	QPoint p( _zone->getMouseX(), _zone->getMouseY() );
	actualManager->dropPopupMenu( _zone, url, &p );
    }
    else // dropped over white ground
    {
	printf("Dropped over white\n");
	
	QPoint p( _zone->getMouseX(), _zone->getMouseY() );
	actualManager->dropPopupMenu( _zone, actualManager->getURL(), &p );
    }
}

void KFileWindow::slotQuit()
{
    if ( !QMessageBox::query( "KFM", "Do you really want to quit" ) )
	return;
    
    printf("QUIT\n");
    QString file = QDir::homeDirPath();
    file += "/.kfm.run";
    unlink( file.data() );

    // Clean up IO stuff
    KIOJob::deleteAllJobs();
    delete ( KIOServer::getKIOServer() );
    
    exit(0);
}

void KFileWindow::slotClose()
{
    close();
}

void KFileWindow::slotCopy()
{
    clipboard.clear();
    view->getSelected( clipboard );
}

void KFileWindow::slotDelete()
{
    QStrList marked;
    
    view->getSelected( marked );

    KIOJob * job = new KIOJob;
    job->del( marked );
}

void KFileWindow::slotPaste()
{
    KIOJob * job = new KIOJob;
    job->copy( clipboard, actualManager->getURL().data() );
}

void KFileWindow::slotAbout()
{
    QMessageBox::message( "About", "KFM 0.6.4\n\r(c) by Torben Weis\n\rweis@kde.org", "Ok" );
}

void KFileWindow::slotHelp()
{
    if ( fork() == 0 )
    {
	QString arg = "file:";
	arg += getenv( "KDEDIR" );
	arg += "/doc/HTML/kfm/kfm.html";
        execlp( "kdehelp", "kdehelp", arg.data(), 0 );
        exit( 1 );
    }
}

void KFileWindow::slotPopupMenu( QStrList &_urls, const QPoint &_point )
{
    printf("slotPopupMenu %i\n",_urls.count());
    actualManager->openPopupMenu( _urls, _point );
}

void KFileWindow::slotPopupMenu( const char *_url, const QPoint &_point )
{
    printf("slotPopupMenu single '%s'\n",_url);
    
    QStrList list;
    list.append( _url );
    actualManager->openPopupMenu( list, _point );
}

void KFileWindow::slotTreeViewPopupMenu( const char *_url, const QPoint &_point )
{
    printf("slotPopupMenu single '%s'\n",_url);
    
    QStrList list;
    list.append( _url );
    manager->openPopupMenu( list, _point );
}

void KFileWindow::slotPopupProperties()
{
    if ( popupFiles.count() != 1 )
    {
	printf("ERROR: Can not open properties for multiple files\n");
	return;
    }

    new Properties( popupFiles.first() );
}

void KFileWindow::slotPopupCopy()
{
    clipboard.clear();
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	clipboard.append( s );
}

void KFileWindow::slotPopupPaste()
{
    if ( popupFiles.count() != 1 )
    {
	QMessageBox::message( "KFM Error", "Can not paste in multiple directories" );
	return;
    }
    
    KIOJob * job = new KIOJob;
    job->copy( clipboard, popupFiles.first() );
}

void KFileWindow::slotPopupDelete()
{
    // This function will emit a signal that causes us to redisplay the
    // contents of our directory if neccessary.
    KIOJob * job = new KIOJob;
    job->del( popupFiles );
}

void KFileWindow::slotPopupNewView()
{
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	KFileWindow *m = new KFileWindow( 0L, 0L, s );
	m->show();
    }
}

void KFileWindow::slotPopupCd()
{
    if ( popupFiles.count() != 1 )
    {
	printf("ERROR: Can not change to multiple directories\n");
	return;
    }
    
    openURL( popupFiles.first() );
}

void KFileWindow::slotTitle( const char *_title )
{
    setCaption( _title );
    title = _title;
    title.detach();
}

const char * KFileWindow::getURL()
{
    return actualManager->getURL();
}

void KFileWindow::openURL( const char *_url )
{
    QString url;
    /*    if ( actualManager != 0L )
    {
	KURL base( actualManager->getURL().data() );
	KURL u( base, _url );
	url = u.url();
    }
    else */
	url = _url;
    
    bool erg = FALSE;
    
    QString old_url = "";
    if ( actualManager != 0L )
	old_url = actualManager->getURL();
    old_url.detach();
    
    if ( strncmp( _url, "tar:", 4 ) == 0 )
    {
	erg = tarManager->openURL( url.data() );
	if ( erg )
	    refresh( tarManager );
    }
    else if ( strncmp( _url, "ftp:", 4 ) == 0 )
    {
	erg = ftpManager->openURL( url.data() );
	if ( erg )
	    refresh( ftpManager );
    }
    else if ( strncmp( _url, "file:", 5 ) == 0 )
    {
	erg = manager->openURL( url.data() ); 
	if ( erg )
	    refresh( manager );
    }
    else if ( strncmp( _url, "http:", 5 ) == 0 )
    {
	erg = httpManager->openURL( url.data() );
	if ( erg )
	    refresh( httpManager );
    }
    else
    {
	// Run the default binding on this if we dont know
	// the protocol
	KFileType::runBinding( url.data() );
	return;
    }
    
    if ( erg && old_url.data()[0] != 0 && !stackLock )
    {
	QString *s = new QString( old_url.data() );
	s->detach();
	backStack.push( s );
	enableToolbarButton( 0, TRUE );
	forwardStack.setAutoDelete( TRUE );
	forwardStack.clear();
	forwardStack.setAutoDelete( FALSE );
	enableToolbarButton( 1, FALSE );
    }
}

void KFileWindow::slotDoubleClick( const char *_url, int _button )
{
    printf("######### Double Click '%s'\n",_url);
    
    if ( _button == MidButton && KIOServer::isDir( _url ) )
    {
	KURL base( actualManager->getURL() );
	KURL u( base, _url );
	QString url = u.url();

	KFileWindow *m = new KFileWindow( 0L, 0L, url.data() );
	m->show();
    }
    
    if ( _button == LeftButton )
	openURL( _url );
}

void KFileWindow::setPopupFiles( QStrList &_files )
{
    popupFiles.copy( _files );
}

/*
 * Used after the contents of the KFileView has changed.
 * The 1st argument is the file manager responsible for the new
 * contents.
 */
void KFileWindow::refresh( KAbstractManager *_man )
{
    actualManager = _man;
}

void KFileWindow::slotDocumentChanged()
{    
    // view->repaint();
    
    // horz->setValue( 0 );
    // vert->setValue( 0 );
    
    if ( view->docWidth() > view->width() )
	horz->setRange( 0, view->docWidth() - view->width() );
    else
	horz->setRange( 0, 0 );
    
    if ( view->docHeight() > view->height() )
	vert->setRange( 0, view->docHeight() - view->height() );	    
    else
	vert->setRange( 0, 0 );
}

void KFileWindow::slotOnURL( const char *_url )
{
    if ( _url == 0L )
    {
	if ( actualManager )
	    statusBar->setText( actualManager->getURL() );
	statusBar2->setText( "" );
    }
    else
    {
	KFileType *typ = KFileType::findType( _url );
	if ( typ )
	{
	    QString com = typ->getComment( _url );
	    if ( !com.isNull() )
		statusBar2->setText( com.data() );
	    else
		statusBar2->setText( "" );
	}
	else
	    statusBar2->setText( "" );
	
	statusBar->setText( _url );
    }
}

void KFileWindow::resizeEvent( QResizeEvent * )
{
    printf("Got resize event !!!!!!!!!!!!!!!!!!!!!!!! %i %i\n",width(),height());
    
    if ( toolbar != 0L )
	toolbar->setGeometry( 0, menu->height(), width(), toolbar->height() );

    if ( panner )
    {
	printf("++++++++++ Resizing Panner ++++++++++++++++++++ %i %i\n",topOffset,width());
	panner->setGeometry( 0, topOffset , width(), height() - topOffset - 20 );
    }
    else
	pannerChild1->setGeometry( 0, topOffset , width(), height() - topOffset - 20 );

    printf("Panner %i is now %i | %i\n", panner->width(), pannerChild0->width(), pannerChild1->width());
    
    /* view->setGeometry( 0, 0, pannerChild1->width() - vert->width() , 
		       pannerChild1->height() - horz->height() );
    horz->setGeometry( 0, pannerChild1->height() - 16,
		       pannerChild1->width() - 16, 16 );
    vert->setGeometry( pannerChild1->width() - 16, 0, 16, pannerChild1->height() - 16 );
    view->parse(); */

    statusBar->setGeometry( 0, height() - 20, width() / 2, 20 );
    statusBar2->setGeometry( width() / 2, height() - 20, width() / 2 - 1, 20 );

    treeView->setGeometry( 0, 0, pannerChild0->width(), pannerChild0->height() );
    
    // view->repaint();
    
    if ( view->docWidth() > view->width() )
	horz->setRange( 0, view->docWidth() - view->width() );

    if ( view->docHeight() > view->height() )
	vert->setRange( 0, view->docHeight() - view->height() );
}

#include "kfmwin.moc"



