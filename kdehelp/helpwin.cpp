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
#include <qclipbrd.h>
#include <kapp.h>
#include <Kconfig.h>
#include <kurl.h>
#include <kfm.h>
#include "cgi.h"
#include "kbutton.h"
#include "helpwin.h"

#include "dbnew.h"

#include "helpwin.moc"

// for selection
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>


static QString QUOTE( "\"" );
static QString PIXDIR;
static QString DOCS_PATH;

#define hand_width 16
#define hand_height 16

static unsigned char hand_bits[] = {
	0x00,0x00,0xfe,0x01,0x01,0x02,0x7e,0x04,0x08,0x08,0x70,0x08,0x08,0x08,0x70,
	0x14,0x08,0x22,0x30,0x41,0xc0,0x20,0x40,0x12,0x80,0x08,0x00,0x05,0x00,0x02,
	0x00,0x00};
static unsigned char hand_mask_bits[] = {
	0xfe,0x01,0xff,0x03,0xff,0x07,0xff,0x0f,0xfe,0x1f,0xf8,0x1f,0xfc,0x1f,0xf8,
	0x3f,0xfc,0x7f,0xf8,0xff,0xf0,0x7f,0xe0,0x3f,0xc0,0x1f,0x80,0x0f,0x00,0x07,
	0x00,0x02};

//-----------------------------------------------------------------------------

KOpenURLDialog::KOpenURLDialog( QWidget *parent, const char *name )
	: QDialog( parent, name )
{
	setCaption( "KDE Help: Open URL" );

	QLabel *label = new QLabel( "Open URL", this );
	label->setGeometry( 20, 20, 60, 20 );
	
	lineEdit = new QLineEdit( this );
	lineEdit->setGeometry( 30+label->width(), 20, 140, 20 );
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
KBookmarkManager KHelpWindow::bookmarkManager;
int  KHelpWindow::fontBase = 3;
QString KHelpWindow::standardFont;
QString KHelpWindow::fixedFont;

KHelpWindow::KHelpWindow( QWidget *parent, const char *name )
	: QWidget( parent, name ), history(50), format(&html)
{
	openURLDialog = NULL;
	remotePage = NULL;
	CGIServer = NULL;
	busy = false;
	scrollTo = 0;
	rmbPopup = NULL;

	char *kdedir = getenv( "KDEDIR" );
	if (!kdedir)
		kdedir = "/usr/local/kde";
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

	locationBar = new KLocationBar( this );
	locationBar->setFrameStyle( QFrame::Panel | QFrame::Raised );

	connect( locationBar, SIGNAL(openURL( const char *, int )),
						SLOT(slotURLSelected( const char *, int )) );

	QBitmap cb( hand_width, hand_height, hand_bits, TRUE );
	QBitmap cm( hand_width, hand_height, hand_mask_bits, TRUE );
	QCursor handCursor( cb, cm, 0, 0 );

	view = new KHTMLWidget( this, NULL, PIXDIR );
	CHECK_PTR( view );
	view->setDefaultFontBase( fontBase );
	view->setStandardFont( standardFont );
	view->setFixedFont( fixedFont );
	view->setURLCursor( handCursor );
	view->setFocusPolicy( QWidget::StrongFocus );
	view->setFocus();
	view->installEventFilter( this );
	view->setUpdatesEnabled( true );

	vert = new QScrollBar( 0, 0, 12, view->height(), 0,
			QScrollBar::Vertical, this, "vert" );

	horz = new QScrollBar( 0, 0, 24, view->width(), 0,
			QScrollBar::Horizontal, this, "horz" );

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
	connect( view, SIGNAL( textSelected( bool ) ), 
			SLOT( slotTextSelected( bool ) ) );
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

	setLocationBar(TRUE); // CC: LocationBar is on by default
	layout();
}


KHelpWindow::~KHelpWindow()
{
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
			emit enableMenuItems();
            emit setURL( currentURL );
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
				history.Current()->setOffset( viewPos );
			}
			history.Add( new KPageInfo( currentURL, 0 ) );
		}
		emit enableMenuItems();

		if ( !isRemote )
		{
			view->parse();
			horz->setValue( 0 );
			emit setURL( currentURL );
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

	return rv;
}


// convert from general info description to HTML
int KHelpWindow::formatInfo( int bodyOnly )
{
	QString converted;
	bool inMenu = FALSE;
	int rowCount = 100;
	cNodeLine *curr = info->node.nodeLines.GotoHead();

	if ( !bodyOnly )
	{
		view->setGranularity( 200 );
		view->begin();
		view->write( "<html><head><title>" );
		view->write( info->GetTitle() );
		view->write( "</title></head><body>" );
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
					rowCount = 100;
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
					char level = 1;
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
	int len, maxlen = 0;

	while (curr)
	{
		if ( curr->type == MAN_DIR )
		{
			len = strlen( curr->text );

			if ( ((cManDir *)curr)->desc )
				len += strlen( ((cManDir *)curr)->desc ) + 3;

			if ( len > maxlen )
				maxlen = len;
		}
		curr = curr->next;
	}

	QFont font( fixedFont, HTMLFont::pointSize( fontBase - 1 ) );
	QFontMetrics fm( font );

	QString gridWidth;
	gridWidth.setNum( maxlen * fm.maxWidth() );

	curr = man->manList.GotoHead();

	if ( !bodyOnly )
	{
		view->setGranularity( 500 );
		view->begin();
		view->write( "<html><head><title>" );
		view->write( man->GetTitle() );
		view->write( "</title></head><body>" );
	}

	view->write( "<pre>" );

	while (curr)
	{
		if ( inDir && curr->type != MAN_DIR )
		{
#if KHTMLW_VERSION > 800
			view->write( "</grid><pre>" );
#else
			view->write( "</grid>" );
#endif
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
#if KHTMLW_VERSION > 800
					view->write( "</pre>" );
#endif
					view->write( "<grid width=" );
					view->write( gridWidth );
					view->write( " align=left>" );
					inDir = TRUE;
				}

#if KHTMLW_VERSION > 800
				view->write( "<cell width=" );
				view->write( gridWidth );
				view->write( " align=left><pre>" );
#else
				view->write( "<cell>" );
#endif
				view->write( "<a href=\"man:" );
				view->write( ((cManDir *)curr)->page );
				view->write( "\">" );
				convertSpecial( curr->text, converted );
				view->write( converted );
				view->write( "</a>" );

				if ( ((cManDir *)curr)->desc )
				{
					view->write( "  " );
					convertSpecial( ((cManDir *)curr)->desc, converted );
					view->write( converted );
				}
#if KHTMLW_VERSION > 800
				view->write( "</pre></cell>" );
#else
				view->write( "</cell>" );
#endif
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
		buffer[0] = '\0';
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
	mktemp( localFile.data() + 5 );
	connect( remotePage, SIGNAL( finished() ), this, SLOT( slotRemoteDone() ) );
	remotePage->copy( remoteFile.data(), localFile.data() );

	return 0;
}


int KHelpWindow::runCGI( const char *_url )
{
	if ( CGIServer )
	{
		delete CGIServer;
		KURL u( localFile );
		unlink( u.path() );
	}

	CGIServer = new KCGI;

	connect( CGIServer, SIGNAL( finished() ), this, SLOT( slotCGIDone() ) );

	localFile.sprintf( "file:/tmp/kdehelpXXXXXX" );
	mktemp( localFile.data() + 5 );

	CGIServer->get( _url, localFile, "Get" );

	return 0;
}


// attempt to detect the file type
KHelpWindow::FileType KHelpWindow::detectFileType( const QString &fileName )
{
	FileType type = UnknownFile;

	// attempt to identify file type
	// This is all pretty dodgey at the moment...
	if ( fileName.find( ".htm" ) > 0 && !access( fileName, 0 ) )
	{
		return HTMLFile;
	}
	else if ( fileName.find (".info" ) > 0 && !access( fileName, 0 ) )
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


bool KHelpWindow::canCurrentlyDo(AllowedActions action)
{
    switch (action)
	{
	case Copy:        return view->isTextSelected();
	case GoBack:      return history.IsBack();
	case GoForward:   return history.IsForward();
	case GoPrevious:  return format->PrevNode() != NULL;
	case GoNext:      return format->NextNode() != NULL;
	case GoUp:        return format->UpNode() != NULL;
	case GoTop:       return format->TopNode() != NULL;
	case Stop:        return busy;
	default: 
	      warning("KHelpWindow::canCurrentlyDo: missing case in \"switch\" statement\n");
	      return FALSE;
	}
	return FALSE; // just to make the compiler happy...
}


const char *KHelpWindow::getCurrentURL()
{
  return (const char *) currentURL;
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
	int top = 0;
	int bottom = height();

	if ( locationBar->isVisible() )
	{
		locationBar->setGeometry( 0, top, width(), 32 );
		top += 32;
	}

	bottom -= SCROLLBAR_WIDTH;
	horz->setGeometry( 0, bottom,
					width() - SCROLLBAR_WIDTH, SCROLLBAR_WIDTH );

	vert->setGeometry( width()-SCROLLBAR_WIDTH, top,
					SCROLLBAR_WIDTH, bottom-top );
	view->setGeometry( 0, top, width() - SCROLLBAR_WIDTH, bottom-top );
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


void KHelpWindow::slotPrint()
{
	view->print();
}


void KHelpWindow::slotCopy()
{
	QString text;

	view->getSelectedText( text );
	QClipboard *cb = kapp->clipboard();
	cb->setText( text );
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

void KHelpWindow::slotTextSelected( bool )
{
	emit enableMenuItems();

	XSetSelectionOwner( qt_xdisplay(), XA_PRIMARY, handle(), CurrentTime );
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
	{
		emit setURL( bm->getURL() );
	}
	else
	{
		emit setURL( currentURL );
	}
}


void KHelpWindow::slotBookmarkChanged()
{
	emit bookmarkChanged(bookmarkManager.getRoot());
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
	emit enableMenuItems();
}


void KHelpWindow::setLocationBar(bool enabled)
{
	if ( !enabled )
	{
		locationBar->hide();
	}
	else
	{
		locationBar->show();
	}
	layout();
}


void KHelpWindow::slotSetTitle( const char *_title )
{
	title = _title;
	emit setTitle(_title);
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
		emit enableMenuItems();
	}
}


void KHelpWindow::slotOnURL( const char *url )
{
	if ( url )
	{
		emit setURL( url );
	}
	else
	{
		emit setURL( currentURL );
	}
}


void KHelpWindow::slotFormSubmitted( const char *, const char *url )
{
	openURL( url );
}


void KHelpWindow::slotPopupMenu( const char *url, const QPoint &p )
{
	int id;

	if ( rmbPopup )
		delete rmbPopup;

	rmbPopup = new QPopupMenu;
	rmbPopup->installEventFilter( this );

	id = rmbPopup->insertItem( "Back", this, SLOT( slotBack() ) );
	rmbPopup->setItemEnabled( id, history.IsBack() );
	id = rmbPopup->insertItem( "Forward", this, SLOT( slotForward() ) );
	rmbPopup->setItemEnabled( id, history.IsForward() );
	
	if ( url )
	{
		rmbPopup->insertSeparator();
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
		rmbPopup->insertItem("Open this Link",this,SLOT(slotPopupOpenURL()));
		rmbPopup->insertItem("Add Bookmark",this,SLOT(slotPopupAddBookmark()));
		rmbPopup->insertItem("Open in new Window",this,SLOT(slotPopupOpenNew()));
	}

	rmbPopup->popup( p );
}


void KHelpWindow::slotDropEvent( KDNDDropZone *zone )
{
	KURL kurl( zone->getData() );
	if ( !kurl.isMalformed() )
		openURL( kurl.url() );
}

/*
void KHelpWindow::slotImageRequest( const char * _url )
{
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
	KURL u( localFile.data() );
	openFile( u.path() );
	emit setURL( currentURL );
	locationBar->setLocation( currentURL );

	delete remotePage;
	remotePage = NULL;

	unlink( u.path() );
}


void KHelpWindow::slotCGIDone()
{
	KURL u( localFile );
	if ( !openFile( u.path() ) )
		view->parse();
	emit setURL( currentURL );
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
	emit enableMenuItems();
}


void KHelpWindow::slotStandardFont( const char *n )
{
	standardFont = n;
	view->setStandardFont( n );
	view->parse();
	busy = true;
	emit enableMenuItems();
}


void KHelpWindow::slotFixedFont( const char *n )
{
	fixedFont = n;
	view->setFixedFont( n );
	view->parse();
	busy = true;
	emit enableMenuItems();
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
	emit openNewWindow( newURL );
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
		scrollTo = view->yOffset();
		view->parse();
		busy = true;
		emit enableMenuItems();
		viewWidth = s.width();
	}

	QApplication::restoreOverrideCursor();
}


void KHelpWindow::resizeEvent( QResizeEvent * )
{
	layout();
}


// all this just so that the rmb popup menu doesn't disappear
bool KHelpWindow::eventFilter( QObject *obj, QEvent *ev )
{
	static QPoint tmpPoint;

	if ( obj == rmbPopup )
	{
		if ( ev->type() == Event_MouseButtonRelease )
		{
			if ( QCursor::pos() == tmpPoint )
			{
				tmpPoint = QPoint( -10, -10 );
				return true;
			}
		}
	}

	if ( obj == view )
	{
		switch ( ev->type() )
		{
			case Event_MouseButtonPress:
			case Event_MouseButtonDblClick:
				tmpPoint = QCursor::pos();
				break;

			case Event_MouseButtonRelease:
				tmpPoint = QPoint(-10,-10);
				break;
		}

	}

	return false;
}

bool KHelpWindow::x11Event( XEvent *xevent )
{
	switch ( xevent->type )
	{
		case SelectionRequest:
			{
				if ( view->isTextSelected() )
				{
					QString text;
					view->getSelectedText( text );
					XSelectionRequestEvent *req = &xevent->xselectionrequest;
					XEvent evt;
					evt.xselection.type = SelectionNotify;
					evt.xselection.display  = req->display;
					evt.xselection.requestor = req->requestor;
					evt.xselection.selection = req->selection;
					evt.xselection.target = req->target;
					evt.xselection.property = None;
					evt.xselection.time = req->time;
					if ( req->target == XA_STRING )
					{
						XChangeProperty ( dpy, req->requestor, req->property,
							XA_STRING, 8, PropModeReplace,
							(uchar *)text.data(), text.length() );
						evt.xselection.property = req->property;
					}
					XSendEvent( dpy, req->requestor, False, 0, &evt );
				}

				return true;
			}
			break;

		case SelectionClear:
			// Do we want to clear the selection???
			view->selectText( 0, 0, 0, 0, 0 );
			break;
	}

	return false;
}

// called as html is parsed
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


// called when all html has been parsed
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
	emit enableMenuItems();
}

//-----------------------------------------------------------------------------

