#include <qapp.h>

#include "kioslave_ipc.h"
#include "main.h"
#include "kio_errors.h"
#include "manage.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define FTP_LOGIN "anonymous"
#define FTP_PASSWD "joedoe@nowhere.crg"

void sig_handler(int);
void sig_handler2(int);

KIOSlave *slave = 0L;

int main(int argc, char **argv)
{
    if ( argc != 2 )
    {
	printf("Usage: kioslave port\n");
	exit(1);
    }

    signal(SIGTERM,sig_handler);
    signal(SIGCHLD,sig_handler2);
    
    QApplication a( argc, argv );

    slave = new KIOSlave( atoi( argv[1] ) );
    
    a.exec();
	return(0);
}

void KIOSlave::ProcessError(KProtocol *prot, const char *srcurl)
{
    int KError, SysError;
    QString message;

    prot->GetLastError(KError, message, SysError);

    printf("KIOSlave-ERROR (%s): %s\n",srcurl,message.data());
    ipc->fatalError(KError, srcurl, SysError);
}

KIOSlave::KIOSlave( int _port )
{
    ipc = new KIOSlaveIPC( _port );
    
    if ( !ipc->isConnected() )
    {
	printf("Could not connect to KIO om port %i\n", _port );
	exit(1);
    }

    copyDestFile = 0L;

    connect( ipc, SIGNAL( copy( const char*, const char*, bool ) ),
	     this, SLOT( copy( const char*, const char*, bool ) ) );
    connect( ipc, SIGNAL( get( const char* ) ),
	     this, SLOT( get( const char* ) ) );
    connect( ipc, SIGNAL( del( const char* ) ), this, SLOT( del( const char* ) ) );
    connect( ipc, SIGNAL( mkdir( const char* ) ), this, SLOT( mkdir( const char* ) ) );
    connect( ipc, SIGNAL( unmount( const char* ) ), this, SLOT( unmount( const char* ) ) );
    connect( ipc, SIGNAL( mount( bool, const char*, const char*, const char* ) ),
	     this, SLOT( mount( bool, const char*, const char*, const char* ) ) );
    connect( ipc, SIGNAL( list( const char*, bool ) ), this, SLOT( list( const char*, bool ) ) );
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
    KURL u( _url );
    if ( u.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    KURL su( u.nestedURL() );
    if ( su.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    if(ProtocolSupported(_url))
    {
	KProtocol *prot = CreateProtocol(_url);
	if(prot->MkDir(&su) != KProtocol::SUCCESS)
	{
	    ProcessError(prot, _url);
	    return;
	}
	delete prot;
    }
    else
    {
	printf("ERROR: Not implemented\n");
	QString err = "Mkdir ";
	err += _url;
	ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
	return;
    }
    
    ipc->done();
}

void KIOSlave::list( const char *_url, bool _bHTML )
{
    const char *old_url = _url;

    KURL u( _url );
    if ( u.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    KURL su( u.nestedURL() );
    if ( su.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    printf("MAIN:List starting...\n");

    if (ProtocolSupported(_url))
    {
	KProtocolDirEntry *de;
	KProtocol *prot = CreateProtocol(_url);
	
	printf("MAIN:Direntries starting...\n");
	prot->AllowHTML( _bHTML );
	prot->OpenDir(&su);
	ipc->flushDir(old_url);
	// Do we get some HTML as response
	if ( prot->isHTML() )
	{
	    // Let us emit all the HTML
	    prot->EmitData( ipc );
	}
	else // We get directory entries
	{
	    // Emit them all ...
	    while( ( de = prot->ReadDir() ) )
	    {
		/*
		  printf("MAIN:Direntry found... %s\n",de->name.data());
		  printf("*>       isdir: %d\n",de->isdir);
		  printf("*>        size: %d\n",de->size);
		  printf("*>        date: %s\n",de->date.data());
		  printf("*>      access: %s\n",de->access.data());
		  printf("*> owner/group: %s/%s\n",de->owner.data(),de->group.data());
		  */
		ipc->dirEntry(old_url, de->name.data(), de->isdir, de->size,
			      de->date.data(), de->access.data(),
			      de->owner.data(), de->group.data());
	    }
	    printf("MAIN:Direntries ready...\n");
	    prot->CloseDir();
	    delete prot;
	}
    }
    else
    {
	printf("ERROR: Not implemented\n");
	QString err = "List ";
	err += _url;
	ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
	return;
    }

    printf("MAIN:List ready...\n");
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
		
    printf("EXEC: '%s'\n",buffer);
    
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
    KURL u( _url );
    if ( u.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    KURL su( u.nestedURL() );
    if ( su.isMalformed() )
	return;
    
    if ( strcmp( su.protocol(), "file" ) == 0 )
    {
	int erg;

	struct stat buff;
	stat( su.path(), &buff );
	if ( S_ISDIR( buff.st_mode ) )
	    erg = rmdir( su.path() );
	else
	    erg = unlink( su.path() );
	
	if ( erg != 0 )
	{
	    printf("ERROR: Could not delete '%s'\n",_url );
	    ipc->fatalError( KIO_ERROR_CouldNotDelete, _url, errno );
	    return;
	}
    }
#ifdef TODO
    else if ( strcmp( su.protocol(), "tar" ) == 0 )
    {
	QString tar = su.path();
	
	if ( su.path()[ strlen( su.path() ) - 1 ] == 'z' )
	{
	    if ( !lockTgz( su.path() ) )
		return;
	    lockedTgzModified = TRUE;
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
	    printf("ERROR: Could not delete '%s'\n",_url );
	    ipc->fatalError( KIO_ERROR_CouldNotDelete, _url, errno );
	    return;
	}
    }
#endif
    else
    {
		printf("ERROR: Not implemented\n");
		QString err = "Delete ";
		err += _url;
		ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
		return;
    }
    
    ipc->done();
}


void KIOSlave::copy( const char *_src_url, const char *_dest_url, bool _overwriteExistingFiles )
{
    KURL u( _src_url );
    if ( u.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_src_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _src_url, 0 );
	return;
    }

    KURL su( u.nestedURL() );
    if ( su.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_src_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _src_url, 0 );
	return;
    }

    KURL u2( _dest_url );
    if ( u2.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_dest_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _dest_url, 0 );
	return;
    }
    KURL du( u2.nestedURL() );
    if ( du.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_dest_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _dest_url, 0 );
	return;
    }

    if(ProtocolSupported(_src_url) && ProtocolSupported(_dest_url))
    {
		KProtocol *src_prot = CreateProtocol(_src_url);
		KProtocol *dest_prot = CreateProtocol(_dest_url);

		int destmode = KProtocol::WRITE;
		if( _overwriteExistingFiles ) destmode |= KProtocol::OVERWRITE;
	
		if(dest_prot->Open(&du, destmode) != KProtocol::SUCCESS)
		{
	    	ProcessError(dest_prot, _dest_url);
	    	return;
		}

		printf("Open src\n");
		if(src_prot->Open(&su, KProtocol::READ) != KProtocol::SUCCESS)
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

		long c = 0, last = 0, l = 1;
		long size = src_prot->Size();
		char buffer[4096];

		printf("Copyloop starting\n");
		while ( !src_prot->atEOF() )
		{
			printf("read\n");
			if ((l = src_prot->Read( buffer, 4096 ) ) < 0)
			{
			    printf("read error (%ld)\n",l);
			    ProcessError(src_prot, _src_url);
			    return;
			}
			printf("write\n");
			if (dest_prot->Write(buffer, l) < l )
			{
			    ProcessError(dest_prot, _dest_url);
			    return;
			}
			printf("progress\n");
			c += l;
			if ( ( c * 100 / size ) != last )
			{
			    last = ( c * 100 / size );
			    ipc->progress( last );
			}
		}
	   	printf("ready: Close\n"); 
		src_prot->Close();
		dest_prot->Close();

		int permissions = src_prot->GetPermissions( su );
		printf("Got Permissions '%i'\n",permissions);
		dest_prot->SetPermissions( du, permissions );         
		
		printf("delete\n");
		delete src_prot;
		delete dest_prot;
	
		printf("******** COPY: Job completed using ProtocolManagement!\n");
		ipc->done();
		return;
    }
    else
    {
		printf("ERROR: Not implemented\n");
		QString err = "Copy ";
		err += _src_url;
		err += " to ";
		err += _dest_url;
		ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
		return;
    }

    ipc->done();
}

void KIOSlave::get( const char *_url )
{
    printf("******** GET '%s'\n",_url);
    // _url = ReformatURL(_url);
 
    KURL u( _url );
    if ( u.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    KURL su( u.nestedURL() );
    if ( su.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }
    
    if( ProtocolSupported( _url ) )
    {
		KProtocol *src_prot = CreateProtocol(_url);

		connect( src_prot, SIGNAL( mimeType( const char* ) ),
			 ipc, SLOT( mimeType( const char* ) ) );
		connect( src_prot, SIGNAL( redirection( const char* ) ),
			 ipc, SLOT( redirection( const char* ) ) );
		connect( src_prot, SIGNAL( info( const char* ) ),
			 ipc, SLOT( info( const char* ) ) );

		printf("Open src\n");
		if(src_prot->Open(&su, KProtocol::READ) != KProtocol::SUCCESS)
		{
		    ProcessError( src_prot, _url );
		    return;
		}

		long c = 0, last = 0, l = 1;
		long size = src_prot->Size();
		char buffer[1025];

		printf("Copyloop starting\n");
		while ( !src_prot->atEOF() )
		{
			printf("read\n");
			if ((l = src_prot->Read( buffer, 1024 ) ) < 0)
			{
			    printf("read error (%ld)\n",l);
			    ProcessError(src_prot, _url);
			    return;
			}
			buffer[l] = 0;
			ipc->data( buffer );
			printf("progress\n");
			c += l;
			if ( ( c * 100 / size ) != last )
			{
			    last = ( c * 100 / size );
			    ipc->progress( last );
			}
		}
	   	printf("ready: Close\n"); 
		src_prot->Close();
		
		printf("delete\n");
		delete src_prot;
	
		printf("******** COPY: Job completed using ProtocolManagement!\n");
		ipc->done();
		return;
    }
    else
    {
		printf("ERROR: Not implemented\n");
		QString err = "GET ";
		err += _url;
		ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
		return;
    }

    ipc->done();
}

void KIOSlave::cleanUp()
{
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

// Prevent us from zombies
void sig_handler2( int )
{
    int pid;
    int status;
    
    while( 1 )
    {
	pid = waitpid( -1, &status, WNOHANG );
	if ( pid <= 0 )
	{
	    // Reinstall signal handler, since Linux resets to default after
	    // the signal occured ( BSD handles it different, but it should do
	    // no harm ).
	    signal(SIGCHLD,sig_handler2);
	    return;
	}
    }
}

void sig_handler(int _sig)
{
    printf("GOT TERM\n");
    if ( _sig == SIGTERM )
	if ( slave != 0L )
	    slave->terminate();
    exit(0);
}


#include "main.moc"










