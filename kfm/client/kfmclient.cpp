#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <qapp.h>
#include <qdir.h>

#include "kfmclient_ipc.h"

char flag = 0;

int doIt( int argc, char **argv );

int main( int argc, char **argv )
{
    if ( argc == 1 )
    {
	printf("\nSyntax:\n");
	printf("./kfmclient openURL                # Opens an dialog to ask you for the URL\n");
	printf("./kfmclient openURL 'url'          # Opens a window showing 'url'. If such a window\n");
	printf("                                   # exists, it is showed\n");
	printf("                                   # 'url' may be \"trash:/\" to open the trash bin.\n");
	printf("./kfmclient refreshDesktop         # Refreshes the desktop\n");
	printf("./kfmclient refreshDirectory 'url' # Tells KFM that an URL has changes. If KFM\n");
	printf("                                   # is displaying that URL, it will be reloaded.\n");
	printf("./kfmclient openProperties 'url'   # Opens a properties menu\n");
	printf("./kfmclient exec 'url' ['binding'] # Tries to execute 'url'. 'url' may be a usual URL, this\n");
	printf("  URL will be opened. You may omit 'binding'. In this case the default binding is tried.\n");
	printf("  Of course URL may be the URL of a document, or it may be a *.kdelnk file. This way you\n");
	printf("  could for example mount a device by passing 'Mount default' as binding to 'cdrom.kdelnk'\n");
	printf("  Examples: ./kfmclient exec file:/usr/local/kde/bin/kdehelp Open              // Starts kdehelp\n");
	printf("            ./kfmclient exec file:/root/Desktop/cdrom.kdelnk \"Mount default\" // Mounts the CDROM\n");	
	printf("            ./kfmclient exec file:/home/weis/data/test.html    // Opens the file with default binding\n");
	printf("            ./kfmclient exec file:/home/weis/data/test.html Netscape  // Opens the file with netscape\n");
	printf("            ./kfmclient exec ftp://localhost/ Open             // Opens new window with URL\n");
	printf("            ./kfmclient exec file:/root/Desktop/emacs.kdelnk   // Starts emacs\n");
	printf("            ./kfmclient exec file:/root/Desktop/cdrom.kdelnk   // Opens the mount directory\n");
	printf("./kfmclient move 'src' 'dest'  # Copies the URL 'src' to 'dest'. 'src' may be a list of URLs.\n");
	printf("                               # 'dest' may be \"trash:/\" to move the files in the trash bin.\n");
	printf("./kfmclient folder 'src' 'dest'  # Like move if 'src' is given, otherwise like openURL dest \n");
	printf("./kfmclient sortDesktop          # Rearranges all icons on the desktop.\n");
	printf("\n(c) Torben Weis, 1997\nPart of the KDE Project\n\n");
	return 0;
    }
    
    QApplication a( argc, argv );

    return doIt( argc, argv );
}

int doIt( int argc, char **argv )
{
    QString file = QDir::homeDirPath();
    file += "/.kfm.run";
    
    FILE *f = fopen( file.data(), "rb" );
    if ( f == 0L )
    {
	if ( flag == 0 )
	{
	    printf("!!!!!!! Running new KFM !!!!!!!!!!!!!!\n");
	    system( "kfm &" );
	    flag = 1;
	    sleep( 5 );
	    return doIt( argc, argv );
	}
	
	printf("ERROR: KFM is not running\n");
	exit(1);
    }
    
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, f );
    int pid = atoi( buffer );
    if ( pid <= 0 )
    {
	printf("ERROR: Invalid PID\n");
	exit(1);
    }

    if ( kill( pid, 0 ) != 0 )
    {
	if ( flag == 0 )
	{
	    flag = 1;
	    printf("!!!!!!! Running new KFM !!!!!!!!!!!!!!\n");
	    system( "kfm &" );
	    sleep( 5 );
	    return doIt( argc, argv );
	}

	printf("ERROR: KFM crashed\n");
	exit(1);
    }

    buffer[0] = 0;
    fgets( buffer, 1023, f );
    int slot = atoi( buffer );
    if ( slot <= 0 )
    {
	printf("ERROR: Invalid Slot\n");
	exit(1);
    }
    
    KfmIpc kfm( slot );
    
    if ( argc < 2 )
    {
	printf( "Syntax Error: Too few arguments\n" );
	exit(1);
    }
    
    if ( strcmp( argv[1], "refreshDesktop" ) == 0 )
    {
	if ( argc != 2 )
	{
	    printf( "Syntax Error: Too many arguments\n" );
	    exit(1);
	}
	kfm.refreshDesktop();
    }
    if ( strcmp( argv[1], "sortDesktop" ) == 0 )
    {
	if ( argc != 2 )
	{
	    printf( "Syntax Error: Too many arguments\n" );
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
	    printf( "Syntax Error: Too many arguments\n" );
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
	    printf( "Syntax Error: Too many arguments\n" );
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
	    printf( "Syntax Error: Too many arguments\n" );
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
	    printf( "Syntax Error: Too many/few arguments\n" );
	    exit(1);
	}
    }
    else if ( strcmp( argv[1], "move" ) == 0 )
    {
	if ( argc <= 3 )
	{
	    printf( "Syntax Error: Too many/few arguments\n" );
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
    else if ( strcmp( argv[1], "folder" ) == 0 )
    {
	if ( argc <=2 )
	{
	    printf( "Syntax Error: Too many/few arguments\n" );
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
	else {
	  kfm.openURL( argv[2] );
	}
    }
    else
    {
	printf("Synatx Error: Unknown command '%s'\n",argv[1] );
	exit(1);
    }
}




