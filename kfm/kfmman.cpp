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

KAbstractManager::KAbstractManager( KfmView *_v )
{
    url  = "";
    view = _v;
    maxLabelWidth = 80;
    labelFontMetrics = new QFontMetrics(view->defaultFont());
    popupMenu = new QPopupMenu();
    popupMenu->installEventFilter( this );
}

KAbstractManager::~KAbstractManager()
{
    if (labelFontMetrics) delete labelFontMetrics;
}

bool KAbstractManager::isBindingHardcoded( const char *_txt )
{
    if ( strcmp( "Cd", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "New View", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Copy", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Delete", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Move to Trash", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Paste", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Open With", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Cut", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Move", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Properties", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Link", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Open", _txt ) == 0 )
	return TRUE;
    if ( strcmp( "Empty Trash Bin", _txt ) == 0 )
	return TRUE;

    return FALSE;
}
  
void KAbstractManager::writeWrapped( char *_str )
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

bool KAbstractManager::eventFilter( QObject *ob, QEvent *ev )
{
    // let the popup stay on the screen if the user
    // didn't move the mouse (Matthias)
    if (ob == popupMenu && ev->type() == Event_MouseButtonRelease
	&& QCursor::pos() == popupMenuPosition )
    {
	popupMenuPosition = QPoint(-10,-10);
	return TRUE;
    }
    return FALSE;
}
    
KFileManager::KFileManager( KfmView * _v ) : KAbstractManager( _v )
{
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );
}

void KFileManager::dropPopupMenu( KDNDDropZone *_zone, const char *_dest, const QPoint *_p )
{
    dropDestination = _dest;
    dropDestination.detach();
    
    dropZone = _zone;
    
    debugT(" Drop with destination %s\n", _dest );
    
    // Perhaps an executable ?
    if ( !KIOServer::isDir( _dest ) )
    {
	KMimeType::runBinding( _dest, "Open", &(_zone->getURLList()) );
	return;
    }
    
    popupMenu->clear();
    // store the mouse position. (Matthias)
    popupMenuPosition = QCursor::pos();         

    int id = 1;
    // Ask wether we can read from the dropped URL.
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( _dest, KIO_Write ) )
	id = popupMenu->insertItem( "Copy", this, SLOT( slotDropCopy() ) );
    // Ask wether we can read from the URL and delete it afterwards
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( _zone->getURLList(), KIO_Delete ) &&
	 KIOServer::supports( _dest, KIO_Write ) )
	id = popupMenu->insertItem( "Move", this, SLOT( slotDropMove() ) );
    // We can link everything on the local file system
    if ( KIOServer::supports( _dest, KIO_Link ) )
	id = popupMenu->insertItem( "Link", this, SLOT( slotDropLink() ) );
    if ( id == -1 )
    {
	debugT("ERROR: Can not accept drop\n");
	return;
    }
    
    popupMenu->popup( *_p );
}

void KFileManager::slotDropCopy()
{
    KIOJob * job = new KIOJob;
    debugT("COPY count=%i\n",dropZone->getURLList().count());
    
    job->copy( dropZone->getURLList(), dropDestination.data() );
}

void KFileManager::slotDropMove()
{
    KIOJob * job = new KIOJob;
    job->move( dropZone->getURLList(), dropDestination.data() );
}

void KFileManager::slotDropLink()
{
    KIOJob * job = new KIOJob;
    job->link( dropZone->getURLList(), dropDestination.data() );
}

void KFileManager::openDirHTML( const char *_filename, bool _refresh )
{
    // Open HTML file like .kde.html or index.html or just a usual HTML file
    FILE *f = fopen( _filename, "rb" );
    if ( f == 0 )
    {
	debugT("ERROR: Could not read '%s'\n",_filename );
	return;
    }
    
    KURL u( url.data() );

    QString d;
    if ( !KIOServer::isDir( url.data() ) )
	d = u.directoryURL();    
    else
	d = url.data();
    d.detach();
    if ( d.right(1) != "/" )
	d += "/";

    debugT("Hi from openDir '%s' '%s'\n", url.data(),d.data());
    
    if ( _refresh )
	view->begin( d.data(), view->xOffset(), view->yOffset() );
    else
	view->begin( d.data() );
    debugT("begun...\n");
    
    view->parse();
    debugT("Parsed\n");

    // Are we not displaying a .kde.html or index.html file ? => Makes things easier
    if ( !KIOServer::isDir( url.data() ) )
    {
	debugT("Simle\n");
	char mem[ 1024 ];
	while ( !feof( f ) )
	{
	    int n = fread( mem, 1, 1023, f );
	    if ( n > 0 )
	    {
		mem[n] = 0;
		view->write( mem );
	    }
	}
	fclose( f );
    
	view->end();
	return;
    }
    debugT("Done\n");
    
    // Read the HTML code into memory
    char *mem = (char*) malloc( 30000 );
    int size = 0;
    while ( !feof( f ) )
    {
	int n = fread( mem + size, 1, 29999, f );
	size += n;
	if ( n == 29999 )
	{
	    char *p = (char*) malloc( size + 30000 );
	    memcpy( p, mem, size );
	    free( mem );
	    mem = p;
	}
    }
    mem[ size ] = 0;
    
    fclose( f );

    debugT("Read\n");
    
    // Read the directory listing
    DIR *dp;
    struct dirent *ep;
    dp = opendir( u.path() );
    if ( dp == NULL )
    {
	view->write( "<h1>Could not access directory</h1></body></html>" );
	view->end();
	return;
    }

    QStrList strlist( TRUE );    
    // Loop thru all directory entries
    while ( ( ep = readdir( dp ) ) != 0L )
	strlist.inSort( ep->d_name );
    (void) closedir( dp );
	
    // Stores all files we already displayed.
    // This is used for filter option "Rest"
    QList<QString> displayedFiles;
    displayedFiles.setAutoDelete( TRUE );
    // Indicates wether filter option "Rest" is currently selected.
    bool rest;

    debugT("Looping\n");
    
    char *p = mem;
    char *old = mem;

    
    // Search All <files...> tags
    while ( ( p = strstr( p, "<files " ) ) != 0L )
    {
	rest = FALSE;
	
	*p = 0;
	// Write everything up to the start of <files...
	view->write( old );
	// Make the <files...> tag a 0 terminated string
	char *begin = p + 1;
	p = strchr( p + 1, '>' );
	if ( p == 0 )
	    p = mem + strlen( mem );
	*p = 0;
	
	{
	    // RegExpr. for wildcard pattern
	    QRegExp re;
	    // Wildcard pattern
	    QString name;
	    // Filter flags
	    int filter = 0;
	    
	    // Delete the "<files " part of the string
	    QString str = (const char*) begin + 6;
	    // Parse the arguments of the <files...> tag
	    StringTokenizer st( str, " " );
	    // Look at each argument
	    while ( st.hasMoreTokens() )
	    {
		const char* token = st.nextToken();
		if (strncasecmp( token, "name=", 5 ) == 0)
		{
		    name = token + 5;
		    name.detach();
		    // A regular expression
		    if ( name.data()[0] == '/' )
		    {
			re = name.data() + 1;
			re.setWildcard( FALSE );
		    }
		    // a wildcard pattern
		    else
		    {
			re = name.data();
			re.setWildcard( TRUE );
		    }
		}
		else if (strncasecmp( token, "filter=", 7 ) == 0)
		{
		    QString s = (const char*) token + 7;
		    StringTokenizer st2( s, "|" );
		    while ( st2.hasMoreTokens() )
		    {
			const char* f = st2.nextToken();
			if ( strcasecmp( f, "Dirs" ) == 0 )
			    filter |= QDir::Dirs;
			else if ( strcasecmp( f, "Files" ) == 0 )
			    filter |= QDir::Files;
			else if ( strcasecmp( f, "Drives" ) == 0 )
			    filter |= QDir::Drives;
			else if ( strcasecmp( f, "Executable" ) == 0 )
			    filter |= QDir::Executable;
			else if ( strcasecmp( f, "Hidden" ) == 0 )
			    filter |= QDir::Hidden;
			else if ( strcasecmp( f, "Rest" ) == 0 )
			{
			    rest = TRUE;
			    filter = 0;
			}
			else
			    debugT("Unknown filter option %s\n",f);
		    }
		}
	    }

	    // The name pattern. Default is no pattern
	    const char *pname = 0L;
	    // Do we have a pattern ?
	    if ( !name.isNull() )
		if ( name.data()[0] != 0 )
		    pname = name.data();

	    char buff [ 1024 ];
	    
	    // Traverse all files
	    char* fn;
	    for ( fn = strlist.first(); fn != 0L; fn = strlist.next() )
	    {
		if ( strcmp( fn, "." ) != 0 )
		{
		    bool ok = FALSE;
		    bool ok2 = TRUE;
		    // Is the filter option rest active ?
		    if ( rest )
		    {
			QString *s;
			// Have we displayed this file already?
			for ( s = displayedFiles.first(); s != 0L; s = displayedFiles.next() )
			    if ( strcmp( s->data(), fn ) == 0 )
				ok2 = FALSE;
		    }
		 
		    if ( strcmp( fn, ".." ) == 0 )
		    {
			KURL u2( u.path() );
			u2.cd("..");
			u2.cleanURL();
			strcpy( buff, "file:" );
			strcat( buff, u2.path() );
		    }
		    else
		    {
			strcpy( buff, "file:" );
			strcat( buff, u.path() );
			if ( buff[ strlen( buff ) - 1 ] != '/' )
			    strcat( buff, "/" );
			strcat( buff, fn );
		    }
		    
		    struct stat sbuff;
		    stat( buff+5, &sbuff );

		    struct stat lbuff;
		    lstat( buff+5, &lbuff );

		    // Test wether the file matches our filters
		    if ( ( filter & QDir::Dirs ) == QDir::Dirs && S_ISDIR( sbuff.st_mode ) )
			ok = TRUE;
		    if ( ( filter & QDir::Files ) == QDir::Files && S_ISREG( sbuff.st_mode ) )
			ok = TRUE;
		    if ( ( filter & QDir::Drives ) == QDir::Drives && 
			 ( S_ISCHR( sbuff.st_mode ) || S_ISBLK( sbuff.st_mode ) ) )
			ok = TRUE;
		    if ( ( filter & QDir::Executable ) == QDir::Executable &&
			 ( sbuff.st_mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) != 0 ) &&
			 !S_ISDIR( sbuff.st_mode ) )
			ok = TRUE;
		    if ( ( filter & QDir::Hidden ) == QDir::Hidden && fn[0] == '.' &&
			 strcmp( fn, ".." ) != 0 )
			ok = TRUE;
		    else if ( filter == 0 )
			ok = TRUE;
		    
		    if ( pname != 0L )
		    {
			if ( re.match( fn ) == -1 )
			    ok = FALSE;
		    }
		    
		    if ( ok && ok2 )
		    {
			// Remember all files we already displayed
			if ( !rest )
			{
			    QString *s = new QString( fn );
			    displayedFiles.append( s );
			}
			
			view->write( "<cell><a href=\"" );
			view->write( buff );
			view->write( "\"><center><img src=\"" ); 
			view->write( KMimeType::findType( buff )->getPixmapFile( buff ) );
			view->write( "\"><br>" );
			view->write( fn );
			view->write( "</center><br></a></cell>" );
		    }
		}
	    }
	}

	old = ++p;	
    }
    debugT("Writing stuff\n");
    view->write( old );
    debugT("Wrote Stuff\n");
    free( mem );
    debugT("Freeing\n");
    view->end();
    debugT("Freed\n");
}

bool KFileManager::openURL( const char *_url, bool _refresh )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return FALSE;

    debugT("Changing to %s\n", _url );

    // Is it a directory? If not ...
    if ( !KIOServer::isDir( _url ) )
    {
	// Test wether it is a HTML file
	FILE *f = fopen( u.path(), "rb" );
	if ( f != 0L )
	{
	    char buffer[ 1024 ];
	    int n = fread( buffer, 1, 1023, f );
	    fclose( f );
	    buffer[n] = 0;
	    if ( strstr( buffer, "<html>" ) != 0 || strstr( buffer, "<HTML>" ) != 0 ||
		 strstr( buffer, "<HEAD>" ) != 0 || strstr( buffer, "<head>" ) != 0 ||
		 strstr( buffer, "<BODY>" ) != 0 || strstr( buffer, "<body>" ) != 0 )
	    {
		// url = u.directoryURL();
		url = _url;
		url.detach();
		debugT("Opening as HTML file '%s'\n", u.path());
		// Are we belonging to the root widget ?
		if ( view == 0L )
		{
		    KfmGui *m = new KfmGui( 0L, 0L, _url );
		    m->show();
		}
		else
		    openDirHTML( u.path(), _refresh );
		return TRUE;
	    }
	}
	
	// Did the user click on a *.kdelnk file ?
	QString tmp = _url;
	if ( tmp.length() > 7 && tmp.right(7) == ".kdelnk" )
	{
	    // KMimeType::runKDEFile( _dir );
	    debugT("#################### KFileManager::openURL\n");
	    KMimeType::runBinding( _url );
	    return FALSE;
	}
	// Are we entering a tar file ?
	else if ( strstr( _url, ".tar" ) != 0 || strstr( _url, ".tgz") != 0 || strstr( _url, ".tar.gz" ) != 0 )
	{	
	    QString t( "tar:" );
	    t += u.path();

	    // Are we belonging to the root widget ?
	    if ( view == 0L )
	    {
		KfmGui *m = new KfmGui( 0L, 0L, t.data() );
		m->show();
	    }
	    else
	    {
		view->getTarManager()->openURL( t.data() );
		view->setActiveManager( view->getTarManager() );
	    }
	    return FALSE;
	}
	else
	{
	    struct stat buff;
	    if ( 0 > stat( u.path(), &buff ) )
	    {
		debugT("ERROR: Could not access file %s\n",_url );
		return FALSE;
	    }
	    
	    // Executable ?
	    if ( ( buff.st_mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) != 0 ) )
	    {
		char buffer[ 1024 ];
		sprintf( buffer, "%s &",u.path() );
		system( buffer );
	    }
	    // Start the appropriate binding.
	    else
	    {
		KMimeType::runBinding( _url );
		return FALSE;
	    }   
	}
	debugT("ERROR: Could not change to %s\n",_url);
	return FALSE;
    }

    // Are we used by the root widget perhaps ?
    if ( view == 0L )
    {
	KfmGui *m = new KfmGui( 0L, 0L, _url );
	m->show();
	return FALSE;
    }

    url = _url;
    url.detach();

    // Open the directory
    DIR *dp;
    struct dirent *ep;
    QString pathOfU = u.path();
    KURL::decodeURL(pathOfU);
    dp = opendir( pathOfU.data() );
    if ( dp == 0L )
    {
        view->begin( _url );
        view->parse();
	view->write( "<h1>Could not access directory</h1></body></html>" );
	view->end();
	return TRUE;
    }
    
    // Name of the file that holds HTML data for this directory.
    // This String might be empty
    QString html;
    
    // List of all files & directories
    QStrList strlist( TRUE );
    
    // Loop thru all directory entries
    while ( ( ep = readdir( dp ) ) != 0L )
    {
    	QString name(ep->d_name);
	// Dont allow '..' and '.' files
	if ( name != ".." && name != "." )
	{
	    KURL::encodeURL(name);
	    strlist.inSort( name.data() );
	    if ( name == ".kde.html" )
		html = ".kde.html";
	    else if ( name.lower() == "index.html" && html.isNull() )
		html = name;
	}
    }
    closedir( dp );

    strlist.insert( 0, ".." );
    
    // Is there dome HTML to show ?
    if ( !html.isNull() && view->getGUI()->isViewHTML() )
    {
	QString h = u.path();
	if ( h.right(1) != "/" )
	    h += "/";
	h += html.data();
	openDirHTML( h.data(), _refresh );
	return TRUE;
    }

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
	    bg_image = kapp->kdedir();
	    bg_image += "/lib/pics/wallpapers/";
	    bg_image += tmp2.data();
	}
    
    QString d = u.path();
    if ( d.right( 1 ) != "/" )
	d += "/.directory";
    else
	d += ".directory";
    
    debugT("Trying .directory\n");
    
    QFile f( d.data() );
    if ( f.open( IO_ReadOnly ) )
    {
	debugT("Opened .directory\n");
	
	QTextStream pstream( &f );
	KConfig config( &pstream );
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
		bg_image = kapp->kdedir();
		bg_image += "/lib/pics/wallpapers/";
		bg_image += tmp.data();
	    }
    }

    debugT("Back again\n");
    
    // Open the directory the usual way.    
    char str[ 1024 ];

    QString d2 = _url;
    debugT("d2 = '%s'\n",d2.data());
    
    d2.detach();
    if ( d2.right( 1 ) != "/" )
	d2 += "/";

    debugT("B2\n");
    
    // If we only want to refresh dont change the scrollbars if possible.
    if ( _refresh )
    {
	debugT("!!!!!!!!!!!!!!!! REFRESH !!!!!!!!!!!!!!!!!\n");
	view->begin( d2.data(), view->xOffset(), view->yOffset() );
    }
    else
    {
	debugT("X\n");
	view->begin( d2.data() );
	debugT("Y\n");
    }
    view->parse();

    debugT("1\n");
    
    view->write( "<html><title>" );
    view->write( _url );
    view->write( "</title><body" );

    debugT("2\n");

    if ( !text_color.isNull() )
    {
	debugT("3\n");
	view->write(" text=" );
	view->write( text_color.data() );
    }
    debugT("4\n");
    if ( !link_color.isNull() )
    {
	debugT("5\n");
	view->write(" link=" );
	view->write( link_color.data() );
    }
    debugT("6\n");
    if ( !bg_image.isNull() )
    {
	debugT("7\n");
	KURL u2( u, bg_image.data() );
	view->write(" background=\"" );
	QString t = u2.url();
	view->write( t.data() );
	view->write( "\"" );
    }
    else if ( !bg_color.isNull() )
    {
	debugT("8\n");
	view->write(" bgcolor=" );
	view->write( bg_color.data() );
    }
    debugT("9\n");
    view->write( ">" );

    debugT("C\n");
    
    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
    { }
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	view->write( "<table width=100%>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	view->write( "<table width=100%>" );
    
    debugT("Loop\n");

    bool visualSchnauzer = FALSE;
    QString xv = u.path();
    if ( view->getGUI()->isVisualSchnauzer() )
    {
	if ( xv.right(1) != "/" )
	    xv += "/.xvpics";
	else
	    xv += ".xvpics";
	dp = opendir( xv.data() );
	if ( dp != NULL )
	{
	    visualSchnauzer = TRUE;
	    closedir( dp );
	}
	else
	{
	    if ( ::mkdir( xv.data(), S_IRWXU ) != -1 )
		visualSchnauzer = TRUE;
	    else
		QMessageBox::message( "KFM Visual Schnauzer", "Could not create\n" + xv );
	}
    }
    xv += "/";

    char buffer[ 1024 ];    
    char* cfn;
    QString sfn;
    for ( cfn = strlist.first(); cfn != 0L; cfn = strlist.next() )
    {
        sfn = cfn; // make a QString copy of cfn
	KURL::decodeURL(sfn);
	// Dont display the file "."
        if ( sfn !=  "."  )
	{
	  if ( sfn[0] == '.' && !view->getGUI()->isShowDot() && sfn != ".." )
	      continue;
	    if ( sfn == ".."  )
	    {
	        KURL u2( _url );
		u2.cd("..");
		u2.cleanURL();
		strcpy( str, "file:" );
		strcat( str, u2.path() );
	    }
	    else
	    {
		strcpy( str, _url );
		if ( str[ strlen( str ) - 1 ] != '/' )
		    strcat( str, "/" );
		strcat( str, sfn.data() );
	    }
	    
	    // Select the view mode.
	    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
	    {
		view->write( "<cell><a href=\"" ); 
		view->write( str );
		view->write( "\"><center><img src=\"" ); 
	
		if ( view->getGUI()->isVisualSchnauzer() && visualSchnauzer )
                {
		    // Assume XV pic is not available
		    bool is_avail = FALSE;
		    // Assume that the xv pic has size 0
		    bool is_null = TRUE;
		    
		    // Time of the image
		    struct stat buff;
		    lstat( str+5, &buff );
		    time_t t1 = buff.st_mtime;
		    if ( buff.st_size != 0 )
			is_null = FALSE;
		    // Get the times of the xv pic
		    QString xvfile = xv.data();
		    xvfile += sfn;
		    // Is the XV pic available ?
		    if ( lstat( xvfile, &buff ) == 0 )
		    {
			time_t t2 = buff.st_mtime;
			// Is it outdated ?
			if ( t1 <= t2 )
			    is_avail = TRUE;
		    }
		    
		    // Create the pic if it does not exist
		    if ( !is_avail )
		    {
			QPixmap pixmap;
			pixmap.load( str + 5 );
			if ( !pixmap.isNull() )
			{
			    write_xv_file( xvfile, pixmap );
			    is_avail = TRUE;
			}
		    }
		    
		    // Test wether it is really an image
		    if ( is_avail && !is_null )
		    {
			QFile f( xvfile );
			if ( f.open( IO_ReadOnly ) )
			{
			    char str4[ 1024 ];
			    f.readLine( str4, 1024 );
			    if ( strncmp( "P7 332", str4, 6 ) != 0 )
				is_null = TRUE;
			    // Slip line
			    f.readLine( str4, 1024 );
			    f.readLine( str4, 1024 );
			    if ( strncmp( "#BUILTIN:UNKNOWN", str4, 16 ) == 0 )
				is_null = TRUE;
			    f.close();
			}
			else
			    is_null = TRUE;
		    }
		    
		    if ( is_avail && !is_null )
		    {
			view->write( xvfile );
			view->write( "\"><br>" );
		    }
		    else
		    {
			view->write( KMimeType::findType( str )->getPixmapFile( str ) );
			view->write( "\"><br>" );
		    }   
		}
		else
                {    
		  view->write( KMimeType::findType( str )->getPixmapFile( str ) );
		  view->write( "\"><br>" );
		}

		// Delete a trailing .kdelnk in the filename
		if ( sfn.length() > 7 && sfn.right(7) == ".kdelnk" )
		  {
		    strcpy(buffer, sfn.left( sfn.length() - 7).data());
		    writeWrapped(buffer);
		    // view->write( buffer );
		}
		else
		{
		    strcpy( buffer, sfn.data() );
		    writeWrapped( buffer );
		    //view->write( fn );
		}
		
		view->write( "</center><br></a></cell>" );
	    }
	    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW ||
		      view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	    {
		struct stat buff;
		stat( str+5, &buff );
		struct stat lbuff;
		lstat( str+5, &lbuff );
		struct tm *t = localtime( &lbuff.st_mtime );
		struct passwd * user = getpwuid( buff.st_uid );
		struct group * grp = getgrgid( buff.st_gid );

		view->write( "<tr><td><a href=\"" );
		view->write( str );
		view->write( "\">" );
		if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
		{
		    view->write( "<img width=16 height=16 src=\"" ); 
		    view->write( KMimeType::findType( str )->getPixmapFile( str ) );
		    view->write( "\"></td><td>" );
		}
		// Delete a trailing .kdelnk in the filename
		if ( sfn.length() > 7 && sfn.right(7) == ".kdelnk" )
		{
		    strcpy( buffer, sfn.left( sfn.length() - 7).data() );
		    view->write( buffer );
		}
		else 
		  view->write( sfn.data() );
		
		view->write( "</td><td><tt>" );

		if ( S_ISLNK( lbuff.st_mode ) )
		    buffer[0] = 'l';		
		else if ( S_ISDIR( buff.st_mode ) )
		    buffer[0] = 'd';
		else
		    buffer[0] = '-';

                char uxbit,gxbit,oxbit;
 
                if ( (buff.st_mode & (S_IXUSR|S_ISUID)) == (S_IXUSR|S_ISUID) )
                        uxbit = 's';
                else if ( (buff.st_mode & (S_IXUSR|S_ISUID)) == S_ISUID )
                        uxbit = 'S';
                else if ( (buff.st_mode & (S_IXUSR|S_ISUID)) == S_IXUSR )
                        uxbit = 'x';
                else
                        uxbit = '-';
 
                if ( (buff.st_mode & (S_IXGRP|S_ISGID)) == (S_IXGRP|S_ISGID) )
                        gxbit = 's';
                else if ( (buff.st_mode & (S_IXGRP|S_ISGID)) == S_ISGID )
                        gxbit = 'S';
                else if ( (buff.st_mode & (S_IXGRP|S_ISGID)) == S_IXGRP )
                        gxbit = 'x';
                else
                        gxbit = '-';
 
                if ( (buff.st_mode & (S_IXOTH|S_ISVTX)) == (S_IXOTH|S_ISVTX) )
                        oxbit = 't';
                else if ( (buff.st_mode & (S_IXOTH|S_ISVTX)) == S_ISVTX )
                        oxbit = 'T';
                else if ( (buff.st_mode & (S_IXOTH|S_ISVTX)) == S_IXOTH )
                        oxbit = 'x';
                else
                        oxbit = '-';
    
		sprintf( buffer + 1,
			 "%c%c%c%c%c%c%c%c%c</tt></td><td>%8s</td><td>%8s</td><td align=right>%li</td><td>%02i:%02i</td><td>%02i.%02i.%02i</td><td>",
			 ((( buff.st_mode & S_IRUSR ) == S_IRUSR ) ? 'r' : '-' ),
			 ((( buff.st_mode & S_IWUSR ) == S_IWUSR ) ? 'w' : '-' ),
			 uxbit,
			 ((( buff.st_mode & S_IRGRP ) == S_IRGRP ) ? 'r' : '-' ),
			 ((( buff.st_mode & S_IWGRP ) == S_IWGRP ) ? 'w' : '-' ),
			 gxbit,
			 ((( buff.st_mode & S_IROTH ) == S_IROTH ) ? 'r' : '-' ),
			 ((( buff.st_mode & S_IWOTH ) == S_IWOTH ) ? 'w' : '-' ),
			 oxbit,
			 (( user != 0L ) ? user->pw_name : "???" ),
			 (( grp != 0L ) ? grp->gr_name : "???" ),
			 buff.st_size, t->tm_hour,t->tm_min,t->tm_mday,t->tm_mon + 1,t->tm_year );
		view->write( buffer );
		view->write( "</a></td></tr>" );
	    }
	}
    }

    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
    	view->write( "</body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	view->write( "</table></body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	view->write( "</table></body></html>" );

    view->end();
    
    return TRUE;
}

void KFileManager::slotPopupActivated( int _id )
{
    if ( popupMenu->text( _id ) == 0)
	return;

    QString txt = popupMenu->text( _id );
    
    if ( isBindingHardcoded( txt ) )
	return;
    
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	debugT("$$$$$$$$$$ Doing it for '%s'\n",s);
	KMimeType::runBinding( s, popupMenu->text( _id ) );    
    }
}

void KFileManager::openPopupMenu( QStrList &_urls, const QPoint &_point )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	KURL u( s );
	if ( u.isMalformed() )
	    return;
    }
    
    popupMenu->clear();
    // store the mouse position. (Matthias)
    popupMenuPosition = QCursor::pos();        
    
    QStrList bindings;
    bindings.setAutoDelete( TRUE );
    QStrList bindings2;
    bindings2.setAutoDelete( TRUE );
    
    // char buffer[ 1024 ];
    
    bool isdir = KIOServer::isDir( _urls );

    int id;
    if ( isdir )
    {
	id = popupMenu->insertItem( "Cd", view, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( "New View", view, SLOT( slotPopupNewView() ) );
	popupMenu->insertSeparator();
	id = popupMenu->insertItem( "Copy", view, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Move to Trash",  view, SLOT( slotPopupTrash() ) );
	id = popupMenu->insertItem( "Delete",  view, SLOT( slotPopupDelete() ) );
	id = popupMenu->insertItem( "Paste",  view, SLOT( slotPopupPaste() ) );
	if ( KfmView::clipboard.count() == 0 )
	    popupMenu->setItemEnabled( id, FALSE );
    }
    else
    {
	id = popupMenu->insertItem( "Open With", view, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( "Copy", view, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Move to Trash",  view, SLOT( slotPopupTrash() ) );
	id = popupMenu->insertItem( "Delete",  view, SLOT( slotPopupDelete() ) );
    }

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
    
    if ( !bindings.isEmpty() )
    {
	popupMenu->insertSeparator();

	char *str;
	for ( str = bindings.first(); str != 0L; str = bindings.next() )
	{
	    popupMenu->insertItem( str );
	}
    }

    if ( _urls.count() == 1 )
    {
	popupMenu->insertSeparator();
	popupMenu->insertItem( "Properties", view, SLOT( slotPopupProperties() ) );
    }
    
    popupMenu->popup( _point );
}

KFileManager::~KFileManager()
{
}

//------------------------------------------------------------------------
//
// KTarManager
//
//------------------------------------------------------------------------

KTarManager::KTarManager( KfmView * _v )
    : KAbstractManager( _v )
{
    files.setAutoDelete( TRUE );
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );
}

bool KTarManager::openURL( const char *_url, bool _reload )
{
    debugT("Changing to tar %s\n", _url );
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
	debugT("ERROR: Malformed URL\n");
	return FALSE;
    }
    
    url = _url;
    url.detach();
    if ( url.find( "#" ) == -1 )
	url += "#";

    if ( !KIOServer::isDir( url.data() ) )
    {
	KMimeType::runBinding( url.data() );
	return FALSE;
    }

    /* if ( url.right( 1 ) != "#" && url.right( 1 ) != "/" )
	url += "/"; */
    
    view->getGUI()->slotAddWaitingWidget( view );
    
    /*    view->parse();
    view->begin();
    view->write( "<html><head><title>Invoking TAR</title></head><body>");
    view->write( "<h1>Loading ");
    view->write( url.data() );
    view->write( "</h1></body>" );
    view->end(); */
    
    files.clear();

    KIOJob * job = new KIOJob;
    connect( job, SIGNAL( newDirEntry( int, KIODirectoryEntry* ) ),
	     this, SLOT( slotNewDirEntry( int, KIODirectoryEntry* ) ) );
    connect( job, SIGNAL( finished( int ) ), this, SLOT( slotShowFiles( int ) ) );

    job->list( url.data(), _reload );

    view->getGUI()->slotSetStatusBar( "Reading tar file..." );

    return TRUE;
}

void KTarManager::slotNewDirEntry( int, KIODirectoryEntry * _entry )
{
    if ( _entry != 0L )
    {
	KIODirectoryEntry *e = new KIODirectoryEntry( *_entry );
	files.append( e );
    }
}

void KTarManager::slotShowFiles( int )
{
    view->getGUI()->slotSetStatusBar( "" );

    view->begin();
    view->parse();
    view->write( "<html><title>" );
    view->write( url.data() );
    view->write( "</title><body>" );

    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
    { }
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	view->write( "<table width=100%>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	view->write( "<table width=100%>" );

    // Write a link back in the filesystem if we are on the 
    // tar files root directory.
    QString fd;

    KURL u( url.data() );
    
    if ( u.reference()[0] == 0 )
    {
	KURL u( url.data() );
	u.cd("..");
	
	fd = "file:";
	fd += u.path();
	debugT("BACK: %s\n",fd.data());
    }
    // Make a link back to the parent directory, but in the tar file!
    else
    {
	int pos = url.find( "#" );
	// Skip the trailing "/" => -2
	int j = url.findRev( "/", url.length() - 2 );
	if ( j == -1 || j < pos )
	    fd = url.left( pos + 1 );
	else
	    fd = url.left( j + 1 );
    }
    
    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
    {
	view->write( "<cell><a href=\"");
	view->write( fd.data() );
	view->write( "\"><center><img src=\"file:" );
	view->write( KMimeType::findType( fd.data() )->getPixmapFile( fd.data() ) );
	view->write( "\"><br>..</center><br></a></cell>" );
    }
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
    {
	view->write( "<tr><td><a href=\"" );
	view->write( fd.data() );
	view->write( "\"><img width=16 height=16 src=\"file:" );
	view->write( KMimeType::findType( fd.data() )->getPixmapFile( fd.data() ) );
	view->write( "\"></td><td>..</a></td>" );
	view->write( "<td></td><td></td><td></td><td></td><td></td></tr>" );
    }
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
    {
	view->write( "<tr><td><a href=\"" );
	view->write( fd.data() );
	view->write( "\">../</td><td></a></td>" );
	view->write( "<td></td><td></td><td></td><td></td></tr>" );
    }

    char buffer[ 1024 ];
    
    KIODirectoryEntry *s;
    for ( s = files.first(); s != 0L; s = files.next() )
    {
	if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
	{ 
	    view->write( "<cell><a href=\"" );
	    
	    QString filename( url );
	    filename.detach();
	    filename += s->getName();
	    
	    view->write( filename.data() );
	    view->write( "\"><center><img src=\"file:" );
	    
	    view->write( KMimeType::findType( filename.data() )->getPixmapFile( filename.data() ) );
	    
	    view->write( "\"><br>" );
	    // view->write( s->getName() );
	    strcpy( buffer, s->getName() );
	    writeWrapped( buffer );
	    view->write( "</center><br></a></cell>" );
	}
	else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	{
	    view->write( "<tr><td><a href=\"" );
	    QString filename( url );
	    filename.detach();
	    filename += s->getName();
	    
	    view->write( filename.data() );
	    view->write( "\"><img width=16 height=16 src=\"file:" );
	    view->write( KMimeType::findType( filename.data() )->getPixmapFile( filename.data() ) );
	    view->write( "\"></td><td>" );
	    view->write( s->getAccess() );
	    view->write( "</td><td>" ); 
	    view->write( s->getOwner() );
	    view->write( "</td><td>" );
	    view->write( s->getGroup() );
	    view->write( "</td><td>" );
	    QString tmp;
	    tmp.sprintf( "%i</td><td>", s->getSize() );
	    view->write( tmp.data() );
	    view->write( s->getCreationDate() );
	    view->write( "</td><td>" ); 
	    view->write( s->getName() );
	    view->write( "</a></td></tr>" );
	}
	else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	{
	    view->write( "<tr><td><a href=\"" );
	    QString filename( url );
	    KURL::decodeURL(filename);
	    filename.detach();
	    filename += s->getName();
	    
	    view->write( filename.data() );
	    view->write( "\">" );
	    view->write( s->getName() );
	    view->write( "</td><td>" );
	    view->write( s->getAccess() );
	    view->write( "</td><td>" ); 
	    view->write( s->getOwner() );
	    view->write( "</td><td>" );
	    view->write( s->getGroup() );
	    view->write( "</td><td>" );
	    QString tmp;
	    tmp.sprintf( "%i</td><td>", s->getSize() );
	    view->write( tmp.data() );
	    view->write( s->getCreationDate() );
	    view->write( "</td></td></tr>" );
	}
    }

    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
	view->write( "</body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	view->write( "</table></body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	view->write( "</table></body></html>" );

    view->end();

    view->getGUI()->slotRemoveWaitingWidget( view );
}

void KTarManager::slotPopupActivated( int _id )
{
    if ( popupMenu->text( _id ) == 0)
	return;
    
    QString txt = popupMenu->text( _id );
    
    if ( isBindingHardcoded( txt ) )
	return;

    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
	KMimeType::runBinding( s, popupMenu->text( _id ) );    
}

void KTarManager::openPopupMenu( QStrList &_urls, const QPoint & _point )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	KURL u( s );
	if ( u.isMalformed() )
	    return;
    }
    
    popupMenu->clear();
    // store the mouse position. (Matthias)
    popupMenuPosition = QCursor::pos();         

    QStrList bindings;
    bindings.setAutoDelete( TRUE );
    QStrList bindings2;
    bindings2.setAutoDelete( TRUE );
    
    // char buffer[ 1024 ];
    
    bool isdir = KIOServer::isDir( _urls );

    if ( isdir )
    {
	int id;
	id = popupMenu->insertItem( "Cd", view, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( "New View", view, SLOT( slotPopupNewView() ) );
	id = popupMenu->insertItem( "Move to Trash",  view, SLOT( slotPopupTrash() ) );
	id = popupMenu->insertItem( "Delete",  view, SLOT( slotPopupDelete() ) );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( "Open With", view, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( "Copy", view, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Move to Trash", view, SLOT( slotPopupTrash() ) );
	id = popupMenu->insertItem( "Delete", view, SLOT( slotPopupDelete() ) );
    }

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
    
    if ( !bindings.isEmpty() )
    {
	popupMenu->insertSeparator();

	char *str;
	for ( str = bindings.first(); str != 0L; str = bindings.next() )
	{
	    popupMenu->insertItem( str );
	}
    }

    if ( _urls.count() == 1 )
    {
	popupMenu->insertSeparator();
	popupMenu->insertItem( "Properties", view, SLOT( slotPopupProperties() ) );
    }
    
    popupMenu->popup( _point );
}

KTarManager::~KTarManager()
{
}

//------------------------------------------------------------------------
//
// KFtpManager
//
//------------------------------------------------------------------------

KFtpManager::KFtpManager( KfmView * _v )
    : KAbstractManager( _v )
{
    files.setAutoDelete( TRUE );
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );
    job = 0L;

    timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( slotProgressTimeout() ) );
}

bool KFtpManager::openURL( const char *_url, bool _reload )
{
    debugT("Changing to ftp %s\n", _url );
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
	debugT("ERROR: Malformed URL\n");
	return FALSE;
    }
    
    if ( !KIOServer::isDir( _url ) )
    {
	KMimeType::runBinding( _url );
	return FALSE;
    }
    
    url = _url;
    url.detach();
    if ( url.right( 1 ) != "/" )
	url += "/"; 

    /* view->begin();
    view->parse();
    view->write( "<html><head><title>HTML loading...</title></head><body>");
    view->write( "<h1>Loading ");
    view->write( url.data() );
    view->write( "</h1></body>" );
    view->end(); */
    
    files.clear();

    if ( job )
	job->cancel();
    
    job = new KIOJob;
    connect( job, SIGNAL( newDirEntry( int, KIODirectoryEntry* ) ),
	     this, SLOT( slotNewDirEntry( int, KIODirectoryEntry* ) ) );
    connect( job, SIGNAL( finished( int ) ), this, SLOT( slotShowFiles( int ) ) );
    connect( job, SIGNAL( progress( int, int ) ), this, SLOT( slotProgress( int, int ) ) );

    bytesTransfered = 0;
    percent = 0;
    timer1.start();
    timer2.start();
    timer->start( 5000 );

    QString tmp;
    tmp.sprintf( "Contacting host %s", u.host() );
    view->getGUI()->slotSetStatusBar( tmp );
    
    job->display( FALSE );
    job->list( url.data(), _reload );

    return TRUE;
}

void KFtpManager::slotNewDirEntry( int , KIODirectoryEntry * _entry )
{
    if ( _entry != 0L )
    {
	KIODirectoryEntry *e = new KIODirectoryEntry( *_entry );
	files.append( e );
	
	// first entry ?
	if ( files.count() == 1 )
	    writeBeginning();
	
	debugT("FTP 1\n");
	writeEntry( e );
	debugT("FTP 2\n");
    }
}

void KFtpManager::writeBeginning()
{
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
	    view->write( "\"><center><img src=\"file:" );
	    view->write( KMimeType::findType( s.data() )->getPixmapFile( s.data() ) );
	    view->write( "\"><br>..</center><br></a></cell>" );
	}
	else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	{
	    view->write( "<tr><td><a href=\"" );
	    view->write( s.data() );
	    view->write( "\"><img width=16 height=16 src=\"file:" );
	    view->write( KMimeType::findType( s.data() )->getPixmapFile( s.data() ) );
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

void KFtpManager::writeEntry( KIODirectoryEntry *s )
{
    char buffer[ 1024 ];
    
    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
    { 
	view->write( "<cell><a href=\"" );
	
	QString filename( url );
	filename.detach();
	filename += s->getName();
	
	view->write( filename.data() );
	view->write( "\"><center><img src=\"file:" );
	
	view->write( KMimeType::findType( filename.data() )->getPixmapFile( filename.data() ) );
	
	view->write( "\"><br>" );
	// view->write( s->getName() );
	strcpy( buffer, s->getName() );
	writeWrapped( buffer );
	view->write( "</center><br></a></cell>" );
    }
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
    {
	view->write( "<tr><td><a href=\"" );
	QString filename( url );
	filename.detach();
	filename += s->getName();
	
	view->write( filename.data() );
	view->write( "\"><img width=16 height=16 src=\"file:" );
	view->write( KMimeType::findType( filename.data() )->getPixmapFile( filename.data() ) );
	view->write( "\"></td><td>" );
	view->write( s->getName() );
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
	view->write( s->getName() );
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

void KFtpManager::slotProgressTimeout()
{
    slotProgress( percent, bytesTransfered );
}

void KFtpManager::slotProgress( int _percent, int _bytesTransfered )
{
    int oldBytesTransfered = bytesTransfered;
    bool stalled = FALSE;
    
    percent = _percent;
    bytesTransfered = _bytesTransfered;
    
    if ( bytesTransfered != oldBytesTransfered )
	timer2.restart();
    else if ( timer2.elapsed() > 5000 )
	stalled = TRUE;

    float rate = (float)bytesTransfered / 1024. / ( (float)timer1.elapsed() / 1000.0 );

    QString tmp2;
    if ( bytesTransfered > 1024 )
	tmp2.sprintf( "%i kB read at ", bytesTransfered / 1024 );
    else
	tmp2.sprintf( "%i Bytes read at ", bytesTransfered );
    
    QString tmp;
    if ( stalled )
	tmp.sprintf( "(stalled)" );
    else if ( rate >= 1 )
	tmp.sprintf( "%f kB/s", rate );
    else
	tmp.sprintf( "%f Byte/s", rate * 1024 );

    view->getGUI()->slotSetStatusBar( tmp2 + tmp );
}

void KFtpManager::stop()
{
    if ( job )
    {
	job->cancel();
	view->end();
    }
}

void KFtpManager::slotShowFiles( int )
{
    job = 0L;

    timer->stop();

    if ( files.count() == 0 )
	writeBeginning();
    
    // Write the end of the HTML stuff
    if ( view->getGUI()->getViewMode() == KfmGui::ICON_VIEW )
	view->write( "</body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::LONG_VIEW )
	view->write( "</table></body></html>" );
    else if ( view->getGUI()->getViewMode() == KfmGui::TEXT_VIEW )
	view->write( "</table></body></html>" );

    debugT("End\n");
    view->end();

    debugT("Done\n");
}

void KFtpManager::slotPopupActivated( int _id )
{
    if ( popupMenu->text( _id ) == 0)
	return;
    
    QString txt = popupMenu->text( _id );
    
    if ( isBindingHardcoded( txt ) )
	return;

    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	debugT("Exec '%s'\n", s );
	KMimeType::runBinding( s, popupMenu->text( _id ) );    
    }
}

void KFtpManager::openPopupMenu( QStrList &_urls, const QPoint & _point )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	debugT("Opening for '%s'\n",s);
	
	KURL u( s );
	if ( u.isMalformed() )
	    return;
    }
    
    popupMenu->clear();
    // store the mouse position. (Matthias)
    popupMenuPosition = QCursor::pos();       
    
    QStrList bindings;
    bindings.setAutoDelete( TRUE );
    QStrList bindings2;
    bindings2.setAutoDelete( TRUE );
    
    // char buffer[ 1024 ];
    
    bool isdir = KIOServer::isDir( _urls );

    if ( isdir )
    {
	int id;
	id = popupMenu->insertItem( "Cd", view, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( "New View", view, SLOT( slotPopupNewView() ) );
	id = popupMenu->insertItem( "Move to Trash",  view, SLOT( slotPopupTrash() ) );
	id = popupMenu->insertItem( "Delete",  view, SLOT( slotPopupDelete() ) );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( "Open With", view, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( "Copy", view, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Move to Trash",  view, SLOT( slotPopupTrash() ) );
	id = popupMenu->insertItem( "Delete",  view, SLOT( slotPopupDelete() ) );
    }

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
    
    if ( !bindings.isEmpty() )
    {
	popupMenu->insertSeparator();

	char *str;
	for ( str = bindings.first(); str != 0L; str = bindings.next() )
	{
	    popupMenu->insertItem( str );
	}
    }

    if ( _urls.count() == 1 )
    {
	popupMenu->insertSeparator();
	popupMenu->insertItem( "Properties", view, SLOT( slotPopupProperties() ) );
    }
    
    popupMenu->popup( _point );
}

void KFtpManager::dropPopupMenu( KDNDDropZone *_zone, const char *_dest, const QPoint *_p )
{
    dropDestination = _dest;
    dropDestination.detach();
    
    dropZone = _zone;
    
    debugT(" Drop with destination %s\n", _dest );
    
    // Perhaps an executable ?
    if ( !KIOServer::isDir( _dest ) )
    {
	debugT("ERROR: Dont know what to do on ftp file\n");
	return;
    }
    
    popupMenu->clear();
    
    int id = 1;
    // Ask wether we can read from the dropped URL.
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( _dest, KIO_Write ) )
	id = popupMenu->insertItem( "Copy", this, SLOT( slotDropCopy() ) );
    // Ask wether we can read from the URL and delete it afterwards
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( _zone->getURLList(), KIO_Delete ) &&
	 KIOServer::supports( _dest, KIO_Write ) )
	id = popupMenu->insertItem( "Move", this, SLOT( slotDropMove() ) );
    if ( id == -1 )
    {
	debugT("ERROR: Can not accept drop\n");
	return;
    }
    
    popupMenu->popup( *_p );
}

void KFtpManager::slotDropCopy()
{
    KIOJob * job = new KIOJob;
    debugT("COPY count=%i\n",dropZone->getURLList().count());
    
    job->copy( dropZone->getURLList(), dropDestination.data() );
}

void KFtpManager::slotDropMove()
{
    KIOJob * job = new KIOJob;
    job->move( dropZone->getURLList(), dropDestination.data() );
}

KFtpManager::~KFtpManager()
{
}

KHttpManager::KHttpManager( KfmView * _v )
    : KAbstractManager( _v )
{
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );
    htmlCache = new HTMLCache();

    running = FALSE;
    f = 0L;
    
    connect( htmlCache, SIGNAL( urlLoaded( const char*, const char *) ),
	     this, SLOT( slotShowHTML( const char*, const char* ) ) );
    connect( htmlCache, SIGNAL( progress( const char*, const char*, int, int, float, bool ) ),
	     this, SLOT( slotProgress( const char*, const char*, int, int, float, bool ) ) );
}

bool KHttpManager::openURL( const char *_url, bool )
{
    debugT("Changing to http %s\n", _url );

    KURL u( _url );
    if ( u.isMalformed() )
    {
	debugT("ERROR: Malformed URL\n");
	return FALSE;
    }
    
    url = _url;
    url.detach();

    if ( running )
      stop();
    
    bytesRead = 0;
    
    debugT("Requesting\n");
    
    htmlCache->slotURLRequest( url );
    running = TRUE;

    debugT("Requested\n");
    
    // Set the status bar
    QString tmp;
    tmp.sprintf( "Contacting host %s", u.host() );
    view->getGUI()->slotSetStatusBar( tmp );
    
    debugT("Returning\n");
    
    return TRUE;
}

void KHttpManager::slotProgress( const char *_url, const char *_filename, 
				 int _percent, int , float _rate, bool _stalled )
{
  debugT("Progress called\n");

    if ( url.isNull() )
	return;
    
    if ( strcmp( _url, url.data() ) != 0 )
	return;

    debugT("Is for me\n");
    
    QString tmp;
    if ( _stalled )
	tmp.sprintf( "%i %% of page, (stalled)", _percent );
    else if ( _rate >= 1 )
	tmp.sprintf( "%i %% of page, %f kB/s", _percent, _rate );
    else
	tmp.sprintf( "%i %% of page, %f Byte/s", _percent, _rate * 1024 );
    view->getGUI()->slotSetStatusBar( tmp );
    
    debugT("Done Statusbar\n");
    
    // Load all stuff that is available
    struct stat buff;
    stat( _filename, &buff );
    int bytes = buff.st_size;
    
    if ( bytes < 1024 )
	return;
    
    debugT("Reading bytes\n");
    
    if ( bytes - bytesRead >= 1024 )
    {
	if ( f == 0L )
	{
	  debugT("Opening\n");
	  
	    view->begin( url.data() );
	    view->parse();
	    f = fopen( _filename, "rb" );
	}
	
	if ( f != 0L )
	{
	  debugT("Have handle\n");
	  
	    int m = bytes - bytesRead + 1;
	    if ( m > 4095 )
		m = 4095;
	    char buffer[ m + 1 ];
	    int n = fread( buffer, 1, m, f );
	    buffer[n] = 0;
	    view->write( buffer );
	    bytesRead += n;
	}
	debugT("Done1\n");
    }
    debugT("Done2\n");
}

void KHttpManager::slotShowHTML( const char *_url, const char *_filename )
{  
    debugT("ShowHTML\n");
  
    if ( url.isNull() )
	return;
    
    if ( strcmp( _url, url.data() ) != 0 )
	return;

    view->getGUI()->slotSetStatusBar( "Reading images..." );
    
    debugT("ShowHTML 2\n");
    running = FALSE;
    
    char buffer[ 1024 ];
    
    if ( f == 0 )
    {
	f = fopen( _filename, "rb" );
	view->begin( url.data() );
	view->parse();
    }
    
    if ( f == 0 )
    {
	debugT("ERROR: Could not access '%s'\n",_filename );
	return;
    }
    
    debugT("ShowHTML 3\n");
    int n = 0;
    do
    {
	n = fread( buffer, 1, 1023, f );
	debugT("Read '%i' Bytes\n", n );
	buffer[n] = 0;
	view->write( buffer );
    } while ( n > 0 );

    debugT("ShowHTML 4\n");
    fclose( f );
    f = 0L;
    
  debugT("ShowHTML 5\n");
    view->end();
  debugT("ShowHTML 6\n");
} 

void KHttpManager::slotPopupActivated( int _id )
{
    QString txt = popupMenu->text( _id );
    
    if ( isBindingHardcoded( txt ) )
	return;

    // TODO
    /*
    if ( popupMenu->text( _id ) == 0)
	return;
    
    KMimeType *typ = KMimeType::findType( popup_file );
    KFileBind *bind;
    for ( bind = typ->firstBinding(); bind != 0L; bind = typ->nextBinding() )
    {
	if ( strcmp( bind->getProgram(), popupMenu->text( _id ) ) == 0 )
	{
	    debugT( bind->getCmd(), ((const char*)popup_file)+6 );
	    char *s = new char[ strlen( bind->getCmd() ) + strlen( (const char*)popup_file ) + 10 ];
	    sprintf( s, bind->getCmd(), ((const char*)popup_file)+6 );
	    strcat( s, " &" );
	    system( s );
	}
    }
    */
}

void KHttpManager::openPopupMenu( QStrList & _urls, const QPoint & _point )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	KURL u( s );
	if ( u.isMalformed() )
	    return;
    }
    
    popupMenu->clear();
    // store the mouse position. (Matthias)
    popupMenuPosition = QCursor::pos();        
    
    QStrList bindings;
    bindings.setAutoDelete( TRUE );
    QStrList bindings2;
    bindings2.setAutoDelete( TRUE );
    
    // char buffer[ 1024 ];
    
    bool isdir = KIOServer::isDir( _urls );

    int id;
    if ( isdir )
    {
	id = popupMenu->insertItem( "Cd", view, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( "New View", view, SLOT( slotPopupNewView() ) );
	popupMenu->insertSeparator();
	id = popupMenu->insertItem( "Copy", view, SLOT( slotPopupCopy() ) );
    }
    else
    {
	id = popupMenu->insertItem( "Open With", view, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( "Copy", view, SLOT( slotPopupCopy() ) );
    }

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
    
    if ( !bindings.isEmpty() )
    {
	popupMenu->insertSeparator();

	char *str;
	for ( str = bindings.first(); str != 0L; str = bindings.next() )
	{
	    popupMenu->insertItem( str );
	}
    }

    if ( _urls.count() == 1 )
    {
	popupMenu->insertSeparator();
	popupMenu->insertItem( "Properties", view, SLOT( slotPopupProperties() ) );
    }
    
    popupMenu->popup( _point );
}

void KHttpManager::stop()
{
    if ( f )
    {
	fclose( f );
	f = 0L;
    }
    
    if ( running )
    {
      running = FALSE;
      htmlCache->slotCancelURLRequest( url );
    }
}

KHttpManager::~KHttpManager()
{
}

KCgiManager::KCgiManager( KfmView * _v )
    : KAbstractManager( _v )
{
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );

    cgiServer = new KCGI();
    connect( cgiServer, SIGNAL( finished() ), this, SLOT( slotShowHTML() ) );
}

bool KCgiManager::openURL( const char *_url, bool )
{
    debugT("Changing to cgi %s\n", _url );

    debugT("1\n");
    
    url = _url;
        
    debugT("2\n");

    /* view->begin( url.data() );
    view->parse();
    view->write( "<html><head><title>HTML loading...</title></head><body>");
    view->write( "<h1>Loading ");
    view->write( _url );
    view->write( "</h1></body></html>" );
    view->end(); */

    debugT("3\n");

    if ( running )
      cgiServer->stop();
    
    // Slip "cgi:"
    tmpFile = cgiServer->get( _url + 4, "GET" );
    if ( tmpFile.isNull() )
    {
	view->begin( url.data() );
	view->parse();
	view->write( "<html><head><title>CGI Server Error</title></head><body>");
	view->write( "<h1>CGI Server Error</h1> ");
	view->write( _url );
	view->write( "</body></html>" );
	view->end();
	return TRUE;
    }
    
    tmpFile.detach();
    running = TRUE;

    view->getGUI()->slotAddWaitingWidget( view );
    
    debugT("5\n");
    return TRUE;
}

void KCgiManager::slotShowHTML()
{
    view->getGUI()->slotRemoveWaitingWidget( view );
    
    debugT("((((((((((((((((((((((((((( CGI finished )))))))))))))))))))))))))))\n");
    
    if ( url.isNull() )
	return;
        
    running = FALSE;
    
    char buffer[ 1024 ];
    
    FILE *f = fopen( tmpFile.data(), "rb" );
    if ( f == 0 )
    {
	debugT("ERROR: Could not access '%s'\n", tmpFile.data() );
	return;
    }

    view->begin( url.data() );
    view->parse();
    
    int n;
    do
    {
	n = fread( buffer, 1, 1023, f );
	if ( n > 0 )
	{
	    buffer[n] = 0;
	    debugT("'%s'\n",buffer);
	    view->write( buffer );
	}
    } while ( n != 0 );

    fclose( f );
    unlink( tmpFile.data() );
    
    view->end();
} 

void KCgiManager::slotPopupActivated( int _id )
{
    QString txt = popupMenu->text( _id );
    
    if ( isBindingHardcoded( txt ) )
	return;
}

void KCgiManager::openPopupMenu( QStrList & , const QPoint &  )
{
}

void KCgiManager::stop()
{
    view->getGUI()->slotRemoveWaitingWidget( view );

    if ( running )
    {
      running = FALSE;
      cgiServer->stop();
    }
}

KCgiManager::~KCgiManager()
{
}


#include "kfmman.moc"
