#include "kbind.h"
#include "kfmwin.h"
#include "root.h"
#include "kfmserver.h"
#include "xview.h"

#include <kapp.h>
#include <unistd.h>
#include <html.h>
#include <qmsgbox.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <dirent.h>

void autostart();
void testDir();

void testDir( const char *_name )
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir( _name );
  if ( dp == NULL )
  {
    QString m = _name;
    QMessageBox::message( "KFM Error", "Missing directory:\n" + m );
    exit(1);
  }
}

int main( int argc, char ** argv )
{
    KApplication a( argc, argv, "kfm" );

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
    d = getenv( "KDEDIR" );
    d += "/apps";
    testDir( d );
    d = getenv( "KDEDIR" );
    d += "/filetypes";
    testDir( d );
    d = getenv( "KDEDIR" );
    d += "/filetypes";
    testDir( d );
    d = getenv( "KDEDIR" );
    d += "/lib/pics";
    testDir( d );
    d = getenv( "KDEDIR" );
    d += "/lib/pics/toolbar";
    testDir( d );
    d = getenv( "KDEDIR" );
    d += "/lib/pics/wallpapers";
    testDir( d );

    KHTMLWidget::registerFormats();
    QImageIO::defineIOHandler( "XV", "^P7 332", 0, read_xv_file, 0L );
    
    printf("0. Init IPC\n");
    
    KFMServer ipc;
    
    QString file = QDir::homeDirPath();
    file += "/.kfm.run";
    
    FILE *f = fopen( file.data(), "wb" );
    if ( f == 0L )
    {
	printf("ERROR: Could not write PID file\n");
	exit(1);
    }
    fprintf( f, "%i\n%i\n", (int)getpid(),(int)ipc.getPort() );
    fclose( f );
    
    printf("1. Init KIOManager\n");

    KIOServer *server = new KIOServer();

    printf("2. Init FileTypes\n");
    
    KFileType::init();
    
    printf("3. Init Root widget\n");

    new KRootWidget();
    
    printf("4. Init window\n");
    
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
	printf("Opening window\n");
	KFileWindow *m = new KFileWindow( 0L, 0L, home.data() );
	m->show();
	printf("Opended\n");
    }
    
    QWidget w( 0L, "Main" );
    a.setMainWidget( &w );

    Window root = DefaultRootWindow( a.getDisplay() );
    Window win = w.winId();
    Atom atom = XInternAtom( a.getDisplay(), "DndRootWindow", False );    
    XChangeProperty( a.getDisplay(), root, atom, XA_STRING, 32,
		     PropModeReplace, (const unsigned char*)(&win), 1);
    printf("Root window = %x\n",(int)win);

	printf("Fetching RootWindow\n");
	
	unsigned char *Data;
	unsigned long Size;
        Atom    ActualType;
        int     ActualFormat;
        unsigned long RemainingBytes;
      
	printf("Call\n");

        XGetWindowProperty(a.getDisplay(),root,atom,
                           0L,4L,
                           FALSE,AnyPropertyType,
                           &ActualType,&ActualFormat,
                           &Size,&RemainingBytes,
                           &Data);

	printf("Called and Data is %x\n",Data);

	if ( Data != 0L )
	    win = *((Window*)Data);
	else
	    win = 0L;
	
	printf("root window is %x\n",(int)win);

    printf("5. running\n");

    bool as = TRUE;
    
    if ( argc > arg )
	if ( strcmp( argv[arg++], "-na" ) == 0 )    
	  as = FALSE;
    
    if ( as )
      autostart();
    
    return a.exec();
}






