#include <qstrlist.h>
#include <qpushbt.h>
#include <qpainter.h>
#include <qdict.h>
#include <kurl.h>
#include <qmsgbox.h>
#include <qmessagebox.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>  

#include "kioserver_ipc.h"
#include "kiojob.h"
#include "kbind.h"
#include "kioslave/kio_errors.h"
#include "krenamewin.h"
#include "passworddialog.h"
#include "utils.h"
#include "config-kfm.h"

#include <klocale.h>
#include <kstring.h>
#include <kwm.h>

#ifndef _PATH_FSTAB
#define _PATH_FSTAB "/etc/fstab"
#endif

QList<KIOJob> *KIOJob::jobList;
QDict<QString> *KIOJob::passwordDict;

/*#define done() \
  {  debug("-------------------- %s, %d : calling done()",__FUNCTION__,__LINE__); this->done(); }
*/

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
    layout = 0L;
    globalNotify = true;
    id = _id;
    overwriteExistingFiles = false;
    reload = false;
    
    jobList->setAutoDelete( false );
    jobList->append( this );
}

KIOJob::~KIOJob()
{
  if ( slave != 0L )
  {
    // printf("################# WARNING: KIOJOB destructor has still kioslave ################\n");
    disconnect( server, 0, this, 0 );
    disconnect( slave, 0, this, 0 );
    server->freeSlave( slave );
    slave = 0L;
  }

  server->removeJob( this );
  jobList->removeRef( this );
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
        warning(QString(i18n("ERROR: Malformed URL"))+" : %s",u.path());
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
    KURL u( _url );
    
    action = KIOJob::JOB_LIST;
    
    bHTML = _bHTML;
   
    // u.setPassword("");
    lstURL = u.url().data();
    if ( lstURL.right(1) != "/" && u.hasPath() )
	lstURL += "/";
    
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
	warning(i18n("INTERNAL ERROR: You must at least specify the device for mounting") );
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

    // Find the mount point an notify us about the changes
    QString n = KIOServer::findDeviceMountPoint( _dev, _PATH_FSTAB );
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
	QString s = KIOServer::getDestNameForLink( p );
	KURL::encodeURL( s );

	QString d = _dest_dir_url;
	d.detach();
	if ( d.length() > 0 && d.data()[ d.length() - 1 ] != '/' )
	    d += "/";
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

	QString supath( su.path() );  // source path
	supath.detach();

	QString dupath( du.path() );  // destination path
	dupath.detach();
	
	// Which directories do we have to notify ?
	if ( notifyList.find( du.directoryURL( false ) ) == -1 )
	    notifyList.append( du.directoryURL( false ) );
	
	if ( su.isMalformed() )
	{
	    QString tmp;
	    ksprintf(&tmp, i18n("Malformed URL\n%s"), p);
	    QMessageBox::warning( 0L, i18n( "KFM Error" ), tmp );
	    done();
	    return;
	}
	else if ( du.isMalformed() )
	{
	    QString tmp;
	    ksprintf(&tmp, i18n("Malformed URL\n%s"), p);
	    QMessageBox::warning( 0L, i18n( "KFM Error" ), tmp );
	    done();
	    return;
	}	
	// I can only make links on the local file system.
	else if ( strcmp( du.protocol(), "file" ) != 0L )
	{
	    QMessageBox::warning( 0L, i18n( "KFM Error" ),
				  i18n("Can only make links on local file system") );
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
		if ( symlink( supath, dupath ) == -1 )
		{
		    // Does the destination already exist ?
		    if ( errno == EEXIST )
		    {
			// Are we allowed to overwrite the files ?
			if ( overwriteExistingFiles )
			{
			    // Try to delete the destination
			    if ( unlink( dupath ) != 0 )
			    {
				QString tmp;
				ksprintf(&tmp, i18n( "Could not overwrite\n%s"), dupath.data() );
				QMessageBox::warning( 0, i18n( "KFM Error" ), tmp );
				done();
				return;
			    }
			}
			else
			{
			    // Ask the user what to do
			    KRenameWin *r = new KRenameWin( 0L, supath, dupath, true );
			    int button = r->exec();
			    if ( button == 0 ) // Overwrite 
			    {
				// Try to delete the destination
				if ( unlink( dupath ) != 0 )
				{
				    delete r;
				    QString tmp;
				    
				    ksprintf(&tmp, i18n( "Could not overwrite\n%s"), dupath.data());
				    QMessageBox::warning( 0, i18n( "KFM Error" ), tmp );
				    done();
				    return;
				}
				// Try again
				if ( symlink( supath, dupath ) == -1 )
				{
				    QString tmp;
				    ksprintf(&tmp, 
					     i18n( "Could not make symlink to\n%s"), 
					     dupath.data());
				    QMessageBox::warning( 0, i18n( "KFM Error" ), tmp );
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
				dupath = du.path();
                                dupath.detach();
				// Try again
				if ( symlink( supath, dupath ) == -1 )
				{
				    QString tmp;
				    ksprintf(&tmp, i18n( "Could not make symlink to\n%s" ), dupath.data());
				    QMessageBox::warning( 0, i18n( "KFM Error" ), tmp );
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
			ksprintf(&tmp, i18n( "Could not make symlink to\n%s"), dupath.data());
			QMessageBox::warning( 0, i18n( "KFM Error" ), tmp );
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
		    else if ( strncmp( su.protocol(), "http", 4 ) == 0 )
			config.writeEntry( "Icon", "www.xpm" );
		    else if ( strcmp( su.protocol(), "info" ) == 0 )
			config.writeEntry( "Icon", "info.xpm" );
		    else if ( strcmp( su.protocol(), "mailto" ) == 0 ) // sven:
			config.writeEntry( "Icon", "kmail.xpm" );      // added mailto: support
		    else
			config.writeEntry( "Icon", KMimeType::getDefaultPixmap() );
		    config.sync();
		    if ( globalNotify )
			KIOServer::sendNotify( p2 );
		    emit notify( id, p2 );
		}
		else
		    warning(i18n(" ERROR: Could not write to %s"),p);
	    }
	}
	p2 = cmDestURLList.next();
    }

    // We dont need a slave, so quit
    done();
}

void KIOJob::get( const char *_url, bool _reload, const char *_data, const char *_cookies )
{
    action = KIOJob::JOB_GET;

    reload = _reload;
    post_data = _data;
    cookies = _cookies;

    cmSrcURLList.clear();
    cmSrcURLList.append( _url );

    server->getSlave( this );
}

void KIOJob::copy( QStrList & _src_url_list, const char *_dest_dir_url )
{
    action = KIOJob::JOB_COPY;
    
    // cmSrcURLList.copy( _src_url_list );
    tmpSrcURLList.clear();
    tmpDestURLList.clear();

    // Fill cmDestURLList
    char *p;
    for ( p = _src_url_list.first(); p != 0L; p = _src_url_list.next() )
    {
	QString tmp = p;
	// Is there a trailing '/' ? Delete is, so that KURL::filename works
	if ( tmp.right(1) == "/" && tmp.right(2) != ":/" )
	    tmp.truncate( tmp.length() - 1 );
	KURL su( tmp.data() );
	QString enc( su.filename() );
	KURL::encodeURL( enc );
	
	QString d = _dest_dir_url;
	d.detach();
	if ( d.right(1) != "/" )
	    d += "/";
	d += enc;

	// printf("############# COPY '%s' to '%s'\n", tmp.data(), d.data());
    
	tmpDestURLList.append( d.data() );
	tmpSrcURLList.append( tmp.data() );
    }

    copy();
}

void KIOJob::copy( const char *_src_url, const char *_dest_url )
{
    action = KIOJob::JOB_COPY;

    tmpSrcURLList.clear();
    tmpDestURLList.clear();    

    QString tmp = _src_url;
    // Is there a trailing '/' ? Delete is, so that KURL::filename works
    if ( tmp.right(1) == "/" && tmp.right(2) != ":/" )
      tmp.truncate( tmp.length() - 1 );
    tmpSrcURLList.append( tmp );

    tmp = _dest_url;
    // Is there a trailing '/' ? Delete is, so that KURL::filename works
    if ( tmp.right(1) == "/" && tmp.right(2) != ":/" )
      tmp.truncate( tmp.length() - 1 );
    tmpDestURLList.append( tmp );

    // printf("############# COPY '%s' to '%s'\n", _src_url, _dest_url);
    
    copy();
}

void KIOJob::copy()
{
    cmSrcURLList.clear();
    cmDestURLList.clear();    

    // Does anybody try to copy a file on itself ?
    QListIterator<char> it3( tmpSrcURLList );
    QListIterator<char> it4( tmpDestURLList );
    for ( ; it3.current(); ++it3 )
    {
        char *p;
	char *p2;
	p = it3.current();
	p2 = it4.current();
	QString tmp;
	
	// Never copy a file on itself!!
	if ( strcmp( p, p2 ) == 0 )
        {
	  do 
	  {      
	    KRenameWin *r = new KRenameWin( 0L, p, p2, true );
	    int button = r->exec();
	    if ( button == 2 ) // Skip
	    {
	      // Do not copy this one
	      goto ende;
	    }
	    else if ( button == 3 ) // Rename
	    {
	      // Get the new destinations name
	      tmp = r->getNewName();
	      // printf("Got '%s'\n",tmp.data());
	      if ( tmp.right(1) == "/" )
		tmp.truncate( tmp.length() - 1 );
	      KURL du( tmp.data() );
	      tmp = du.url();
	      p2 = tmp.data();
	      delete r;
	    }
	    else
	    {
		delete r;
	      done();
	      return;
	    }
	    // printf("Now '%s' '%s'\n",p,p2);
	  }
	  while ( strcmp( p, p2 ) == 0 );
	}

	cmSrcURLList.append( p );
	cmDestURLList.append( p2 );
    ende:
	++it4;
    }

    QStrList tmpList1;
    QStrList tmpList2;

    int skipped = 0;
    QString skippedFile;
    enum { RECURSE_ERROR_REMOTE, RECURSE_ERROR_SUBPROTOCOL, RECURSE_OK } 
       recurse = RECURSE_OK;
    
    // Recursive directory    
    QListIterator<char> it( cmSrcURLList );
    QListIterator<char> it2( cmDestURLList );
    for ( ; it.current(); ++it )
    {
        char *p;
	char *p2;
	p = it.current();
	p2 = it2.current();
	KURL su( p );
	KURL du( p2 );
	
	if ( su.isLocalFile() && du.isLocalFile() )
	{
	    struct stat buff;

	    QString supath( su.path() );  // source path
	    supath.detach();

	    QString dupath( du.path() );  // destination path
	    dupath.detach();

	    lstat( supath.data(), &buff );
            if ( S_ISLNK( buff.st_mode ) )
            {
                char buffer [1024];
                int linkLength = readlink( supath.data(), buffer, 1022 );
                if (linkLength > 0)
                {
                     buffer[linkLength] = 0;
                     if (symlink( buffer, dupath.data()) == -1)
                     {
                     	QString tmp;
			ksprintf(&tmp, i18n( "Could not make symbolic link\n%s"), dupath.data());
			QMessageBox::warning( 0, i18n( "KFM Error" ), tmp.data() );
			done();
			return;
                     }
                }

		// Remove links from the copy list
		tmpList1.append( p );
		tmpList2.append( p2 );
            }
	    else if ( S_ISDIR( buff.st_mode ) )
	    {
		if ( ::mkdir( dupath.data(), buff.st_mode ) == -1 )
                {    
		    if ( errno != EEXIST )
		    {
			QString tmp;
			ksprintf(&tmp, i18n( "Could not make directory\n%s"), dupath.data());
			QMessageBox::warning( 0, i18n( "KFM Error" ), tmp.data() );
			done();
			return;
		    }
		}

		DIR *dp;
		struct dirent *ep;
		dp = opendir( supath );
		if ( dp == NULL )
		{
		    warning(i18n("ERROR: Could not access directory '%s'"), p );
		    done();
		    return;
		}
		
		while ( (ep = readdir( dp ) ) != 0L )
		{
		    if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
		    {
			// append directory-contents do 'todo'-list
			QString fname = ep->d_name;
			fname.detach();
			KURL::encodeURL (fname);

			QString s = p;
			s.detach();
			if ( s.length() > 0 && s.data()[ s.length() - 1 ] != '/' )
			    s += "/";
			s += fname;

			
			QString d = p2;
			d.detach();
			if ( d.length() > 0 && d.data()[ d.length() - 1 ] != '/' )
			    d += "/";
			d += fname;
			
			cmSrcURLList.append( s.data() );
			cmDestURLList.append( d.data() );
			// printf("Adding '%s' -> '%s'\n", s.data(), d.data() );
		    }
		}
		
		closedir( dp );
		// Remove directories from the copy list
		tmpList1.append( p );
		tmpList2.append( p2 );
	    }
	    else if ( !S_ISREG( buff.st_mode ) )
	    {
		// Remove non-regular files from the copy list
		// If we copy fifo/sockets/device-files hell breakes out!
		skipped++;
		skippedFile = supath.data();
		tmpList1.append( p );
		tmpList2.append( p2 );
	    }
	} else // src or/and dest isn't local.
        {
            if ( KIOServer::isDir( p ) == 1 ) // is source a directory ?
            {
                if ( su.hasSubProtocol() )
                    recurse = RECURSE_ERROR_SUBPROTOCOL;
                else
                    recurse = RECURSE_ERROR_REMOTE;
            }
        }
	++it2;
    }

    if (skipped)
    {
	QString tmp;
	if (skipped == 1)
	    ksprintf(&tmp, i18n( "Special file will not be copied\n%s"), skippedFile.data());
	else
	    ksprintf(&tmp, i18n( "%d special files will not be copied."), skipped);
	QMessageBox::warning( 0, i18n( "KFM Warning" ), tmp.data() );
    }
    if (recurse == RECURSE_ERROR_SUBPROTOCOL)
    {
        emit fatalError (KIO_ERROR_NotImplemented, i18n("Recurse copying from file with sub-protocol\n(for instance, tar files)"), 0);
        return;
    }
    if (recurse == RECURSE_ERROR_REMOTE)
    {
        emit fatalError (KIO_ERROR_NotImplemented, i18n("Recurse copying from/to distant locations"), 0);
        return;
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

    if (!cmCount)
    {
        done();
        return;
    }
    
    server->getSlave( this );
}

void KIOJob::move( const char *_src_url, const char *_dest_url )
{
    action = KIOJob::JOB_MOVE;

    tmpSrcURLList.clear();
    tmpDestURLList.clear();    

    QString tmp = _src_url;
    // Is there a trailing '/' ? Delete is, so that KURL::filename works
    if ( tmp.right(1) == "/" && tmp.right(2) != ":/" )
      tmp.truncate( tmp.length() - 1 );
    tmpSrcURLList.append( tmp );

    tmp = _dest_url;
    // Is there a trailing '/' ? Delete is, so that KURL::filename works
    if ( tmp.right(1) == "/" && tmp.right(2) != ":/" )
      tmp.truncate( tmp.length() - 1 );
    tmpDestURLList.append( tmp );

    move();
}

void KIOJob::move( QStrList & _src_url_list, const char *_dest_dir_url )
{
    action = KIOJob::JOB_MOVE;
    
    tmpSrcURLList.clear();
    tmpDestURLList.clear();

    char *p;
    for ( p = _src_url_list.first(); p != 0L; p = _src_url_list.next() )
    {
	QString tmp = p;
	// Is there a trailing '/' ? Delete is, so that KURL::filename works
	if ( tmp.right(1) == "/" && tmp.right(2) != ":/" )
	    tmp.truncate( tmp.length() - 1 );
	KURL su( tmp.data() );
	QString enc( su.filename() );
	KURL::encodeURL( enc );
	
	QString d = _dest_dir_url;
	d.detach();
	if ( d.right(1) != "/" )
	    d += "/";
	d += enc;
	
	tmpSrcURLList.append( tmp.data() );
	tmpDestURLList.append( d.data() );
    }

    move();
}

void KIOJob::move()
{
    tmpDelURLList.clear();
    cmDestURLList.clear();
    cmSrcURLList.clear();

    int skipped = 0;
    QString skippedFile;    
    
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

	QString tmp;
	// Never copy a file on itself!!
        // on local filesystems: maybe one should check inodes here (hardlinks!) (hen)
	if ( strcmp( p, p2 ) == 0 ) 
        {
	  do 
	  {      
	    KRenameWin *r = new KRenameWin( 0L, p, p2, true );
	    int button = r->exec();
	    if ( button == 3 ) // Rename
	    {
	      // Get the new destinations name
	      tmp = r->getNewName();
	      // printf("Got '%s'\n",tmp.data());
	      if ( tmp.right(1) == "/" )
		tmp.truncate( tmp.length() - 1 );
	      du = tmp.data();
	      tmp = du.url();
	      p2 = tmp.data();
	      delete r;
	    }
	    else
	    {
		delete r;
	      done();
	      return;
	    }
	    // printf("Now '%s' '%s'\n",p,p2);
	  }
	  while ( strcmp( p, p2 ) == 0 );
	}
	

	QString supath( su.path() );  // source path
	supath.detach();

	QString dupath( du.path() );  // destination path
	dupath.detach();
	
	int i = 1;
	// Moving on the local hard disk ?
	if ( su.isLocalFile() && du.isLocalFile() )
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
		  goto ende2;
		}
		else if ( button == 3 ) // Rename
		{
		  du = r->getNewName();
		  dupath = du.path();
		}
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
	
	    // Did we succeed with our call to 'rename'
	    if ( i == -1 )
	    {
	      // We tried to move across devices ?
              // Ok, that is not a real error.
              if ( errno == EXDEV )
              {
                struct stat buff;
                lstat( supath, &buff );
                if ( S_ISLNK( buff.st_mode ) )
                {
                    // It's a link, we can move it ourselves
                    // First generate the notifies (this should be a function call!)
                    {
                        QString tmp = p;
                        if ( tmp.right(1) == "/" )
                            tmp.truncate( tmp.length() - 1 );
                        KURL u( tmp );
                        if ( notifyList.find( u.directoryURL( false ) ) == -1 )
                            notifyList.append( u.directoryURL( false ) );
                        tmp = p2;
                        if ( tmp.right(1) == "/" )
                            tmp.truncate( tmp.length() - 1 );
                        u = tmp.data();
                        if ( notifyList.find( u.directoryURL( false ) ) == -1 )
                            notifyList.append( u.directoryURL( false ) );
                    }

                    char buffer [1024];
                    int linkLength = readlink( supath.data(), buffer, 1022 );
                    if (linkLength > 0)
                    {
                      buffer[linkLength] = 0;
                      if (overwriteExistingFiles) unlink(dupath.data()); // remove dest
                      if (symlink( buffer, dupath.data()) == -1)
                      {
                        QString tmp;
                        ksprintf(&tmp, i18n( "Could not make symbolic link\n%s"), dupath.data());
                        QMessageBox::warning( 0, i18n( "KFM Error" ), tmp.data() );
                        done();
                        return;
                      }
                    }
                   
                    // tmpDelURLList.append( supath ); // delete this link at the end
                    // Commented out by David. rmdir on it will fail.
                    // Instead we want to remove it now (the symlink() call succeeded anyway)
                    if (unlink(supath.data()) == -1)
                    {
                        QString tmp;
                        ksprintf(&tmp, i18n( "Could not remove symbolic link\n%s"), supath.data());
                        QMessageBox::warning( 0, i18n( "KFM Error" ), tmp.data() );
                        done();
                        return;
                    }
                }

                // We want to move a directory ?
                // Then we must know each file in the tree ....
                //** maybe here we should call copy(old,new); del(old) ? (hen) **/
               
		else if ( S_ISDIR( buff.st_mode ) )
		{
		  if ( ::mkdir( dupath, S_IRWXU ) == -1 )
		    if ( errno != EEXIST )
		    {
		      QString tmp;
		      ksprintf(&tmp, i18n( "Could not make directory\n%s"), dupath.data());
		      QMessageBox::warning( 0, i18n( "KFM Error" ), tmp.data() );
		      done();
		      return;
		    }
		  
		  // Dont forget to delete this directory at the end
		  tmpDelURLList.append( supath );
		  
		  // Create notifys for the directories
		  {
		    QString tmp = p;
		    if ( tmp.right(1) == "/" )
		      tmp.truncate( tmp.length() - 1 );
		    KURL u( tmp );
		    if ( notifyList.find( u.directoryURL( false ) ) == -1 )
		      notifyList.append( u.directoryURL( false ) );
		    tmp = p2;
		    if ( tmp.right(1) == "/" )
		      tmp.truncate( tmp.length() - 1 );
		    u = tmp.data();
		    if ( notifyList.find( u.directoryURL( false ) ) == -1 )
		      notifyList.append( u.directoryURL( false ) );
		  }

		  DIR *dp;
		  struct dirent *ep;
		  dp = opendir( supath );
		  if ( dp == NULL )
		  {
		    QString tmp;
		    ksprintf(&tmp, i18n( "Could not access directory\n%s"), dupath.data());
		    QMessageBox::warning( 0, i18n( "KFM Error" ), tmp.data() );
		    done();
		    return;
		  }
		  
		  while ( ( ep = readdir( dp ) ) != 0L )
		  {
		    if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
		    {
		      QString fname = ep->d_name;
		      fname.detach();
		      KURL::encodeURL (fname);

		      QString s = p;
		      s.detach();
		      if ( s.right(1) != "/" )
			s += "/";
		      s += fname;
		      
		      QString d = p2;
		      d.detach();
		      if ( d.right(1) != "/" )
			d += "/";
		      d += fname;
		      
		      tmpSrcURLList.append( s.data() );
		      tmpDestURLList.append( d.data() );
		    }
		  }
		  (void) closedir ( dp );
		}
                else if ( !S_ISREG( buff.st_mode ) )
                {
                  // Don't move non-regular files
                  // If we copy fifo/sockets/device-files hell breakes out!
                  skipped++;
                  skippedFile = supath.data();
                }
		else // A usual file
		{
		  cmSrcURLList.append( p );
		  cmDestURLList.append( p2 );
		}
	      }
	      else
	      {
		QString tmp;
		ksprintf(&tmp, i18n( "Could not move directory\n%s\nto %s\nPerhaps access denied"), supath.data(), dupath.data());
		QMessageBox::warning( 0, i18n( "KFM Error" ), tmp.data() );
		done();
		return;
	      }
	    }
	    else // We already moved the file => We just need to generate notifys
	    {
	      QString tmp = p;
	      if ( tmp.right(1) == "/" )
		tmp.truncate( tmp.length() - 1 );
	      KURL u( tmp );
	      if ( notifyList.find( u.directoryURL( false ) ) == -1 )
		notifyList.append( u.directoryURL( false ) );
	      tmp = p2;
	      if ( tmp.right(1) == "/" )
		tmp.truncate( tmp.length() - 1 );
	      u = tmp.data();
	      if ( notifyList.find( u.directoryURL( false ) ) == -1 )
		notifyList.append( u.directoryURL( false ) );
	    }
	}
	else // Some URL movement ( not both local files )
	{
	  cmSrcURLList.append( p );
	  cmDestURLList.append( p2 );
	}

    ende2:
	// Dont forget to get the next dest. We get the corresponding src
	// in the 'for' clause.
	++itDest;
    }
    
    // Create a list of all directories we wish to notify about changes.
    char *s;
    for ( s = cmDestURLList.first(); s != 0L; s = cmDestURLList.next() )
    {
	QString tmp = s;
	if ( tmp.right(1) == "/" )
	    tmp.truncate( tmp.length() - 1 );
	KURL u( tmp );
	if ( notifyList.find( u.directoryURL( false ) ) == -1 )
	{
	  notifyList.append( u.directoryURL( false ) );
	}
    }
    for ( s = cmSrcURLList.first(); s != 0L; s = cmSrcURLList.next() )
    {
	QString tmp = s;
	if ( tmp.right(1) == "/" )
	    tmp.truncate( tmp.length() - 1 );
	KURL u( tmp );
	if ( notifyList.find( u.directoryURL( false ) ) == -1 )
	{
	  notifyList.append( u.directoryURL( false ) );
	}
    }
    
    cmCount = cmSrcURLList.count();
    
    // Delete subdirectories first => inverse the order of the list
    mvDelURLList.clear();
    for ( p = tmpDelURLList.last(); p != 0L; p = tmpDelURLList.prev() )
    {
	mvDelURLList.append( p );
    }

    if (skipped)
    {
	QString tmp;
	if (skipped == 1)
	    ksprintf(&tmp, i18n( "Special file will not be moved\n%s"), skippedFile.data());
	else
	    ksprintf(&tmp, i18n( "%d special files will not be moved."), skipped);
	QMessageBox::warning( 0, i18n( "KFM Warning" ), tmp.data() );
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

	    if( notifyList.find( u.url() ) == -1 )
	      notifyList.append( u.url() );
	}
	else if ( notifyList.find( u.directoryURL() ) == -1 )
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
	    QString supath( su.path() );  // file to delete
	    supath.detach();

	    // Delete a trailing '/', for lstat
	    if (( supath.length() > 1 ) && ( supath.right(1) == "/" ))
	    supath.truncate( supath.length() - 1 );

	    struct stat buff;
	    stat( supath, &buff );
	    struct stat lbuff;
	    lstat( supath, &lbuff );

	    if ( S_ISLNK( lbuff.st_mode ) )
	    {
		// No recursion here!
                // This is a symlink. If it points to a directory, it may have
                // a '/' appended, which we have to remove
                if ( S_ISDIR( buff.st_mode ) )
                {
                    char * e = p + strlen(p) - 1;
                    if (*e=='/') *e='\0';
                }
	    }
	    else if ( S_ISDIR( buff.st_mode ) )
	    {
		DIR *dp;
		struct dirent *ep;
		dp = opendir( supath );
		if ( dp == NULL )
		{
		    warning(i18n("ERROR: Could not access directory '%s'"), p );
		    return;
		}

		while ( ( ep = readdir( dp ) ) != 0L )
		{
		    if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
		    {
		        QString fname = ep->d_name;
		        fname.detach();
		        KURL::encodeURL (fname);

			QString s = p;
			s.detach();
			if ( s.length() > 0 && s.data()[ s.length() - 1 ] != '/' )
			    s += "/";
			s += fname;

			// debugT("Appending '%s'\n",s.data());
			tmpDelURLList.append( s.data() );
		    }
		}
		(void) closedir ( dp );
	    }
	}
    }
    // reverse list
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
    connect( slave, SIGNAL( closed( KIOSlaveIPC* ) ), this, SLOT( slotSlaveClosed( KIOSlaveIPC* ) ) );

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

void KIOJob::fatalError( int _kioerror, const char* _error, int _errno )
{
  // Assume we want to process this error
  bProcessError = true;
  
  // Allow others to see and process the error
  emit errorFilter( _kioerror, _error, _errno );
  
  // Is the error already process ? (should NEVER be now)
  if ( bProcessError )
  {
    // We process the error.
    processError( _kioerror, _error, _errno );
  }
}

void KIOJob::processError( int _kioerror, const char* _error, int )
{
    bProcessError = false;
    int retVal = -1;
    bool okToContinue = TRUE;

    kioError = _kioerror;
    
    // printf("################################# processError called '%s', %d\n",_error,_kioerror);
    
    // We have to delete the password!!! ( if there is one )
    QString url( _error );
    KURL u( _error );
    if ( !u.isMalformed() )
    {
      u.setPassword( "" );
      url = stringSqueeze( u.url(), 100 );
    }
    
    KRenameWin *r = 0L;
    
    QString msg = "";

    switch( action )
    {
    case KIOJob::JOB_LIST:
    case KIOJob::JOB_COPY:
    case KIOJob::JOB_MKDIR:
    case KIOJob::JOB_DELETE:
    case KIOJob::JOB_MOVE:
    case KIOJob::JOB_LINK:
      switch( _kioerror ) {
      case KIO_ERROR_MalformedURL:
	ksprintf(&msg, i18n("Malformed URL\n%s"), url.data()); 
	break;
      case KIO_ERROR_CouldNotRead:
	ksprintf(&msg, i18n("Could not read\n%s\nFile does not exist or access denied"), url.data());
	break;
      case KIO_ERROR_CouldNotWrite:
	ksprintf(&msg, i18n("Could not write\n%s\nPerhaps access denied"),
		 url.data());
	if (action==JOB_MOVE) okToContinue = FALSE; // dangerous to continue
	break;
      case KIO_ERROR_CouldNotCreateSocket:
	ksprintf(&msg, i18n("Could not create Socket for\n%s"), url.data());
	break;
      case KIO_ERROR_UnknownHost:
	ksprintf(&msg, i18n("Unknown host in\n%s"), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_CouldNotConnect:
	ksprintf(&msg, i18n("Could not connect to\n%s"), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_NotImplemented:
	ksprintf(&msg, i18n("The requested action\n%s\nis not implemented yet."), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_CouldNotMkdir:
	ksprintf(&msg, i18n("Could not make directory\n%s"), url.data());
	if (action==JOB_MOVE) okToContinue = FALSE; // dangerous to continue
	break;
      case KIO_ERROR_CouldNotList:
	ksprintf(&msg,i18n("Could not list directory contents\n%s"), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_CouldNotDelete:
	ksprintf(&msg, i18n("Could not delete\n%s\nURL does not exist or permission denied"), url.data());
	break;
      case KIO_ERROR_CouldNotLogin: {
	KURL u( _error );
	ksprintf(&msg, i18n("Could not login for\n%s\nPerhaps wrong password"), u.host());
	// Remove the password from the dict, since it seems to be wrong.
	if ( u.user() != 0L && u.user()[0] != 0 && 
	     u.host() != 0L && u.host()[0] != 0 ) {
	  QString tmp;
	  ksprintf(&tmp, "%s@%s", u.user(), u.host());
	  passwordDict->remove( tmp.data() );
	}
	okToContinue = FALSE;
	break;
      }
      case KIO_ERROR_TarError:
	ksprintf(&msg, i18n("Tar reported an error\n%s"), url.data());
	break;
      case KIO_ERROR_GzipError:
	ksprintf(&msg, i18n("Gzip reported an error for\n%s"), url.data());
	break;
      case KIO_ERROR_FileExists:
	r = new KRenameWin( 0L, lastSource.data(), lastDest.data(), false );
	break;
      case KIO_ERROR_NotPossible:
	ksprintf(&msg, i18n("Operation technically impossible"));
	break;
      case KIO_ERROR_FileDoesNotExist:
	ksprintf(&msg,i18n("File %s\ndoes not exist"), url.data());
	break;
      case KIO_ERROR_DiskFull:
	ksprintf(&msg, i18n("Could not write\n%s\nPerhaps disk full"), url.data());
	okToContinue = FALSE;
        break;
      }
      break;
    case KIOJob::JOB_MOUNT:
    case KIOJob::JOB_UNMOUNT:
    case KIOJob::JOB_GET:
      switch( _kioerror ) {
      case KIO_ERROR_FileDoesNotExist:
	ksprintf(&msg,i18n("File %s\ndoes not exist"), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_MalformedURL:
	ksprintf(&msg,i18n("Malformed URL\n%s"), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_CouldNotRead:
	ksprintf(&msg,i18n("Could not read\n%s\nFile does not exist or access denied"), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_CouldNotCreateSocket:
	ksprintf(&msg,i18n("Could not create Socket for\n%s"), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_UnknownHost:
	ksprintf(&msg,i18n("Unknown host in\n%s"), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_CouldNotConnect:
	ksprintf(&msg,i18n("Could not connect to\n%s"), url.data());;
	okToContinue = FALSE;
	break;
      case KIO_ERROR_NotImplemented:
	ksprintf(&msg,i18n("The requested action\n%s\nis not implemented yet."), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_CouldNotLogin: {
	KURL u( _error );
	ksprintf(&msg,i18n("Could not login for\n%s\nPerhaps wrong password"), u.host());
	// Remove the password from the dict, since it seems to be wrong.
	if ( u.user() != 0L && u.user()[0] != 0 && 
	     u.passwd() != 0L && u.passwd()[0] != 0 ) {
	  QString tmp;
	  ksprintf(&tmp, "%s@%s", u.user(), u.host());
	  passwordDict->remove( tmp.data() );
	}
	okToContinue = FALSE;
	break;
      }
      case KIO_ERROR_TarError:
	ksprintf(&msg,i18n("Tar reported a fatal error\n%s"), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_GzipError:
	ksprintf(&msg, i18n("Gzip reported a fatal error\n%s"), url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_CouldNotMount:
	ksprintf(&msg, i18n("Could not mount\nError log:\n\n%s"),url.data());
	okToContinue = FALSE;
	break;
      case KIO_ERROR_CouldNotUnmount:
	ksprintf(&msg, i18n("Could not unmount\nError log:\n\n%s"),url.data());
	okToContinue = FALSE;
	break;
      }
      break;
    default:
      kdebug(1,1201,"warning, case not handled in processError()");
      kdebug(1,1201,"action = %d, kioerror = %d", action, _kioerror);
      kdebug(1,1201,"error msg = %s", _error);
      emit error( _kioerror, "" );
      return;
    }
	
    if (r != 0) { // the rename dialog should be executed
      connect(r, SIGNAL(result(QWidget *, int, const char *,
			       const char *)),
	      this, SLOT(msgResult2(QWidget *, int, const char *,
				    const char *)));
      r->show(); // this is a modal window
    } else { // the error dialog should be executed (modal).
      if ( bDisplay )
	if (okToContinue) {
	  retVal = QMessageBox::warning(0L, i18n("KFM Error"),
					msg.data(), i18n("Continue"),
					i18n("Cancel"));
	  if (retVal == 0)
	    slaveIsReady();
	  else if (retVal == 1)
          {
	    done();
	    return;
          }
	} else {
	  done(); // do it before, to avoid any signal called while msgbox is shown
	  QMessageBox::warning(0L, i18n("KFM Error"),
			       msg.data(), i18n("Cancel"));
	  return;
	}
    }

    if ( !msg.isEmpty() && r == 0L )
	emit error( _kioerror, msg );
    else if ( r == 0L ) // some sort of error w/o a message?
	emit error( _kioerror, "" );
}

void KIOJob::start( int _pid )
{
    slave->pid = _pid;

    if ( bDisplay )
    {
	dlg = 0L;

	switch( action )
	{
	case KIOJob::JOB_GET:
	    {
		dlg = new QDialog( 0L );
		progressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, dlg );
		line1 = new QLabel( dlg );
	    }
	    break;
	case KIOJob::JOB_COPY:
	case KIOJob::JOB_MOVE:
	    {
		dlg = new QDialog( 0L );
		progressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, dlg );
		line1 = new QLabel( dlg );
		line2 = new QLabel( dlg );
		line3 = new QLabel( dlg );
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
		    progressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, dlg );
		    line1 = new QLabel( dlg );
		    line2 = new QLabel( dlg );
		}
	    }
	    break;
	case KIOJob::JOB_MOUNT:
	case KIOJob::JOB_UNMOUNT:
	    {
		dlg = new QDialog( 0L );
		line1 = new QLabel( dlg );
	    }
	    break;
	case KIOJob::JOB_LIST:
	    {
		KURL u( lstURL.data() );
		if ( strcmp( u.protocol(), "ftp" ) == 0 )
		{
		    dlg = new QDialog( 0L );
		    line1 = new QLabel( dlg );
		}
	    }
	    break;
	case KIOJob::JOB_MKDIR:
	    {
		KURL u( mkdirURL.data() );
		if ( strcmp( u.protocol(), "ftp" ) == 0 )
		{
		    dlg = new QDialog( 0L );
		    line1 = new QLabel( i18n("Making directory"), dlg );
		    line2 = new QLabel( mkdirURL.data() );
		}
	    }
	    break;	
	 default:
	     warning("Case not handled here");
	}

	if ( dlg != 0L )
	{
	  // make sure window doesn't initially get wm focus
	  KWM::setDecoration(dlg->winId(), 
			     KWM::normalDecoration | KWM::noFocus);

	    layout = new QVBoxLayout( dlg, 10, 0 );
	    layout->addStrut( 360 );	// makes dlg at least that wide
	    if ( line1 != 0L )
	    {
		line1->setFixedHeight( 20 );
		layout->addWidget( line1 );
	    }
	    if ( line2 != 0L )
	    {
		line2->setFixedHeight( 20 );
		layout->addWidget( line2 );
	    }
	    if ( line3 != 0L )
	    {
		line3->setFixedHeight( 20 );
		layout->addWidget( line3 );
	    }
	    if ( progressBar != 0L )
	    {
		progressBar->setFixedHeight( 20 );
		layout->addSpacing( 10 );
		layout->addWidget( progressBar );
	    }
	    QPushButton *pb = new QPushButton( i18n("Cancel"), dlg );
	    pb->setFixedSize( pb->sizeHint() );
	    pb->setDefault(TRUE);
	    connect( pb, SIGNAL( clicked() ), this, SLOT( cancel() ) );
	    layout->addSpacing( 10 );
	    layout->addWidget( pb );

	    layout->addStretch( 10 );
	    layout->activate();
	    dlg->resize( dlg->sizeHint() );
	    dlg->show();
	}
    }
    
    connect( slave, SIGNAL( done() ), this, SLOT( slaveIsReady() ) );
    connect( slave, SIGNAL( data( IPCMemory ) ), this, SLOT( slotData( IPCMemory ) ) );
    connect( slave, SIGNAL( info( const char* ) ), this, SLOT( slotInfo( const char* ) ) );
    connect( slave, SIGNAL( redirection( const char* ) ), this, SLOT( slotRedirection( const char* ) ) );
    connect( slave, SIGNAL( mimeType( const char* ) ), this, SLOT( slotMimeType( const char* ) ) );
    connect( slave, SIGNAL( cookie( const char*, const char* ) ), this, SLOT( slotCookie( const char*, const char* ) ) );
    
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
	    slave->get( src.data(), post_data.data(), cookies.data(), reload );
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
		QString buffer;
		buffer.sprintf( i18n("File %i/%i"), cmCount - cmSrcURLList.count() + 1, cmCount );
		line1->setText( buffer );

		QString curFile;
		curFile = cmSrcURLList.first();
		KURL::decodeURL( curFile );
		line2->setText( curFile );

		curFile = cmDestURLList.first();
		KURL::decodeURL( curFile );
		ksprintf(&buffer, i18n("to %s"), curFile.data() );
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
			ksprintf(&tmp, i18n( "Could not delete directory\n%s" ), p);
			QMessageBox::warning( 0, i18n( "KFM Error" ), tmp.data() );
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
		QString buffer;
		buffer.sprintf( i18n("File %i/%i"), 
			 cmCount - cmSrcURLList.count() + 1, cmCount );	    
		line1->setText( buffer );

		QString curFile;
		curFile = cmSrcURLList.first();
		KURL::decodeURL( curFile );
		line2->setText( curFile );

		curFile = cmDestURLList.first();
		KURL::decodeURL( curFile );
		ksprintf(&buffer, i18n("to %s"), curFile.data());
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
		QString buffer;
		buffer.sprintf( i18n("File %i/%i"), cmCount - mvDelURLList.count() + 1, cmCount );	    
		line1->setText( buffer );

		QString curFile;
		curFile = mvDelURLList.first();
		KURL::decodeURL( curFile );
		line2->setText( curFile );

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
		QString buffer;
		if ( action == KIOJob::JOB_MOUNT )
		    buffer.sprintf( i18n("Mounting %s ..."), mntDev.data() );
		else
		    buffer.sprintf( i18n("Unmounting %s ..."), mntPoint.data() );
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
		QString buffer;
		ksprintf(&buffer, i18n("Retrieving listing of %s"), lstURL.data());
		line1->setText( buffer );
	    }
	    
	    // Fill in password if we know it
	    QString s = completeURL( lstURL.data() ).data();
	    slave->list( s.data(), bHTML );
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
	    
	    // Fill in password if we know it
	    QString s = completeURL( mkdirURL.data() ).data();
	    slave->mkdir( s.data() );
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
    // The data will arrive at the cache never the less
    if ( lstURL == _url )
    {
	KIODirectoryEntry e( _name, _isDir, _size, _creationDate, _access, _owner, _group );

	emit newDirEntry( id, &e );
    }
    /* else
	printf("Does not fit '%s' '%s'\n",lstURL.data(),_url); */
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
  // printf("KIOJOB::redirection\n");
    if ( action == JOB_LIST )
    {
	KURL u( _url );
	lstURL = _url;
	if ( lstURL.right(1) != "/" && u.hasPath() )
	    lstURL += "/";
    }
    
    emit redirection( _url );
}

void KIOJob::slotMimeType( const char *_type )
{
    emit mimeType( _type );
}

void KIOJob::slotCookie( const char *_url, const char *_cookie_str )
{
    emit cookie( _url, _cookie_str );
}

void KIOJob::slotInfo( const char *_text )
{
    emit info( _text );
}

void KIOJob::cancel()
{
    server->removeJob( this );

    // printf("**********A\n");
    if ( slave )
    {
      // printf("Killing slave\n");
	KIOSlaveIPC *s = slave;
	slave = 0L;
	pid_t p = (pid_t)s->pid;    
	delete s;
	kill( p, SIGTERM );
	int status;
	waitpid( p, &status, 0 );
    }
    
    slave = 0L;
    
    done();
}

//#undef done
void KIOJob::done()
{
  // printf("Done\n");

    server->removeJob( this );
    
    disconnect( server, 0, this, 0 );

    if ( slave != 0L )
    {
      // printf("Handing slave back\n");
	disconnect( slave, 0, this, 0 );
	server->freeSlave( slave );
    }
    
    if ( dlg != 0L )
    {
        if ( layout )
	  delete layout;
	if ( line1 )
	  delete line1;
	if ( line2 )
	  delete line2;
	if ( line3 )
	  delete line3;
	
	delete dlg;
	layout = 0L;
	line1 = line2 = line3 = 0L;
	dlg = 0L;
    }

    char *s;
    for ( s = notifyList.first(); s != 0L; s = notifyList.next() )
    {
	if ( globalNotify )
	{
	    KIOServer::sendNotify( s );
	}
	emit notify( id, s ); 
    }

    if ( action == KIOJob::JOB_MOUNT || action == KIOJob::JOB_UNMOUNT )
	KIOServer::sendMountNotify();

    emit finished( id );
    
    slave = 0L;
    
    if ( bAutoDelete )
	delete this;
}

void KIOJob::deleteAllJobs()
{
    KIOJob *j;
    for ( j = jobList->first(); j != 0L; j = jobList->next() )
	j->cancel();
}

QString KIOJob::completeURL( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return QString( _url );
    
    // We got a user, but no password ?
    if ( u.user() != 0L && u.user()[0] != 0 && ( u.passwd() == 0L || u.passwd()[0] == 0 ) )
    {
	QString head;
	ksprintf(&head, "Password for %s@%s", u.user(), u.host());
	
	QString passwd;
	QString tmp2;
	ksprintf(&tmp2, "%s@%s", u.user(), u.host());
	
	if ( (*passwordDict)[ tmp2.data() ] == 0L )
	{
	    PasswordDialog *dlg = new PasswordDialog( head.data(), 0L, "", true );
	    if ( !dlg->exec() )
	    {
		return QString( _url );
	    }
	    passwd = dlg->password();
	    KURL::encodeURL(passwd);
	    delete dlg;

	    // If the password is wrong, the error function will remove it from
	    // the dict again.
	    passwordDict->insert( tmp2.data(), new QString( passwd.data() ) );
	}
	else
	    passwd = (*passwordDict)[ tmp2.data() ]->data();
	
	QString tmp;
	tmp << u.user() << ":" << passwd.data();
	QString url = u.url();
	int i = url.find( "@" );
	int j = url.find( "://" );
	url.replace( j + 3, i - ( j + 3 ), tmp.data() );

	return QString( url );
    }
    // We have a user and a password => we want to remember that.
    else if ( u.user() != 0L && u.user()[0] != 0 && u.passwd() != 0L && u.passwd()[0] != 0 )
    {
	QString tmp2;
	ksprintf(&tmp2, "%s@%s", u.user(), u.host());

	// We dont know this passwort ?
	if ( (*passwordDict)[ tmp2.data() ] == 0L )
	    passwordDict->insert( tmp2.data(), new QString( u.passwd() ) );
    }
    
    return QString( _url );
}

void KIOJob::slotSlaveClosed( KIOSlaveIPC* )
{
    // We assumed that the slave is going to die.
    if ( slave == 0L )
	return;
    
    slave = 0L;
    
    emit error( KIO_ERROR_SlaveDied, i18n( "Segmentation fault in io subprocess" ) );

    cancel();
}

#include "kiojob.moc"
