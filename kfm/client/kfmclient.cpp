#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <qapp.h>
#include <qdir.h>
#include <qmsgbox.h>
#include <qstring.h>
#include "kfmclient_ipc.h"
#include "config-kfm.h"

char flag = 0;

int doIt( int argc, char **argv );

QString displayName()
{
    QString d( getenv( "DISPLAY" ) );
    int i = d.find( ':' );
    if ( i != -1 )
	d[i] = '_';
    
    return d;
}

int main( int argc, char **argv )
{
    if ( argc == 1 )
    {
	debugT("\nSyntax:\n");
	debugT("./kfmclient openURL                # Opens an dialog to ask you for the URL\n");
	debugT("./kfmclient openURL 'url'          # Opens a window showing 'url'. If such a window\n");
	debugT("                                   # exists, it is showed\n");
	debugT("                                   # 'url' may be \"trash:/\" to open the trash bin.\n");
	debugT("./kfmclient refreshDesktop         # Refreshes the desktop\n");
	debugT("./kfmclient refreshDirectory 'url' # Tells KFM that an URL has changes. If KFM\n");
	debugT("                                   # is displaying that URL, it will be reloaded.\n");
	debugT("./kfmclient openProperties 'url'   # Opens a properties menu\n");
	debugT("./kfmclient exec 'url' ['binding'] # Tries to execute 'url'. 'url' may be a usual URL, this\n");
	debugT("  URL will be opened. You may omit 'binding'. In this case the default binding is tried.\n");
	debugT("  Of course URL may be the URL of a document, or it may be a *.kdelnk file. This way you\n");
	debugT("  could for example mount a device by passing 'Mount default' as binding to 'cdrom.kdelnk'\n");
	debugT("  Examples: ./kfmclient exec file:/usr/local/kde/bin/kdehelp Open              // Starts kdehelp\n");
	debugT("            ./kfmclient exec file:/root/Desktop/cdrom.kdelnk \"Mount default\" // Mounts the CDROM\n");	
	debugT("            ./kfmclient exec file:/home/weis/data/test.html    // Opens the file with default binding\n");
	debugT("            ./kfmclient exec file:/home/weis/data/test.html Netscape  // Opens the file with netscape\n");
	debugT("            ./kfmclient exec ftp://localhost/ Open             // Opens new window with URL\n");
	debugT("            ./kfmclient exec file:/root/Desktop/emacs.kdelnk   // Starts emacs\n");
	debugT("            ./kfmclient exec file:/root/Desktop/cdrom.kdelnk   // Opens the mount directory\n");
	debugT("./kfmclient move 'src' 'dest'  # Copies the URL 'src' to 'dest'. 'src' may be a list of URLs.\n");
	debugT("                               # 'dest' may be \"trash:/\" to move the files in the trash bin.\n");
	debugT("./kfmclient folder 'src' 'dest'  # Like move if 'src' is given, otherwise like openURL dest \n");
	debugT("./kfmclient sortDesktop          # Rearranges all icons on the desktop.\n");
	debugT("\n(c) Torben Weis, 1997\nPart of the KDE Project\n\n");
	return 0;
    }
    
    QApplication a( argc, argv );

    return doIt( argc, argv );
}

int doIt( int argc, char **argv )
{
    QString file = QDir::homeDirPath();
    file += "/.kde/share/apps/kfm/pid";
    file += displayName();
    
    FILE *f = fopen( file.data(), "rb" );
    if ( f == 0L )
    {
	if ( flag == 0 )
	{
	    debugT("!!!!!!! Running new KFM !!!!!!!!!!!!!!\n");
	    system( "kfm &" );
	    flag = 1;
	    sleep( 5 );
	    return doIt( argc, argv );
	}
	
	debugT("ERROR: KFM is not running\n");
	exit(1);
    }
    
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, f );
    int pid = atoi( buffer );
    if ( pid <= 0 )
    {
	debugT("ERROR: Invalid PID\n");
	exit(1);
    }

    if ( kill( pid, 0 ) != 0 )
    {
	if ( flag == 0 )
	{
	    flag = 1;
	    debugT("!!!!!!! Running new KFM !!!!!!!!!!!!!!\n");
	    system( "kfm &" );
	    sleep( 5 );
	    return doIt( argc, argv );
	}

	debugT("ERROR: KFM crashed\n");
	exit(1);
    }

    buffer[0] = 0;
    fgets( buffer, 1023, f );
    int slot = atoi( buffer );
    if ( slot <= 0 )
    {
	debugT("ERROR: Invalid Slot\n");
	exit(1);
    }
    
    KfmIpc kfm( slot );

    // Read the password
    QString fn = getenv( "HOME" );
    fn += "/.kde/share/apps/kfm/magic";
    f = fopen( fn.data(), "rb" );
    if ( f == 0L )
    {
	QMessageBox::message( "KFM Error",
			      "You dont have the file ~/.kde/share/apps/kfm/magic\n\rCould not do Authorization" );
	exit(1);
    }
    char *p = fgets( buffer, 1023, f );
    fclose( f );
    if ( p == 0L )
    {
	QMessageBox::message( "KFM Error",
			      "The file ~/.kde/share/apps/kfm/magic is corrupted\n\rCould not do Authorization" );
	exit(1);
    }

    kfm.auth( buffer );
    
    if ( argc < 2 )
    {
	debugT( "Syntax Error: Too few arguments\n" );
	exit(1);
    }
    
    if ( strcmp( argv[1], "refreshDesktop" ) == 0 )
    {
	if ( argc != 2 )
	{
	    debugT( "Syntax Error: Too many arguments\n" );
	    exit(1);
	}
	kfm.refreshDesktop();
    }
    else if ( strcmp( argv[1], "sortDesktop" ) == 0 )
    {
	if ( argc != 2 )
	{
	    debugT( "Syntax Error: Too many arguments\n" );
	    exit(1);
	}
	kfm.sortDesktop();
    }
    else if ( strcmp( argv[1], "openURL" ) == 0 )
    {
	if ( argc == 2 )
	{
	    kfm.openURL( "" );
	}
	else if ( argc == 3 )
	{
	    kfm.openURL( argv[2] );
	}
	else
	{
	    debugT( "Syntax Error: Too many arguments\n" );
	    exit(1);
	}
    }
    else if ( strcmp( argv[1], "refreshDirectory" ) == 0 )
    {
	if ( argc == 2 )
	{
	    kfm.openURL( "" );
	}
	else if ( argc == 3 )
	{
	    kfm.refreshDirectory( argv[2] );
	}
	else
	{
	    debugT( "Syntax Error: Too many arguments\n" );
	    exit(1);
	}
    }
    else if ( strcmp( argv[1], "openProperties" ) == 0 )
    {
	if ( argc == 3 )
	{
	    kfm.openProperties( argv[2] );
	}
	else
	{
	    debugT( "Syntax Error: Too many arguments\n" );
	    exit(1);
	}
    }
    else if ( strcmp( argv[1], "exec" ) == 0 )
    {
	if ( argc == 3 )
	{
	    kfm.exec( argv[2], 0L );
	}
	else if ( argc == 4 )
	{
	    kfm.exec( argv[2], argv[3] );
	}
	else
	{
	    debugT( "Syntax Error: Too many/few arguments\n" );
	    exit(1);
	}
    }
    else if ( strcmp( argv[1], "move" ) == 0 )
    {
	if ( argc <= 3 )
	{
	    debugT( "Syntax Error: Too many/few arguments\n" );
	    exit(1);
	}
	QString src = "";
	int i = 2;
	while ( i <= argc - 2 )
	{
	    src += argv[i];
	    if ( i < argc - 2 )
		src += "\n";
	    i++;
	}
	
	kfm.moveClient( src.data(), argv[ argc - 1 ] );
    }
    else if ( strcmp( argv[1], "copy" ) == 0 )
    {
	if ( argc <= 3 )
	{
	    debugT( "Syntax Error: Too many/few arguments\n" );
	    exit(1);
	}
	QString src = "";
	int i = 2;
	while ( i <= argc - 2 )
	{
	    src += argv[i];
	    if ( i < argc - 2 )
		src += "\n";
	    i++;
	}
	
	kfm.copy( src.data(), argv[ argc - 1 ] );
    }
    else if ( strcmp( argv[1], "folder" ) == 0 )
    {
	if ( argc <=2 )
	{
	    debugT( "Syntax Error: Too many/few arguments\n" );
	    exit(1);
	}

	if (argc > 3) {
	  QString src = "";
	  int i = 2;
	  while ( i <= argc - 2 )
	    {
	      src += argv[i];
	    if ( i < argc - 2 )
	      src += "\n";
	    i++;
	    }
	
	  kfm.moveClient( src.data(), argv[ argc - 1 ] );
	}
	else
	{
	    kfm.openURL( argv[2] );
	}
    }
    else if ( strcmp( argv[1], "selectRootIcons" ) == 0 )
    {
	if ( argc == 7 )
	{
	  int x = atoi( argv[2] );
	  int y = atoi( argv[3] );	  
	  int w = atoi( argv[4] );
	  int h = atoi( argv[5] );
	  int add = atoi( argv[6] );
	  bool bAdd = (bool)add;
	  kfm.selectRootIcons( x, y, w, h, bAdd );
	}
	else
	{
	    debugT( "Syntax Error: Too many arguments\n" );
	    exit(1);
	}
    }
    else
    {
	debugT("Synatx Error: Unknown command '%s'\n",argv[1] );
	exit(1);
    }
    return 0; // Stephan: default return
}




