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

#include "kcookiejar.h"
#include "kcookiewin.h"
#include "kfmman.h"
#include "kbind.h"
#include "config-kfm.h"
#include "kfmdlg.h"
#include "kfmexec.h"
#include "utils.h"
#include "kfm.h"
#include "root.h"
#include "kfmpaths.h"

// enable this to show a readonly indicator over the icon.
//#define OVERLAY_READONLY

QString *KFMManager::link_overlay = 0;
QString *KFMManager::ro_overlay = 0;

KFMManager::KFMManager( KfmView *_v )
{
    //-------- Sven's overlayed mime/app dirs start ---
    bindingDir = false;
    pass2 = false;
    dupList = 0;
    //-------- Sven's overlayed mime/app dirs end ---
    url  = "";
    view = _v;
    maxLabelWidth = 80;
    labelFontMetrics = new QFontMetrics(view->defaultFont());
    popupMenu = new QPopupMenu();
    popupMenu->installEventFilter( this );

    menuNew = new KNewMenu();

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

    if (cookiejar)
    {
        connect( job, SIGNAL( cookie( const char*, const char* ) ), this, SLOT( slotCookie( const char*, const char* ) ) );
    }
            
    if ( !link_overlay )
    {
	link_overlay = new QString;
	*link_overlay = kapp->kde_icondir().copy();
	*link_overlay += "/link.xpm";
	HTMLImage::cacheImage( link_overlay->data() );
    }
    if ( !ro_overlay )
    {
	ro_overlay = new QString;
	*ro_overlay = kapp->kde_icondir().copy();
	*ro_overlay += "/readonly.xpm";
	HTMLImage::cacheImage( ro_overlay->data() );
    }
}

KFMManager::~KFMManager()
{
    if (labelFontMetrics) delete labelFontMetrics;
    delete menuNew;
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
    if ( strcmp( klocale->getAlias(ID_STRING_SAVE_URL_PROPS), _txt ) == 0)
        return true;
    if ( strcmp( klocale->getAlias(ID_STRING_SHOW_MENUBAR), _txt ) == 0)
        return true;
    if ( strcmp( klocale->getAlias(ID_STRING_UP), _txt ) == 0)
        return true;
    if ( strcmp( klocale->getAlias(ID_STRING_BACK), _txt ) == 0)
        return true;
    if ( strcmp( klocale->getAlias(ID_STRING_FORWARD), _txt ) == 0)
        return true;
                                    
    return false;
}
  
void KFMManager::writeWrapped( char *_str, int _maxlen )
{
    short        j, width, charWidth;
    char*        pos;
    // char*        first;
    char*        sepPos;
    char* part;
    char  c;
    
    if ( _maxlen == -1 )
      _maxlen = maxLabelWidth;
    
    if ( labelFontMetrics->width(_str) <= _maxlen )
    {
	view->writeHTMLquoted ( _str );
	return;
    }

    for (width=0, part=_str, pos=_str; *pos; pos++)
    {
	charWidth = labelFontMetrics->width( *pos );
	if ( width+charWidth >= _maxlen )
	{
	    // search for a suitable separator in the previous 8 characters
	    for (sepPos=pos, j=0; j<8 && sepPos>part; j++, sepPos--)
	    {
		/* if (ispunct(*sepPos) || isdigit(*sepPos)) break;
		   if (isupper(*sepPos) && !isupper(*(sepPos-1))) break;
	    }
	    if (j<8 && j>0)
	    {
		pos = sepPos;
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
    
bool KFMManager::openURL( const char *_url, bool _reload, int _xoffset, int _yoffset, const char *_data )
{
    jobURL = "";

    nextXOffset = _xoffset;
    nextYOffset = _yoffset;
    if (_reload && (_xoffset==0) && (_yoffset==0)) {
        // Reload and no offsets specified. Keep the current ones.
        nextXOffset = view->xOffset();
        nextYOffset = view->yOffset();
    }

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
    
    QString cachePath("file:");
    cachePath += KFMPaths::CachePath().data();
    cachePath += "/index.html";
    if (cachePath == _url)
          HTMLCache::save();
    
    // Store the parameter for recursive function calls
    bReload = _reload;
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
	warning(klocale->translate("ERROR: Malformed URL"));
	return false;
    }

    Kfm::addToHistory( _url );
    
    // Dirty hack for calling kmail
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
    
    /*
     * Check if we want to jump to a reference or (x,y)-pos within the 
     * current page and jump directly to it without reloading / reparsing
     * 
     * Don't do this if a reload request was made
     */

    if ( !_reload && 
    	 ( (strlen(u.reference()) != 0) || 
    	   (nextXOffset != 0) || 
    	   (nextYOffset != 0)
    	 )
       )
    {
    	KURL newUrlWithoutRef( _url );	
    	KURL oldUrlWithoutRef( url );
    	newUrlWithoutRef.setReference( "" );	
    	oldUrlWithoutRef.setReference( "" );	
    	if (newUrlWithoutRef.url() == oldUrlWithoutRef.url())
    	{
	    // Add old URL to history stacks
	    view->setUpURL( newUrlWithoutRef.url() ); // ?? (David)
	    view->pushURLToHistory();
	    if ((nextXOffset != 0) || (nextYOffset != 0))
	    {
	    	return (view->gotoXY(nextXOffset, nextYOffset));    	     
            }
	    else
	    { 
                return (view->gotoAnchor(u.reference()));
            }
    	}
    }

    // Is page cached ?
    const char *file;
    if ( ( file = view->getHTMLCache()->isCached( _url ) ) != 0L && 
         !_reload && 
         !_data
       )
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

	    // Add old URL to history stacks
	    view->pushURLToHistory();

	    url = _url;
            setUpURL();
            view->setHTMLMode(true);
	    view->begin( _url, nextXOffset, nextYOffset );
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
    if ( u.isLocalFile() && KIOServer::isDir(_url) == 0 && path.right(7) == ".kdelnk" )
    {
	// Try to open the *.kdelnk file
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

    bHTML = FALSE;
    bFinished = FALSE;
    // Prevent us from modifying the history stack if the stack
    // is locked during this functions run. Mention that the
    // stack may be unlocked when we want to modify it. So
    // we have to remember the current state here.
    bHistoryStackLock = view->isHistoryStackLocked();
    
    // Do we know that it is !really! a file ?
    // Then we can determine the mime type for sure and run
    // the best matching binding
    if ( u.isLocalFile() && KIOServer::isDir( _url ) == 0 )
    {    
	tryURL = KFMExec::openLocalURL( _url );
	if ( tryURL.isEmpty() )
	    return false;
    }
    else
	// We try to load this URL now
	tryURL = _url;

    // Start the rotating gear
    view->getGUI()->slotAddWaitingWidget( view );
    
    // Stop a running job. Calling this if no job is running does no harm.
    job->stop();

    //-------- Sven's overlayed mime/app dirs start ---
    // Here we check if tryURL is (part of) local/global mimedir or local
    // applnkdir. In that case we set flage bindingDir == true.
    // In that case:
    //   - when this job finishes (in slotFinished()) we will start
    //     new job to read global mime/applnk dir too an display them
    //     here
    //   - writeEntry() will check for and discard duplicates
    // We must take care about dirs; suppose user has only image/ dir.
    // Then all other dirs (audio/, text/...) would be from global dir.
    // In each case "share/mimelnk" or "share/applnk" are part of the
    // path. in each case when we find kde_appsdir, kde_mimedir,
    // kde_localkdedir + "share/mimelnk", local_kdedir + "share/applnk"
    // to be a part of tryURL we first read local variant, and then
    // global. We must also test does local thing exist; if not we go
    // to pass2,

    

#define GLOBALMIME kapp->kde_mimedir().data()
#define LOCALMIME (kapp->localkdedir() + "/share/mimelnk").data()

#define GLOBALAPPS kapp->kde_appsdir().data()
#define LOCALAPPS (kapp->localkdedir() + "/share/applnk").data()

    // First it is possible that use clicks on DIR that doesn't exist
    // in user's directory. In that case, pretend nothing happened

    QString tryPath = tryURL;
    //debug ("DIROVERLAY: OK, url to test is %s", tryPath.data());
    
    if (tryPath.contains(GLOBALMIME))
    {
      tryPath.remove(5, strlen(GLOBALMIME));
      tryPath.insert(5, LOCALMIME);
      
      bindingDir = true;
      pass2 = false;
    }

    else if (tryPath.contains(LOCALMIME))
    {
      bindingDir = true;
      pass2 = false;
    }
    
    else  if (tryPath.contains(GLOBALAPPS))
    {
      tryPath.remove(5, strlen(GLOBALAPPS));
      tryPath.insert(5, LOCALAPPS);
      bindingDir = true;
      pass2 = false;
    }

    else if (tryPath.contains(LOCALAPPS))
    {
      bindingDir = true;
      pass2 = false;
    }

    // We must check here are we root; In that case whole storry is off
    if (view->getGUI()->sumode)
    {
      //debug ("I'm a rooooooot!!!!!!!!!!");
      bindingDir = false;
      pass2 = false;
    }
    
    if (bindingDir)
    {
      // Now check does tryPath exist at all
      const char *f = tryPath.data();
      if (access(&f[5], F_OK) == 0)
      {
	//debug ("DIROVERLAY: OK, %s exists!", tryPath.data());
	tryURL = tryPath;
	if (dupList == 0) // create dupList
	{
	  dupList = new QStrList(true); // deep copies
	  dupList->setAutoDelete(true); // delete on end
	}
	else // Sometimes when reloading, mimedir list still exists
	  dupList->clear(); //deleted in stop, slotFinished, slotError.
      }
      else
      {
	//debug ("DIROVERLAY: OK, %s doesn't exist!", tryPath.data());
	bindingDir = false;
        pass2 = false;
      }
    }
    //-------- Sven's overlayed mime/app dirs end ---

    // We pass our current KIODirectoryEntries. Sometimes this information
    // can speed up the process of finding out whether tryURL is
    // a directory or some file. Mention that files is cleared
    // at once by a call to this function. 'browse' may emit the signal
    // 'newDirEntry' which uses 'files'. So 'files' has to be cleared
    // before! such signal is emitted.

    job->browse( tryURL, _reload, view->getGUI()->isViewHTML(), url, &files, _data );

    if (_reload)
        // Restore the position where we were in the page. David.
        view->gotoXY(nextXOffset, nextYOffset);
    
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

    // New URL displayed : reset the current key sequence
    view->getKHTMLWidget()->resetCurrentKeySeq();

    return true;
}

void KFMManager::slotError( int, const char * )
{
    //-------- Sven's overlayed mime/app dirs start ---
    pass2 = false;
    bindingDir = false;
    if (dupList)
    {
      delete dupList;
      dupList = 0;
    }
    //-------- Sven's overlayed mime/app dirs end ---
    bFinished = TRUE;

    // Stop the spinning gear
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
    QString tmp2 = config->readEntry( "BgImage" );
    QString bg_image;
    if ( !tmp2.isNull() )
	if ( tmp2.data()[0] != 0 )
	{
	    bg_image = "file:";
	    if (tmp2.left(1)!="/") // relative path
            {
              bg_image += kapp->kde_wallpaperdir().copy();
              bg_image += "/";
            }
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
                  bg_image = "";
                  if (tmp.left(1)!="/") // relative path
                  {
		    bg_image = kapp->kde_wallpaperdir().copy();
		    bg_image += "/";
                  }
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

void KFMManager::setUpURL()
{
    KURL u( url.data() );
    
    if ( strcmp( u.path(), "/" ) != 0 )
    {
        KURL u2( url.data() );
        u2.cd("..");
	
        QString s = u2.url();
        if ( s.right( 1 ) != "/" )
            s += "/";

        view->setUpURL( s );	
    }
    else
        view->setUpURL( "/" );
}

void KFMManager::writeBeginning()
{
    // Push the old URL on the stack if we are allowed to
    if ( !url.isEmpty() && !bHistoryStackLock )
	view->pushURLToHistory();
    // The 'job->browse' command was successful. So lets
    // get the correct URL. This URL may vary from the URL
    // passed to 'openURL' in a trailing "/" for example.
    // Or we got a HTTP redirection or stuff like that.
    url = job->getURL();
    
    view->begin( url.data(), nextXOffset, nextYOffset );
    view->parse();
    view->write( "<html><head><title>" );
    view->write( url.data() );
    view->write( "</title></head>" );
    
    writeBodyTag();
    
    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
    { }
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
        view->write( "<table cellspacing=0 cellpadding=0 width=100% padding=1>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	view->write( "<table cellspacing=0 cellpadding=0 width=100% padding=1>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::SHORT_VIEW )
	;
    setUpURL();
    view->setHTMLMode(false);
}

void KFMManager::writeEnd()
{
    // Write the end of the HTML stuff
    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
	view->write( "</body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	view->write( "</table></body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	view->write( "</table></body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::SHORT_VIEW )
	view->write( "</body></html>" );
    
    view->end();
}

void KFMManager::writeEntry( KIODirectoryEntry *s )
{
    char buffer[ 1024 ];

    if ( strcmp( s->getName(), "." ) == 0 || strcmp( s->getName(), ".." ) == 0 ||
	 strcmp( s->getName(), "./" ) == 0 || strcmp( s->getName(), "../" ) == 0 )
	return;

    if ( s->getName()[0] == '.' && !view->getGUI()->isShowDot() )
	return;

    //-------- Sven's overlayed mime/app dirs start ---
    // We load first user's dir and then global dir
    // kdelnks/mimelnks from both dirs will be shown in one window
    // so we must discard those *lnks from global dir that are already
    // read from local.
    // Since I cannot read them from the files list, I must have my own
    // special list for that :-(
    if (bindingDir) // flag if we read those "system" dirs
    {
      if (!pass2) // pass 1 or 2?
      {
	if (dupList)
	  dupList->append(s->getName()); // we're in first pass; add to list;
	else // debug fallback
	{
	  debug ("DIROVERLAY: BAD, no dupList! (pass 1)");
	  return;
	}
      }
      else // pass 2
      {
	// this is pass 2; discard duplicates
	if (dupList)
	{
	  if (dupList->contains(s->getName())) // if we have it...
	  {
	    debug ("DIROVERLAY: OK, discarded duplicate %s", s->getName());
	    // we might remove it from the list making future search
	    // faster. But since removing would imply search (we
	    // cannot search by reference, copies are deep, they
	    // have to be deep since I don't trust if old
	    // KIODirectoryEntries live long enough) we wouldn gain
	    // much if anything at all.
	    return;                                    // yeah, discard.
	  }
	}
	else // debug fallback
	{
	  debug ("DIROVERLAY: BAD, no dupList! (pass 2)");
	  return;
	}

      }
    }

    // Maybe when we rmb click to local lnk displayed here, we get
    // nonexisting file; kfmprops will in thate case from local dir.
    
    //-------- Sven's overlayed mime/app dirs end ---
    
    /* QString decoded = s->getName();
    decoded.detach();
    KURL::decodeURL( decoded );   // decoded to pass to writeWrapped()

    QString filename( url );        // filename, Filename, useable to find file on Disk (Hen)
    filename.detach();
    filename += s->getName();

    QString encodedURL ( url );  
    encodedURL.detach();            // encodedURL, URL,  encoded for <a href ..> (Hen)
    encodedURL += s->getName(); */

    QString n( s->getName() );
    KURL::encodeURL( n );
    QString encodedURL = url.data();
    encodedURL += n;
    
    QString decoded( s->getName() );
    decodeFileName( decoded );
    
    // Hack
    QString filename = encodedURL;

    //-------- Sven's overlayed mime/app dirs start ---
    // we have foreign items in our directory. To be able to access
    // them they must have their own correct paths. This counts only
    // for pass2: items for global dir shown in lcoal

    if (bindingDir && pass2)
    {
      // We must fix encodedURL to point where real mime/kdelnk is: to
      // global place.
      if (encodedURL.contains(LOCALMIME))
      {
	encodedURL.remove(5, strlen(LOCALMIME)); // 5 is for "file:"
	encodedURL.insert(5, GLOBALMIME);
        filename = encodedURL;
	//debug ("encodedURL = %s", encodedURL.data());
      }

      else if (encodedURL.contains(LOCALAPPS))
      {
	encodedURL.remove(5, strlen(LOCALAPPS));
	encodedURL.insert(5, GLOBALAPPS);
	filename = encodedURL;
      }
      else  // Debug fallback
      {
	debug ("DIROVERLAY:BAD, encodedURL doesn't contain LOCAL part: %s (pass2)",
	       encodedURL.data());
	return;
      }
    }
    //-------- Sven's overlayed mime/app dirs end ---
    
    bool readonly = false;

#ifdef OVERLAY_READONLY
    //KURL u( filename );
    KURL u( encodedURL )
    // if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
    if ( u.isLocalFile() )
    {
	if ( access( u.path(), W_OK ) < 0 )
	    readonly = true;
    }
    else
    {
	QString user = u.user();
	if ( user.data() == 0L || user.data()[0] == 0 )
	    user = "anonymous";
	
	if ( !s->mayWrite( user ) )
	    readonly = true;
    }
#endif

    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
    { 
	// Delete ".kdelnk" extension ( only in Icon View )
	if ( decoded.right(7) == ".kdelnk" )
	    decoded.truncate( decoded.length() - 7 );

	view->write( "<a href=\"" );
	
	view->write( encodedURL.data() );
	if ( view->getGUI()->isVisualSchnauzer() )
	{
	    view->write( "\"><cell><center>" );
	    view->write( getVisualSchnauzerIconTag( filename ).data() );
	    view->write( "<br>" );
	}
	else
	{
	    view->write( "\"><cell><center><img border=0 src=\"file:" );
	    view->write( KMimeType::getPixmapFileStatic( filename ) );
	    if ( readonly )
	    {
		view->write( "\" oversrc=\"" );
		view->write( ro_overlay->data() );
	    }
	    else if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    {
		view->write( "\" oversrc=\"" );
		view->write( link_overlay->data() );
	    }
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
	view->write( "<tr><td width=22><a href=\"" );

	view->write( encodedURL.data() );
	view->write( "\"><img border=0 width=16 height=16 src=\"file:" );
	view->write( KMimeType::getPixmapFileStatic( filename.data(), TRUE ) );
	view->write( "\"></td><td><cell width=100% padding=0>" );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("<i>");
	view->writeHTMLquoted ( decoded );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("</i>");
	view->write( "</cell></a></td><td><tt>" ); 
	view->write( s->getAccess() );
	view->write( "</tt></td><td>" );
	view->write( s->getOwner() );
	view->write( "</td><td>" );
	view->write( s->getGroup() );
	view->write( "</td><td align=right>" );
	QString tmp;
        // write the size and then an empty cell to add some space
	tmp.sprintf( "%i </td><td>&nbsp;</td><td>", s->getSize() );
	view->write( tmp.data() );
	view->write( s->getCreationDate() );
	
	view->write( "</td></tr>" );
    }
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
    {
	view->write( "<tr><td><a href=\"" );
	
	view->write( encodedURL.data() );
	view->write( "\"><cell width=100% padding=0>" );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("<i>");
	view->writeHTMLquoted ( decoded );
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("</i>");
	view->write( "</cell></a></td><td><tt>" );
	view->write( s->getAccess() );
	view->write( "</tt></td><td>" ); 
	view->write( s->getOwner() );
	view->write( "</td><td>" );
	view->write( s->getGroup() );
	view->write( "</td><td align=right>" );
	QString tmp;
        // write the size and then an empty cell to add some space
	tmp.sprintf( "%i </td><td>&nbsp;</td><td>", s->getSize() );
	view->write( tmp.data() );
	view->write( s->getCreationDate() );
	view->write( "</td></tr>" );
    }
    else if ( view->getGUI()->getViewMode() == KfmGui::SHORT_VIEW )
    {	
	// Begin Link
	view->write( "<a href=\"" );
	view->write( encodedURL.data() );
	view->write( "\">" );

	// Begin Cell
	QString tmp;
	tmp.sprintf( "<cell width=%i>", 180 );
	view->write( tmp.data() );

	// Begin Link
	// We have to duplicate this here, since KHTMLW addes a </a>
	// at this place automatically.
	view->write( "<table cellspacing=0 cellpadding=0><tr><td width=22 valign=top><a href=\"" );
	view->write( encodedURL.data() );
	view->write( "\">" );

	// Write Icon
	view->write( "<img border=0 width=16 height=16 src=\"file:" );
	view->write( KMimeType::getPixmapFileStatic( filename.data(), TRUE ) );
	view->write( "\"></a></td><td><a href=\"" );

	// Begin Link
	view->write( encodedURL.data() );
	view->write( "\">" );

	// Begin italic (if file is a link)
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("<i>");

	// Write Filename
	// view->writeHTMLquoted ( decoded );
	strcpy( buffer, decoded );
	writeWrapped( buffer, 150 );  // writeWrapped htmlQuotes itself

	// End italic
	if ( s->getAccess() && s->getAccess()[0] == 'l' )
	    view->write("</i>");
	
	// End Cell & Link
	view->write( "</a></td></tr></table></cell></a>" );
    }
}

void KFMManager::slotData( const char *_text, int _len)
{
    QString     tmp;
    const char *char_p;
    int         len;

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
	_text = tmp.data();
        _len = strlen(_text);
    }
    
    // HACK
    // Data may contain ASCII NUL, copy all data but NUL
    char_p = _text;
   
    do
    {
       len = strlen(char_p);
       if (len > _len)
          len = _len;
       _len -= len;
       pageBuffer += char_p;
       if (!bBufferPage)
       {
          view->write(char_p);
       }
       char_p += len+1;
    } 
    while ( _len > 1);
}

void KFMManager::stop()
{
    //-------- Sven's overlayed mime/app dirs start ---
    bindingDir = false;
    pass2 = false;
    if (dupList)
    {
      delete dupList;
      dupList = 0;
    }
    //-------- Sven's overlayed mime/app dirs end ---
    view->getGUI()->slotRemoveWaitingWidget( view );
    job->stop();

    if ( !bFinished )
	view->end();
}

void KFMManager::slotRedirection( const char *_url )
{
    debug("Setting job URL to %s",_url);
    jobURL = _url; // store it to a special QString.
    if (KURL(_url).isLocalFile())
    {
        // disable any change if redirection on a local file.
        // This allows to store the "/index.html" in 'url'
        // without changing the url stored here. David.
    } else
    {
        url = _url;
        view->getGUI()->setToolbarURL( _url );
        // view->getGUI()->slotSetStatusBar( _text );
    }
}

void KFMManager::slotInfo( const char *_text )
{
    view->getGUI()->slotSetStatusBar( _text );
}


void KFMManager::slotCookie( const char *_url, const char *_cookie_str )
{
    KCookieAdvice userAdvice = KCookieDunno;
    printf("KFMManager: got Cookie from %s!\n\"%s\"\n", _url, _cookie_str);

    if (!cookiejar)
    {
	printf("KFMManager: No cookiejar, ignoring cookie.\n");
	return;
    }

    KCookiePtr cookie = cookiejar->makeCookies(_url, _cookie_str);
    
    while (cookie)
    {
        KCookiePtr next_cookie = cookie->next();
        KCookieAdvice advice = cookiejar->cookieAdvice(cookie);
        if ((advice == KCookieAsk) || (advice == KCookieDunno))
        {
            // We only ask the user once, even if we get multiple
            // cookies from the same site.
            if (userAdvice == KCookieDunno)
            {
                printf("Asking user for advice for cookie from %s\n", _url);
                KCookieWin *kw = new KCookieWin( 0L, cookie);
	        userAdvice = (KCookieAdvice) kw->exec();
	        delete kw;
	        // Save the cookie config if it has changed
	        cookiejar->saveConfig( kapp->getConfig() ); 
	    }
	    advice = userAdvice;
        }
        switch(advice)
        {
        case KCookieAccept:
            printf("Accepting cookie from %s\n", _url);
            cookiejar->addCookie(cookie);
	    break;
	
	case KCookieReject:
        default:
            printf("Rejecting cookie from %s\n", _url);
            delete cookie; 
	    break;
        }
        cookie = next_cookie;
    }
}

void KFMManager::slotMimeType( const char *_type )
{

    char *typestr=0;
    const char *aType=0;
    const char *aCharset=0;
    if (_type)
    {
        typestr=new char[strlen(_type)+1];
        strcpy(typestr,_type);
	aType=strtok(typestr," ;\t\n");
	char *tmp;
	while( ( tmp = strtok( 0, " ;\t\n" ) ) )
	{
	  if ( strncmp( tmp, "charset=", 8 ) == 0 )
	    aCharset = tmp + 8;
	}    
	if ( aCharset != 0 )
	{
	    tmp = strpbrk( aCharset, " ;\t\n" );
	    if ( tmp != 0 )
	      *tmp = 0;
	}    
    }  
 
    // Recursion for special mime types which are
    // handled by KFM itself

    // GZIP
    if ( aType &&  strcmp( aType, "application/x-gzip" ) == 0L )
    {
	job->stop();
	tryURL += "#gzip:/";
	openURL( tryURL, bReload );
    }
    // TAR
    else if ( aType && strcmp( aType, "application/x-tar" ) == 0L )
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
    else if ( aType == 0L || strcmp( aType, "text/html" ) != 0L )
    {
	view->getGUI()->slotRemoveWaitingWidget( view );

	// Stop browsing. We need an application
	job->stop();

	// Do we know the mime type ?
	if ( aType )
	{
	    KMimeType *typ = KMimeType::findByName( aType );
	    // Have we registered this mime type in KDE ?
	    if ( typ && typ->run( tryURL ) ){
	        delete typestr;
		return;
             }		
	}
	
	// Ask the user what we should do
	OpenWithDlg l( klocale->translate("Open With:"), "", 0L, true );
	// debugT("OPENING DLG\n");
	if ( l.exec() )
	{
	  KMimeBind *bind = l.mimeBind();
	  if ( bind )
	  {
	    bind->runBinding( tryURL );
	    if (typestr) delete typestr;
	    return;
	  }

	  QString pattern = l.getText();
	  if ( pattern.isEmpty() )
	  {
	    if (typestr) delete typestr;
	    return;
	  }	
	  
	  QStrList list;
	  list.append( tryURL );
	  openWithOldApplication( l.getText(), list );
	}
    }
    else
    {
	// Push the old URL on the stack if we are allowed to
	if ( !url.isEmpty() && !bHistoryStackLock )
	    view->pushURLToHistory();
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
            setUpURL(); // will set "".
            view->setHTMLMode(true);
	    view->begin( url, nextXOffset, nextYOffset );
	    if ( aCharset != 0 )
	      view->setCharset(aCharset);
	    view->parse();
	}
	else
	    bBufferPage = TRUE;
    }
    if (typestr) delete typestr;
}


void KFMManager::setDefaultTextColors( const QColor& textc,const QColor& linkc,
				    const QColor& vlinkc)
{
  text_color = text_color.sprintf("#%02x%02x%02x",
				  textc.red(),textc.green(),textc.blue());

  link_color = link_color.sprintf("#%02x%02x%02x",
				  linkc.red(),linkc.green(),linkc.blue());

  vlink_color = vlink_color.sprintf("#%02x%02x%02x",
				 vlinkc.red(),vlinkc.green(),vlinkc.blue());
}

void KFMManager::setDefaultBGColor( const QColor& bgcolor )
{
  bg_color = bg_color.sprintf("#%02x%02x%02x",
			      bgcolor.red(),bgcolor.green(),bgcolor.blue());
}

void KFMManager::slotFinished()
{
    //-------- Sven's overlayed mime/app dirs start ---
    if (bindingDir && !pass2) // did we read first part (local dir)?
    {
      // We just read local part of our two dirs; now read the global
      // part. We must stop job to clear our list, since we need it
      // for checking duplicates in writeEntry().

      pass2 = true; // we will be called again

      //set up dir to read. We just read local dir: prepare for global:
      QString tryURL(job->getURL()); //deep copy, is that right?

      // debug ("DIROVERLAY:pass 2, old url = %s", tryURL.data());
      
      if (tryURL.contains(LOCALMIME))
      {
	tryURL.remove(5, strlen(LOCALMIME)); // 5 is for "file:"
	tryURL.insert(5, GLOBALMIME);
        // debug ("DIROVERLAY:pass 2, new url = %s", tryURL.data());
      }

      else if (tryURL.contains(LOCALAPPS))
      {
	tryURL.remove(5, strlen(LOCALAPPS));
	tryURL.insert(5, GLOBALAPPS);
      }
      else  // Debug fallback
      {
	debug ("DIROVERLAY:BAD, job->url doesn't contain LOCAL part: %s (pass2)",
	       tryURL.data());
	// cleanup:
	pass2 = false;
	bindingDir = false;

	if ( files.count() == 0 )
	  writeBeginning();

	writeEnd();
	return;
      }
      // Sven's bugfix: it is possible that directory exists only in
      // local variant; test it for existance befor browsing:
      if (access (&(tryURL.data())[5], F_OK) == 0)
	job->browse( tryURL, false, view->getGUI()->isViewHTML(), tryURL, 0, 0 );
      else
	slotFinished(); // just call again this to write end (flag is set)
      return; // don't finish yet;
    }
    // when we get here it means one of:
    //  - we are not reading binding dirs at all
    //  - we have read local and globals
    // in any case clear both flags and delete dupList if not 0:
    pass2 = false;
    bindingDir = false;
    if (dupList)
    {
      delete dupList;
      dupList = 0;
    }

    // and proceed with normal stuff.
    //-------- Sven's overlayed mime/app dirs end ---
    bFinished = TRUE;

    // We retrieved a ready to go HTML page ?
    if ( bHTML )
    {
	KURL u( url );
	// Did we buffer the complete HTML stuff ?
	if ( bBufferPage )
	{
	    // Display it now
            setUpURL();
            view->setHTMLMode(true);
	    view->begin( url, nextXOffset, nextYOffset );
	    view->write( pageBuffer );
	    view->parse();
	}
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

    writeEnd();
    // view->getGUI()->slotRemoveWaitingWidget( view );
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
	// Run the action 'txt' on every single file
	KMimeBind::runBinding( s, txt );    
    }
}

void KFMManager::openPopupMenu( QStrList &_urls, const QPoint & _point, bool _current_dir )
{
    // please note that this code is strongly related to
    // root.cpp: void KRootWidget::openPopupMenu( ... )


    bool bHttp = true;

    // Check wether all URLs are correct
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	KURL u( s );
	if ( u.isMalformed() )
	{
	    QString tmp;
	    tmp << klocale->translate("Malformed URL\n") << s;
	    QMessageBox::warning( 0, klocale->translate("KFM Error"), tmp );
	    return;
	}
        if (strcmp(u.protocol(),"http")) bHttp = false; // not HTTP
    }
    
    popupMenu->clear();
    // store the mouse position. (Matthias)
    popupMenuPosition = QCursor::pos();       
    //---------- Sven --------------
    // check if menubar is hidden and if yes add "Show Menubar"
    if (view->getGUI()->isMenuBarHidden())
    {
      
      popupMenu->insertItem(klocale->getAlias(ID_STRING_SHOW_MENUBAR),
                                view->getGUI(), SLOT(slotShowMenubar()));
      popupMenu->insertSeparator();
    }
    //------------------------------
    bool isdir = (KIOServer::isDir( _urls ) == 1);
    KURL viewURL ( view->getActiveView()->getURL() );
    
    if ( KIOServer::isTrash( _urls ) )
    {
	int id;

        /* Commented out. Left click does it. Why have it on right click menu ?. David.
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_CD), 
				    view, SLOT( slotPopupCd() ) );
        */
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_NEW_VIEW), 
				    view, SLOT( slotPopupNewView() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_TRASH), 
				    view, SLOT( slotPopupEmptyTrashBin() ) );
    } 
    else if ( isdir )
    {
	int id;

        popupMenu->insertItem( klocale->translate("&New"), menuNew );
        popupMenu->insertSeparator();

        if ( _current_dir ) {
          id = popupMenu->insertItem(klocale->getAlias(ID_STRING_UP), view, SLOT( slotUp() ), 100 );

          if ( !view->hasUpHistory() )
            popupMenu->setItemEnabled( id, false );
          id = popupMenu->insertItem(klocale->getAlias(ID_STRING_BACK), view, SLOT( slotBack() ), 101 );

          if ( !view->hasBackHistory() )
            popupMenu->setItemEnabled( id, false );
          id = popupMenu->insertItem(klocale->getAlias(ID_STRING_FORWARD), view, SLOT( slotForward() ), 102 );

          if ( !view->hasForwardHistory() )
            popupMenu->setItemEnabled( id, false );
	
          popupMenu->insertSeparator();  
        }

        /* Commented out. Left click does it. Why have it on right click menu ?. David.
        if (!_current_dir) {
            id = popupMenu->insertItem( klocale->getAlias(ID_STRING_CD), view, 
                                        SLOT( slotPopupCd() ) );
        }
        */
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_NEW_VIEW), 
				    view, SLOT( slotPopupNewView() ) );
	popupMenu->insertSeparator();    
	if ( KIOServer::supports( _urls, KIO_Read ) )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_COPY ), 
					view, SLOT( slotPopupCopy() ) );
	if ( KIOServer::supports( _urls, KIO_Write ) && KfmView::clipboard->count() != 0 )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_PASTE ), 
					view, SLOT( slotPopupPaste() ) );
	if ( KIOServer::supports( _urls, KIO_Move ) &&
             !KIOServer::isTrash ( viewURL.directory() ) && !_current_dir )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_MOVE_TO_TRASH ),  
					view, SLOT( slotPopupTrash() ) );
	if ( KIOServer::supports( _urls, KIO_Delete ) && !_current_dir )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_DELETE ),  
					view, SLOT( slotPopupDelete() ) );
    }
    else
    {
	int id;
        if (bHttp) {
            /* Should be for http URLs (HTML pages) only ... */
            id = popupMenu->insertItem( klocale->getAlias(ID_STRING_NEW_VIEW), 
                                        view, SLOT( slotPopupNewView() ) );
        }
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_OPEN_WITH), 
				    view, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	if ( KIOServer::supports( _urls, KIO_Read ) )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_COPY ), 
					view, SLOT( slotPopupCopy() ) );
	if ( KIOServer::supports( _urls, KIO_Move )  &&
             !KIOServer::isTrash ( viewURL.directory() ) && !_current_dir )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_MOVE_TO_TRASH ),  
					view, SLOT( slotPopupTrash() ) );
	if ( KIOServer::supports( _urls, KIO_Delete ) && !_current_dir )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_DELETE ),  
					view, SLOT( slotPopupDelete() ) );
    }

    popupMenu->insertItem( klocale->getAlias(ID_STRING_ADD_TO_BOOMARKS),
			   view, SLOT( slotPopupBookmarks() ) );

    menuNew->setPopupFiles( _urls );
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
	    bindings2.clear(); 
	    pixlist2.clear();
	    KMimeType::getBindings( bindings2, pixlist2, s, isdir );
	    char *b;
	    QPixmap *p = pixlist.first();
	    bindings3.clear();
	    pixlist3.clear();
	    // Look thru all bindings we have so far
	    for ( b = bindings.first(); b != 0L; b = bindings.next() )
	    {
		// Does the binding match this file, too
		if ( bindings2.find( b ) != -1  )
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
	if ( bindings.isEmpty() )
	    break;
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
        //--------------------------------------------------------------------
        // Sven's changes: if this is shown view add entry Save settings..

        if (_current_dir && pkfm->isURLPropesEnabled ())
        {
          popupMenu->insertSeparator();
          popupMenu->insertItem(klocale->getAlias(ID_STRING_SAVE_URL_PROPS),
                                view, SLOT(slotSaveLocalProperties()));
        }
        //--------------------------------------------------------------------
	popupMenu->insertSeparator();
        popupMenu->insertItem( klocale->getAlias(ID_STRING_PROP), view, SLOT( slotPopupProperties() ) );
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
    // -- sven: only copy from "file:/tmp/kmail*" start ---
    {
      char *s;
      bool attachment = false;
      for ( s = _zone->getURLList().first(); s != 0L; s = _zone->getURLList().next() )
	if (strstr (s, "file:/tmp/kmail") != 0 )
	  attachment = true;
      if (attachment)
      {
	if (KIOServer::supports( _dest, KIO_Write ) && !_nestedURLs )
	{
	  popupMenu->insertItem( klocale->getAlias( ID_STRING_COPY ),
				 this, SLOT( slotDropCopy() ) );
	  popupMenu->popup(*_p);
	}
	else
	  warning(klocale->translate("ERROR: Can not accept drop"));
	return;
      }
    }
    // -- sven: only copy from "file:/tmp/kmail*" end ---
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
			       klocale->translate("Dont know what to do.") );
	return;
    }

    // Show the popup menu
    popupMenu->popup( *_p );
}

void KFMManager::slotDropCopy()
{
    KIOJob * job = new KIOJob;
    job->copy( dropZone->getURLList(), dropDestination.data() );
    if(KRootWidget::pKRootWidget)
      KRootWidget::pKRootWidget->unselectAllIcons();
}

void KFMManager::slotDropMove()
{
    KIOJob * job = new KIOJob;
    job->move( dropZone->getURLList(), dropDestination.data() );
    if(KRootWidget::pKRootWidget)
      KRootWidget::pKRootWidget->unselectAllIcons();
}

void KFMManager::slotDropLink()
{
    KIOJob * job = new KIOJob;
    job->link( dropZone->getURLList(), dropDestination.data() );
    if(KRootWidget::pKRootWidget)
      KRootWidget::pKRootWidget->unselectAllIcons();
}

QString KFMManager::getVisualSchnauzerIconTag( const char *_url )
{
    // directories URL
    KURL u( url );
    // URL of the file we need an icon for
    KURL u2( _url );
    
    // Look for .xvpics directory on local hard disk only.
    if ( u.isLocalFile() )
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
		    QString result;
                    KURL::encodeURL(xvfile); // in case of "'" in the name, for instance.
		    result.sprintf("<img border=0 src=\"file:%s\">", xvfile.data() );
		    return result;
		}
	    }
	}
    
	// At this time the icon protocol works for local files only.
	KMimeType *mime = KMimeType::getMagicMimeType( _url );
	if ( strcmp( mime->getMimeType(), "image/jpeg" ) == 0 ||
	     strcmp( mime->getMimeType(), "image/gif" ) == 0 ||
	     strcmp( mime->getMimeType(), "image/bmp" ) == 0 ||  
                            // Added bmp, because it seems to work. David.
	     strcmp( mime->getMimeType(), "image/x-xpm" ) == 0 )
	{
	    QString result;
            QString path(u2.path());
            KURL::encodeURL(path); // in case of "'" in the name, for instance.
	    result.sprintf("<img border=0 src=\"icon:%s\">", path.data() );
	    return result;
	}
    }
    
    KMimeType *mime = KMimeType::getMagicMimeType( _url );
    QString result;
    QString path( mime->getPixmapFile( _url ) );
    KURL::encodeURL(path); // in case of "'" in the name, for instance.
    result.sprintf("<img border=0 src=\"file:%s\">", path.data() );
    return result;
}


#include "kfmman.moc"
