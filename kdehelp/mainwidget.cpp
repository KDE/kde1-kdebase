#include "mainwidget.h"
#include "helpwin.h"

#include <qaccel.h>

#include <qmsgbox.h>

#include <kapp.h>

#include <stdlib.h>

#include "mainwidget.moc"

#include <klocale.h>

QList<KHelpMain> KHelpMain::helpWindowList;
KHelpOptionsDialog *KHelpMain::optionsDialog = NULL;
static QString DOCS_PATH;


KHelpMain::KHelpMain(const char *name)
  :KTopLevelWidget(name)
{
	QString kdedir;
	helpwin = new KHelpWindow(this, name);     
	CHECK_PTR(helpwin);

	DOCS_PATH = kapp->kdedir() + "/doc/HTML/";


	createMenu();
	createToolbar();
	createStatusbar();
	setMenu(menu);
	setStatusBar(statusbar);
	addToolBar(toolbar);
	setView(helpwin);

	readConfig();
	menu->show();
	if (showStatusBar)
		enableStatusBar(KStatusBar::Show);
	else
		enableStatusBar(KStatusBar::Hide);

	if (showToolBar) 
		enableToolBar(KToolBar::Show);
	else
		enableToolBar(KToolBar::Hide);
	helpwin->show();

	helpWindowList.setAutoDelete(FALSE);
	helpWindowList.append( this );
	helpWindowList.first()->enableMenuItems();
	enableMenuItems();

	if ( optionsDialog )
	{
		connect( optionsDialog->fontOptions, SIGNAL(fontSize( int )),
			helpwin, SLOT(slotFontSize( int )) );
		connect( optionsDialog->fontOptions,
			SIGNAL(standardFont( const char * )),
			helpwin, SLOT(slotStandardFont( const char * )) );
		connect( optionsDialog->fontOptions,
			SIGNAL(fixedFont( const char * )),
			helpwin, SLOT(slotFixedFont( const char * )) );
		connect( optionsDialog->colorOptions,
			SIGNAL(colorsChanged(const QColor&, const QColor&,
			const QColor&, const QColor&)),
			helpwin, SLOT(slotColorsChanged(const QColor&,
			const QColor&, const QColor&, const QColor&)) );
	}

	setMinimumSize( 200, 100 );

	connect (helpwin, SIGNAL ( enableMenuItems() ), 
			this, SLOT ( slotEnableMenuItems() ) );
	connect (helpwin, SIGNAL ( openNewWindow(const char *) ),
			this, SLOT ( slotNewWindow(const char *) ) );
	connect (helpwin, SIGNAL ( setURL(const char *) ),
			this, SLOT ( slotSetStatusText(const char *) ) );
	connect (helpwin, SIGNAL ( bookmarkChanged(KBookmark *) ),
			this, SLOT ( slotBookmarkChanged(KBookmark *) ) );

	connect( helpwin, SIGNAL( setTitle(const char *) ),
			this, SLOT( slotSetTitle(const char *) ) );

	optionsMenu->setItemChecked( optionsMenu->idAt( 2 ), showToolBar );
	optionsMenu->setItemChecked( optionsMenu->idAt( 3 ), showLocationBar);
	optionsMenu->setItemChecked( optionsMenu->idAt( 4 ), showStatusBar );

	helpwin->setLocationBar(showLocationBar);

	// restore geometry settings
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );
	QString geom = config->readEntry( "Geometry" );
	if ( !geom.isEmpty() )
	{
		int width, height;
		sscanf( geom, "%dx%d", &width, &height );
		resize( width, height );
	}

	// put bookmarks into boormark menu
	helpwin->slotBookmarkChanged();
}


KHelpMain::~KHelpMain()
{
	helpWindowList.removeRef( this );
	delete toolbar;
	delete menu;
}


void KHelpMain::createMenu()
{
	fileMenu = new QPopupMenu;
	CHECK_PTR( fileMenu );
	fileMenu->insertItem( klocale->translate("&New Help Window"), this, SLOT( slotCloneWindow() ),
						CTRL+Key_N );
	fileMenu->insertSeparator();
	fileMenu->insertItem( klocale->translate("&Open File..."), helpwin, SLOT(slotOpenFile()) );
	fileMenu->insertItem( klocale->translate("Open UR&L..."), helpwin, SLOT(slotOpenURL()) );
	fileMenu->insertItem( klocale->translate("&Reload"), helpwin, SLOT(slotReload()) );
	fileMenu->insertSeparator();
	fileMenu->insertItem( klocale->translate("&Search"), helpwin, SLOT(slotSearch()) );
	fileMenu->insertSeparator();
	fileMenu->insertItem( klocale->translate("&Print..."), helpwin, SLOT(slotPrint()), CTRL+Key_P );
	fileMenu->insertSeparator();
	idClose = fileMenu->insertItem(klocale->translate("&Close"),this,SLOT(slotClose()),CTRL+Key_W); // CC :!!!!!
	fileMenu->insertItem( klocale->translate("&Quit"), this, SLOT(slotQuit()), CTRL+Key_Q );

	editMenu = new QPopupMenu;
	CHECK_PTR( editMenu );
	idCopy = editMenu->insertItem(klocale->translate("&Copy"), helpwin, SLOT(slotCopy()), CTRL+Key_C );

	gotoMenu = new QPopupMenu;
	CHECK_PTR( gotoMenu );
	idBack = gotoMenu->insertItem( klocale->translate("&Back"), helpwin, SLOT(slotBack()) );
	idForward = gotoMenu->insertItem( klocale->translate("&Forward"), helpwin, SLOT(slotForward()) );
	gotoMenu->insertSeparator();
	idDir = gotoMenu->insertItem( klocale->translate("&Contents"), helpwin, SLOT(slotDir()) );
	idTop = gotoMenu->insertItem( klocale->translate("&Top"), helpwin, SLOT(slotTop()) );
	idUp = gotoMenu->insertItem( klocale->translate("&Up"), helpwin, SLOT(slotUp()) );
	idPrev = gotoMenu->insertItem( klocale->translate("&Previous"), helpwin, SLOT(slotPrev()) );
	idNext = gotoMenu->insertItem( klocale->translate("&Next"), helpwin, SLOT(slotNext()) );

	bookmarkMenu = new QPopupMenu;
	CHECK_PTR( bookmarkMenu );
	connect( bookmarkMenu, SIGNAL( activated( int ) ),
			helpwin, SLOT( slotBookmarkSelected( int ) ) );
	connect( bookmarkMenu, SIGNAL( highlighted( int ) ),
			helpwin, SLOT( slotBookmarkHighlighted( int ) ) );

	optionsMenu = new QPopupMenu;
	CHECK_PTR( optionsMenu );
	optionsMenu->setCheckable( true );
	optionsMenu->insertItem( klocale->translate("&General Preferences..."), this,
			SLOT(slotOptionsGeneral()) );
	optionsMenu->insertSeparator();
	optionsMenu->insertItem(klocale->translate( "Show &Toolbar"), this,SLOT(slotOptionsToolbar()));

	optionsMenu->insertItem( klocale->translate("Show &Location"), this, SLOT(slotOptionsLocation()) );
	optionsMenu->insertItem( klocale->translate("Show Status&bar"), this,
		SLOT(slotOptionsStatusbar()) );
	optionsMenu->insertSeparator();
	optionsMenu->insertItem( klocale->translate("&Save Options"), this, SLOT(slotOptionsSave()) );

	QPopupMenu *helpMenu = new QPopupMenu;
	CHECK_PTR( helpMenu );
	helpMenu->insertItem( klocale->translate("&Using KDE Help"), this, SLOT(slotUsingHelp()) ); 
	helpMenu->insertSeparator();
	helpMenu->insertItem( klocale->translate("&About"), this, SLOT(slotAbout()) );

	menu = new KMenuBar( this );
	CHECK_PTR( menu );
	menu->insertItem( klocale->translate("&File"), fileMenu );
	menu->insertItem( klocale->translate("&Edit"), editMenu );
	menu->insertItem( klocale->translate("&Goto"), gotoMenu );
	menu->insertItem( klocale->translate("&Bookmarks"), bookmarkMenu );
	menu->insertItem( klocale->translate("&Options"), optionsMenu );
	menu->insertSeparator();
	menu->insertItem( klocale->translate("&Help"), helpMenu );
}


void KHelpMain::createToolbar()
{
	QPixmap pixmap;
	QString pmpath;

	KToolBar *tb = new KToolBar( this );


	pmpath = kapp->findFile( "lib/pics/toolbar/back.xpm" );
	pixmap.load( pmpath );
	tb->insertButton(pixmap, 0, SIGNAL( clicked() ),
		    helpwin, SLOT( slotBack() ),
		    FALSE, klocale->translate("Previous Document"));

	pmpath = kapp->findFile( "lib/pics/toolbar/forward.xpm" );
	pixmap.load( pmpath );
	tb->insertButton(pixmap, 1, SIGNAL( clicked() ),
		    helpwin, SLOT( slotForward() ),
		    FALSE, klocale->translate("Next Document"));

	tb->insertSeparator();

	pmpath = kapp->findFile( "lib/pics/toolbar/prev.xpm" );
	pixmap.load( pmpath );
	tb->insertButton(pixmap, 2, SIGNAL( clicked() ),
		    helpwin, SLOT( slotPrev() ),
		    FALSE, klocale->translate("Previous Node"));

	pmpath = kapp->findFile( "lib/pics/toolbar/next.xpm" );
	pixmap.load( pmpath );
	tb->insertButton(pixmap, 3, SIGNAL( clicked() ),
		    helpwin, SLOT( slotNext() ),
		    FALSE, klocale->translate("Next Node"));

	pmpath = kapp->findFile( "lib/pics/toolbar/up.xpm" );
	pixmap.load( pmpath );
	tb->insertButton(pixmap, 4, SIGNAL( clicked() ),
		    helpwin, SLOT( slotUp() ),
		    FALSE,klocale->translate( "Up one Node"));

	pmpath = kapp->findFile( "lib/pics/toolbar/top.xpm" );
	pixmap.load( pmpath );
	tb->insertButton(pixmap, 5, SIGNAL( clicked() ),
		    helpwin, SLOT( slotTop() ),
		    FALSE, klocale->translate("Top Node"));

	tb->insertSeparator();

	pmpath = kapp->findFile( "lib/pics/toolbar/contents.xpm" );
	pixmap.load( pmpath );
	tb->insertButton(pixmap, 6, SIGNAL( clicked() ),
		    helpwin, SLOT( slotDir() ),
		    FALSE, klocale->translate("Help Contents"));

	pmpath = kapp->findFile( "lib/pics/stop.xpm" );
	pixmap.load( pmpath );
	tb->insertButton(pixmap, 7, SIGNAL( clicked() ),
		    helpwin, SLOT( slotStopProcessing() ),
		    FALSE, klocale->translate("Stop"));

	pmpath = kapp->findFile( "lib/pics/toolbar/reload.xpm" );
	pixmap.load( pmpath );
	tb->insertButton(pixmap, 8, SIGNAL( clicked() ),
		helpwin, SLOT( slotReload() ),
		TRUE, "Reload current document");

	tb->setBarPos( KToolBar::Top );
	toolbar = tb;
}


void KHelpMain::createStatusbar()
{
	KStatusBar *sbar = new KStatusBar(this);
	sbar->insertItem((char*)klocale->translate("In the Constructor!"), 0);
	statusbar = sbar;
}


void KHelpMain::readConfig()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	QString o;
	config->setGroup( "Appearance" );

	o = config->readEntry( "ShowToolBar" );
	if ( !o.isEmpty() && o.find( "No", 0, false ) == 0 )
		showToolBar = false;
	else
		showToolBar = true;

	// now the position for the toolbar
/*
	o = config->readEntry( "ToolBarPos" );
	if ( o.isEmpty() )
		toolBar()->setBarPos(KToolBar::Top);
	else if ("Top" == o) 
		toolBar()->setBarPos(KToolBar::Top);
	else if ("Bottom" == o)
		toolBar()->setBarPos(KToolBar::Bottom);
	else if ("Left" == o)
		toolBar()->setBarPos(KToolBar::Left);
	else if ("Right" == o)
		toolBar()->setBarPos(KToolBar::Right);
	else
		toolBar()->setBarPos(KToolBar::Top);
*/
	o = config->readEntry( "ShowStatusBar" );
	if ( !o.isEmpty() && o.find( "No", 0, false ) == 0 )
		showStatusBar = false;
	else
		showStatusBar = true;

	o = config->readEntry( "ShowLocationBar" );
	if ( !o.isEmpty() && o.find( "No", 0, false ) == 0 )
		showLocationBar = false;
	else
		showLocationBar = true;
}



// enable/disable menu & toolbar options according to current status
//
void KHelpMain::enableMenuItems()
{
    bool val;
//	fileMenu->setItemEnabled( idClose, helpWindowList.count() != 1 );

    val = helpwin->canCurrentlyDo(KHelpWindow::Copy);
	editMenu->setItemEnabled( idCopy, val );

    val = helpwin->canCurrentlyDo(KHelpWindow::GoBack); // history.isback
    gotoMenu->setItemEnabled( idBack, val );
    toolbar->setItemEnabled( 0, val );

    val = helpwin->canCurrentlyDo(KHelpWindow::GoForward); // history.IsForward
    gotoMenu->setItemEnabled( idForward, val );
    toolbar->setItemEnabled( 1, val );

    val = helpwin->canCurrentlyDo(KHelpWindow::GoPrevious); // format->PrevNode()
    gotoMenu->setItemEnabled( idPrev, val );
    toolbar->setItemEnabled( 2, val );

    val = helpwin->canCurrentlyDo(KHelpWindow::GoNext); // format->NextNode()
    gotoMenu->setItemEnabled( idNext, val );
    toolbar->setItemEnabled( 3, val );

    val = helpwin->canCurrentlyDo(KHelpWindow::GoUp); // format->UpNode()
    gotoMenu->setItemEnabled( idUp, val );
    toolbar->setItemEnabled( 4, val );

    val = helpwin->canCurrentlyDo(KHelpWindow::GoTop); // format->UpTop()
    gotoMenu->setItemEnabled( idTop, val );
    toolbar->setItemEnabled( 5, val );

    gotoMenu->setItemEnabled( idDir, TRUE );
    toolbar->setItemEnabled( 6, TRUE );
    
    val = helpwin->canCurrentlyDo(KHelpWindow::Stop); // busy
    toolbar->setItemEnabled( 7, val );
}


void KHelpMain::resizeEvent( QResizeEvent * )
{
	// save size of the application window
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );
	QString geom;
	geom.sprintf( "%dx%d", geometry().width(), geometry().height() );
	config->writeEntry( "Geometry", geom );

	updateRects();
}


int KHelpMain::openURL( const char *URL, bool withHistory)
{
	return helpwin->openURL(URL, withHistory);
}


void KHelpMain::closeEvent (QCloseEvent *)
{
    KHelpMain *win;
    /*    
    delete toolbar; CC: !!! are this destructors called automatically ?
    delete statusbar;
    delete helpwin;
    */
    delete this;
    if ( helpWindowList.isEmpty() )
    {
        qApp->quit();
    }
    else
    for (win = helpWindowList.first(); win != NULL; win = helpWindowList.next())
    	win->enableMenuItems();    
}


void KHelpMain::fillBookmarkMenu(KBookmark *parent, QPopupMenu *menu, int &id)
{
	KBookmark *bm;

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


void KHelpMain::slotClose()
{
	close();
}

void KHelpMain::slotQuit()
{
	while ( helpWindowList.getFirst() )
		helpWindowList.getFirst()->close();
}


void KHelpMain::slotEnableMenuItems()
{
	enableMenuItems();
}


void KHelpMain::slotOptionsGeneral()
{
	if ( !optionsDialog )
	{
		optionsDialog = new KHelpOptionsDialog();
		KHelpMain *w;
		for (w = helpWindowList.first(); w != NULL; w = helpWindowList.next() )
		{
		    connect( optionsDialog->fontOptions, SIGNAL(fontSize(int)),
			 w->helpwin, SLOT(slotFontSize( int )) );
		    connect( optionsDialog->fontOptions,
			SIGNAL(standardFont( const char * )),
			w->helpwin, SLOT(slotStandardFont( const char * )) );
		    connect( optionsDialog->fontOptions,
			SIGNAL(fixedFont( const char * )),
			w->helpwin, SLOT(slotFixedFont( const char * )) );
		    connect( optionsDialog->colorOptions,
			SIGNAL(colorsChanged(const QColor&, const QColor&,
			const QColor&, const QColor&)),
			w->helpwin, SLOT(slotColorsChanged(const QColor&,
			const QColor&, const QColor&, const QColor&)) );
		}
	}

	optionsDialog->show();
}


void KHelpMain::slotOptionsToolbar()
{
  	if ( showToolBar )
	{
	        enableToolBar(KToolBar::Hide);
		showToolBar = false;
	}
	else
	{
	        enableToolBar(KToolBar::Show);
		showToolBar = true;
	}
	
	optionsMenu->setItemChecked( optionsMenu->idAt( 2 ), showToolBar );
	updateRects();
}



void KHelpMain::slotNewWindow(const char *url)
{
	KHelpMain *win;

	win = new KHelpMain;
	win->resize( size() );
	win->openURL( url );
	win->show();
	enableMenuItems();
}



void KHelpMain::slotCloneWindow()
{
	QString url = helpwin->getCurrentURL();
	KHelpMain *win = new KHelpMain;

	win->resize( size() );

	// CC: check, if the Window we are trying to clone is currently 
	// waiting for data. If so, select the title page
	if ( helpwin->canCurrentlyDo(KHelpWindow::Stop) )
	{
		// in that case, select the title page...
		url = "file:";
		url += kapp->kdedir();
		url += "/doc/HTML/index.html";
	}

	win->openURL( url );
	win->show();
	enableMenuItems();
}


void KHelpMain::slotUsingHelp()
{
	KHelpMain *win;

	win = new KHelpMain;
	win->resize( size() );
	win->openURL( "file:" + DOCS_PATH + "kdehelp.html" );
	win->show();
	enableMenuItems();
}


void KHelpMain::slotAbout()
{
	QMessageBox mb;
	mb.setText( klocale->translate("KDE Help System\nVersion ") + QString( KDEHELP_VERSION ) +
			"\n\nMartin Jones <mjones@kde.org>" );
	mb.setCaption( klocale->translate("About KDE Help") );
	mb.show();
}


void KHelpMain::slotOptionsLocation()
{ 
	showLocationBar = !showLocationBar;
	optionsMenu->setItemChecked( optionsMenu->idAt( 3 ), showLocationBar); 
	helpwin->setLocationBar(showLocationBar);
}


void KHelpMain::slotOptionsStatusbar()
{
	showStatusBar = !showStatusBar;
	optionsMenu->setItemChecked( optionsMenu->idAt( 4 ), showStatusBar); 
	enableStatusBar(KStatusBar::Toggle);
}


void KHelpMain::slotSetStatusText(const char *text)
{
	statusbar->changeItem( (char *)text, 0);
	// CC: as an interim solution, discard the "const" since
	// KStatusBar::changeItem takes a (char *) at the moment
}


void KHelpMain::slotBookmarkChanged(KBookmark *parent)
{
	bookmarkMenu->clear();
	bookmarkMenu->insertItem( klocale->translate("&Add Bookmark"), helpwin, SLOT(slotAddBookmark()) );
	bookmarkMenu->insertSeparator();
	int idStart = BOOKMARK_ID_BASE;
	fillBookmarkMenu( parent, bookmarkMenu, idStart );
}


void KHelpMain::slotOptionsSave()
{
	KConfig *config = KApplication::getKApplication()->getConfig();

	config->setGroup( "Appearance" ); 
	config->writeEntry( "ShowToolBar", showToolBar ? "Yes" : "No" );
	switch (toolbar->barPos())
	{
		case KToolBar::Top:
		  config->writeEntry( "ToolBarPos", "Top");
		  break;
		case KToolBar::Bottom:
		  config->writeEntry( "ToolBarPos", "Bottom");
		  break;
		case KToolBar::Left:
		  config->writeEntry( "ToolBarPos", "Left");
		  break;
		case KToolBar::Right:
		  config->writeEntry( "ToolBarPos", "Right");
		  break;
		default:
		  warning("KHelpMain::slotOptionsSave: illegal default in case reached\n");
		  break;
	}
	config->writeEntry( "ShowStatusBar", showStatusBar ? "Yes" : "No" );  
	config->writeEntry( "ShowLocationBar", showLocationBar ? "Yes" : "No" );
}



void KHelpMain::slotSetTitle( const char * _title )
{
	QString appCaption = kapp->getCaption();
	appCaption += " - ";
	appCaption += _title;

	setCaption( appCaption );
}

