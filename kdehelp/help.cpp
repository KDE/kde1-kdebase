//-----------------------------------------------------------------------------
//
// KDE help System
//
// (C) Martin R. Jones 1996
//

#include <iostream.h>
#include <qapp.h>
#include <qaccel.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qfiledlg.h>
#include <qmsgbox.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <kapp.h>
#include <Kconfig.h>
#include <kurl.h>
#include <kfm.h>
#include "cgi.h"
#include "kbutton.h"
#include "help.h"

#include "dbnew.h"

#include "help.moc"

QList<KHelpWindow> helpWindowList;
static QString QUOTE( "\"" );
static QString PIXDIR;
static QString DOCS_PATH;

//-----------------------------------------------------------------------------

KOpenURLDialog::KOpenURLDialog( QWidget *parent, const char *name )
	: QDialog( parent, name )
{
	setCaption( "KDE Help: Open URL" );

	QLabel *label = new QLabel( "Open URL", this );
	label->setGeometry( 20, 20, 50, 20 );
	
	lineEdit = new QLineEdit( this );
	lineEdit->setGeometry( 30+label->width(), 20, 150, 20 );
	lineEdit->setFocus();

	QPushButton *openButton = new QPushButton( "Open", this );
	openButton->setGeometry( 20, 60, 50, 30 );
	openButton->setDefault( TRUE );
	connect( openButton, SIGNAL(clicked()), SLOT(openPressed()) );

	QPushButton *cancelButton = new QPushButton( "Cancel", this );
	cancelButton->setGeometry( 230-50, 60, 50, 30 );
	connect( cancelButton, SIGNAL(clicked()), SLOT(reject()) );
}

void KOpenURLDialog::openPressed()
{
	emit openURL( lineEdit->text(), LeftButton );
	accept();
}

//-----------------------------------------------------------------------------

KLocationBar::KLocationBar( QWidget *parent, const char *name )
	: QFrame( parent, name )
{
	QBoxLayout *gm = new QBoxLayout( this, QBoxLayout::LeftToRight, 4 );

	QLabel *label = new QLabel( " Location:", this );
	label->setMaximumSize( label->sizeHint() );
	gm->addWidget( label );

	lineEdit = new QLineEdit( this );
	connect( lineEdit, SIGNAL( returnPressed() ), SLOT( slotReturnPressed() ) );
	gm->addWidget( lineEdit );
}

void KLocationBar::slotReturnPressed()
{
	emit openURL( lineEdit->text(), LeftButton );
}

//-----------------------------------------------------------------------------

// statics
QString KHelpWindow::newURL;
KHelpOptionsDialog *KHelpWindow::optionsDialog = NULL;
KBookmarkManager KHelpWindow::bookmarkManager;
int  KHelpWindow::fontBase = 3;
QString KHelpWindow::standardFont;
QString KHelpWindow::fixedFont;

KHelpWindow::KHelpWindow( QWidget *parent, const char *name )
	: QWidget( parent, name ), history(50), format(&html)
{
	toolbar = NULL;
	openURLDialog = NULL;
	remotePage = NULL;
	CGIServer = NULL;
	busy = false;
	scrollTo = 0;

	char *kdedir = getenv( "KDEDIR" );

	if ( kdedir )
	{
		PIXDIR = kdedir;
		PIXDIR += "/lib/pics/";
		DOCS_PATH = kdedir;
		DOCS_PATH += "/doc/HTML/";
	}
//	else
//		QMessageBox::message( "KDE Help", "Please set a $KDEDIR environment variable" );

	readOptions();

	man = new cMan;
	info = new cInfo;

	accel = new QAccel( this );
	accel->insertItem( CTRL + Key_C,		COPY );
	accel->insertItem( CTRL + Key_Insert,	COPY );
	accel->insertItem( CTRL + Key_N,		NEW );
	accel->insertItem( CTRL + Key_W,		CLOSE );
	accel->insertItem( CTRL + Key_Q,		QUIT );

	accel->connectItem( COPY, this, SLOT(slotCopy()) );

	createMenu();

	createToolbar();
	if ( showToolBar )
		toolbar->show();
	else
		toolbar->hide();

	locationBar = new KLocationBar( this );
	locationBar->setFrameStyle( QFrame::Panel | QFrame::Raised );
	if ( showLocationBar )
		locationBar->show();
	else
		locationBar->hide();
	connect( locationBar, SIGNAL(openURL( const char *, int )),
						SLOT(slotURLSelected( const char *, int )) );

	view = new KHTMLWidget( this, NULL, PIXDIR );
	CHECK_PTR( view );
	view->setDefaultFontBase( fontBase );
	view->setStandardFont( standardFont );
	view->setFixedFont( fixedFont );
	view->setURLCursor( upArrowCursor );
	view->setFocusPolicy( QWidget::StrongFocus );
	view->setFocus();

	vert = new QScrollBar( 0, 0, 12, view->height(), 0,
			QScrollBar::Vertical, this, "vert" );

	horz = new QScrollBar( 0, 0, 24, view->width(), 0,
			QScrollBar::Horizontal, this, "horz" );

	statusBar = new QLabel( this );
	statusBar->setAlignment( AlignLeft | AlignVCenter );
	statusBar->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	if ( showStatusBar )
		statusBar->show();
	else
		statusBar->hide();

	dropZone = new KDNDDropZone( view , DndURL );
	connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ), this,
			SLOT( slotDropEvent( KDNDDropZone *) ) );


	connect( &bookmarkManager, SIGNAL( changed() ), 
			SLOT( slotBookmarkChanged() ) );

	connect( view, SIGNAL( scrollVert( int ) ), SLOT( slotScrollVert( int ) ) );
	connect( view, SIGNAL( scrollHorz( int ) ), SLOT( slotScrollHorz( int ) ) );

	connect( view, SIGNAL( setTitle(const char *) ),
			SLOT( slotSetTitle(const char *) ) );
	connect( view, SIGNAL( URLSelected( const char *, int ) ),
			SLOT( slotURLSelected(const char *, int ) ) );
	connect( view, SIGNAL( onURL( const char * ) ),
			SLOT( slotOnURL(const char * ) ) );
	connect( view, SIGNAL( popupMenu( const char *, const QPoint & ) ),
			SLOT( slotPopupMenu(const char *, const QPoint & ) ) );
	connect( view, SIGNAL( formSubmitted( const char *, const char * ) ),
			SLOT( slotFormSubmitted( const char *, const char * ) ) );
	connect( view, SIGNAL( resized( const QSize & ) ),
			SLOT( slotViewResized( const QSize & ) ) );
//	connect( view, SIGNAL( imageRequest( const char * ) ), 
//			SLOT( slotImageRequest( const char * ) ) );

	connect( vert, SIGNAL(valueChanged(int)), view, SLOT(slotScrollVert(int)) );
	connect( horz, SIGNAL(valueChanged(int)), view, SLOT(slotScrollHorz(int)) );

	connect( view, SIGNAL( documentChanged() ),
			SLOT( slotDocumentChanged() ) );

	connect( view, SIGNAL( documentDone() ),
			SLOT( slotDocumentDone() ) );


	// load bookmarks
	QString p = getenv( "HOME" );
	QString bmFile = p + "/.kdehelp/bookmarks.html";
	bookmarkManager.read( bmFile );

	helpWindowList.append( this );
	helpWindowList.first()->enableMenuItems();
	enableMenuItems();

	if ( optionsDialog )
	{
		connect( optionsDialog->fontOptions, SIGNAL(fontSize( int )),
			SLOT(slotFontSize( int )) );
		connect( optionsDialog->fontOptions,
			SIGNAL(standardFont( const char * )),
			SLOT(slotStandardFont( const char * )) );
		connect( optionsDialog->fontOptions,
			SIGNAL(fixedFont( const char * )),
			SLOT(slotFixedFont( const char * )) );
	}

	setMinimumSize( 200, 100 );

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

	layout();
}

KHelpWindow::~KHelpWindow()
{
	helpWindowList.removeRef( this );
	if (openURLDialog)
		delete openURLDialog;
	delete man;
	delete info;
//	delete dropZone;		// causes SEG FAULT. Why?
}


void KHelpWindow::readOptions()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );

	QString bf = config->readEntry( "BaseFontSize" );
	if ( !bf.isEmpty() )
		fontBase = bf.toInt();

	standardFont = config->readEntry( "StandardFont" );
	if ( standardFont.isEmpty() )
		standardFont = "times";

	fixedFont = config->readEntry( "FixedFont" );
	if ( fixedFont.isEmpty() )
		fixedFont = "courier";

	QString o = config->readEntry( "ShowToolBar" );
	if ( !o.isEmpty() && o.find( "no", 0, false ) == 0 )
		showToolBar = false;
	else
		showToolBar = true;

	o = config->readEntry( "ShowStatusBar" );
	if ( !o.isEmpty() && o.find( "no", 0, false ) == 0 )
		showStatusBar = false;
	else
		showStatusBar = true;

	o = config->readEntry( "ShowLocationBar" );
	if ( !o.isEmpty() && o.find( "no", 0, false ) == 0 )
		showLocationBar = false;
	else
		showLocationBar = true;
}

// Open a URL.  Initiate remote transfer if necessary
int KHelpWindow::openURL( const char *URL, bool withHistory )
{
	char location[256];
	int pos, rv = 1, viewPos = view->yOffset();
	const char *colon;
	bool isRemote = false;

	scrollTo = 0;
	ref = "";

	if ( !URL )
		return 1;

	if ( URL[0] == '\"')	// skip leading "
		URL++;

	fullURL = URL;

	if ( ( pos = fullURL.findRev( '\"' ) ) > 0)
		fullURL.truncate( pos );

	// is this actually a file:
	if ( (fullURL[0] == '/' && fullURL[1] != '/') || fullURL[0] == '~' )
		fullURL = "file:" + fullURL;

	// extract reference
	int refPos = fullURL.findRev( '#' );
	int prevPos = currentURL.findRev( '#' );
	prevPos = prevPos > 0 ? prevPos : 0;
	if ( refPos >= 0 )
	{
		ref = fullURL.right( fullURL.length() - refPos - 1 );
		fullURL.truncate( refPos );

		if ( ( refPos == 0 || fullURL == currentURL.left( prevPos ) ) &&
			!ref.isEmpty() )
		{
			if ( !view->gotoAnchor( ref ) )
				return 1;
			if ( prevPos > 0 )
				currentURL.truncate( prevPos + 1 );
			currentURL += ref;
			if ( withHistory )
			{
				if ( history.Current() )
					history.Current()->setOffset( viewPos );
				history.Add( new KPageInfo( currentURL, 0 ) );
			}
			enableMenuItems();
			statusBar->setText( currentURL );
			locationBar->setLocation( currentURL );
			return 0;
		}
	}

	// if this is a relative location, then use path of current URL
	colon = strchr( fullURL, ':' );
	if ( !colon )
	{
		int i = currentURL.findRev( '/' );
		if ( i >= 0 )
		{
			fullURL = currentURL.left( i + 1 ) + fullURL;
			colon = strchr( fullURL, ':' );
		}
		else
			return 1;
	}

	cout << "Opening: " << fullURL << endl;

	strcpy( location, colon+1 );

	QCursor oldCursor = cursor();
	setCursor( waitCursor );

	if ( fullURL.find( "file:" ) >= 0 )
	{
		// use internal CGI sever for local CGI stuff
		if ( fullURL.find( "cgi-bin" ) > 0 )
		{
			rv = runCGI( fullURL );
			isRemote = true;
		}
		else
			rv = openFile( location );
	}
	else if ( fullURL.find( "info:" ) >= 0 )
	{
		if ( location[0] != '(' )
			fullURL = "info:(" + currentInfo + ')' + location;
		else
		{
			currentInfo = location+1;
			int i = currentInfo.find( ')' );
			if (i > 0)
				currentInfo.truncate(i);
		}
		if ( ( rv = info->ReadLocation( location ) ) == 0 )
			formatInfo();
	}
	else if ( fullURL.find( "man:" ) >= 0 )
	{
		if ( ( rv = man->ReadLocation( location ) ) == 0 )
			formatMan();
	}
	else if ( fullURL.find( "http:" ) >= 0 )
	{// until http in kdehelp works properly, let kfm handle it.
		QString cmd = "kfmclient exec ";
		cmd += fullURL;
		cmd += " Open";
		system( cmd );
/*
		rv = openRemote( fullURL );
		isRemote = true;
*/
	}
	else if ( fullURL.find( "ftp:" ) >= 0 )
	{
		QString cmd = "kfmclient exec ";
		cmd += fullURL;
		cmd += " Open";
		system( cmd );
	}
	else if ( fullURL.find( "mailto:" ) >= 0 )
	{
		QMessageBox::message("KDE Help","Waiting for kmail to be finished...");
	}

	if ( !rv )
	{
		busy = true;

		currentURL = fullURL.copy();
		if ( ref.length() > 0 )
		{
			currentURL += '#';
			currentURL += ref;
		}
		if ( withHistory )
		{
			if ( history.Current() )
			{
				printf( "Setting offset: %d\n", viewPos );
				history.Current()->setOffset( viewPos );
			}
			history.Add( new KPageInfo( currentURL, 0 ) );
		}
		enableMenuItems();

		if ( !isRemote )
		{
			view->parse();
			horz->setValue( 0 );
			statusBar->setText( currentURL );
			locationBar->setLocation( currentURL );
		}
	}

	setCursor( oldCursor );

	return rv;
}

// actually read the file (must be on local disk)
int KHelpWindow::openFile( const QString &location )
{
	int rv = 1;
	QString fileName;

	if ( location[0] == '~' )
	{
		fileName = getenv( "HOME" );
		fileName += '/';
		fileName += location.data() + 1;
	}
	else
		fileName = location.copy();

	switch ( detectFileType( fileName ) )
	{
		case HTMLFile:
			rv = openHTML( fileName );
			break;

		case InfoFile:
			currentInfo = fileName.copy();
			if ( ( rv = info->ReadLocation( "(" + fileName + ")" ) ) == 0 )
				formatInfo();
			break;
			
		case ManFile:
			if ( ( rv = man->ReadLocation( fileName ) ) == 0 )
				formatMan();
			break;

		case CannotOpenFile:
			{
				QMessageBox mb;
				mb.setText( "Cannot open: " + fileName );
				mb.setButtonText( "Oops!" );
				mb.show();
			}
			break;

		default:
			{
				QMessageBox mb;
				mb.setText( "Unknown format: " + fileName );
				mb.setButtonText( "Oops!" );
				mb.show();
			}
	}

	if ( rv == 0 )
		view->parse();

	return rv;
}

// convert from general info description to HTML
int KHelpWindow::formatInfo( int bodyOnly )
{
	QString converted;
	bool inMenu = FALSE;
	int rowCount;
	cNodeLine *curr = info->node.nodeLines.GotoHead();

	if ( !bodyOnly )
	{
		view->setGranularity( 200 );
		view->begin();
		view->write( "<html><head><title>" );
		view->write( info->GetTitle() );
		view->write( "</title></head><body bgcolor=#FFFFFF>" );
	}

	view->write( "<pre>" );

	while (curr)
	{
		if ( inMenu )
		{
			if ( ( curr->type != INFO_NODEMENU &&
				 curr->type != INFO_NODEMENUCONT ) || rowCount <= 0 )
			{
				view->write( "</td></tr></table>" );
				inMenu = FALSE;
			}
		}

		switch (curr->type)
		{
			case INFO_NODETEXT:
				convertSpecial( ((cNodeText *)curr)->text(), converted );
				view->write( converted );
				view->write( "\n" );
				break;

			case INFO_NODEINLINE:
				convertSpecial( ((cNodeText *)curr)->text(), converted );
				view->write( converted );
				break;

			case INFO_NODEMENU:
				if ( !inMenu )
				{
					view->write( "<table>" );
					rowCount = 50;
					inMenu = TRUE;
				}
				else
				{
					view->write( "</td></tr>" );
					rowCount--;
				}
				view->write( "<tr><td valign=top>" );
				view->write( "<a href=\"info:" );
				view->write( ((cNodeMenu *)curr)->node );
				view->write( "\">" );
				convertSpecial( ((cNodeMenu *)curr)->name, converted );
				view->write( converted );
				view->write( "</a>   </td><td valign=top>" );
				convertSpecial( ((cNodeMenu *)curr)->desc, converted );
				view->write( converted );
				view->write( "\n" );
				break;

			case INFO_NODEMENUCONT:
				convertSpecial( ((cNodeText *)curr)->text(), converted );
				view->write( converted );
				view->write( "\n" );
				break;

			case INFO_NODEXREF:
				view->write( "<a href=\"info:" );
				view->write( ((cNodeXRef *)curr)->node );
				view->write( "\">" );
				convertSpecial( ((cNodeXRef *)curr)->name, converted );
				view->write( converted );
				view->write( "</a>" );
				break;

			case INFO_NODEHEADING:
				{
					char level;
					switch ( ((cNodeHeading *)curr)->type )
					{
						case INFO_HEADING1:
							level = '1';
							break;

						case INFO_HEADING2:
							level = '2';
							break;

						case INFO_HEADING3:
							level = '3';
							break;
					}

					QString str;
					str.sprintf("</pre><br><h%c>", level);
					view->write( str );
					convertSpecial( ((cNodeText *)curr)->text(), converted );
					view->write( converted );
					view->write( "<br>" );
					str.sprintf("</h%c>", level);
					view->write( str );
					view->write( "<pre>" );
				}
				break;
		}
		curr = curr->next;
	}

	view->write( "</pre>" );

	if ( inMenu )
	{
		view->write( "</td></tr></table>" );
		inMenu = FALSE;
	}

	if ( !bodyOnly )
	{
		view->write("</body></html>");
		view->end();
	}

	format = info;

	return 0;
}

// convert from general man description to HTML
int KHelpWindow::formatMan( int bodyOnly )
{
	cManBase *curr = man->manList.GotoHead();
	QString converted;
	bool inDir = FALSE;

	if ( !bodyOnly )
	{
		view->setGranularity( 500 );
		view->begin();
		view->write( "<html><head><title>" );
		view->write( man->GetTitle() );
		view->write( "</title></head><body bgcolor=#FFFFFF>" );
	}

	view->write( "<pre>" );

	while (curr)
	{
		if ( inDir && curr->type != MAN_DIR )
		{
			view->write( "</cell>" );
			inDir = FALSE;
		}

		switch (curr->type)
		{
			case MAN_TEXT:
				convertSpecial( curr->text, converted );
				view->write( converted );
				break;

			case MAN_UNDERLINE:
				view->write( "<em>" );
				convertSpecial( curr->text, converted );
				view->write( converted );
				view->write( "</em>" );
				break;

			case MAN_BOLD:
				view->write( "<b>" );
				convertSpecial( curr->text, converted );
				view->write( converted );
				view->write( "</b>" );
				break;

			case MAN_HEADING1:
				view->write( "</pre><h1>" );
				convertSpecial( curr->text, converted );
				view->write( converted );
				view->write( "</h1><pre>" );
				break;

			case MAN_HEADING2:
				view->write( "</pre><h2>" );
				convertSpecial( curr->text, converted );
				view->write( converted );
				view->write( "</h2><pre>" );
				break;

			case MAN_HEADING3:
				view->write( "<b><i>" );
				convertSpecial( curr->text, converted );
				view->write( converted );
				view->write( "</i></b>" );
				break;

			case MAN_DIR:
				if ( !inDir )
				{
					if ( fontBase > 4 )
						view->write( "<grid width=400 align=left>" );
					else
						view->write( "<grid width=300 align=left>" );
					inDir = TRUE;
				}

				view->write( "<cell>" );
				view->write( "<a href=\"man:" );
				view->write( ((cManDir *)curr)->page );
				view->write( "\">" );
				convertSpecial( curr->text, converted );
				view->write( converted );
				view->write( "</a>   " );

				if ( ((cManDir *)curr)->desc )
				{
					convertSpecial( ((cManDir *)curr)->desc, converted );
					view->write( converted );
				}
				view->write( "</cell>" );
				break;

			case MAN_XREF:
				view->write( "<a href=\"man:" );
				view->write( ((cManXRef *)curr)->page );
				view->write( "\">" );
				convertSpecial( curr->text, converted );
				view->write( converted );
				view->write( "</a>" );
				break;

			case MAN_CR:
				view->write( "\n" );
				break;
		}
		curr = curr->next;
	}

	view->write( "</pre>" );

	if ( !bodyOnly )
	{
		view->write("</body></html>");
		view->end();
	}

	format = man;

	return 0;
}

// open a HTML file
int KHelpWindow::openHTML( const char *location )
{
	if ( html.ReadLocation( location ) )
		return 1;

	char buffer[256];
	int val;
	QFile file( location) ;

	if ( !file.open(IO_ReadOnly) )
		return 1;

	view->setGranularity( 500 );
	view->begin( fullURL );

	do
	{
		val = file.readLine( buffer, 256 );
		view->write(buffer);
	}
	while ( !file.atEnd() );

	view->end();

	format = &html;

	return 0;
}

// initiate a remote transfer
int KHelpWindow::openRemote( const char *_url )
{
	remoteFile = _url;
	remoteFile.detach();
	KURL u( remoteFile.data() );
	if ( u.isMalformed() )
	{
		QMessageBox::message( "Error", "Malformed URL", "Ok" );
		return 1;
	}

	if ( remotePage != NULL )
	{
		delete remotePage;
		KURL u( localFile );
		unlink( u.path() );
	}

	remotePage = new KFM;

	if ( !remotePage->isOK() )
	{
		QMessageBox::message ("Error", "Could not start or find KFM", "Ok");
		delete remotePage;
		remotePage = NULL;
		return 1;
	}

	localFile.sprintf( "file:/tmp/kdehelpXXXXXX" );
	mktemp( localFile.data() );
	connect( remotePage, SIGNAL( finished() ), this, SLOT( slotRemoteDone() ) );
	remotePage->copy( remoteFile.data(), localFile.data() );
	printf( "Getting file\n" );

	return 0;
}

int KHelpWindow::runCGI( const char *_url )
{
	printf( "Form submitted: %s\n", _url );

	if ( CGIServer )
	{
		delete CGIServer;
		KURL u( localFile );
		unlink( u.path() );
	}

	CGIServer = new KCGI;

	connect( CGIServer, SIGNAL( finished() ), this, SLOT( slotCGIDone() ) );

	localFile.sprintf( "file:/tmp/kdehelpXXXXXX" );
	mktemp( localFile.data() );

	CGIServer->get( _url, localFile, "Get" );

	return 0;
}

// attempt to detect the file type
KHelpWindow::FileType KHelpWindow::detectFileType( const QString &fileName )
{
	FileType type = UnknownFile;

	// attempt to identify file type
	// This is all pretty dodgey at the moment...
	if ( fileName.find( ".htm" ) > 0 )
	{
		return HTMLFile;
	}
	else if ( fileName.find (".info" ) > 0 )
	{
		return InfoFile;
	}
	else
	{
		// open file and attempt to identify
		char fname[256];

		strcpy( fname, fileName );

		if ( strstr( fileName, ".gz" ) )
		{
			char sysCmd[256];
			sprintf( fname, "/tmp/khelpXXXXXX" );
			mktemp( fname );
			sprintf(sysCmd, "gzip -cd %s > %s", (const char *)fileName, fname);
			system( sysCmd );
		}
		QFile file( fname );
		QString buffer(256);
		if ( file.open(IO_ReadOnly) )
		{
			for ( int i = 0; !file.atEnd(), i < 80; i++ )
			{
				file.readLine( buffer.data(), 256 );
				if ( buffer.find( "<HTML>", 0, FALSE ) >= 0 ||
					buffer.find( "<BODY", 0, FALSE ) >= 0 )
				{
					type = HTMLFile;
					break;
				}
				else if ( buffer.find( ".TH" ) >= 0 ||
						buffer.find( ".so" ) >= 0 )
				{
					type = ManFile;
					break;
				}
				else if ( buffer.find( "Indirect:" ) >= 0 ||
						buffer.find( "Node:" ) >= 0 )
				{
					type = InfoFile;
					break;
				}
			}
		}
		else
			type = CannotOpenFile;
		if ( strstr( fileName, ".gz" ) )
			remove( fname );
	}

	return type;
}

// turn special characters into their HTML equivalents
//
void KHelpWindow::convertSpecial( const char *buffer, QString &converted )
{
	QString special = "<>&\"";
	const char *ptr, *replace[] = { "&lt;", "&gt;", "&amp;", "&quot;", 0 };
	int pos;

	converted = "";

	if ( !buffer )
		return;
	ptr = buffer;

	while ( *ptr )
	{
		if ( (pos = special.find( *ptr )) >= 0 )
			converted += replace[pos];
		else
			converted += *ptr;
		ptr++;
	}
}

// enable/disable menu & toolbar options according to current status
//
void KHelpWindow::enableMenuItems()
{
//	fileMenu->setItemEnabled( idClose, helpWindowList.count() != 1 );

	gotoMenu->setItemEnabled( idBack, history.IsBack() );
	enableToolbarButton( 0, history.IsBack() );
	gotoMenu->setItemEnabled( idForward, history.IsForward() );
	enableToolbarButton( 1, history.IsForward() );
	gotoMenu->setItemEnabled( idPrev, format->PrevNode() );
	enableToolbarButton( 2, format->PrevNode() );
	gotoMenu->setItemEnabled( idNext, format->NextNode() );
	enableToolbarButton( 3, format->NextNode() );
	gotoMenu->setItemEnabled( idUp, format->UpNode() );
	enableToolbarButton( 4, format->UpNode() );
	gotoMenu->setItemEnabled( idTop, format->TopNode() );
	enableToolbarButton( 5, format->TopNode() );
	gotoMenu->setItemEnabled( idDir, TRUE );
	enableToolbarButton( 6, TRUE );

	enableToolbarButton( 7, busy );
}

void KHelpWindow::createMenu()
{
	fileMenu = new QPopupMenu;
	CHECK_PTR( fileMenu );
	fileMenu->insertItem( "&New Help Window", this, SLOT(slotNewWindow()),
						CTRL+Key_N );
	fileMenu->insertSeparator();
	fileMenu->insertItem( "&Open File...", this, SLOT(slotOpenFile()) );
	fileMenu->insertItem( "Open UR&L...", this, SLOT(slotOpenURL()) );
	fileMenu->insertSeparator();
	fileMenu->insertItem( "&Search", this, SLOT(slotSearch()) );
	fileMenu->insertSeparator();
	idClose = fileMenu->insertItem("&Close",this,SLOT(slotClose()),CTRL+Key_W);
	fileMenu->insertItem( "&Quit", qApp, SLOT(quit()), CTRL+Key_Q );

	QPopupMenu *editMenu = new QPopupMenu;
	CHECK_PTR( editMenu );
	idCopy = editMenu->insertItem("&Copy", this, SLOT(slotCopy()), CTRL+Key_C );
	editMenu->setItemEnabled( idCopy, FALSE );

	gotoMenu = new QPopupMenu;
	CHECK_PTR( gotoMenu );
	idBack = gotoMenu->insertItem( "&Back", this, SLOT(slotBack()) );
	idForward = gotoMenu->insertItem( "&Forward", this, SLOT(slotForward()) );
	gotoMenu->insertSeparator();
	idDir = gotoMenu->insertItem( "&Contents", this, SLOT(slotDir()) );
	idTop = gotoMenu->insertItem( "&Top", this, SLOT(slotTop()) );
	idUp = gotoMenu->insertItem( "&Up", this, SLOT(slotUp()) );
	idPrev = gotoMenu->insertItem( "&Previous", this, SLOT(slotPrev()) );
	idNext = gotoMenu->insertItem( "&Next", this, SLOT(slotNext()) );

	bookmarkMenu = new QPopupMenu;
	CHECK_PTR( bookmarkMenu );
	connect( bookmarkMenu, SIGNAL( activated( int ) ),
			SLOT( slotBookmarkSelected( int ) ) );
	connect( bookmarkMenu, SIGNAL( highlighted( int ) ),
			SLOT( slotBookmarkHighlighted( int ) ) );

	optionsMenu = new QPopupMenu;
	CHECK_PTR( optionsMenu );
	optionsMenu->setCheckable( true );
	optionsMenu->insertItem( "&General Preferences...", this,
			SLOT(slotOptionsGeneral()) );
	optionsMenu->insertSeparator();
	optionsMenu->insertItem( "Show &Toolbar", this,SLOT(slotOptionsToolbar()));
	optionsMenu->setItemChecked( optionsMenu->idAt( 2 ), showToolBar );
	optionsMenu->insertItem( "Show &Location", this,
		SLOT(slotOptionsLocation()) );
	optionsMenu->setItemChecked( optionsMenu->idAt( 3 ), showLocationBar );
	optionsMenu->insertItem( "Show Status&bar", this,
		SLOT(slotOptionsStatusbar()) );
	optionsMenu->setItemChecked( optionsMenu->idAt( 4 ), showStatusBar );
	optionsMenu->insertSeparator();
	optionsMenu->insertItem( "&Save Options", this, SLOT(slotOptionsSave()) );

	QPopupMenu *helpMenu = new QPopupMenu;
	CHECK_PTR( helpMenu );
	helpMenu->insertItem( "&Using KDE Help", this, SLOT(slotUsingHelp()) );
	helpMenu->insertSeparator();
	helpMenu->insertItem( "&About", this, SLOT(slotAbout()) );

	menu = new QMenuBar( this );
	CHECK_PTR( menu );
	menu->insertItem( "&File", fileMenu );
	menu->insertItem( "&Edit", editMenu );
	menu->insertItem( "&Goto", gotoMenu );
	menu->insertItem( "&Bookmarks", bookmarkMenu );
	menu->insertItem( "&Options", optionsMenu );
	menu->insertSeparator();
	menu->insertItem( "&Help", helpMenu );
}

// enable/disable toolbar button and set appropriate pixmap
//
void KHelpWindow::enableToolbarButton( int id, bool enable )
{
	QButton *b = toolbar->find( id );

	b->setEnabled( enable );
	b->setPixmap( toolbarPixmaps[ id*2 + (enable ? 0 : 1) ] );
}

void KHelpWindow::createToolbar()
{
	int	pos = 6, buttonWidth, buttonHeight;
	KButton *pb;

	if ( toolbar )
		delete toolbar;

	QString file = KApplication::findFile( "lib/pics/toolbar/back.xpm" );
	toolbarPixmaps[0].load( file );
	file = KApplication::findFile( "lib/pics/toolbar/back_gray.xpm" );
	toolbarPixmaps[1].load( file );
	file = KApplication::findFile( "lib/pics/toolbar/forward.xpm" );
	toolbarPixmaps[2].load( file );
	toolbarPixmaps[3].load( PIXDIR + "toolbar/forward_gray.xpm");
	toolbarPixmaps[4].load( PIXDIR + "toolbar/prev.xpm");
	toolbarPixmaps[5].load( PIXDIR + "toolbar/prev_gray.xpm");
	toolbarPixmaps[6].load( PIXDIR + "toolbar/next.xpm");
	toolbarPixmaps[7].load( PIXDIR + "toolbar/next_gray.xpm");
	toolbarPixmaps[8].load( PIXDIR + "toolbar/up.xpm");
	toolbarPixmaps[9].load( PIXDIR + "toolbar/up_gray.xpm");
	toolbarPixmaps[10].load( PIXDIR + "toolbar/top.xpm");
	toolbarPixmaps[11].load( PIXDIR + "toolbar/top_gray.xpm");
	toolbarPixmaps[12].load( PIXDIR + "toolbar/contents.xpm");
	toolbarPixmaps[13].load( PIXDIR + "toolbar/contents_gray.xpm");
	toolbarPixmaps[14].load( PIXDIR + "stop.xpm");
	toolbarPixmaps[15].load( PIXDIR + "stop_gray.xpm");

	buttonWidth = BUTTON_WIDTH;
	buttonHeight = BUTTON_HEIGHT;

	toolbar = new QButtonGroup( this );
	toolbar->setFrameStyle( QFrame::Panel | QFrame::Raised );

	pb = new KButton( toolbar );
	pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
	connect( pb, SIGNAL( clicked() ), SLOT( slotBack() ) );
	pos += buttonWidth;
	QToolTip::add( pb, "Previous Document" );

	pb = new KButton( toolbar );
	pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
	connect( pb, SIGNAL( clicked() ), SLOT( slotForward() ) );
	pos += buttonWidth + BUTTON_SEPARATION;
	QToolTip::add( pb, "Next Document" );

	pb = new KButton( toolbar );
	pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
	connect( pb, SIGNAL( clicked() ), SLOT( slotPrev() ) );
	pos += buttonWidth;
	QToolTip::add( pb, "Previous Node" );

	pb = new KButton( toolbar );
	pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
	connect( pb, SIGNAL( clicked() ), SLOT( slotNext() ) );
	pos += buttonWidth;
	QToolTip::add( pb, "Next Node" );

	pb = new KButton( toolbar );
	pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
	connect( pb, SIGNAL( clicked() ), SLOT( slotUp() ) );
	pos += buttonWidth;
	QToolTip::add( pb, "Up one Node" );

	pb = new KButton( toolbar );
	pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
	connect( pb, SIGNAL( clicked() ), SLOT( slotTop() ) );
	pos += buttonWidth + BUTTON_SEPARATION;
	QToolTip::add( pb, "Top Node" );

	pb = new KButton( toolbar );
	pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
	connect( pb, SIGNAL( clicked() ), SLOT( slotDir() ) );
	pos += buttonWidth + BUTTON_SEPARATION;
	QToolTip::add( pb, "Help Contents" );

	pb = new KButton( toolbar );
	pb->setGeometry( pos, 3, buttonWidth, buttonHeight );
	connect( pb, SIGNAL( clicked() ), SLOT( slotStopProcessing() ) );
	pos += buttonWidth + BUTTON_SEPARATION;
	QToolTip::add( pb, "Stop" );
}

// add bookmarks to the Bookmarks menu
//
void KHelpWindow::fillBookmarkMenu(KBookmark *parent, QPopupMenu *menu, int &id)
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

QString KHelpWindow::getPrefix()
{
	QString prefix = "";

	int pos = currentURL.find( ':' );

	if ( pos > 0 )
		prefix = currentURL.left( pos+1 );

	return prefix;
}

QString KHelpWindow::getLocation()
{
	QString loc = "";

	int pos1 = currentURL.find( ':' ) + 1;
	int pos2 = currentURL.find( '#' );

	if ( pos2 < 0 )
		pos2 = currentURL.length();
	
	loc = currentURL.mid( pos1, pos2-pos1 );

	return loc;
}

void KHelpWindow::openNewWindow( const char *url )
{
	KHelpWindow *helpWin = new KHelpWindow;
	helpWin->resize( size() );
	helpWin->openURL( url );
	helpWin->show();
	enableMenuItems();
}

void KHelpWindow::addBookmark( const char *_title, const char *url )
{
	QString p = getenv( "HOME" );
	QString bmFile = p + "/.kdehelp/bookmarks.html";
	bookmarkManager.add( _title, url );
	bookmarkManager.write( bmFile );
}

void KHelpWindow::layout()
{
	int top = menu->frameGeometry().height();
	int bottom = height();

	if ( toolbar->isVisible() )
	{
		toolbar->setGeometry( 0, top, width(), BUTTON_HEIGHT + 6 );
		top += BUTTON_HEIGHT + 6;
	}

	if ( locationBar->isVisible() )
	{
		locationBar->setGeometry( 0, top, width(), 32 );
		top += 32;
	}

	if ( statusBar->isVisible() )
	{
		bottom -= STATUSBAR_HEIGHT;
		statusBar->setGeometry( 0, bottom, width(), STATUSBAR_HEIGHT );
	}

	bottom -= SCROLLBAR_WIDTH;
	horz->setGeometry( 0, bottom,
					width() - SCROLLBAR_WIDTH, SCROLLBAR_WIDTH );

	vert->setGeometry( width()-SCROLLBAR_WIDTH, top,
					SCROLLBAR_WIDTH, bottom-top );
	view->setGeometry( 0, top, width() - SCROLLBAR_WIDTH, bottom-top );
}

void KHelpWindow::slotNewWindow()
{
	openNewWindow( currentURL );
}

void KHelpWindow::slotOpenFile()
{
	QString fileName = QFileDialog::getOpenFileName();

	if ( !fileName.isNull() )
	{
		QString url = "file:";
		url += fileName;
		openURL( url );
	}
}

void KHelpWindow::slotOpenURL()
{
	if (!openURLDialog)
	{
		openURLDialog = new KOpenURLDialog();
		connect( openURLDialog, SIGNAL(openURL( const char *, int )),
								SLOT(slotURLSelected( const char *, int )) );
	}

	openURLDialog->show();
}

void KHelpWindow::slotSearch()
{
	QString searchURL = DOCS_PATH + "/ksearch.html";
	openURL( searchURL );
}

void KHelpWindow::slotClose()
{
	close();
}

void KHelpWindow::slotCopy()
{
	cout << "copy selected" << endl;
}

void KHelpWindow::slotBack()
{
	if ( history.Current() )
		history.Current()->setOffset( view->yOffset() );

	KPageInfo *p = history.Back();

	if ( p )
	{
		if ( !openURL( p->getUrl(), false ) )
			scrollTo = p->getOffset();
	}
}

void KHelpWindow::slotForward()
{
	if ( history.Current() )
		history.Current()->setOffset( view->yOffset() );

	KPageInfo *p = history.Forward();

	if ( p )
	{
		if ( !openURL( p->getUrl(), false ) )
			scrollTo = p->getOffset();
	}
}

void KHelpWindow::slotDir()
{
	QString u = "file:";
	u += DOCS_PATH;
	u += "index.html";

	openURL( u );
}

void KHelpWindow::slotTop()
{
	openURL( QUOTE + getPrefix() + format->TopNode() + QUOTE );
}

void KHelpWindow::slotUp()
{
	openURL( QUOTE + getPrefix() + format->UpNode() + QUOTE );
}

void KHelpWindow::slotPrev()
{
	openURL( QUOTE + getPrefix() + format->PrevNode() + QUOTE );
}

void KHelpWindow::slotNext()
{
	openURL( QUOTE + getPrefix() + format->NextNode() + QUOTE );
}

void KHelpWindow::slotAddBookmark()
{
	addBookmark( title, currentURL );
}

void KHelpWindow::slotBookmarkSelected( int id )
{
	id -= BOOKMARK_ID_BASE;

	KBookmark *bm = bookmarkManager.getBookmark( id );

	if ( bm )
		openURL( bm->getURL() );
}

void KHelpWindow::slotBookmarkHighlighted( int id )
{
	id -= BOOKMARK_ID_BASE;

	KBookmark *bm = bookmarkManager.getBookmark( id );

	if ( bm )
		statusBar->setText( bm->getURL() );
	else
		statusBar->setText( currentURL );
}

void KHelpWindow::slotBookmarkChanged()
{
	bookmarkMenu->clear();
	bookmarkMenu->insertItem( "&Add Bookmark", this, SLOT(slotAddBookmark()) );
	bookmarkMenu->insertSeparator();
	int idStart = BOOKMARK_ID_BASE;
	fillBookmarkMenu( bookmarkManager.getRoot(), bookmarkMenu, idStart );
}

void KHelpWindow::slotStopProcessing()
{
	if ( !busy )
		return;

	if ( remotePage )
	{
		delete remotePage;
		KURL u( localFile );
		unlink( u.path() );
		remotePage = NULL;
	}

	if ( CGIServer )
	{
		delete CGIServer;
		KURL u( localFile );
		unlink( u.path() );
		CGIServer = NULL;
	}

	view->stopParser();

	busy = false;
	enableMenuItems();
}

void KHelpWindow::slotOptionsGeneral()
{
	if ( !optionsDialog )
	{
		optionsDialog = new KHelpOptionsDialog();
		KHelpWindow *w;
		for (w = helpWindowList.first(); w != NULL; w = helpWindowList.next() )
		{
			connect( optionsDialog->fontOptions, SIGNAL(fontSize( int )),
				SLOT(slotFontSize( int )) );
			connect( optionsDialog->fontOptions,
				SIGNAL(standardFont( const char * )),
				SLOT(slotStandardFont( const char * )) );
			connect( optionsDialog->fontOptions,
				SIGNAL(fixedFont( const char * )),
				SLOT(slotFixedFont( const char * )) );
		}
	}

	optionsDialog->show();
}

void KHelpWindow::slotOptionsToolbar()
{
	if ( showToolBar )
	{
		toolbar->hide();
		showToolBar = false;
	}
	else
	{
		toolbar->show();
		showToolBar = true;
	}

	optionsMenu->setItemChecked( optionsMenu->idAt( 2 ), showToolBar );
	layout();
}

void KHelpWindow::slotOptionsLocation()
{
	if ( showLocationBar )
	{
		locationBar->hide();
		showLocationBar = false;
	}
	else
	{
		locationBar->show();
		showLocationBar = true;
	}

	optionsMenu->setItemChecked( optionsMenu->idAt( 3 ), showLocationBar );
	layout();
}

void KHelpWindow::slotOptionsStatusbar()
{
	if ( showStatusBar )
	{
		statusBar->hide();
		showStatusBar = false;
	}
	else
	{
		statusBar->show();
		showStatusBar = true;
	}

	optionsMenu->setItemChecked( optionsMenu->idAt( 4 ), showStatusBar );
	layout();
}

void KHelpWindow::slotOptionsSave()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );
	config->writeEntry( "ShowToolBar", showToolBar ? "Yes" : "No" );
	config->writeEntry( "ShowStatusBar", showStatusBar ? "Yes" : "No" );
	config->writeEntry( "ShowLocationBar", showLocationBar ? "Yes" : "No" );
}

void KHelpWindow::slotUsingHelp()
{
	KHelpWindow *helpWin = new KHelpWindow;
	helpWin->resize( size() );
	helpWin->openURL( "file:" + DOCS_PATH + "kdehelp.html" );
	helpWin->show();
	enableMenuItems();
}

void KHelpWindow::slotAbout()
{
	QMessageBox mb;
	mb.setText( "KDE Help System\nVersion " + QString( KDEHELP_VERSION ) +
			"\n\nMartin Jones <mjones@kde.org>" );
	mb.setCaption( "About KDE Help" );
	mb.show();
}

void KHelpWindow::slotSetTitle( const char *_title )
{
	title = _title;
	setCaption( QString( "KDE Help - " ) + title );
}

void KHelpWindow::slotURLSelected( const char *URL, int button )
{
	if ( button == LeftButton )
	{
		openURL( URL );
		view->setFocus();
	}
	else if ( button == MidButton )
	{
		KHelpWindow *helpWin = new KHelpWindow;
		helpWin->openURL( URL );
		helpWin->show();
		enableMenuItems();
	}
}

void KHelpWindow::slotOnURL( const char *url )
{
	if ( url )
		statusBar->setText( url );
	else
		statusBar->setText( currentURL );
}

void KHelpWindow::slotFormSubmitted( const char *method, const char *url )
{
	openURL( url );
}

void KHelpWindow::slotPopupMenu( const char *url, const QPoint &p )
{
	int id;
	static QPopupMenu *popup = NULL;

	if ( popup )
		delete popup;

	popup = new QPopupMenu;

	id = popup->insertItem( "Back", this, SLOT( slotBack() ) );
	popup->setItemEnabled( id, history.IsBack() );
	id = popup->insertItem( "Forward", this, SLOT( slotForward() ) );
	popup->setItemEnabled( id, history.IsForward() );
	
	if ( url )
	{
		popup->insertSeparator();
		if ( strstr( url, "info:" ) )
		{
			const char *ptr = strchr( url, ':' ) + 1;
			if ( *ptr != '(' )
				newURL = "info:(" + currentInfo + ')' + ptr;
		}
		else
		{
			int pos;
			if ( url[0] == '\"' )
				url++;
			newURL = url;
			if ( ( pos  = newURL.findRev( '\"' ) ) > 0)
				newURL.truncate( pos );
		}
		popup->insertItem( "Open this Link", this, SLOT(slotPopupOpenURL()) );
		popup->insertItem( "Add Bookmark", this, SLOT(slotPopupAddBookmark()) );
		popup->insertItem( "Open in new Window", this, SLOT(slotPopupOpenNew()) );
	}

	popup->popup( p );
}

void KHelpWindow::slotDropEvent( KDNDDropZone *zone )
{
	printf("Received Drop %s\n", zone->getData() );

	KURL kurl( zone->getData() );
	if ( !kurl.isMalformed() )
		openURL( kurl.url() );
}
/*
void KHelpWindow::slotImageRequest( const char * _url )
{
	printf( "Image requested: %s\n", _url );

	remoteFile = _url;
	remoteFile.detach();
	KURL u( remoteFile.data() );
	if ( u.isMalformed() )
	{
		QMessageBox::message( "Error", "Malformed URL", "Ok" );
		return 1;
	}

	QString file;
	file.sprintf( "file:/tmp/kdehelpXXXXXX" );
	mktemp( file.data() );

	RemoteImage *img = new RemoteImage( _url, file );
	remoteImage.append( img );

	connect( img, SIGNAL( finished() ), this, SLOT( slotRemoteImageDone() ) );
	img->copy( remoteFile.data(), file.data() );
}
*/
void KHelpWindow::slotRemoteDone()
{
	printf( "KDEHelp: Remote Done\n" );

	KURL u( localFile.data() );
	openFile( u.path() );
	statusBar->setText( currentURL );
	locationBar->setLocation( currentURL );

	delete remotePage;
	remotePage = NULL;

	unlink( u.path() );
}

void KHelpWindow::slotCGIDone()
{
	printf( "KDEHelp: CGI Done\n" );

	KURL u( localFile );
	openFile( u.path() );
	statusBar->setText( currentURL );
	locationBar->setLocation( currentURL );

	delete CGIServer;
	CGIServer = NULL;

	unlink( u.path() );
}

void KHelpWindow::slotScrollVert( int _y )
{
	vert->setValue( _y );
}

void KHelpWindow::slotScrollHorz( int _x )
{
	horz->setValue( _x );
}

void KHelpWindow::slotBackgroundColor( const QColor &col )
{
	view->setBackgroundColor( col );
}

void KHelpWindow::slotFontSize( int size )
{
	fontBase = size;
	view->setDefaultFontBase( size );
	view->parse();
	busy = true;
	enableMenuItems();
}

void KHelpWindow::slotStandardFont( const char *n )
{
	standardFont = n;
	view->setStandardFont( n );
	view->parse();
	busy = true;
	enableMenuItems();
}

void KHelpWindow::slotFixedFont( const char *n )
{
	fixedFont = n;
	view->setFixedFont( n );
	view->parse();
	busy = true;
	enableMenuItems();
}

void KHelpWindow::slotPopupOpenURL()
{
	openURL( newURL );
}

void KHelpWindow::slotPopupAddBookmark()
{
	addBookmark( newURL, newURL );
}

void KHelpWindow::slotPopupOpenNew()
{
	openNewWindow( newURL );
}


void KHelpWindow::slotViewResized( const QSize &s )
{
	QApplication::setOverrideCursor( waitCursor );

	vert->setSteps( 12, view->height() - 20 );
	horz->setSteps( 24, view->width() );

	if ( view->docHeight() > view->height() )
		vert->setRange( 0, view->docHeight() - view->height() );
	else
		vert->setRange( 0, 0 );

	// we need to parse again if the width of the widget changes
	if ( !currentURL.isEmpty() && s.width() != viewWidth )
	{
		view->parse();
		busy = true;
		enableMenuItems();
		viewWidth = s.width();
	}

	QApplication::restoreOverrideCursor();
}

void KHelpWindow::resizeEvent( QResizeEvent * )
{
	layout();

	// save size of the application window
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );
	QString geom;
	geom.sprintf( "%dx%d", geometry().width(), geometry().height() );
	config->writeEntry( "Geometry", geom );
}

void KHelpWindow::closeEvent( QCloseEvent * )
{
	delete this;

	if ( helpWindowList.isEmpty() )
	{
		qApp->quit();
	}

	KHelpWindow *win;
	for (win = helpWindowList.first(); win != NULL; win = helpWindowList.next())
		win->enableMenuItems();
}

void KHelpWindow::slotDocumentChanged()
{
	if ( view->docHeight() > view->height() )
		vert->setRange( 0, view->docHeight() - view->height() );
	else
		vert->setRange( 0, 0 );

	if ( view->docWidth() > view->width() )
		horz->setRange( 0, view->docWidth() - view->width() );
	else
		horz->setRange( 0, 0 );
}

void KHelpWindow::slotDocumentDone()
{
	if ( scrollTo )
	{
		if ( scrollTo > view->docHeight() - view->height() )
			scrollTo = view->docHeight() - view->height();
		view->slotScrollVert( scrollTo );
		vert->setValue( scrollTo );
	}
	else if ( ref.isEmpty() )
	{
		vert->setValue( 0 );
	}
	else
	{
		if ( !view->gotoAnchor( ref ) )
			vert->setValue( 0 );
	}

	busy = false;
	enableMenuItems();
}

//-----------------------------------------------------------------------------

