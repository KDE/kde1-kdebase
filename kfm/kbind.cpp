#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include <qmsgbox.h>
#include <qtstream.h>

#include <kurl.h>

#include "kbind.h"
#include "kfmpaths.h"
#include "kfmgui.h"
#include <config-kfm.h>

#include <klocale.h>

KMimeMagic* KMimeType::magic = 0L;

QList<KMimeType> *types;
// This types bindings apply to every file or directory which protocol matches
// the protocol of the binding.
KMimeType *defaultType;
KMimeType *kdelnkType;
KMimeType *folderType;
KMimeType *execType;
KMimeType *batchType;
KMimeType *lockedfolderType;
KMimeType *PipeType;
KMimeType *SocketType;
KMimeType *CDevType;
KMimeType *BDevType;   

char KMimeType::icon_path[ 1024 ];

QStrList KMimeBind::appList;

// A Hack, since KConfig has no constructor taking only a filename
KFMConfig::KFMConfig( QFile * _f, QTextStream *_s ) : KConfig( _s )
{
    f = _f;
    pstream = _s;
}

KFMConfig::~KFMConfig()
{
    f->close();
    delete f;
    delete pstream;
}

const char* KFolderType::getPixmapFile( const char *_url )
{
    pixmapFile2 = 0L;

    QString decoded( _url );
    KURL::decodeURL( decoded );

    // Is this a file located in a tar archive ?
    if ( strstr( _url, "tar:" ) != 0L )
    {
	KURL u( _url );
	if ( !u.isMalformed() )
	{
	    // Get the last subprotocol
	    QString child = u.childURL();
	    // Do we really have some subprotocol ?
	    if ( !child.isEmpty() )
	    {
		KURL u2( child );
		// is "tar" the last subprotocol ?
		if ( strcmp( u2.protocol(), "tar" ) == 0 )
		{
		    // We return an folder icon. Otherwise
		    // we would get a folder icon with access denied!!!
		    return folderType->getPixmapFile();
		    /* pixmapFile2 = getIconPath();
		    pixmapFile2.detach();
		    pixmapFile2 += "/folder.xpm"; 
		    return pixmapFile2.data(); */
		}
	    }
	}
    }
    
    if ( strncmp( _url, "file:", 5 ) != 0 )
	return KMimeType::getPixmapFile( _url );

    /* Stephan: this is Torben's code. It allows many (readonly) Trash bins 
    int len = strlen( _url );
    if ( ( len > 6 && strcmp( _url + len - 6, "/Trash" ) == 0 ) ||
	 ( len > 7 && strcmp( _url + len - 7, "/Trash/" ) == 0 ) )
      */
    QString path = decoded;
    if ( path.right(1) != "/" )
       path += "/";
    QString path2 = QString("file:") + KFMPaths::TrashPath();
    
    if ( path == path2 )
    {
        DIR *dp;
        struct dirent *ep;
        dp = opendir( decoded.data() + 5 );
	// Stephan: Congratulation Torben for this trick :-)
	// Torben: Thanks!
        ep=readdir( dp );
        ep=readdir( dp );      // ignore '.' and '..' dirent
        if ( readdir( dp ) == 0L ) // third file is NULL entry -> empty directory
        {
           pixmapFile2 = getIconPath();
           pixmapFile2.detach();
           pixmapFile2 += "/kfm_trash.xpm";
	   closedir( dp );
	   return pixmapFile2.data();
         }
         else
         {
	   pixmapFile2 = getIconPath();
	   pixmapFile2.detach();
	   pixmapFile2 += "/kfm_fulltrash.xpm";
	   closedir( dp );
	   return pixmapFile2.data();
         }
    }
    
    QString n = decoded.data() + 5;
    if ( _url[ decoded.length() - 1 ] != '/' )
	n += "/";
    n += ".directory";

    /* FILE *fh = fopen( n.data(), "rb" );
    if ( fh == 0L )
	return KMimeType::getPixmapFile( _url );
    
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, fh );
    fclose( fh );

    if ( strstr( buffer, "[KDE Desktop Entry]" ) == 0L )
	return KMimeType::getPixmapFile( _url ); */

    QFile f( n.data() );
    if ( !f.open( IO_ReadOnly ) )
	return KMimeType::getPixmapFile( _url );
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );

    QString icon = config.readEntry( "Icon" );
    if ( icon.isEmpty() )
	return KMimeType::getPixmapFile( _url );

    /* pixmapFile2 = getIconPath();
    pixmapFile2.detach();
    pixmapFile2 += "/";
    pixmapFile2 += icon.data(); */
    pixmapFile2 = pixmap_file.data();
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
	return QString( klocale->translate("Folder") );
  
    QString decoded( _url );
    KURL::decodeURL( decoded );
    
    QString n = decoded.data() + 5;
    if ( _url[ decoded.length() - 1 ] != '/' )
	n += "/";
    n += ".directory";

    QFile f( n.data() );
    if ( !f.open( IO_ReadOnly ) )
	return QString( klocale->translate("Folder") );
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    
    QString com = config.readEntry( "Comment" );

    if ( com.isNull() )
	com = klocale->translate("Folder");
    
    return QString( com.data() );
}

const char* KDELnkMimeType::getPixmapFile( const char *_url )
{
    // Try to read the file as a [KDE Desktop Entry]
    KFMConfig *config = KMimeBind::openKFMConfig( _url );
    
    if ( config != 0L )
    {
	QString typ = config->readEntry( "Type" );
	if ( typ == "FSDevice" )
	{
	    QString dev = config->readEntry( "Dev" );
	    QString icon = config->readEntry( "Icon" );	    
	    QString icon2 = config->readEntry( "UnmountIcon" );	    
	    delete config;
	    
	    if ( !dev.isEmpty() && !icon.isEmpty() && !icon2.isEmpty() )
	    {
		QString mp = KIOServer::findDeviceMountPoint( dev.data() );
		pixmapFile2 = getIconPath();
		pixmapFile2.detach();
		pixmapFile2 += "/";
		if ( mp.isNull() )
		    pixmapFile2 += icon2.data();
		else
		    pixmapFile2 += icon.data();
		return pixmapFile2.data();
	    }
	}
	else
	{
	    QString icon = config->readEntry( "Icon" );
	    delete config;
	    debugT("ICON=%s '%s'\n",icon.data(),_url);
	    if ( !icon.isEmpty() )
	    {
		pixmapFile2 = getIconPath();
		pixmapFile2.detach();
		pixmapFile2 += "/";
		pixmapFile2 += icon.data();
		return pixmapFile2.data();
	    }
	}
    }
    
    // The default behavior.
    /* pixmap_file = getIconPath();
    pixmap_file.detach();
    pixmap_file += "/kmailion.xpm";
    return pixmap_file.data(); */
    pixmapFile2 = KMimeType::getPixmapFile( _url );
    return pixmapFile2.data();
}

QString KDELnkMimeType::getComment( const char *_url )
{
    KFMConfig *config = KMimeBind::openKFMConfig( _url );
    
    if ( config == 0L )
	return QString();
    
    QString erg = config->readEntry( "Comment" );
    return QString( erg.data() );
}

QPixmap& KDELnkMimeType::getPixmap( const char *_url )
{
    getPixmapFile( _url );
    pixmap.load( pixmapFile2 );
    return pixmap;
}

KMimeType::KMimeType( const char *_mime_type, const char *_pixmap )
{
    bApplicationPattern = false;
    
    mimeType = _mime_type;
    mimeType.detach();
    
    pixmap_file = getIconPath();
    pixmap_file += "/";
    pixmap_file.detach();
    pixmap_file += _pixmap;

    HTMLImage::cacheImage( pixmap_file.data() );
}

bool KMimeType::isDefault()
{
    return ( this == defaultType );
}

void KMimeBind::initApplications( const char * _path )
{
    DIR *dp;
    struct dirent *ep;
    dp = opendir( _path );
    if ( dp == 0L )
	return;
    
    // Loop thru all directory entries
    while ( ( ep = readdir( dp ) ) != 0L )
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

		KMimeBind::appendApplication( app.data() );
		
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
		QString mime = config.readEntry( "MimeType" );
		// A ';' separated list of protocols
		QString protocols = config.readEntry( "Protocols" );
		// Allow this program to be a default application for a mime type?
		// For example gzip should never be a default for any mime type.
		QString str_allowdefault = config.readEntry( "AllowDefault" );
		bool allowdefault = true;
		if ( str_allowdefault == "0" )
		    allowdefault = false;
		
		// Define an icon for the program file perhaps ?
		if ( !app_icon.isNull() && !app_pattern.isNull() )
		{
		    debugT("Installing binary pattern '%s' '%s'\n",app_pattern.data(),comment.data());
		    KMimeType *t;
		    types->append( t = new KMimeType( app.data(), app_icon.data() ) );
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
			QMessageBox::message( klocale->translate("Error"),
					      klocale->translate("Too many protocols in file\n") + file );
			pos2 = protocols.length();
		    }
		    
		    pos2++;
		    old_pos2 = pos2;
		}
		
		// To which mime types is the application bound ?
		pos2 = 0;
		old_pos2 = 0;
		while ( ( pos2 = mime.find( ";", pos2 ) ) != - 1 )
		{
		    // 'bind' is the name of a mime type
		    QString bind = mime.mid( old_pos2, pos2 - old_pos2 );
		    // Bind this application to all files/directories
		    if ( strcasecmp( bind.data(), "all" ) == 0 )
		    {
			defaultType->append( new KMimeBind( app.data(), exec.data(), allowdefault,
							    prots[0].data(),
							    prots[1].data(), prots[2].data(),
							    prots[3].data(), prots[4].data()) );
			folderType->append( new KMimeBind( app.data(), exec.data(), allowdefault,
							   prots[0].data(),
							   prots[1].data(), prots[2].data(),
							   prots[3].data(), prots[4].data() ) );
		    }
		    else if ( strcasecmp( bind.data(), "alldirs" ) == 0 )
		    {
			folderType->append( new KMimeBind( app.data(), exec.data(), allowdefault,
							   prots[0].data(),
							   prots[1].data(), prots[2].data(),
							   prots[3].data(), prots[4].data() ) );
		    }
		    else if ( strcasecmp( bind.data(), "allfiles" ) == 0 )
		    {
			defaultType->append( new KMimeBind( app.data(), exec.data(), allowdefault,
							    prots[0].data(),
							    prots[1].data(), prots[2].data(),
							    prots[3].data(), prots[4].data() ) );
		    }
		    // Bind this application to a mime type
		    else
		    {
			KMimeType *t = KMimeType::findByName( bind.data() );
			if ( t == 0 )
			    QMessageBox::message( klocale->translate("ERROR"), 
						  klocale->translate("Could not find mime type\n") + bind + "\n" + klocale->translate("in ") + file );
			else
			    t->append( new KMimeBind( app.data(), exec.data(), allowdefault,
						      prots[0].data(),
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

void KMimeType::initMimeTypes( const char* _path )
{   
    DIR *dp;
    struct dirent *ep;
    dp = opendir( _path );
    if ( dp == 0L )
	return;
    
    // Loop thru all directory entries
    while ( (ep = readdir( dp ) ) != 0L )
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
		initMimeTypes( file.data() );
	    else if ( tmp.length() > 7 && tmp.right( 7 ) == ".kdelnk" )
	    {
		QFile f( file.data() );
		if ( !f.open( IO_ReadOnly ) )
		    return;
		
		QTextStream pstream( &f );
		KConfig config( &pstream );
		config.setGroup( "KDE Desktop Entry" );
		
		// Read a new extension groups name
		QString ext = ep->d_name;
		if ( ext.length() > 7 && ext.right(7) == ".kdelnk" )
		    ext = ext.left( ext.length() - 7 );
		
		// Get a ';' separated list of all pattern
		QString pats = config.readEntry( "Patterns" );
		QString icon = config.readEntry( "Icon" );
		QString defapp = config.readEntry( "DefaultApp" );
		QString comment = config.readEntry( "Comment" );
		QString mime = config.readEntry( "MimeType" );
		
		// Is this file type already registered ?
		KMimeType *t = KMimeType::findByName( ext.data() );
		// If not then create a new type, but only if we have an icon
		if ( t == 0L && !icon.isNull() )
		{
		    if ( mime == "inode/directory" )
			types->append( t = new KFolderType( ext.data(), icon.data() ) );
		    else if ( mime == "application/x-kdelnk" )
			types->append( t = new KDELnkMimeType( ext.data(), icon.data() ) );
		    else
			types->append( t = new KMimeType( ext.data(), icon.data() ) );
		}
		// If we have this type already we perhaps only change the pixmap ?
		else if ( !icon.isNull() )
		    t->setPixmap( icon.data() );
		// Set the default binding
		if ( !defapp.isNull() && t != 0L )
		    t->setDefaultBinding( defapp.data() );
		if ( t != 0L )
		{
		    t->setComment( comment.data() );
		    t->setMimeType( mime.data() );
		}
		
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

void KMimeType::init()
{
    QString ipath = kapp->kdedir();
    ipath.detach();
    ipath += "/share/icons";
    strcpy( icon_path, ipath.data() );
    
    // KMimeType *ft;

    // kdelnkType = new KDELnkMimeType();

    // Read some informations about standard pixmaps.
    // KConfig *config = KApplication::getKApplication()->getConfig();

    // Read the deault icons
    // QString icon;
    
    /* execType = new KMimeType();
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
    execType->setMimeType( "application/x-executable" );
    execType->setComment( "Executable" );
    
    batchType = new KMimeType();
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
    batchType->setMimeType( "application/x-shellscript" );
    batchType->setComment( "Shell Script" );
    
    PipeType = new KMimeType();
    icon = config->readEntry( "Pipe" );
    if ( icon.isNull() )
    {
      strcpy( PipePixmap, "pipe.xpm" );
      PipeType->setPixmap( "pipe.xpm" );
    }
    else
    {
       PipeType->setPixmap( icon.data() );
       strcpy( PipePixmap, icon.data() );
    }
    PipeType->setMimeType( "inode/fifo" );
    PipeType->setComment( "Pipe" );


    SocketType = new KMimeType();
    icon = config->readEntry( "Socket" );
    if ( icon.isNull() )
    {
       strcpy( SocketPixmap, "socket.xpm" );
       SocketType->setPixmap( "socket.xpm" );
    }
    else
    {
       SocketType->setPixmap( icon.data() );
       strcpy( SocketPixmap, icon.data() );
    }
    SocketType->setMimeType( "inode/socket" );
    SocketType->setComment( "Socket" );


    CDevType = new KMimeType();
    icon = config->readEntry( "Chardevice" );
    if ( icon.isNull() )
    {
       strcpy( CDevPixmap, "char_dev.xpm" );
       CDevType->setPixmap( "char_dev.xpm" );
    }
    else
    {
       CDevType->setPixmap( icon.data() );
       strcpy( CDevPixmap, icon.data() );
    }
    CDevType->setMimeType( "inode/chardevice" );
    CDevType->setComment( "Character device" );  

    BDevType = new KMimeType();
    icon = config->readEntry( "Blockdevice" );
    if ( icon.isNull() )
    {
       strcpy( BDevPixmap, "block_dev.xpm" );
       BDevType->setPixmap( "block_dev.xpm" );
    }
    else
    {
       BDevType->setPixmap( icon.data() );
       strcpy( BDevPixmap, icon.data() );
    }
    BDevType->setMimeType( "inode/blockdevice" );
    BDevType->setComment( "Block device" );
                            
    defaultType = new KMimeType();
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
    defaultType->setMimeType( "kfm/default" );

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
    folderType->setMimeType( "inode/directory" );
    folderType->setComment( "Folder" );
    
    lockedfolderType = new KFolderType();
    icon = config->readEntry( "LockedFolder" );
    if ( icon.isNull() )
    {
       strcpy( lockedfolderPixmap, "lockedfolder.xpm" );
       lockedfolderType->setPixmap( "lockedfolder.xpm" );
    }
    else
    {
       strcpy( lockedfolderPixmap, icon.data() );
       lockedfolderType->setPixmap( icon.data() );
    }
    lockedfolderType->setMimeType( "inode/directory-locked" );
    lockedfolderType->setComment( "Locked Folder" );
    */
    types = new QList<KMimeType>;
    types->setAutoDelete( true );
    
    // Read the application bindings
    QString path = kapp->kdedir();
    path += "/share/mimelnk";
    initMimeTypes( path.data() );

    // Otherwise we are not allowed to call 'errorMissingMimeType'
    defaultType = 0L;
    // Try to find the default type
    if ( ( defaultType = KMimeType::findByName( "application/octet-stream" ) ) == 0L )
	errorMissingMimeType( "application/octet-stream", &defaultType );

    // No default type ?
    if ( defaultType == 0L )
	defaultType = new KMimeType( "application/octet-stream", getDefaultPixmap() );

    // No Mime-Types installed ?
    // Lets do some rescue here.
    if ( types->count() == 0 )
    {
	QMessageBox::message( klocale->translate( "KFM Error" ),
			      klocale->translate( "No mime types installed!" ) );
	
	// Lets have at least the default for all
	// and dont bother the user any longer
	folderType = defaultType;
	lockedfolderType = defaultType;	
	BDevType = defaultType;
	CDevType = defaultType;
	SocketType = defaultType;
	PipeType = defaultType;
	batchType = defaultType;
	execType = defaultType;
	kdelnkType = defaultType;
    }
    else
    {
	if ( ( folderType = KMimeType::findByName( "inode/directory" ) ) == 0L )
	    errorMissingMimeType( "inode/directory", &folderType );
	if ( ( lockedfolderType = KMimeType::findByName( "inode/directory-locked" ) ) == 0L )
	    errorMissingMimeType( "inode/directory-locked", &lockedfolderType );
	if ( ( BDevType = KMimeType::findByName( "inode/blockdevice" ) ) == 0L )
	    errorMissingMimeType( "inode/blockdevice", &BDevType );
	if ( ( CDevType = KMimeType::findByName( "inode/chardevice" ) ) == 0L )
	    errorMissingMimeType( "inode/chardevice", &CDevType );
	if ( ( SocketType = KMimeType::findByName( "inode/socket" ) ) == 0L )
	    errorMissingMimeType( "inode/socket", &SocketType );
	if ( ( PipeType = KMimeType::findByName( "inode/fifo" ) ) == 0L )
	    errorMissingMimeType( "inode/fifo", &PipeType );
	if ( ( batchType = KMimeType::findByName( "application/x-shellscript" ) ) == 0L )
	    errorMissingMimeType( "application/x-shellscript", &batchType );
	if ( ( execType = KMimeType::findByName( "application/x-executable" ) ) == 0L )
	    errorMissingMimeType( "application/x-executable", &execType );
	if ( ( kdelnkType = KMimeType::findByName( "application/x-kdelnk" ) ) == 0L )
	    errorMissingMimeType( "application/x-kdelnk", &kdelnkType );
    }
    
    // Read the application bindings
    path = kapp->kdedir();
    path += "/apps";
    KMimeBind::initApplications( path.data() );
}

void KMimeType::errorMissingMimeType( const char *_type, KMimeType **_ptr )
{
    QString tmp = klocale->translate( "Could not find mime type\r\n" );
    tmp += _type;
    
    QMessageBox::message( klocale->translate( "KFM Error" ), tmp );
    
    *_ptr = defaultType;
}
    
void KMimeType::clearAll()
{
    KMimeBind::clearApplicationList();
    
    delete types;
}

void KMimeType::append( KMimeBind *_bind )
{
    bindings.append( _bind );
}

KMimeType* KMimeType::getFirstMimeType()
{
    return types->first();
}

KMimeType* KMimeType::getNextMimeType()
{
    return types->next();
}

KMimeType* KMimeType::findByPattern( const char *_pattern )
{
    KMimeType *typ;
    
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

KMimeType* KMimeType::findByName( const char *_name )
{
    KMimeType *typ;
    
    for ( typ = types->first(); typ != 0L; typ = types->next() )
    {
	if ( strcmp( typ->getMimeType(), _name ) == 0 )
	    return typ;
    }

    return 0L;
}

KMimeType* KMimeType::findType( const char *_url )
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
    
    // Is it really a directory ?
    if ( KIOServer::isDir( _url ) > 0 )
    {
	// File on the local hard disk ?
	if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	{   
	    if ( access( u.path(), R_OK|X_OK ) < 0 )
		return lockedfolderType;
	}
	else
	{
	    // Is it a directory in a tar file ?
	    KURL u2( u.nestedURL() );
	    if ( strcmp( u2.protocol(), "tar" ) == 0 )
		// Those directories are always accessible
		return folderType;
	    
	    QString user = u.user();
	    if ( user.data() == 0L || user.data()[0] == 0 )
		user = "anonymous";
	    KIODirectoryEntry *e = KIOServer::getKIOServer()->getDirectoryEntry( _url );
	    if ( e != 0L )
	    {
		if ( !e->mayRead( user.data() ) || !e->mayExec( user.data() ) )
		    return lockedfolderType;
	    } 
	}    
	return folderType;
    }              

    KMimeType *typ;

    // Links may appear on the local hard disk only. If this is a link
    // we will use the file/dir the link is pointing to to determine
    // the file type.
    if ( strcmp( u.protocol(), "file" ) == 0 && ( u.reference() == 0L || *(u.reference()) == 0 ) )
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

    QString filename;
    // Do we have a nested URL perhaps ?
    if ( u.reference() != 0L && *(u.reference()) != 0 )
    {
	KURL u3( u.nestedURL() );
	filename = u3.filename();
    }
    else
	filename = u.filename();
    
    for ( typ = types->first(); typ != 0L; typ = types->next() )
    {
	QStrList& pattern = typ->getPattern();
	char *s;
	for ( s = pattern.first(); s != 0L; s = pattern.next() )
	{
	    int pattern_len = strlen( s );
	    int len = filename.length();	

	    if ( s[ pattern_len - 1 ] == '*' && len + 1 >= pattern_len )
	    {
		if ( strncmp( filename.data(), s, pattern_len - 1 ) == 0 )
		    return typ;
	    }
	    if ( s[ 0 ] == '*' )
		if ( strncmp( filename.data() + len - pattern_len + 1, s + 1, pattern_len - 1 ) == 0 )
		    return typ;
	    if ( strcmp( filename.data(), s ) == 0 )
		return typ;
	}
    }

    // Since mounted ms file systems often set the executable flag without
    // providing real executables, we first check the extension before looking
    // at the flags.
    // Executable ? Must be on the local drive.
    if ( strcmp( u.protocol(), "file" ) == 0 && ( u.reference() == 0L || *(u.reference()) == 0 ) )
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
	if ( S_ISFIFO( buff.st_mode ) )
	  return PipeType;
	if ( S_ISSOCK( buff.st_mode ) )
	  return SocketType;
	if ( S_ISCHR( buff.st_mode ) )
	  return CDevType;
	if ( S_ISBLK( buff.st_mode ) )
	  return BDevType;  
    }

    return defaultType;
}

void KMimeType::getBindings( QStrList &_list, const char *_url, bool _isdir )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return;
    
    // Used to store a new value for _url
    QString tmp;
    
    // Try to read the file as a [KDE Desktop Entry]
    KFMConfig *config = KMimeBind::openKFMConfig( _url );
    
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
		debugT("Changed to '%s'\n",_url);
		KURL u2( _url );
		u = u2;
	    }
	}
	delete config;
    }

    // If it is a link, get the name the link is pointing to
    /* if ( strcmp( u.protocol(), "file" ) == 0 )
    {
	struct stat lbuff;
	lstat( u.path(), &lbuff );
	if ( S_ISLNK( lbuff.st_mode ) )
	{
	    QDir d( u.path() );
	    QString x = d.canonicalPath();
	    if ( !x.isNull() )
	    {
		tmp = x.data();
		tmp.detach();
		_url = tmp.data();
		debugT("$$$$$$$$$ Changed to '%s'\n",_url);
		KURL u2( _url );
		u = u2;
	    }
	}
    } */
    
    // A directory named dir.html for example does not have a binding to
    // netscape or arena.
    if ( _isdir )
    {
	KMimeBind *bind;
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
	KMimeType *typ = KMimeType::findType( _url );
	if ( !typ->hasBindings() )
	    return;

	KMimeBind *bind;
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

KMimeBind* KMimeType::findBinding( const char *_name )
{
    KMimeBind *b;
    for ( b = bindings.first(); b != 0L; b = bindings.next() )
    {
	if ( strcmp( _name, b->getProgram() ) == 0L )
	    return b;
    }
    
    return 0L;
}

bool KMimeType::run( const char *_url )
{
    // Do we have some default binding ?
    if ( !defaultBinding.isEmpty() )
    {
	KMimeBind* bind = findBinding( defaultBinding );
	if ( bind )
	    return KMimeBind::runBinding( _url, bind );
    }
    
    // Only one binding ?
    if ( bindings.count() == 1 )
    {
	KMimeBind* bind = bindings.first();
	if ( bind->isAllowedAsDefault() )
	    return KMimeBind::runBinding( _url, bind );    
    }

    // Take first binding which is allowed as default
    KMimeBind *bind;
    for ( bind = bindings.first(); bind != 0L; bind = bindings.next() )
    {
	if ( bind->isAllowedAsDefault() )
	    return KMimeBind::runBinding( _url, bindings.first() );
    }

    return FALSE;
}

bool KMimeBind::runBinding( const char *_url )
{    
    KMimeType *typ = KMimeType::findType( _url );

    // KFM must execute *.kdelnk. So dont look for bindings here.
    // Perhaps this function is called again from within ::runBinding( .. ,.. ).
    // Have a look at the code for type "URL".
    QString u = _url;
    if ( u.length() > 7 && u.left(5) == "file:" && u.right( 7 ) == ".kdelnk" )
    {
	return KMimeBind::runBinding( _url, 
				      klocale->getAlias(ID_STRING_OPEN) );
    }
    
    // Is there only one binding ?
    QStrList bindings;
    bindings.setAutoDelete( true );
    KMimeType::getBindings( bindings, _url, false );    
    // Any bindings available
    if ( bindings.isEmpty() )
    {
	// Try the default binding named "Open"
	return KMimeBind::runBinding( _url, 
				      klocale->getAlias(ID_STRING_OPEN) );
    }
    
    // Only one binding ?
    if ( bindings.count() == 1 )
    {
	KMimeBind* bind = 0L;
	if ( typ )
	    bind = typ->findBinding( bindings.first() );
	if ( bind && !bind->isAllowedAsDefault() )
	{
	    // Try the default binding named "Open"
	    return KMimeBind::runBinding( _url, klocale->getAlias(ID_STRING_OPEN) );
	}
	else
	    return KMimeBind::runBinding( _url, bindings.first() );    
    }

    // Find default binding
    if ( typ->getDefaultBinding() != 0 )
    {
	return KMimeBind::runBinding( _url, typ->getDefaultBinding() );    
    }

    // Take first binding which is allowed as default
    char *s;
    for ( s = bindings.first(); s != 0L; s = bindings.next() )
    {
	KMimeBind *b = 0L;
	if ( typ )
	    b = typ->findBinding( s );
	if ( b && b->isAllowedAsDefault() )
	{
	    return KMimeBind::runBinding( _url, bindings.first() );
	}
    }
    
    // Try the default binding named "Open"
    return KMimeBind::runBinding( _url, klocale->getAlias(ID_STRING_OPEN));    
}


bool KMimeBind::runBinding( const char *_url, const char *_binding, QStrList * _arguments )
{
    if ( _binding == 0L )
	_binding = "";

    debugT("Binding is '%s', URL is '%s'\n",_binding,_url);

    KURL u( _url );
    if ( u.isMalformed() )
	return FALSE;

    QString decodedPath( u.path() );
    KURL::decodeURL( decodedPath );
    QString decodedURL( _url );
    KURL::decodeURL( decodedURL );
    
    // Is it an executable on the local hard disk ?
    if ( strcmp( u.protocol(), "file" ) == 0 && 
	 strcasecmp( _binding,  klocale->getAlias(ID_STRING_OPEN) ) == 0 &&
	 u.childURL().isEmpty() )
    {
	struct stat buff;
	if ( stat( decodedPath, &buff ) == 0 )
	{
	    if ( ( buff.st_mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) != 0 )
	    {
		debugT("Executing '%s'\n",decodedPath.data());
		
		QString exec = KIOServer::shellQuote( decodedPath );
		QString cmd = "\"";
		cmd += exec;
		cmd += "\" ";
		
		if ( _arguments != 0L )
		{
		    char *s;
		    for ( s = _arguments->first(); s != 0L; s = _arguments->next() )
		    {
			KURL su( s );
			cmd += "\"";
			if ( strcmp( u.protocol(), "file" ) == 0 )
			{
			    QString dec( su.path() );
			    KURL::decodeURL( dec );
			    dec = KIOServer::shellQuote( dec );
			    cmd += dec;
			}
			else
			{
			    QString dec( s );
			    KURL::decodeURL( dec );
			    dec = KIOServer::shellQuote( dec );
			    cmd += dec;
			}
			cmd += "\" ";
		    }
		}
		
		cmd += "&";
		system( cmd.data() );
		return TRUE;
	    }
	}
    }
    
    // Used to store a modified value for _url
    QString tmp;
    
    debugT("Testing for KDE Desktop Entry\n");
    
    KFMConfig *config = 0L;
    // Is it a "[KDE Desktop Entry]" file and do we want to open it ?
    // ... but only if it is on the local hard disk!
    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	config = KMimeBind::openKFMConfig( _url );
    
    // Could we open the config file ?
    if ( config != 0L )
    {
	debugT("################### Is a KDE Desktop Entry file\n");
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
		    QString decoded( su.path() );
		    KURL::decodeURL( decoded );
		    decoded = KIOServer::shellQuote( decoded ).data();
		    f += "\"";
		    f += decoded;
		    f += "\" ";
		    u += "\"";
		    u += s;
		    u += "\" ";
		    QString tmp = decoded.data();
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
	    while ( ( i = exec.find( "%c" ) ) != -1 )
            {
	      QString s = _url;
	      if ( s.length() > 7 && s.right( 7 ) == ".kdelnk" )
	      {
		s = s.left(s.length()-7);
              }
              int a = s.findRev("/");
              if ( a > -1 )
                s = s.right(s.length()-1-a);
              exec.replace(i,2,s.data());
            }     

	    if ( term == "1" )
	    {
		KConfig *config = KApplication::getKApplication()->getConfig();
		config->setGroup( "Terminal" );
		QString cmd = "kvt";
		cmd = config->readEntry( "Terminal", cmd );
		cmd.detach();
		if ( cmd.isNull() )
		{
		    warning(klocale->translate("ERROR: No Terminal Setting"));
		    delete config;
		    return TRUE;
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
		debugT("Running '%s'\n",cmd.data());
		system( cmd.data() );
	    }
	    else
	    {
		QString cmd = exec.data();
		cmd += " &";
		debugT("Running '%s'\n",cmd.data());
		system( cmd.data() );
	    }
	    delete config;
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
		    return TRUE;
		}
		else if ( strncmp( _binding, "Mount ", 6 ) == 0 )
		{
		    QString readonly = config->readEntry( "ReadOnly" );
		    bool ro = FALSE;
		    if ( !readonly.isNull() )
			if ( readonly == '1' )
			    ro = true;
			 
		    KIOJob *job = new KIOJob;
		   
		    // The binding is names 'Mount FSType' => +6
		    if ( strcasecmp( _binding + 6, "default" ) == 0 )
			job->mount( false, 0L, dev, 0L );
		    else
			job->mount( ro, _binding + 6, dev, point );
		    delete config;
		    return TRUE;
		}
		else if ( strcmp( _binding, 
				  klocale->getAlias(ID_STRING_OPEN) ) == 0 )
		{
		    QString mp = KIOServer::findDeviceMountPoint( dev.data() );
		    if ( !mp.isNull() )
		    {
			QString mp2 = "file:";
			mp2 += mp;
			debugT("KDE: Opening %s\n",mp2.data());
			KfmGui *m = new KfmGui( 0L, 0L, mp2.data() );
			m->show();
		    }
		    else
			warning(klocale->translate("ERROR: You must first mount the device. Use the right mouse button"));
		    delete config;
		    return TRUE;
		}
		else
		{
		    warning(klocale->translate("ERROR: Unknown Binding '%s'"),
			    _binding);
		    delete config;
		}
	    }
	}
	else if ( strcmp( typ, "Link" ) == 0 )
	{
	    QString url = config->readEntry( "URL" );
	    delete config;
	    if ( !url.isNull() )
	    {
		debugT("######################## It is a link\n");
		// Can KFM handle such an ULR ? Does the user wish the default binding => _binding == "open" ?
		if ( ( strncasecmp( url.data(), "tar:", 4 ) == 0 || strncasecmp( url.data(), "http:", 5 ) == 0 ||
		     strncasecmp( url.data(), "file:", 5 ) == 0 || strncasecmp( url.data(), "ftp:", 4 ) == 0 ) &&
		     strcasecmp( _binding, "open" ) == 0 && KIOServer::isDir( url ) > 0 )
		{
		    KfmGui *m = new KfmGui( 0L, "3", url.data() );
		    m->show();
		}
		// Call runBinding( .. ) again with the URL as parameter.
		// Perhaps we have an external program for such URLs
		else
		    return KMimeBind::runBinding( url.data() );
		return TRUE;
	    }
	}
	else
	    delete config;
    }

    debugT("Testing for other Binding\n");
    
    KMimeType *typ = KMimeType::findType( _url );
    KMimeBind *bind;
    if ( typ )
	for ( bind = typ->firstBinding(); bind != 0L; bind = typ->nextBinding() )
	{
	    debugT("!!! '%s' vs '%s'\n",bind->getProgram(), _binding );
	    if ( strcmp( bind->getProgram(), _binding ) == 0 )
		return runBinding( _url, bind );
	}

    // If we still dont know what to do, perhaps we can open a new window with the URL ?
    /* if ( KIOServer::isDir( _url ) && strcasecmp( _binding, "open" ) == 0 )
    {
	KfmGui * m = new KfmGui( 0L, 0L, _url );
	m->show();
    } */
    return FALSE;
}

bool KMimeBind::runBinding( const char *_url, KMimeBind *bind )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return FALSE;

    QString decodedPath( u.path() );
    KURL::decodeURL( decodedPath );
    QString decodedURL( _url );
    KURL::decodeURL( decodedURL );

    debugT("!!! '%s'\n",bind->getCmd());

    QString quote1 = KIOServer::shellQuote( decodedPath ).copy();
    QString quote2 = KIOServer::shellQuote( decodedURL ).copy();
    
    QString f;
    QString ur;
    QString n = "";
    QString d = "";
    f.sprintf( "\"%s\"", quote1.data() );
    ur.sprintf( "\"%s\"", quote2.data() );
    QString tmp = quote1.data();
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
    
    // Did the user forget to append something like '%f' ?
    // If so, then assume that '%f' is the right joice.
    if ( cmd.find( "%f" ) == -1 && cmd.find( "%u" ) == -1 && cmd.find( "%n" ) == -1 &&
	 cmd.find( "%d" ) == -1 )
    {
	cmd += " ";
	cmd += f.data();
    }
    else
    {
	int pos;
	while ( ( pos = cmd.find( "%f" )) != -1 )
	    cmd.replace( pos, 2, f.data() );
	while ( ( pos = cmd.find( "%u" )) != -1 )
	    cmd.replace( pos, 2, ur.data() );
	while ( ( pos = cmd.find( "%n" )) != -1 )
	    cmd.replace( pos, 2, n.data() );
	while ( ( pos = cmd.find( "%d" )) != -1 )
	    cmd.replace( pos, 2, d.data() );
    }
    int pos;
    while ( ( pos = cmd.find( "%c" ) ) != -1 )
    {
	QString s = _url;
	if ( s.length() > 7 && s.right( 7 ) == ".kdelnk" )
	{
	    s = s.left(s.length()-7);
	}
	int a = s.findRev("/");
	if ( a > -1 )
	    s = s.right(s.length()-1-a);
	cmd.replace(pos,2,s.data());
    }        

    debugT("::CMD = %s\n",cmd.data());
    runCmd( cmd.data() );
    debugT("Executed the binding\n");
    return TRUE;
}

void KMimeBind::runCmd( const char *_cmd )
{
    char *cmd = new char[ strlen( _cmd ) + 1 ];
    strcpy( cmd, _cmd );
    
    QString exec;
    QStrList args;
    
    char *p2;
    char *p = strchr( cmd, ' ' );
    if ( p == 0L )
    {
	exec = cmd;
	exec.detach();
    }
    else
    {
	*p++ = 0;
	exec = cmd;
	exec.detach();

	while ( *p == ' ' ) p++;
	do
	{
	    // Found a quotes string ?
	    if ( *p == '\"' )
	    {
		// Find the end of the quotes string
		p++;
		p2 = p;
		while ( *p2 != '\"' && *p2 != 0 )
		{
		    if ( *p2 == '\\' && p2[1] == '\"' )
			p2 += 2;
		    else
			p2++;
		}
	    }
	    else // the string is not quoted
		p2 = strchr( p, ' ' );
	    
	    // add to the list of parameters
	    if ( p2 )
	    {
		*p2++ = 0;
		QString tmp = KIOServer::shellUnquote( p ).copy();
		args.append( tmp.data() );
		p = p2;
		while ( *p == ' ' ) p++;   
	    }
	} while ( p2 );
	if ( strlen( p ) > 0 )
	    args.append( p );
    }
    
    char **argv = new char*[ args.count() + 2 ];
    char* s;
    argv[ 0 ] = (char*)exec.data();
    int i = 1;
    for ( s = args.first(); s != 0L; s = args.next() )
	argv[ i++ ] = (char*)s;
    argv[ i ] = 0L;
    
    debugT("Running '%s'\n",exec.data() );

    int pid;
    if ( ( pid = fork() ) == 0 )
    {    
	execvp( exec.data(), argv );
	QString txt = klocale->translate("Could not execute program\n");
	txt += exec.data();

	char* a[ 3 ];
	a[ 0 ] = "kfmwarn";
	a[ 1 ] = txt.data();
	a[ 2 ] = 0L;
	execvp( "kfmwarn", a );

	exit( 1 );
    }
    debugT("Deleting\n");
    delete [] argv;
    delete [] cmd;
    debugT("PID of started process is '%i'\n",pid);
}

KFMConfig* KMimeBind::openKFMConfig( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return 0L;
    if ( strcmp( u.protocol(), "file" ) != 0 )
	return 0L;
    
    QString decoded( u.path() );
    KURL::decodeURL( decoded );
    
    FILE *f;
    // _url must be of type 'file:/'
    f = fopen( decoded, "rb" );
    if ( f == 0L )
	return 0L;
    
    char buff[ 100 ];
    buff[ 0 ] = 0;
    fgets( buff, 100, f );
    fclose( f );

    if ( strncmp( buff, "# KDE Config File", 17 ) != 0L )
	return 0L; 
       
    QFile *file = new QFile( decoded );
    if ( !file->open( IO_ReadOnly ) )
    {
	delete file;
	return 0L;
    }
    
    QTextStream *pstream = new QTextStream( file );
    KFMConfig *config = new KFMConfig( file, pstream );
    config->setGroup( "KDE Desktop Entry" );
    return config;
}

QPixmap& KMimeType::getPixmap( const char * )
{
    if ( pixmap.isNull() )
	pixmap.load( pixmap_file );

    return pixmap;
}

const char* KMimeType::getPixmapFile( const char * )
{
    return pixmap_file.data();
}

void KMimeType::setPixmap( const char *_file )
{
    pixmap_file = getIconPath();
    pixmap_file.detach();
    pixmap_file += "/";
    pixmap_file += _file;

    HTMLImage::cacheImage( pixmap_file.data() );
}


KMimeBind::KMimeBind( const char *_prg, const char *_cmd, bool _allowdefault, const char *_prot1, 
		      const char *_prot2, const char *_prot3,
		      const char *_prot4, const char *_prot5 )
{
    allowDefault = _allowdefault;
    
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

KMimeType* KMimeType::getMagicMimeType( const char *_url )
{
    // Just for speedup, because KURL is slow
    if ( strncmp( _url, "file:/", 6 ) == 0 || _url[0] == '/' )
    {
	KURL u( _url );
	// Not a tar file ?
	if ( !u.hasSubProtocol() )
	{
	    KMimeMagicResult* result = KMimeType::findFileType( u.path() );

	    // Is it a directory or dont we know anything about it ?
	    if ( result->getContent() == 0L || strcmp( "inode/directory", result->getContent() ) == 0 ) 
		/* strcmp( "application/x-kdelnk", result->getContent() ) == 0 ) */
		return KMimeType::findType( _url );

	    // Can we trust the result ?
	    if ( result->getAccuracy() >= 50 )
	    {
		KMimeType *type = KMimeType::findByName( result->getContent() );
		// Do we know this mime type ?
		if ( type )
		    return type;
	    }
	    // Try the usual way
	    KMimeType *type = KMimeType::findType( _url );
	    // Perhaps we should better listen to the magic :-)
	    if ( ( type == 0L || type->isDefault() ) && result->getContent() != 0L && result->getAccuracy() >= 30 )
	    {
		KMimeType *type = KMimeType::findByName( result->getContent() );
		// Do we know this mime type ?
		if ( type )
		    return type;
	    }
	}
    }
    
    return KMimeType::findType( _url );
}

const char* KMimeType::getPixmapFileStatic( const char *_url )
{
    return KMimeType::getMagicMimeType( _url )->getPixmapFile( _url );
}

void KMimeType::initKMimeMagic()
{
    // Magic file detection init
    QString mimefile = kapp->kdedir();
    mimefile += "/share/magic";
    magic = new KMimeMagic( mimefile );
    magic->setFollowLinks( TRUE );
}

bool KMimeBind::supportsProtocol( const char *_protocol )
{
    if ( strcmp( protocol1, _protocol ) == 0 )
	return true;
    if ( strcmp( protocol2, _protocol ) == 0 )
	return true;
    if ( strcmp( protocol3, _protocol ) == 0 )
	return true;
    if ( strcmp( protocol4, _protocol ) == 0 )
	return true;
    if ( strcmp( protocol5, _protocol ) == 0 )
	return true;

    return false;
}
