#include <qstrlist.h>
#include <qpushbt.h>
#include <qpainter.h>
#include <qdict.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "kioserver_ipc.h"
#include "kiojob.h"
#include "kbind.h"
#include "kioslave/kio_errors.h"
#include "kmsgwin.h"
#include "krenamewin.h"

QList<KIOJob> KIOJob::jobList;

KIOJob::KIOJob( int _id )
{
    server = KIOServer::getKIOServer();
    slave = 0L;
    dlg = 0L;
    bDisplay = TRUE;
    progress = 0L;
    line1 = 0L;
    line2 = 0L;
    line3 = 0L;
    globalNotify = TRUE;
    id = _id;
    overwriteExistingFiles = FALSE;
    
    jobList.setAutoDelete( FALSE );
    jobList.append( this );
}

KIOJob::~KIOJob()
{
    jobList.removeRef( this );
}

void KIOJob::mkdir( const char *_url )
{
    printf("Making '%s'\n",_url);
    
    action = KIOJob::JOB_MKDIR;
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
	printf("ERROR: Malformed URL\n");
	return;
    }
    
    action = KIOJob::JOB_MKDIR;
    
    mkdirURL = _url;
    mkdirURL.detach();
    
    notifyList.append( u.directoryURL() );

    server->getSlave( this );
}

void KIOJob::list( const char *_url, bool _reload )
{
    action = KIOJob::JOB_LIST;
    
    lstURL = _url;
    lstURL.detach();
    printf("LIST: Got '%s'\n",lstURL.data());
 
    KIODirectory *dir = 0L;
    if ( !_reload )
	dir = server->getDirectory( _url );
    
    if ( dir != 0L )
    {
	KIODirectoryEntry *e;
	for ( e = dir->first(); e != 0L; e = dir->next() )
	    emit newDirEntry( id, e );
	done();
	return;
    }

    server->getSlave( this );
}

void KIOJob::mount( bool _ro, const char *_fstype, const char* _dev, const char *_point )
{
    if ( _dev == 0L )
    {
	printf("INTERNAL ERROR: You must at least specify the device for mounting\n");
	exit(1);
    }
    
    action = KIOJob::JOB_MOUNT;
    
    mntReadOnly = _ro;
    mntFSType = _fstype;
    mntFSType.detach();
    mntDev = _dev;
    mntDev.detach();
    mntPoint = _point;
    mntPoint.detach();

    // Get the mount directory
    /*
    if ( _point == 0L )
    {
	FILE *f = setmntent( "/etc/fstab", 0 );
	if ( f == 0 )
	    printf("WARNING: Could not access /etc/fstab\n");
	else
	{
	    bool bend = FALSE;
	    while ( !bend )
	    {
		struct mntent * m = getmntent( f );
		if ( m != 0L )
		{
		    if ( strcmp( m->mnt_fsname, _dev ) == 0 )
		    {
			QString n = "file:";
			n += m->mnt_dir;
			notifyList.append( n.data() );
			bend = TRUE;
		    }
		}
		else
		    bend = TRUE;
	    }
	    endmntent( f );
	}
    }
    else
    {
	QString n = "file:";
	n += _point;
	notifyList.append( n.data() );
    } */

    QString n = KIOServer::findDeviceMountPoint( _dev, "/etc/fstab" );
    if ( !n.isNull() )
    {
	QString n2 = "file:";
	n2 += n.data();
	notifyList.append( n2.data() );
    }
    
    server->getSlave( this );
}

void KIOJob::unmount( const char *_point )
{
    action = KIOJob::JOB_UNMOUNT;
    
    mntPoint = _point;
    mntPoint.detach();

    QString n2 = "file:";
    n2 += _point;
    notifyList.append( n2.data() );

    server->getSlave( this );
}

void KIOJob::link( QStrList & _src_url_list, const char *_dest_dir_url )
{
    action = KIOJob::JOB_LINK;

    cmSrcURLList.copy( _src_url_list );
    cmDestURLList.clear();

    // Fill cmDestURLList
    char *p;
    for ( p = cmSrcURLList.first(); p != 0L; p = cmSrcURLList.next() )
    {
	KURL su( p );
	
	QString d = _dest_dir_url;
	d.detach();
	if ( d.length() > 0 && d.data()[ d.length() - 1 ] != '/' )
	    d += "/";
	QString s = KIOServer::getDestNameForLink( p );
	d += s.data();

	cmDestURLList.append( d.data() );
    }

    link();
}

void KIOJob::link( const char *_src_url, const char *_dest_url )
{
    action = KIOJob::JOB_LINK;

    cmSrcURLList.clear();
    cmDestURLList.clear();    
    cmSrcURLList.append( _src_url );
    cmDestURLList.append( _dest_url );

    link();
}

void KIOJob::link()
{
    char *p;
    char *p2 = cmDestURLList.first();
    for ( p = cmSrcURLList.first(); p != 0L; p = cmSrcURLList.next() )
    {
	KURL su( p );
	KURL du( p2 );
	
	if ( notifyList.find( du.directoryURL() ) == -1 )
	    notifyList.append( du.directoryURL() );
	
	if ( su.isMalformed() )
	{
	    printf("ERROR: Malformed URL '%s'\n",p);
	}
	else if ( du.isMalformed() )
	{
	    printf("ERROR: Malformed URL '%s'\n",p2);
	}	
	// I can only make links on the local file system.
	else if ( strcmp( du.protocol(), "file" ) != 0L )
	{
	    printf("ERROR: Can only make links on local file system\n");
	}
	else
	{
	    // Do we link a file on the local disk?
	    if ( strcmp( su.protocol(), "file" ) == 0 )
	    {
		if ( symlink( su.path(), du.path() ) == -1 )
		    printf("ERROR: Could not make symlink to %s\n",du.path() );
	    }
	    // Make a link to a file in a tar archive, ftp, http or what ever
	    else
	    {
		QFile f( du.path() );
		if ( f.open( IO_ReadWrite ) )
		{
		    QTextStream pstream( &f );
		    KConfig config( &pstream );
		    config.setGroup( "KDE Desktop Entry" );
		    config.writeEntry( QString("URL"), QString(p2) );
		    config.writeEntry( QString("Type"), QString("Link"));
		    if ( strcmp( du.protocol(), "ftp" ) == 0 )
			config.writeEntry( QString("Icon"), QString("ftp.xpm") );
		    else if ( strcmp( du.protocol(), "http" ) == 0 )
			config.writeEntry( QString("Icon"), QString("www.xpm") );
		    else if ( strcmp( du.protocol(), "info" ) == 0 )
			config.writeEntry( QString("Icon"), QString("info.xpm") );
		    else
			config.writeEntry( QString("Icon"), QString(KFileType::getDefaultPixmap()) );
		    config.sync();
		    if ( globalNotify )
			KIOServer::sendNotify( p2 );
		    emit notify( id, p2 );
		}
		else
		    printf(" ERROR: Could not write to %s\n",p);
	    }
	}
	p2 = cmDestURLList.next();
    }

    // We dont need a slave, so quit
    done();
}

void KIOJob::copy( QStrList & _src_url_list, const char *_dest_dir_url )
{
    action = KIOJob::JOB_COPY;
    
    cmSrcURLList.copy( _src_url_list );
    cmDestURLList.clear();

    // Fill cmDestURLList
    char *p;
    for ( p = cmSrcURLList.first(); p != 0L; p = cmSrcURLList.next() )
    {
	KURL su( p );
	
	QString d = _dest_dir_url;
	d.detach();
	if ( d.length() > 0 && d.data()[ d.length() - 1 ] != '/' )
	    d += "/";
	if ( strcmp( su.protocol(), "tar" ) == 0 )
	    d += su.filename( TRUE );
	else
	    d += su.filename();

	// HTTP names may end with '/', so append 'index.html' in this case.
	if ( d.length() > 0 && d.data()[ d.length() - 1 ] == '/' )
	    d += "index.html";

	cmDestURLList.append( d.data() );
    }

    copy();
}

void KIOJob::copy( const char *_src_url, const char *_dest_url )
{
    action = KIOJob::JOB_COPY;

    cmSrcURLList.clear();
    cmDestURLList.clear();    
    cmSrcURLList.append( _src_url );
    cmDestURLList.append( _dest_url );

    copy();
}

void KIOJob::copy()
{
    // Recursive directory
    char *p;
    char *p2 = cmDestURLList.first();
    for ( p = cmSrcURLList.first(); p != 0L; p = cmSrcURLList.next() )
    {
	KURL su( p );
	KURL du( p2 );

	if ( strcmp( su.protocol(), "file" ) == 0 && strcmp( du.protocol(), "file" ) == 0 )
	{
	    struct stat buff;
	    stat( su.path(), &buff );
	    if ( S_ISDIR( buff.st_mode ) )
	    {
		printf("Making diretory '%s'\n",du.path());
		
		if ( ::mkdir( du.path(), S_IRWXU ) == -1 )
		    if ( errno != EEXIST )
		    {
			printf("ERROR: Could not make directory\n");
			return;
		    }
		
		DIR *dp;
		struct dirent *ep;
		dp = opendir( su.path() );
		if ( dp == NULL )
		{
		    printf("ERROR: Could not access directory '%s'\n", p );
		    return;
		}
		
		while ( ep = readdir( dp ) )
		{
		    if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
		    {
			QString s = p;
			s.detach();
			if ( s.length() > 0 && s.data()[ s.length() - 1 ] != '/' )
			    s += "/";
			s += ep->d_name;
			
			QString d = p2;
			d.detach();
			if ( d.length() > 0 && d.data()[ d.length() - 1 ] != '/' )
			    d += "/";
			d += ep->d_name;
			
			cmSrcURLList.append( s.data() );
			cmDestURLList.append( d.data() );
		    }
		}
		
		// Remove directories from the copy list
		cmSrcURLList.removeRef( p );
		cmDestURLList.removeRef( p2 );
	    }
	}
	
	p2 = cmDestURLList.next();
    }
    
    cmCount = cmSrcURLList.count();

    char *s;
    for (s = cmDestURLList.first(); s != 0L; s = cmDestURLList.next() )
    {
	KURL u( s );
	if ( notifyList.find( u.directoryURL() ) == -1 )
	    notifyList.append( u.directoryURL() );
    }

    server->getSlave( this );
}

void KIOJob::move( const char *_src_url, const char *_dest_url )
{
    action = KIOJob::JOB_MOVE;

    cmSrcURLList.clear();
    cmDestURLList.clear();    
    cmSrcURLList.append( _src_url );
    cmDestURLList.append( _dest_url );

    move();
}

void KIOJob::move( QStrList & _src_url_list, const char *_dest_dir_url )
{
    action = KIOJob::JOB_MOVE;
    
    cmSrcURLList.copy( _src_url_list );
    cmDestURLList.clear();

    // Fill cmDestURLList
    char *p;
    for ( p = cmSrcURLList.first(); p != 0L; p = cmSrcURLList.next() )
    {
	KURL su( p );
	
	QString d = _dest_dir_url;
	d.detach();
	if ( d.length() > 0 && d.data()[ d.length() - 1 ] != '/' )
	    d += "/";
	if ( strcmp( su.protocol(), "tar" ) == 0 )
	    d += su.filename( TRUE );
	else
	    d += su.filename();
	
	cmDestURLList.append( d.data() );
    }

    move();
}

void KIOJob::move()
{
    tmpDelURLList.clear();
    
    // Recursive directory
    QListIterator<char> itSrc( cmSrcURLList );
    QListIterator<char> itDest( cmDestURLList );    
    char *p;
    char *p2 = cmDestURLList.first();
    for ( ; itSrc.current(); ++itSrc )
    {
	p = itSrc.current();
	p2 = itDest.current();
	
	KURL su( p );
	KURL du( p2 );

	int i = 1;
	if ( strcmp( su.protocol(), "file" ) == 0 && strcmp( du.protocol(), "file" ) == 0 )
	{
	    // Does the file already exist ?
	    struct stat buff;
	    if ( stat( du.path(), &buff ) == 0 && !overwriteExistingFiles )
	    {
		KRenameWin *r = new KRenameWin( 0L, p, p2, TRUE );
		int button = r->exec();
		if ( button == 0 ) // Overwrite 
		{
		}
		else if ( button == 1 ) // Overwrite All
		    overwriteExistingFiles = TRUE;
		else if ( button == 2 ) // Skip
		{
		    cmSrcURLList.removeRef( p );
		    cmDestURLList.removeRef( p2 );
		    continue;
		}
		else if ( button == 3 ) // Rename
		    du = r->getNewName();
		else if ( button == 4 ) // Cancel
		{
		    delete r;
		    done();
		    return;
		}
		delete r;
	    }
	    
	    i = rename( su.path(), du.path() );
	}
	
	// If we tried to move across devices, we have to copy and delete, otherwise
	// the task is already done. Try this only, if src and dest are in the local
	// file system.
	if ( i == -1 && errno == EXDEV )
	{
	    if ( errno == EXDEV )
	    {
		struct stat buff;
		stat( su.path(), &buff );
		if ( S_ISDIR( buff.st_mode ) )
		{
		    printf("Making diretory '%s'\n",du.path());
	    
		    if ( ::mkdir( du.path(), S_IRWXU ) == -1 )
			if ( errno != EEXIST )
			{
			    printf("ERROR: Could not make directory\n");
			    return;
			}

		    DIR *dp;
		    struct dirent *ep;
		    dp = opendir( su.path() );
		    if ( dp == NULL )
		    {
			printf("ERROR: Could not access directory '%s'\n", p );
			return;
		    }
		    
		    while ( ep = readdir( dp ) )
		    {
			if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
			{
			    QString s = p;
			    s.detach();
			    if ( s.length() > 0 && s.data()[ s.length() - 1 ] != '/' )
				s += "/";
			    s += ep->d_name;
			    
			    QString d = du.url();
			    d.detach();
			    if ( d.length() > 0 && d.data()[ d.length() - 1 ] != '/' )
				d += "/";
			    d += ep->d_name;
			    
			    cmSrcURLList.append( s.data() );
			    cmDestURLList.append( d.data() );
			}
		    }

		    // Remove the directory from the copy list
		    // ... but delete it at the end
		    tmpDelURLList.append( p );
		    cmSrcURLList.removeRef( p );
		    cmDestURLList.removeRef( p2 );
		}
	    }
	    else
	    {
		printf("ERROR: Could not move '%s'\n",p );
		return;
	    }
	}
	// We moved the files already, so take them from the list
	else if ( i == 0 )
	{
	    printf("?????????????????? Already moved '%s' '%s'\n",p,p2 );
	    
	    cmSrcURLList.removeRef( p );
	    cmDestURLList.removeRef( p2 );
	    // ... but send some notifies afterwards
	    if ( notifyList.find( su.directoryURL( FALSE ) ) == -1 )
		notifyList.append( su.directoryURL( FALSE ) );
	    if ( notifyList.find( du.directoryURL( FALSE ) ) == -1 )
		notifyList.append( du.directoryURL( FALSE ) );
	}
	
	// Dont forget to get the next dest. We get the corresponding src
	// in the 'for' clause.
	++itDest;
    }

    // Delete the last name from the URL to get the directory we have to notify.
    // If one copies "file:/home/weis/test.txt" to "file:/tmp/trash/test.txt" we have
    // to notify "file:/home/weis/" and "file:/tmp/trash/" about changes.
    // Usually no directories appear in cmDestURLList and cmSrcURLList, but if one says
    // move( "ftp://www.ftp.org/contrib/", "file:/GreatStuff/" ), we can not change the
    // directory path "/contrib/ in the source into lost of pathes for files like
    // "/contrib/kfm.tgz" and "/contrib/kwm.tgz" and so on. In this case we might get
    // directories in here.
    char *s;
    for (s = cmDestURLList.first(); s != 0L; s = cmDestURLList.next() )
    {
	KURL u( s );
	if ( notifyList.find( u.directoryURL( FALSE ) ) == -1 )
	    notifyList.append( u.directoryURL( FALSE ) );
    }
    for (s = cmSrcURLList.first(); s != 0L; s = cmSrcURLList.next() )
    {
	KURL u( s );		
	if ( notifyList.find( u.directoryURL( FALSE ) ) == -1 )
	    notifyList.append( u.directoryURL( FALSE ) );
    }
    
    cmCount = cmSrcURLList.count();

    // Delete subdirectories first => inverse the order of the list
    mvDelURLList.clear();
    for ( p = tmpDelURLList.last(); p != 0L; p = tmpDelURLList.prev() )
	mvDelURLList.append( p );

    server->getSlave( this );
}

void KIOJob::del( const char *_url )
{
    action = KIOJob::JOB_DELETE;

    tmpDelURLList.clear();
    tmpDelURLList.append( _url );
    
    KURL u( _url );
    if ( KIOServer::isDir( _url ) )
    {
	u.cd( ".." );
	notifyList.append( u.url() );
    }
    else
	notifyList.append( u.directoryURL() );
    
    del();
}

void KIOJob::del( QStrList & _url_list )
{
    action = KIOJob::JOB_DELETE;
    
    tmpDelURLList.copy( _url_list );

    char *s;
    for ( s = tmpDelURLList.first(); s != 0L; s = tmpDelURLList.next() )
    {
	KURL u( s );
	if ( KIOServer::isDir( s ) )
	{
	    u.cd( ".." );
	    notifyList.append( u.url() );
	}
	else
	    notifyList.append( u.directoryURL() );
    }
    
    del();
}

void KIOJob::del()
{    
    // Recursive directory
    char *p;
    QListIterator<char> it( tmpDelURLList );
    for ( ; it.current(); ++it )
    {
	p = it.current();
	
	printf("Looking at '%s'\n",p);
	KURL su( p );

	int i = 1;
	if ( strcmp( su.protocol(), "file" ) == 0 )
	{
	    struct stat buff;
	    stat( su.path(), &buff );
	    if ( S_ISDIR( buff.st_mode ) )
	    {
		DIR *dp;
		struct dirent *ep;
		dp = opendir( su.path() );
		if ( dp == NULL )
		{
		    printf("ERROR: Could not access directory '%s'\n", p );
		    return;
		}
		    
		while ( ep = readdir( dp ) )
		{
		    if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
		    {
			QString s = p;
			s.detach();
			if ( s.length() > 0 && s.data()[ s.length() - 1 ] != '/' )
			    s += "/";
			s += ep->d_name;
			    
			printf("Appending '%s'\n",s.data());
			tmpDelURLList.append( s.data() );
		    }
		}
	    }
	}
    }
    
    mvDelURLList.clear();
    for ( p = tmpDelURLList.last(); p != 0L; p = tmpDelURLList.prev() )
	mvDelURLList.append( p );
    cmCount = mvDelURLList.count();

    server->getSlave( this );
}

void KIOJob::doIt( KIOSlaveIPC * _slave )
{
    slave = _slave;

    connect( slave, SIGNAL( fatalError( int, const char*, int ) ),
	     this, SLOT( fatalError( int, const char*, int ) ) );
    connect( slave, SIGNAL( setPID( int ) ), this, SLOT( start( int ) ) );
    slave->getPID();
}

void KIOJob::msgResult2( QWidget * _win, int _button, const char *_src, const char *_dest )
{
    QString src = _src;
    QString dest = _dest;
    
    delete _win;

    switch( action )
    {
    case KIOJob::JOB_MOVE:
	moveDelMode = FALSE;
	if ( _button == 0 ) // Overwrite
	{
	    overwriteExistingFiles = TRUE;
	    slaveIsReady();
	    overwriteExistingFiles = FALSE;
	}
	else if ( _button == 1 ) // Overwrite All
	{
	    overwriteExistingFiles = TRUE;
	    slaveIsReady();
	}
	else if ( _button == 2 ) // Skip
	{
	    cmSrcURLList.removeRef( cmSrcURLList.first() );
	    cmDestURLList.removeRef( cmDestURLList.first() );
	    slaveIsReady();
	}
	else if ( _button == 3 ) // Rename
	{
	    cmSrcURLList.removeRef( cmSrcURLList.first() );
	    cmDestURLList.removeRef( cmDestURLList.first() );

	    cmSrcURLList.insert( 0, src.data() );
	    cmDestURLList.insert( 0, dest.data() );
	    slaveIsReady();
	}
	else if ( _button == 4 ) // Cancel
	    done();
	break;
    case KIOJob::JOB_COPY:
	if ( _button == 0 ) // Overwrite
	{
	    overwriteExistingFiles = TRUE;
	    cmSrcURLList.insert( 0, src.data() );
	    cmDestURLList.insert( 0, dest.data() );
	    slaveIsReady();
	    overwriteExistingFiles = FALSE;
	}
	else if ( _button == 1 ) // Overwrite All
	{
	    overwriteExistingFiles = TRUE;
	    cmSrcURLList.insert( 0, src.data() );
	    cmDestURLList.insert( 0, dest.data() );
	    slaveIsReady();
	}
	else if ( _button == 2 ) // Skip
	{
	    slaveIsReady();
	}
	else if ( _button == 3 ) // Rename
	{
	    cmSrcURLList.insert( 0, src.data() );
	    cmDestURLList.insert( 0, dest.data() );
	    slaveIsReady();
	}
	else if ( _button == 4 ) // Cancel
	    done();
	break;
    }
}

void KIOJob::msgResult( QWidget * _win, int _button )
{
    delete _win;
    
    switch( action )
    {
    case KIOJob::JOB_COPY:
    case KIOJob::JOB_LIST:
    case KIOJob::JOB_MKDIR:
    case KIOJob::JOB_DELETE:
    case KIOJob::JOB_MOVE:
    case KIOJob::JOB_LINK:
	if ( _button == 1 )
	    slaveIsReady();
	else if ( _button == 2 )
	    done();
	return;
    case KIOJob::JOB_MOUNT:
    case KIOJob::JOB_UNMOUNT:
	done();
    }
}

void KIOJob::fatalError( int _kioerror, const char* _url, int _errno )
{
    kioError = _kioerror;
    
    printf("################################# fatalError called '%s'\n",_url);
    
    KMsgWin *m = 0L;
    KRenameWin *r = 0L;
    
    QString msg;

    switch( action )
    {
    case KIOJob::JOB_COPY:
    case KIOJob::JOB_LIST:
    case KIOJob::JOB_MKDIR:
    case KIOJob::JOB_DELETE:
    case KIOJob::JOB_MOVE:
    case KIOJob::JOB_LINK:
	switch( _kioerror )
	{
	case KIO_ERROR_MalformedURL:
	    msg.sprintf("Malformed URL\n%s",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_CouldNotRead:
	    msg.sprintf("Could not read\n%s\nFile does not exist or access denied",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_CouldNotWrite:
	    msg.sprintf("Could not write\n%s\nPerhaps access denied",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_CouldNotCreateSocket:
	    msg.sprintf("Could not create Socket for\n%s",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_UnknownHost:
	    msg.sprintf("Unknwon host in\n%s",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_CouldNotConnect:
	    msg.sprintf("Could not connect to\n%s",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_NotImplemented:
	    msg.sprintf("The requested action\n'%s'\nis not implemented yet.",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_CouldNotMkdir:
	    msg.sprintf("Could not make directory\n%s",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_CouldNotList:
	    msg.sprintf("Could not list directory contents\n%s",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_CouldNotDelete:
	    msg.sprintf("Could not delete\n%s\nURL does not exist or permission denied",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_CouldNotLogin:
	    msg.sprintf("Could not login for\n%s\nPerhaps wrong password",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_TarError:
	    msg.sprintf("Tar reproted an error\n%s",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_GzipError:
	    msg.sprintf("Gzip reproted an error\n%s",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Continue", "Cancel" );
	    break;
	case KIO_ERROR_FileExists:
	    r = new KRenameWin( 0L, lastSource.data(), lastDest.data() );
	    break;
	}	
	break;
    case KIOJob::JOB_MOUNT:
    case KIOJob::JOB_UNMOUNT:
	switch( _kioerror )
	{
	case KIO_ERROR_CouldNotRead:
	    msg.sprintf("Could not read\n%s\nFile does not exist or access denied",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Close" );
	    break;
	case KIO_ERROR_CouldNotMount:
	    msg.sprintf("Could not mount\nError log:\n\n%s",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Close" );
	    break;
	case KIO_ERROR_CouldNotUnmount:
	    msg.sprintf("Could not unmount\nError log:\n\n%s",_url);
	    m = new KMsgWin( 0L, "Error", msg.data(), KMsgWin::EXCLAMATION, "Close" );
	    break;
	}
	break;
    }
    
    if ( m != 0L )
    {
	connect( m, SIGNAL( result( QWidget*, int ) ), this, SLOT( msgResult( QWidget*, int ) ) );
	m->show();
    }
    if ( r != 0L )
    {
	printf("++++++++++++++++++++++++++++++ Connect ++++++++++++++++++++++++++\n");
	connect( r, SIGNAL( result( QWidget*, int, const char*, const char* ) ),
		 this, SLOT( msgResult2( QWidget*, int, const char*, const char* ) ) );
	r->show();
    }
}

void KIOJob::start( int _pid )
{
    slave->pid = _pid;

    if ( bDisplay )
    {
	switch( action )
	{
	case KIOJob::JOB_COPY:
	case KIOJob::JOB_MOVE:
	    {
		dlg = new QDialog( 0L );
		dlg->resize( 300, 180 );
		progress = new KProgress( 0, 100, 0, KProgress::Horizontal, dlg );
		progress->setGeometry( 10, 100, 280, 20 );
		QPushButton *pb = new QPushButton( "Cancel", dlg );
		pb->setGeometry( 110, 140, 80, 30 );
		connect( pb, SIGNAL( clicked() ), this, SLOT( cancel() ) );
		line1 = new QLabel( dlg );
		line1->setGeometry( 10, 10, 280, 20 );
		line2 = new QLabel( dlg );
		line2->setGeometry( 10, 30, 280, 20 );
		line3 = new QLabel( dlg );
		line3->setGeometry( 10, 50, 280, 20 );
		dlg->show();
	    }
	    break;
	case KIOJob::JOB_DELETE:
	    {
		bool showDlg = TRUE;
		if ( mvDelURLList.count() == 1 )
		    if ( strncmp( mvDelURLList.first(), "file", 4 ) == 0 )
			showDlg = FALSE;
		if ( showDlg )
		{
		    dlg = new QDialog( 0L );
		    dlg->resize( 300, 180 );
		    progress = new KProgress( 0, 100, 0, KProgress::Horizontal, dlg );
		    progress->setGeometry( 10, 100, 280, 20 );
		    QPushButton *pb = new QPushButton( "Cancel", dlg );
		    pb->setGeometry( 110, 140, 80, 30 );
		    connect( pb, SIGNAL( clicked() ), this, SLOT( cancel() ) );
		    line1 = new QLabel( dlg );
		    line1->setGeometry( 10, 10, 280, 20 );
		    line2 = new QLabel( dlg );
		    line2->setGeometry( 10, 30, 280, 20 );
		    dlg->show();
		}
	    }
	    break;
	case KIOJob::JOB_MOUNT:
	case KIOJob::JOB_UNMOUNT:
	    {
		dlg = new QDialog( 0L );
		dlg->resize( 300, 100 );
		QPushButton *pb = new QPushButton( "Cancel", dlg );
		pb->setGeometry( 110, 60, 80, 30 );
		connect( pb, SIGNAL( clicked() ), this, SLOT( cancel() ) );
		line1 = new QLabel( dlg );
		line1->setGeometry( 10, 10, 280, 20 );
		line1->show();
		dlg->show();
	    }
	    break;
	case KIOJob::JOB_LIST:
	    {
		KURL u( lstURL.data() );
		if ( strcmp( u.protocol(), "ftp" ) == 0 )
		{
		    dlg = new QDialog( 0L );
		    dlg->resize( 300, 100 );
		    QPushButton *pb = new QPushButton( "Cancel", dlg );
		    pb->setGeometry( 110, 60, 80, 30 );
		    connect( pb, SIGNAL( clicked() ), this, SLOT( cancel() ) );
		    line1 = new QLabel( dlg );
		    line1->setGeometry( 10, 10, 280, 20 );
		    dlg->show();
		}
		else
		    dlg = 0L;
	    }
	    break;
	case KIOJob::JOB_MKDIR:
	    {
		KURL u( mkdirURL.data() );
		if ( strcmp( u.protocol(), "ftp" ) == 0 )
		{
		    dlg = new QDialog( 0L );
		    dlg->resize( 300, 100 );
		    QPushButton *pb = new QPushButton( "Cancel", dlg );
		    pb->setGeometry( 110, 60, 80, 30 );
		    connect( pb, SIGNAL( clicked() ), this, SLOT( cancel() ) );
		    line1 = new QLabel( "Making directory", dlg );
		    line1->setGeometry( 10, 10, 200, 20 );
		    line2 = new QLabel( mkdirURL.data() );
		    line2->setGeometry( 10, 10, 200, 20 );
		    dlg->show();
		}
		else
		    dlg = 0L;
	    }
	    break;	
	}
    }
    
    connect( slave, SIGNAL( done() ), this, SLOT( slaveIsReady() ) );

    started = FALSE;
    cleanedUp = FALSE;
    
    slaveIsReady();
}

void KIOJob::slaveIsReady()
{
    printf("SlaveIsReady\n");

    if ( cleanedUp )
    {
	if ( dlg != 0L )
	    delete dlg;
	dlg = 0L;
	done();
	return;
    }
    
    switch( action )
    {
    case KIOJob::JOB_COPY:
	{
	    if ( !started )
		connect( slave, SIGNAL( progress( int ) ), this, SLOT( slaveProgress( int ) ) );
	    
	    if ( cmSrcURLList.count() == 0 )
	    {
		slave->cleanUp();
		cleanedUp = TRUE;
		return;
	    }

	    if ( dlg )
	    {
		char buffer[ 1024 ];
		sprintf( buffer, "File %i/%i", cmCount - cmSrcURLList.count() + 1, cmCount );	    
		line1->setText( buffer );
		line2->setText( cmSrcURLList.first() );
		sprintf( buffer, "to %s", cmDestURLList.first() );
		line3->setText( buffer );
	    }

	    lastSource = cmSrcURLList.first();
	    lastDest = cmDestURLList.first();
	    
	    slave->copy( cmSrcURLList.first(), cmDestURLList.first(), overwriteExistingFiles );
	    cmSrcURLList.removeRef( cmSrcURLList.first() );
	    cmDestURLList.removeRef( cmDestURLList.first() );
	}
	break;

    case KIOJob::JOB_MOVE:
	{
	    if ( !started )
	    {
		connect( slave, SIGNAL( progress( int ) ), this, SLOT( slaveProgress( int ) ) );
		moveDelMode = FALSE;
	    }
	    
	    if ( cmSrcURLList.count() == 0 )
	    {
		char *p;
		for( p = mvDelURLList.first(); p != 0L; p = mvDelURLList.next() )
		{
		    if ( rmdir( p ) == -1 )
		    {
			printf("ERROR: Could not delete dir '%s'\n",p );
			slave->cleanUp();
			cleanedUp = TRUE;
			delete dlg;
			dlg = 0L;
			done();
			return;
		    }
		}
		mvDelURLList.clear();
		
		printf("Removing dialog\n");

		slave->cleanUp();
		cleanedUp = TRUE;
		return;
	    }
	 
	    if ( dlg )
	    {
		char buffer[ 1024 ];
		sprintf( buffer, "File %i/%i", cmCount - cmSrcURLList.count() + 1, cmCount );	    
		line1->setText( buffer );
		line2->setText( cmSrcURLList.first() );
		sprintf( buffer, "to %s", cmDestURLList.first() );
		line3->setText( buffer );
	    }
	    
	    // In this turn delete the file we copied last turn
	    if ( moveDelMode )
	    {
		slave->del( cmSrcURLList.first() );
		cmSrcURLList.removeRef( cmSrcURLList.first() );
		cmDestURLList.removeRef( cmDestURLList.first() );
	    }
	    else
	    {
		lastSource = cmSrcURLList.first();
		lastDest = cmDestURLList.first();

		slave->copy( cmSrcURLList.first(), cmDestURLList.first(), overwriteExistingFiles );
	    }
	    
	    moveDelMode = !moveDelMode;
	}
	break;

    case KIOJob::JOB_DELETE:
	{
	    if ( mvDelURLList.count() == 0 )
	    {
		printf("Removing dialog\n");
		slave->cleanUp();
		cleanedUp = TRUE;
		return;
	    }

	    if ( dlg != 0L )
	    {
		char buffer[ 1024 ];
		sprintf( buffer, "File %i/%i", cmCount - mvDelURLList.count() + 1, cmCount );	    
		line1->setText( buffer );
		line2->setText( mvDelURLList.first() );
		if ( cmCount != mvDelURLList.count() )
		     progress->setValue( cmCount * 100 / ( cmCount - mvDelURLList.count() ) );
	    }
	    
	    slave->del( mvDelURLList.first() );
	    mvDelURLList.removeRef( mvDelURLList.first() );
	}
	break;
    case KIOJob::JOB_MOUNT:
    case KIOJob::JOB_UNMOUNT:
	{
	    // mount already called ?
	    if ( started )
	    {
		printf("Removing dialog\n");
		slave->cleanUp();
		cleanedUp = TRUE;
		return;
	    }

	    if ( dlg )
	    {
		char buffer[ 1024 ];
		if ( action == KIOJob::JOB_MOUNT )
		    sprintf( buffer, "Mounting %s ...", mntDev.data() );
		else
		    sprintf( buffer, "Unmounting %s ...", mntPoint.data() );
		line1->setText( buffer );
	    }
	    
	    if ( action == KIOJob::JOB_MOUNT )
		slave->mount( mntReadOnly, mntFSType.data(), mntDev.data(), mntPoint.data() );
	    else
		slave->unmount( mntPoint.data() );
	}
	break;
    case KIOJob::JOB_LIST:
	{
	    // Already told the slave what to do ?
	    if ( started )
	    {
		slave->cleanUp();
		cleanedUp = TRUE;
		return;
	    }

	    connect( slave, SIGNAL( dirEntry( const char *, const char *, bool, int, const char *,
					      const char *, const char *, const char * ) ), 
		     this, SLOT( slotDirEntry( const char *, const char *, bool, int, const char *,
				       const char *, const char *, const char * ) ) );
	    
	    if ( dlg != 0L )
	    {
		char buffer[ 1024 ];
		sprintf( buffer, "Retrieving listing of %s ...", lstURL.data() );
		line1->setText( buffer );
	    }
	    
	    slave->list( lstURL.data() );
	}
	break;
    case KIOJob::JOB_MKDIR:
	{
	    // Already told the slave what to do ?
	    if ( started )
	    {
		slave->cleanUp();
		cleanedUp = TRUE;
		return;
	    }
	    
	    slave->mkdir( mkdirURL.data() );
	}
	break;
    }

    started = TRUE;
}

void KIOJob::slotDirEntry( const char *_url, const char *_name, bool _isDir, int _size,
			  const char * _creationDate, const char * _access,
			  const char * _owner, const char *_group )
{
    // Perhaps we get more than the client requested ( for example when looking in tar files )
    
    if ( strcmp( _url, lstURL.data() ) == 0 )
    {
	KIODirectoryEntry e( _name, _isDir, _size, _creationDate, _access, _owner, _group );

	emit newDirEntry( id, &e );
    }
}


void KIOJob::slaveProgress( int _percent )
{
    if ( dlg == 0L )
	return;
    
    if ( progress != 0L )
	progress->setValue( _percent );
}

void KIOJob::cancel()
{
    pid_t p = (pid_t)slave->pid;
    
    printf("Cancel\n");
    delete slave;
    delete dlg;
    dlg = 0L;
    slave = 0L;
    kill( p, SIGTERM );
    
    done();
}

void KIOJob::done()
{
    printf("Done\n");
    
    if ( slave != 0L )
	server->freeSlave( slave );
    if ( dlg != 0L )
    {
	if ( line1 != 0L )
	    delete line1;
	if ( line2 != 0L )
	    delete line2;
	if ( line3 != 0L )
	    delete line3;
	
	delete dlg;
    }

    char *s;
    for ( s = notifyList.first(); s != 0L; s = notifyList.next() )
    {
	printf("NOTIFY '%s'\n",s);
	if ( globalNotify )
	    KIOServer::sendNotify( s );
	emit notify( id, s );
    }

    if ( action == KIOJob::JOB_MOUNT || action == KIOJob::JOB_UNMOUNT )
	KIOServer::sendMountNotify();
    
    emit finished( id );
    
    dlg = 0L;
    slave = 0L;
    delete this;
}

void KIOJob::deleteAllJobs()
{
    KIOJob *j;
    for ( j = jobList.first(); j != 0L; j = jobList.next() )
	j->cancel();
}

#include "kiojob.moc"
