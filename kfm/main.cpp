#include <qdir.h>

#include "kbind.h"
#include "kfmgui.h"
#include "root.h"
#include "kfmserver.h"
#include "kfmpaths.h"
#include "kmimemagic.h"
#include "kfm.h"
#include "utils.h"
#include "config-kfm.h"

// needed for InitStatic call:
#include "htmlcache.h"
#include "kfmview.h"
#include "kiojob.h"

#include <kapp.h>
#include <unistd.h>
#include <html.h>
#include <qmsgbox.h>
#include <qtimer.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

void autostart();
void testDir( const char* );
void testDir2( const char* );
void testDir3( const char* );

void sig_handler( int signum );
void sig_term_handler( int signum );

#include <klocale.h>

void testDir3( const char *_name )
{
    DIR *dp;
    dp = opendir( _name );
    if ( dp == NULL )
	::mkdir( _name, S_IRWXU );
    else
	closedir( dp );
}

void testDir2( const char *_name )
{
    DIR *dp;
    QString c = kapp->localkdedir().data();
    c += _name;
    dp = opendir( c.data() );
    if ( dp == NULL )
	::mkdir( c.data(), S_IRWXU );
    else
	closedir( dp );
}

void testDir( const char *_name )
{
  DIR *dp;
  dp = opendir( _name );
  if ( dp == NULL )
  {
    QString m = _name;
    if ( m.right(1) == "/" )
      m.truncate( m.length() - 1 );
    
    QMessageBox::information( 0, klocale->translate("KFM Information"), 
			  klocale->translate("Creating directory:\n") + m );
    ::mkdir( m, S_IRWXU );
  }
  else
    closedir( dp );
}


void copyDirectoryFile(const char *filename, const QString& dir)
{
    if (!QFile::exists(dir + "/.directory")) {
	QString cmd;
	cmd.sprintf( "cp %s/kfm/%s %s/.directory", kapp->kde_datadir().data(), 
		     filename, dir.data() );
	system( cmd.data() );
    }
}


void InitStaticMembers()
{
    /*
     * hack to get static classes up and running even with C++-Compilers
     * Systems where a constructor of a class element declared static
     * would never get called (Aix, Alpha,...). In the next versions these
     * elements should disappear.
     */
    KMimeBind::InitStatic();
    KMimeType::InitStatic();
    HTMLCache::InitStatic();
    KfmGui::InitStatic();
    KfmView::InitStatic();
    KIOJob::InitStatic();
}

int main( int argc, char ** argv )
{
    KApplication a( argc, argv, "kfm" );

    testDir2( "" );
    testDir2( "/share" );    
    testDir2( "/share/config" );
    testDir2( "/share/apps" );
    testDir2( "/share/apps/kfm" );
    testDir2( "/share/apps/kfm/tmp" );
    testDir2( "/share/apps/kfm/bookmarks" );
    testDir2( "/share/icons" );
    testDir2( "/share/icons/mini" );
    testDir2( "/share/applnk" );
    testDir2( "/share/mimelnk" );

    QString c;
    // Clean this directory
    c.sprintf("rm -f %s/share/apps/kfm/tmp/*", kapp->localkdedir().data() );
    system( c );

    FILE *f2;
    c = kapp->localkdedir().data();
    c += "/share/apps/kfm/desktop";
    f2 = fopen( c.data(), "rb" );
    if ( f2 == 0L )
    {
	QString cmd;
	cmd.sprintf( "cp %s/desktop %s/share/apps/kfm/desktop", kapp->kde_configdir().data(), kapp->localkdedir().data() );
	system( cmd.data() );
    }
    else
	fclose( f2 );
    
    // Stephan: I must find a better place for this somewhen
    KFMPaths::initPaths();

    testDir3( KFMPaths::CachePath() );

    //Stephan: This variable is not deleted here, but in the 
    // slotQuit methode of KFileWindow. I'm not that sure, if
    // this is the best way! 
    (void)new KIOServer();

    InitStaticMembers();

    // Test for existing Templates
    bool bTemplates = true;

    DIR* dp = opendir( KFMPaths::TemplatesPath().data() );
    if ( dp == NULL )
	bTemplates = false;
    else
      closedir( dp );

    signal(SIGCHLD,sig_handler);
    
    // Test for directories
    testDir( KFMPaths::DesktopPath() );
    testDir( KFMPaths::TrashPath() );
    copyDirectoryFile("directory.trash", KFMPaths::TrashPath());
    testDir( KFMPaths::TemplatesPath() );
    copyDirectoryFile("directory.templates", KFMPaths::TemplatesPath());
    testDir( KFMPaths::AutostartPath() );
    copyDirectoryFile("directory.autostart", KFMPaths::AutostartPath());

    testDir( kapp->kde_appsdir() );
    testDir( kapp->kde_mimedir() );

    if ( !bTemplates )
    {
	QMessageBox::information( 0, klocale->translate("KFM Information"),
			      klocale->translate("Installing Templates") );
	QString cmd;
	cmd.sprintf("cp %s/kfm/Desktop/Templates/* %s", 
		    kapp->kde_datadir().data(), KFMPaths::TemplatesPath().data() );
	system( cmd.data() );
    }

    // Initialize the KMimeMagic stuff
    KMimeType::initKMimeMagic();
    
    KHTMLWidget::registerFormats();
    
    KFMServer ipc;
    
    QString file = kapp->localkdedir().data();
    file += "/share/apps/kfm/pid";
    file += displayName();
    
    FILE *f = fopen( file.data(), "wb" );
    if ( f == 0L )
    {
	debugT("ERROR: Could not write PID file\n");
	exit(1);
    }
    // Keep in sync with the same in kfmserver.cpp!
    QString idir;
    idir.sprintf(_PATH_TMP"kfm_%i_%i%s\n",(int)getuid(),(int)getpid(),displayName().data());
    fprintf( f, "%i\n%s\n", (int)getpid(),idir.data() );
    fclose( f );

    // Stephan: alias some translated string to find them faster
    klocale->aliasLocale("Open", ID_STRING_OPEN);
    klocale->aliasLocale("Cd", ID_STRING_CD);
    klocale->aliasLocale("New View", ID_STRING_NEW_VIEW);
    klocale->aliasLocale("Copy", ID_STRING_COPY);
    klocale->aliasLocale("Delete", ID_STRING_DELETE);
    klocale->aliasLocale("Move to Trash", ID_STRING_MOVE_TO_TRASH);
    klocale->aliasLocale("Paste", ID_STRING_PASTE);
    klocale->aliasLocale("Open with", ID_STRING_OPEN_WITH);
    klocale->aliasLocale("Cut", ID_STRING_CUT);
    klocale->aliasLocale("Move", ID_STRING_MOVE);
    klocale->aliasLocale("Properties", ID_STRING_PROP);
    klocale->aliasLocale("Link", ID_STRING_LINK);
    klocale->aliasLocale("Empty Trash Bin", ID_STRING_TRASH);
    klocale->aliasLocale("Add to Bookmarks", ID_STRING_ADD_TO_BOOMARKS );
    
    int arg = 1;
    
    if ( argc > arg )
	if ( argv[arg][0] == '-' )
	{
	    if ( strchr( argv[arg], 'w' ) != 0L )
		KfmGui::rooticons = false;
	    if ( strchr( argv[arg], 's' ) != 0L )
		KfmGui::sumode = true;
	}

    KMimeType::init();
    
    Kfm kfm;
    a.setMainWidget( &kfm );

    if ( KfmGui::rooticons )
	(void)new KRootWidget();
    
    if ( KfmGui::rooticons == false )
    {
	QString home = "file:";
	home.detach();
	home += QDir::homeDirPath().data();
	KfmGui *m = new KfmGui( 0L, 0L, home.data() );
	m->show();
    }
    
    Window root = DefaultRootWindow( a.getDisplay() );
    Window win = kfm.winId();
    Atom atom = XInternAtom( a.getDisplay(), "DndRootWindow", False );    
    XChangeProperty( a.getDisplay(), root, atom, XA_STRING, 32,
		     PropModeReplace, (const unsigned char*)(&win), 1);
    
    unsigned char *Data;
    unsigned long Size;
    Atom    ActualType;
    int     ActualFormat;
    unsigned long RemainingBytes;
    
    XGetWindowProperty(a.getDisplay(),root,atom,
		       0L,4L,
		       false,AnyPropertyType,
		       &ActualType,&ActualFormat,
		       &Size,&RemainingBytes,
		       &Data);

    if ( Data != 0L )
      win = *((Window*)Data);
    else
      win = 0L;
	
    bool as = true;
    
    if ( argc > arg )
      if ( strcmp( argv[arg++], "-na" ) == 0 )    
	as = false;
    
    if ( as )
      autostart();
    
    QTimer timer;
    if ( argc < 2 || strcmp( argv[1], "-debug" ) != 0L )
    {
      signal(SIGTERM,sig_term_handler);
      // Install SIGSEGV handler only if KFM worked for at least one minute.
      // This hopefully avoids endless loops of crashes and restarts.
      QObject::connect( &timer, SIGNAL( timeout() ), pkfm, SLOT( slotInstallSegfaultHandler() ) );
      timer.start( 60000, true );
    }
    
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
	    signal( SIGCHLD, sig_handler );
	    return;
	}
    }
}

void sig_term_handler( int )
{
  printf("###################### TERM: Deleting sockets ###################\n");

  if ( pkfm->isGoingDown() )
    return;
  
  // Save cache and stuff and delete the sockets ...
  pkfm->slotSave();
  pkfm->slotShutDown();
  
  exit(1);
}
