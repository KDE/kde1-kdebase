#include <kurl.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <kapp.h>
#include <kfm.h>
#include <qstring.h>
#include <qmsgbox.h>

#include "main.h"

int kfmCounter = 0;

KFMExec::KFMExec( int argc, char **argv )
{
    for ( int i = 3; i <= argc; i++ )
    {
        // A local file, not an URL ?
	if ( *argv[ i - 1 ] == '/' )
	{
	    QString tmp( shellQuote( argv[ i - 1 ] ) );
	    files += tmp.data();
	}
	// It is an URL
	else
        {
	  KURL u( argv[ i - 1 ] );
	  if ( u.isMalformed() )
	    {
	      QString err;
	      err.sprintf( "%s\n\r%s", argv[ i - 1 ], klocale->translate( "is malformed" ) );
	      QMessageBox::message( klocale->translate( "KFM Error" ), err );
	    }
	  // Must KFM fetch the file ?
	  else if ( strcmp( u.protocol(), "file" ) != 0 )
	    {
	      KFM *kfm = new KFM;
	      connect( kfm, SIGNAL( finished() ), this, SLOT( slotFinished() ) );
	      
	      QString tmp;
	      tmp.sprintf( "%s/.kde/kfm/tmp/%s.%i.%i", getenv( "HOME" ), u.filename(), getpid(), kfmCounter++ );
	      kfm->copy( argv[ i - 1 ], tmp );
	      if ( !files.isEmpty() )
		files += " ";
	      files += "\"";
	      files += tmp;
	      files += "\"";
	      fileList.append( tmp );
	      urlList.append( argv[ i - 1 ] );
	    }
	  else
	    // It is a local file system URL
	    {
	      QString tmp1( u.path() );
	      KURL::decodeURL( tmp1 );
	      QString tmp( shellQuote( tmp1 ) );
	      files += tmp.data();
	    }
	}
    }
    
    command = argv[ 1 ];
    
    expectedCounter = argc - 2;
    counter = 0L;
}

void KFMExec::slotFinished()
{
    counter++;

    if ( counter == expectedCounter )
    {
	int i = 0;
	while ( ( i = command.find( "%f", i ) ) != -1 )
	{
	    command.replace( i, 2, files );
	    i += files.length();
	}
    }

    int* times = new int[ expectedCounter ];
    
    int i = 0;
    const char *s;
    for ( s = fileList.first(); s != 0L; s = fileList.next() )
    {
	struct stat buff;
	stat( s, &buff );
	times[i++] = buff.st_mtime;
    }

    printf("EXEC '%s'\n",command.data() );
    
    system( command );

    i = 0;
    const char *u;
    u = urlList.first();
    for ( s = fileList.first(); s != 0L; s = fileList.next() )
    {
	struct stat buff;
	stat( s, &buff );
	if ( times[i++] != buff.st_mtime )
	{
	    QString tmp;
	    tmp.sprintf( klocale->translate( "The file\n\r%s\n\rhas been modified.\n\rDo you want to save it?" ), s );
	    if ( QMessageBox::query( klocale->translate( "KFM Question" ), tmp ) )
	    {
		KFM *kfm = new KFM;
		printf("s='%s'\nu='%s'\n",s,u);
		kfm->moveClient( s, u );
	    }
	}
	else
	{
	    unlink( s );
	}
	u = urlList.next();
    }

    sleep( 10 );
    exit(1);
}

QString KFMExec::shellQuote( const char *_data )
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

int main( int argc, char **argv )
{
    KApplication app( argc, argv );

    if ( argc < 2 )
    {
	fprintf( stderr, "Syntax Error:\nkfmexec command [URLs ....]\n" );
	exit(1);
    }
    
    // KFMExec *exec = new KFMExec( argc, argv );
    KFMExec exec(argc, argv);

    app.exec();

    return 0;
}

#include "main.moc"
