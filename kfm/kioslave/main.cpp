#include <kapp.h>

#include "kioslave_ipc.h"
#include "main.h"
#include "kio_errors.h"
#include "manage.h"
#include "protocol.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef HAVE_VOLMGT
#include <volmgt.h>
#include <qmsgbox.h>		/* for eject popup */
#endif

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif


#define FTP_LOGIN "anonymous"
#define FTP_PASSWD "joedoe@nowhere.crg"

void sig_handler(int);
void sig_handler2(int);

KIOSlave *slave = 0L;

int main(int argc, char **argv)
{
  // printf("A new kioslave has been started\n");
    
    if ( argc != 2 )
    {
      fprintf( stderr, "Usage: kioslave path\n");
	exit(1);
    }

    signal(SIGTERM,sig_handler);
    signal(SIGCHLD,sig_handler2);
    
    KApplication a( argc, argv );

    KIOSlave slave( argv[1] );
    
    a.exec();
	return(0);
}

void KIOSlave::ProcessError(KProtocol *prot, const char *srcurl)
{
    int KError, SysError;
    QString message;

    prot->GetLastError(KError, message, SysError);

    // fprintf( stderr, "KIOSlave-ERROR (%s): %s; %s\n",srcurl,message.data(),
    // strerror(SysError));
    ipc->fatalError(KError, srcurl, SysError);
}

KIOSlave::KIOSlave( char * _path )
{
    ipc = new KIOSlaveIPC( _path );
    
    if ( !ipc->isConnected() )
    {
//	fprintf( stderr, "Could not connect to KIO om path %s\n", _path );
	exit(1);
    }

    copyDestFile = 0L;

    connect( ipc, SIGNAL( copy( const char*, const char*, bool ) ),
	     this, SLOT( copy( const char*, const char*, bool ) ) );
    connect( ipc, SIGNAL( del( const char* ) ),
	     this, SLOT( del( const char* ) ) );
    connect( ipc, SIGNAL( get( const char*, const char*, const char *, bool ) ),
	     this, SLOT( get( const char*, const char*, const char *, bool ) ) );
    connect( ipc, SIGNAL( mkdir( const char* ) ), this, SLOT( mkdir( const char* ) ) );
    connect( ipc, SIGNAL( unmount( const char* ) ), this, SLOT( unmount( const char* ) ) );
    connect( ipc, SIGNAL( mount( bool, const char*, const char*, const char* ) ),
	     this, SLOT( mount( bool, const char*, const char*, const char* ) ) );
    connect( ipc, SIGNAL( list( const char*, bool ) ), this, SLOT( list( const char*, bool ) ) );
    connect( ipc, SIGNAL( cleanUp() ), this, SLOT( cleanUp() ) );
    connect( ipc, SIGNAL( getPID() ), this, SLOT( getPID() ) );

    ipc->hello();
}

KIOSlave::~KIOSlave()
{
debug("delete ~KIOSlave");
    delete ipc;
}

void KIOSlave::getPID()
{
    ipc->setPID( (int)getpid() );
}

void KIOSlave::slotRedirection( const char *_url )
{
    sRedirection = _url;
}

int KIOSlave::OpenWithRedirect(KProtocol *prot, KURL *url, int mode)
{
    int result = KProtocol::FAIL;
    for(;;)
    {
       sRedirection = "";
       result = prot->Open(url, KProtocol::READ);
       if (sRedirection.isEmpty())
           break;
       *url = sRedirection.data();
    }
    return result;
}


void KIOSlave::mkdir( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() )
    {
      // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    KURL su( u.nestedURL() );
    if ( su.isMalformed() )
    {
      // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    if(ProtocolSupported(_url))
    {
	KProtocol *prot = CreateProtocol(_url);
	if(prot->MkDir(&su) != KProtocol::SUCCESS)
	{
	    ProcessError(prot, _url);
	    delete prot;
	    return;
	}
	delete prot;
    }
    else
    {
      // fprintf( stderr,"ERROR: Not implemented\n");
	QString err = "Mkdir ";
	err += _url;
	ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
	return;
    }
    
    ipc->done();
}

void KIOSlave::list( const char *_url, bool _bHTML )
{
    KURL u( _url );
    if ( u.isMalformed() )
    {
      // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    KURL su;
    if ( u.hasSubProtocol() )
	su = u.nestedURL().data();
    else
	su = _url;
    if ( su.isMalformed() )
    {
      // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    if ( ProtocolSupported( _url ) )
    {
	KProtocolDirEntry *de;
	KProtocol *prot = CreateProtocol( _url );

	sRedirection = "";
	connect( prot, SIGNAL( redirection( const char* ) ),
		 ipc, SLOT( redirection( const char* ) ) );
	connect( prot, SIGNAL( redirection( const char* ) ),
		 this, SLOT( slotRedirection( const char* ) ) );

	prot->AllowHTML( _bHTML );
	if ( prot->OpenDir(&su) == KProtocol::FAIL )
	{
	    int kerror;
	    QString msg;
	    int syserror;
	    prot->GetLastError( kerror, msg, syserror );
	    // fprintf( stderr, "ERROR: Could not enter '%s'\n",_url );
	    ipc->fatalError( kerror, _url, syserror );
            delete prot;
	    return;
	}
	    
	if ( !sRedirection.isEmpty() )
	    _url = sRedirection.data();
	
	// Do we get some HTML as response
	if ( prot->isHTML() )
	{
	    // Let us emit all the HTML
	    prot->EmitData( ipc );
	    prot->CloseDir();
	}
	else // We get directory entries
	{
	    // Remove password
	    KURL u( _url );
	    u.setPassword( "" );
	    QString url = u.url().data();

            // Clear the list stored in kioserver. With password removed !
            ipc->flushDir( url.data() );
	    
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
		ipc->dirEntry( url, de->name.data(), de->isdir, de->size,
			      de->date.data(), de->access.data(),
			      de->owner.data(), de->group.data());
	    }
	    prot->CloseDir();
	}
        delete prot;
    }
    else
    {
      // fprintf( stderr, "ERROR: Not implemented\n");
	QString err = "List ";
	err += _url;
	ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
	return;
    }

    ipc->done();
}

void KIOSlave::mount( bool _ro, const char *_fstype, const char* _dev, const char *_point )
{
    int  retvalue;
    QString err;

#ifdef HAVE_VOLMGT

    retvalue = 1;

    if( volmgt_running() ) {
	    if( volmgt_check( (char *) _dev ) == 0 ) {
		err = QString( "No Media inserted or Media not recognized." );
	    } else {
		retvalue = 0;
	    }
    } else {
	err = QString( "Volume Management is not running." );
    }
#else
    char buffer[ 1024 ];
    int t = (int)time( 0L );
    
    // Look in /etc/fstab ?
    if ( _fstype == 0L || _point == 0L || _fstype[0] == 0 || _point[0] == 0 )
	sprintf( buffer, "mount %s 2>"_PATH_TMP"/mnt%i",_dev, t );
    else if ( _ro )
	sprintf( buffer, "mount -rt %s %s %s 2>"_PATH_TMP"/mnt%i",_fstype, _dev, _point, t );
    else
	sprintf( buffer, "mount -t %s %s %s 2>"_PATH_TMP"/mnt%i",_fstype, _dev, _point, t );
		
    retvalue = system( buffer );
    // test logfile here to remove it if it exists
    // (if there is a warning, it can exist even if retvalue==0)
    sprintf( buffer, _PATH_TMP"/mnt%i", t );
    err = testLogFile( buffer );
#endif /* HAVE_VOLMGT */

    if (retvalue == 0)
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

    int retvalue;
    QString err;
 
#ifdef HAVE_VOLMGT
    char *res;
 
    if( volmgt_running() ) {
        if( (res = volmgt_symname( (char *) _point )) == NULL ) {
            retvalue = 1;
            err = QString( strerror( errno ));
        } else {
            sprintf( buffer, "/usr/bin/eject %s 2>"_PATH_TMP"mnt%i",
                     res, t );
            /*
             * from eject(1): exit status == 4 => manually eject media
             *                exit status == 0 => media was ejected
             */
            if( (retvalue = WEXITSTATUS( system( buffer ))) == 4 ) {
                retvalue = 0;
                sprintf( buffer, i18n( "The media \"%s\" can now be manually ejected" ), res );
                QMessageBox::information( 0, i18n( "Volume Management" ), buffer );
            }
            err = testLogFile( buffer );
        }
    } else {
 	err = QString( "Volume Management is not running." );
    }
#else
    sprintf( buffer, "umount %s 2>"_PATH_TMP"mnt%i", _point, t );
    retvalue = system( buffer );
    
    // test logfile here to remove it if it exists
    // (if there is a warning, it can exist even if revalue==0)
    sprintf( buffer, _PATH_TMP"/mnt%i", t );
    err = testLogFile( buffer );
#endif /* HAVE_VOLMGT */

    if (retvalue == 0)
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
      // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }

    KURL su( u.nestedURL() );
    if ( su.isMalformed() )
	return;
    
    if ( ProtocolSupported( _url ) )
    {
	KProtocol *prot = CreateProtocol( _url );
        
        if (prot->Delete( &su) == KProtocol::FAIL)
        {
	  // fprintf( stderr, "ERROR: Could not delete '%s'\n",_url );
	    ipc->fatalError( KIO_ERROR_CouldNotDelete, _url, errno );
	    delete prot;
	    return;
	}
	delete prot;
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
	logFile.sprintf( _PATH_TMP"/tarlog%i", time( 0L ) );

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
#endif
    else
    {
      // fprintf( stderr, "ERROR: Not implemented\n");
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
      // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_src_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _src_url, 0 );
	return;
    }

    KURL su( u.nestedURL() );
    if ( su.isMalformed() )
    {
      // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_src_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _src_url, 0 );
	return;
    }

    KURL u2( _dest_url );
    if ( u2.isMalformed() )
    {
      // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_dest_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _dest_url, 0 );
	return;
    }
    KURL du( u2.nestedURL() );
    if ( du.isMalformed() )
    {
      // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_dest_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _dest_url, 0 );
	return;
    }

    if( ProtocolSupported( _src_url ) && ProtocolSupported( _dest_url ) )
    {
		KProtocol *src_prot = CreateProtocol(_src_url);
		KProtocol *dest_prot = CreateProtocol(_dest_url);

                connect( src_prot, SIGNAL( redirection( const char* ) ),
                         ipc, SLOT( redirection( const char* ) ) );
                connect( src_prot, SIGNAL( redirection( const char* ) ),
                         this, SLOT( slotRedirection( const char* ) ) );

		int destmode = KProtocol::WRITE;
		if( _overwriteExistingFiles ) destmode |= KProtocol::OVERWRITE;
	
		if( dest_prot->Open(&du, destmode) != KProtocol::SUCCESS )
		{
		    ProcessError(dest_prot, _dest_url);
		    return;
		}
		if( OpenWithRedirect(src_prot, &su, KProtocol::READ) != KProtocol::SUCCESS )
		{
		    ProcessError(src_prot, _src_url);
		    return;
		}
		long c = 0, l = 1;
		float last = 0.0;
		long size = src_prot->Size();
		char buffer[4096];

		while ( !src_prot->atEOF() )
		{
			if ( ( l = src_prot->Read( buffer, 4096 ) ) < 0 )
			{
			  // fprintf( stderr, "read error (%ld)\n", l );
			    ProcessError(src_prot, _src_url);
			    return;
			}
			if (dest_prot->Write(buffer, l) < l )
			{
                            ipc->fatalError( KIO_ERROR_DiskFull, _dest_url, errno );
			    return;
			}
			c += l;
			if ( size != 0 && ( (float)c / (float)size * 100.0 ) != last )
			{
			    last = ( (float)c / (float)size * 100.0 );
			    ipc->progress( (int)last );
			}
		}
		src_prot->Close();
		dest_prot->Close();

		int permissions = src_prot->GetPermissions( su );
		dest_prot->SetPermissions( du, permissions );         
		
		delete src_prot;
		delete dest_prot;
	
		ipc->done();
		return;
    }
    else
    {
      // fprintf( stderr, "ERROR: Not implemented\n");
		QString err = "Copy ";
		err += _src_url;
		err += " to ";
		err += _dest_url;
		ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
		return;
    }

    ipc->done();
}

void KIOSlave::get( const char *_url, const char *_data, const char *_cookies, bool _reload )
{  
  get( _url, _reload, _data, _cookies );
}

void KIOSlave::get( const char *_url, bool _reload, const char *_data, const char *_cookies )
{
  KURL u( _url );
  if ( u.isMalformed() )
  {
    // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_url );
    ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
    return;
  }

  KURL su( u.nestedURL() );
  if ( su.isMalformed() )
  {
    // fprintf( stderr, "ERROR: Malformed URL '%s'\n",_url );
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
    connect( src_prot, SIGNAL( cookie( const char*, const char* ) ),
	     ipc, SLOT( cookie( const char*, const char* ) ) );
    int err;
    src_prot->SetData(_data);
    src_prot->SetCookies(_cookies);
    
    if ( !_reload )
      err = src_prot->Open( &su, KProtocol::READ );
    else    
      err = src_prot->ReOpen( &su, KProtocol::READ );

    if( err != KProtocol::SUCCESS )
    {
      ProcessError( src_prot, _url );
      return;
    }
    
    long c = 0, last = 0, l = 1;
    long size = src_prot->Size();
    char buffer[1025];
    
    while ( !src_prot->atEOF() )
    {
      if ( ( l = src_prot->Read( buffer, 1024 ) ) < 0)
      {
	// fprintf( stderr, "read error (%ld)\n",l);
	ProcessError(src_prot, _url);
	return;
      }
      buffer[l] = 0;
      ipc->data( IPCMemory( buffer, l ) );
      c += l;
      if ( ( c * 100 / size ) != last )
      {
	last = ( c * 100 / size );
	ipc->progress( last );
      }
    }
    src_prot->Close();
		
    delete src_prot;
    
    ipc->done();
    return;
  }
  else
  {
    // fprintf( stderr, "ERROR: Not implemented\n");
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
    {
	unlink( _filename );
	return QString();
    }
    
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
  // fprintf( stderr, "KIOSLAVE GOT TERM\n");
    if ( _sig == SIGTERM )
	if ( slave != 0L )
	    slave->terminate();
    exit(0);
}


#include "main.moc"

#ifdef TORBEN_TEST

/********************************************************************************************
 * 
 * Experimental code to fix bugs in recursive move/copy/delete
 *
 ********************************************************************************************/

void KIOSlave::move( const char *_src_url, const char *_dest_url, bool _overwriteExistingFiles )
{
    destURLList.clear();
    srcURLList.clear();
    
    /**
     * Test every URL. We wont test them later on.
     */
    KURL u( _src_url );
    if ( u.isMalformed() )
    {
	// fprintf( stderr, "ERROR: Malformed URL '%s'\n",_src_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _src_url, 0 );
	return;
    }

    KURL su( u.nestedURL() );
    if ( su.isMalformed() )
    {
	// fprintf( stderr, "ERROR: Malformed URL '%s'\n",_src_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _src_url, 0 );
	return;
    }

    KURL u2( _dest_url );
    if ( u2.isMalformed() )
    {
	// fprintf( stderr, "ERROR: Malformed URL '%s'\n",_dest_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _dest_url, 0 );
	return;
    }
    KURL du( u2.nestedURL() );
    if ( du.isMalformed() )
    {
	// fprintf( stderr, "ERROR: Malformed URL '%s'\n",_dest_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _dest_url, 0 );
	return;
    }

    // Perhaps we can move on one device ?
    // That is very fast!
    // HACK
    // Perhaps other protocols have fast move functions, too ?
    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() &&
	 strcmp( u2.protocol(), "file" ) == 0 && !u2.hasSubProtocol() )
    {
	// Everything ok ? => We are done
	if ( rename( su.path(), du.path() ) == 0 )
	{
	    ipc->done();
	    return;
	}
    }

    recursionLevel = 0;
    moveRecursive( _src_url, _dest_url, _overwriteExistingFiles );
}

void KIOSlave::moveRecursive( const char *_src_url, const char *_dest_url, bool _overwriteExistingFiles )
{
    /**
     * We can assume that these URLs are already tested.
     */
    KURL u( _src_url );
    KURL su( u.nestedURL() );
    KURL u2( _dest_url );
    KURL du( u2.nestedURL() );
    
    /**
     * Lets test wether we have to move a directory
     */
    KProtocolDirEntry *de;
    KProtocol *prot = CreateProtocol(_url);
    
    // We want KProtocolDirEntries!!
    prot->AllowHTML( FALSE );
    // No error? => It is a directory
    if ( prot->OpenDir(&su) != KProtocol::FAIL )
    {
	// Loop over all entries
	while( ( de = prot->ReadDir() ) )
	{
	    // Caculate destination
	    QString tmp = _dest_url;
	    if ( tmp.right(1) != "/" )
		tmp += "/";
	    tmp += de->name.data();
	    // Calculate src
	    QString tmp2 = _dest_url;
	    if ( tmp2.right(1) != "/" )
		tmp2 += "/";
	    tmp2 += de->name.data();
	    
	    // Directory ? => Recursion
	    if ( de->isdir )
	    {
		// Append this directory to the list
		// of directories we have to create.
		mkdirURLList.append( tmp );
		// Append this directory to the list
		// of directories we have to delete afterwards.
		rmdirURLList.append( tmp );
		
		// Recursion
		recursionLevel++;
		moveRecursive( tmp2, tmp, _overwriteExistingFiles );
		recursionLevel--;
	    }
	    // HACK!!!
	    /* else if ( de->islink )
	       {
	       } */
	    // It is a usual file
	    else
	    {
		destURLList.append( tmp.data() );
		srcURLList.append( tmp2.data() );
	    }
	}
	prot->CloseDir();
	delete prot;

	// Did we traverse the complete tree ?
	if ( recursionLevel == 0 )
	{
	    moveList( _overwriteExistingFiles );
	    ipc->done();
	    return;
	}
    }
    // It is just a simple file
    else
    {
	moveSimple( _src_url, _dest_url, _overwriteExistingFiles );
	ipc->done();
	return;
    }
}
    
void KIOSlave::moveList( bool _overwriteExistingFiles )
{
    char *dir;
    for ( mkdir = dirURLList.first(); dir != 0L; mkdir = dirURLList.next() )
	mkdirSimple( dir );
    
    char *s;
    char *d;
    for ( s = srcURLList.first(), d = destURLList.first();
	  s != 0L && d != 0L;
	  s = srcURLList.next(), d = destURLList.next() )
    {
	moveSimple( s, d, _overwriteExistingFiles );
    }

    // Remove directories in reverse order
    for ( dir = rmdirURLList.last(); dir != 0L; dir = rmdirURLList.prev() )
	rmdirSimple( dir );
}

bool KIOSlave::moveSimple( const char *_src_url, const char *_dest_url, bool _overwriteExistingFiles )
{
    /**
     * We can assume that these URLs are already tested.
     */
    KURL u( _src_url );
    KURL su( u.nestedURL() );
    KURL u2( _dest_url );
    KURL du( u2.nestedURL() );

    if ( !copySimple( src_url, _dest_url, _overwriteExistingFiles ) )
	return FALSE;
    
    // Delete the file we just copied
    if ( !delSimple( src_url ) )
	return FALSE;

    return TRUE;
}

bool KIOSlave::mkdirSimple( const char *_url )
{
    // We can assume that this URL is correct
    KURL u( _url );
    KURL su( u.nestedURL() );

    if( ProtocolSupported(_url) )
    {
	KProtocol *prot = CreateProtocol( _url );
	if( prot->MkDir( &su ) != KProtocol::SUCCESS )
	{
	    ProcessError(prot, _url);
	    return;
	}
	delete prot;
    }
    else
    {
	// fprintf( stderr, "ERROR: Not implemented\n");
	QString err = "Mkdir ";
	err += _url;
	ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
	return FALSE;
    }

    return TRUE;
}

bool KIOSlave::delSimple( const char *_url )
{
    // We can assume that this URL is correct
    KURL u( _url );
    KURL su( u.nestedURL() );

    if( ProtocolSupported(_url) )
    {
	KProtocol *prot = CreateProtocol( _url );
	if( prot->Del( &su ) != KProtocol::SUCCESS )
	{
	    ProcessError(prot, _url);
	    return;
	}
	delete prot;
    }
    else
    {
	// fprintf( stderr, "ERROR: Not implemented\n");
	QString err = "Delete ";
	err += _url;
	ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );
	return FALSE;
    }

    return TRUE;
}

bool KIOSlave::copySimple( const char *_src_url, const char *_dest_url, bool _overwriteExistingFiles )
{
    if( ProtocolSupported( _src_url ) && ProtocolSupported( _dest_url ) )
    {
	KProtocol *src_prot = CreateProtocol(_src_url);
	KProtocol *dest_prot = CreateProtocol(_dest_url);

	int destmode = KProtocol::WRITE;
	if( _overwriteExistingFiles )
	    destmode |= KProtocol::OVERWRITE;
	
	if( dest_prot->Open( &du, destmode ) != KProtocol::SUCCESS )
	{
	    ProcessError(dest_prot, _dest_url);
	    return;
	}

	if(src_prot->Open(&su, KProtocol::READ) != KProtocol::SUCCESS)
	{
	    ProcessError(src_prot, _src_url);
	    return;
	}

	long c = 0, last = 0, l = 1;
	long size = src_prot->Size();
	char buffer[4096];

	while ( !src_prot->atEOF() )
	{
	    if ((l = src_prot->Read( buffer, 4096 ) ) < 0)
	    {
		// fprintf( stderr, "read error (%ld)\n",l);
		ProcessError(src_prot, _src_url);
		return;
	    }
	    if (dest_prot->Write(buffer, l) < l )
	    {
                ipc->fatalError( KIO_ERROR_DiskFull, _dest_url, errno );
		return;
	    }
	    c += l;
	    if ( size != 0 && ( c * 100 / size ) != last )
	    {
		last = ( c * 100 / size );
		ipc->progress( last );
	    }
	}
	src_prot->Close();
	dest_prot->Close();
	
	int permissions = src_prot->GetPermissions( su );
	dest_prot->SetPermissions( du, permissions );         
	
	delete src_prot;
	delete dest_prot;
    }
    else
    {
	// fprintf( stderr, "ERROR: Not implemented\n");
	QString err = "Copy ";
	err += _src_url;
	err += " to ";
	err += _dest_url;
	ipc->fatalError( KIO_ERROR_NotImplemented, err.data(), 0 );

	return FALSE;
    }

    return TRUE;
}

#endif
