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
#include <qmsgbox.h>
#include <Kconfig.h>

#include "kfmman.h"
#include "xview.h"

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
    
KFileManager::KFileManager( KFileWindow * _w, KFileView * _v ) : KAbstractManager( _w, _v )
{
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );
}

void KFileManager::dropPopupMenu( KDNDDropZone *_zone, const char *_dest, const QPoint *_p )
{
    dropDestination = _dest;
    dropDestination.detach();
    
    dropZone = _zone;
    
    printf(" Drop with destination %s\n", _dest );
    
    // Perhaps an executable ?
    if ( !KIOServer::isDir( _dest ) )
    {
	KFileType::runBinding( _dest, "Open", &(_zone->getURLList()) );
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
	printf("ERROR: Can not accept drop\n");
	return;
    }
    
    popupMenu->popup( *_p );
}

void KFileManager::slotDropCopy()
{
    KIOJob * job = new KIOJob;
    printf("COPY count=%i\n",dropZone->getURLList().count());
    
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
    // Open HTML file ".kde.html"
    FILE *f = fopen( _filename, "rb" );
    if ( f == 0 )
    {
	printf("ERROR: Could not read '%s'\n",_filename );
	return;
    }
    
    KURL u( url.data() );

    QString d = url.data();
    if ( d.right(1) != "/" )
	d += "/";

    if ( _refresh )
	view->begin( d.data(), view->xOffset(), view->yOffset() );
    else
	view->begin( d.data() );
    
    // Read the HTML code into memory
    char *mem = (char*) malloc( 30000 );
    int size = 0;
    while ( !feof( f ) )
    {
	int n = fread( mem + size, 1, 30000, f );
	size += n;
	if ( n == 30000 )
	{
	    char *p = (char*) malloc( size + 30000 );
	    memcpy( p, mem, size );
	    free( mem );
	    mem = p;
	}
    }

    fclose( f );

    // Read the directory listing
    DIR *dp;
    struct dirent *ep;
    dp = opendir( u.path() );
    if ( dp == NULL )
    {
	view->write( "<h1>Could not access directory</h1></body></html>" );
	view->end();
	view->parse();
	return;
    }

    QStrList strlist( TRUE );    
    // Loop thru all directory entries
    while ( ep = readdir( dp ) )
	strlist.inSort( ep->d_name );
    (void) closedir( dp );
	
    // Stores all files we already displayed.
    // This is used for filter option "Rest"
    QList<QString> displayedFiles;
    displayedFiles.setAutoDelete( TRUE );
    // Indicates wether filter option "Rest" is currently selected.
    bool rest;
    
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
			/* else if ( strcasecmp( f, "NoSymLinks" ) == 0 )
			    filter |= QDir::NoSymLinks;
			else if ( strcasecmp( f, "Readable" ) == 0 )
			    filter |= QDir::Readable;
			else if ( strcasecmp( f, "Writable" ) == 0 )
			    filter |= QDir::Writable; */
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
			    printf("Unknown filter option %s\n",f);
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

	    view->write( "<grid width=80>" );
	    
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
			view->write( KFileType::findType( buff )->getPixmapFile( buff ) );
			view->write( "\"><br>" );
			view->write( fn );
			view->write( "</center><br></a></cell>" );
		    }
		}
	    }
	    view->write( "</grid>" );

	}

	old = ++p;	
    }
    view->write( old );

    free( mem );
    
    view->end();
    view->parse();
}

bool KFileManager::openURL( const char *_url, bool _refresh )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return FALSE;

    printf("Changing to %s\n", _url );

    // Is it a directory? If not ...
    if ( !KIOServer::isDir( _url ) )
    {
	// Did the user double click on a *.kdelnk file ?
	QString tmp = _url;
	if ( tmp.length() > 7 && tmp.right(7) == ".kdelnk" )
	{
	    // KFileType::runKDEFile( _dir );
	    printf("#################### KFileManager::openURL\n");
	    KFileType::runBinding( _url );
	    return FALSE;
	}
	// Are we entering a tar file ?
	else if ( strstr( _url, ".tar" ) != 0 || strstr( _url, ".tgz") != 0 || strstr( _url, ".tar.gz" ) != 0 )
	{	
	    QString t( "tar:" );
	    t += u.path();

	    // Are we belonging to the root widget ?
	    if ( window == 0L || view == 0L )
	    {
		KFileWindow *m = new KFileWindow( 0L, 0L, t.data() );
		m->show();
	    }
	    else
	    {
		window->getTarManager()->openURL( t.data() );
		window->refresh( window->getTarManager() );
	    }
	    return FALSE;
	}
	else
	{
	    struct stat buff;
	    if ( 0 > stat( u.path(), &buff ) )
	    {
		printf("ERROR: Could not access file %s\n",_url );
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
		KFileType::runBinding( _url );
		return FALSE;
	    }   
	}
	printf("ERROR: Could not change to %s\n",_url);
	return FALSE;
    }

    printf("Nothing special\n");
    
    // Are we used by the root widget perhaps ?
    if ( window == 0L || view == 0L )
    {
	KFileWindow *m = new KFileWindow( 0L, 0L, _url );
	m->show();
	return FALSE;
    }

    url = _url;
    url.detach();

    DIR *dp;
    struct dirent *ep;
    dp = opendir( u.path() );
    if ( dp == NULL )
    {
	view->begin( _url );
	view->write( "<h1>Could not access directory</h1></body></html>" );
	view->end();
	view->parse();
	return TRUE;
    }
    
    printf("Opened dir\n");
    
    QString html;
    
    QStrList strlist( TRUE );
    
    // Loop thru all directory entries
    while ( ep = readdir( dp ) )
    {
	strlist.inSort( ep->d_name );
	if ( strcmp( ep->d_name, ".kde.html" ) == 0 )
	    html = ".kde.html";
	else if ( strcasecmp( ep->d_name, "index.html" ) == 0 && html.isNull() )
	    html = ep->d_name;
    }
    closedir( dp );
    
    printf("Read Dir\n");
    
    if ( !html.isNull() && window->isViewHTML() )
    {
	QString h = u.path();
	if ( h.right(1) != "/" )
	    h += "/";
	h += html.data();
	openDirHTML( h.data(), _refresh );
	return TRUE;
    }

    printf("Checking for config\n");
    
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
	    bg_image = getenv( "KDEDIR" );
	    bg_image += "/lib/pics/wallpapers/";
	    bg_image += tmp2.data();
	}
    
    QString d = u.path();
    if ( d.right( 1 ) != "/" )
	d += "/.directory";
    else
	d += ".directory";
    
    printf("Trying .directory\n");
    
    QFile f( d.data() );
    if ( f.open( IO_ReadOnly ) )
    {
	printf("Opened .directory\n");
	
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
		bg_image = getenv( "KDEDIR" );
		bg_image += "/lib/pics/wallpapers/";
		bg_image += tmp.data();
	    }
    }

    printf("Back again\n");
    
    // Open the directory the usual way.    
    char str[ 1024 ];

    QString d2 = _url;
    printf("d2 = '%s'\n",d2.data());
    
    d2.detach();
    if ( d2.right( 1 ) != "/" )
	d2 += "/";

    printf("B2\n");
    
    // If we only want to refresh dont change the scrollbars if possible.
    if ( _refresh )
    {
	printf("!!!!!!!!!!!!!!!! REFRESH !!!!!!!!!!!!!!!!!\n");
	view->begin( d2.data(), view->xOffset(), view->yOffset() );
    }
    else
    {
	printf("X\n");
	view->begin( d2.data() );
	printf("Y\n");
    }
    
    printf("1\n");
    
    view->write( "<html><title>" );
    view->write( _url );
    view->write( "</title><body" );

    printf("2\n");

    if ( !text_color.isNull() )
    {
	printf("3\n");
	view->write(" text=" );
	view->write( text_color.data() );
    }
    printf("4\n");
    if ( !link_color.isNull() )
    {
	printf("5\n");
	view->write(" link=" );
	view->write( link_color.data() );
    }
    printf("6\n");
    if ( !bg_image.isNull() )
    {
	printf("7\n");
	KURL u2( u, bg_image.data() );
	view->write(" background=\"" );
	QString t = u2.url();
	view->write( t.data() );
	view->write( "\"" );
    }
    else if ( !bg_color.isNull() )
    {
	printf("8\n");
	view->write(" bgcolor=" );
	view->write( bg_color.data() );
    }
    printf("9\n");
    view->write( ">" );

    printf("C\n");
    
    if ( window->getViewMode() == KFileWindow::ICON_VIEW )
	view->write( "<grid width=80>" );
    else if ( window->getViewMode() == KFileWindow::LONG_VIEW )
	view->write( "<table width=100%>" );
    else if ( window->getViewMode() == KFileWindow::TEXT_VIEW )
	view->write( "<table width=100%>" );
    
    printf("Loop\n");

    bool visualSchnauzer = FALSE;
    QString xv = u.path();
    if ( window->isVisualSchnauzer() )
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
    char* fn;
    for ( fn = strlist.first(); fn != 0L; fn = strlist.next() )
    {
	// Dont display the file "."
	if ( strcmp( fn, "." ) != 0 )
	{
	    if ( fn[0] == '.' && !window->getShowDot() && strcmp( fn, ".." ))
	      continue;
	    if ( strcmp( fn, ".." ) == 0 )
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
		strcat( str, fn );
	    }
	    
	    QString tmp = fn;
	    // Select the view mode.
	    if ( window->getViewMode() == KFileWindow::ICON_VIEW )
	    {
		view->write( "<cell><a href=\"" ); 
		view->write( str );
		view->write( "\"><center><img src=\"" ); 
	
		if ( window->isVisualSchnauzer() && visualSchnauzer )
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
		    xvfile += fn;
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
			view->write( KFileType::findType( str )->getPixmapFile( str ) );
			view->write( "\"><br>" );
		    }   
		}
		else
                {    
		  view->write( KFileType::findType( str )->getPixmapFile( str ) );
		  view->write( "\"><br>" );
		}

		// Delete a trailing .kdelnk in the filename
		if ( tmp.length() > 7 && tmp.right(7) == ".kdelnk" )
		{
		    strcpy( buffer, fn );
		    buffer[ strlen( buffer ) - 7 ] = 0;
		    view->write( buffer );
		}
		else
		  view->write( fn );
		
		view->write( "</center><br></a></cell>" );
	    }
	    else if ( window->getViewMode() == KFileWindow::LONG_VIEW ||
		      window->getViewMode() == KFileWindow::TEXT_VIEW )
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
		if ( window->getViewMode() == KFileWindow::LONG_VIEW )
		{
		    view->write( "\"><img width=16 height=16 src=\"" ); 
		    view->write( KFileType::findType( str )->getPixmapFile( str ) );
		    view->write( "\"></td><td>" );
		}
		else
		    view->write( "\">" );
		
		if ( S_ISDIR( buff.st_mode ) )
		    buffer[0] = 'd';
		else if ( S_ISLNK( lbuff.st_mode ) )
		    buffer[0] = 'l';
		else
		    buffer[0] = '-';
		sprintf( buffer + 1,
			 "%c%c%c%c%c%c%c%c%c</td><td>%8s</td><td>%8s</td><td>%i</td><td>%i:%i</td><td>%i.%i.%i</td><td>",
			 ((( buff.st_mode & S_IRUSR ) == S_IRUSR ) ? 'r' : '-' ),
			 ((( buff.st_mode & S_IWUSR ) == S_IWUSR ) ? 'w' : '-' ),
			 ((( buff.st_mode & S_IXUSR ) == S_IXUSR ) ? 'x' : '-' ),
			 ((( buff.st_mode & S_IRGRP ) == S_IRGRP ) ? 'r' : '-' ),
			 ((( buff.st_mode & S_IWGRP ) == S_IWGRP ) ? 'w' : '-' ),
			 ((( buff.st_mode & S_IXGRP ) == S_IXGRP ) ? 'x' : '-' ),
			 ((( buff.st_mode & S_IROTH ) == S_IROTH ) ? 'r' : '-' ),
			 ((( buff.st_mode & S_IWOTH ) == S_IWOTH ) ? 'w' : '-' ),
			 ((( buff.st_mode & S_IXOTH ) == S_IXOTH ) ? 'x' : '-' ),
			 (( user != 0L ) ? user->pw_name : "???" ),
			 (( grp != 0L ) ? grp->gr_name : "???" ),
			 buff.st_size, t->tm_hour,t->tm_min,t->tm_mday,t->tm_mon,t->tm_year );
		view->write( buffer );
		// Delete a trailing .kdelnk in the filename
		if ( tmp.length() > 7 && tmp.right(7) == ".kdelnk" )
		{
		    strcpy( buffer, fn );
		    buffer[ strlen( buffer ) - 7 ] = 0;
		    view->write( buffer );
		}
		else
		    view->write( fn );
		view->write( "</a></td></tr>" );
	    }
	}
    }

    printf("End Loop\n");
    
    if ( window->getViewMode() == KFileWindow::ICON_VIEW )
	view->write( "</grid></body></html>" );
    else if ( window->getViewMode() == KFileWindow::LONG_VIEW )
	view->write( "</table></body></html>" );
    else if ( window->getViewMode() == KFileWindow::TEXT_VIEW )
	view->write( "</table></body></html>" );

    view->end();

    printf("Parse\n");
    
    view->parse();

    printf("/Parse\n");
    
    return TRUE;
}

void KFileManager::slotPopupActivated( int _id )
{
    if ( popupMenu->text( _id ) == 0)
	return;
    
    printf("******** PopupFile '%i'\n",popupFiles.count());
    
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	printf("$$$$$$$$$$ Doing it for '%s'\n",s);
	KFileType::runBinding( s, popupMenu->text( _id ) );    
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
    
    char buffer[ 1024 ];
    
    bool isdir = KIOServer::isDir( _urls );

    if ( isdir )
    {
	int id;
	id = popupMenu->insertItem( "Cd", window, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( "New View", window, SLOT( slotPopupNewView() ) );
	popupMenu->insertSeparator();
	id = popupMenu->insertItem( "Copy", window, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Delete",  window, SLOT( slotPopupDelete() ) );
	id = popupMenu->insertItem( "Paste",  window, SLOT( slotPopupPaste() ) );
	if ( KFileWindow::clipboard.count() == 0 )
	    popupMenu->setItemEnabled( id, FALSE );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( "Copy", window, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Delete",  window, SLOT( slotPopupDelete() ) );
    }

    window->setPopupFiles( _urls );
    popupFiles.copy( _urls );
    
    // Get all bindings matching all files.
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	// If this is the first file in the list, assume that all bindings are ok
	if ( s == _urls.getFirst() )
	{
	    KFileType::getBindings( bindings, s, isdir );
	}
	// Take only bindings, matching all files.
	else
	{
	    KFileType::getBindings( bindings2, s, isdir );
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
	popupMenu->insertItem( "Properties", window, SLOT( slotPopupProperties() ) );
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

KTarManager::KTarManager( KFileWindow * _w, KFileView * _v )
    : KAbstractManager( _w, _v )
{
    files.setAutoDelete( TRUE );
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );
}

bool KTarManager::openURL( const char *_url, bool _reload )
{
    printf("Changing to tar %s\n", _url );
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
	printf("ERROR: Malformed URL\n");
	return FALSE;
    }
    
    url = _url;
    url.detach();
    if ( url.find( "#" ) == -1 )
	url += "#";
    if ( url.right( 1 ) != "#" && url.right( 1 ) != "/" )
	url += "/";
    
    view->begin();
    view->write( "<html><head><title>Invoking TAR</title></head><body>");
    view->write( "<h1>Loading ");
    view->write( url.data() );
    view->write( "</h1></body>" );
    view->end();
    view->parse();
    
    files.clear();

    KIOJob * job = new KIOJob;
    connect( job, SIGNAL( newDirEntry( int, KIODirectoryEntry* ) ),
	     this, SLOT( slotNewDirEntry( int, KIODirectoryEntry* ) ) );
    connect( job, SIGNAL( finished( int ) ), this, SLOT( slotShowFiles( int ) ) );

    job->list( url.data(), _reload );

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

void KTarManager::slotShowFiles( int _id )
{
    view->begin();
    view->write( "<html><title>" );
    view->write( url.data() );
    view->write( "</title><body><grid width=80>" );

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
	printf("BACK: %s\n",fd.data());
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
    
    view->write( "<cell><a href=\"");
    view->write( fd.data() );
    view->write( "\"><center><img src=\"file:" );
    view->write( PICS_PATH );
    view->write( "/folder.xpm\"><br>..</center><br></a></cell>" );

    KIODirectoryEntry *s;
    for ( s = files.first(); s != 0L; s = files.next() )
    { 
	view->write( "<cell><a href=\"" );

	QString filename( url );
	filename.detach();
	if ( filename[ strlen( filename ) - 1 ] != '/' && filename[ strlen( filename ) - 1 ] != '#' )
	    filename += "/";
	filename += s->getName();
	
	view->write( filename.data() );
	view->write( "\"><center><img src=\"file:" );
	
	view->write( KFileType::findType( filename.data() )->getPixmapFile( filename.data() ) );
	
        view->write( "\"><br>" );
	view->write( s->getName() );
	view->write( "</center><br></a></cell>" );
    }

    view->write( "</grid></body></html>" );
    view->end();

    view->parse();
}

void KTarManager::slotPopupActivated( int _id )
{
    if ( popupMenu->text( _id ) == 0)
	return;
    
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
	KFileType::runBinding( s, popupMenu->text( _id ) );    
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
    
    char buffer[ 1024 ];
    
    bool isdir = KIOServer::isDir( _urls );

    if ( isdir )
    {
	int id;
	id = popupMenu->insertItem( "Cd", window, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( "New View", window, SLOT( slotPopupNewView() ) );
	id = popupMenu->insertItem( "Delete",  window, SLOT( slotPopupDelete() ) );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( "Copy", window, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Delete",  window, SLOT( slotPopupDelete() ) );
    }

    window->setPopupFiles( _urls );
    popupFiles.copy( _urls );
    
    // Get all bindings matching all files.
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	// If this is the first file in the list, assume that all bindings are ok
	if ( s == _urls.getFirst() )
	{
	    KFileType::getBindings( bindings, s, isdir );
	}
	// Take only bindings, matching all files.
	else
	{
	    KFileType::getBindings( bindings2, s, isdir );
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
	popupMenu->insertItem( "Properties", window, SLOT( slotPopupProperties() ) );
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

KFtpManager::KFtpManager( KFileWindow * _w, KFileView * _v )
    : KAbstractManager( _w, _v )
{
    files.setAutoDelete( TRUE );
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );
    job = 0L;
}

bool KFtpManager::openURL( const char *_url, bool _reload )
{
    printf("Changing to ftp %s\n", _url );
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
	printf("ERROR: Malformed URL\n");
	return FALSE;
    }
    
    url = _url;
    url.detach();
    if ( url.right( 1 ) != "/" )
	url += "/";

    view->begin();
    view->write( "<html><head><title>HTML loading...</title></head><body>");
    view->write( "<h1>Loading ");
    view->write( url.data() );
    view->write( "</h1></body>" );
    view->end();
    view->parse();
    
    files.clear();

    if ( job )
	job->cancel();
    
    job = new KIOJob;
    connect( job, SIGNAL( newDirEntry( int, KIODirectoryEntry* ) ),
	     this, SLOT( slotNewDirEntry( int, KIODirectoryEntry* ) ) );
    connect( job, SIGNAL( finished( int ) ), this, SLOT( slotShowFiles( int ) ) );

    window->enableToolbarButton( 7, TRUE );
    job->display( FALSE );
    job->list( url.data(), _reload );

    return TRUE;
}

void KFtpManager::slotNewDirEntry( int _id, KIODirectoryEntry * _entry )
{
    if ( _entry != 0L )
    {
	KIODirectoryEntry *e = new KIODirectoryEntry( *_entry );
	files.append( e );
    }
}

void KFtpManager::stop()
{
    job->cancel();
    window->enableToolbarButton( 7, FALSE );
}

void KFtpManager::slotShowFiles( int _id )
{
    job = 0L;
    window->enableToolbarButton( 7, FALSE );
    
    view->begin( url.data() );
    view->write( "<html><head><title>" );
    view->write( url.data() );
    view->write( "</title></head><body bgcolor=#FFFFFF>" );

    // Write a link back in the ftp filesystem if we are not in the 
    // ftp servers root directory.
    QString fd;

    KURL u( url.data() );
    
    if ( window->getViewMode() == KFileWindow::ICON_VIEW )
	view->write( "<grid width=80>" );
    else if ( window->getViewMode() == KFileWindow::LONG_VIEW )
	view->write( "<table width=100%>" );
    else if ( window->getViewMode() == KFileWindow::TEXT_VIEW )
	view->write( "<table width=100%>" );
	
    if ( strcmp( u.path(), "/" ) != 0 )
    {
	KURL u2( url.data() );
	u2.cd("..");
	
	QString s = u2.url();
	if ( s.right( 1 ) != "/" )
	    s += "/";
    
	if ( window->getViewMode() == KFileWindow::ICON_VIEW )
	{
	    view->write( "<cell><a href=\"");
	    view->write( s.data() );
	    view->write( "\"><center><img src=\"file:" );
	    view->write( KFileType::findType( s.data() )->getPixmapFile( s.data() ) );
	    view->write( "\"><br>..</center><br></a></cell>" );
	}
	else if ( window->getViewMode() == KFileWindow::LONG_VIEW )
	{
	    view->write( "<tr><td><a href=\"" );
	    view->write( s.data() );
	    view->write( "\"><img src=\"file:" );
	    view->write( KFileType::findType( s.data() )->getPixmapFile( s.data() ) );
	    view->write( "\"></td><td>..</a></td>" );
	    view->write( "<td></td><td></td><td></td><td></td><td></td></tr>" );
	}
	else if ( window->getViewMode() == KFileWindow::TEXT_VIEW )
	{
	    view->write( "<tr><td><a href=\"" );
	    view->write( s.data() );
	    view->write( "\">../</td><td></a></td>" );
	    view->write( "<td></td><td></td><td></td><td></td></tr>" );
	}
    }
    
    KIODirectoryEntry *s;
    for ( s = files.first(); s != 0L; s = files.next() )
    {
	if ( window->getViewMode() == KFileWindow::ICON_VIEW )
	{ 
	    view->write( "<cell><a href=\"" );
	    
	    QString filename( url );
	    filename.detach();
	    filename += s->getName();
	    
	    view->write( filename.data() );
	    view->write( "\"><center><img src=\"file:" );
	    
	    view->write( KFileType::findType( filename.data() )->getPixmapFile( filename.data() ) );
	    
	    view->write( "\"><br>" );
	    view->write( s->getName() );
	    view->write( "</center><br></a></cell>" );
	}
	else if ( window->getViewMode() == KFileWindow::LONG_VIEW )
	{
	    view->write( "<tr><td><a href=\"" );
	    QString filename( url );
	    filename.detach();
	    filename += s->getName();
	    
	    view->write( filename.data() );
	    view->write( "\"><img width=16 height=16 src=\"file:" );
	    view->write( KFileType::findType( filename.data() )->getPixmapFile( filename.data() ) );
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
	else if ( window->getViewMode() == KFileWindow::TEXT_VIEW )
	{
	    view->write( "<tr><td><a href=\"" );
	    QString filename( url );
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

    if ( window->getViewMode() == KFileWindow::ICON_VIEW )
	view->write( "</grid></body></html>" );
    else if ( window->getViewMode() == KFileWindow::LONG_VIEW )
	view->write( "</table></body></html>" );
    else if ( window->getViewMode() == KFileWindow::TEXT_VIEW )
	view->write( "</table></body></html>" );

    printf("End\n");
    view->end();
    printf("Parse\n");
    view->parse();
    printf("Done\n");
}

void KFtpManager::slotPopupActivated( int _id )
{
    if ( popupMenu->text( _id ) == 0)
	return;
    
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
	KFileType::runBinding( s, popupMenu->text( _id ) );    
}

void KFtpManager::openPopupMenu( QStrList &_urls, const QPoint & _point )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	printf("Opening for '%s'\n",s);
	
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
    
    char buffer[ 1024 ];
    
    bool isdir = KIOServer::isDir( _urls );

    if ( isdir )
    {
	int id;
	id = popupMenu->insertItem( "Cd", window, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( "New View", window, SLOT( slotPopupNewView() ) );
	id = popupMenu->insertItem( "Delete",  window, SLOT( slotPopupDelete() ) );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( "Copy", window, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Delete",  window, SLOT( slotPopupDelete() ) );
    }

    window->setPopupFiles( _urls );
    popupFiles.copy( _urls );
    
    // Get all bindings matching all files.
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	// If this is the first file in the list, assume that all bindings are ok
	if ( s == _urls.getFirst() )
	{
	    KFileType::getBindings( bindings, s, isdir );
	}
	// Take only bindings, matching all files.
	else
	{
	    KFileType::getBindings( bindings2, s, isdir );
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
	popupMenu->insertItem( "Properties", window, SLOT( slotPopupProperties() ) );
    }
    
    popupMenu->popup( _point );
}

void KFtpManager::dropPopupMenu( KDNDDropZone *_zone, const char *_dest, const QPoint *_p )
{
    dropDestination = _dest;
    dropDestination.detach();
    
    dropZone = _zone;
    
    printf(" Drop with destination %s\n", _dest );
    
    // Perhaps an executable ?
    if ( !KIOServer::isDir( _dest ) )
    {
	printf("ERROR: Dont know what to do on ftp file\n");
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
	printf("ERROR: Can not accept drop\n");
	return;
    }
    
    popupMenu->popup( *_p );
}

void KFtpManager::slotDropCopy()
{
    KIOJob * job = new KIOJob;
    printf("COPY count=%i\n",dropZone->getURLList().count());
    
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

KHttpManager::KHttpManager( KFileWindow * _w, KFileView * _v )
    : KAbstractManager( _w, _v )
{
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );
    htmlCache = new HTMLCache();
    
    connect( htmlCache, SIGNAL( urlLoaded( const char*, const char *) ),
	     this, SLOT( slotShowHTML( const char*, const char* ) ) );
}

/*
 * Does not do any printing. Assumes that the _url is ok
 */
bool KHttpManager::openURL( const char *_url, bool _refresh )
{
    printf("Changing to http %s\n", _url );

    KURL u( _url );
    if ( u.isMalformed() )
    {
	printf("ERROR: Malformed URL\n");
	return FALSE;
    }
 
    printf("1\n");
    
    url = _url;
    url.detach();
        
    printf("2\n");

    view->begin( url.data() );
    view->write( "<html><head><title>HTML loading...</title></head><body>");
    view->write( "<h1>Loading ");
    view->write( _url );
    view->write( "</h1></body>" );
    view->end();
    view->parse();

    printf("3\n");

    if ( running )
      stop();
    
    htmlCache->slotURLRequest( url );
    running = TRUE;
    
    window->enableToolbarButton( 7, TRUE );
 
    printf("5\n");
    return TRUE;
}

void KHttpManager::slotShowHTML( const char *_url, const char *_filename )
{
    window->enableToolbarButton( 7, FALSE );
    running = FALSE;
    
    char buffer[ 1024 ];
    
    FILE *f = fopen( _filename, "rb" );
    if ( f == 0 )
    {
	printf("ERROR: Could not access '%s'\n",_filename );
	return;
    }

    view->begin( url.data() );
    
    char *s;
    do
    {
	s = fgets( buffer, 1023, f );
	if ( s != 0L )
	    view->write( buffer );
    } while ( s != 0L );

    fclose( f );
    
    view->end();
    view->parse();
} 

void KHttpManager::slotPopupActivated( int _id )
{
    // TODO
    /*
    if ( popupMenu->text( _id ) == 0)
	return;
    
    KFileType *typ = KFileType::findType( popup_file );
    KFileBind *bind;
    for ( bind = typ->firstBinding(); bind != 0L; bind = typ->nextBinding() )
    {
	if ( strcmp( bind->getProgram(), popupMenu->text( _id ) ) == 0 )
	{
	    printf( bind->getCmd(), ((const char*)popup_file)+6 );
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
    // TODO
    /*
      popupMenu->clear();
      // store the mouse position. (Matthias)
      popupMenuPosition = QCursor::pos();     

    bool isdir = KIOManager::getKIOManager()->isDir( _url );
    
    if ( isdir )
    {
	int id;
	id = popupMenu->insertItem( "Cd", window, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( "New View", window, SLOT( slotPopupNewView() ) );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( "Copy", window, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Cut",  window, SLOT( slotPopupCut() ) );	
    }
    
    QList<QString> bindings;
    bindings.setAutoDelete( TRUE );
    
    KFileType::getBindings( &bindings, _url, isdir );

    if ( !bindings.isEmpty() )
    {
	popupMenu->insertSeparator();

	QString *str;
	for ( str = bindings.first(); str != 0L; str = bindings.next() )
	{
	    popupMenu->insertItem( *str );
	}
    }
    
    window->setPopupFile( _url );
    popup_file = _url;
    
    popupMenu->popup( *_point );
    */
}

void KHttpManager::stop()
{
    if ( running )
    {
      running = FALSE;
      htmlCache->slotCancelURLRequest( url );
    }
    
    window->enableToolbarButton( 7, FALSE );
}

KHttpManager::~KHttpManager()
{
}

#include "kfmman.moc"
