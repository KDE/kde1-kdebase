#include "icon.h"
#include "kio_errors.h"
#include "xview.h"
#include "time.h"
#include "jpeg.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

int registered = FALSE;

KProtocolICON::KProtocolICON()
{
    bDeleteFile = FALSE;
    f = 0L;
}

KProtocolICON::~KProtocolICON()
{
}

int KProtocolICON::Open(KURL * u, int mode)
{
    // We dont care about the URL, since icon is just a filter
    if( mode != READ )
    {
	return Error( KIO_ERROR_NotImplemented, "This is not implemented..." );
    }
    
    if ( access( u->path(), R_OK ) < 0 )
    {
	Error( KIO_ERROR_CouldNotRead, "Can't open File for reading", errno );
	return FAIL;
    }

    if ( !registered )
    {
	registered = TRUE;
	QImageIO::defineIOHandler( "XV", "^P7 332", 0, read_xv_file, 0L );
	QImageIO::defineIOHandler("JFIF","^\377\330\377\340..JFIF", 0, read_jpeg_jfif, NULL);                              
    }

    // Path for .xvpics
    QString xv = u->directory();
    if ( xv.right(1) != "/" )
	xv += "/.xvpics";
    else
	xv += ".xvpics";
    
    DIR *dp = opendir( xv.data() );
    if ( dp != NULL )
	closedir( dp );
    else
    {
	// Create directory
	if ( ::mkdir( xv.data(), S_IRWXU ) == -1 )
	    xv = "";
    }
    if ( !xv.isEmpty() )
	xv += "/";

    // Assume XV pic is not available
    bool is_avail = FALSE;
    // Assume that the xv pic has size 0
    bool is_null = TRUE;
		    
    // Time of the original image
    struct stat buff;
    lstat( u->path(), &buff );
    time_t t1 = buff.st_mtime;
    if ( buff.st_size != 0 )
	is_null = FALSE;
    // Get the times of the xv pic
    if ( !xv.isEmpty() )
    {
	xvfile = xv.data();
	xvfile += u->filename();
	// Is the XV pic available ?
	if ( lstat( xvfile, &buff ) == 0 )
	{
	    time_t t2 = buff.st_mtime;
	    // Is it outdated ?
	    if ( t1 <= t2 )
		is_avail = TRUE;
	}
    }
    
    // Create the pic if it does not exist
    if ( !is_avail )
    {
	QPixmap pixmap;
	pixmap.load( u->path() );
	if ( !pixmap.isNull() && !xv.isEmpty() && access( xv, W_OK|R_OK|X_OK ) >= 0 )
	{
	  printf("LOADED image %s\n", u->path() );
	  write_xv_file( xvfile, pixmap );
	  is_avail = TRUE;
	}
	else if ( !pixmap.isNull() )
	{
	  printf("LOADED 2 image %s\n", u->path() );
	  bDeleteFile = TRUE;
	  xvfile.sprintf( "%s/.kde/share/apps/kfm/tmp/%s.%i.%i", getenv( "HOME" ), u->filename(), (int)time( 0L ), (int)getpid() );
	  is_avail = TRUE;
	  write_xv_file( xvfile, pixmap );
	}
	else
	  fprintf( stderr, "COULD not load IMAGE %s\n", u->path() );
    }
    		    
    // Test wether it is really an image
    if ( is_avail && !is_null )
    {
	f = fopen( xvfile, "rb" );
	if ( f != 0L )
	{
	    char str4[ 1024 ];
	    fgets( str4, 1024, f );
	    if ( strncmp( "P7 332", str4, 6 ) != 0 )
		is_null = TRUE;
	    // Skip line
	    fgets( str4, 1024, f );
	    fgets( str4, 1024, f );
	    if ( strncmp( "#BUILTIN:UNKNOWN", str4, 16 ) == 0 )
		is_null = TRUE;
	    fclose( f );
	    if ( !is_null )
	      f = fopen( xvfile, "rb" );
	    else
	      f = 0L;
	}
    }

    return SUCCESS;
}

long KProtocolICON::Read(void *buffer, long len)
{
    if ( f != 0L )
    {
	long n = fread( buffer, 1, len, f );
	if( n < 0 )
	{
	    Error( KIO_ERROR_CouldNotRead, "Reading from file failed", errno );
	    return FAIL;
	}
	return n;
    }

    return 0;
}

int KProtocolICON::Close()
{
    if ( f != 0L )
	fclose( f );

    // Delete temporary file
    if ( !xvfile.isEmpty() && bDeleteFile )
      unlink( xvfile );
    
    return SUCCESS;
}

int KProtocolICON::atEOF()
{
    if ( f == 0L )
	return TRUE;
    
    return feof( f );
}

long KProtocolICON::Size()
{
    return 0x7fffffff;		// dunno ... ;)
}

int KProtocolICON::OpenDir( KURL * )
{
    // This is not really an error. The protocol can not support it
    // since the operation is technical impossible
    return ( Error( KIO_ERROR_NotPossible, "This is not possible...") ); 
}

#include "icon.moc"




