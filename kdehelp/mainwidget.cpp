#include "mainwidget.h"
#include "helpwin.h"

#include <qaccel.h>
#include <qmsgbox.h>

#include <kapp.h>
#include <kiconloader.h>
#include <kstdaccel.h>

#include <stdlib.h>

#include <klocale.h>
#include <kwm.h>

// CC: #defines for IDs on the Location Toolbar
#define QLINEDIT_ITEM 1

QList<KHelpMain> KHelpMain::helpWindowList;
KHelpOptionsDialog *KHelpMain::optionsDialog = NULL;
static QString DOCS_PATH;


KHelpMain::KHelpMain(const char *name)
  :KTopLevelWidget(name)
{
	helpwin = new KHelpWindow(this, name);     
	CHECK_PTR(helpwin);

	DOCS_PATH = kapp->kde_htmldir() + "/default/kdehelp/";

	createMenu();
	createLocationbar();
	createToolbar();
	createStatusbar();
	setMenu(menu);
	setStatusBar(statusbar);
	addToolBar(toolbar);
	addToolBar(location);
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
			const QColor&, const QColor&, const bool )),
			helpwin, SLOT(slotColorsChanged(const QColor&,
			const QColor&, const QColor&, const QColor&, const bool)) );
	}

	setMinimumSize( 200, 100 );

	connect (helpwin, SIGNAL ( enableMenuItems() ), 
			this, SLOT ( slotEnableMenuItems() ) );
	connect (helpwin, SIGNAL ( openNewWindow(const char *) ),
			this, SLOT ( slotNewWindow(const char *) ) );
	connect (helpwin, SIGNAL ( setURL(const char *) ),
			this, SLOT ( slotSetStatusText(const char *) ) );
	connect (helpwin, SIGNAL ( setLocation(const char *) ),
		        this, SLOT ( slotSetLocation(const char *) ) );  
	connect (helpwin, SIGNAL ( bookmarkChanged(KBookmark *) ),
			this, SLOT ( slotBookmarkChanged(KBookmark *) ) );

	connect( helpwin, SIGNAL( setTitle(const char *) ),
			this, SLOT( slotSetTitle(const char *) ) );

	optionsMenu->setItemChecked( optionsMenu->idAt( 2 ), showToolBar );
	optionsMenu->setItemChecked( optionsMenu->idAt( 3 ), showLocationBar);
	optionsMenu->setItemChecked( optionsMenu->idAt( 4 ), showStatusBar );

	if (showLocationBar)
	  location->enable(KToolBar::Show);
	else
	  location->enable(KToolBar::Hide);

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
	delete location;
	delete toolbar;
	delete menu;
}


void KHelpMain::createMenu()
{
    KStdAccel stdAccel;
	fileMenu = new QPopupMenu;
	CHECK_PTR( fileMenu );
	fileMenu->insertItem( klocale->translate("&New Help Window"), this,
                        SLOT( slotCloneWindow() ), stdAccel.openNew() );
	fileMenu->insertSeparator();
	fileMenu->insertItem( klocale->translate("&Open File..."), helpwin,
                        SLOT(slotOpenFile()), stdAccel.open() );
//	fileMenu->insertItem( klocale->translate("Open UR&L..."), helpwin,
//                        SLOT(slotOpenURL()) );
	fileMenu->insertItem( klocale->translate("&Reload"), helpwin,
                        SLOT(slotReload()) );
	fileMenu->insertSeparator();
	fileMenu->insertItem( klocale->translate("&Search"), helpwin,
                        SLOT(slotSearch()) );
	fileMenu->insertSeparator();
	fileMenu->insertItem( klocale->translate("&Print..."), helpwin,
                        SLOT(slotPrint()), stdAccel.print() );
	fileMenu->insertSeparator();
	idClose = fileMenu->insertItem(klocale->translate("&Close"),this,
                        SLOT(slotClose()), stdAccel.close()); // CC :!!!!!
	fileMenu->insertItem( klocale->translate("&Quit"), this,
                        SLOT(slotQuit()), stdAccel.quit() );

	editMenu = new QPopupMenu;
	CHECK_PTR( editMenu );
	idCopy = editMenu->insertItem(klocale->translate("&Copy"), helpwin,
                        SLOT(slotCopy()), stdAccel.copy() );
	editMenu->insertItem(klocale->translate("&Find..."), helpwin,
                        SLOT(slotFind()), stdAccel.find() );
	editMenu->insertItem(klocale->translate("Find &next"), helpwin,
                        SLOT(slotFindNext()), Key_F3 );

	gotoMenu = new QPopupMenu;
	CHECK_PTR( gotoMenu );
	idBack = gotoMenu->insertItem( klocale->translate("&Back"), helpwin,
                        SLOT(slotBack()) );
	idForward = gotoMenu->insertItem( klocale->translate("&Forward"), helpwin,
                        SLOT(slotForward()) );
	gotoMenu->insertSeparator();
	idDir = gotoMenu->insertItem( klocale->translate("&Contents"), helpwin,
                        SLOT(slotDir()) );
	idTop = gotoMenu->insertItem( klocale->translate("&Top"), helpwin,
                        SLOT(slotTop()) );
	idUp = gotoMenu->insertItem( klocale->translate("&Up"), helpwin,
                        SLOT(slotUp()) );
	idPrev = gotoMenu->insertItem( klocale->translate("&Previous"), helpwin,
                        SLOT(slotPrev()) );
	idNext = gotoMenu->insertItem( klocale->translate("&Next"), helpwin,
                        SLOT(slotNext()) );

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
	optionsMenu->insertItem(klocale->translate( "Show &Toolbar"), this,
                        SLOT(slotOptionsToolbar()));

	optionsMenu->insertItem( klocale->translate("Show &Location"), this,
                        SLOT(slotOptionsLocation()) );
	optionsMenu->insertItem( klocale->translate("Show Status&bar"), this,
		SLOT(slotOptionsStatusbar()) );
	optionsMenu->insertSeparator();
	optionsMenu->insertItem( klocale->translate("&Save Options"), this,
                        SLOT(slotOptionsSave()) );

	QString at = klocale->translate("KDE Help System\n");
	at+= klocale->translate("Version ");
	at+= KDEHELP_VERSION;
	at+=klocale->translate("\n\nCopyright (c) 1997 Martin Jones <mjones@kde.org>"\
	"\n\nThis program is licensed under the GNU General Public License (GPL)."\
	"\nKDEHelp comes with ABSOLUTELY NO WARRANY to the extent permitted by applicable law.");

	QPopupMenu *helpMenu = kapp->getHelpMenu( true, at );
/*
	QPopupMenu *helpMenu = new QPopupMenu;
	CHECK_PTR( helpMenu );
	helpMenu->insertItem( klocale->translate("&Using KDE Help"), this,
                        SLOT(slotUsingHelp()) ); 
	helpMenu->insertSeparator();
	helpMenu->insertItem( klocale->translate("&About"), this, SLOT(slotAbout()) );
*/
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


	pixmap = kapp->getIconLoader()->loadIcon( "back.xpm" );
	tb->insertButton(pixmap, 0, SIGNAL( clicked() ),
		    helpwin, SLOT( slotBack() ),
		    FALSE, klocale->translate("Previous Document"));

	pixmap = kapp->getIconLoader()->loadIcon( "forward.xpm" );
	tb->insertButton(pixmap, 1, SIGNAL( clicked() ),
		    helpwin, SLOT( slotForward() ),
		    FALSE, klocale->translate("Next Document"));

	tb->insertSeparator();

	pixmap = kapp->getIconLoader()->loadIcon( "prev.xpm" );
	tb->insertButton(pixmap, 2, SIGNAL( clicked() ),
		    helpwin, SLOT( slotPrev() ),
		    FALSE, klocale->translate("Previous Node"));

	pixmap = kapp->getIconLoader()->loadIcon( "next.xpm" );
	tb->insertButton(pixmap, 3, SIGNAL( clicked() ),
		    helpwin, SLOT( slotNext() ),
		    FALSE, klocale->translate("Next Node"));

	pixmap = kapp->getIconLoader()->loadIcon( "up.xpm" );
	tb->insertButton(pixmap, 4, SIGNAL( clicked() ),
		    helpwin, SLOT( slotUp() ),
		    FALSE,klocale->translate( "Up one Node"));

	pixmap = kapp->getIconLoader()->loadIcon( "top.xpm" );
	tb->insertButton(pixmap, 5, SIGNAL( clicked() ),
		    helpwin, SLOT( slotTop() ),
		    FALSE, klocale->translate("Top Node"));

	tb->insertSeparator();

	pixmap = kapp->getIconLoader()->loadIcon( "contents.xpm" );
	tb->insertButton(pixmap, 6, SIGNAL( clicked() ),
		    helpwin, SLOT( slotDir() ),
		    FALSE, klocale->translate("Help Contents"));

	pixmap = kapp->getIconLoader()->loadIcon( "reload.xpm" );
	tb->insertButton(pixmap, 7, SIGNAL( clicked() ),
		helpwin, SLOT( slotReload() ),
		TRUE, klocale->translate( "Reload current document" ) );

	pixmap = kapp->getIconLoader()->loadIcon( "stop.xpm" );
	tb->insertButton(pixmap, 8, SIGNAL( clicked() ),
		    helpwin, SLOT( slotStopProcessing() ),
		    FALSE, klocale->translate("Stop"));

	tb->setBarPos( KToolBar::Top );
	toolbar = tb;
}


void KHelpMain::createLocationbar()
{
  KToolBar *tb = new KToolBar(this);

  tb->insertLined("", QLINEDIT_ITEM, SIGNAL( returnPressed() ), this, SLOT( slotLocationEntered() ) );
  tb->setFullWidth(TRUE);
  tb->setItemAutoSized( QLINEDIT_ITEM, TRUE);
  tb->enable(KToolBar::Show);

  location = tb;
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

	o = config->readEntry( "ToolBarPos" );
	if ( o.isEmpty() )
		toolbar->setBarPos(KToolBar::Top);
	else if ("Top" == o) 
		toolbar->setBarPos(KToolBar::Top);
	else if ("Bottom" == o)
		toolbar->setBarPos(KToolBar::Bottom);
	else if ("Left" == o)
		toolbar->setBarPos(KToolBar::Left);
	else if ("Right" == o)
		toolbar->setBarPos(KToolBar::Right);
	else if ("Floating" == o)
		toolbar->setBarPos(KToolBar::Floating);
	else
		toolbar->setBarPos(KToolBar::Top);

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

	o = config->readEntry( "LocationBarPos" );
	if ( o.isEmpty() )
		location->setBarPos(KToolBar::Top);
	else if ("Top" == o) 
		location->setBarPos(KToolBar::Top);
	else if ("Bottom" == o)
		location->setBarPos(KToolBar::Bottom);
	else if ("Left" == o)
		location->setBarPos(KToolBar::Left);
	else if ("Right" == o)
		location->setBarPos(KToolBar::Right);
	else if ("Floating" == o)
		location->setBarPos(KToolBar::Floating);
	else
		location->setBarPos(KToolBar::Top);
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
    toolbar->setItemEnabled( 8, val );
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

void KHelpMain::saveProperties( KConfig *config )
{
    config->writeEntry( "URL", helpwin->getCurrentURL() );
}

void KHelpMain::readProperties( KConfig *config )
{
    QString url;

    url = config->readEntry( "URL" );

    if ( !url.isEmpty() )
	openURL( url, true );
}

int KHelpMain::openURL( const char *URL, bool withHistory)
{
    return helpwin->openURL(URL, withHistory);
}


void KHelpMain::closeEvent (QCloseEvent *)
{
    KHelpMain *win;

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
			const QColor&, const QColor&, const bool)),
			w->helpwin, SLOT(slotColorsChanged(const QColor&,
			const QColor&, const QColor&, const QColor&, const bool)) );
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
		url += kapp->kde_htmldir();
		url += "/default/kdehelp/main.html";
	}

	win->openURL( url );

	if ( !helpwin->canCurrentlyDo(KHelpWindow::Stop) )
    {
        win->helpWindow()->setHistory( helpwin->getHistory() );
        win->enableMenuItems();
    }

	win->show();
	enableMenuItems();
}


void KHelpMain::slotUsingHelp()
{
	KHelpMain *win;

	win = new KHelpMain;
	win->resize( size() );
	win->openURL( "file:" + DOCS_PATH + "index.html" );
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
	location->enable(KToolBar::Toggle);
	updateRects();
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
	        case KToolBar::Floating:
		  config->writeEntry( "ToolBarPos", "Floating");
		  break;
		default:
		  warning("KHelpMain::slotOptionsSave: illegal default in case reached\n");
		  break;
	}

	switch (location->barPos())
	{
		case KToolBar::Top:
		  config->writeEntry( "LocationBarPos", "Top");
		  break;
		case KToolBar::Bottom:
		  config->writeEntry( "LocationBarPos", "Bottom");
		  break;
		case KToolBar::Left:
		  config->writeEntry( "LocationBarPos", "Left");
		  break;
		case KToolBar::Right:
		  config->writeEntry( "LocationBarPos", "Right");
		  break;
	        case KToolBar::Floating:
		  config->writeEntry( "LocationBarPos", "Floating");
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



void KHelpMain::slotSetLocation(const char *url)
{
  location->setLinedText(QLINEDIT_ITEM, url);
}



void KHelpMain::slotLocationEntered()
{
  helpwin->openURL(location->getLinedText(QLINEDIT_ITEM), LeftButton);
}
