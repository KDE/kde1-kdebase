#include <qstrlist.h>
#include <qpushbt.h>
#include <qpainter.h>
#include <qdict.h>
#include <kurl.h>
#include <qmsgbox.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "kioserver_ipc.h"
#include "kiojob.h"
#include "kbind.h"
#include "kioslave/kio_errors.h"
#include "kmsgwin.h"
#include "krenamewin.h"
#include "passworddialog.h"
#include <config-kfm.h>

#include <klocale.h>

QList<KIOJob> KIOJob::jobList;
QDict<QString> KIOJob::passwordDict;

KIOJob::KIOJob( int _id )
{
    bAutoDelete = TRUE;
    server = KIOServer::getKIOServer();
    slave = 0L;
    dlg = 0L;
    bDisplay = true;
    progressBar = 0L;
    line1 = 0L;
    line2 = 0L;
    line3 = 0L;
    globalNotify = true;
    id = _id;
    overwriteExistingFiles = false;
    
    jobList.setAutoDelete( false );
    jobList.append( this );
}

KIOJob::~KIOJob()
{
    jobList.removeRef( this );
}

void KIOJob::setOverWriteExistingFiles( bool _o )
{
    overwriteExistingFiles = _o;
}

void KIOJob::mkdir( const char *_url )
{
    // debugT("Making '%s'\n",_url);
    
    action = KIOJob::JOB_MKDIR;
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
        warning(klocale->translate("ERROR: Malformed URL"));
	return;
    }
    
    action = KIOJob::JOB_MKDIR;
    
    mkdirURL = _url;
    mkdirURL.detach();
    
    notifyList.append( u.directoryURL() );

    server->getSlave( this );
}

void KIOJob::list( const char *_url, bool _reload, bool _bHTML )
{
    action = KIOJob::JOB_LIST;
    
    bHTML = _bHTML;
   
    lstURL = _url;
    lstURL.detach();
    // debugT("LIST: Got '%s'\n",lstURL.data());
 
    KIODirectory *dir = 0L;
    if ( !_reload )
	dir = server->getDirectory( _url );
    
    if ( dir != 0L )
    {
	// debugT("GOT Cached information\n");
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
	warning(klocale->translate("INTERNAL ERROR: You must at least specify the device for mounting") );
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
    skipURLList.clear();
    
    char *p;
    char *p2 = cmDestURLList.first();
    // Loop over all filess
    for ( p = cmSrcURLList.first(); p != 0L; p = cmSrcURLList.next() )
    {
	KURL su( p );
	KURL du( p2 );
	
	// Which directories do we have to notify ?
	if ( notifyList.find( du.directoryURL( false ) ) == -1 )
	    notifyList.append( du.directoryURL( false ) );
	
	if ( su.isMalformed() )
	{
	    QString tmp;
	    tmp.sprintf( "%s\n%s", klocale->translate( "Malformed URL" ), p );
	    QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp );
	    done();
	    return;
	}
	else if ( du.isMalformed() )
	{
	    QString tmp;
	    tmp.sprintf( "%s\n%s", klocale->translate( "Malformed URL" ), p );
	    QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp );
	    done();
	    return;
	}	
	// I can only make links on the local file system.
	else if ( strcmp( du.protocol(), "file" ) != 0L )
	{
	    QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
				  klocale->translate("Can only make links on local file system") );
	    done();
	    return;
	}
	// No error yet ...
	else
	{
	    // Do we link a file on the local disk?
	    if ( strcmp( su.protocol(), "file" ) == 0 && !su.hasSubProtocol() )
	    {
		// Make a symlink
		if ( symlink( su.path(), du.path() ) == -1 )
		{
		    // Does the destination already exist ?
		    if ( errno == EEXIST )
		    {
			// Are we allowed to overwrite the files ?
			if ( overwriteExistingFiles )
			{
			    // Try to delete the destination
			    if ( unlink( du.path() ) != 0 )
			    {
				QString tmp;
				tmp.sprintf( "%s\n%s", klocale->translate( "Could not overwrite" ), du.path() );
				QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp );
				done();
				return;
			    }
			}
			else
			{
			    // Ask the user what to do
			    KRenameWin *r = new KRenameWin( 0L, su.path(), du.path(), true );
			    int button = r->exec();
			    if ( button == 0 ) // Overwrite 
			    {
				// Try to delete the destination
				if ( unlink( du.path() ) != 0 )
				{
				    delete r;
				    QString tmp;
				    tmp.sprintf( "%s\n%s", klocale->translate( "Could not overwrite" ), du.path() );
				    QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp );
				    done();
				    return;
				}
				// Try again
				if ( symlink( su.path(), du.path() ) == -1 )
				{
				    QString tmp;
				    tmp.sprintf( "%s\n%s", klocale->translate( "Could not make symlink to" ), du.path() );
				    QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp );
				    done();
				    return;
				}
			    }
			    else if ( button == 1 ) // Overwrite All
				overwriteExistingFiles = true;
			    else if ( button == 2 ) // Skip
			    {
				skipURLList.append( p );
				skipURLList.append( p2 );
				continue;
			    }
			    else if ( button == 3 ) // Rename
			    {
				// Get the new destinations name
				du = r->getNewName();
				// Try again
				if ( symlink( su.path(), du.path() ) == -1 )
				{
				    QString tmp;
				    tmp.sprintf( "%s\n%s", klocale->translate( "Could not make symlink to" ), du.path() );
				    QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp );
				    done();
				    return;
				}
			    }
			    else if ( button == 4 ) // Cancel
			    {
				delete r;
				done();
				return;
			    }
			    delete r;
			}
		    }
		    else
		    {
			// Some error occured while we tried to symlink
			QString tmp;
			tmp.sprintf( "%s\n%s", klocale->translate( "Could not make symlink to" ), du.path() );
			QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp );
			done();
			return;
		    }
		    
		}
	    }
	    // Make a link from a file in a tar archive, ftp, http or what ever
	    else
	    {
		QFile f( du.path() );
		if ( f.open( IO_ReadWrite ) )
		{
		  f.close(); // kalle
		  // kalle		    QTextStream pstream( &f );
		  KConfig config( du.path() ); // kalle
		    config.setGroup( "KDE Desktop Entry" );
		    config.writeEntry( "URL", p );
		    config.writeEntry( "Type", "Link" );
		    if ( strcmp( su.protocol(), "ftp" ) == 0 )
			config.writeEntry( "Icon", "ftp.xpm" );
		    else if ( strcmp( su.protocol(), "http" ) == 0 )
			config.writeEntry( "Icon", "www.xpm" );
		    else if ( strcmp( su.protocol(), "info" ) == 0 )
			config.writeEntry( "Icon", "info.xpm" );
		    else
			config.writeEntry( "Icon", KMimeType::getDefaultPixmap() );
		    config.sync();
		    if ( globalNotify )
			KIOServer::sendNotify( p2 );
		    emit notify( id, p2 );
		}
		else
		    warning(klocale->translate(" ERROR: Could not write to %s"),p);
	    }
	}
	p2 = cmDestURLList.next();
    }

    // We dont need a slave, so quit
    done();
}

void KIOJob::get( const char *_url )
{
    action = KIOJob::JOB_GET;

    cmSrcURLList.clear();
    cmSrcURLList.append( _url );

    server->getSlave( this );
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
	QString tmp = p;
	// Is there a trailing '/' ? Delete is, so that KURL::filename works
	if ( tmp.right(1) == "/" && tmp.right(2) != ":/" )
	    tmp.truncate( tmp.length() - 1 );
	KURL su( tmp.data() );
	
	QString d = _dest_dir_url;
	d.detach();
	if ( d.right(1) != "/" )
	    d += "/";
	d += su.filename();
	
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
    QStrList tmpList1;
    QStrList tmpList2;
    
    // Recursive directory
    char *p;
    char *p2;

    QListIterator<char> it( cmSrcURLList );
    QListIterator<char> it2( cmDestURLList );
    for ( ; it.current(); ++it )
    {
	p = it.current();
	p2 = it2.current();
	KURL su( p );
	KURL du( p2 );

	// debugT("Have: '%s' '%s'\n",p,p2);
	
	if ( strcmp( su.protocol(), "file" ) == 0 && strcmp( du.protocol(), "file" ) == 0 )
	{
	    struct stat buff;
	    stat( su.path(), &buff );
	    if ( S_ISDIR( buff.st_mode ) )
	    {
		// debugT("Making diretory '%s'\n",du.path());
		
		if ( ::mkdir( du.path(), buff.st_mode ) == -1 )
		    if ( errno != EEXIST )
		    {
			QString tmp;
			tmp.sprintf( klocale->translate( "Could not make directory\n%s" ), du.path() );
			QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp.data() );
			return;
		    }
		
		DIR *dp;
		struct dirent *ep;
		dp = opendir( su.path() );
		if ( dp == NULL )
		{
		    warning(klocale->translate("ERROR: Could not access directory '%s'"), p );
		    return;
		}
		
		while ( (ep = readdir( dp ) ) != 0L )
		{
		    if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
		    {
			// debugT("Adding '%s'\n",ep->d_name);
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
		
		closedir( dp );
		
		// Remove directories from the copy list
		tmpList1.append( p );
		tmpList2.append( p2 );
	    }
	}

	++it2;
    }

    char *s;
    for ( s = cmDestURLList.first(); s != 0L; s = cmDestURLList.next() )
    {
	KURL u( s );
	if ( notifyList.find( u.directoryURL( false ) ) == -1 )
	    notifyList.append( u.directoryURL( false ) );
    }
    
    for ( s = tmpList1.first(); s != 0L; s = tmpList1.next() )
	cmSrcURLList.remove( s );
    for ( s = tmpList2.first(); s != 0L; s = tmpList2.next() )
	cmDestURLList.remove( s );
    
    cmCount = cmSrcURLList.count();

    server->getSlave( this );
}

void KIOJob::move( const char *_src_url, const char *_dest_url )
{
    action = KIOJob::JOB_MOVE;

    tmpSrcURLList.clear();
    tmpDestURLList.clear();    
    tmpSrcURLList.append( _src_url );
    tmpDestURLList.append( _dest_url );

    move();
}

void KIOJob::move( QStrList & _src_url_list, const char *_dest_dir_url )
{
    action = KIOJob::JOB_MOVE;
    
    tmpSrcURLList.copy( _src_url_list );
    tmpDestURLList.clear();

    char *p;
    for ( p = tmpSrcURLList.first(); p != 0L; p = tmpSrcURLList.next() )
    {
	QString tmp = p;
	// Is there a trailing '/' ? Delete is, so that KURL::filename works
	if ( tmp.right(1) == "/" && tmp.right(2) != ":/" )
	    tmp.truncate( tmp.length() - 1 );
	KURL su( tmp.data() );
	
	QString d = _dest_dir_url;
	d.detach();
	if ( d.right(1) != "/" )
	    d += "/";
	d += su.filename();
	
	tmpDestURLList.append( d.data() );
    }

    move();
}

void KIOJob::move()
{
    tmpDelURLList.clear();
    skipURLList.clear();
    
    // Recursive directory
    QListIterator<char> itSrc( tmpSrcURLList );
    QListIterator<char> itDest( tmpDestURLList );    
    char *p;
    char *p2 = tmpDestURLList.first();
    for ( ; itSrc.current(); ++itSrc )
    {
	p = itSrc.current();
	p2 = itDest.current();
	
	KURL su( p );
	KURL du( p2 );

	QString supath( su.path() );
	QString dupath( du.path() );
	KURL::decodeURL( supath );
	KURL::decodeURL( dupath );
	
	int i = 1;
	// Moving on the local hard disk ?
	if ( strcmp( su.protocol(), "file" ) == 0 && !su.hasSubProtocol() &&
	     strcmp( du.protocol(), "file" ) == 0 && !du.hasSubProtocol() )
	{
	    // Does the file already exist ?
	    // If yes, care about renaming.
	    struct stat buff;
	    if ( stat( dupath, &buff ) == 0 && !overwriteExistingFiles )
	    {
		KRenameWin *r = new KRenameWin( 0L, p, p2, true );
		int button = r->exec();
		if ( button == 0 ) // Overwrite 
		{
		}
		else if ( button == 1 ) // Overwrite All
		    overwriteExistingFiles = true;
		else if ( button == 2 ) // Skip
		{
		    skipURLList.append( p );
		    skipURLList.append( p2 );
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
	    
	    // Try to move the files with the help of UNIX.
	    // 'i' will indicate wether we succeeded.
	    // Now i is either 0 or -1.
	    i = rename( supath, dupath );
	}
	
	// Did we succeed with our call to 'rename'
	if ( i == -1 )
	{
	    // We tried to move across devices ?
	    // Ok, that is not a real error.
	    if ( errno == EXDEV )
	    {
		// debugT("Testing for dir '%s'\n",su.path());
		struct stat buff;
		stat( supath, &buff );
		// We want to move a directory ?
		// Then we must know each file in the tree ....
		if ( S_ISDIR( buff.st_mode ) )
		{
		    // debugT("Making diretory '%s'\n",du.path());
	    
		    if ( ::mkdir( dupath, S_IRWXU ) == -1 )
			if ( errno != EEXIST )
			{
			    QString tmp;
			    tmp.sprintf( klocale->translate( "Could not make directory\n%s" ), dupath.data() );
			    QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp.data() );
			    return;
			}

		    // Dont forget to delete this directory at the end
		    tmpDelURLList.append( supath );
		    // ... and dont try to copy it
		    skipURLList.append( p );
		    
		    DIR *dp;
		    struct dirent *ep;
		    dp = opendir( supath );
		    if ( dp == NULL )
		    {
			QString tmp;
			tmp.sprintf( klocale->translate( "Could not access directory\n%s" ), dupath.data() );
			QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp.data() );
			return;
		    }
		    
		    while ( ( ep = readdir( dp ) ) != 0L )
		    {
			if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
			{
			    QString s = p;
			    s.detach();
			    if ( s.right(1) != "/" )
				s += "/";
			    s += ep->d_name;
			    
			    QString d = du.url();
			    d.detach();
			    if ( d.right(1) != "/" )
				d += "/";
			    d += ep->d_name;
			    
			    tmpSrcURLList.append( s.data() );
			    tmpDestURLList.append( d.data() );
			}
		    }
		}
	    }
	    else
	    {
		QString tmp;
		tmp.sprintf( klocale->translate( "Could not move directory\n%s\nto '%s'\nPerhaps access denied" ), supath.data(), dupath.data() );
		QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp.data() );
		return;
	    }
	}
	// We moved the files already, so take them from the list
	else if ( i == 0 )
	{
	    skipURLList.append( p );
	    skipURLList.append( p2 );
	}
	
	// Dont forget to get the next dest. We get the corresponding src
	// in the 'for' clause.
	++itDest;
    }

    cmDestURLList.clear();
    cmSrcURLList.clear();
    
    // Delete the last name from the URL to get the directory we have to notify.
    // If one copies "file:/home/weis/test.txt" to "file:/tmp/trash/test.txt" we have
    // to notify "file:/home/weis/" and "file:/tmp/trash/" about changes.
    // Usually no directories appear in cmDestURLList and cmSrcURLList, but if one says
    // move( "ftp://www.ftp.org/contrib/", "file:/GreatStuff/" ), we can not change the
    // directory path "/contrib/ in the source into lots of pathes for files like
    // "/contrib/kfm.tgz" and "/contrib/kwm.tgz" and so on. In this case we might get
    // directories in here.
    char *s;
    for ( s = tmpDestURLList.first(); s != 0L; s = tmpDestURLList.next() )
    {
	QString tmp = s;
	if ( tmp.right(1) == "/" )
	    tmp.truncate( tmp.length() - 1 );
	KURL u( tmp );
	if ( notifyList.find( u.directoryURL( false ) ) == -1 )
	    notifyList.append( u.directoryURL( false ) );
	if ( skipURLList.find( s ) == -1 )
	    cmDestURLList.append( s );
    }
    for ( s = tmpSrcURLList.first(); s != 0L; s = tmpSrcURLList.next() )
    {
	QString tmp = s;
	if ( tmp.right(1) == "/" )
	    tmp.truncate( tmp.length() - 1 );
	KURL u( tmp );
	if ( notifyList.find( u.directoryURL( false ) ) == -1 )
	    notifyList.append( u.directoryURL( false ) );
	if ( skipURLList.find( s ) == -1 )
	    cmSrcURLList.append( s );
    }
    
    cmCount = cmSrcURLList.count();
    
    // Delete subdirectories first => inverse the order of the list
    mvDelURLList.clear();
    for ( p = tmpDelURLList.last(); p != 0L; p = tmpDelURLList.prev() )
    {
	// debugT(">> REMOVING '%s'\n",p);
	mvDelURLList.append( p );
    }
    
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
    mvDelURLList.clear();
 
    // Recursive directory
    char *p;
    QListIterator<char> it( tmpDelURLList );
    for ( ; it.current(); ++it )
    {
	p = it.current();
	
	// debugT("Looking at '%s'\n",p);
	KURL su( p );

	// int i = 1;
	if ( strcmp( su.protocol(), "file" ) == 0 )
	{
	    struct stat buff;
	    stat( su.path(), &buff );
	    struct stat lbuff;
	    lstat( su.path(), &buff );

	    if ( S_ISLNK( lbuff.st_mode ) )
	    {
		// No recursion here!
	    }
	    else if ( S_ISDIR( buff.st_mode ) )
	    {
		DIR *dp;
		struct dirent *ep;
		dp = opendir( su.path() );
		if ( dp == NULL )
		{
		    warning(klocale->translate("ERROR: Could not access directory '%s'"), p );
		    return;
		}
		    
		while ( ( ep = readdir( dp ) ) != 0L )
		{
		    if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
		    {
			QString s = p;
			s.detach();
			if ( s.length() > 0 && s.data()[ s.length() - 1 ] != '/' )
			    s += "/";
			s += ep->d_name;
			    
			// debugT("Appending '%s'\n",s.data());
			tmpDelURLList.append( s.data() );
		    }
		}
	    }
	}
    }
    
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
    connect( slave, SIGNAL( closed() ), this, SLOT( slotSlaveClosed() ) );

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
	moveDelMode = false;
	if ( _button == 0 ) // Overwrite
	{
	    overwriteExistingFiles = true;
	    slaveIsReady();
	    overwriteExistingFiles = false;
	}
	else if ( _button == 1 ) // Overwrite All
	{
	    overwriteExistingFiles = true;
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
	    overwriteExistingFiles = true;
	    cmSrcURLList.insert( 0, src.data() );
	    cmDestURLList.insert( 0, dest.data() );
	    slaveIsReady();
	    overwriteExistingFiles = false;
	}
	else if ( _button == 1 ) // Overwrite All
	{
	    overwriteExistingFiles = true;
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
	default: // Stephan: added default handler
	    warning("Case not handled here");
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
    case KIOJob::JOB_GET:
	done();
	break;
    }
}

void KIOJob::fatalError( int _kioerror, const char* _url, int )
{
    kioError = _kioerror;
    
    // debugT("################################# fatalError called '%s'\n",_url);
    
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
	    msg.sprintf(klocale->translate("Malformed URL\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotRead:
	    msg.sprintf(klocale->translate("Could not read\n%s\nFile does not exist or access denied"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotWrite:
	    msg.sprintf(klocale->translate("Could not write\n%s\nPerhaps access denied"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), msg.data(), KMsgWin::EXCLAMATION, klocale->translate("Continue"), klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotCreateSocket:
	    msg.sprintf(klocale->translate("Could not create Socket for\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_UnknownHost:
	    msg.sprintf(klocale->translate("Unknwon host in\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotConnect:
	    msg.sprintf(klocale->translate("Could not connect to\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_NotImplemented:
	    msg.sprintf(klocale->translate("The requested action\n'%s'\nis not implemented yet."),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotMkdir:
	    msg.sprintf(klocale->translate("Could not make directory\n%s"),
			_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION,  
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotList:
	    msg.sprintf(klocale->translate("Could not list directory contents\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotDelete:
	    msg.sprintf(klocale->translate("Could not delete\n%s\nURL does not exist or permission denied"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"),
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotLogin:
	    {
		KURL u( _url );
		msg.sprintf(klocale->translate("Could not login for\n%s\nPerhaps wrong password"),u.host());
		// Remove the password from the dict, since it seems to be wrong.
		if ( u.user() != 0L && u.user()[0] != 0 && u.passwd() != 0L && u.passwd()[0] != 0 )
		{
		    QString tmp;
		    tmp.sprintf( "%s@%s", u.user(), u.host() );
		    // debugT("Removing '%s' from dict !!!!!!!!!!!!!!!! ########### !!!!!!\n",tmp.data() );
		    passwordDict.remove( tmp.data() );
		}
	    }
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"),
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_TarError:
	    msg.sprintf(klocale->translate("Tar reproted an error\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_GzipError:
	    msg.sprintf(klocale->translate("Gzip reproted an error\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), msg.data(), KMsgWin::EXCLAMATION, klocale->translate("Continue"), klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_FileExists:
	    r = new KRenameWin( 0L, lastSource.data(), lastDest.data() );
	    break;
	}	
	break;
    case KIOJob::JOB_MOUNT:
    case KIOJob::JOB_UNMOUNT:
    case KIOJob::JOB_GET:
	switch( _kioerror )
	{
	case KIO_ERROR_MalformedURL:
	    msg.sprintf(klocale->translate("Malformed URL\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotRead:
	    msg.sprintf(klocale->translate("Could not read\n%s\nFile does not exist or access denied"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotCreateSocket:
	    msg.sprintf(klocale->translate("Could not create Socket for\n%s"),
			_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_UnknownHost:
	    msg.sprintf(klocale->translate("Unknwon host in\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"),
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotConnect:
	    msg.sprintf(klocale->translate("Could not connect to\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"),
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_NotImplemented:
	    msg.sprintf(klocale->translate("The requested action\n'%s'\nis not implemented yet."),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel" ));
	    break;
	case KIO_ERROR_CouldNotLogin:
	    {
		KURL u( _url );
		msg.sprintf(klocale->translate("Could not login for\n%s\nPerhaps wrong password"),u.host());
		// Remove the password from the dict, since it seems to be wrong.
		if ( u.user() != 0L && u.user()[0] != 0 && u.passwd() != 0L && u.passwd()[0] != 0 )
		{
		    QString tmp;
		    tmp.sprintf( "%s@%s", u.user(), u.host() );
		    // debugT("Removing '%s' from dict !!!!!!!!!!!!!!!! ########### !!!!!!\n",tmp.data() );
		    passwordDict.remove( tmp.data() );
		}
	    }
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_TarError:
	    msg.sprintf(klocale->translate("Tar reproted an error\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_GzipError:
	    msg.sprintf(klocale->translate("Gzip reproted an error\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), 
				 msg.data(), KMsgWin::EXCLAMATION, 
				 klocale->translate("Continue"), 
				 klocale->translate("Cancel") );
	    break;
	case KIO_ERROR_CouldNotMount:
	    msg.sprintf(klocale->translate("Could not mount\nError log:\n\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), msg.data(), KMsgWin::EXCLAMATION, klocale->translate( "Close") );
	    break;
	case KIO_ERROR_CouldNotUnmount:
	    msg.sprintf(klocale->translate("Could not unmount\nError log:\n\n%s"),_url);
	    if ( bDisplay )
		m = new KMsgWin( 0L, klocale->translate("Error"), msg.data(), KMsgWin::EXCLAMATION, klocale->translate("Close") );
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
	// debugT("++++++++++++++++++++++++++++++ Connect ++++++++++++++++++++++++++\n");
	connect( r, SIGNAL( result( QWidget*, int, const char*, const char* ) ),
		 this, SLOT( msgResult2( QWidget*, int, const char*, const char* ) ) );
	r->show();
    }
    if ( !msg.isEmpty() && r == 0L )
	emit error( _kioerror, msg );
    else if ( r == 0L )
	emit error( _kioerror, "" );
}

void KIOJob::start( int _pid )
{
    slave->pid = _pid;

    if ( bDisplay )
    {
	switch( action )
	{
	case KIOJob::JOB_GET:
	    {
		dlg = new QDialog( 0L );
		dlg->resize( 300, 180 );
		progressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, dlg );
		progressBar->setGeometry( 10, 100, 280, 20 );
		QPushButton *pb = new QPushButton( klocale->translate("Cancel"), dlg );
		pb->setGeometry( 110, 140, 80, 30 );
		connect( pb, SIGNAL( clicked() ), this, SLOT( cancel() ) );
		line1 = new QLabel( dlg );
		line1->setGeometry( 10, 10, 280, 20 );
		dlg->show();
	    }
	    break;
	case KIOJob::JOB_COPY:
	case KIOJob::JOB_MOVE:
	    {
		dlg = new QDialog( 0L );
		dlg->resize( 300, 180 );
		progressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, dlg );
		progressBar->setGeometry( 10, 100, 280, 20 );
		QPushButton *pb = 
		  new QPushButton( klocale->translate("Cancel"), dlg );
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
		bool showDlg = true;
		if ( mvDelURLList.count() == 1 )
		    if ( strncmp( mvDelURLList.first(), "file", 4 ) == 0 )
			showDlg = false;
		if ( showDlg )
		{
		    dlg = new QDialog( 0L );
		    dlg->resize( 300, 180 );
		    progressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, dlg );
		    progressBar->setGeometry( 10, 100, 280, 20 );
		    QPushButton *pb =
		      new QPushButton( klocale->translate("Cancel"), dlg );
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
		QPushButton *pb = 
		  new QPushButton( klocale->translate("Cancel"), dlg );
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
		    QPushButton *pb = 
		      new QPushButton( klocale->translate("Cancel"), dlg );
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
		    QPushButton *pb = 
		      new QPushButton( klocale->translate("Cancel"), dlg );
		    pb->setGeometry( 110, 60, 80, 30 );
		    connect( pb, SIGNAL( clicked() ), this, SLOT( cancel() ) );
		    line1 = new QLabel( klocale->translate("Making directory"), dlg );
		    line1->setGeometry( 10, 10, 200, 20 );
		    line2 = new QLabel( mkdirURL.data() );
		    line2->setGeometry( 10, 10, 200, 20 );
		    dlg->show();
		}
		else
		    dlg = 0L;
	    }
	    break;	
	 default:
	     warning("Case not handled here");
	}
    }
    
    connect( slave, SIGNAL( done() ), this, SLOT( slaveIsReady() ) );
    connect( slave, SIGNAL( data( IPCMemory ) ), this, SLOT( slotData( IPCMemory ) ) );
    connect( slave, SIGNAL( info( const char* ) ), this, SLOT( slotInfo( const char* ) ) );
    connect( slave, SIGNAL( redirection( const char* ) ), this, SLOT( slotRedirection( const char* ) ) );
    connect( slave, SIGNAL( mimeType( const char* ) ), this, SLOT( slotMimeType( const char* ) ) );
    
    started = false;
    cleanedUp = false;
    
    slaveIsReady();
}

void KIOJob::slaveIsReady()
{
    // debugT("SlaveIsReady\n");

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
    case KIOJob::JOB_GET:
	{
	    if ( !started )
		connect( slave, SIGNAL( progress( int ) ), this, SLOT( slaveProgress( int ) ) );
	    
	    if ( cmSrcURLList.count() == 0 )
	    {
		slave->cleanUp();
		cleanedUp = true;
		return;
	    }

	    if ( dlg )
	    {
		line1->setText( cmSrcURLList.first() );
	    }

	    lastSource = cmSrcURLList.first();

	    QString src = completeURL( cmSrcURLList.first() ).data();	    
	    slave->get( src.data() );
	    cmSrcURLList.removeRef( cmSrcURLList.first() );
	}
	break;

    case KIOJob::JOB_COPY:
	{
	    if ( !started )
		connect( slave, SIGNAL( progress( int ) ), this, SLOT( slaveProgress( int ) ) );
	    
	    if ( cmSrcURLList.count() == 0 )
	    {
		slave->cleanUp();
		cleanedUp = true;
		return;
	    }

	    if ( dlg )
	    {
		char buffer[ 1024 ];
		sprintf( buffer, klocale->translate("File %i/%i"), cmCount - cmSrcURLList.count() + 1, cmCount );	    
		line1->setText( buffer );
		line2->setText( cmSrcURLList.first() );
		sprintf( buffer, klocale->translate("to %s"), cmDestURLList.first() );
		line3->setText( buffer );
	    }

	    lastSource = cmSrcURLList.first();
	    lastDest = cmDestURLList.first();

	    QString src = completeURL( cmSrcURLList.first() ).data();	    
	    QString dest = completeURL( cmDestURLList.first() ).data();
	    slave->copy( src.data(), dest.data(), overwriteExistingFiles );
	    cmSrcURLList.removeRef( cmSrcURLList.first() );
	    cmDestURLList.removeRef( cmDestURLList.first() );
	}
	break;

    case KIOJob::JOB_MOVE:
	{
	    if ( !started )
	    {
		connect( slave, SIGNAL( progress( int ) ), this, SLOT( slaveProgress( int ) ) );
		moveDelMode = false;
	    }

	    if ( cmSrcURLList.count() == 0 )
	    {
		char *p;
		for( p = mvDelURLList.first(); p != 0L; p = mvDelURLList.next() )
		{
		    if ( rmdir( p ) == -1 )
		    {
			QString tmp;
			tmp.sprintf( klocale->translate( "Could not delete directory\n%s" ), p );
			QMessageBox::warning( 0, klocale->translate( "KFM Error" ), tmp.data() );
			slave->cleanUp();
			cleanedUp = true;
			delete dlg;
			dlg = 0L;
			done();
			return;
		    }
		}
		mvDelURLList.clear();
		
		slave->cleanUp();
		cleanedUp = true;
		return;
	    }
	 
	    if ( dlg )
	    {
		char buffer[ 1024 ];
		sprintf( buffer, klocale->translate("File %i/%i"), 
			 cmCount - cmSrcURLList.count() + 1, cmCount );	    
		line1->setText( buffer );
		line2->setText( cmSrcURLList.first() );
		sprintf( buffer, klocale->translate("to %s"), cmDestURLList.first() );
		line3->setText( buffer );
	    }
	    	    
	    // In this turn delete the file we copied last turn
	    if ( moveDelMode )
	    {
		QString src = completeURL( cmSrcURLList.first() ).data();
		slave->del( src.data() );
		cmSrcURLList.removeRef( cmSrcURLList.first() );
		cmDestURLList.removeRef( cmDestURLList.first() );
	    }
	    else
	    {
		lastSource = cmSrcURLList.first();
		lastDest = cmDestURLList.first();

		QString dest = completeURL( cmDestURLList.first() ).data();
		QString src = completeURL( cmSrcURLList.first() ).data();
		slave->copy( src.data(), dest.data(), overwriteExistingFiles );
	    }
	    
	    moveDelMode = !moveDelMode;
	}
	break;

    case KIOJob::JOB_DELETE:
	{
	    if ( mvDelURLList.count() == 0 )
	    {
		slave->cleanUp();
		cleanedUp = true;
		return;
	    }

	    if ( dlg != 0L )
	    {
		char buffer[ 1024 ];
		sprintf( buffer, klocale->translate("File %i/%i"), cmCount - mvDelURLList.count() + 1, cmCount );	    
		line1->setText( buffer );
		line2->setText( mvDelURLList.first() );
		if ( cmCount != (int)mvDelURLList.count() )
		     progressBar->setValue( cmCount * 100 / ( cmCount - mvDelURLList.count() ) );
	    }

	    QString dest = completeURL( mvDelURLList.first() ).data();	    
	    slave->del( dest.data() );
	    mvDelURLList.removeRef( mvDelURLList.first() );
	}
	break;
    case KIOJob::JOB_MOUNT:
    case KIOJob::JOB_UNMOUNT:
	{
	    // mount already called ?
	    if ( started )
	    {

		slave->cleanUp();
		cleanedUp = true;
		return;
	    }

	    if ( dlg )
	    {
		char buffer[ 1024 ];
		if ( action == KIOJob::JOB_MOUNT )
		    sprintf( buffer, 
			     klocale->translate("Mounting %s ..."), 
			     mntDev.data() );
		else
		    sprintf( buffer, 
			     klocale->translate("Unmounting %s ..."), 
			     mntPoint.data() );
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
		cleanedUp = true;
		return;
	    }

	    connect( slave, SIGNAL( dirEntry( const char *, const char *, bool, int, const char *,
					      const char *, const char *, const char * ) ), 
		     this, SLOT( slotDirEntry( const char *, const char *, bool, int, const char *,
				       const char *, const char *, const char * ) ) );
	    
	    if ( dlg != 0L )
	    {
		char buffer[ 1024 ];
		sprintf( buffer, 
			 klocale->translate("Retrieving listing of %s ..."), 
			 lstURL.data() );
		line1->setText( buffer );
	    }
	    
	    lstURL = completeURL( lstURL.data() ).data();
	    slave->list( lstURL.data(), bHTML );
	}
	break;
    case KIOJob::JOB_MKDIR:
	{
	    // Already told the slave what to do ?
	    if ( started )
	    {
		slave->cleanUp();
		cleanedUp = true;
		return;
	    }
	    
	    slave->mkdir( mkdirURL.data() );
	}
	break;
    default: // Stephan: added default hander
       warning("case not handled here");
    }

    started = true;
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
    
    if ( progressBar != 0L )
	progressBar->setValue( _percent );

    emit progress( _percent, 0 );
}

void KIOJob::slotData( IPCMemory _mem )
{
    emit data( _mem.data, _mem.size );
}

void KIOJob::slotRedirection( const char *_url )
{
    emit redirection( _url );
}

void KIOJob::slotMimeType( const char *_type )
{
    emit mimeType( _type );
}

void KIOJob::slotInfo( const char *_text )
{
    emit info( _text );
}

void KIOJob::cancel()
{
    // debugT("**********A\n");
    if ( slave )
    {
	KIOSlaveIPC *s = slave;
	slave = 0L;
	pid_t p = (pid_t)s->pid;    
	delete s;
	kill( p, SIGTERM );
    }
    
    slave = 0L;
    
    done();
}

void KIOJob::done()
{
    // debugT("Done\n");
    
    if ( slave != 0L )
    {
	disconnect( server, 0, this, 0 );
	server->freeSlave( slave );
    }
    
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
	// debugT("NOTIFY '%s'\n",s);
	if ( globalNotify )
	{
	    // debugT("NOTIFY IS GLOBAL '%s'\n", s);
	    KIOServer::sendNotify( s );
	}
	emit notify( id, s ); 
    }

    if ( action == KIOJob::JOB_MOUNT || action == KIOJob::JOB_UNMOUNT )
	KIOServer::sendMountNotify();

    emit finished( id );
    
    dlg = 0L;
    slave = 0L;
    
    if ( bAutoDelete )
	delete this;
}

void KIOJob::deleteAllJobs()
{
    KIOJob *j;
    for ( j = jobList.first(); j != 0L; j = jobList.next() )
	j->cancel();
}

QString KIOJob::completeURL( const char *_url )
{
    // debugT("Is '%s' complete ? \n",_url );
    
    KURL u( _url );
    if ( u.isMalformed() )
	return QString( _url );
    
    // debugT("Is not malformed '%s' '%s'\n",u.user(), u.passwd() );
    
    if ( u.user() != 0L && u.user()[0] != 0 && ( u.passwd() == 0L || u.passwd()[0] == 0 ) )
    {
	// debugT("Looking for password\n");
	   
	QString head;
	head.sprintf( "Password for %s@%s", u.user(), u.passwd() );
	
	QString passwd;
	QString tmp2;
	tmp2.sprintf( "%s@%s", u.user(), u.host() );
	
	if ( passwordDict[ tmp2.data() ] == 0L )
	{
	    // debugT("A\n");
	    
	    PasswordDialog *dlg = new PasswordDialog( head.data(), 0L, "", true );
	    if ( !dlg->exec() )
	    {
		// debugT("Cancled\n");
		return QString( _url );
	    }
	    // debugT("B\n");
	    passwd = dlg->password();
	    delete dlg;
	}
	else
	    passwd = passwordDict[ tmp2.data() ]->data();

	// If the password is wrong, the error function will remove it from
	// the dict again.
	passwordDict.insert( tmp2.data(), new QString( passwd.data() ) );
	
	QString tmp;
	tmp.sprintf( "%s:%s", u.user(), passwd.data() );
	QString url = u.url();
	int i = url.find( "@" );
	int j = url.find( "://" );
	url.replace( j + 3, i - ( j + 3 ), tmp.data() );

	return QString( url );
    }
    
    return QString( _url );
}

void KIOJob::slotSlaveClosed()
{
    // We assumed that the slave is going to die.
    if ( slave == 0L )
	return;
    
    slave = 0L;
    
    emit error( KIO_ERROR_SlaveDied, klocale->translate( "Segmentation fault in io subprocess" ) );

    cancel();
}

#include "kiojob.moc"
