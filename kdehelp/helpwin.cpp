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
#include <qbitmap.h>
#include <qregexp.h>
#include <kapp.h>
#include <kconfig.h>
#include <kurl.h>
#include <kfm.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmsgbox.h>
#include "cgi.h"
#include "kbutton.h"
#include "helpwin.h"
#include <kcursor.h>

#include "dbnew.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif

// for selection
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

static QString QUOTE( "\"" );
static QString DOCS_PATH;

//-----------------------------------------------------------------------------

KOpenURLDialog::KOpenURLDialog( QWidget *parent, const char *name )
	: QDialog( parent, name )
{
	setCaption( klocale->translate("KDE Help: Open URL") );

	QLabel *label = new QLabel( klocale->translate("Open URL"), this );
	label->setGeometry( 20, 20, 60, 20 );
	
	lineEdit = new QLineEdit( this );
	lineEdit->setGeometry( 30+label->width(), 20, 200, 20 );
	lineEdit->setFocus();

	QPushButton *openButton = new QPushButton( klocale->translate("Open"), this );
	openButton->setGeometry( 20, 60, 50, 30 );
	openButton->setDefault( TRUE );
	connect( openButton, SIGNAL(clicked()), SLOT(openPressed()) );

	QPushButton *cancelButton = new QPushButton( klocale->translate("Cancel"), this );
	cancelButton->setGeometry( 290-50, 60, 50, 30 );
	connect( cancelButton, SIGNAL(clicked()), SLOT(reject()) );
}

void KOpenURLDialog::openPressed()
{
	emit openURL( lineEdit->text(), LeftButton );
	accept();
}

//-----------------------------------------------------------------------------

KHistory *KHelpView::urlHistory = 0;

KHelpView::KHelpView( QWidget *parent, const char *name )
    : KHTMLWidget( parent, name )
{
    QString histFile = KApplication::localkdedir() + "/share/apps/kdehelp/history";

    if ( !urlHistory )
	urlHistory = new KHistory( histFile );
}

KHelpView::~KHelpView()
{
    urlHistory->saveHistory();
}

bool KHelpView::URLVisited( const char *_url )
{
    return urlHistory->inHistory( _url );
}

//-----------------------------------------------------------------------------

// statics
QString KHelpWindow::newURL;
KBookmarkManager KHelpWindow::bookmarkManager;
int  KHelpWindow::fontBase = 3;
QString KHelpWindow::standardFont;
QString KHelpWindow::fixedFont;
QColor KHelpWindow::bgColor;
QColor KHelpWindow::textColor;
QColor KHelpWindow::linkColor;
QColor KHelpWindow::vLinkColor;
bool KHelpWindow::underlineLinks;
bool KHelpWindow::forceDefaults;

KHelpWindow::KHelpWindow( QWidget *parent, const char *name )
	: QWidget( parent, name ), history(50), format(&html)
{
	openURLDialog = 0;
	CGIServer = 0;
	busy = false;
	scrollTo = 0;
	rmbPopup = 0;
	findDialog = 0;

	DOCS_PATH = kapp->kde_htmldir() + "/default/kdehelp/";

	readOptions();
	man = new cMan;
	info = new cInfo;
/*
	accel = new QAccel( this );
	accel->insertItem( CTRL + Key_C,	COPY );
	accel->insertItem( CTRL + Key_Insert,	COPY );
	accel->insertItem( CTRL + Key_N,	NEW );
	accel->insertItem( CTRL + Key_W,	CLOSE );
	accel->insertItem( CTRL + Key_Q,	QUIT );

	accel->connectItem( COPY, this, SLOT(slotCopy()) );
*/
	view = new KHelpView( this );
	CHECK_PTR( view );
	view->setDefaultFontBase( fontBase );
	view->setStandardFont( standardFont );
	view->setFixedFont( fixedFont );
	view->setURLCursor( KCursor::handCursor() );
	view->setUnderlineLinks( underlineLinks );
	view->setForceDefault( forceDefaults );
	view->setFocusPolicy( QWidget::StrongFocus );
	view->setFocus();
	view->installEventFilter( this );
	view->setUpdatesEnabled( true );
	view->setDefaultBGColor( bgColor );
	view->setDefaultTextColors( textColor, linkColor, vLinkColor );

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
	connect( view, SIGNAL( formSubmitted( const char *, const char *, const char * ) ),
			SLOT( slotFormSubmitted( const char *, const char *, const char * ) ) );
	connect( view, SIGNAL( resized( const QSize & ) ),
			SLOT( slotViewResized( const QSize & ) ) );
	connect( view, SIGNAL( textSelected( bool ) ), 
			SLOT( slotTextSelected( bool ) ) );

	connect( vert, SIGNAL(valueChanged(int)), view, SLOT(slotScrollVert(int)) );
	connect( horz, SIGNAL(valueChanged(int)), view, SLOT(slotScrollHorz(int)) );

	connect( view, SIGNAL( documentChanged() ),
			SLOT( slotDocumentChanged() ) );

	connect( view, SIGNAL( documentDone() ),
			SLOT( slotDocumentDone() ) );

	// load bookmarks
	const QString p = KApplication::localkdedir();
	const QString bmFile = p + "/share/apps/kdehelp/bookmarks.html";
	bookmarkManager.read( bmFile );

	layout();
}


KHelpWindow::~KHelpWindow()
{
	if (openURLDialog)
		delete openURLDialog;

	if ( findDialog )
        delete findDialog;

	delete man;
	delete info;
//	delete dropZone;		// causes SEG FAULT. Why?
}


void KHelpWindow::readOptions()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );

	QString qs = config->readEntry( "BaseFontSize" );
	if ( !qs.isEmpty() )
		fontBase = qs.toInt();

	qs = "times";
	standardFont = config->readEntry( "StandardFont", qs );
	qs = "courier";
	fixedFont = config->readEntry( "FixedFont", qs );

	bgColor = config->readColorEntry( "BgColor", &white );
	textColor = config->readColorEntry( "TextColor", &black );
	linkColor = config->readColorEntry( "LinkColor", &blue );
	vLinkColor = config->readColorEntry( "VLinkColor", &darkMagenta );
	underlineLinks = config->readBoolEntry( "UnderlineLinks", TRUE );
	forceDefaults = config->readBoolEntry( "ForceDefaultColors", TRUE );
}


// Open a URL.  Initiate remote transfer if necessary
int KHelpWindow::openURL( const char *URL, bool withHistory )
{
	char location[256];
	int pos, rv = 1, viewPos = view->yOffset();
	bool isRemote = false;

	scrollTo = 0;
	ref = "";

	if ( busy )
		view->setCursor( oldCursor );

	if ( CGIServer )
	{
		delete CGIServer;
		CGIServer = 0;
	}

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
	int prevPos = currentURL.findRev( '#' );
	prevPos = prevPos >= 0 ? prevPos : currentURL.length();

	int refPos = fullURL.findRev( '#' );
	if ( refPos < 0 )
	{
	    ref.resize( 0 );
	    refPos = fullURL.length();
	}
	else
	    ref = fullURL.right( fullURL.length() - refPos - 1 );
	fullURL.truncate( refPos );

	if ( fullURL == currentURL.left( prevPos ) )
	{
		if ( !ref.isEmpty() )
		{
		    if ( !view->gotoAnchor( ref ) )
			    return 1;
		    currentURL.truncate( prevPos );
		    currentURL += "#";
		    currentURL += ref;
		}
		else
		    currentURL.truncate( prevPos );

		if ( withHistory )
		{
			if ( history.Current() )
				history.Current()->setOffset( viewPos );
			history.Add( new KPageInfo( currentURL, 0 ) );
			view->urlHistory->addURL( currentURL );
		}
		emit enableMenuItems();
		emit setURL( currentURL );
		emit setLocation( currentURL );
		return 0;
	}

	// if this is a relative location, then use path of current URL
	const char * colon = strchr( fullURL, ':' );
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

	oldCursor = view->cursor();
	view->setCursor( waitCursor );

	if ( fullURL.contains( "file:" ) )
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
	else if ( fullURL.contains( "info:" ) )
	{
		if ( location[0] != '(' )
			fullURL = "info:(" + currentInfo + ')' + location;
		else
		{
			currentInfo = location+1;
			const int i = currentInfo.find( ')' );
			if (i > 0)
				currentInfo.truncate(i);
		}
		if ( ( rv = info->ReadLocation( location ) ) == 0 )
			formatInfo();
	}
	else if ( fullURL.contains( "man:" ) )
	{
		if ( ( rv = man->ReadLocation( location ) ) == 0 )
			formatMan();
	}
	else if ( fullURL.contains( "http:" ) )
	{
		KFM *kfm = new KFM;
		kfm->openURL( fullURL );
		delete kfm;
	}
	else if ( fullURL.contains( "ftp:" ) )
	{
		KFM *kfm = new KFM;
		kfm->openURL( fullURL );
		delete kfm;
	}
	else if ( fullURL.contains( "mailto:" ) )
	{
		KFM *kfm = new KFM;
		kfm->openURL( fullURL );
		delete kfm;
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
				history.Current()->setOffset( viewPos );
			history.Add( new KPageInfo( currentURL, 0 ) );
			view->urlHistory->addURL( currentURL );
		}
		emit enableMenuItems();

		if ( !isRemote )
		{
			view->parse();
			horz->setValue( 0 );
			emit setURL( currentURL );
			emit setLocation( currentURL ); 
			view->setCursor( oldCursor );
		}
	}
	else
	    view->setCursor( oldCursor );

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
			if( tryHtmlDefault( fileName ) )
			{
				fullURL = "file:" + fileName;
				rv = openHTML( fileName );
			}
			else
			{
				QMessageBox mb;
				mb.setText( klocale->translate("Cannot open: ") + fileName );
				mb.setButtonText( klocale->translate("Oops!") );
				mb.show();
			}
			break;

		default:
			{
				QMessageBox mb;
				mb.setText( klocale->translate("Unknown format: ") + fileName );
				mb.setButtonText( klocale->translate("Oops!") );
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
		view->setGranularity( 400 );
		view->begin();
		view->write( "<html><head><title>" );
		view->write( info->GetTitle() );
		view->write( "</title></head><body>" );
	}

	view->write( "<pre>\n" );

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
					char level = '1';
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
					str.sprintf("\n</pre><br><h%c>", level);
					view->write( str );
					convertSpecial( ((cNodeText *)curr)->text(), converted );
					view->write( converted );
					view->write( "<br>" );
					str.sprintf("</h%c>", level);
					view->write( str );
					view->write( "<pre>\n" );
				}
				break;
		}
		curr = curr->next;
	}

	view->write( "\n</pre>" );

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

int KHelpWindow::formatMan( int bodyOnly )
{
	if ( !bodyOnly )
	{
		view->setGranularity( 600 );
		view->begin(fullURL);
		view->write( "<html><head><title>" );
		view->write( man->GetTitle() );
		view->write( "</title></head><body>" );
	}

	view->write( man->page() );

	if ( !bodyOnly )
	{
		view->write("</body></html>");
		view->end();
	}

	format = man;

	return 0;
}

#if 0
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

        int fonts[7];
        view->getFontSizes(fonts);
	QFont font( fixedFont, fonts[fontBase - 1] );
	QFontMetrics fm( font );

	QString gridWidth;
	gridWidth.setNum( maxlen * fm.maxWidth() );

	curr = man->manList.GotoHead();

	if ( !bodyOnly )
	{
		view->setGranularity( 600 );
		view->begin();
		view->write( "<html><head><title>" );
		view->write( man->GetTitle() );
		view->write( "</title></head><body>" );
	}

	view->write( "<pre>\n" );

	while (curr)
	{
		if ( inDir && curr->type != MAN_DIR )
		{
			view->write( "</grid><pre>\n" );
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
				view->write( "\n</pre><h1>" );
				convertSpecial( curr->text, converted );
				view->write( converted );
				view->write( "</h1><pre>\n" );
				break;

			case MAN_HEADING2:
				view->write( "\n</pre><h2>" );
				convertSpecial( curr->text, converted );
				view->write( converted );
				view->write( "</h2><pre>\n" );
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
					view->write( "\n</pre>" );
					view->write( "<grid width=" );
					view->write( gridWidth );
					view->write( " align=left>" );
					inDir = TRUE;
				}

				view->write( "<cell width=" );
				view->write( gridWidth );
				view->write( " align=left><pre>\n" );
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
				view->write( "\n</pre></cell>" );
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

	view->write( "\n</pre>" );

	if ( !bodyOnly )
	{
		view->write("</body></html>");
		view->end();
	}

	format = man;

	return 0;
}
#endif

// open a HTML file
int KHelpWindow::openHTML( const char *location )
{
	if ( html.ReadLocation( location ) )
		return 1;

	int val;
	QFile file( location) ;

	if ( !file.open(IO_ReadOnly) )
		return 1;

	view->setGranularity( 600 );
	view->begin( fullURL );

	do
	{
		char buffer[256];
		buffer[0] = '\0';
		val = file.readLine( buffer, sizeof( buffer ) );
		if ( strncmp( buffer, "Content-type", 12 ) )
		    view->write(buffer);
	}
	while ( val >= 0 && !file.atEnd() );
	// must check for both; QFile's "index" gets out of sync
	// if the file contains nulls (which it should not, of course).

	view->end();

	format = &html;

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

	char filename[256];
	sprintf( filename, "%s/kdehelpXXXXXX", _PATH_TMP );
	mktemp( filename );
	localFile.sprintf( "file:%s", filename );

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
			sprintf( fname, "%s/khelpXXXXXX", _PATH_TMP );
			mktemp( fname );
			sprintf(sysCmd, "gzip -cd %s > %s", (const char *)fileName, fname);
            if ( safeCommand( fileName ) )
                system( sysCmd );
		}
		QFile file( fname );
		if ( file.open(IO_ReadOnly) )
		{
			// check the first 80 lines for some keywords:
			for ( int i = 0; !file.atEnd(), i < 80; i++ )
			{
				char buf[256];
				file.readLine( buf, sizeof( buf ) );
				QString buffer = buf;
				if ( buffer.contains( "<HTML>", FALSE ) ||
				     buffer.contains( "<BODY",  FALSE ) )
				{
					type = HTMLFile;
					break;
				}
				else if ( buffer.contains( ".TH" ) ||
				          buffer.contains( ".so" ) )
				{
					type = ManFile;
					break;
				}
				else if ( buffer.contains( "Indirect:" ) ||
				          buffer.contains( "Node:" ) )
				{
					type = InfoFile;
					break;
				}
			}
		}
		else
			type = CannotOpenFile;
		// remove the temporary file.
		if ( strstr( fileName, ".gz" ) )
			remove( fname );
	}

	return type;
}

/**
 * if we could not find a file in a locale-specific help subtree
 * try the default tree.
 * @param fileName is the name of the link target.
 *        On output it is modified according to the redirection
 *        by changing the language part to "default".
 * @return whether a new target was found
 */
bool
KHelpWindow::tryHtmlDefault( QString &fileName )
const
{
	bool found = false;

	if ( fileName.find( ".htm" ) > 0  )
	{
		const char * const htmldir = kapp->kde_htmldir();
		if( fileName.find( htmldir ) == 0 )
		{
			const int htmldirlen = strlen( htmldir );
			const int slashpos = fileName.find( '/', htmldirlen+1 );
			if( slashpos > 0 )
			{
				fileName.replace( htmldirlen, slashpos - htmldirlen,
				                  "/default" );
				found = ( access( fileName, 0 ) == 0 );
			}
		}
	}

	return found;
}

// turn special characters into their HTML equivalents
//
void KHelpWindow::convertSpecial( const char *buffer, QString &converted )
{
	const QString special = "<>&\"";
	const char * const replace[] = { "&lt;", "&gt;", "&amp;", "&quot;", 0 };

	converted = "";

	if ( !buffer )
		return;

	for ( const char *ptr = buffer; *ptr; ptr++ )
	{
		int pos;
		if ( (pos = special.find( *ptr )) >= 0 )
			converted += replace[pos];
		else
			converted += *ptr;
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
	int pos1 = currentURL.find( ':' ) + 1;
	int pos2 = currentURL.find( '#' );

	if ( pos2 < 0 )
		pos2 = currentURL.length();
	
	const QString loc = currentURL.mid( pos1, pos2-pos1 );

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
	QString bmFile = KApplication::localkdedir() + "/share/apps/kdehelp/bookmarks.html";
	bookmarkManager.add( _title, url );
	bookmarkManager.write( bmFile );
}


void KHelpWindow::layout()
{
	int top = 0;
	int bottom = height();

	bottom -= SCROLLBAR_WIDTH;
	horz->setGeometry( 0, bottom,
					width() - SCROLLBAR_WIDTH, SCROLLBAR_WIDTH );

	vert->setGeometry( width()-SCROLLBAR_WIDTH, top,
					SCROLLBAR_WIDTH, bottom-top );
	view->setGeometry( 0, top, width() - SCROLLBAR_WIDTH, bottom-top );
}


void KHelpWindow::slotOpenFile()
{
	QString fileName = KFileDialog::getOpenFileName();

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

void KHelpWindow::slotReload()
{
    currentURL = "";
    openURL( QUOTE + getPrefix() + format->GetLocation() + QUOTE, false );
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

void KHelpWindow::slotFind()
{
    // if we haven't already created a find dialog then do it now
    if ( !findDialog )
    {
	findDialog = new KFindTextDialog();
	connect( findDialog, SIGNAL( find( const QRegExp & ) ),
			     SLOT( slotFindNext( const QRegExp & ) ) );
    }

    // reset the find iterator
    view->findTextBegin();

    findDialog->show();
}

void KHelpWindow::slotFindNext()
{
    if ( findDialog && !findDialog->regExp().isEmpty() )
    {
	// We have a find dialog, so use the reg exp it maintains.
	slotFindNext( findDialog->regExp() );
    }
    else
    {
	// no find has been attempted yet - open the find dialog.
	slotFind();
    }
}

void KHelpWindow::slotFindNext( const QRegExp &regExp )
{
    if ( !view->findTextNext( regExp ) )
    {
	// We've reached the end of the document.
	// Can either stop searching and close the find dialog,
	// or start again from the top.
	if ( KMsgBox::yesNo( this, i18n( "Find Complete" ),
	    i18n( "Continue search from the top of the page?" ),
	    KMsgBox::DB_SECOND | KMsgBox::QUESTION ) == 1 )
	{
	    view->findTextBegin();
	    slotFindNext( regExp );
	}
	else
	{
	    view->findTextEnd();
	    findDialog->hide();
	}
    }
}

void KHelpWindow::slotBack()
{
	if ( history.Current() )
		history.Current()->setOffset( view->yOffset() );

	const KPageInfo * const p = history.Back();

	if ( p )
	{
		if ( !openURL( p->getUrl(), false ) )
		{
		    scrollTo = p->getOffset();
		    if ( !busy )
		    {
			view->slotScrollVert( scrollTo );
			vert->setValue( scrollTo );
		    }
		}
	}
}


void KHelpWindow::slotForward()
{
	if ( history.Current() )
		history.Current()->setOffset( view->yOffset() );

	const KPageInfo * const p = history.Forward();

	if ( p )
	{
		if ( !openURL( p->getUrl(), false ) )
		{
		    scrollTo = p->getOffset();
		    if ( !busy )
		    {
			view->slotScrollVert( scrollTo );
			vert->setValue( scrollTo );
		    }
		}
	}
}


void KHelpWindow::slotDir()
{
    QString initDoc = "file:";
    initDoc += kapp->findFile( "/share/doc/HTML/default/kdehelp/main.html" );

    openURL( initDoc );
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

	if ( CGIServer )
	{
		delete CGIServer;
		KURL u( localFile );
		unlink( u.path() );
		CGIServer = NULL;
		view->setCursor( oldCursor );
	}

	view->stopParser();

	busy = false;
	emit enableMenuItems();
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
        emit openNewWindow( URL );
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


void KHelpWindow::slotFormSubmitted( const char *_method, const char *_url, const char* _data )
{
    if (strcasecmp(_method, "GET")==0)
    {
      QString url( _url );
      url += "?";
      url += _data; 
      openURL( url );
    }
    else
      openURL( _url );
}


void KHelpWindow::slotPopupMenu( const char *url, const QPoint &p )
{
	int id;

	if ( rmbPopup )
		delete rmbPopup;

	rmbPopup = new QPopupMenu;
	rmbPopup->installEventFilter( this );

	id = rmbPopup->insertItem( klocale->translate("Back"), this, SLOT( slotBack() ) );
	rmbPopup->setItemEnabled( id, history.IsBack() );
	id = rmbPopup->insertItem( klocale->translate("Forward"), this, SLOT( slotForward() ) );
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
		rmbPopup->insertItem(klocale->translate("Open this Link"),this,SLOT(slotPopupOpenURL()));
		rmbPopup->insertItem(klocale->translate("Add Bookmark"),this,SLOT(slotPopupAddBookmark()));
		rmbPopup->insertItem(klocale->translate("Open in new Window"),this,SLOT(slotPopupOpenNew()));
	}

	rmbPopup->popup( p );
}


void KHelpWindow::slotDropEvent( KDNDDropZone *zone )
{
	KURL kurl( zone->getData() );
	if ( !kurl.isMalformed() )
		openURL( kurl.url() );
}

void KHelpWindow::slotCGIDone()
{
	view->setCursor( oldCursor );
	KURL u( localFile );
	if ( !openFile( u.path() ) )
		view->parse();
	emit setURL( currentURL );
	emit setLocation( currentURL ); 

	delete CGIServer;
	CGIServer = 0;

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


void KHelpWindow::slotColorsChanged( const QColor &bg, const QColor &text,
	const QColor &link, const QColor &vlink, const bool uline, const bool force)
{
	view->setForceDefault( force );
	view->setDefaultBGColor( bg );
	view->setDefaultTextColors( text, link, vlink );
	view->setUnderlineLinks(uline);
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


void KHelpWindow::slotViewResized( const QSize & )
{
	QApplication::setOverrideCursor( waitCursor );

	vert->setSteps( 12, view->height() - 20 ); 
	horz->setSteps( 24, view->width() );

	if ( view->docHeight() > view->height() )
		vert->setRange( 0, view->docHeight() - view->height() );
	else
		vert->setRange( 0, 0 );

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
			XChangeProperty ( qt_xdisplay(), req->requestor,
				req->property, XA_STRING, 8, PropModeReplace,
				(uchar *)text.data(), text.length() );
			evt.xselection.property = req->property;
		    }
		    XSendEvent( qt_xdisplay(), req->requestor, False, 0, &evt );
		}

		return true;
	    }
	    break;

	case SelectionClear:
	    // Do we want to clear the selection???
	    view->selectText( 0, 0, 0, 0 );
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
	else if ( !ref.isEmpty() )
	{
		if ( !view->gotoAnchor( ref ) )
			vert->setValue( 0 );
	}

	busy = false;
	emit enableMenuItems();
}

//-----------------------------------------------------------------------------

