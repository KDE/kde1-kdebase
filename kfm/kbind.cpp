#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>

#include <qmsgbox.h>
#include <qtstream.h>

#include "kbind.h"
#include "kfmwin.h"

QList<KFileType> *types;
// This types bindings apply to every file or directory which protocol matches
// the protocol of the binding.
KFileType *defaultType;
KFileType *kdelnkType;
KFileType *folderType;
KFileType *execType;
KFileType *batchType;

char KFileType::icon_path[ 1024 ];
char KFileType::executablePixmap[ 1024 ];
char KFileType::batchPixmap[ 1024 ];
char KFileType::defaultPixmap[ 1024 ];
char KFileType::folderPixmap[ 1024 ];

QStrList KFileBind::appList;

void KFileType::clearAll()
{
    KFileBind::clearApplicationList();
    
    delete types;
}

// A Hack, since KConfig has no constructor taking only a filename
KFMConfig::KFMConfig( QFile * _f, QTextStream *_s ) : KConfig( _s )
{
    f = _f;
    pstream = _s;
}

KFMConfig::~KFMConfig()
{
    delete f;
    delete pstream;
}


const char* KFolderType::getPixmapFile( const char *_url )
{
    pixmapFile2 = 0L;
    
    if ( strncmp( _url, "file:", 5 ) != 0 )
	return KFileType::getPixmapFile( _url );

    int len = strlen( _url );
    if ( ( len > 6 && strcmp( _url + len - 6, "/Trash" ) == 0 ) ||
	 ( len > 7 && strcmp( _url + len - 7, "/Trash/" ) == 0 ) )
    {
	pixmapFile2 = getIconPath();
	pixmapFile2.detach();
	pixmapFile2 += "/kfm_trash.xpm";
	return pixmapFile2.data();
    }
    
    QString n = _url + 5;
    if ( _url[ strlen( _url ) - 1 ] != '/' )
	n += "/";
    n += ".directory";

    FILE *fh = fopen( n.data(), "rb" );
    if ( fh == 0L )
	return KFileType::getPixmapFile( _url );
    
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, fh );
    fclose( fh );

    if ( strstr( buffer, "[KDE Desktop Entry]" ) == 0L )
	return KFileType::getPixmapFile( _url );

    QFile f( n.data() );
    if ( !f.open( IO_ReadOnly ) )
	return KFileType::getPixmapFile( _url );
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );

    QString icon = config.readEntry( "Icon" );
    if ( icon.isNull() )
	return KFileType::getPixmapFile( _url );

    pixmapFile2 = getIconPath();
    pixmapFile2.detach();
    pixmapFile2 += "/";
    pixmapFile2 += icon.data();
    
    return pixmapFile2.data();
}

QPixmap& KFolderType::getPixmap( const char *_url )
{
    getPixmapFile( _url );
    if ( pixmapFile2.isNull() )
	pixmap.load( pixmap_file );
    else
	pixmap.load( pixmapFile2 );
    return pixmap;
}

QString KFolderType::getComment( const char *_url )
{
    if ( strncmp( _url, "file:", 5 ) != 0 )
	return QString( "Folder" );
  
    QString n = _url + 5;
    if ( _url[ strlen( _url ) - 1 ] != '/' )
	n += "/";
    n += ".directory";

    QFile f( n.data() );
    if ( !f.open( IO_ReadOnly ) )
	return QString( "Folder" );
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    
    QString com = config.readEntry( "Comment" );

    if ( com.isNull() )
	com = "Folder";
    
    return QString( com.data() );
}

void KFileType::initApplications( const char * _path )
{
    DIR *dp;
    struct dirent *ep;
    dp = opendir( _path );
    if ( dp == NULL )
	return;
    
    // Loop thru all directory entries
    while ( ep = readdir( dp ) )
    {
	if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
	{
	    QString tmp = ep->d_name;	
    
	    QString file = _path;
	    file += "/";
	    file += ep->d_name;
	    struct stat buff;
	    stat( file.data(), &buff );
	    if ( S_ISDIR( buff.st_mode ) )
		initApplications( file.data() );
	    else if ( tmp.length() > 7 && tmp.right( 7 ) == ".kdelnk" )
	    {
		// The name of the application
		QString app = ep->d_name;
		if ( app.length() > 7 && app.right(7) == ".kdelnk" )
		    app = app.left( app.length() - 7 );

		KFileBind::appendApplication( app.data() );
		
		QFile f( file.data() );
		if ( !f.open( IO_ReadOnly ) )
		    return;
		
		QTextStream pstream( &f );
		KConfig config( &pstream );

		config.setGroup( "KDE Desktop Entry" );
		QString exec = config.readEntry( "Exec" );
		// An icon for the binary
		QString app_icon = config.readEntry( "Icon" );
		// The pattern to identify the binary
		QString app_pattern = config.readEntry( "BinaryPattern" );
		QString comment = config.readEntry( "Comment" );
		// A ';' separated list of extension groups
		QString exts = config.readEntry( "Extensions" );
		// A ';' separated list of protocols
		QString protocols = config.readEntry( "Protocols" );
		
		// Define an icon for the program file perhaps ?
		if ( !app_icon.isNull() && !app_pattern.isNull() )
		{
		    printf("Installing binary pattern '%s' '%s'\n",app_pattern.data(),comment.data());
		    KFileType *t;
		    types->append( t = new KFileType( app.data(), app_icon.data() ) );
		    t->setComment( comment.data() );
		    t->setApplicationPattern();
		    int pos2 = 0;
		    int old_pos2 = 0;
		    while ( ( pos2 = app_pattern.find( ";", pos2 ) ) != - 1 )
		    {
			QString pat = app_pattern.mid( old_pos2, pos2 - old_pos2 );
			t->addPattern( pat.data() );
			pos2++;
			old_pos2 = pos2;
		    }
		} 
		    
		// Which protocols, e.g. file, http, tar, ftp, does the
		// application support ?
		int pos2 = 0;
		int old_pos2 = 0;
		QString prots[5];
		int cp = 0;
		while ( ( pos2 = protocols.find( ";", pos2 ) ) != - 1 )
		{
		    QString prot = protocols.mid( old_pos2, pos2 - old_pos2 );
		    if ( cp <= 4 )
		    {
			prots[ cp ] = prot.data();
			prots[ cp++ ].detach();
		    }
		    else
		    {
			QMessageBox::message( "Error", "Too many protocols in file\n" + file );
			pos2 = protocols.length();
		    }
		    
		    pos2++;
		    old_pos2 = pos2;
		}
		
		// To which extensions is the application bound ?
		pos2 = 0;
		old_pos2 = 0;
		while ( ( pos2 = exts.find( ";", pos2 ) ) != - 1 )
		{
		    QString bind = exts.mid( old_pos2, pos2 - old_pos2 );
		    // Bind this application to all files/directories
		    if ( strcasecmp( bind.data(), "all" ) == 0 )
		    {
			defaultType->append( new KFileBind( app.data(), exec.data(), prots[0].data(),
							    prots[1].data(), prots[2].data(),
							    prots[3].data(), prots[4].data()) );
			folderType->append( new KFileBind( app.data(), exec.data(), prots[0].data(),
							   prots[1].data(), prots[2].data(),
							   prots[3].data(), prots[4].data() ) );
		    }
		    else if ( strcasecmp( bind.data(), "alldirs" ) == 0 )
		    {
			folderType->append( new KFileBind( app.data(), exec.data(), prots[0].data(),
							   prots[1].data(), prots[2].data(),
							   prots[3].data(), prots[4].data() ) );
		    }
		    else if ( strcasecmp( bind.data(), "allfiles" ) == 0 )
		    {
			defaultType->append( new KFileBind( app.data(), exec.data(), prots[0].data(),
							   prots[1].data(), prots[2].data(),
							   prots[3].data(), prots[4].data() ) );
		    }
		    // Bind this application to an extension group
		    else
		    {
			KFileType *t = KFileType::findByName( bind.data() );
			if ( t == 0 )
			    QMessageBox::message( "ERROR", "Could not find file type\n" + bind + "\nin " + file );
			
			t->append( new KFileBind( app.data(), exec.data(), prots[0].data(),
						  prots[1].data(), prots[2].data(),
						  prots[3].data(), prots[4].data() ) );
		    }
		    
		    pos2++;
		    old_pos2 = pos2;
		} 
		
		f.close();
	    }
	
	}
    }
    (void) closedir( dp );
}

void KFileType::initFileTypes( const char* _path )
{   
    DIR *dp;
    struct dirent *ep;
    dp = opendir( _path );
    if ( dp == NULL )
	return;
    
    // Loop thru all directory entries
    while ( ep = readdir( dp ) )
    {
	if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
	{
	    QString tmp = ep->d_name;

	    QString file = _path;
	    file += "/";
	    file += ep->d_name;
	    struct stat buff;
	    stat( file.data(), &buff );
	    if ( S_ISDIR( buff.st_mode ) )
		initFileTypes( file.data() );
	    else if ( tmp.length() > 7 && tmp.right( 7 ) == ".kdelnk" )
	    {
		QFile f( file.data() );
		if ( !f.open( IO_ReadOnly ) )
		    return;
		
		QTextStream pstream( &f );
		KConfig config( &pstream );
		config.setGroup( "KDE Desktop Entry" );
		
		// Read a new extension group
		QString ext = ep->d_name;
		if ( ext.length() > 7 && ext.right(7) == ".kdelnk" )
		    ext = ext.left( ext.length() - 7 );
		
		// Get a ';' separated list of all pattern
		QString pats = config.readEntry( "Patterns" );
		QString icon = config.readEntry( "Icon" );
		QString defapp = config.readEntry( "DefaultApp" );
		QString comment = config.readEntry( "Comment" );
		
		// Is this file type already registered ?
		KFileType *t = KFileType::findByName( ext.data() );
		// If not then create a new type, but only if we have an icon
		if ( t == 0L && !icon.isNull() )
		    types->append( t = new KFileType( ext.data(), icon.data() ) );
		// If we have this type already we perhaps only change the pixmap ?
		else if ( !icon.isNull )
		    t->setPixmap( icon.data() );
		// Set the default binding
		if ( !defapp.isNull() && t != 0L )
		    t->setDefaultBinding( defapp.data() );
		if ( t != 0L )
		    t->setComment( comment.data() );
		
		int pos2 = 0;
		int old_pos2 = 0;
		while ( ( pos2 = pats.find( ";", pos2 ) ) != - 1 )
		{
		    // Read a pattern from the list
		    QString name = pats.mid( old_pos2, pos2 - old_pos2 );
		    if ( t != 0L )
			 t->addPattern( name.data() );
		    pos2++;
		    old_pos2 = pos2;
		}
		
		f.close();
	    }
	}
    }
}

void KFileType::init()
{
    QString ipath( getenv("KDEDIR") );
    if ( ipath.isNull() )
    {
	printf("ERROR: You did not set $KDEDIR\n");
	exit(2);
    }
    ipath.detach();
    ipath += "/lib/pics";
    strcpy( icon_path, ipath.data() );
    
    KFileType *ft;

    kdelnkType = new KDELnkFileType();

    // Read some informations about standard pixmaps.
    KConfig *config = KApplication::getKApplication()->getConfig();

    // Read the deault icons
    QString icon;
    
    execType = new KFileType();
    icon = config->readEntry( "Executable" );
    if ( icon.isNull() )
    {
	execType->setPixmap( "exec.xpm" );
        strcpy( executablePixmap, "exec.xpm" );
    }
    else
    {
	execType->setPixmap( icon.data() );
        strcpy( executablePixmap, icon.data() );
    }
    execType->setComment( "Executable" );
    
    batchType = new KFileType();
    icon = config->readEntry( "Batch" );
    if ( icon.isNull() )
    {
	strcpy( batchPixmap, "terminal.xpm" );
	batchType->setPixmap( "terminal.xpm" );
    }
    else
    {
	batchType->setPixmap( icon.data() );
	strcpy( batchPixmap, icon.data() );
    }
    batchType->setComment( "Shell Script" );
    
    defaultType = new KFileType();
    icon = config->readEntry( "Default" );
    if ( icon.isNull() )
    {
	strcpy( defaultPixmap, "unknown.xpm" );
	defaultType->setPixmap( "unknown.xpm" );
    }
    else
    {
	strcpy( defaultPixmap, icon.data() );
	defaultType->setPixmap( icon.data() );
    }
    
    folderType = new KFolderType();
    icon = config->readEntry( "Folder" );
    if ( icon.isNull() )
    {
	strcpy( folderPixmap, "folder.xpm" );
	folderType->setPixmap( "folder.xpm" );
    }
    else
    {
	strcpy( folderPixmap, icon.data() );
    	folderType->setPixmap( icon.data() );
    }
    
    types = new QList<KFileType>;
    types->setAutoDelete( TRUE );
    
    // Read the application bindings
    QString path = getenv( "KDEDIR" );
    path += "/filetypes";
    initFileTypes( path.data() );

    // Read the application bindings
    path = getenv( "KDEDIR" );
    path += "/apps";
    initApplications( path.data() );
}

const char* KDELnkFileType::getPixmapFile( const char *_url )
{
    // Try to read the file as a [KDE Desktop Entry]
    KFMConfig *config = KFileType::openKFMConfig( _url );
    
    if ( config != 0L )
    {
	QString typ = config->readEntry( "Type" );
	if ( typ == "FSDevice" )
	{
	    QString dev = config->readEntry( "Dev" );
	    QString icon = config->readEntry( "Icon" );	    
	    QString icon2 = config->readEntry( "UnmountIcon" );	    
	    delete config;
	    
	    if ( !dev.isNull() && !icon.isNull() && !icon2.isNull() )
	    {
		QString mp = KIOServer::findDeviceMountPoint( dev.data() );
		pixmap_file = getIconPath();
		pixmap_file.detach();
		pixmap_file += "/";
		if ( mp.isNull() )
		    pixmap_file += icon2.data();
		else
		    pixmap_file += icon.data();
		return pixmap_file.data();
	    }
	}
	else
	{
	    QString icon = config->readEntry( "Icon" );
	    delete config;
	    
	    if ( !icon.isNull() )
	    {
		pixmap_file = getIconPath();
		pixmap_file.detach();
		pixmap_file += "/";
		pixmap_file += icon.data();
		return pixmap_file.data();
	    }
	}
    }
    
    // The default behavior.
    pixmap_file = getIconPath();
    pixmap_file.detach();
    pixmap_file += "/unknown.xpm";
    return pixmap_file.data();
}

QString KDELnkFileType::getComment( const char *_url )
{
    KFMConfig *config = KFileType::openKFMConfig( _url );
    
    if ( config == 0L )
	return QString();
    
    QString erg = config->readEntry( "Comment" );
    return QString( erg.data() );
}

QPixmap& KDELnkFileType::getPixmap( const char *_url )
{
    getPixmapFile( _url );
    pixmap.load( pixmap_file );
    return pixmap;
}

KFileType* KFileType::getFirstFileType()
{
    return types->first();
}

KFileType* KFileType::getNextFileType()
{
    return types->next();
}

KFileType* KFileType::findByPattern( const char *_pattern )
{
    KFileType *typ;
    
    for ( typ = types->first(); typ != 0L; typ = types->next() )
    {
	QStrList & pattern = typ->getPattern();
	char *s;
	for ( s = pattern.first(); s != 0L; s = pattern.next() )
	    if ( strcmp( s, _pattern ) == 0 )
		return typ;
    }

    return 0L;
}

KFileType* KFileType::findByName( const char *_name )
{
    KFileType *typ;
    
    for ( typ = types->first(); typ != 0L; typ = types->next() )
    {
	if ( strcmp( typ->getName(), _name ) == 0 )
	    return typ;
    }

    return 0L;
}

KFileType* KFileType::findType( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return defaultType;

    // Used to store a modified value for _url.
    QString tmp;
    
    // Some of the *.kdelnk files have special hard coded bindings like
    // mouting/unmounting of devices.
    tmp = _url;
    if ( tmp.length() > 7 && tmp.right(7) == ".kdelnk" )
	return kdelnkType;
    
    if ( KIOServer::isDir( _url ) )
	return folderType;
    
    KFileType *typ;

    // Links may appear on the local hard disk only. If this is a link
    // we will use the file/dir the link is pointin to to determine
    // the file type.
    if ( strcmp( u.protocol(), "file" ) == 0 )
    {
	// Is it a link?
	struct stat lbuff;
	char buffer[ 1024 ];
	lstat( u.path(), &lbuff );
	    
	// If we have a link, we want to have the icon matching
	// the filename to which the link is pointing.
	if ( S_ISLNK( lbuff.st_mode ) )
	{
	    QDir d( u.path() );
	    d.cd("..");
	    int n = readlink( u.path(), buffer, 1022 );
	    if ( n > 0 )
	    {
		buffer[ n ] = 0;
		// Is it a directory ?
		if ( !d.cd( buffer ) )
		{
		    QFileInfo fi( d, buffer );
		    tmp = "file:";
		    tmp += fi.absFilePath().data();
		}
		else
		{
		    tmp = "file:";
		    tmp += d.absPath().data();
		}
		// Change the URL 
		_url = tmp.data();
		KURL u2( _url );
		u = u2;
	    }
	}
    }

    for ( typ = types->first(); typ != 0L; typ = types->next() )
    {
	QStrList& pattern = typ->getPattern();
	char *s;
	for ( s = pattern.first(); s != 0L; s = pattern.next() )
	{
	    int pattern_len = strlen( s );
	    const char *filename;
	    if ( strcmp( u.protocol(), "tar" ) == 0 )
		filename = u.filename( TRUE );
	    else
		filename = u.filename();
	    int filename_len = strlen( filename );	

	    if ( s[ pattern_len - 1 ] == '*' && filename_len + 1 >= pattern_len )
	    {
		if ( strncmp( filename, s, pattern_len - 1 ) == 0 )
		    return typ;
	    }
	    if ( s[ 0 ] == '*' )
		if ( strncmp( filename + filename_len - pattern_len + 1, s + 1, pattern_len - 1 ) == 0 )
		    return typ;
	    if ( strcmp( filename, s ) == 0 )
		return typ;
	}
    }

    // Since mounted ms file systems often set the executable flag without
    // providing real executables, we first check the extension before looking
    // at the flags.
    // Executable ? Must be on the local drive.
    if ( strncmp( _url, "file:/", 6 ) == 0 )
    {
	struct stat buff;
	stat( _url + 5, &buff );
	if ( ( buff.st_mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) != 0 )
	{
	    FILE *f = fopen( _url + 5, "rb" );
	    if ( f == 0 )
		return execType;
	    char buffer[ 10 ];
	    int n = fread( buffer, 1, 2, f );
	    fclose( f );
	    buffer[ n ] = 0;
	    // Is it a batchfile
	    if ( strcmp( buffer, "#!" ) == 0 )
		return batchType;
	    // It is a binary executable
	    return execType;
	}
    }

    return defaultType;
}

void KFileType::getBindings( QStrList &_list, const char *_url, bool _isdir )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return;
    
    // Used to store a new value for _url
    QString tmp;

    
    // Try to read the file as a [KDE Desktop Entry]
    KFMConfig *config = KFileType::openKFMConfig( _url );
    
    if ( config != 0L )
    {
	QString typ = config->readEntry( "Type" );
	if ( typ == "FSDevice" )
	{
	    QString dev = config->readEntry( "Dev" );
	    QString fstype = config->readEntry( "FSType" );
	    if ( !dev.isNull() && !fstype.isNull() )
	    {
		QString mp = KIOServer::findDeviceMountPoint( dev.data() );
		if ( mp.isNull() )
		{
		    char buff[ 1024 ];
		    strcpy( buff, fstype.data() );
		    char *p = buff;
		    while ( *p != ';' && *p != 0 )
		    {
			char *start = p;
			while ( *p != ';' && *p != 0 && *p != ',' )
			    p++;
			if ( *p == ';' )
			    *p = 0;
			else if ( *p == ',' )
			{
			    *p = 0;
			    p++;
			}
			
			QString s( "Mount " );
			s.detach();
			s += (const char*)start;
			_list.append( s.data() );   
		    }			
		}
		else
		{
		    QString s( "Unmount" );
		    s.detach();
		    _list.append( s.data() );
		}
	    }
	}
	else if ( typ == "Link" )
	{
	    QString url = config->readEntry( "URL" );
	    if ( !url.isNull() )
	    {
		tmp = url.data();
		tmp.detach();
		_url = tmp.data();
		printf("Changed to '%s'\n",_url);
		KURL u2( _url );
		u = u2;
	    }
	}
	delete config;
    }

    // If it is a link, get the name the link is pointing to
    if ( strcmp( u.protocol(), "file" ) == 0 )
    {
	QDir d( u.path() );
	QString x = d.canonicalPath();
	if ( !x.isNull )
	{
	    tmp = x.data();
	    tmp.detach();
	    _url = tmp.data();
	    printf("$$$$$$$$$ Changed to '%s'\n",_url);
	    KURL u2( _url );
	    u = u2;
	}
    }

    // A directory named dir.html for example does not have a binding to
    // netscape or arena.
    if ( _isdir )
    {
	KFileBind *bind;
	for ( bind = folderType->firstBinding(); bind != 0L; bind = folderType->nextBinding() )
	{
	    // Does the application support the protocol ?
	    if ( bind->supportsProtocol( u.protocol() ) )
		_list.append( bind->getProgram() );
	}
    }
    // usual files
    else
    {
	KFileType *typ = KFileType::findType( _url );
	if ( !typ->hasBindings() )
	    return;

	KFileBind *bind;
	for ( bind = typ->firstBinding(); bind != 0L; bind = typ->nextBinding() )
	{
	    if ( bind->supportsProtocol( u.protocol() ) )
		_list.append( bind->getProgram() );
	}

	// Add all default bindings if we did not already do it
	if ( typ != defaultType )
	{
	    for ( bind = defaultType->firstBinding(); bind != 0L; bind = defaultType->nextBinding() )
	    {
		// Does the application support the protocol ?
		if ( bind->supportsProtocol( u.protocol() ) )
		    _list.append( bind->getProgram() );
	    }
	}
    }
}

		
void KFileType::runBinding( const char *_url )
{    
    // KFM must execute *.kdelnk. So dont look for bindings here.
    // Perhaps this function is called again from within ::runBinding( .. ,.. ).
    // Have a look at the code for type "URL".
    QString u = _url;
    if ( u.length() > 7 && u.right( 7 ) == ".kdelnk" )
    {
	KFileType::runBinding( _url, "Open" );
	return;
    }
    
    // Is there only one binding ?
    QStrList bindings;
    bindings.setAutoDelete( TRUE );
    KFileType::getBindings( bindings, _url, FALSE );    
    // Any bindings available
    if ( bindings.isEmpty() )
    {
	// Try the default binding named "Open"
	KFileType::runBinding( _url, "Open" );
	return;
    }
    
    // Only one binding ?
    if ( bindings.count() == 1 )
    {
	KFileType::runBinding( _url, bindings.first() );    
	return;
    }
    // Find default binding
    KFileType *typ = KFileType::findType( _url );
    if ( typ->getDefaultBinding() != 0 )
    {
	KFileType::runBinding( _url, typ->getDefaultBinding() );    
	return;
    }
    // Take first binding
    KFileType::runBinding( _url, bindings.first() );
}


void KFileType::runBinding( const char *_url, const char *_binding, QStrList * _arguments )
{
    if ( _binding == 0L )
	_binding = "";

    printf("Binding is %s\n",_binding);

    KURL u( _url );
    if ( u.isMalformed() )
	return;

    // Is it an executable ?
    if ( strcmp( u.protocol(), "file" ) == 0 && strcasecmp( _binding, "Open" ) == 0 )
    {
	struct stat buff;
	stat( u.path(), &buff );
	if ( ( buff.st_mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) != 0 )
	{
	    printf("Executing '%s'\n",u.path());
	    
	    QString cmd = u.path();
	    cmd.detach();
	    cmd += " ";

	    if ( _arguments != 0L )
	    {
		char *s;
		for ( s = _arguments->first(); s != 0L; s = _arguments->next() )
		{
		    KURL su( s );
		    cmd += "\"";
		    if ( strcmp( u.protocol(), "file" ) == 0 )
			cmd += su.path();
		    else
			cmd += s;
		    cmd += "\" ";
		}
	    }
	    
	    cmd += "&";
	    system( cmd.data() );
	    return;
	}
    }
    
    // Used to store a modified value for _url
    QString tmp;
    
    printf("Testing for KDE Desktop Entry\n");
    
    KFMConfig *config = 0L;
    // Is it a "[KDE Desktop Entry]" file and do we want to open it ?
    // if ( strcasecmp( _binding, "Open" ) == 0 )
	config = KFileType::openKFMConfig( _url );
    
    if ( config != 0L )
    {
	printf("################### Is a KDE Desktop Entry file\n");
	QString typ = config->readEntry( "Type" );
	QString exec = config->readEntry( "Exec" );
	// Must we start an executable ?
	if ( !exec.isNull() )
	{
	    QString term = config->readEntry( "Terminal" );
	    QString termOptions = config->readEntry( "TerminalOptions" );

	    QString f = "";
	    QString u = "";
	    QString n = "";
	    QString d = "";
	    if ( _arguments != 0L )
	    {
		char *s;
		for ( s = _arguments->first(); s != 0L; s = _arguments->next() )
		{
		    KURL su( s );
		    f += "\"";
		    f += su.path();
		    f += "\" ";
		    u += "\"";
		    u += s;
		    u += "\" ";
		    QString tmp = su.path();
		    // Is it a directory ? Then strip the "/"
		    if ( tmp.right(1) == "/" )
			tmp = tmp.left( tmp.length() - 1 );
		    int i = tmp.findRev( "/" );
		    if ( i != -1 )
		    {
			n += "\"";
			n += tmp.mid( i + 1, tmp.length() );
			n += "\" ";
			d += "\"";
			d += tmp.left( i + 1 );
			d += "\" ";
		    }
		}
	    }

	    int i;
	    while ( ( i = exec.find( "%f" ) ) != -1 )
		exec.replace( i, 2, f.data() );
	    while ( ( i = exec.find( "%u" ) ) != -1 )
		exec.replace( i, 2, u.data() );
	    while ( ( i = exec.find( "%n" ) ) != -1 )
		exec.replace( i, 2, n.data() );
	    while ( ( i = exec.find( "%d" ) ) != -1 )
		exec.replace( i, 2, d.data() );
	    while ( ( i = exec.find( "%k" ) ) != -1 )
		exec.replace( i, 2, _url );
	    
	    if ( term == "1" )
	    {
		KConfig *config = KApplication::getKApplication()->getConfig();
		config->setGroup( "Terminal" );
		QString cmd = config->readEntry( "Terminal" );
		cmd.detach();
		if ( cmd.isNull() )
		{
		    printf("ERROR: No Terminal Setting\n");
		    return;
		}
		cmd += " ";
		if ( !termOptions.isNull() )
		{
		    cmd += termOptions.data();
		    cmd += " ";
		}
		cmd += "-e ";
		cmd += exec.data();
		cmd += " &";
		printf("Running '%s'\n",cmd.data());
		system( cmd.data() );
	    }
	    else
	    {
		QString cmd = exec.data();
		cmd += " &";
		printf("Running '%s'\n",cmd.data());
		system( cmd.data() );
	    }
	}
	// Is it a device like CDROM or Floppy ?
	else if ( typ == "FSDevice" && strcmp( u.protocol(), "file" ) == 0 )
	{
	    QString point = config->readEntry( "MountPoint" );
	    QString dev = config->readEntry( "Dev" );
	    if ( !point.isNull() && !dev.isNull() )
	    {
		if ( strcmp( _binding, "Unmount" ) == 0 )
		{
		    KIOJob * job = new KIOJob();
		    job->unmount( point.data() );
		    delete config;
		    return;
		}
		else if ( strncmp( _binding, "Mount ", 6 ) == 0 )
		{
		    QString readonly = config->readEntry( "ReadOnly" );
		    bool ro;
		    if ( !readonly.isNull() )
			if ( readonly == '1' )
			    ro = TRUE;
			 
		    KIOJob *job = new KIOJob;
		   
		    // The binding is names 'Mount FSType' => +6
		    if ( strcasecmp( _binding + 6, "default" ) == 0 )
			job->mount( FALSE, 0L, dev, 0L );
		    else
			job->mount( ro, _binding + 6, dev, point );
		    delete config;
		    return;
		}
		else if ( strcmp( _binding, "Open" ) == 0 )
		{
		    QString mp = KIOServer::findDeviceMountPoint( dev.data() );
		    if ( !mp.isNull() )
		    {
			QString mp2 = "file:";
			mp2 += mp;
			printf("KDE: Opening %s\n",mp2.data());
			KFileWindow *m = new KFileWindow( 0L, 0L, mp2.data() );
			m->show();
		    }
		    else
			printf("ERROR: You must first mount the device. Use the right mouse button\n");
		    delete config;
		    return;
		}
		else
		    printf("ERROR: Unknown Binding '%s'\n",_binding);
	    }
	}
	else if ( strcmp( typ, "Link" ) == 0 )
	{
	    QString url = config->readEntry( "URL" );
	    delete config;
	    if ( !url.isNull() )
	    {
		printf("######################## It is a link\n");
		// Can KFM handle such an ULR ? Does the user wish the default binding => _binding == "open" ?
		if ( ( strncasecmp( url.data(), "tar:", 4 ) == 0 || strncasecmp( url.data(), "http:", 5 ) == 0 ||
		     strncasecmp( url.data(), "file:", 5 ) == 0 || strncasecmp( url.data(), "ftp:", 4 ) == 0 ) &&
		     strcasecmp( _binding, "open" ) == 0 )
		{
		    KFileWindow *m = new KFileWindow( 0L, "3", url.data() );
		    m->show();
		}
		// Call runBinding( .. ) again with the URL as parameter.
		// Perhaps we have an external program for such URLs
		else
		    KFileType::runBinding( url.data() );
		return;
	    }
	}
    }
    
    KFileType *typ = KFileType::findType( _url );
    KFileBind *bind;
    for ( bind = typ->firstBinding(); bind != 0L; bind = typ->nextBinding() )
    {
	printf("!!! '%s' vs '%s'\n",bind->getProgram(), _binding );
	if ( strcmp( bind->getProgram(), _binding ) == 0 )
	{
	    printf("!!! '%s' == '%s'\n",bind->getProgram(), _binding );
	    printf("!!! '%s'\n",bind->getCmd());
	    
	    QString f;
	    QString ur;
	    QString n = "";
	    QString d = "";
	    f.sprintf( "\"%s\"", u.path() );
	    ur.sprintf( "\"%s\"", _url );
	    QString tmp = u.path();
	    // Is it a directory ? Then strip the "/"
	    if ( tmp.right(1) == "/" )
		tmp = tmp.left( tmp.length() - 1 );
	    int i = tmp.findRev( "/" );
	    if ( i != -1 )
	    {
		n = "\"";
		n += tmp.mid( i + 1, tmp.length() );
		n += "\"";
		d += "\"";
		d += tmp.left( i + 1 );
		d += "\"";
	    }

	    QString cmd = bind->getCmd();
	    cmd.detach();
	    int pos;
	    while ( ( pos = cmd.find( "%f" )) != -1 )
		cmd.replace( pos, 2, f.data() );
	    while ( ( pos = cmd.find( "%u" )) != -1 )
		cmd.replace( pos, 2, ur.data() );
	    while ( ( pos = cmd.find( "%n" )) != -1 )
		cmd.replace( pos, 2, n.data() );
	    while ( ( pos = cmd.find( "%d" )) != -1 )
		cmd.replace( pos, 2, d.data() );

	    cmd += " &";
	    printf("::CMD = %s\n",cmd.data());
	    system( cmd.data() );
	    return;
	}
    }

    // If we still dont know what to do, perhaps we can open a new window with the URL ?
    if ( KIOServer::isDir( _url ) && strcasecmp( _binding, "open" ) == 0 )
    {
	KFileWindow * m = new KFileWindow( 0L, 0L, _url );
	m->show();
    }
}

KFMConfig* KFileType::openKFMConfig( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return 0L;
    if ( strcmp( u.protocol(), "file" ) != 0 )
	return 0L;
    
    FILE *f;
    // _url must be of type 'file:/'
    f = fopen( u.path(), "rb" );
    if ( f == 0L )
	return 0L;
    
    char buff[ 1024 ];
    buff[ 0 ] = 0;
    fgets( buff, 1023, f );
    if ( strstr( buff, "[KDE Desktop Entry]" ) == 0L )
	return 0L;
    
    fclose( f );
    
    QFile *file = new QFile( u.path() );
    if ( !file->open( IO_ReadOnly ) )
	return 0L;
    
    QTextStream *pstream = new QTextStream( file );
    KFMConfig *config = new KFMConfig( file, pstream );
    config->setGroup( "KDE Desktop Entry" );
    return config;
}

KFileType::KFileType( const char *_name, const char *_pixmap )
{
    bApplicationPattern = FALSE;
    
    name = _name;
    pixmap_file = getIconPath();
    pixmap_file += "/";
    pixmap_file.detach();
    pixmap_file += _pixmap;

    HTMLImage::cacheImage( pixmap_file.data() );
}

void KFileType::append( KFileBind *_bind )
{
    bindings.append( _bind );
}

QPixmap& KFileType::getPixmap( const char *_url )
{
    if ( pixmap.isNull() )
	pixmap.load( pixmap_file );

    return pixmap;
}

const char* KFileType::getPixmapFile( const char *_url )
{
    return pixmap_file.data();
}

void KFileType::setPixmap( const char *_file )
{
    pixmap_file = getIconPath();
    pixmap_file.detach();
    pixmap_file += "/";
    pixmap_file += _file;

    HTMLImage::cacheImage( pixmap_file.data() );
}


KFileBind::KFileBind( const char *_prg, const char *_cmd, const char *_prot1, 
		      const char *_prot2, const char *_prot3,
		      const char *_prot4, const char *_prot5 )
{
    program = _prg;
    program.detach();
  
    cmd = _cmd;
    cmd.detach();
    
    protocol1 = _prot1;
    protocol1.detach();
    
    if ( _prot2 != 0 )
    {
	protocol2 = _prot2;
	protocol2.detach();
    }
    else
	protocol2 = "";
    
    if ( _prot3 != 0 )
    {
	protocol3 = _prot3;
	protocol3.detach();
    }
    else
	protocol3 = "";

    if ( _prot4 != 0 )
    {
        protocol4 = _prot4;
	protocol4.detach();
    }
    else
	protocol4 = "";

    if ( _prot5 != 0 )
    {
	protocol5 = _prot5;
	protocol5.detach();
    }
    else
	protocol5 = "";
}

bool KFileBind::supportsProtocol( const char *_protocol )
{
    if ( strcmp( protocol1, _protocol ) == 0 )
	return TRUE;
    if ( strcmp( protocol2, _protocol ) == 0 )
	return TRUE;
    if ( strcmp( protocol3, _protocol ) == 0 )
	return TRUE;
    if ( strcmp( protocol4, _protocol ) == 0 )
	return TRUE;
    if ( strcmp( protocol5, _protocol ) == 0 )
	return TRUE;

    return FALSE;
}
