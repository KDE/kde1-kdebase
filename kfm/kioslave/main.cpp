#include <qapp.h>

#include "kioslave_ipc.h"
#include "main.h"
#include "kio_errors.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include "ftplib.h"

#define FTP_LOGIN "anonymous"
#define FTP_PASSWD "joedoe@nowhere.crg"

void sig_handler(int);

KIOSlave *slave = 0L;

void main( int argc, char **argv )
{
    if ( argc != 2 )
    {
	printf("Usage: kioslave port\n");
	exit(1);
    }

    signal(SIGTERM,sig_handler);
    
    QApplication a( argc, argv );

    slave = new KIOSlave( atoi( argv[1] ) );
    
    a.exec();
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
	printf("ERROR: Malformed URL '%s'\n",_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _url, 0 );
	return;
    }
    
    if ( strcmp( su.protocol(), "file" ) == 0 )
    {
	if ( ::mkdir( su.path(), S_IXUSR | S_IWUSR | S_IRUSR ) != 0L )
	{
	    printf("ERROR: Could not mkdir '%s'\n",_url );
	    ipc->fatalError( KIO_ERROR_CouldNotMkdir, _url, errno );
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

	ftpMkdir( su.path() );
	{
	    printf("ERROR: Could not mkdir '%s'\n",_url );
	    ipc->fatalError( KIO_ERROR_CouldNotMkdir, _url, errno );
	    return;
	}
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

void KIOSlave::list( const char *_url )
{
    KURL su( _url );
    if ( su.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_url );
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
	    cmd.sprintf( "tar -tzf %s 1>%s 2>%s", su.path(), outFile.data(), logFile.data() );
	else
	    cmd.sprintf( "tar -tf %s 1>%s 2>%s", su.path(), outFile.data(), logFile.data() );

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
	    printf("ERROR: Could not read '%s'\n",outFile.data());
	    ipc->fatalError( KIO_ERROR_CouldNotRead, outFile.data(), errno );
	    return;
	}
	
	// Delete the output file
	unlink( outFile.data() );

	// List of all directories we already have flushed
	QStrList dirs;
	// List of all di
	// Flush the parent directory
	QString t;
	t.sprintf( "tar:%s#", su.path() );
	ipc->flushDir( t.data() );

	// Find all files in all directories
	char *s;
	for ( s = files.first(); s != 0L; s = files.next() )
	{
	    printf("########### '%s'\n",s);
	    
	    // Get all directories of the current file and check wether
	    // we have noticed them already. Otherwise send them to the server.
	    QString s2 = s;
	    int i = 0;
	    int j = -1;
	    
	    while ( ( i = s2.find( "/", i ) ) != -1 )
	    {
		// Get the directories name
		QString s3 = s2.mid( j + 1, i - j );
		// Get the URL of the directories parent directory
		QString s4;
		s4.sprintf( "tar:%s#", su.path() );
		if ( j > -1 )
		    s4 += s2.left( j + 1 );
		// Send to the server if new
		QString s5 = s2.left( i + 1 );
		if ( dirs.find( s5.data() ) == - 1 )
		{
		    // Get the URL of the new directory
		    QString s6;
		    s6.sprintf( "tar:%s#", su.path() );
		    s6 += s2.left( i + 1 );
		    ipc->flushDir( s6.data() );
		    // ipc->dirEntry( s4.data(), s3.data(), TRUE, -1, 0L, 0L, 0L, 0L );
		    dirs.append( s5.data() );
		}
		j = i;
		i++;
	    }
	    
	    // Separate the filename from the directory
	    QString dir;
	    QString entry;
	    QString str = s;
	    i = str.findRev( "/" );
	    // Root directory ?
	    if ( i == - 1 )
	    {}
	    if ( i == str.length() - 1 )
	    {
		int k = str.findRev( "/", i - 1 );
		if ( k == -1 )
		{
		    dir.sprintf( "tar:%s#", su.path() );
		    entry = str.data();
		}
		else
		{
		    dir.sprintf( "tar:%s#", su.path() );
		    dir += str.left( k + 1 );
		    entry = str.mid( k + 1, i - k );
		}
	    }
	    else
	    {
		dir.sprintf( "tar:%s#%s", su.path(), str.left( i + 1 ).data() );
		entry = str.mid( i + 1, str.length() ).data();
	    }
	    // Sned the new directory entry
	    ipc->dirEntry( dir.data(), entry.data(), FALSE, -1, 0L, 0L, 0L, 0L );
	}
    }
    else if ( strcmp( su.protocol(), "ftp" ) == 0 )
    {
	QString outFile;
	outFile.sprintf( "/tmp/ftp%i", time( 0L ) );

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

	printf("Logging in at '%s'as '%s' with '%s'\n",user.data(),passwd.data());
	
	if ( !lockFTP( su.host(), 21, user.data(), passwd.data() ) )
	    return;

	FILE * f = fopen( outFile.data(), "wb" );
	if ( f == 0L )
	{
	    printf("ERROR: Could not write '%s'\n",outFile.data());
	    ipc->fatalError( KIO_ERROR_CouldNotWrite, outFile.data(), errno );
	    return;
	}

	copyDestFile = f;
	copyDestFileName = outFile.data();
	copyDestFileName.detach();
	
	if ( !ftpDir( f, su.path() ) )
	{
	    fclose( f );
	    unlink( outFile.data() );
	    printf("ERROR: Could not list '%s'\n",_url );
	    ipc->fatalError( KIO_ERROR_CouldNotList, _url, 0 );
	    return;	    
	}

	fclose( f );

	copyDestFile = 0L;
	
	// Open the output file containing the directory listing in long from
	f = fopen( outFile.data(), "rb" );
	if ( f != 0L )
	{
	    char line[1024];
	    char* p = "";

	    // Flush the directory
	    ipc->flushDir( _url );
	    
	    // Load all line from the output file
	    while ( p != 0L )
	    {
		if ( ( p = fgets( line, 1023, f ) ) != 0L )
		{
		    QString l = line;
		    l = l.simplifyWhiteSpace();
		    if ( l != "." && l !=".." )
		    {
			strcpy( line, l.data() );
			
			// Dont use the line starting with "total"
			if ( strncmp( line, "total", 5 ) != 0L )
			{
			    // Parse the line
			    bool err = FALSE;
			    bool isdir = ( line[0] == 'd' );
			    QString owner;
			    QString group;
			    QString access;
			    QString creationDate;
			    QString name;
			    int size;
			    
			    char *p2 = p;
			    char *p = strchr( line, ' ' );
			    
			    if ( p == 0L )
				err = TRUE;
			    else
			    {
				*p = 0;
				access = p2;
				p2 = p + 1;

				p = strchr( p2, ' ' );
				if ( p == 0L )
				    err = TRUE;
				else
				{
				    p2 = p + 1;
				    p = strchr( p2, ' ' );
				    if ( p == 0L )
					err = TRUE;
				    else
				    {
					*p = 0;
					owner = p2;
					p2 = p + 1;
					p = strchr( p2, ' ' );
					if ( p == 0L )
					    err = TRUE;
					else
					{
					    *p = 0;
					    group = p2;
					    p2 = p + 1;
					    p = strchr( p2, ' ' );
					    if ( p == 0L )
						err = TRUE;
					    else
					    {
						*p = 0;
						size = atoi( p2 );
						p2 = p + 1;
						p = strchr( p2, ' ' );
						if ( p == 0L )
						    err = TRUE;
						else
						{
						    p = strchr( p + 1, ' ' );
						    if ( p == 0L )
							err = TRUE;
						    else
						    {
							p = strchr( p + 1, ' ' );
							if ( p == 0L )
							    err = TRUE;
							else
							{
							    *p = 0;
							    creationDate = p2;
							    p2 = p + 1;
							    name = p2;
							    if ( isdir )
								name += "/";
							}
						    }
						}
					    }
					}
				    }
				}
			    }
			    if ( !err )
			    ipc->dirEntry( _url, name.data(), isdir, size, creationDate.data(), access.data(),
					   owner.data(), group.data() );
			}
		    }
		}
	    }
	    fclose( f );
	}
	else
	{
	    unlink( outFile.data() );
	    printf("ERROR: Could not read '%s'\n",outFile.data());
	    ipc->fatalError( KIO_ERROR_CouldNotRead, outFile.data(), errno );
	    return;
	}

	// Delete the outpt file
	unlink( outFile.data() );
    }
    else
    {
	printf("ERROR: Not implemented\n");
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
    KURL su( _url );
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
    printf("******** COPY '%s' to '%s\n",_src_url,_dest_url);
    
    KURL su( _src_url );
    if ( su.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_src_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _src_url, 0 );
	return;
    }
    
    KURL du( _dest_url );
    if ( du.isMalformed() )
    {
	printf("ERROR: Malformed URL '%s'\n",_dest_url );
	ipc->fatalError( KIO_ERROR_MalformedURL, _dest_url, 0 );
	return;
    }
    
    /*
     * Copy from hard disk to hard disk.
     */
    if ( strcmp( su.protocol(), "file" ) == 0 && strcmp( du.protocol(), "file" ) == 0 )
    {
	struct stat buff;
	if ( stat( du.path(), &buff ) == 0 && !_overwriteExistingFiles )
	{
	    ipc->fatalError( KIO_ERROR_FileExists, _dest_url, errno );	
	    return;
	}
	
	stat( su.path(), &buff );
	int size = buff.st_size;

	FILE *in = fopen( su.path(), "rb" );
	if ( in != 0L )
	{
	    FILE * out = fopen( du.path(), "wb" );
	    if ( out != 0L )
	    {
		copyDestFile = out;
		copyDestFileName = du.path();
		copyDestFileName.detach();
		
		int c = 0;
		int last = 0;
		int l = 1;
		char buffer[4096];
		while ( l > 0 )
		{
		    l = fread( buffer, 1, 4096, in );
		    if ( l > 0 )
		    {
			printf(" l = %i\n",l);
			fwrite( buffer, 1, l, out );
			c += l;
			if ( ( c * 100 / size ) != last )
			{
			    last = ( c * 100 / size );
			    printf("percent %i\n", last );
			    ipc->progress( last );
			}
		    }
		}
		
		fclose( in );
		fclose( out );
		copyDestFile = 0L;
	    }
	    else
	    {
		printf(" Error: Could not write to %s\n", _dest_url );
		ipc->fatalError( KIO_ERROR_CouldNotWrite, _dest_url, errno );
		return;
	    }
	}
	else
	{
	    printf(" Error: Could not read from %s\n",_src_url );
	    ipc->fatalError( KIO_ERROR_CouldNotRead, _src_url, errno );
	    return;
	}
    }
    
    else if( strcmp( su.protocol(), "http" ) == 0 && strcmp( du.protocol(), "file" ) == 0 )
    {
	struct stat buff;
	if ( stat( du.path(), &buff ) == 0 && !_overwriteExistingFiles )
	{
	    ipc->fatalError( KIO_ERROR_FileExists, _dest_url, errno );	
	    return;
	}
	
	int sock = ::socket(PF_INET,SOCK_STREAM,0);
	if (sock < 0)
	{
	    printf("ERROR:Could not make socket\n");
	    ipc->fatalError( KIO_ERROR_CouldNotCreateSocket, _src_url, errno );
	    return;
	}

	struct sockaddr_in server_name;
	struct hostent *hostinfo;
	server_name.sin_family = AF_INET;
	server_name.sin_port = htons( 80 );
	hostinfo = gethostbyname( su.host() );
  
	if ( hostinfo == 0L )
	{
	    printf("ERROR: Unknown host\n");
	    ipc->fatalError( KIO_ERROR_UnknownHost, _src_url, errno );
	    return;
	}
	server_name.sin_addr = *(struct in_addr*) hostinfo->h_addr;    
  
	if ( 0 > ::connect( sock, (struct sockaddr*)(&server_name), sizeof( server_name ) ) )
	{
	    printf("ERROR:Could not make socket\n");
	    ipc->fatalError( KIO_ERROR_CouldNotConnect, _src_url, errno );
	    return;
	}

	write( sock, "GET ", 4 );
	if ( su.path()[0] == 0 )
	    write( sock, "/", 1 );
	else
	    write( sock, su.path(), strlen( su.path() ) );
	write( sock, " HTTP/1.0\n\n", 11 );
	
	FILE* in = fdopen( sock, "r+" );
	if ( in == 0L )
	{
	    printf("ERROR:Could not open for read\n");
	    ipc->fatalError( KIO_ERROR_CouldNotRead, _src_url, errno );
	    return;
	}
	
	/// Dont know the size yet
	int size = 0xFFFFFFF;

	char buffer[4096];
	bool header = TRUE;
	while ( header )
	{
	    if ( fgets( buffer, 4095, in ) == NULL )
	    {
		printf("ERROR:Could not read header from socket\n");
		ipc->fatalError( KIO_ERROR_CouldNotRead, _src_url, errno );
		return;
	    }
	    printf(">> '%s'\n",buffer);
	    
	    if ( strncmp( buffer, "Content-length: ", 16 ) == 0 )
		size = atoi( buffer + 16 );
	    
	    /* if ( strncmp( buffer, "Location: ", 10 ) == 0 )
	    {
		close( sock );
		buffer[ strlen( buffer ) - 1 ] = 0;
		QString tmp = buffer + 10;
		tmp = tmp.stripWhiteSpace();
		printf("######### Trying '%s' instead\n",tmp.data() );
		copy( tmp.data(), _dest_url );
		return;
	    } */
	    
	    if ( buffer[0] == 10 || buffer[0] == 13 )
		header = FALSE;
	}

	FILE * out = fopen( du.path(), "wb" );
	if ( out != 0L )
	{
	    copyDestFile = out;
	    copyDestFileName = du.path();
	    copyDestFileName.detach();
		    	    
	    int c = 0;
	    int last = 0;
	    int l = 1;
	    while ( c < size && !feof( in ) )
	    {
		if ( ( l = fread( buffer, 1, 4096, in ) ) <= 0 )
		{
		    printf("ERROR:Could not read from socket\n");
		    ipc->fatalError( KIO_ERROR_CouldNotRead, _src_url, errno );
		    return;
		}
		if ( fwrite( buffer, 1, l, out ) <= 0 )
		{
		    printf("ERROR:Could not write\n");
		    ipc->fatalError( KIO_ERROR_CouldNotWrite, _dest_url, errno );
		    return;
		}
		c += l;
		if ( ( c * 100 / size ) != last )
		{
		    last = ( c * 100 / size );
		    printf("percent %i\n", last );
		    ipc->progress( last );
		}
	    }
	    
	    close( sock );
	    fclose( out );
	    copyDestFile = 0L;
	}
	else
	{
	    printf(" Error: Could not write to %s\n", _dest_url );
	    ipc->fatalError( KIO_ERROR_CouldNotWrite, _dest_url, errno );
	    return;
	}
    }
    else if( strcmp( su.protocol(), "tar" ) == 0 && strcmp( du.protocol(), "file" ) == 0 )
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
    else if( strcmp( su.protocol(), "ftp" ) == 0 && strcmp( du.protocol(), "file" ) == 0 )
    {
	struct stat buff;
	if ( stat( du.path(), &buff ) == 0 && !_overwriteExistingFiles )
	{
	    ipc->fatalError( KIO_ERROR_FileExists, _dest_url, errno );	
	    return;
	}
	
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
	
	FILE * out = fopen( du.path(), "wb" );
	if ( out == 0L )
	{
	    printf("ERROR: Could not write\n");
	    ipc->fatalError( KIO_ERROR_CouldNotWrite, du.path(), 0 );
	    return;
	}

	copyDestFile = out;
	copyDestFileName = du.path();
	copyDestFileName.detach();
	
	int erg = ftpGet( out, su.path(), 'I', ipc );
	
	copyDestFile = 0L;
	fclose( out );
	
	if ( !erg )
	{
	    printf("ERROR: CouldNotRead\n");
	    ipc->fatalError( KIO_ERROR_CouldNotRead, _src_url, errno );
	    return;
	}
    }
    else if( strcmp( su.protocol(), "file" ) == 0 && strcmp( du.protocol(), "ftp" ) == 0 )
    {
	// TODO: Port not implemented!!!
	// if ( !lockFTP( su.host(), su.port(), FTP_LOGIN, FTP_PASSWD ) )
        QString user = du.user();
        QString passwd = du.passwd();
	if ( user.length() <= 0 )
	  user = FTP_LOGIN;
	if ( passwd.length() <= 0 )
	  passwd = FTP_PASSWD;
	user.detach();
	passwd.detach();

	if ( !lockFTP( du.host(), 21, user.data(), passwd.data() ) )
	    return;

	FILE * in = fopen( su.path(), "rb" );
	if ( in == 0L )
	{
	    printf("ERROR: CouldNotRead\n");
	    ipc->fatalError( KIO_ERROR_CouldNotRead, su.path(), errno );
	    return;
	}

	struct stat buff;
	stat( su.path(), &buff );
	int size = buff.st_size;
	
	int erg = ftpPut( in, du.path(), 'I', ipc, size );
	
	fclose( in );

	if ( !erg )
	{
	    printf("ERROR: CouldNotWrite\n");
	    ipc->fatalError( KIO_ERROR_CouldNotWrite, _dest_url, errno );
	    return;
	}
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

bool KIOSlave::lockFTP( const char *_host, int _port, const char* _login, const char *_passwd )
{
    if ( strcmp( _host, lockedFTPHost.data() ) == 0 && _port == lockedFTPPort &&
	 strcmp( _login, lockedFTPLogin.data() ) == 0 && strcmp( _passwd, lockedFTPPasswd.data() ) == 0 )
	return TRUE;
    
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
	printf("ERROR: Could not connect\n");
	ipc->fatalError( KIO_ERROR_CouldNotConnect, _host, 0 );
	return FALSE;
    }
    
    printf("Login\n");
	
    if ( !ftpLogin( lockedFTPLogin.data(), lockedFTPPasswd.data() ) )
    {
	printf("ERROR: Could not login\n");
	QString err;
	err.sprintf( "ftp://%s:%s@%s", _login, _passwd, _host );
	ipc->fatalError( KIO_ERROR_CouldNotLogin, err.data(), 0 );
	return FALSE;
    }

    return TRUE;
}

bool KIOSlave::unlockFTP()
{
    if ( lockedFTPHost.data() != 0L )
	if ( lockedFTPHost.data()[0] != 0 )
	{
	    ftpQuit();
	    lockedFTPHost = "";
	}

    return TRUE;
}

bool KIOSlave::lockTgz( const char *_name )
{
    if ( strcmp( _name, lockedTgzSource.data() ) == 0 )
	return TRUE;
    
    if ( !unlockTgz() )
	return FALSE;

    QString logFile;
    logFile.sprintf( "/tmp/gziplog%i", time( 0L ) );
	    
    QString cmd;
    lockedTgzSource = _name;
    lockedTgzSource.detach();
    lockedTgzModified = FALSE;
    lockedTgzTemp.sprintf( "tmp%i.tar", time( 0L ) );
    cmd.sprintf("gzip -d <%s 1>%s 2>%S", _name, lockedTgzTemp.data(), logFile.data() );
    system( cmd );

    QString err = testLogFile( logFile.data() );
    if ( err.isNull() )
	return TRUE;

    unlink( lockedTgzTemp.data() );
    unlink( logFile.data() );
    lockedTgzSource = "";
    QString err2;
    err2.sprintf( "%s\n\nError log:\n%s", _name, err.data() );	    
    ipc->fatalError( KIO_ERROR_GzipError, err2.data(), 0 );

    return FALSE;
}

bool KIOSlave::unlockTgz()
{
    if ( lockedTgzSource.data() == 0L )
	return TRUE;
    if ( lockedTgzSource.data()[0] == 0 )
	return TRUE;
    
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
	    return FALSE;
	    
	}
    }
    
    unlink( lockedTgzTemp );

    lockedTgzSource = "";

    return TRUE;
}

void KIOSlave::cleanUp()
{
    bool ok1 = TRUE;
    bool ok2 = TRUE;

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
    printf("GOT TERM\n");
    if ( _sig == SIGTERM )
	if ( slave != 0L )
	    slave->terminate();
    exit(0);
}

#include "main.moc"










