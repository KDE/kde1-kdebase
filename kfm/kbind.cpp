/*
 * "$Id"
 */

#include <qdir.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/resource.h>
 
#include <limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>

#include <qmsgbox.h>
#include <qtstream.h>

#include <kurl.h>
#include <klocale.h>
#include <kstring.h>

#include "kbind.h"
#include "kfmpaths.h"
#include "kfmprops.h"
#include "kfmgui.h"
#include "config-kfm.h"
#include "kfmexec.h"
#include "utils.h"

QPixmapCache* KMimeType::pixmapCache = 0L;

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

// Holds the path of global icons, but it is not a URL.
QString *globalIconPath = 0L;
// Holds the path of local icons, but it is not a URL.
QString *localIconPath = 0L;
// Holds the full qualified path and name of the default icon,
// but this is NOT a URL
QString *defaultIcon = 0L;
// Holds the full qualified path and name of the default mini icon,
// but this is NOT a URL
QString *defaultMiniIcon = 0L;

QList<KMimeBind> *KMimeBind::s_lstBindings;

QDict<QString>* KMimeType::iconDict = 0L;
QDict<QString>* KMimeType::miniIconDict = 0L;

QPixmap* emptyPixmap = 0L;

void KMimeBind::InitStatic()
{
  s_lstBindings = new QList<KMimeBind>;
  if ( emptyPixmap == 0L )
    emptyPixmap = new QPixmap();
}

void KMimeType::InitStatic()
{
    if ( emptyPixmap == 0L )
	emptyPixmap = new QPixmap();
    if ( pixmapCache == 0L )
	pixmapCache = new QPixmapCache;
    if ( globalIconPath == 0L )
    {
	globalIconPath = new QString( kapp->kde_icondir().copy() );
    }
    if ( localIconPath == 0L )
    {
	localIconPath = new QString( kapp->localkdedir().data() );
	*localIconPath += "/share/icons";
    }    
    if ( defaultIcon == 0L )
    {
	defaultIcon = new QString( globalIconPath->data() );
	*defaultIcon += "/";
	*defaultIcon += getDefaultPixmap();
    }
    if ( defaultMiniIcon == 0L )
    {
	defaultMiniIcon = new QString( globalIconPath->data() );
	*defaultMiniIcon += "/mini/";
	*defaultMiniIcon += getDefaultPixmap();
    }
    if ( iconDict == 0L )
	iconDict = new QDict<QString>;
    if ( miniIconDict == 0L )
	miniIconDict = new QDict<QString>;
}    

/***************************************************************
 *
 * KMimeType
 *
 ***************************************************************/

KMimeType::KMimeType( const char *_mime_type, const char *_pixmap )
{
    bApplicationPattern = false;
    
    mimeType = _mime_type;
    mimeType.detach();
    
    pixmapName = _pixmap;
    pixmapName.detach();

    pixmap = 0L;
}

const char* KMimeType::getIconPath( const char *_icon, bool _mini )
{
    QString *res;
    if ( _mini && ( res = (*miniIconDict)[_icon] ) != 0L )
	return res->data();
    else if ( !_mini && ( res = (*iconDict)[_icon] ) != 0L )
	return res->data();

    QString *s = new QString; // We must create a new one each time, because its
                              // address is stored in the QDict instances
    
    *s = localIconPath->data();

    if ( _mini )
	*s += "/mini/";
    else
	*s += "/";
    *s += _icon;
    
    FILE *f = fopen( s->data(), "r" );
    if ( f != 0L )
    {
	fclose( f );
	if ( _mini )
	    miniIconDict->insert( _icon, s );
	else
	    iconDict->insert( _icon, s );
	return s->data();
    }

    *s = globalIconPath->data();
    if ( _mini )
	*s += "/mini/";
    else
	*s += "/";
    *s += _icon;
    
    f = fopen( s->data(), "r" );
    if ( f != 0L )
    {
	fclose( f );
	if ( _mini )
	    miniIconDict->insert( _icon, s );
	else
	    iconDict->insert( _icon, s );
	return s->data();
    }

    delete s;

    if ( _mini )
	return defaultMiniIcon->data();
    else
	return defaultIcon->data();
}
    
// We dont have a look at the URL ( the 1. parameter )
QPixmap* KMimeType::getPixmap( const char *, bool _mini )
{
    if ( pixmap )
	return pixmap;

    pixmap = pixmapCache->find( getPixmapFile( _mini ) );
    if ( pixmap == 0L )
    {
	pixmap = new QPixmap;
	pixmap->load( getPixmapFile( _mini ) );
	pixmapCache->insert( getPixmapFile( _mini ), pixmap );
    }
    
    return pixmap;
}

const char* KMimeType::getPixmapFile( bool _mini )
{
    if ( _mini )
    {
        if ( miniPixmapFile.isEmpty() )
        {
            miniPixmapFile = getIconPath( pixmapName, true );
            HTMLImage::cacheImage( miniPixmapFile.data() );
        }
        return miniPixmapFile;
    }
    else
    {
        if ( pixmapFile.isEmpty() )
        {
            pixmapFile = getIconPath( pixmapName );
            HTMLImage::cacheImage( pixmapFile.data() );
        }
        return pixmapFile;
    }
}

KMimeType* KMimeType::getMagicMimeType( const char *_url )
{
    KMimeType *type = KMimeType::findType( _url );
    if ( type != defaultType )
	return type;

    if ( strncmp( _url, "file:/", 6 ) != 0 && _url[0] != '/' )
	return type;

    KURL u( _url );
    // Not a tar file ?
    if ( u.hasSubProtocol() )
	return type;
    
    QString path( u.path() );
    KMimeMagicResult* result = KMimeType::findFileType( path );

    // Is it a directory or dont we know anything about it ?
    if ( result->getContent() == 0L || strcmp( "inode/directory", result->getContent() ) == 0 ||
	 strcmp( "application/octet-stream", result->getContent() ) == 0 )
	return type;
    
    KMimeType *type2 = KMimeType::findByName( result->getContent() );
    if ( type2 )
	return type2;
    
    return type;
}

const char* KMimeType::getPixmapFileStatic( const char *_url, bool _mini )
{
    return KMimeType::getMagicMimeType( _url )->getPixmapFile( _url, _mini );
}

void KMimeType::initKMimeMagic()
{
    // Magic file detection init
    QString mimefile = kapp->kde_mimedir().copy();
    mimefile += "/magic";
    magic = new KMimeMagic( mimefile );
    magic->setFollowLinks( TRUE );
}

bool KMimeType::isDefault()
{
    return ( this == defaultType );
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

	    else if ( tmp.length() > 7 && tmp.right( 7 ) == ".kdelnk")
	    {
		KSimpleConfig config( file, true );
		config.setGroup( "KDE Desktop Entry" );
		
		QString mime = config.readEntry( "MimeType" );

		// Skip this one ?
		if ( mime.isEmpty() )
		{
		    // If we don't have read access, we'll skip quietly.
		    if ( access( file.data(), R_OK ) )
		       continue;

		    QString tmp;
		    tmp.sprintf( "%s\n%s\n%s", i18n( "The mime type config file " ),
				 file.data(), i18n("does not contain a MimeType=... entry" ) );
		    QMessageBox::warning( 0, i18n( "KFM Error" ), tmp );
		    continue;
		}

		// Get a ';' separated list of all pattern
		QString pats = config.readEntry( "Patterns" );
		QString icon = config.readEntry( "Icon" );
		QString defapp = config.readEntry( "DefaultApp" );
		QString comment = config.readEntry( "Comment" );

		// Is this file type already registered ?
		KMimeType *t = KMimeType::findByName( mime.data() );
		// If not then create a new type
		if ( t == 0L )
		{
		    if ( icon.isEmpty() )
			icon = KMimeType::getDefaultPixmap();
		    
		    if ( mime == "inode/directory" )
			types->append( t = new KFolderType( mime.data(), icon.data() ) );
		    else if ( mime == "application/x-kdelnk" )
			types->append( t = new KDELnkMimeType( mime.data(), icon.data() ) );
		    else if ( mime == "application/x-executable" ||
			      mime == "application/x-shellscript" )
			types->append( t = new ExecutableMimeType( mime.data(), icon.data() ) );
		    else
			types->append( t = new KMimeType( mime.data(), icon.data() ) );

		    // Set the default binding
		    if ( !defapp.isNull() && t != 0L )
			t->setDefaultBinding( defapp.data() );
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
		}
            }
        }
    }
    closedir(dp);
}

void KMimeType::init()
{
    types = new QList<KMimeType>;
    types->setAutoDelete( true );

    // Read the application bindings in the local directories
    QString path = kapp->localkdedir().copy();
    path += "/share/mimelnk";
    initMimeTypes( path.data() );
    
    // Read the application bindings in the global directories
    path = kapp->kde_mimedir().copy();
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
	QMessageBox::warning( 0, i18n( "KFM Error" ),
			      i18n( "No mime types installed!" ) );
	
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
    
    // Read the application bindings in the local directories
    path = kapp->localkdedir().copy();
    path += "/share/applnk";
    KMimeBind::initApplications( path.data() );

    // Read the application bindings in the global directories
    path = kapp->kde_appsdir().copy();
    KMimeBind::initApplications( path.data() );
}

void KMimeType::errorMissingMimeType( const char *_type, KMimeType **_ptr )
{
    QString tmp = i18n( "Could not find mime type\n" );
    tmp += _type;
    
    QMessageBox::warning( 0, i18n( "KFM Error" ), tmp );
    
    *_ptr = defaultType;
}
    
void KMimeType::clearAll()
{
    KMimeBind::clearBindingList();
    
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
    QString tmp( _url );
    
    // Is it really a directory ?
    if ( KIOServer::isDir( _url ) > 0 )
    {
	// File on the local hard disk ?
	if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	{   
	    QString path = u.path();
	    if ( access( path, R_OK|X_OK ) < 0 )
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

    // Some of the *.kdelnk files have special hard coded bindings like
    // mouting/unmounting of devices.
    if ( tmp.length() > 7 && tmp.right(7) == ".kdelnk" )
	return kdelnkType;
    
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

    KMimeType *typ=0L;
    for ( typ = types->first(); typ != 0L; typ = types->next() )
    {
	QStrList& pattern = typ->getPattern();
	char *s;
	for ( s = pattern.first(); s != 0L; s = pattern.next() )
	{
	    int pattern_len = strlen( s );
            if (!pattern_len)
               continue;
	    int len = filename.length();	

	    if ( s[ pattern_len - 1 ] == '*' && len + 1 >= pattern_len )
	    {
		if ( strncmp( filename.data(), s, pattern_len - 1 ) == 0 )
		    return typ;
	    }
	    if ( s[ 0 ] == '*' && len + 1 >= pattern_len )
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
    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
    {
	QString path( u.path() );
	
	struct stat buff;
	// Can we make a stat ?
	if ( stat( path.data(), &buff ) == 0 )
	{
	    if ( S_ISFIFO( buff.st_mode ) )
		return PipeType;
	    if ( S_ISSOCK( buff.st_mode ) )
		return SocketType;
	    if ( S_ISCHR( buff.st_mode ) )
		return CDevType;
	    if ( S_ISBLK( buff.st_mode ) )
		return BDevType;  

	    if ( ( buff.st_mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) != 0 )
	    {
		FILE *f = fopen( path, "rb" );
		if ( f == 0 )
		    return execType;
		char buffer[ 10 ];
		int n = fread( buffer, 1, 2, f );
		fclose( f );
		if ( n < 0 )
		    return execType;
		buffer[ n ] = 0;
		// Is it a batchfile
		if ( strcmp( buffer, "#!" ) == 0 )
		    return batchType;
		// It is a binary executable
		return execType;
	    }
	}
    }
    
    return defaultType;
}

/**
 * TODO: Some of this code belonks in KDELnkMimeType and others.
 */
void KMimeType::getBindings( QStrList &_list, QList<QPixmap> &_pixlist, const char *_url, int _isdir )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return;

    // Used to store a new value for _url
    QString tmp;
    
    // Try to read the file as a [KDE Desktop Entry]
    KSimpleConfig *config = KMimeBind::openKConfig( _url );
    
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
		// The device is yet not mounted ...
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
			
			// This if clause is a hack for backward copatibility
			if ( stricmp( start, "Default" ) != 0 )
			{
			    QString s( i18n( "Mount" ) );
			    s.detach();
			    s += " ";
			    s += (const char*)start;
			    _list.append( s.data() );   
			    _pixlist.append( emptyPixmap );
			}
		    }			

		    // Add default mount binding
		    QString s( i18n( "Mount" ) );
		    _list.append( s.data() );   
		    _pixlist.append( emptyPixmap );
		}
		else
		{
		    QString s( i18n( "Unmount" ) );
		    s.detach();
		    _list.append( s.data() );
		    _pixlist.append( emptyPixmap );
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
		KURL u2( _url );
		u = u2;
	    }
	}
	delete config;
    }

    // A directory named dir.html for example does not have a binding to
    // netscape or arena => we check for directories first.
    // Are we really shure that it is a directory? In cases like
    // ftp://ftp.kde.org/pub/kde/qt we can not be shure.
    if ( _isdir == 1 )
    {
	KMimeBind *bind;
	for ( bind = folderType->firstBinding(); bind != 0L; bind = folderType->nextBinding() )
	{
	    _list.append( bind->getName() );
	    _pixlist.append( bind->getPixmap( true ) );
	}
    }
    // usual files
    else
    {
	KMimeType *typ = KMimeType::getMagicMimeType( _url );

// this makes it impossible to have a defaut binding (e.g. allfiles)
// without at least one mime binding  rjakob (rjakob@duffy1.franken.de)
//	if ( !typ->hasBindings() )
//	    return;

	KMimeBind *bind;
	for ( bind = typ->firstBinding(); bind != 0L; bind = typ->nextBinding() )
	{
	    _list.append( bind->getName() );
	    _pixlist.append( bind->getPixmap( true ) );
	}
	
	// Add all default bindings if we did not already do it
	if ( typ != defaultType )
	{
	    for ( bind = defaultType->firstBinding(); bind != 0L; bind = defaultType->nextBinding() )
	    {
	      _list.append( bind->getName() );
	      _pixlist.append( bind->getPixmap( true ) );
	    }
	}
    }
}

KMimeBind* KMimeType::findBinding( const char *_kdelnkName )
{
    /* Find a binding by its kdelnk name */
    KMimeBind *b;
    for ( b = bindings.first(); b != 0L; b = bindings.next() )
    {
	if ( strcmp( _kdelnkName, b->getKdelnkName() ) == 0L )
	    return b;
    }
    
    if (defaultType)
        // Also look in default bindings (allfiles, all)
        for ( b = defaultType->firstBinding(); b != 0L; b = defaultType->nextBinding() )
        {
            if ( strcmp( _kdelnkName, b->getKdelnkName() ) == 0L )
                return b;
        }
    
    return 0L;
}

/***************************************************************
 *
 * KFolderType
 *
 ***************************************************************/

const char* KFolderType::getPixmapFile( const char *_url, bool _mini )
{
    DIR *dp=NULL;
    struct dirent *ep=NULL;

    /**
     * This function is very tricky and dirty, but it is
     * designed to be as fast as possible.
     */

  /*
    QString decoded( _url );
    // Add the protocol if its is missing
    if ( decoded[0] == '/' )
	decoded.prepend( "file:" );
    KURL::decodeURL( decoded );
    */

    // Is this a file located in a tar archive ?
    // This is a preliminary and dirty check, but it
    // is fast!
    if ( strstr( _url, "#tar:/" ) != 0L )
    {
	// Now we check wether the preliminary test was right ...
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
		    // We return an folder icon. Otherwise
		    // we would get a folder icon with access denied!!!
		    return folderType->getPixmapFile( _mini );
	    }
	}
    }
    
    // Not a pure local file ? ( this is a quick hack, KURL is too slow )
    if ( *_url != '/' && ( strncmp( _url, "file:", 5 ) != 0 || strchr( _url, '#' ) != 0L ) )
	return KMimeType::getPixmapFile( _url, _mini );

    // Is trash 
    bool is_trash = false;
    
    if ( *_url != '/' )
    {
      QString path = _url + 5;
      KURL::decodeURL( path );
      if ( path[ path.length() - 1 ] != '/' )
	path += "/";
      if ( path == KFMPaths::TrashPath() )
	is_trash = true;
    }
    else if ( strcmp( _url, KFMPaths::TrashPath() ) == 0 )
      is_trash = true;
    
    if ( is_trash )
    {
	// "+5" to skip "file:", this is fast :-)
        dp = opendir( KFMPaths::TrashPath() );
	// Stephan: Congratulation Torben for this trick :-)
	// Torben: Thanks!
        ep=readdir( dp );
        ep=readdir( dp );      // ignore '.' and '..' dirent
	ep=readdir( dp );      
	bool empty = (ep == 0); // no third file
	// or third file is .directory and no fourth file
	if (!empty && !strcmp(ep->d_name, ".directory")) 
	    empty = (readdir(dp) == 0L);
	if ( empty ) // empty directory
        {
           pixmapFile2 = getIconPath( "kfm_trash.xpm", _mini );
	   closedir( dp );
	   return pixmapFile2.data();
         }
         else
         {
	   pixmapFile2 = getIconPath( "kfm_fulltrash.xpm", _mini );
	   closedir( dp );
	   return pixmapFile2.data();
         }
    }
    
    // Skip "file:" =>  "+5"
    QString n;
    if ( *_url != '/' )
    {
      n = _url + 5;
      KURL::decodeURL( n );
    }
    else 
      n = _url;
    
    if ( n[ n.length() - 1 ] != '/' )
	n += "/";
    n += ".directory";

    QFile f( n.data() );
    if ( !f.open( IO_ReadOnly ) )
	return KMimeType::getPixmapFile( _url, _mini );
    f.close();

    KSimpleConfig config( n, true );
    config.setGroup( "KDE Desktop Entry" );

    QString icon;
    if ( _mini )
      icon = config.readEntry( "MiniIcon" );
    else
      icon = config.readEntry( "Icon" );
    
    if ( icon.isEmpty() )
	return KMimeType::getPixmapFile( _url, _mini );

    pixmapFile2 = getIconPath( icon, _mini );

    return pixmapFile2.data();
}

QPixmap* KFolderType::getPixmap( const char *_url, bool _mini )
{
    const char *_pixmapFile = getPixmapFile( _url, _mini );
    pixmap = pixmapCache->find( _pixmapFile );
    if ( pixmap == 0L )
    {
	pixmap = new QPixmap;
	pixmap->load( _pixmapFile );
	pixmapCache->insert( _pixmapFile, pixmap );
    }
    
    return pixmap;
}

QString KFolderType::getComment( const char *_url )
{
    if ( strncmp( _url, "file:", 5 ) != 0 )
	return QString( i18n("Folder") );
  
    QString decoded( _url );
    KURL::decodeURL( decoded );
    
    QString n = decoded.data() + 5;
    if ( _url[ decoded.length() - 1 ] != '/' )
	n += "/";
    n += ".directory";

    QFile f( n.data() );
    if ( !f.open( IO_ReadOnly ) )
	  return QString( i18n("Folder") );
    
    f.close();

    KSimpleConfig config( n, true );
    config.setGroup( "KDE Desktop Entry" );
    
    QString com = config.readEntry( "Comment" );

    if ( com.isNull() )
	com = i18n("Folder");
    
    return QString( com.data() );
}

/***************************************************************
 *
 * KDELnkMimeType
 *
 ***************************************************************/

const char* KDELnkMimeType::getPixmapFile( const char *_url, bool _mini )
{
    // Try to read the file as a [KDE Desktop Entry]
    KSimpleConfig *config = KMimeBind::openKConfig( _url );
    
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
		if ( mp.isNull() )
		    pixmapFile2 = getIconPath( icon2, _mini );
		else
		    pixmapFile2 = getIconPath( icon, _mini );
		return pixmapFile2.data();
	    }
	}
	else
	{
	    QString icon = config->readEntry( "Icon" );
	    delete config;
	    if ( !icon.isEmpty() )
	    {
		pixmapFile2 = getIconPath( icon, _mini );
		return pixmapFile2.data();
	    }
	}
    }
    
    return KMimeType::getPixmapFile( _url, _mini );
}

QPixmap* KDELnkMimeType::getPixmap( const char *_url, bool _mini )
{
    const char *_pixmapFile = getPixmapFile( _url, _mini );
    pixmap = pixmapCache->find( _pixmapFile );
    if ( pixmap == 0L )
    {
	pixmap = new QPixmap;
	pixmap->load( _pixmapFile );
	pixmapCache->insert( _pixmapFile, pixmap );
    }
    
    return pixmap;
}

QString KDELnkMimeType::getComment( const char *_url )
{
  KSimpleConfig *config = KMimeBind::openKConfig( _url ); // kalle
    
    if ( config == 0L )
	return QString();
    
    QString erg = config->readEntry( "Comment" );
    return QString( erg.data() );
}

/***************************************************************
 *
 * KMimeBind
 *
 ***************************************************************/

KMimeBind::KMimeBind( const char * _kdelnkName, const char *_name, const char *_cmd, const char *_icon, bool _allowdefault, const char *_termOptions ) :
    name(_name), kdelnkName(_kdelnkName), cmd(_cmd), pixmap(0L), allowDefault(_allowdefault), termOptions(_termOptions)
{
    appendBinding( this );
  
    if ( _icon != 0L && *_icon != 0 )
    {
	pixmapFile = KMimeType::getIconPath( _icon );
	miniPixmapFile = KMimeType::getIconPath( _icon, true );
    }
}

QListIterator<KMimeBind> KMimeBind::bindingIterator()
{
  return QListIterator<KMimeBind>( *s_lstBindings );
}

KMimeBind* KMimeBind::findByName( const char *_name )
{
  QListIterator<KMimeBind> it = KMimeBind::bindingIterator();
  for ( ; it.current() != 0L; ++it )
  {
    if ( strcmp( it.current()->getName(), _name ) == 0 )
      return it.current();
  }

  return 0L;
}

QPixmap* KMimeBind::getPixmap( bool _mini )
{
    if ( pixmap )
	return pixmap;

    if ( _mini && miniPixmapFile.isEmpty() )
	return emptyPixmap;
    if ( !_mini && pixmapFile.isEmpty() )
	return emptyPixmap;
    
    if ( _mini )
	pixmap = KMimeType::pixmapCache->find( miniPixmapFile );
    else
	pixmap = KMimeType::pixmapCache->find( pixmapFile );

    if ( pixmap == 0L )
    {
	pixmap = new QPixmap;
	if ( _mini )
	{
	     pixmap->load( miniPixmapFile );
	     KMimeType::pixmapCache->insert( miniPixmapFile, pixmap );
	}
	else
	{
	     pixmap->load( pixmapFile );
	     KMimeType::pixmapCache->insert( pixmapFile, pixmap );
	}
    }
    
    return pixmap;
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
	  QString tmp( ep->d_name );	
	  
	  QString file( _path );
	  file += "/";
	  file += ep->d_name;

	  struct stat buff;
	  stat( file.data(), &buff );

	  if (S_ISDIR(buff.st_mode))
	      initApplications( file.data() );
	  else if (tmp.length() > 7 && tmp.right( 7 ) == ".kdelnk")
	  {
	      // The name of the application
	      QString app = tmp.left( tmp.length() - 7 );
	      
	      // Do we have read access ?
	      if ( access( file, R_OK ) == 0 )
    		{
		  KSimpleConfig config( file, true );
		  
		  config.setGroup( "KDE Desktop Entry" );
		  QString exec = config.readEntry( "Exec" );
		  QString name = config.readEntry( "Name" );
		  if ( name.isEmpty() )
		    name = app;
		  // An icon for the binary
		  QString app_icon = config.readEntry( "Icon" );
		  // The pattern to identify the binary
		  QString app_pattern = config.readEntry( "BinaryPattern" );
		  QString comment = config.readEntry( "Comment" );
		  // A ';' separated list of mime types
		  QString mime = config.readEntry( "MimeType" );
		  // Allow this program to be a default application for a mime type?
		  // For example gzip should never be a default for any mime type.
		  QString str_allowdefault = config.readEntry( "AllowDefault" );
		  bool allowdefault = true;
		  if ( str_allowdefault == "0" )
		    allowdefault = false;

		  // Read the terminal settings.
		  // termOptions will store both values (being 0L if Terminal=="0").
		  QString term = config.readEntry( "Terminal", "0" );
		  QString termOptions = config.readEntry( "TerminalOptions" );
		  if (term=="0") termOptions = 0L;
		  
		  // Define an icon for the program file perhaps ?
		  if ( !app_icon.isEmpty() && !app_pattern.isEmpty() )
    		    {
		      KMimeType *t;
		      types->append( t = new KMimeType( name.data(), app_icon.data() ) );
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
		  
		  // To which mime types is the application bound ?
		  int pos2 = 0;
		  int old_pos2 = 0;
		  while ( ( pos2 = mime.find( ";", pos2 ) ) != - 1 )
    		    {
		      // 'bind' is the name of a mime type
		      QString bind = mime.mid( old_pos2, pos2 - old_pos2 );
		      // Bind this application to all files/directories
		      if ( strcasecmp( bind.data(), "all" ) == 0 )
    			{
			  defaultType->append( new KMimeBind( app.data(), name.data(), exec.data(), app_icon, allowdefault, termOptions.data() ) );
			  folderType->append( new KMimeBind( app.data(), name.data(), exec.data(), app_icon, allowdefault, termOptions.data() ) );
    			}
		      else if ( strcasecmp( bind.data(), "alldirs" ) == 0 )
    			{
			  folderType->append( new KMimeBind( app.data(), name.data(), exec.data(), app_icon, allowdefault, termOptions.data() ) );
    			}
		      else if ( strcasecmp( bind.data(), "allfiles" ) == 0 )
    			{
			  defaultType->append( new KMimeBind( app.data(), name.data(), exec.data(), app_icon, allowdefault, termOptions.data() ) );
    			}
		      // Bind this application to a mime type
		      else
    			{
			  KMimeType *t = KMimeType::findByName( bind.data() );
			  if ( t == 0 )
			    QMessageBox::warning( 0L, i18n("ERROR"), 
						  i18n("Could not find mime type\n") + bind + "\n" + i18n("in ") + file );
			  else
    			    {
			      t->append( new KMimeBind( app.data(), name.data(), exec.data(), app_icon, allowdefault, termOptions.data() ) );
    			    }
    			}
		      
		      pos2++;
		      old_pos2 = pos2;
    		    }
		}
          }
	}
    }
  (void) closedir( dp );
}


bool KMimeBind::runBinding( const char *_url, const char *_binding )
{
    if ( _binding == 0L )
	_binding = "";

    KURL u( _url );
    if ( u.isMalformed() )
	return FALSE;

    KMimeType *typ = KMimeType::getMagicMimeType( _url );    
    return typ->runBinding( _url, _binding );
}

bool KMimeBind::runBinding( const char *_url )
{
    char getwd_buffer[PATH_MAX];

    KURL u( _url );
    if ( u.isMalformed() )
	return FALSE;

    QString quote1 = KIOServer::shellQuote( u.path() ).copy();
    QString quote2 = KIOServer::shellQuote( _url ).copy();
    
    QString f;
    QString ur;
    QString n = "";
    QString d = "";

// preset working directory to current working directory
    getcwd(getwd_buffer,PATH_MAX);
    QString workdir(getwd_buffer);

    f << "\"" << quote1 << "\"";
    ur << "\"" << quote2 << "\"";
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
        // set working directory to path of file
        // only for local files
	if (u.isLocalFile())
            workdir = tmp.left( i + 1 );
    }
    
    QString cmd = getCmd();
    cmd.detach();

    bool b_local_app = false;
    if ( cmd.find( "%u" ) == -1 )
      b_local_app = true;
    
    // Did the user forget to append something like '%f' ?
    // If so, then assume that '%f' is the right joice => the application
    // accepts only local files.
    if ( cmd.find( "%f" ) == -1 && cmd.find( "%u" ) == -1 && cmd.find( "%n" ) == -1 &&
	 cmd.find( "%d" ) == -1 )
      cmd.append( " %f" );
    
    int pos;
    // Replacing of '%f' is done in openWithOldApplication
    if ( !b_local_app )
      while ( ( pos = cmd.find( "%f" )) != -1 )
	cmd.replace( pos, 2, f.data() );
    while ( ( pos = cmd.find( "%u" )) != -1 )
      cmd.replace( pos, 2, ur.data() );
    while ( ( pos = cmd.find( "%n" )) != -1 )
      cmd.replace( pos, 2, n.data() );
    while ( ( pos = cmd.find( "%d" )) != -1 )
      cmd.replace( pos, 2, d.data() );

    while ( ( pos = cmd.find( "%c" ) ) != -1 )
    {
	if ( !name.isEmpty() )
	    cmd.replace( pos, 2, name.data() );
	else
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
    }        

    QString icon = pixmapFile.data();
    if (!icon.isEmpty())
      icon.prepend("-icon ");
    while ( ( i = cmd.find( "%i" ) ) != -1 )
      cmd.replace( i, 2, icon.data());
    QString miniicon = miniPixmapFile.data();
    if (!miniicon.isEmpty())
      miniicon.prepend("-miniicon ");
    while ( ( i = cmd.find( "%m" ) ) != -1 )
      cmd.replace( i, 2, miniicon.data());

    if ( !termOptions.isNull() )
    {
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "KFM Misc Defaults" );
	QString termCmd = config->readEntry( "Terminal", DEFAULT_TERMINAL);
	termCmd.detach();
	if ( termCmd.isNull() )
	{
	    warning(i18n("ERROR: No Terminal Setting"));
	    return TRUE;
	}
	termCmd += " ";
	if ( !termOptions.isEmpty() )
	{
	    termCmd += termOptions.data();
	    termCmd += " ";
	}
	termCmd += "-e ";
	termCmd += cmd.data();
        cmd = termCmd.copy();
    }

    // The application accepts only local files ?
    if ( b_local_app )
    {
      QStrList list;
      list.append( _url );
      printf("openWithOldApplication(%s,...,%s)\n",
      	cmd.data(),workdir.data());
      openWithOldApplication( cmd, list, workdir.data() );      
      return TRUE;
    }

    printf("KDELnkMimeType::runAsApplication starts runCmd(%s,%s)\n",
	cmd.data(),workdir.data());
    runCmd( cmd.data(), workdir.data() );
    return TRUE;
}

// new function with the working directory
void KMimeBind::runCmd( const char *_exec, QStrList &_args, const char *_workdir )
{
    char **argv = new char*[ _args.count() + 3 ];
    char* s;
    int rc;

    argv[0] = (char*)_exec;
    
    int i = 1;
    for ( s = _args.first(); s != 0L; s = _args.next() )
	argv[ i++ ] = (char*)s;
    argv[ i ] = 0L;

    int pid;
    if ( ( pid = fork() ) == 0 )
    {    
        // change to the working directory if set
	if (_workdir) {
            rc=chdir(_workdir);
            if (rc) printf("chdir(%s) failed : %d\n",_workdir,rc);
        }
        // --- The following detaches the program's output from kfm
        // Due to a X11 bug found by Stefan Westerfeld <stefan@space.twc.de>
        // if one runs a ncurses program (e.g. less), it makes X hang
        struct rlimit rlp;
        getrlimit(RLIMIT_NOFILE, &rlp); // getdtablesize() equivalent.
        int fd = rlp.rlim_cur;
        while( fd >= 0 ) {
            close( fd );
            fd--;
        }
        open( "/dev/null", O_RDWR );
        dup( 0 );
        dup( 0 );
        // --- End of detach patch. David Faure.

	execvp( argv[0], argv );
	QString txt = i18n("Could not execute program\n");
	txt += argv[0];

	// Run this program if something went wrong
	char* a[ 3 ];
	a[ 0 ] = "kfmwarn";
	a[ 1 ] = txt.data();
	a[ 2 ] = 0L;
	execvp( "kfmwarn", a );

	exit( 1 );
    }
    delete [] argv;
}

void KMimeBind::runCmd( const char *_cmd, const char *_workdir )
{
    int rc;

  // printf("CMD='%s'\n",_cmd );
    
    char *cmd = new char[ strlen( _cmd ) + 1 ];
    strcpy( cmd, _cmd );
    
    QStrList args;
    
    char *p2 = 0L;
    char *p = cmd;
    // Skip leading spaces
    while ( *p == ' ' ) p++;
    // Iterate over all args in the string
    do
    {
	// Found a quoted string ?
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
    // Append the rest
    if ( strlen( p ) > 0 )
    {
	QString tmp = KIOServer::shellUnquote( p ).copy();
	args.append( tmp );
    }
        
    char **argv = new char*[ args.count() + 3 ];
    char* s;
    int i = 0;
    for ( s = args.first(); s != 0L; s = args.next() )
	argv[ i++ ] = (char*)s;
    argv[ i ] = 0L;
    
    if ( i == 0 )
    {
	warning("0 sized command, '%s'\n", _cmd );
	return;
    }
    
    /* printf("Running '%s'\n",argv[0] );
    for ( int j = 0; j < i; j++ )
	printf("ARG: '%s'\n",argv[j]); */
    
    int pid;
    if ( ( pid = fork() ) == 0 )
    {    
        // change to the working directory if set
	if (_workdir) {
            rc=chdir(_workdir);
            if (rc) printf("chdir(%s) failed : %d\n",_workdir,rc);
        }
        // --- The following detaches the program's output from kfm
        // Due to a X11 bug found by Stefan Westerfeld <stefan@space.twc.de>
        // if one runs a ncurses program (e.g. less), it makes X hang
        struct rlimit rlp;
        getrlimit(RLIMIT_NOFILE, &rlp); // getdtablesize() equivalent.
        int fd = rlp.rlim_cur;
        while( fd >= 0 ) {
            close( fd );
            fd--;
        }
        open( "/dev/null", O_RDWR );
        dup( 0 );
        dup( 0 );
        // --- End of detach patch. David Faure.

	execvp( argv[0], argv );
	QString txt = i18n("Could not execute program\n");
	txt += argv[0];

	// Run this program if something went wrong
	char* a[ 3 ];
	a[ 0 ] = "kfmwarn";
	a[ 1 ] = txt.data();
	a[ 2 ] = 0L;
	execvp( "kfmwarn", a );

	exit( 1 );
    }
    delete [] argv;
    delete [] cmd;
}

KSimpleConfig* KMimeBind::openKConfig( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return 0L;
    if ( strcmp( u.protocol(), "file" ) != 0 )
	return 0L;
    
    QString decoded( u.path() );
    
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
       
    /* QFile *file = new QFile( decoded );
    if ( !file->open( IO_ReadOnly ) )
    {
	delete file;
	return 0L;
    }
    file->close(); */

    KSimpleConfig *config = new KSimpleConfig( decoded, true );
    config->setGroup( "KDE Desktop Entry" );
    return config;
}

/*****************************************************************************
 *****************************************************************************
 **
 ** The following methods are used to open the documents, applications
 ** or whatever.
 **
 *****************************************************************************
 *****************************************************************************/

/*****************************************************************************
 *
 * KMimeType
 *
 *****************************************************************************/

bool KMimeType::run( const char *_url )
{
    // Do we have some default binding (set in the applnk file) ?
    if ( !defaultBinding.isEmpty() )
    {
	KMimeBind* bind = findBinding( defaultBinding );
	if ( bind )
	    return bind->runBinding( _url );
    }
    
    // Take first binding which is allowed as default
    KMimeBind *bind;
    for ( bind = bindings.first(); bind != 0L; bind = bindings.next() )
    {
	if ( bind->isAllowedAsDefault() )
	    return bind->runBinding( _url );
    }

    if (defaultType)
        // Also look in all default bindings (allfiles, all)
        for ( bind = defaultType->firstBinding(); bind != 0L; bind = defaultType->nextBinding() )
        {
            if ( bind->isAllowedAsDefault() )
                return bind->runBinding( _url );
        }

    return FALSE;
}

bool KMimeType::runBinding( const char *_url, const char *_binding )
{
    KMimeBind *bind;
    // Iterate over all applications bound to the mime type
    for ( bind = firstBinding(); bind != 0L; bind = nextBinding() )
    {
	// Is it the one we want ?
	if ( strcmp( bind->getName(), _binding ) == 0 )
	    return bind->runBinding( _url );
    }
    // Repeat the loop for default bindings before we say we have none. rjakob
    for ( bind = defaultType->firstBinding(); bind != 0L; bind = defaultType->nextBinding() )
    {
	// Is it the one we want ?
	if ( strcmp( bind->getName(), _binding ) == 0 )
	    return bind->runBinding( _url );
    }

    QString tmp;
    tmp.sprintf( "%s\n%s", i18n( "Could not find binding" ), _binding );
    QMessageBox::warning( 0L, i18n( "KFM Error" ), tmp );
    // Tell, that we have done the job since there is not more to do except showing
    // the above error message.
    return TRUE;
}

bool KMimeType::runAsApplication( const char *, QStrList * )
{
    QMessageBox::warning( 0L, i18n( "KFM Error" ),
			  i18n( "This is a document!\nNot an application" ) );
    // Tell, that we have done the job since there is not more to do except showing
    // the above error message.
    return TRUE;
}

/*****************************************************************************
 *
 * ExecutableMimeType
 *
 *****************************************************************************/

bool ExecutableMimeType::run( const char *_url )
{
    KURL u( _url );
    if ( strcmp( u.protocol(), "file" ) != 0 || u.hasSubProtocol() )
    {
	QMessageBox::warning( 0, i18n( "KFM Error" ), 
			      i18n( "Can only start executables on local disks\n" ) );
	return TRUE;
    }
			     
    QString cmd;
    cmd << "\"" << KIOServer::shellQuote( u.path() ) << "\"";

    printf("ExecutableMimeType starts runCmd(%s)\n",cmd.data());
    KMimeBind::runCmd( cmd );

    return TRUE;
}

bool ExecutableMimeType::runAsApplication( const char *_url, QStrList *_arguments )
{
    KURL u( _url );
    if ( strcmp( u.protocol(), "file" ) != 0 || u.hasSubProtocol() )
    {
	QMessageBox::warning( 0, i18n( "KFM Error" ), 
			      i18n( "Can only start executables on local disks\n" ) );
	return TRUE;
    }
    
    QString decodedURL( _url );
    KURL::decodeURL( decodedURL );

    QString exec = KIOServer::shellQuote( u.path() );
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
    
    printf("runAsApplication starts runCmd(%s)\n",cmd.data());
    KMimeBind::runCmd( cmd );
    // system( cmd.data() );
    return TRUE;
}

/*****************************************************************************
 *
 * KDELnkMimeType
 *
 *****************************************************************************/

bool KDELnkMimeType::run( const char *_url )
{
    KURL u( _url );
    KSimpleConfig *config = 0L;
    // Is it a "[KDE Desktop Entry]" file and do we want to open it ?
    // ... but only if it is on the local hard disk!
    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	  config = KMimeBind::openKConfig( _url );
    else
    {
	QMessageBox::critical( 0L, i18n( "KFM Error" ),
		     i18n( "Can work with *.kdelnk files only\non local hard disk" ) );
	// Say: Yes we have done the job. That is not quite right, but
	// we want to stop this here, before KFM tries some stupid things :-)
	return TRUE;
    }

    // Could we open the config file ?
    if ( config == 0L )
    {
	QString tmp;
	ksprintf(&tmp, i18n("Could not access\n%s"), _url);
	QMessageBox::critical( 0L, i18n( "KFM Error" ), tmp );
	// Say: Yes we have done the job. That is not quite right, but
	// we want to stop this here, before KFM tries some stupid things :-)
	return TRUE;
    }

    QString typ = config->readEntry( "Type" );
    
    // Must we start an executable ?
    if ( typ == "Application" )
    {
	delete config;
	return runAsApplication( _url, 0L );
    }
    // Is it a device like CDROM or Floppy ?
    else if ( typ == "FSDevice" )
    {
	delete config;
	// Try to mount&open the device
	return runBinding( _url, klocale->getAlias( ID_STRING_OPEN ) );
    }
    else if ( strcmp( typ, "Link" ) == 0 )
    {
	QString url = config->readEntry( "URL" );

	if ( url.isEmpty() )
	{
	    QMessageBox::warning( 0, i18n( "KFM Error" ),
				  i18n( "The \"Link=....\" entry is empty" ) );
	    delete config;
	    return TRUE;
	}
	
	KFMExec *exec = new KFMExec;
	// Try to open the URl somehow
	exec->openURL( url );
	delete config;
	return TRUE;
    }
    else if ( strcmp( typ, "MimeType" ) == 0 )
    {
	(void)new Properties ( _url );
	delete config;
	return TRUE;
    }
    else
    {
	delete config;
	QMessageBox::warning( 0, i18n( "KFM Error" ),
			      i18n( "The config file has no \"Type=...\" line" ) );
	// Just say: The job is done
	return TRUE;
    }
    
    delete config;
    return FALSE;
}

bool KDELnkMimeType::runAsApplication( const char *_url, QStrList *_arguments )
{
    KURL u2( _url );
    KSimpleConfig *config = 0L;
    // Is it a "[KDE Desktop Entry]" file and do we want to open it ?
    // ... but only if it is on the local hard disk!
    if ( strcmp( u2.protocol(), "file" ) == 0 && !u2.hasSubProtocol() )
	  config = KMimeBind::openKConfig( _url );
    else
    {
	QMessageBox::critical( 0L, i18n( "KFM Error" ),
		     i18n( "Can work with *.kdelnk files only\non local hard disk" ) );
	// Say: Yes we have done the job. That is not quite right, but
	// we want to stop this here, before KFM tries some stupid things :-)
	return TRUE;
    }
    
    // Could we open the config file ?
    if ( config == 0L )
    {
	QString tmp;
	tmp.sprintf( "%s\n%s", i18n( "Could not access" ), _url );
	QMessageBox::critical( 0L, i18n( "KFM Error" ), tmp );
	// Say: Yes we have done the job. That is not quite right, but
	// we want to stop this here, before KFM tries some stupid things :-)
	return TRUE;
    }
    
    QString exec = config->readEntry( "Exec" );
    QString name = config->readEntry( "Name" );
    QString term = config->readEntry( "Terminal" );
    QString termOptions = config->readEntry( "TerminalOptions" );
    QString icon = config->readEntry( "Icon", "" );
    QString miniicon = config->readEntry( "MiniIcon", "" );
    
    // At least the executable is needed!
    if ( exec.isEmpty() )
    {
	QMessageBox::critical( 0L, i18n( "KFM Error" ),
		     i18n( "This file does not contain an\nExec=....\nentry. Edit the Properties\nto solve the problem" ) );

	// Say: Yes we have done the job. That is not quite right, but
	// we want to stop this here, before KFM tries some stupid things :-)
	return TRUE;
    }
    
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
	exec.detach();

	if ( !name.isEmpty() ) 
	    exec.replace( i, 2, name.data() );
	else 
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
    }     
    if (!icon.isEmpty()) {
      icon.detach();
      icon.prepend("-icon ");
    }

    while ( ( i = exec.find( "%i" ) ) != -1 ) {
      exec.detach();
      exec.replace( i, 2, icon.data());
    }

    if (!miniicon.isEmpty()) {
      miniicon.detach();
      miniicon.prepend("-miniicon ");
    } 
    while ( ( i = exec.find( "%m" ) ) != -1 )
      exec.replace( i, 2, miniicon.data());
    
    if ( term == "1" )
    {
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "KFM Misc Defaults" );
	QString cmd = config->readEntry( "Terminal", DEFAULT_TERMINAL);
	cmd.detach();
	if ( cmd.isNull() )
	{
	    warning(i18n("ERROR: No Terminal Setting"));
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
	printf("KDELnkMimeType::runAsApplication starts runCmd(%s)\n",
		cmd.data());
	KMimeBind::runCmd( cmd );
    }
    else
    {
	QString cmd = exec.data();
	printf("KDELnkMimeType::runAsApplication starts runCmd(%s)\n",
		cmd.data());
	KMimeBind::runCmd( cmd );
    }

    return TRUE;
}

bool KDELnkMimeType::runBinding( const char *_url, const char *_binding )
{
    KURL u( _url );
    KSimpleConfig *config = 0L;
    // Is it a "[KDE Desktop Entry]" file and do we want to open it ?
    // ... but only if it is on the local hard disk!
    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	  config = KMimeBind::openKConfig( _url );
    else
    {
	QMessageBox::critical( 0L, i18n( "KFM Error" ),
		     i18n( "Can work with *.kdelnk files only\non local hard disk" ) );
	// Say: Yes we have done the job. That is not quite right, but
	// we want to stop this here, before KFM tries some stupid things :-)
	return TRUE;
    }

    // Could we open the config file ?
    if ( config == 0L )
    {
	QString tmp;
	tmp.sprintf( "%s\n%s", i18n( "Could not access" ), _url );
	QMessageBox::critical( 0L, i18n( "KFM Error" ), tmp );
	// Say: Yes we have done the job. That is not quite right, but
	// we want to stop this here, before KFM tries some stupid things :-)
	return TRUE;
    }

    QString typ = config->readEntry( "Type" );
    
    if ( typ == "FSDevice" )
    {
	QString point = config->readEntry( "MountPoint" );
	QString dev = config->readEntry( "Dev" );
	if ( !dev.isNull() )
	{
	    if ( strcmp( _binding, i18n( "Unmount" ) ) == 0 )
	    {
		KIOJob * job = new KIOJob();
		// job->unmount( point.data() );
		// Patch ( unmount the device )
		job->unmount( dev.data() );
		delete config;
		return TRUE;
	    }
	    else if ( strncmp( _binding, i18n( "Mount" ),
			       strlen( i18n( "Mount" ) ) ) == 0 )
	    {
		QString readonly = config->readEntry( "ReadOnly" );
		bool ro = FALSE;
		if ( !readonly.isNull() )
		    if ( readonly == '1' )
			ro = true;
		
		KIOJob *job = new KIOJob;
		
		// The binding is named 'Mount FSType' => length >=5
		QString tmp;
		tmp.sprintf( "%s Default", i18n( "Mount" ) );
		if ( stricmp( _binding, tmp.data() ) == 0 ||
		     strcmp( _binding, i18n( "Mount" ) ) == 0 )
		    job->mount( false, 0L, dev, 0L );
		else
		{
		    if ( point.isEmpty() )
		    {
			delete config;
			QMessageBox::critical( 0L, i18n( "KFM Error" ),
					       i18n( "No mount point in *.kdelnk file" ) );
			return true;
		    }
			    
		    job->mount( ro, _binding + 6, dev, point );
		}
		delete config;
		return TRUE;
	    }
	    else if ( strcmp( _binding, klocale->getAlias(ID_STRING_OPEN) ) == 0 )
	    {
		QString mp = KIOServer::findDeviceMountPoint( dev.data() );
		// Is the device already mounted ?
		if ( !mp.isNull() )
		{
		    QString mp2 = "file:";
		    mp2 += mp;
		    // Open a new window
		    KfmGui *m = new KfmGui( 0L, 0L, mp2.data() );
		    m->show();
		    delete config;
		    return TRUE;
		}
		else
		{
		    QString readonly = config->readEntry( "ReadOnly" );
		    bool ro = FALSE;
		    if ( !readonly.isNull() )
			if ( readonly == '1' )
			    ro = true;
		    
		    (void) new KFMAutoMount( ro, 0L, dev, 0L );

		    delete config;
		    return TRUE;
		}
	    }
	}    
    }
    
    if ( config )
	delete config;

    return KMimeType::runBinding( _url, _binding );
}

/***********************************************************************
 ***********************************************************************
 **
 ** Utility classes
 **
 ***********************************************************************
 ***********************************************************************/


/***********************************************************************
 *
 * Utility classes
 *
 ***********************************************************************/

KFMAutoMount::KFMAutoMount( bool _readonly, const char *_format, const char *_device, const char *_mountpoint )
{
    device = _device;
    
    job = new KIOJob();
    connect( job, SIGNAL( finished( int ) ), this, SLOT( slotFinished( int ) ) );
    connect( job, SIGNAL( errorFilter( int, const char*, int ) ), this, SLOT( slotError( int, const char*, int ) ) );
    
    if ( !_format )
	job->mount( false, 0L, _device, 0L );
    else
	job->mount( _readonly, _format, _device, _mountpoint );
}

void KFMAutoMount::slotFinished( int )
{
    job->setAutoDelete( TRUE );

    QString mp = KIOServer::findDeviceMountPoint( device );
    KfmGui *m = new KfmGui( 0L, 0L, mp );
    m->show();

    delete this;
}

void KFMAutoMount::slotError( int _kioerror, const char *_text, int _errno )
{
    job->setAutoDelete( TRUE );
    disconnect(job,0,this,0); // we don't want slotFinished to be called
    delete this;
}

#include "kbind.moc"
