#include <qdir.h>

#include "kurl.h"
#include "kfmgui.h"
#include "kfm.h"
#include "utils.h"

#include <qstrlist.h>
#include <kapp.h>
#include <kconfig.h>
#include <htmlcache.h>

#include <stdlib.h>
#include <unistd.h> 
#include <sys/signal.h>

Kfm* Kfm::pKfm = 0L;
QStrList* Kfm::pHistory = 0L;
bool Kfm::s_bGoingDown = false;

void sig_segv_handler( int signum );

Kfm::Kfm()
{
    pKfm = this;
    pHistory = new QStrList;
    
    kapp->setTopWidget( this );

    HTMLCache::load();
    
    if ( KfmGui::rooticons )
    {
	kapp->enableSessionManagement( TRUE );
	kapp->setWmCommand( "" );
	
	connect( kapp, SIGNAL( saveYourself() ), this, SLOT( slotSave() ) );
	connect( kapp, SIGNAL( shutDown() ), this, SLOT( slotShutDown() ) );

	KConfig *config = kapp->getConfig();
	config->setGroup( "SM" );
	bool flag = config->hasKey( "URLs" );
	
	QStrList urlList;
	int n = config->readListEntry( "URLs", urlList );
	
	if ( !flag && KfmGui::rooticons == true )
	{
	    QString home = "file:";
	    home.detach();
	    home += QDir::homeDirPath().data();
	    KfmGui *m = new KfmGui( 0L, 0L, home.data() );
	    m->show();
	}

	if ( flag )
	{
	    int i;
	    for ( i = 1; i <= n; i++ )
	    {
		KfmGui *m = new KfmGui( 0L, 0L, urlList.at( i - 1 ) );
		m->readProperties(i);
		m->show();
	    }
	}
    }

    pIconLoader = new KIconLoader();

    QStrList* list = pIconLoader->getDirList();
    list->clear();
    QString tmp = kapp->kde_icondir().copy();
    list->append( tmp.data() );
    tmp = KApplication::localkdedir();
    tmp += "/share/icons";
    list->append( tmp.data() );

    connect( &timer, SIGNAL( timeout() ), this, SLOT( slotTouch() ) );
    // Call every hour
    timer.start( 3600000 );
}

void Kfm::slotTouch()
{
  // Prevent the sockets from being removed by the cleanup daemon of some systems.
  QString tmp;
  tmp.sprintf("touch /tmp/kfm_%i_%i%s",(int)getuid(),(int)getpid(),displayName().data() );
  system( tmp.data() );
  tmp.sprintf("touch /tmp/kio_%i_%i%s",(int)getuid(),(int)getpid(),displayName().data() );
  system( tmp.data() );
}

Kfm::~Kfm()
{
}
    
void Kfm::slotSave()
{
  timer.stop();
  
  KConfig *config = kapp->getConfig();

  QStrList urlList;
  
  KfmGui *gui;
  int i = 0;
  for ( gui = KfmGui::getWindowList().first(); gui != 0L; gui = KfmGui::getWindowList().next() )
  {
    i++;
    gui->saveProperties(i);
    urlList.append( gui->getURL() );
  }
  
  config->setGroup( "SM" );
  config->writeEntry( "URLs", urlList );
  config->sync();
  
  HTMLCache::save();
}

void Kfm::slotShutDown()
{
  // Delete the sockets
  QString sock;
  sock.sprintf("/tmp/kio_%i_%i%s",(int)getuid(), (int)getpid(),displayName().data());
  unlink( sock );

  sock.sprintf("/tmp/kfm_%i_%i%s",(int)getuid(), (int)getpid(),displayName().data());
  unlink( sock );

  s_bGoingDown = true;
}

void Kfm::addToHistory( const char *_url )
{
  if ( pHistory->find( _url ) != -1 )
    return;
  
  if ( pHistory->count() == 100 )
    pHistory->removeRef( pHistory->first() );
  
  pHistory->append( _url );
}

bool Kfm::saveHTMLHistory( const char *_filename )
{
  FILE *f = fopen( _filename, "w" );
  if ( f == 0L )
  {
    warning( "Could not write to %s\n",_filename );
    return false;
  }
  
  fprintf( f, "<HTML><HEAD><TITLE>History</TITLE></HEAD><BODY><H1>History</H1>\n" );
  
  const char *p;
  for ( p = pHistory->first(); p != 0L; p = pHistory->next() )
    fprintf( f, "<a href=\"%s\">%s</a><br>\n", p, p );
  
  fprintf( f, "</HTML></BODY>\n" );
  
  fclose( f );
  
  return true;
}

void Kfm::slotInstallSegfaultHandler()
{
  printf("+++++++++++++++++++++ INSTALLING SIGSEGV handler +++++++++++++++++\n");
  
  signal( SIGSEGV, sig_segv_handler );
}

void sig_segv_handler( int )
{
  static bool forgetit = false;
  
  if ( forgetit )
    return;
  
  forgetit = true;
  
  printf("###################### SEGV ###################\n");
  
  QString kfm = kapp->kde_bindir().data();
  kfm += "/kfm &";                           // sven: run in background
  system (kfm.data());                       // sven: tell shell to do it.
  //execl( kfm.data(), 0L );
  exit (0);                                  // exit clean.
}

#include "kfm.moc"

