#include <qapp.h>

#include "kioslave_ipc.h"
#include "main.h"
#include "kio_errors.h"
#include "manage.h"
#include <config-kfm.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

/*#include "ftplib.h"*/

#define FTP_LOGIN "anonymous"
#define FTP_PASSWD "joedoe@nowhere.crg"

void sig_handler(int);

KIOSlave *slave = 0L;

int main( int argc, char **argv )
{
    if ( argc != 2 )
    {
	debugT("Usage: kioslave port\n");
	exit(1);
    }

    signal(SIGTERM,sig_handler);
    
    QApplication a( argc, argv );

    slave = new KIOSlave( atoi( argv[1] ) );
    
    a.exec();
}

void KIOSlave::ProcessError(KProtocol *prot, const char *srcurl)
{
    int KError, SysError;
    QString message;

    prot->GetLastError(KError, message, SysError);

    debugT("KIOSlave-ERROR (%s): %s\n",srcurl,message.data());
    ipc->fatalError(KError, srcurl, SysError);
}

KIOSlave::KIOSlave( int _port )
{
    ipc = new KIOSlaveIPC( _port );
    
    if ( !ipc->isConnected() )
    {
	debugT("Could not connect to KIO om port %i\n", _port );
	exit(1);
    }

    copyDestFile = 0L;

    connect( ipc, SIGNAL( copy( const char*, const char*, bool ) ),
	     this, SLOT( copy( const char*, const char*, bool ) ) );
    connect( ipc, SIGNAL( del( const char* ) ), this, SLOT( del( const char* ) ) );
    connect( ipc, SIGNAL( mkdir( const char* ) ), this, SLOT( mkdir( const char* ) ) );
    connect( ipc, SIGNAL( unmount( const char* ) ), this, SLOT( unmount( const char* ) ) );
    connect( ipc, SIGNAL( mount( bool, const char*, const char*, const char* ) ),
	     this, SLOT( mount( bool, const char*, const char*, const char* ) ) );
    connect( ipc, SIGNAL( list( const char* ) ), this, SLOT( list( const char* ) ) );
    connect( ipc, SIGNAL( cleanUp() ), this, SLOT( cleanUp() ) );
    connect( ipc, SIGNAL( getPID() ), this, SLOT( getPID() ) );

    ipc->hello();
}

void KIOSlave::getPID()
{
    ipc->setPID( (int)getpid() );
}

void KIOSlave::mkdir( const char *_url )
{
    KURL su( _url );

    if ( su.isMalformed() )
    {
		debugT("ERROR: Malformed URL '%s'\n",_url );
		ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
		return;
    }

    if(ProtocolSupported(&su))
	{
		KProtocol *prot = CreateProtocol(&su);
		if(prot->MkDir(&su) != KProtocol::SUCCESS)
		{
			ProcessError(prot, _url);
			return;
		}
		delete prot;
	}
    else
    {
		debugT("ERROR: Not implemented\n");
		QString err = "Mkdir ";
		err += _url;
		ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
		return;
    }
    
    ipc->done();
}

void KIOSlave::list( const char *_url )
{
    KURL su( _url );
    if ( su.isMalformed() )
    {
	debugT("ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }
    
    if ( strcmp( su.protocol(), "tar" ) == 0 )
    {
	QString outFile;
	outFile.sprintf( "/tmp/tar%i", time( 0L ) );
	QString logFile;
	logFile.sprintf( "/tmp/tarlog%i", time( 0L ) );
    
	QStrList files;
	
	QString cmd;
	if ( su.path()[ strlen( su.path() ) - 1 ] == 'z' )
	    cmd.sprintf( "tar -tvzf %s 1>%s 2>%s", su.path(), outFile.data(), logFile.data() );
	else
	    cmd.sprintf( "tar -tvf %s 1>%s 2>%s", su.path(), outFile.data(), logFile.data() );

	system( cmd.data() );

	QString err = testLogFile( logFile.data() );
	if ( !err.isNull() )
	{
	    unlink( outFile.data() );
	    unlink( logFile.data() );
	    QString err2;
	    err2.sprintf( "%s\n\nError log:\n%s", _url, err.data() );	    
	    ipc->fatalError( KIO_ERROR_TarError, err2.data(), 0 );
	    return;
	}
	
	// Open the output of the tar command
	QFile f( outFile.data() );

	if ( f.open( IO_ReadOnly ) )
	{
	    char line[1024];

	    while ( !f.atEnd() )
	    {
		if ( f.readLine( line, 1023 ) > 0 )
		{
		    // delete the '\n' at the lines end
		    line[ strlen( line ) - 1 ] = 0;
		    files.append( line );
		}
	    }
	    f.close();
	}
	else
	{
	    debugT("ERROR: Could not read '%s'\n",outFile.data());
	    ipc->fatalError( KIO_ERROR_CouldNotRead, outFile.data(), errno );
	    return;
	}
	
	// Delete the output file
	unlink( outFile.data() );

	// Flush the parent directory
	QString t;
	t.sprintf( "tar:%s#", su.path() );
	ipc->flushDir( t.data() );

	char line[1024];

	// Find all files in all directories
	char *s;
	for ( s = files.first(); s != 0L; s = files.next() )
	{
	    strcpy( line, s );
	    // Parse the line
	    bool err = false;
	    bool isdir = ( line[0] == 'd' );
	    QString owner;
	    QString group;
	    QString access;
	    QString creationDate;
	    QString name;
	    int size;
			    
	    char *p2 = line;
	    char *p = strchr( line, ' ' );
			    
	    if ( p == 0L )
		err = true;
	    else
	    {
		*p = 0;
		access = p2;
		debugT("ACCESS = '%s'\n",access.data());
		p2 = p + 1;
		
		    p = strchr( p2, '/' );
		    if ( p == 0L )
			err = true;
		    else
		    {
			*p = 0;
			owner = p2;
			debugT("OWNER = '%s'\n",owner.data());
			p2 = p + 1;
			p = strchr( p2, ' ' );
			if ( p == 0L )
			    err = true;
			else
			{
			    *p = 0;
			    group = p2;
			    debugT("GROUP = '%s'\n",group.data());
			    p2 = p + 1;
			    while ( *p2 == ' ' ) p2++;
			    p = strchr( p2, ' ' );
			    if ( p == 0L )
				err = true;
			    else
			    {
				*p = 0;
				size = atoi( p2 );
				debugT("SIZE = '%i'\n",size);
				p2 = p + 1;
				p = strchr( p2, ' ' );
				if ( p == 0L )
				    err = true;
				else
				{
				    p = strchr( p + 1, ' ' );
				    if ( p == 0L )
					err = true;
				    else
				    {
					p = strchr( p + 1, ' ' );
					if ( p == 0L )
					    err = true;
					else
					{
					    p = strchr( p + 1, ' ' );
					    if ( p == 0L )
						err = true;
					    else
					    {
						*p = 0;
						creationDate = p2;
						debugT("DATE = '%s'\n",creationDate.data());
						p2 = p + 1;
						name = p2;
						debugT("NAME = '%s'\n",name.data() );
					    }
					}
				    }
				}
			    }   
			}
		    }    

	    }
	    if ( !err )
	    {
		if ( isdir )
		{
		    t.sprintf( "tar:%s#%s", su.path(), name.data() );
		    ipc->flushDir( t.data() );
		    // Delete the trailing '/'
		    name = name.left( name.length() - 1 ).data();
		}
		QString dir = "";
		int i = name.findRev( "/" );
		if ( i != -1 )
		{
		    dir = name.left( i + 1 ).data();
		    name = name.data() + i + 1;
		}
		if ( isdir )
		    name += "/";
		t.sprintf( "tar:%s#%s", su.path(), dir.data() );
		ipc->dirEntry( t.data(), name.data(), isdir, size, creationDate.data(),
			       access.data(), owner.data(), group.data() );
	    }
	}
    }
    else if (ProtocolSupported(&su))
    {
		KProtocolDirEntry *de;
		KProtocol *prot = CreateProtocol(&su);

		prot->OpenDir(&su);
		ipc->flushDir(_url);
		while( (de = prot->ReadDir()) != 0L )
		{
			ipc->dirEntry( _url, de->name.data(), de->isdir, de->size,
							de->date.data(), de->access.data(),
						   de->owner.data(), de->group.data() );
		}
		prot->CloseDir();
    }
    else
    {
	debugT("ERROR: Not implemented\n");
	QString err = "List ";
	err += _url;
	ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
	return;
    }
    
    ipc->done();
}

void KIOSlave::mount( bool _ro, const char *_fstype, const char* _dev, const char *_point )
{
    char buffer[ 1024 ];

    int t = (int)time( 0L );
    
    // Look in /etc/fstab ?
    if ( _fstype == 0L || _point == 0L )
	sprintf( buffer, "mount %s >/tmp/mnt%i\n",_dev, t );
    else if ( _ro )
	sprintf( buffer, "mount -rt %s %s %s >/tmp/mnt%i\n",_fstype, _dev, _point, t );
    else
	sprintf( buffer, "mount -rt %s %s %s >/tmp/mnt%i\n",_fstype, _dev, _point, t );
		
    debugT("EXEC: '%s'\n",buffer);
    
    system( buffer );

    sprintf( buffer, "/tmp/mnt%i", t );

    QString err = testLogFile( buffer );
    if ( err.isNull() )
    {
	ipc->done();
	return;
    }

    ipc->fatalError( KIO_ERROR_CouldNotMount, err.data(), 0 );
    return;
}

void KIOSlave::unmount( const char *_point )
{
    char buffer[ 1024 ];

    int t = (int)time( 0L );
    
    sprintf( buffer, "umount %s 2>/tmp/mnt%i\n",_point, t );
    system( buffer );

    sprintf( buffer, "/tmp/mnt%i", t );

    QString err = testLogFile( buffer );
    if ( err.isNull() )
    {
	ipc->done();
	return;
    }

    ipc->fatalError( KIO_ERROR_CouldNotUnmount, err.data(), 0 );
    return;
}

void KIOSlave::del( const char *_url )
{
    KURL su( _url );
    if ( su.isMalformed() )
	return;
    
    if ( strcmp( su.protocol(), "file" ) == 0 )
    {
	int erg;

	struct stat buff;
	stat( su.path(), &buff );

	struct stat lbuff;
	lstat( su.path(), &buff );

	// If it is a directory and not a link
	if ( S_ISDIR( buff.st_mode ) && !S_ISLNK( lbuff.st_mode ) )
	    erg = rmdir( su.path() );
	else
	    erg = unlink( su.path() );
	
	if ( erg != 0 )
	{
	    debugT("ERROR: Could not delete '%s'\n",_url );
	    ipc->fatalError( KIO_ERROR_CouldNotDelete, _url, errno );
	    return;
	}
    }
    else if ( strcmp( su.protocol(), "tar" ) == 0 )
    {
	QString tar = su.path();
	
	if ( su.path()[ strlen( su.path() ) - 1 ] == 'z' )
	{
	    if ( !lockTgz( su.path() ) )
		return;
	    lockedTgzModified = true;
	    tar = lockedTgzTemp.data();
	}

	QString logFile;
	logFile.sprintf( "/tmp/tarlog%i", time( 0L ) );

	QString cmd;
	cmd.sprintf( "tar --delete %s -f %s 2>%s", su.reference(), tar.data(), logFile.data() );
	system( cmd.data() );

	QString err = testLogFile( logFile.data() );
	if ( !err.isNull() )
	{
	    // Delete the lock
	    lockedTgzModified = "";
	    unlink( tar.data() );
	    // Delete the log file
	    unlink( logFile.data() );
	    // Print an error message
	    QString err2;
	    err2.sprintf( "%s\n\nError log:\n%s", _url, err.data() );	    
	    ipc->fatalError( KIO_ERROR_TarError, err2.data(), 0 );
	    return;
	}
    }
    else if ( strcmp( su.protocol(), "ftp" ) == 0 )
    {
#ifdef TODO
	// TODO: Port not implemented!!!
	// if ( !lockFTP( su.host(), su.port(), FTP_LOGIN, FTP_PASSWD ) )
        QString user = su.user();
        QString passwd = su.passwd();
	if ( user.length() <= 0 )
	  user = FTP_LOGIN;
	if ( passwd.length() <= 0 )
	  passwd = FTP_PASSWD;
	user.detach();
	passwd.detach();

	if ( !lockFTP( su.host(), 21, user.data(), passwd.data() ) )
	    return;
	
	int erg;
	
	if ( su.path()[ strlen( su.path() ) - 1 ] == '/' )
	    erg = ftpRmdir( su.path() );
	else
	    erg = ftpDelete( su.path() );

	if ( !erg )
	{
	    debugT("ERROR: Could not delete '%s'\n",_url );
	    ipc->fatalError( KIO_ERROR_CouldNotDelete, _url, errno );
	    return;
	}
#endif
    }
    else
    {
	debugT("ERROR: Not implemented\n");
	QString err = "Delete ";
	err += _url;
	ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
	return;
    }
    
    ipc->done();
}


void KIOSlave::copy( const char *_src_url, const char *_dest_url, bool _overwriteExistingFiles )
{
    debugT("******** COPY '%s' to '%s\n",_src_url,_dest_url);
    
    KURL su( _src_url );
    if ( su.isMalformed() )
    {
	debugT("ERROR: Malformed URL '%s'\n",_src_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _src_url, 0 );
	return;
    }
    
    KURL du( _dest_url );
    if ( du.isMalformed() )
    {
	debugT("ERROR: Malformed URL '%s'\n",_dest_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _dest_url, 0 );
	return;
    }

    if( ProtocolSupported(&su) && ProtocolSupported(&du) )
    {
	KProtocol *src_prot = CreateProtocol(&su);
	KProtocol *dest_prot = CreateProtocol(&du);
	
	int destmode = KProtocol::WRITE;
	if( _overwriteExistingFiles ) destmode |= KProtocol::OVERWRITE;
	
	if( dest_prot->Open(&du, destmode) != KProtocol::SUCCESS )
	{
	    ProcessError( dest_prot, _dest_url );
	    return;
	}
	
	debugT("Open src\n");
	if( src_prot->Open(&su, KProtocol::READ) != KProtocol::SUCCESS )
	{
	    ProcessError(src_prot, _src_url);
	    return;
	}

/************************************** has to be adapted to match protocols *
	copyDestFile = out;
	copyDestFileName = du.path();
	copyDestFileName.detach();

	[...]

	copyDestFile = 0L;
******************************************************************************/

	int c = 0;
	int last = 0;
	int l = 1;
	long size = src_prot->Size();
	char buffer[4096];
	
	debugT("Copyloop starting\n");
	while ( !src_prot->atEOF() )
	{
	    debugT("read\n");
	    if ( ( l = src_prot->Read( buffer, 4096 ) ) < 0 )
	    {
		debugT("read error (%ld)\n",l);
		ProcessError(src_prot, _src_url);
		return;
	    }
	    debugT("write\n");
	    if ( dest_prot->Write(buffer, l) < l )
	    {
		ProcessError(dest_prot, _dest_url);
		return;
	    }
	    debugT("progress\n");
	    c += l;
	    if ( size == 0 )
		ipc->progress( 100 );
	    else if ( ( c * 100 / size ) != last )
	    {
		last = ( c * 100 / size );
		debugT("percent %i\n", last );
		ipc->progress( last );
	    }
	}
	debugT("ready: Close\n"); 
	src_prot->Close();
	dest_prot->Close();

	int permissions = src_prot->GetPermissions( su );
	debugT("Got Permissions '%i'\n",permissions);
	dest_prot->SetPermissions( du, permissions );
	
	debugT("delete\n");
	delete src_prot;
	delete dest_prot;
	
	debugT("******** COPY: Job completed using ProtocolManagement!\n");
	ipc->done();
	return;
    }

    if( strcmp( su.protocol(), "tar" ) == 0 && strcmp( du.protocol(), "file" ) == 0 )
    {
	struct stat buff;
	if ( stat( du.path(), &buff ) == 0 && !_overwriteExistingFiles )
	{
	    ipc->fatalError( KIO_ERROR_FileExists, _dest_url, errno );	
	    return;
	}
	
	QString tar = su.path();
		
	QString logFile;
	logFile.sprintf( "/tmp/tarlog%i", time( 0L ) );

	QString cmd;
	if ( su.path()[ strlen( su.path() ) - 1 ] == 'z' )
	    cmd.sprintf( "tar -xzOf %s %s 1> %s 2> %s ", su.path(), su.reference(), du.path(), logFile.data() );
	else
	    cmd.sprintf( "tar -xOf %s %s 1> %s 2> %s ", su.path(), su.reference(), du.path(), logFile.data() );
	system( cmd.data() );
	
	QString err = testLogFile( logFile.data() );
	if ( !err.isNull() )
	{
	    unlink( du.path() );
	    unlink( logFile.data() );
	    QString err2;
	    err2.sprintf( "%s\n\nError log:\n%s", _src_url, err.data() );	    
	    ipc->fatalError( KIO_ERROR_TarError, err2.data(), 0 );
	    return;
	}
    }
    else
    {
		debugT("ERROR: Not implemented\n");
		QString err = "Copy ";
		err += _src_url;
		err += " to ";
		err += _dest_url;
		ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
		return;
    }

    ipc->done();
}

bool KIOSlave::lockFTP( const char *_host, int _port, const char* , const char *_passwd )
{
#ifdef TODO
    if ( strcmp( _host, lockedFTPHost.data() ) == 0 && _port == lockedFTPPort &&
	 strcmp( _login, lockedFTPLogin.data() ) == 0 && strcmp( _passwd, lockedFTPPasswd.data() ) == 0 )
	return true;
    
    unlockFTP();
    
    lockedFTPHost = _host;
    lockedFTPHost.detach();
    lockedFTPPort = _port;
    lockedFTPLogin = _login;
    lockedFTPLogin.detach();
    lockedFTPPasswd = _passwd;
    lockedFTPPasswd.detach();

    if ( !ftpOpen( _host ) )
    {
	debugT("ERROR: Could not connect\n");
	ipc->fatalError( KIO_ERROR_CouldNotConnect, _host, 0 );
	return false;
    }
    
    debugT("Login\n");
	
    if ( !ftpLogin( lockedFTPLogin.data(), lockedFTPPasswd.data() ) )
    {
	debugT("ERROR: Could not login\n");
	QString err;
	err.sprintf( "ftp://%s:%s@%s", _login, _passwd, _host );
	ipc->fatalError( KIO_ERROR_CouldNotLogin, err.data(), 0 );
	return false;
    }

    return true;
#else
//Stephan: Only to avoid warnings
_host = 0L;
_port = 0;
_passwd = 0L;
return true;
#endif
}

bool KIOSlave::unlockFTP()
{
#ifdef TODO
    if ( lockedFTPHost.data() != 0L )
	if ( lockedFTPHost.data()[0] != 0 )
	{
	    ftpQuit();
	    lockedFTPHost = "";
	}

    return true;
#else //Stephan: to avoid warning
   return true;
#endif
}

bool KIOSlave::lockTgz( const char *_name )
{
    if ( strcmp( _name, lockedTgzSource.data() ) == 0 )
	return true;
    
    if ( !unlockTgz() )
	return false;

    QString logFile;
    logFile.sprintf( "/tmp/gziplog%i", time( 0L ) );
	    
    QString cmd;
    lockedTgzSource = _name;
    lockedTgzSource.detach();
    lockedTgzModified = false;
    lockedTgzTemp.sprintf( "tmp%i.tar", time( 0L ) );
    cmd.sprintf("gzip -d <%s 1>%s 2>%S", _name, lockedTgzTemp.data(), logFile.data() );
    system( cmd );

    QString err = testLogFile( logFile.data() );
    if ( err.isNull() )
	return true;

    unlink( lockedTgzTemp.data() );
    unlink( logFile.data() );
    lockedTgzSource = "";
    QString err2;
    err2.sprintf( "%s\n\nError log:\n%s", _name, err.data() );	    
    ipc->fatalError( KIO_ERROR_GzipError, err2.data(), 0 );

    return false;
}

bool KIOSlave::unlockTgz()
{
    if ( lockedTgzSource.data() == 0L )
	return true;
    if ( lockedTgzSource.data()[0] == 0 )
	return true;
    
    if ( lockedTgzModified )
    {
	QString logFile;
	logFile.sprintf( "/tmp/gziplog%i", time( 0L ) );

	QString cmd;
	cmd.sprintf( "gzip <%s 1>%s 2>%s", lockedTgzTemp.data(), lockedTgzSource.data(), logFile.data() );
	system( cmd.data() );

	QString err = testLogFile( logFile.data() );
	if ( !err.isNull() )
	{
	    unlink( logFile.data() );
	    QString err2;
	    err2.sprintf( "%s\n\nError log:\n%s", lockedTgzTemp.data(), err.data() );	    
	    ipc->fatalError( KIO_ERROR_TarError, err2.data(), 0 );
	    lockedTgzSource = "";
	    return false;
	    
	}
    }
    
    unlink( lockedTgzTemp );

    lockedTgzSource = "";

    return true;
}

void KIOSlave::cleanUp()
{
    bool ok1 = true;
    bool ok2 = true;

    ok1 = unlockTgz();
    ok2 = unlockFTP();
    
    if ( !ok1 || !ok2 )
	return;
    
    ipc->done();
}

void KIOSlave::terminate()
{
    if ( copyDestFile != 0L )
    {
	fclose( copyDestFile);
	unlink( copyDestFileName.data() );
    }
}

QString KIOSlave::testLogFile( const char *_filename )
{
    char buffer[ 1024 ];
    struct stat buff;

    stat( _filename, &buff );
    int size = buff.st_size;
    if ( size == 0 )
	return QString();
    
    QString err = "";
    
    FILE * f = fopen( _filename, "rb" );
    if ( f == 0L )
    {
	unlink( _filename );
	err.sprintf("Could not read '%s'", _filename );
	return QString( err.data() );
    }
    
    char *p = "";
    while ( p != 0L )
    {
	p = fgets( buffer, 1023, f );
	if ( p != 0L )
	    err += buffer;
    }

    fclose( f );
    
    unlink( _filename );

    return QString( err.data() );
}

void sig_handler(int _sig)
{
    debugT("GOT TERM\n");
    if ( _sig == SIGTERM )
	if ( slave != 0L )
	    slave->terminate();
    exit(0);
}

#include "main.moc"










