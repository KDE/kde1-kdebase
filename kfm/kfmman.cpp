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
#include <qkeycode.h>
#include <qdir.h>
#include <qstrlist.h>
#include <qregexp.h>
#include <qfontmet.h>
#include <qmsgbox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstring.h>

#include "kfmman.h"
#include "xview.h"
#include "kbind.h"
#include <config-kfm.h>
#include "kmsgwin.h"
#include "kfmdlg.h"
#include "kfmexec.h"
#include "utils.h"

KFMManager::KFMManager( KfmView *_v )
{
    url  = "";
    view = _v;
    maxLabelWidth = 80;
    labelFontMetrics = new QFontMetrics(view->defaultFont());
    popupMenu = new QPopupMenu();
    popupMenu->installEventFilter( this );

    // Connect to the popup menu
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );

    files.setAutoDelete( true );
    job = new KFMJob;
    connect( job, SIGNAL( error( int, const char* ) ), this, SLOT( slotError( int, const char* ) ) );
    connect( job, SIGNAL( newDirEntry( KIODirectoryEntry* ) ),
	     this, SLOT( slotNewDirEntry( KIODirectoryEntry* ) ) );
    connect( job, SIGNAL( finished() ), this, SLOT( slotFinished() ) );
    connect( job, SIGNAL( data( const char *, int ) ), this, SLOT( slotData( const char *, int ) ) );
    connect( job, SIGNAL( mimeType( const char* ) ), this, SLOT( slotMimeType( const char* ) ) );
    connect( job, SIGNAL( info( const char* ) ), this, SLOT( slotInfo( const char* ) ) );
    connect( job, SIGNAL( redirection( const char* ) ), this, SLOT( slotRedirection( const char* ) ) );
}

KFMManager::~KFMManager()
{
    if (labelFontMetrics) delete labelFontMetrics;
}

bool KFMManager::isBindingHardcoded( const char *_txt )
{
    if ( strcmp( klocale->getAlias(ID_STRING_CD), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_NEW_VIEW), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_COPY), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_DELETE), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_MOVE_TO_TRASH), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_PASTE), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_OPEN_WITH), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_CUT), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_MOVE), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_PROP), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_LINK), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_OPEN), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_TRASH), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_ADD_TO_BOOMARKS), _txt ) == 0 )
	return true;

    return false;
}
  
void KFMManager::writeWrapped( char *_str )
{
    short        j, width, charWidth;
    char*        pos;
    // char*        first;
    char*        sepPos;
    char* part;
    char  c;
    
    if (labelFontMetrics->width(_str) <= maxLabelWidth)
    {
	view->writeHTMLquoted ( _str );
	return;
    }

    for (width=0, part=_str, pos=_str; *pos; pos++)
    {
	charWidth = labelFontMetrics->width( *pos );
	if (width+charWidth >= maxLabelWidth)
	{
	    // search for a suitable separator in the previous 8 characters
	    for (sepPos=pos, j=0; j<8 && sepPos>part; j++, sepPos--)
	    {
		/* if (ispunct(*sepPos) || isdigit(*sepPos)) break;
		   if (isupper(*sepPos) && !isupper(*(sepPos-1))) break;
	    }
	    if (j<8 && j>0)
	    {
q		pos = sepPos;
		//width = width - XTextWidth (fs, pos, j); */
		if (ispunct(*sepPos) || isdigit(*sepPos))
                {
                    pos = sepPos;
                    break;
                }
                if (isupper(*sepPos) && !isupper(*(sepPos-1)))
                {
                    pos = sepPos;
                    break;
               }               
	    }
	    
	    c = *pos;
	    *pos = '\0';
	    view->writeHTMLquoted ( part );
	    view->write( "<br>" );
	    *pos = c;
	    part = pos;
	    width = 0;
	}
	width += charWidth;
    }
    if (*part) view->writeHTMLquoted ( part );
}            

bool KFMManager::eventFilter( QObject *ob, QEvent *ev )
{
    // let the popup stay on the screen if the user
    // didn't move the mouse (Matthias)
    if (ob == popupMenu && ev->type() == Event_MouseButtonRelease
	&& QCursor::pos() == popupMenuPosition )
    {
	popupMenuPosition = QPoint(-10,-10);
	return true;
    }
    return false;
}
    
bool KFMManager::openURL( const char *_url, bool _reload )
{
    // By Default we display everything at the moment we
    // get it => now buffering of HTML code
    bBufferPage = FALSE;
    
    // Used to store the modified URL
    QString tmpurl;
    // Is the URL a local file ?
    if ( *_url == '/' )
    {
	// Prepend the "file" protocol
	tmpurl = "file:";
	tmpurl += _url;
	_url = tmpurl.data();
    }
    
    // Store the parameter for recursive function calls
    bReload = _reload;
    
    // debugT("Changing to URL %s\n", _url );
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
	warning(klocale->translate("ERROR: Malformed URL"));
	return false;
    }

    if ( strcmp( u.protocol(), "mailto" ) == 0 )
    {
	QString subject;
	QString to( u.path() );
	int i;
	if ( ( i = to.find( '?' ) ) != -1 )
	    to = to.left( i ).data();
	QString cmd;
	cmd << "kmail -s \"" << subject << "\" \"" << to << "\"";
	KMimeBind::runCmd( cmd );
	return false;
    }
    
    // Is page cached ?
    const char *file;
    if ( ( file = view->getHTMLCache()->isCached( _url ) ) != 0L && !_reload )
    {
	FILE* f = fopen( file, "rb" );
	if ( f )
	{
	    QString page = "";
	    char buffer[ 1025 ];
	    while ( !feof( f ) )
	    {
		int n = fread( buffer, 1, 1024, f );
		if ( n > 0 )
		{
		    buffer[n] = 0;
		    page += buffer;
		}
	    }
	    fclose( f );

	    // Add old URL to history
	    view->slotURLToStack( url.data() );

	    url = _url;
	    view->begin( _url );	    
	    view->write( page );
	    view->parse();
	    view->end();

	    return true;
	}
	else
	    warning("ERROR: Could not read file in cache\n");
    }
    
    // A link to the web in form of a *.kdelnk file ?
    QString path = u.path();
    if ( !u.hasSubProtocol() && strcmp( u.protocol(), "file" ) == 0 && path.right(7) == ".kdelnk" )
    {
	// Try tp open the *.kdelnk file
	QFile file( path );
	if ( file.open( IO_ReadOnly ) )
	{
	    file.close();

	    KConfig config( path );
	    config.setGroup( "KDE Desktop Entry" );
	    QString typ = config.readEntry( "Type" );
	    // Is it a link ?
	    if ( typ == "Link" )
	    {
		// Is there a URL ?
		QString u2 = config.readEntry( "URL" );
		if ( !u2.isEmpty() )
		    // It is a link and we have new URL => Recursion with the new URL
		    return openURL( u2, _reload );
		else
		{
		    // The *.kdelnk file is broken
		    QMessageBox::warning( 0, klocale->translate("KFM Error"), 
					  klocale->translate("The file does not contain a URL") );
		    return FALSE;
		}
	    }
	    file.close();
	}
    }

    // A HACK
    HTMLBuffer = "";

    bHTML = FALSE;
    bFinished = FALSE;
    // Prevent us from modifying the history stack if the stack
    // is locked during this functions run. Mention that the
    // stack may be unlocked when we want to modify it. So
    // we have to remember the current state here.
    bHistoryStackLock = view->isHistoryStackLocked();
    
    // Do we know that it is !really! a file ?
    // Then we can determine the mime type for shure and run
    // the best matching binding
    if ( KIOServer::isDir( _url ) == 0 && strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
    {    
	tryURL = KFMExec::openLocalURL( _url );
	if ( tryURL.isEmpty() )
	    return false;
    }
    else
	// We try to load this URL now
	tryURL = _url;

    view->getGUI()->slotAddWaitingWidget( view );
    
    // Stop a running job. Calling this if no job is running does no harm.
    job->stop();
    // We pass our current KIODirectoryEntries. Sometimes this information
    // can speed up the process of finding out wether tryURL is
    // a directory or some file. Mention that files is cleared
    // at once by a call to this function. 'browse' may emit the signal
    // 'newDirEntry' which uses 'files'. So 'files' has to be cleared
    // before! such signal is emitted.
    job->browse( tryURL, _reload, view->getGUI()->isViewHTML(), url, &files );
    
    // Something cached ? In this case a call to browse was all we need
    if ( bFinished )
	return true;

    QString tmp;
    if ( u.host() != 0L && u.host()[0] != 0 )
    {
	tmp << klocale->translate("Contacting host ") << u.host();
	view->getGUI()->slotSetStatusBar( tmp );
    }
    else
	view->getGUI()->slotSetStatusBar( klocale->translate( "Working ..." ) );
    return true;
}

void KFMManager::slotError( int, const char * )
{
    view->getGUI()->slotRemoveWaitingWidget( view );
}

void KFMManager::slotNewDirEntry( KIODirectoryEntry * _entry )
{
    if ( _entry != 0L )
    {
	KIODirectoryEntry *e = new KIODirectoryEntry( *_entry );
	files.append( e );
	
	// first entry ?
	if ( files.count() == 1 )
	    writeBeginning();
	
	writeEntry( e );
    }
}

void KFMManager::writeBodyTag()
{
    // Open ".directory"
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup( "KFM HTML Defaults" );
    QString text_color = config->readEntry( "TextColor" );
    QString link_color = config->readEntry( "LinkColor" );
    QString bg_color = config->readEntry( "BgColor" );
    QString tmp2 = config->readEntry( "BgImage" );
    QString bg_image;
    if ( !tmp2.isNull() )
	if ( tmp2.data()[0] != 0 )
	{
	    bg_image = "file:";
	    bg_image += kapp->kdedir().copy();
	    bg_image += "/share/wallpapers/";
	    bg_image += tmp2.data();
	}

    KURL u( url );
    // if we are on the local hard disk, we can look at .directory files    
    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
    {
	QString d = u.path();
	if ( d.right( 1 ) != "/" )
	    d += "/.directory";
	else
	    d += ".directory";
    
	// debugT("Trying .directory\n");
    
	QFile f( d.data() );
	if ( f.open( IO_ReadOnly ) )
	{
	    // debugT("Opened .directory\n");
	    
	  f.close(); // kalle
	  // kalle	    QTextStream pstream( &f );
	    KConfig config( d );
	    config.setGroup( "KDE Desktop Entry" );
	    
	    QString tmp = config.readEntry( "TextColor" );
	    if ( !tmp.isNull() )
		if ( tmp.data()[0] != 0 )
		    text_color = tmp.data();
	    tmp = config.readEntry( "LinkColor" );
	    if ( !tmp.isNull() )
		if ( tmp.data()[0] != 0 )
		    link_color = tmp.data();
	    tmp = config.readEntry( "BgColor" );
	    if ( !tmp.isNull() )
		if ( tmp.data()[0] != 0 )
		    bg_color = tmp.data();
	    tmp = config.readEntry( "BgImage" );
	    if ( !tmp.isNull() )
		if ( tmp.data()[0] != 0 )
		{
		    bg_image = kapp->kdedir().copy();
		    bg_image += "/share/wallpapers/";
		    bg_image += tmp.data();
		}
	}
    }

    view->write( "<body" );

    if ( !text_color.isNull() )
    {
	view->write(" text=" );
	view->write( text_color.data() );
    }

    if ( !link_color.isNull() )
    {
	view->write(" link=" );
	view->write( link_color.data() );
    }

    if ( !bg_image.isNull() )
    {
	KURL u2( u, bg_image.data() );
	view->write(" background=\"" );
	QString t = u2.url();
	view->write( t.data() );
	view->write( "\"" );
    }
    else if ( !bg_color.isNull() )
    {
	view->write(" bgcolor=" );
	view->write( bg_color.data() );
    }

    view->write( ">" );
}

void KFMManager::writeBeginning()
{
    // Push the old URL on the stack if we are allowed to
    if ( !url.isEmpty() && !bHistoryStackLock )
	view->slotURLToStack( url.data() );
    // The 'job->browse' command was successful. So lets
    // get the correct URL. This URL may vary from the URL
    // passed to 'openURL' in a trailing "/" for example.
    // Or we got a HTTP redirection or stuff like that.
    url = job->getURL();
    
    view->begin( url.data() );
    view->parse();
    view->write( "<html><head><title>" );
    view->write( url.data() );
    view->write( "</title></head>" );
    
    writeBodyTag();
    
    KURL u( url.data() );
    
    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
    { }
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	view->write( "<table width=100%>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	view->write( "<table width=100%>" );
    
    if ( strcmp( u.path(), "/" ) != 0 )
    {
	KURL u2( url.data() );
	u2.cd("..");
	
	QString s = u2.url();
	if ( s.right( 1 ) != "/" )
	    s += "/";
	
	if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
	{
	    view->write( "<a href=\"");
	    view->write( s.data() );
	    view->write( "\"><cell><center><img border=0 src=\"file:" );
	    view->write( KMimeType::getPixmapFileStatic( s.data() ) );
	    view->write( "\"><br>..</center><br></cell></a>" );
	}
	else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	{
	    view->write( "<tr><td><a href=\"" );
	    view->write( s.data() );
	    view->write( "\"><img border=0 width=16 height=16 src=\"file:" );
	    view->write( KMimeType::getPixmapFileStatic( s.data(), TRUE ) );
	    view->write( "\"></td><td>..</a></td>" );
	    view->write( "<td></td><td></td><td></td><td></td><td></td></tr>" );
	}
	else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	{
	    view->write( "<tr><td><a href=\"" );
	    view->write( s.data() );
	    view->write( "\">../</td><td></a></td>" );
	    view->write( "<td></td><td></td><td></td><td></td></tr>" );
	}
    }
}

void KFMManager::writeEntry( KIODirectoryEntry *s )
{
    char buffer[ 1024 ];

    if ( strcmp( s->getName(), "." ) == 0 || strcmp( s->getName(), ".." ) == 0 ||
	 strcmp( s->getName(), "./" ) == 0 || strcmp( s->getName(), "../" ) == 0 )
	return;

    if ( s->getName()[0] == '.' && !view->getGUI()->isShowDot() )
	return;
    
    QString decoded = s->getName();
    decoded.detach();
    KURL::decodeURL( decoded );   // decoded to pass to writeWrapped()

    QString filename( url );        // filename, Filename, useable to find file on Disk (Hen)
    filename.detach();
    filename += s->getName();

    QString encodedURL ( url );  
    encodedURL.detach();            // encodedURL, URL,  encoded for <a href ..> (Hen)
    encodedURL += s->getName();

    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
    { 
	// Delete ".kdelnk" extension ( only in Icon View )
	if ( decoded.right(7) == ".kdelnk" )
	    decoded.truncate( decoded.length() - 7 );

	view->write( "<a href=\"" );
	
	view->write( encodedURL.data() );
	if ( view->getGUI()->isVisualSchnauzer() )
	{
	    view->write( "\"><cell>" );
	    view->write( getVisualSchnauzerIconTag( filename ).data() );
	    view->write( "<br>" );
	}
	else
	{
	    view->write( "\"><cell><center><img border=0 src=\"file:" );
	    view->write( KMimeType::getPixmapFileStatic( filename ) );
	    view->write( "\"><br>" );
	}
	
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("<i>");
	strcpy( buffer, decoded );
	writeWrapped( buffer );  // writeWrapped htmlQuotes itself
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("</i>");
	view->write( "</center><br></cell></a>" );
    }
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
    {
	view->write( "<tr><td><a href=\"" );

	view->write( encodedURL.data() );
	view->write( "\"><img border=0 width=16 height=16 src=\"file:" );
	view->write( KMimeType::getPixmapFileStatic( filename.data(), TRUE ) );
	view->write( "\"></td><td>" );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("<i>");
	view->writeHTMLquoted ( decoded );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("</i>");
	view->write( "</td><td><tt>" ); 
	view->write( s->getAccess() );
	view->write( "</tt></td><td>" );
	view->write( s->getOwner() );
	view->write( "</td><td>" );
	view->write( s->getGroup() );
	view->write( "</td><td align=right>" ); 
	QString tmp;
	tmp.sprintf( "%i</td><td>", s->getSize() );
	view->write( tmp.data() );
	view->write( s->getCreationDate() );
	
	view->write( "</a></td></tr>" );
    }
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
    {
	view->write( "<tr><td><a href=\"" );
	
	view->write( encodedURL.data() );
	view->write( "\">" );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("<i>");
	view->writeHTMLquoted ( decoded );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("</i>");
	view->write( "</td><td><tt>" );
	view->write( s->getAccess() );
	view->write( "</tt></td><td>" ); 
	view->write( s->getOwner() );
	view->write( "</td><td>" );
	view->write( s->getGroup() );
	view->write( "</td><td align=right>" );
	QString tmp;
	tmp.sprintf( "%i</td><td>", s->getSize() );
	view->write( tmp.data() );
	view->write( s->getCreationDate() );
	view->write( "</td></td></tr>" );
    }
}

void KFMManager::slotData( const char *_text, int )
{
    QString tmp;
    // HACK
    // Special tag that is only created by kioslave and may only
    // appear at the beginning of some data block.
    if ( strncmp( _text, "<icon ", 6 ) == 0 )
    {
	QString tmp2( _text + 6 );
	tmp2.truncate( tmp2.length() - 1 );
	// Replace the data block with an image tag.
	tmp = "<img border=0 src=\"";
	tmp += KMimeType::getPixmapFileStatic( tmp2 );
	tmp += "\">";
	_text = tmp;
    }
    
    pageBuffer += _text;
    if ( bBufferPage )
	return;
    
    HTMLBuffer += _text;
    do
    {
	int i = HTMLBuffer.find( '\n' );
	int j = HTMLBuffer.find( '\r' );
	if ( j > i ) i = j;
	if ( i == -1 )
	    return;
	int l = HTMLBuffer.length();
	if ( i + 1 == l )
	{
	    view->write( HTMLBuffer );
	    HTMLBuffer = "";
	    return;
	}
    
	char c = HTMLBuffer[i + 1];
	HTMLBuffer[i + 1] = 0;
	view->write( HTMLBuffer );
	HTMLBuffer[i + 1] = c;
	memmove( HTMLBuffer.data(), HTMLBuffer.data() + i + 1, l - i );
    } while( 1 );
}

void KFMManager::stop()
{
    view->getGUI()->slotRemoveWaitingWidget( view );
    job->stop();

    if ( !bFinished )
	view->end();
}

void KFMManager::slotRedirection( const char *_url )
{
    printf("REDIRECTION!!!\n");
    url = _url;
    view->getGUI()->setToolbarURL( _url );
    // view->getGUI()->slotSetStatusBar( _text );
}

void KFMManager::slotInfo( const char *_text )
{
    view->getGUI()->slotSetStatusBar( _text );
}

void KFMManager::slotMimeType( const char *_type )
{
    // Recursion for special mime types which are
    // handled by KFM itself

    // GZIP
    if ( _type &&  strcmp( _type, "application/x-gzip" ) == 0L )
    {
	job->stop();
	tryURL += "#gzip:/";
	openURL( tryURL, bReload );
    }
    // TAR
    else if ( _type && strcmp( _type, "application/x-tar" ) == 0L )
    {
	// Is this tar file perhaps hosted in a gzipped file ?
	KURL u( tryURL );
	// ... then we already have a 'gzip' subprotocol
	if ( u.hasSubProtocol() )
	{
	    KURL u2( u.nestedURL() );
	    if ( strcmp( u2.protocol(), "gzip" ) == 0 )
	    {
		// Remove the 'gzip' protocol. It will only slow down the process,
		// since two subprotocols '#gzip:/#tar:/' are not very fast
		// right now.
		tryURL = u.parentURL();
	    }
	}
	
	job->stop();
	tryURL += "#tar:/";
	openURL( tryURL, bReload );
    }
    // No HTML ?
    else if ( _type == 0L || strcmp( _type, "text/html" ) != 0L )
    {
	view->getGUI()->slotRemoveWaitingWidget( view );

	// Stop browsing. We need an application
	job->stop();

	// Do we know the mime type ?
	if ( _type )
	{
	    KMimeType *typ = KMimeType::findByName( _type );
	    // Have we registered this mime type in KDE ?
	    if ( typ && typ->run( tryURL ) )
		return;
	}
	
	// Ask the user what we should do
	DlgLineEntry l( klocale->translate("Open With:"), "", 0L, true );
	// debugT("OPENING DLG\n");
	if ( l.exec() )
	{
	    QString pattern = l.getText();
	    if ( pattern.isEmpty() )
		return;

	    QStrList list;
	    list.append( tryURL );
	    openWithOldApplication( l.getText(), list );
	    
	    /* QString decoded( tryURL );
	    KURL::decodeURL( decoded );
	    decoded = KIOServer::shellQuote( decoded ).data();
	    
	    QString cmd;
	    cmd = l.getText();
	    cmd += " ";
	    cmd += "\"";
	    cmd += decoded;
	    cmd += "\"";
	    // debugT("Executing stuff '%s'\n", cmd.data());
	    
	    KMimeBind::runCmd( cmd.data() ); */
	}
	return;
    }
    else
    {
	// Push the old URL on the stack if we are allowed to
	if ( !url.isEmpty() && !bHistoryStackLock )
	    view->slotURLToStack( url.data() );
	bHTML = TRUE;
	// Clear the page buffer
	pageBuffer = "";
	// The 'job->browse' command was successful. So lets
	// get the correct URL. This URL may vary from the URL
	// passed to 'openURL' in a trailing "/" for example.
	// Or we got a HTTP redirection or stuff like that.
	url = job->getURL();
	KURL u( url );
	// Lets get the directory
	// QString u2 = u.directoryURL();

	// Initialize the HTML widget,
	// but only if it is NOT a local file.
	// For local files we dont provide progressive updates.
	if ( u.hasSubProtocol() || strcmp( u.protocol(), "file" ) != 0 )
	{
	    bBufferPage = FALSE;
	    // view->begin( u2 );
	    view->begin( url );
	    view->parse();
	}
	else
	    bBufferPage = TRUE;
    }
}

void KFMManager::slotFinished()
{
    bFinished = TRUE;

    // We retrieved a ready to go HTML page ?
    if ( bHTML )
    {
	KURL u( url );
	// Did we buffer the complete HTML stuff ?
	if ( bBufferPage )
	{
	    // Display it now
	    // QString u2 = u.directoryURL();
	    view->begin( url );
	    view->write( pageBuffer );
	    view->parse();
	}
	// Empty the line buffer
	else if ( !HTMLBuffer.isEmpty() )
	    slotData( "\n", 1 );
	view->end();
	// Checkin this page in the cache
	if ( !u.hasSubProtocol() && ( strcmp( u.protocol(), "http" ) == 0 ||
				      strcmp( u.protocol(), "cgi" ) == 0 ) )
	     view->getHTMLCache()->slotCheckinURL( url, pageBuffer );
	// Our job is done
	return;
    }
    
    if ( files.count() == 0 )
	writeBeginning();
		
    // Write the end of the HTML stuff
    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
	view->write( "</body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	view->write( "</table></body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	view->write( "</table></body></html>" );
    
    view->end();
}

void KFMManager::slotPopupActivated( int _id )
{
    if ( popupMenu->text( _id ) == 0 )
	return;
    
    // Text of the menu entry
    QString txt = popupMenu->text( _id );
    
    // Is this some KFM internal stuff ?
    if ( isBindingHardcoded( txt ) )
	return;

    // Loop over all selected files
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	// debugT("Exec '%s'\n", s );
	// Run the action 'txt' on every single file
	KMimeBind::runBinding( s, txt );    
    }
}

void KFMManager::openPopupMenu( QStrList &_urls, const QPoint & _point, bool _current_dir )
{
    // Check wether all URLs are correct
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	// debugT("Opening for '%s'\n",s);
	
	KURL u( s );
	if ( u.isMalformed() )
	{
	    QString tmp;
	    tmp << klocale->translate("Malformed URL\n") << s;
	    QMessageBox::warning( 0, klocale->translate("KFM Error"), tmp );
	    return;
	}
    }
    
    popupMenu->clear();
    // store the mouse position. (Matthias)
    popupMenuPosition = QCursor::pos();       
        
    int isdir = KIOServer::isDir( _urls );
    
    if ( KIOServer::isTrash( _urls ) )
    {
	int id;
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_CD), 
				    view, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_NEW_VIEW), 
				    view, SLOT( slotPopupNewView() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_TRASH), 
				    view, SLOT( slotPopupEmptyTrashBin() ) );
    } 
    else if ( isdir == 1 )
    {
	int id;
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_OPEN_WITH), 
				    view, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_CD), view, 
				    SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_NEW_VIEW), 
				    view, SLOT( slotPopupNewView() ) );
	popupMenu->insertSeparator();    
	if ( KIOServer::supports( _urls, KIO_Read ) )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_COPY ), 
					view, SLOT( slotPopupCopy() ) );
	if ( KIOServer::supports( _urls, KIO_Write ) && KfmView::clipboard->count() != 0 )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_PASTE ), 
					view, SLOT( slotPopupPaste() ) );
	if ( KIOServer::supports( _urls, KIO_Move ) && !_current_dir )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_MOVE_TO_TRASH ),  
					view, SLOT( slotPopupTrash() ) );
	if ( KIOServer::supports( _urls, KIO_Delete ) && !_current_dir )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_DELETE ),  
					view, SLOT( slotPopupDelete() ) );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_OPEN_WITH), 
				    view, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	if ( KIOServer::supports( _urls, KIO_Read ) )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_COPY ), 
					view, SLOT( slotPopupCopy() ) );
	if ( KIOServer::supports( _urls, KIO_Move ) && !_current_dir )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_MOVE_TO_TRASH ),  
					view, SLOT( slotPopupTrash() ) );
	if ( KIOServer::supports( _urls, KIO_Delete ) && !_current_dir )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_DELETE ),  
					view, SLOT( slotPopupDelete() ) );
    }

    popupMenu->insertItem( klocale->translate("Add To Bookmarks"), 
			   view, SLOT( slotPopupBookmarks() ) );

    view->setPopupFiles( _urls );
    popupFiles.copy( _urls );

    QStrList bindings;
    QStrList bindings2;
    QStrList bindings3;
    QList<QPixmap> pixlist;
    QList<QPixmap> pixlist2;
    QList<QPixmap> pixlist3;

    // Get all bindings matching all files.
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	// If this is the first file in the list, assume that all bindings are ok
	if ( s == _urls.getFirst() )
	{
	    KMimeType::getBindings( bindings, pixlist, s, isdir );
	}
	// Take only bindings, matching all files.
	else
	{
	    KMimeType::getBindings( bindings2, pixlist2, s, isdir );
	    char *b;
	    QPixmap *p = pixlist.first();
	    // Look thru all bindings we have so far
	    for ( b = bindings.first(); b != 0L; b = bindings.next() )
	    {
		// Does the binding match this file, too ?
		if ( bindings2.find( b ) != -1 )
		{
		    // Keep these entries
		    bindings3.append( b );
		    pixlist3.append( p );
		}
		p = pixlist.next();
	    }
	    pixlist = pixlist3;
	    bindings = bindings3;
	}
    }
    
    // Add all bindings to the menu
    if ( !bindings.isEmpty() )
    {
	popupMenu->insertSeparator();

	char *str;
	QPixmap *p = pixlist.first();
	for ( str = bindings.first(); str != 0L; str = bindings.next() )
	{
	    if ( p != 0L && !p->isNull() )
		popupMenu->insertItem( *p, str );
	    else
		popupMenu->insertItem( str );
	    p = pixlist.next();
	}
    }

    // Allow properties only if exactly one file is selected
    if ( _urls.count() == 1 )
    {
	popupMenu->insertSeparator();
	popupMenu->insertItem( klocale->translate("Properties"), view, SLOT( slotPopupProperties() ) );
    }
    
    // Show the menu
    popupMenu->popup( _point );
}

void KFMManager::dropPopupMenu( KDNDDropZone *_zone, const char *_dest, const QPoint *_p, bool _nestedURLs )
{
    dropDestination = _dest;
    dropDestination.detach();
    
    dropZone = _zone;
    
    // debugT(" Drop with destination %s\n", _dest );
    
    KURL u( _dest );
    
    // Perhaps an executable ?
    // So lets ask wether we can be shure that it is no directory
    // We can rely on this, since executables are on the local hard disk
    // and KIOServer can query the local hard disk very quickly.
    if ( KIOServer::isDir( _dest ) == 0 )
    {
	// Executables or only of interest on the local hard disk
	if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	{
	    KMimeType *typ = KMimeType::getMagicMimeType( _dest );
	    if ( typ->runAsApplication( _dest, &(_zone->getURLList() ) ) )
		return;

	    /* // Run the executable with the dropped 
	    // files as arguments
	    if ( KMimeBind::runBinding( _dest, klocale->getAlias(ID_STRING_OPEN), &(_zone->getURLList() ) ) )
		// Everything went fine
		return; */
	    else
	    {
		// We did not find some binding to execute
		QMessageBox::warning( 0, klocale->translate("KFM Error"), 
				      klocale->translate("Dont know what to do.") );
		return;
	    }
	}
    }
    
    popupMenu->clear();
    
    int id = -1;
    // Ask wether we can read from the dropped URL.
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( _dest, KIO_Write ) && !_nestedURLs )
	id = popupMenu->insertItem(  klocale->translate("Copy"), 
				     this, SLOT( slotDropCopy() ) );
    // Ask wether we can read from the URL and delete it afterwards
    if ( KIOServer::supports( _zone->getURLList(), KIO_Move ) &&
	 KIOServer::supports( _dest, KIO_Write ) && !_nestedURLs )
	id = popupMenu->insertItem(  klocale->translate("Move"),
				     this, SLOT( slotDropMove() ) );
    // Ask wether we can link the URL 
    if ( KIOServer::supports( _dest, KIO_Link ) )
	id = popupMenu->insertItem(  klocale->translate("Link"), 
				     this, SLOT( slotDropLink() ) );
    if ( id == -1 )
    {
	QMessageBox::warning( 0, klocale->translate("KFM Error"),
			       klocale->translate("Dont know what to do") );
	return;
    }

    // Show the popup menu
    popupMenu->popup( *_p );
}

void KFMManager::slotDropCopy()
{
    KIOJob * job = new KIOJob;
    job->copy( dropZone->getURLList(), dropDestination.data() );
}

void KFMManager::slotDropMove()
{
    KIOJob * job = new KIOJob;
    job->move( dropZone->getURLList(), dropDestination.data() );
}

void KFMManager::slotDropLink()
{
    KIOJob * job = new KIOJob;
    job->link( dropZone->getURLList(), dropDestination.data() );
}

QString KFMManager::getVisualSchnauzerIconTag( const char *_url )
{
    // directories URL
    KURL u( url );
    // URL of the file we need an icon for
    KURL u2( _url );
    
    // Look for .xvpics directory on local hard disk only.
    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
    {
	struct stat buff;
	lstat( u2.path(), &buff );
	// Is it a regular file ?
	if ( S_ISREG( buff.st_mode ) )
	{
	    // Path for .xvpics
	    QString xv = u.directory();
	    if ( xv.right(1) != "/" )
		xv += "/.xvpics";
	    else
		xv += ".xvpics";
	    // debugT("XV='%s'\n",xv.data());
	    
	    // Does the .xvpics directory exist ?
	    DIR *dp = opendir( xv.data() );
	    if ( dp != NULL )
	    {
		closedir( dp );
		
		xv += "/";
		// debugT("XV2='%s'\n",xv.data());
		
		// Assume XV pic is not available
		bool is_avail = FALSE;
		// Assume that the xv pic has size 0
		bool is_null = TRUE;
		
		// Time of the original image
		time_t t1 = buff.st_mtime;
		if ( buff.st_size != 0 )
		    is_null = FALSE;
		// Get the times of the xv pic
		QString xvfile( xv.data() );
		xvfile += u2.filename();
		if ( !xv.isEmpty() )
		{
		    // debugT("Local XVFile '%s'\n",xvfile.data());
		    // Is the XV pic available ?
		    if ( lstat( xvfile, &buff ) == 0 )
		    {
			time_t t2 = buff.st_mtime;
			// Is it outdated ?
			if ( t1 <= t2 )
			    is_avail = TRUE;
		    }
		}
		
		// Do we have a thumb nail image already ?
		if ( is_avail && !is_null )
		{
		    // Does it really contain an image ?
		    FILE *f = fopen( xvfile, "rb" );
		    if ( f != 0L )
		    {
			char str4[ 1024 ];
			fgets( str4, 1024, f );
			if ( strncmp( "P7 332", str4, 6 ) != 0 )
			    is_null = TRUE;
			// Skip line
			fgets( str4, 1024, f );
			fgets( str4, 1024, f );
			if ( strncmp( "#BUILTIN:UNKNOWN", str4, 16 ) == 0 )
			    is_null = TRUE;
			fclose( f );
		    }
		}
		
		if ( is_avail && !is_null )
		{
		    printf("******** XVPIC found for '%s'\n",_url );
		    QString result;
		    result.sprintf("<img border=0 src=\"file:%s\">", xvfile.data() );
		    return result;
		}
	    }
	}
    
	// At this time the icon protocol works for local files only.
	KMimeType *mime = KMimeType::getMagicMimeType( _url );
	if ( strcmp( mime->getMimeType(), "image/jpeg" ) == 0 ||
	     strcmp( mime->getMimeType(), "image/gif" ) == 0 ||
	     strcmp( mime->getMimeType(), "image/x-xpm" ) == 0 )
	{
	    printf("*********** TRYING for '%s'\n",_url );
	    QString result;
	    result.sprintf("<img border=0 src=\"icon:%s\">", u2.path() );
	    return result;
	}
    }
    
    printf("************ DEFAULT for '%s'\n",_url);
    KMimeType *mime = KMimeType::getMagicMimeType( _url );
    QString result;
    result.sprintf("<img border=0 src=\"file:%s\">", mime->getPixmapFile( _url ) );
    return result;
}

#include "kfmman.moc"
