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
#include <Kconfig.h>

#include "kfmman.h"
#include "xview.h"
#include "kbind.h"
#include <config-kfm.h>
#include "kmsgwin.h"
#include "kfmdlg.h"

#include <klocale.h>
#define klocale KLocale::klocale()

// A HACK
QString HTMLBuffer;

KFMManager::KFMManager( KfmView *_v )
{
    url  = "";
    view = _v;
    maxLabelWidth = 80;
    labelFontMetrics = new QFontMetrics(view->defaultFont());
    popupMenu = new QPopupMenu();
    popupMenu->installEventFilter( this );

    files.setAutoDelete( true );
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );
    job = new KFMJob;
    connect( job, SIGNAL( error( const char* ) ), this, SLOT( slotError( const char* ) ) );
    connect( job, SIGNAL( newDirEntry( KIODirectoryEntry* ) ),
	     this, SLOT( slotNewDirEntry( KIODirectoryEntry* ) ) );
    connect( job, SIGNAL( finished() ), this, SLOT( slotFinished() ) );
    connect( job, SIGNAL( data( const char * ) ), this, SLOT( slotData( const char * ) ) );
    connect( job, SIGNAL( mimeType( const char* ) ), this, SLOT( slotMimeType( const char* ) ) );
    connect( job, SIGNAL( info( const char* ) ), this, SLOT( slotInfo( const char* ) ) );
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
      QString str_str(_str);
      KURL::decodeURL(str_str);
	view->write( str_str );
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
		if (ispunct(*sepPos) || isdigit(*sepPos)) break;
       if (isupper(*sepPos) && !isupper(*(sepPos-1))) break;
	    }
	    if (j<8 && j>0)
	    {
		pos = sepPos;
		//width = width - XTextWidth (fs, pos, j);
	    }
	    
	    c = *pos;
	    *pos = '\0';
	    view->write( part );
	    view->write( "<br>" );
	    *pos = c;
	    part = pos;
	    width = 0;
	}
	width += charWidth;
    }
    if (*part) view->write( part );
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
    debugT("Changing to ftp %s\n", _url );
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
	warning(klocale->translate("ERROR: Malformed URL"));
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
	KURL::decodeURL( path );
    
	// Try tp open the *.kdelnk file
	QFile file( path );
	if ( file.open( IO_ReadOnly ) )
	{
	    QTextStream pstream( &file );
	    KConfig config( &pstream );
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
		    QMessageBox::message( klocale->translate("KFM Error"), 
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
    // This gives us some speedup on local hard disks.
    if ( KIOServer::isDir( _url ) == 0 )
    {    
	debugT("It is for shure a FILE\n");
	// A HACK
	// We must support plugin protocols here!
	// Do we try to open a tar file? We try to figure this
	// out by looking at the extension only.
	KMimeType *typ = KMimeType::findType( _url );
	printf("Type: '%s'\n",typ->getMimeType());
	
	if ( strcmp( typ->getMimeType(), "application/x-tar" ) == 0L )
	{
	    // We change the destination on the fly
	    tryURL = _url;
	    tryURL += "#tar:/";
	}
	// HTML stuff is handled by us
	else if ( strcmp( typ->getMimeType(), "text/html" ) == 0L )
	{
	    tryURL = _url;
	}
	else
	{
	    printf("EXEC MIMETYPE\n");
	    // Can we run some default binding ?
	    if ( KMimeBind::runBinding( _url ) )
		return false;
	    else // Ok, lets find out wether it is a directory or HTML
		tryURL = _url;
	}
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
    tmp.sprintf( klocale->translate("Contacting host %s"), u.host() );
    view->getGUI()->slotSetStatusBar( tmp );

    return true;
}

void KFMManager::slotError( const char * )
{
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
    view->write( "</title></head><body bgcolor=#FFFFFF>" );
    
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
	    view->write( "<cell><a href=\"");
	    view->write( s.data() );
	    view->write( "\"><center><img border=0 src=\"file:" );
	    view->write( KMimeType::getPixmapFileStatic( s.data() ) );
	    view->write( "\"><br>..</center><br></a></cell>" );
	}
	else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	{
	    view->write( "<tr><td><a href=\"" );
	    view->write( s.data() );
	    view->write( "\"><img border=0 width=16 height=16 src=\"file:" );
	    view->write( KMimeType::getPixmapFileStatic( s.data() ) );
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
    KURL::decodeURL( decoded );
    
    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
    { 
	view->write( "<cell><a href=\"" );
	
	QString filename( url );
	filename.detach();
	filename += s->getName();
	
	view->write( filename.data() );
	view->write( "\"><center><img border=0 src=\"file:" );

	view->write( KMimeType::getPixmapFileStatic( filename ) );
		
	view->write( "\"><br>" );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("<i>");
	strcpy( buffer, decoded );
	writeWrapped( buffer );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("</i>");
	view->write( "</center><br></a></cell>" );
    }
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
    {
	view->write( "<tr><td><a href=\"" );
	QString filename( url );
	filename.detach();
	filename += s->getName();
	
	view->write( filename.data() );
	view->write( "\"><img border=0 width=16 height=16 src=\"file:" );
	view->write( KMimeType::getPixmapFileStatic( filename.data() ) );
	view->write( "\"></td><td>" );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("<i>");
	view->write( decoded );
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
	QString filename( url );
	filename.detach();
	filename += s->getName();
	
	view->write( filename.data() );
	view->write( "\">" );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("<i>");
	view->write( decoded );
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

void KFMManager::slotData( const char *_text )
{
    pageBuffer += _text;
    
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
	printf(">>%s",HTMLBuffer.data());
	view->write( HTMLBuffer );
	printf("--------\n");
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

void KFMManager::slotInfo( const char *_text )
{
    view->getGUI()->slotSetStatusBar( _text );
}

void KFMManager::slotMimeType( const char *_type )
{
    if ( _type == 0L || strcmp( _type, "text/html" ) != 0L )
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
		
	// Try to run the default binding on the URL since we
	// know that it is a real file, but we dont know the
	// mime type, or the mime type is not registered in KDE.
	if ( KMimeBind::runBinding( tryURL ) )
	{
	    debugT("Our job is done\n");
	    return;
	}
	
	// Ask the user what we should do
	DlgLineEntry l( klocale->translate("Open With:"), "", 0L, true );
	debugT("OPENING DLG\n");
	if ( l.exec() )
	{
	    QString pattern = l.getText();
	    if ( pattern.isEmpty() )
		return;

	    QString decoded( tryURL );
	    KURL::decodeURL( decoded );
	    decoded = KIOServer::shellQuote( decoded ).data();
	    
	    QString cmd;
	    cmd = l.getText();
	    cmd += " ";
	    cmd += "\"";
	    cmd += decoded;
	    cmd += "\"";
	    debugT("Executing stuff '%s'\n", cmd.data());
	    
	    KMimeBind::runCmd( cmd.data() );
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
	// Lets get the directory
	KURL u( url );
	QString u2 = u.directoryURL();
	// Initialize the HTML widget
	view->begin( u2 );
	view->parse();
    }
}

void KFMManager::slotFinished()
{
    debugT("FINISHED\n");

    bFinished = TRUE;

    // We retrieved a ready to go HTML page ?
    if ( bHTML )
    {
	// A HACK
	// view->write( HTMLBuffer );
	if ( !HTMLBuffer.isEmpty() )
	    slotData( "\n" );
	view->end();
	// Checkin this page in the cache
	KURL u( url );
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
	debugT("Exec '%s'\n", s );
	// Run the action 'txt' on every single file
	KMimeBind::runBinding( s, txt );    
    }
}

void KFMManager::openPopupMenu( QStrList &_urls, const QPoint & _point )
{
    // Check wether all URLs are correct
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	debugT("Opening for '%s'\n",s);
	
	KURL u( s );
	if ( u.isMalformed() )
	{
	    QString tmp;
	    tmp.sprintf(klocale->translate("Malformed URL\n%s"), s );
	    QMessageBox::message( klocale->translate("KFM Error"), tmp );
	    return;
	}
    }
    
    popupMenu->clear();
    // store the mouse position. (Matthias)
    popupMenuPosition = QCursor::pos();       
    
    QStrList bindings;
    bindings.setAutoDelete( true );
    QStrList bindings2;
    bindings2.setAutoDelete( true );
    
    // char buffer[ 1024 ];
    
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
	    id = popupMenu->insertItem( klocale->translate("Copy"), 
					view, SLOT( slotPopupCopy() ) );
	if ( KIOServer::supports( _urls, KIO_Write ) )
	    id = popupMenu->insertItem( klocale->translate("Paste"), 
					view, SLOT( slotPopupPaste() ) );
	if ( KIOServer::supports( _urls, KIO_Move ) )
	    id = popupMenu->insertItem( klocale->translate("Move to Trash"),  
					view, SLOT( slotPopupTrash() ) );
	if ( KIOServer::supports( _urls, KIO_Delete ) )
	    id = popupMenu->insertItem( klocale->translate("Delete"),  
					view, SLOT( slotPopupDelete() ) );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_OPEN_WITH), 
				    view, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	if ( KIOServer::supports( _urls, KIO_Read ) )
	    id = popupMenu->insertItem( klocale->translate("Copy"), 
					view, SLOT( slotPopupCopy() ) );
	if ( KIOServer::supports( _urls, KIO_Move ) )
	    id = popupMenu->insertItem( klocale->translate("Move to Trash"),  
					view, SLOT( slotPopupTrash() ) );
	if ( KIOServer::supports( _urls, KIO_Delete ) )
	    id = popupMenu->insertItem( klocale->translate("Delete"),  
					view, SLOT( slotPopupDelete() ) );
    }

    popupMenu->insertItem( klocale->translate("Add To Bookmarks"), 
			   view, SLOT( slotPopupBookmark() ) );

    view->setPopupFiles( _urls );
    popupFiles.copy( _urls );
    
    // Get all bindings matching all files.
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	// If this is the first file in the list, assume that all bindings are ok
	if ( s == _urls.getFirst() )
	{
	    KMimeType::getBindings( bindings, s, isdir );
	}
	// Take only bindings, matching all files.
	else
	{
	    KMimeType::getBindings( bindings2, s, isdir );
	    char *b;
	    // Look thru all bindings we have so far
	    for ( b = bindings.first(); b != 0L; b = bindings.next() )
		// Does the binding match this file, too ?
		if ( bindings2.find( b ) == -1 )
		    // If not, delete the binding
		    bindings.removeRef( b );
	}
    }
    
    // Add all bindings to the menu
    if ( !bindings.isEmpty() )
    {
	popupMenu->insertSeparator();

	char *str;
	for ( str = bindings.first(); str != 0L; str = bindings.next() )
	{
	    popupMenu->insertItem( str );
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

void KFMManager::dropPopupMenu( KDNDDropZone *_zone, const char *_dest, const QPoint *_p )
{
    dropDestination = _dest;
    dropDestination.detach();
    
    dropZone = _zone;
    
    debugT(" Drop with destination %s\n", _dest );
    
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
	    // Run the executable with the dropped 
	    // files as arguments
	    if ( KMimeBind::runBinding( _dest, klocale->getAlias(ID_STRING_OPEN), &(_zone->getURLList() ) ) )
		// Everything went fine
		return;
	    else
	    {
		// We did not find some binding to execute
		QMessageBox::message( klocale->translate("KFM Error"), 
				      klocale->translate("Dont know what to do.") );
		return;
	    }
	}
    }
    
    popupMenu->clear();
    
    int id = -1;
    // Ask wether we can read from the dropped URL.
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( _dest, KIO_Write ) )
	id = popupMenu->insertItem(  klocale->translate("Copy"), 
				     this, SLOT( slotDropCopy() ) );
    // Ask wether we can read from the URL and delete it afterwards
    if ( KIOServer::supports( _zone->getURLList(), KIO_Move ) &&
	 KIOServer::supports( _dest, KIO_Write ) )
	id = popupMenu->insertItem(  klocale->translate("Move"),
				     this, SLOT( slotDropMove() ) );
    // Ask wether we can link the URL 
    if ( KIOServer::supports( _dest, KIO_Link ) )
	id = popupMenu->insertItem(  klocale->translate("Link"), 
				     this, SLOT( slotDropLink() ) );
    if ( id == -1 )
    {
	QMessageBox::message(  klocale->translate("KFM Error"),
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

#include "kfmman.moc"
