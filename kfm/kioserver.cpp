#include <qstrlist.h>
#include <qdict.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <Kconfig.h>

#include "kioserver_ipc.h"
#include "kioserver.h"
#include "kbind.h"

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
    access = _entry.getAccess();
    access.detach();
    owner = _entry.getOwner();
    owner.detach();
    group = _entry.getGroup();
    group.detach();
}

KIOServer::KIOServer() : KIOSlaveIPCServer()
{
    pKIOServer = this;
    
    freeSlaves.setAutoDelete( FALSE );
    waitingJobs.setAutoDelete( FALSE );

    connect( this, SIGNAL( newClient( KIOSlaveIPC* ) ), this, SLOT( newSlave( KIOSlaveIPC* ) ) );
}

KIOServer::~KIOServer()
{
    KIOSlaveIPC *s;
    for ( s = freeSlaves.first(); s != 0L; s = freeSlaves.next() )
    {
	pid_t p = (pid_t)s->pid;
    
	delete s;
	kill( p, SIGTERM );
    }
}

void KIOServer::slotDirEntry( const char *_url, const char *_name, bool _isDir, int _size,
			  const char * _creationDate, const char * _access,
			  const char * _owner, const char *_group )
{
    QList<KIODirectoryEntry> *dir = dirList[ _url ];
    if ( dir == 0L )
    {
	dir = new QList<KIODirectoryEntry>;
	dir->setAutoDelete( TRUE );
	dirList.insert( _url, dir );
    }
    
    KIODirectoryEntry *e = new KIODirectoryEntry( _name, _isDir, _size, _creationDate, _access, _owner, _group );
    dir->append( e );
}

KIODirectory* KIOServer::getDirectory( const char *_url )
{
    return dirList[ _url ];
}

void KIOServer::slotFlushDir( const char *_url )
{
    QList<KIODirectoryEntry> *dir = dirList[ _url ];
    if ( dir == 0L )
	return;
    
    dir->clear();
}

/* -------------------------------------------------------------------
 * Static Functions
 * -----------------------------------------------------------------*/

QString KIOServer::getDestNameForLink( const char *_url )
{
    QString name;

    KURL kurl( _url );
    
    // Delete a trailing '/'
    if ( kurl.path()[ strlen( kurl.path() ) - 1 ] == '/' )
    {
	QString s = kurl.url();
	s = s.left( s.length() - 1 );
	kurl = s.data();
    }
    
    if ( strcmp( kurl.protocol(), "file" ) == 0 )
    {
	name = kurl.filename();
	name.detach();
    }
    else if ( strcmp( kurl.protocol(), "tar" ) == 0 )
    {		
	name = "tar:";
	name += kurl.filename( TRUE );
	name += ".kdelnk";
	name.detach();
    }
    else if ( strcmp( kurl.protocol(), "http" ) == 0 )
    {
	name = "http:";
	name += kurl.filename();

	// HTTP names may end with '/', so append 'index.html' in this case.
	if ( strlen( kurl.path() ) == 0 )
	    name += "index.html";
	else if ( kurl.path()[ strlen( kurl.path() ) - 1 ] == '/' )
	    name += "index.html";

	name += ".kdelnk";
	name.detach();
    }
    else if ( strcmp( kurl.protocol(), "ftp" ) == 0 )
    {
	name = "ftp:";

	// FTP urls may have no path, so append the host as name
	if ( strlen( kurl.path() ) == 0 )
	    name += kurl.host();
	// FTP urls may have no a trailing '/'.
	else if ( kurl.path()[ strlen( kurl.path() ) - 1 ] == '/' )
	{
	    QString s = kurl.url();
	    s = s.left( s.length() - 1 );
	    KURL u( s.data() );
	    name += u.filename();
	}
	else
	    name += kurl.filename();
	name += ".kdelnk";
	name.detach();
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
	    tmp.replace( i++, 1, "_" );
		
	name += tmp.data();
    }

    return name;
}

bool KIOServer::supports( const char *_url, int _mode )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return FALSE;
    
    if ( strcmp( u.protocol(), "http" ) == 0 )
    {
	if ( _mode == KIO_Read )
	    return TRUE;
	return FALSE;
    }
    else if ( strcmp( u.protocol(), "ftp" ) == 0 )
    {
        if ( ( ( KIO_Delete | KIO_Read | KIO_Write | KIO_MakeDir ) & _mode ) == _mode )
	    return TRUE;
	return FALSE;
    }
    else if ( strcmp( u.protocol(), "file" ) == 0 )
    {
        if ( ( ( KIO_Delete | KIO_Read | KIO_Write | KIO_MakeDir | KIO_Link ) & _mode ) == _mode )
	    return TRUE;
	return FALSE;
    }
    else if ( strcmp( u.protocol(), "tar" ) == 0 )
    {
        if ( ( ( KIO_Delete | KIO_Read ) & _mode ) == _mode )
	    return TRUE;
	return FALSE;
    }

    return FALSE;
}

bool KIOServer::supports( QStrList & _urls, int _mode )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
	if ( !supports( s, _mode ) )
	    return FALSE;
    
    return TRUE;
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
    int len = strlen( _device );
    
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
	    if ( strncmp( buff, _device, len ) == 0 )
		// Is a space following ?
		if ( buff[ len ] == ' ' )
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
	}
	
	fclose( f );
    }

    return QString();
}

bool KIOServer::isDir( QStrList & _urls )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
	if ( !isDir( s ) )
	    return FALSE;
    
    return TRUE;
}

bool KIOServer::isDir( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return FALSE;
    
    if ( strcmp( u.protocol(), "http" ) == 0 )
	return FALSE;
    else if ( strcmp( u.protocol(), "ftp" ) == 0 )
    {
	if ( _url[ strlen( _url ) - 1 ] == '/' )
	    return TRUE;
	else
	    return FALSE;
    }
    else if ( strcmp( u.protocol(), "tar" ) == 0 )
    {
	if ( _url[ strlen( _url ) - 1 ] == '/' )
	    return TRUE;
	else if ( _url[ strlen( _url ) - 1 ] == '#' )
	    return TRUE;
	else
	    return FALSE;
    }
    else if ( strcmp( u.protocol(), "file" ) == 0 )
    {
	struct stat buff;
	stat( u.path(), &buff );

	if ( S_ISDIR( buff.st_mode ) )
	    return TRUE;
	else
	    return FALSE;
    }
    else
	return FALSE;
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
    
    newSlave2( _slave );
}

void KIOServer::newSlave2( KIOSlaveIPC * _slave )
{
    printf("New Slave arrived\n");
    
    if ( waitingJobs.count() == 0 )
    {
	printf("No Job waiting\n");
	freeSlaves.append( _slave );
	return;
    }
    
    printf("Job served with new slave\n");
    KIOJob* job = waitingJobs.first();
    waitingJobs.removeRef( job );
    job->doIt( _slave );
}

void KIOServer::getSlave( KIOJob *_job )
{
    printf("Request for slave\n");
    if ( freeSlaves.count() == 0 )
    {
	printf("No slave avaulable\n");
	waitingJobs.append( _job );
	runNewSlave();
	return;
    }
    
    printf("Job served with waiting slave\n");
    KIOSlaveIPC *slave = freeSlaves.first();
    freeSlaves.removeRef( slave );
    _job->doIt( slave );
}

void KIOServer::runNewSlave()
{
    QString ipath( getenv("KDEDIR") );
    if ( ipath.isNull() )
    {
	printf("ERROR: You did not set $KDEDIR\n");
	exit(2);
    }
    ipath.detach();
    ipath += "/bin/kioslave";

    char buffer[ 1024 ];
    sprintf( buffer, "%i", getPort() );
    if ( fork() == 0 )
    {
        execlp( ipath.data(), "kioslave", buffer, 0 );
        exit( 1 );
    }
}

void KIOServer::freeSlave( KIOSlaveIPC *_slave )
{
    printf("++++++++ Got Slace back\n");
    newSlave2( _slave );
}

#include "kioserver.moc"

