#include <qdir.h>
#include <qstrlist.h>
#include <qdict.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>

#include <kconfig.h>

#ifdef HAVE_VOLMGT
#include <volmgt.h>
#include <sys/mnttab.h>
#endif

#ifdef __FreeBSD__
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif      

#include "kioserver_ipc.h"
#include "kioserver.h"
#include "kbind.h"
#include "kfmpaths.h"
#include "utils.h"
#include "config-kfm.h"
#include "bookmark.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

KIOServer* KIOServer::pKIOServer;

KIODirectoryEntry::KIODirectoryEntry( const char *_name, bool _isDir, int _size, const char * _creationDate,
				      const char * _access, const char * _owner, const char *_group )
{
    name = _name;
    name.detach();
    bDir = _isDir;
    size = _size;
    creationDate = _creationDate;
    creationDate.detach();
    int i;
    if ( ( i = creationDate.find( ' ' ) ) != -1 )
	creationDate.replace( i, 1, "&nbsp;" );
    access = _access;
    access.detach();
    owner = _owner;
    owner.detach();
    group = _group;
    group.detach();
}

KIODirectoryEntry::KIODirectoryEntry( KIODirectoryEntry & _entry )
{
    name = _entry.getName();
    name.detach();
    bDir = _entry.isDir();
    size = _entry.getSize();
    creationDate = _entry.getCreationDate();
    creationDate.detach();
    int i;
    if ( ( i = creationDate.find( ' ' ) ) != -1 )
	creationDate.replace( i, 1, "&nbsp;" );
    access = _entry.getAccess();
    access.detach();
    owner = _entry.getOwner();
    owner.detach();
    group = _entry.getGroup();
    group.detach();
}

bool KIODirectoryEntry::mayRead( const char *_user )
{
  if ( _user == 0L )
    _user = "anonymous";
  
  if ( strcmp( _user, owner.data() ) == 0 && access[1] == 'r' )
    return true;
  if ( strcmp( _user, group.data() ) == 0 && access[4] == 'r' )
    return true;
  if ( access[7] == 'r' )
    return true;

  return false;
}

bool KIODirectoryEntry::mayWrite( const char *_user )
{
  if ( _user == 0L )
    _user = "anonymous";
  
  if ( strcmp( _user, owner.data() ) == 0 && access[2] == 'w' )
    return true;
  if ( strcmp( _user, group.data() ) == 0 && access[5] == 'w' )
    return true;
  if ( access[8] == 'w' )
    return true;

  return false;
}

bool KIODirectoryEntry::mayExec( const char *_user )
{
  if ( _user == 0L )
    _user = "anonymous";
  
  if ( strcmp( _user, owner.data() ) == 0 && access[3] == 'x' )
    return true;
  if ( strcmp( _user, group.data() ) == 0 && access[6] == 'x' )
    return true;
  if ( access[9] == 'x' || access[9] == 't' )
    return true;

  return false;
}

KIOServer::KIOServer() : KIOSlaveIPCServer()
{
    pKIOServer = this;
    
    freeSlaves.setAutoDelete( false );
    waitingJobs.setAutoDelete( false );

    connect( &m_timer, SIGNAL( timeout() ), this, SLOT( slotTimer() ) );
    m_timer.start( 10000 );
    
    connect( this, SIGNAL( newClient( KIOSlaveIPC* ) ), this, SLOT( newSlave( KIOSlaveIPC* ) ) );
}

KIOServer::~KIOServer()
{
    KIOSlaveIPC *s;
    for ( s = freeSlaves.first(); s != 0L; s = freeSlaves.next() )
    {
	pid_t p = (pid_t)s->pid;
    
	delete s;
        if (p<=0) printf("kioslave pid is =0, I don't want to hurt myself\n");
        else{
	  kill( p, SIGTERM );
	}  
    }
}

void KIOServer::slotDirEntry( const char *_url, const char *_name, bool _isDir, int _size,
			  const char * _creationDate, const char * _access,
			  const char * _owner, const char *_group )
{
    // debug("KIOServer::slotDirEntry(%s, %s, %d)", _url, _name, _isDir);
    QString url = _url;

    KURL u( _url );

    // Dont cache the local file system
    if ( u.isLocalFile() )
	return;

    // Delete the password if it is in the URL    
    if ( u.passwd() != 0L && u.passwd()[0] != 0 )
    {
        u.setPassword( "" );
	url = u.url();
    }
    
    QList<KIODirectoryEntry> *dir = dirList[ url.data() ];
    if ( dir == 0L )
    {
	dir = new QList<KIODirectoryEntry>;
	dir->setAutoDelete( true );
	dirList.insert( url.data(), dir );
	// debug("Inserted in dirList : url.data() = %s",url.data());
    } else
      dir->setAutoDelete( true );
    
    KIODirectoryEntry *e = new KIODirectoryEntry( _name, _isDir, _size, _creationDate, _access, _owner, _group );
    dir->append( e );
}

KIODirectory* KIOServer::getDirectory( const char *_url )
{
    KURL u( _url );
    
    QString url = _url;
    // Add a trailing '/'
    if ( url.right(1) != "/" && u.hasPath() )
      url += "/";
    return dirList[ url.data() ];
}

KIODirectoryEntry* KIOServer::getDirectoryEntry( const char *_url )
{
    QString url = _url;
    // Delete a trailing '/'
    url.detach();
    if ( url.right(1) == "/" )
      url.truncate( url.length() - 1 );
 
    KURL u( url );
    if ( u.isMalformed() )
      return 0L;
   
    QString name = u.filename();
    QString name2 = u.filename();    
    name2 += "/";
    
    KIODirectory *d = getDirectory( u.directoryURL( false ) );
    if ( d == 0L )
      return 0L;
    
    KIODirectoryEntry *e;
    for ( e = d->first(); e != 0L; e = d->next() )
        if ( name == e->getName() || name2 == e->getName() )
	  return e;
    
    return 0L;
}

void KIOServer::slotFlushDir( const char *_url )
{
    KURL u( _url );
    
    QString url = _url;
    // Add a trailing '/'
    if ( url.right(1) != "/" && u.hasPath() )
      url += "/";

    QList<KIODirectoryEntry> *dir = dirList[ url ];
    if ( dir == 0L )
	return;
    
    dir->clear();
}

/* -------------------------------------------------------------------
 * Static Functions
 * -----------------------------------------------------------------*/

QString KIOServer::getDestNameForCopy( const char *_url )
{
    KURL kurl( _url );

    QString u( _url );
    if ( u.right(2) == ":/" )
    {
	QString res( kurl.protocol() );
	if ( kurl.host() != 0L && kurl.host()[0] != 0 )
	{
	    res += ":";
	    res += kurl.host();
	}
	return res;
    }
    
    if ( u.right(1) == "/" )
	u.truncate( u.length() -1 );
    return QString( kurl.filename() );
}

QString KIOServer::getDestNameForLink( const char *_url )
{
  //debug("KIOServer::getDestNameForLink(%s)",_url);
    QString name;

    KURL kurl( _url );
    
    // Delete a trailing '/'
    if ( kurl.path()[ strlen( kurl.path() ) - 1 ] == '/' )
    {
	QString s = kurl.url();
	s = s.left( s.length() - 1 );
	kurl = s.data();
    }
    
    if ( kurl.isLocalFile() )
    {
	name = kurl.filename();
    }
    else
    {
        name = KBookmark::encode(_url); // use the bookmark way of encoding
    }

    // The old code resulted in file like ftp:filename
    // instead of ftp://host/path/filename
    // Used bookmark code instead. David. (post 1.1)
#if 0
 QString path = kurl.path(); // initial path
 if ( strcmp( kurl.protocol(), "http" ) == 0 )
    {
	name = "http:";
	name += kurl.filename();

	// HTTP names may end with '/', so append 'index.html' in this case.
	if ( path.isEmpty() )
	    name += "index.html";
	else if ( path.right(1) == "/" )
	    name += "index.html";

	name += ".kdelnk";
    }
    else if ( strcmp( kurl.protocol(), "ftp" ) == 0 )
    {
	name = "ftp:";

	// FTP urls may have no path, so append the host as name
	if ( path.isEmpty() )
	    name += kurl.host();
        name += kurl.filename();
	name += ".kdelnk";
    }	
    else
    {
	name = kurl.protocol();
	name.detach();
	name += ":";

	QString tmp = kurl.path();
	tmp.detach();
	int i = 0;
	while ( ( i = tmp.find( "/", i ) ) != -1 )
	    tmp.replace( i++, 1, "%2F" );
		
	name += tmp.data();
    }
#endif

    name.detach();
    return name;
}

bool KIOServer::supports( const char *_url, int _mode )
{
    KURL u2( _url );
    if ( u2.isMalformed() )
	return false;
    // We are interested in the right most protocol.
    // HACK
    // We dont check wether the parent protocols support the needed
    // read/write actions.
    KURL u( u2.nestedURL() );
    if ( u.isMalformed() )
	return false;
    
    if ( strcmp( u.protocol(), "http" ) == 0 )
    {
	if ( _mode == KIO_Read )
	    return true;
	return false;
    }
    else if ( strcmp( u.protocol(), "ftp" ) == 0 )
    {
        if ( ( ( KIO_Delete | KIO_Read | KIO_Write | KIO_MakeDir | KIO_List ) & _mode ) == _mode )
	    return true;
	return false;
    }
    else if ( strcmp( u.protocol(), "file" ) == 0 )
    {
        if ( ( ( KIO_Delete | KIO_Read | KIO_Write | KIO_MakeDir | KIO_Link | KIO_List | KIO_Move ) & _mode ) == _mode )
	    return true;
	return false;
    }
    else if ( strcmp( u.protocol(), "tar" ) == 0 )
    {
        if ( ( ( KIO_Delete | KIO_Read | KIO_List ) & _mode ) == _mode )
	    return true;
	return false;
    }
    else if ( strcmp( u.protocol(), "gzip" ) == 0 )
    {
        if ( ( ( KIO_Read ) & _mode ) == _mode )
	    return true;
	return false;
    }

    return false;
}

bool KIOServer::supports( QStrList & _urls, int _mode )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
	if ( !supports( s, _mode ) )
	    return false;
    
    return true;
}

void KIOServer::sendNotify( const char *_url )
{
    pKIOServer->sendNotify2( _url );
}

void KIOServer::sendNotify2( const char *_url )
{
    emit notify( _url );
}

void KIOServer::sendMountNotify()
{
    pKIOServer->sendMountNotify2();
}

void KIOServer::sendMountNotify2()
{
    emit mountNotify();
}

QString KIOServer::findDeviceMountPoint( const char *_device, const char *_file )
{
#ifdef __FreeBSD__
    if( !strcmp( "/etc/mtab", _file))
    {
	struct statfs *buf;
	long fsno;
	int flags = MNT_WAIT;
	
	fsno = getfsstat( NULL, 0, flags );
	buf = (struct statfs *)malloc(fsno * sizeof( struct statfs ) );
	if( getfsstat(buf, fsno*sizeof( struct statfs ), flags) == -1 )
	{
	    free(buf);
	    return QString(NULL);
	}
	else
	{
	    int i;
	    for( i = 0; i < fsno; i++ )
		if( !strcmp(buf[i].f_mntfromname, _device ) )
		{
		    QString tmpstr((const char *)buf[i].f_mntonname);
		    free(buf);
		    return tmpstr;
		}
	}
    }
    
#endif /* __FreeBSD__ */             

#ifdef HAVE_VOLMGT
    char *volpath;
    char *devname;
    FILE *mnttab;
    struct mnttab mnt;
    int res;
    int len;

    /*
     * get volume manager root path (usually /vol)
     */
    if( (volpath = volmgt_root()) == NULL )
	return QString();

    if( (mnttab = fopen( MNTTAB, "r" )) == NULL )
	return QString();

    if( (devname = malloc( strlen( volpath ) + strlen( _device ) + 2)) == NULL )
	return QString();

    sprintf( devname, "%s%s/", volpath, _device );
    len = strlen( devname );

    /*
     * maybe there's a getmntent() available on other platforms?
     */
    rewind( mnttab );
    while( (res = getmntent( mnttab, &mnt )) != EOF ) {
	if( res < 0 )
		return QString();
        /*
	 * either match the exact device name (floppies),
	 * or a substring (e.g. CD-ROM: /dev/dsk/c0t6d0s2 is mounted
	 * as <volpath>/dev/dsk/c0t6d0/<volume name>
	 */
	if( strncmp( devname, mnt.mnt_special, len ) == 0 
		|| (strncmp( devname, mnt.mnt_special, len - 3 ) == 0
			&& mnt.mnt_special[len - 3] == '/' )) {
		res = 0;
		break;
	}
    }

    fclose( mnttab );
    free( devname );

/*
 * if( res == 0 )
 * 	warning( "Found \"%s\"", mnt.mnt_mountp );
 * else
 * 	warning( "Nothing found" );
 */

    if( res == 0 )
	    return QString( mnt.mnt_mountp );
    else
	    return QString();

#else
    // Get the real device name, not some link.
    char buffer[1024];
    QString tmp;
    
    struct stat lbuff;
    lstat( _device, &lbuff );

    // Perhaps '_device' is just a link ?
    const char *device2 = _device;
    
    if ( S_ISLNK( lbuff.st_mode ) )
    {
	int n = readlink( _device, buffer, 1022 );
	if ( n > 0 )
	{
	    buffer[ n ] = 0;
	    if ( buffer[0] == '/' )
		device2 = buffer;
	    else
	    {
		tmp = "/dev/";
		tmp += buffer;
		device2 = tmp.data();
	    }
	}
    }
    
    debugT("Now Finding mount point of %s\n",_device );

    int len = strlen( _device );
    int len2 = strlen( device2 );
    
    
    FILE *f;
    f = fopen( _file, "rb" );
    if ( f != 0L )
    {
	char buff[ 1024 ];

	while ( !feof( f ) )
	{
	    buff[ 0 ] = 0;
	    // Read a line
	    fgets( buff, 1023, f );
	    // Is it the device we are searching for ?
	    if ( strncmp( buff, _device, len ) == 0 && ( buff[len] == ' ' || buff[len] == '\t' ) )
            {
	      // Skip all spaces
	      while( buff[ len ] == ' ' || buff[ len ] == '\t' )
			len++;
		    
	      char *p = strchr( buff + len, ' ' );
	      if ( p != 0L )
              {
		*p = 0;
		fclose( f );
		return QString( buff + len );
	      }
	    }
	    else if ( strncmp( buff, device2, len2 ) == 0 && ( buff[len2] == ' ' || buff[len2] == '\t' ) )
            {
	      // Skip all spaces
	      while( buff[ len2 ] == ' ' || buff[ len2 ] == '\t' )
			len2++;
		    
	      char *p = strchr( buff + len2, ' ' );
	      if ( p != 0L )
              {
		*p = 0;
		fclose( f );
		return QString( buff + len2 );
	      }	      
	    }
	}
	
	fclose( f );
    }

    return QString();
#endif // HAVE_VOLMGT
}

QString KIOServer::shellQuote( const char *_data )
{
    QString cmd = _data;
   
    int pos = 0;
    while ( ( pos = cmd.find( ";", pos )) != -1 )
    {
	cmd.replace( pos, 1, "\\;" );
	pos += 2;
    }
    pos = 0;
    while ( ( pos = cmd.find( "\"", pos )) != -1 )
    {
	cmd.replace( pos, 1, "\\\"" );
	pos += 2;
    }
    pos = 0;
    while ( ( pos = cmd.find( "|", pos ) ) != -1 )
    {
	cmd.replace( pos, 1, "\\|" );
	pos += 2;
    }
    pos = 0;
    while ( ( pos = cmd.find( "(", pos )) != -1 )
    {
	cmd.replace( pos, 1, "\\(" );
	pos += 2;
    }
    pos = 0;
    while ( ( pos = cmd.find( ")", pos )) != -1 )
    {
	cmd.replace( pos, 1, "\\)" );
	pos += 2;
    }

    return QString( cmd.data() );
}

QString KIOServer::shellUnquote( const char *_data )
{
    QString cmd = _data;
   
    int pos = 0;
    while ( ( pos = cmd.find( "\\;", pos )) != -1 )
    {
	cmd.replace( pos, 2, ";" );
	pos++;
    }
    pos = 0;
    while ( ( pos = cmd.find( "\\\"", pos )) != -1 )
    {
	cmd.replace( pos, 2, "\"" );
	pos++;
    }
    pos = 0;
    while ( ( pos = cmd.find( "\\|", pos ) ) != -1 )
    {
	cmd.replace( pos, 2, "|" );
	pos++;
    }
    pos = 0;
    while ( ( pos = cmd.find( "\\(", pos )) != -1 )
    {
	cmd.replace( pos, 2, "(" );
	pos++;
    }
    pos = 0;
    while ( ( pos = cmd.find( "\\)", pos )) != -1 )
    {
	cmd.replace( pos, 2, ")" );
	pos++;
    }

    return QString( cmd.data() );
}

bool KIOServer::isTrash( QStrList & _urls )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
	if ( !isTrash( s ) )
	    return false;
    
    return true;
}

bool KIOServer::isTrash( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return false;
    
    QString path = u.path();
    if (path.right(1) != "/")
      path += "/";
    
    if ( strcmp( u.protocol(), "file" ) == 0L &&
	 path == KFMPaths::TrashPath() )
	    return true;
    
    return false;
}

int KIOServer::isDir( QStrList & _urls )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	int i = isDir( s );
	if ( i <= 0 )
	    return i;
    }
    
    return 1;
}

int KIOServer::isDir( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return false;

    int i = strlen( _url );

    // A url ending with '/' is always a directory (said somebody)
    // Wrong, says David : file:/tmp/myfile.gz#gzip:/ isn't !
    if ( !u.hasSubProtocol() && i >= 1 && _url[ i - 1 ] == '/' )
	return 1;
    // With HTTP we can be shure that everything that does not end with '/'
    // is NOT a directory
    else if ( strcmp( u.protocol(), "http" ) == 0 )
	return 0;
    // Local filesystem without subprotocol
    else if ( u.isLocalFile() )
	     // strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
    { 
	struct stat buff;
	if ( stat( u.path(), &buff ) != 0 )
	{
	  warning("stat(%s) failed, probably file doesn't exist\n", u.path());
	  return 0;
	}
	
	if ( S_ISDIR( buff.st_mode ) )
	    return 1;
	else
	    return 0;
    }
    else
    {
        // Do we have cached information about such a directory ?
        KIODirectoryEntry * de = pKIOServer->getDirectoryEntry(_url);
        if (de && de->isDir())
	  return 1;
        // If isDir is false, then we know nothing. It might be a link on a 
        // ftp site, pointing to another dir, so let's not assume it's a file

	// We are not sure
	return -1;
    }
}

/* -------------------------------------------------------------------
 * The multi tasking kernel
 * -----------------------------------------------------------------*/

void KIOServer::newSlave( KIOSlaveIPC * _slave )
{
    connect( _slave, SIGNAL( dirEntry( const char *, const char *, bool, int, const char *,
				      const char *, const char *, const char * ) ), 
	     this, SLOT( slotDirEntry( const char *, const char *, bool, int, const char *,
					 const char *, const char *, const char * ) ) );
    connect( _slave, SIGNAL( flushDir( const char* ) ), this, SLOT( slotFlushDir( const char * ) ) );
    connect( _slave, SIGNAL( closed( KIOSlaveIPC* ) ), this, SLOT( slotSlaveClosed( KIOSlaveIPC* ) ) );
    
    newSlave2( _slave );
}

void KIOServer::newSlave2( KIOSlaveIPC * _slave )
{
    if ( waitingJobs.count() == 0 )
    {
	freeSlaves.append( _slave );
	_slave->m_time = time( 0L );
	return;
    }
    
    KIOJob* job = waitingJobs.first();
    waitingJobs.removeRef( job );
    job->doIt( _slave );
}

void KIOServer::getSlave( KIOJob *_job )
{
    if ( freeSlaves.count() == 0 )
    {
	waitingJobs.append( _job );
	runNewSlave();
	return;
    }
    
    KIOSlaveIPC *slave = freeSlaves.first();
    freeSlaves.removeRef( slave );
    _job->doIt( slave );
}

void KIOServer::runNewSlave()
{
    QString ipath = kapp->kde_bindir() + "/kioslave";

    // Keep in sync with the same in kioserver_ipc.cpp!
    QString idir;
    idir.sprintf(_PATH_TMP"/kio_%i_%i%s",(int)getuid(), (int)getpid(),displayName().data());
    
    if ( fork() == 0 )
    {
        execlp( ipath.data(), "kioslave", idir.data(), 0 );
	fatal( "ERROR: Could not start kioslave\n");
        exit( 1 );
    }
}

void KIOServer::freeSlave( KIOSlaveIPC *_slave )
{
    newSlave2( _slave );
}

void KIOServer::slotSlaveClosed( KIOSlaveIPC* _slave )
{
    freeSlaves.removeRef( _slave );
}

bool KIOServer::testDirInclusion( const char *_src_url, const char *_dest_canonical )
{
    KURL u( _src_url );
    if ( u.isMalformed() )
	return false;

    KURL u2( _dest_canonical );
    if ( u2.isMalformed() )
	return false;
    
    // Any URL that is not a file on the local hard disk ?
    if ( strcmp( u.protocol(), "file" ) != 0L || strcmp( u2.protocol(), "file" ) != 0L ||
	 u.hasSubProtocol() || u2.hasSubProtocol() )
    {
	// Inclusion ?
	if ( strncmp( _src_url, _dest_canonical, strlen( _src_url ) ) == 0 )
	    return false;

	return true;
    }
    
    QString url;
    // replace all symlinks if we are on the local hard disk
    // Get the canonical path.
    QDir dir( u.path() );
    url = dir.canonicalPath();
    if ( url.isEmpty() )
	url = u.path();

    if ( strncmp( url.data(), _dest_canonical, url.length() ) == 0 )
	return false;
    
    return true;
}

QString KIOServer::canonicalURL( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() || strcmp( u.protocol(), "file" ) != 0L || u.hasSubProtocol() )
	return QString( _url );
    
    QDir dir( u.path() );
    QString url = dir.canonicalPath();
    if ( url.isEmpty() )
	return QString( _url );
    
    return u.path();
}

void KIOServer::slotTimer()
{
  /*  if ( KIOJob::jobList->count() > 0 )
    printf("====================== ATTENTION: KIOJob memory lack %i ===============\n", KIOJob::jobList->count()); */
    
  QList<KIOSlaveIPC> list = freeSlaves;
 
  time_t t = time( 0L );
  
  KIOSlaveIPC *p;
  for ( p = list.first(); p != 0L; p = list.next() )
  {
    if ( t - p->m_time > 10 )
    {
      pid_t pid = (pid_t)p->pid;    
      freeSlaves.removeRef( p );
      delete p;
      if ( pid <= 0 )
	printf("kioslave pid is =0, I don't want to hurt myself\n");
      else
      {
        kill( pid, SIGTERM );
        int status;
        waitpid( pid, &status, 0 );
      }	
    }
  }
}

#include "kioserver.moc"

