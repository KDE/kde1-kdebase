#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <qapp.h>
#include <qdir.h>
#include <qmsgbox.h>
#include <qstring.h>
#include <kfm.h>
#include <kfmclient_ipc.h>
#include "config-kfm.h"

char flag = 0;

int doIt( int argc, char **argv );

int main( int argc, char **argv )
{
    if ( argc == 1 )
    {
        // Should all the following be i18n'ed ?
	printf("kfmclient1 is (c) Torben Weis, 1997\nPart of the KDE Project\n\n");
	printf("Syntax:\n");
	printf("  kfmclient1 openURL\n"
	       "            # Opens a dialog to ask you for the URL\n\n");
	printf("  kfmclient1 openURL 'url'\n"
	       "            # Opens a window showing 'url'. If such a window\n");
	printf("            #   exists, it is shown. 'url' may be \"trash:/\"\n"
	       "            #   to open the trash bin.\n\n");
	printf("  kfmclient1 refreshDesktop\n"
	       "            # Refreshes the desktop\n\n");
	printf("  kfmclient1 refreshDirectory 'url'\n"
	       "            # Tells KFM that an URL has changes. If KFM\n");
	printf("            #   is displaying that URL, it will be reloaded.\n\n");
	printf("  kfmclient1 openProperties 'url'\n"
	       "            # Opens a properties menu\n\n");
	printf("  kfmclient1 exec 'url' ['binding']\n"
	       "            # Tries to execute 'url'. 'url' may be a usual\n"
	       "            #   URL, this URL will be opened. You may omit\n"
	       "            #   'binding'. In this case the default binding\n");
	printf("            #   is tried. Of course URL may be the URL of a\n"
	       "            #   document, or it may be a *.kdelnk file.\n");
	printf("            #   This way you could for example mount a device\n"
	       "            #   by passing 'Mount default' as binding to \n"
	       "            #   'cdrom.kdelnk'\n\n");
	printf("  kfmclient1 move 'src' 'dest'\n"
	       "            # Copies the URL 'src' to 'dest'.\n"
	       "            #   'src' may be a list of URLs.\n");
	printf("            #   'dest' may be \"trash:/\" to move the files\n"
	       "            #   in the trash bin.\n\n");
	printf("  kfmclient1 folder 'src' 'dest'\n"
	       "            # Like move if 'src' is given,\n"
	       "            #   otherwise like openURL src \n\n");
	printf("  kfmclient1 sortDesktop\n"
	       "            # Rearranges all icons on the desktop.\n\n");
	printf("  kfmclient1 configure\n"
	       "            # Re-read KFM's configuration.\n\n");
	printf("*** Examples:\n"
	       "  kfmclient1 exec file:/usr/local/kde/bin/kdehelp Open\n"
	       "             // Starts kdehelp\n\n");
	printf("  kfmclient1 exec file:/root/Desktop/cdrom.kdelnk \"Mount default\"\n"
	       "             // Mounts the CDROM\n\n");	
	printf("  kfmclient1 exec file:/home/weis/data/test.html\n"
	       "             // Opens the file with default binding\n\n");
	printf("  kfmclient1 exec file:/home/weis/data/test.html Netscape\n"
	       "             // Opens the file with netscape\n\n");
	printf("  kfmclient1 exec ftp://localhost/ Open\n"
	       "             // Opens new window with URL\n\n");
	printf("  kfmclient1 exec file:/root/Desktop/emacs.kdelnk\n"
	       "             // Starts emacs\n\n");
	printf("  kfmclient1 exec file:/root/Desktop/cdrom.kdelnk\n"
	       "             // Opens the CD-ROM's mount directory\n\n");
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
	printf("No PID file called '%s'\n",file.data());
	if ( flag == 0 )
	{
	    printf("No PID file !!!!!!! Running new KFM !!!!!!!!!!!!!!\n");
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
	    printf("KFM seems to be crashed !!!!!!! Running new KFM !!!!!!!!!!!!!!\n");
	    system( "kfm &" );
	    sleep( 5 );
	    return doIt( argc, argv );
	}

	printf("ERROR: KFM crashed\n");
	exit(1);
    }

    buffer[0] = 0;
    fscanf( f, "%s", buffer );
    char * slot = strdup( buffer ); 
    if ( slot == (void *)0 )
    {
	printf("ERROR: Invalid Slot\n");
	exit(1);
    }
    
    KfmIpc kfm( slot );
    free( slot );

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
    else if ( strcmp( argv[1], "sortDesktop" ) == 0 )
    {
	if ( argc != 2 )
	{
	    printf( "Syntax Error: Too many arguments\n" );
	    exit(1);
	}
	kfm.sortDesktop();
    }
    else if ( strcmp( argv[1], "configure" ) == 0 )
    {
	if ( argc != 2 )
	{
	    printf( "Syntax Error: Too many arguments\n" );
	    exit(1);
	}
	kfm.configure();
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
	    printf( "Syntax Error: Too many/few arguments\n" );
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
    else if ( strcmp( argv[1], "copy" ) == 0 )
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
	
	kfm.copy( src.data(), argv[ argc - 1 ] );
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
	    printf( "Syntax Error: Too many/few arguments\n" );
	    exit(1);
	}
    }
    else
    {
	printf("Syntax Error: Unknown command '%s'\n",argv[1] );
	exit(1);
    }
    return 0; // Stephan: default return
}




