#include "kbind.h"
#include "kfmgui.h"
#include "root.h"
#include "kfmserver.h"
#include "xview.h"
#include <config-kfm.h>

#include <kapp.h>
#include <unistd.h>
#include <html.h>
#include <qmsgbox.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>

void autostart();
void testDir();
void sig_handler( int signum );

void testDir( const char *_name )
{
  DIR *dp;
  dp = opendir( _name );
  if ( dp == NULL )
  {
    QString m = _name;
    QMessageBox::message( "KFM Information", "Creating directory:\n" + m );
    ::mkdir( _name, S_IRWXU );
  }
  else
    closedir( dp );
}

int main( int argc, char ** argv )
{
    // Test for config file
    QString c = getenv( "HOME" );
    c += "/.kde";
    DIR *dp;
    dp = opendir( c.data() );
    if ( dp == NULL )
	::mkdir( c.data(), S_IRWXU );
    else
    {
	debugT("Exist '%s'\n", c.data() );
	closedir( dp );
    }
    
    c = getenv( "HOME" );
    c += "/.kde/config";
    dp = opendir( c.data() );
    if ( dp == NULL )
	::mkdir( c.data(), S_IRWXU );
    else
    {
	debugT("Exist '%s'\n", c.data() );
	closedir( dp );
    }
    
    c = getenv( "HOME" );
    c += "/.kde/config/kfmrc";
    FILE *f2 = fopen( c.data(), "rb" );
    if ( f2 == 0L )
    {
	QString cmd;
	cmd.sprintf( "cp %s/lib/kfm/config/kfmrc %s/.kde/config/kfmrc", kapp->kdedir().data(), getenv( "HOME" ) );
	system( cmd.data() );
    }
    else
	fclose( f2 );

    // Test for kfm directories
    c = getenv( "HOME" );
    c += "/.kfm";
    dp = opendir( c.data() );
    if ( dp == NULL )
	::mkdir( c.data(), S_IRWXU );
    else
    {
	debugT("Exist '%s'\n", c.data() );
	closedir( dp );
    }
    
    c = getenv( "HOME" );
    c += "/.kfm/cache";
    dp = opendir( c.data() );
    if ( dp == NULL )
	::mkdir( c.data(), S_IRWXU );
    else
    {
	debugT("Exist '%s'\n", c.data() );
	closedir( dp );
    }

    c = getenv( "HOME" );
    c += "/.kfm/tmp";
    dp = opendir( c.data() );
    if ( dp == NULL )
	::mkdir( c.data(), S_IRWXU );
    else
    {
	debugT("Exist '%s'\n", c.data() );
	closedir( dp );
    }

    c = getenv( "HOME" );
    c += "/.desktop";
    f2 = fopen( c.data(), "rb" );
    if ( f2 == 0L )
    {
	QString cmd;
	cmd.sprintf( "cp %s/lib/kfm/config/desktop %s/.desktop", kapp->kdedir().data(), getenv( "HOME" ) );
	system( cmd.data() );
    }
    else
	fclose( f2 );

    // Test for existing Templates
    bool bTemplates = TRUE;
        c = getenv( "HOME" );
    c += "/Desktop/Templates";
    dp = opendir( c.data() );
    if ( dp == NULL )
	bTemplates = FALSE;
    else
      closedir( dp );

    KApplication a( argc, argv, "kfm" );

    signal(SIGCHLD,sig_handler);

    // Test for directories
    QString d = getenv( "HOME" );
    d += "/Desktop";
    testDir( d );
    d = getenv( "HOME" );
    d += "/Desktop/Trash";
    testDir( d );
    d = getenv( "HOME" );
    d += "/Desktop/Templates";
    testDir( d );
    d = getenv( "HOME" );
    d += "/Desktop/Autostart";
    testDir( d );
    QString kd = kapp->kdedir();
    d = kd.copy();
    d += "/apps";
    testDir( d );
    d = kd.copy();
    d += "/mimetypes";
    testDir( d );
    d = kd.copy();
    d += "/lib/pics";
    testDir( d );
    d = kd.copy();
    d += "/lib/pics/toolbar";
    testDir( d );
    d = kd.copy();
    d += "/lib/pics/wallpapers";
    testDir( d );

    if ( !bTemplates )
    {
	QMessageBox::message( "KFM Information", "Installing Templates" );
	QString cmd;
	cmd.sprintf("cp %s/lib/kfm/Desktop/Templates/* %s/Desktop/Templates", kapp->kdedir().data(), getenv( "HOME" ) );
	system( cmd.data() );
    }
    
    KHTMLWidget::registerFormats();
    QImageIO::defineIOHandler( "XV", "^P7 332", 0, read_xv_file, 0L );
    
    debugT("0. Init IPC\n");
    
    KFMServer ipc;
    
    QString file = QDir::homeDirPath();
    file += "/.kfm.run";
    
    FILE *f = fopen( file.data(), "wb" );
    if ( f == 0L )
    {
	debugT("ERROR: Could not write PID file\n");
	exit(1);
    }
    fprintf( f, "%i\n%i\n", (int)getpid(),(int)ipc.getPort() );
    fclose( f );
    
    debugT("1. Init KIOManager\n");

    KIOServer *server = new KIOServer();

    debugT("2. Init FileTypes\n");
    
    KMimeType::init();
    
    debugT("3. Init Root widget\n");

    new KRootWidget();
    
    debugT("4. Init window\n");
    
    bool openwin = TRUE;

    int arg = 1;
    
    if ( argc > arg )
	if ( strcmp( argv[arg++], "-d" ) == 0 )
	    openwin = FALSE;
    
    if ( openwin )
    {
	QString home = "file:";
	home.detach();
	home += QDir::homeDirPath().data();
	debugT("Opening window\n");
	KfmGui *m = new KfmGui( 0L, 0L, home.data() );
	m->show();
	debugT("Opended\n");
    }
    
    QWidget w( 0L, "Main" );
    a.setMainWidget( &w );

    Window root = DefaultRootWindow( a.getDisplay() );
    Window win = w.winId();
    Atom atom = XInternAtom( a.getDisplay(), "DndRootWindow", False );    
    XChangeProperty( a.getDisplay(), root, atom, XA_STRING, 32,
		     PropModeReplace, (const unsigned char*)(&win), 1);
    debugT("Root window = %x\n",(int)win);

	debugT("Fetching RootWindow\n");
	
	unsigned char *Data;
	unsigned long Size;
        Atom    ActualType;
        int     ActualFormat;
        unsigned long RemainingBytes;
      
	debugT("Call\n");

        XGetWindowProperty(a.getDisplay(),root,atom,
                           0L,4L,
                           FALSE,AnyPropertyType,
                           &ActualType,&ActualFormat,
                           &Size,&RemainingBytes,
                           &Data);

	debugT("Called and Data is %x\n",*Data);

	if ( Data != 0L )
	    win = *((Window*)Data);
	else
	    win = 0L;
	
	debugT("root window is %x\n",(int)win);

    debugT("5. running\n");

    bool as = TRUE;
    
    if ( argc > arg )
	if ( strcmp( argv[arg++], "-na" ) == 0 )    
	  as = FALSE;
    
    if ( as )
      autostart();

    debugT("OOOOOOOOOOOOOOOOOO Display size %i %i\n",
	   XDisplayWidth( a.getDisplay(), 0 ), XDisplayHeight( a.getDisplay(), 0 ) );
    
    return a.exec();
}

void sig_handler( int )
{
    int pid;
    int status;
    
    while( 1 )
    {
	debugT("SIGNAL HANDLER called\n");
	
	pid = waitpid( -1, &status, WNOHANG );
	if ( pid <= 0 )
	{
	    // Reinstall signal handler, since Linux resets to default after
	    // the signal occured ( BSD handles it different, but it should do
	    // no harm ).
	    signal(SIGCHLD,sig_handler);
	    return;
	}
    }
}
