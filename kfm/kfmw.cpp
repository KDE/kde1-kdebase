#include <qdir.h>

#include "kurl.h"
#include "kfmgui.h"
#include "kfmw.h"
#include "utils.h"
#include "kcookiejar.h"

#include <qstrlist.h>
#include <kapp.h>
#include <kconfig.h>
#include <htmlcache.h>

#include <stdlib.h>
#include <unistd.h> 
#include <signal.h>
#include <sys/signal.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

Kfm* Kfm::pKfm = 0L;
QStrList* Kfm::pHistory = 0L;
bool Kfm::s_bGoingDown = false;
bool Kfm::bAllowURLProps = false;
bool Kfm::bTreeViewFollowMode = false;

void sig_segv_handler( int signum );

Kfm::Kfm()
{
    pKfm = this;
    pHistory = new QStrList;
    
    kapp->setTopWidget( this );

    HTMLCache::load();
    
    pIconLoader = new KIconLoader();

    // We need this in KfmGui::KfmGui(), so moved it here. DF.
    QStrList* list = pIconLoader->getDirList();
    list->clear();
    QString tmp = kapp->kde_icondir().copy();
    list->append( tmp.data() );
    tmp = KApplication::localkdedir();
    tmp += "/share/icons";
    list->append( tmp.data() );

    if ( KfmGui::rooticons )
    {
	kapp->enableSessionManagement( TRUE );
	kapp->setWmCommand( "" );
	
	connect( kapp, SIGNAL( saveYourself() ), this, SLOT( slotSave() ) );
	connect( kapp, SIGNAL( shutDown() ), this, SLOT( slotShutDown() ) );

        // Global configuration
	KConfig *config = kapp->getConfig();
	config->setGroup("KFM Misc Defaults");
        bAllowURLProps = config->readBoolEntry( "EnablePerURLProps", false );
        bTreeViewFollowMode = config->readBoolEntry( "TreeFollowsView", false);

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

    // Install HTTP Cookies
    {
        KConfig *config = kapp->getConfig();
        config->setGroup( "Browser Settings/HTTP" );
        
        bool cookiesEnabled = config->readBoolEntry( "Cookies", true );
        if ( cookiesEnabled)
        {
            cookiejar = new KCookieJar();
                            
            cookiejar->loadConfig( config );
                                    
            QString cookieFile = kapp->localkdedir().data();
            cookieFile += "/share/apps/kfm/cookies";
            cookiejar->loadCookies( cookieFile.data() );
        }
    }

    connect( &timer, SIGNAL( timeout() ), this, SLOT( slotTouch() ) );
    // Call every hour
    timer.start( 3600000 );
}

void Kfm::slotTouch()
{
  // Prevent the sockets from being removed by the cleanup daemon of some systems.
  QString tmp;
  tmp.sprintf("touch "_PATH_TMP"/kfm_%i_%i%s",(int)getuid(),(int)getpid(),displayName().data() );
  system( tmp.data() );
  tmp.sprintf("touch "_PATH_TMP"/kio_%i_%i%s",(int)getuid(),(int)getpid(),displayName().data() );
  system( tmp.data() );
}

Kfm::~Kfm()
{
}
    
void Kfm::slotSave()
{
  timer.stop();

  printf("slotSlave()!\n");
  
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

  // Save HTTP Cookies
  if (cookiejar)
  {
      QString cookieFile = kapp->localkdedir().data();
      cookieFile += "/share/apps/kfm/cookies";
      cookiejar->saveCookies( cookieFile.data() );
  }
  
  HTMLCache::save();
}

void Kfm::slotShutDown()
{
  HTMLCache::quit(); // cancel running jobs, if any. David.
  // Delete the sockets
  QString sock;
  sock.sprintf(_PATH_TMP"/kio_%i_%i%s",(int)getuid(), (int)getpid(),displayName().data());
  unlink( sock );

  sock.sprintf(_PATH_TMP"/kfm_%i_%i%s",(int)getuid(), (int)getpid(),displayName().data());
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

void Kfm::setUpDest (QString *url)
{
  // This function checks if destination is global mime/apps path and
  //  if so, makes a local variant of it. It vreates all needed
  // directories and .directory files. It modifies dest so it can be
  // used with kojob or any other functions. If user is root and/or has
  // access to given path nothing is done.
  // This function is here to reduce duplication of code. It is
  // verbatim copy of code from kfmview's slotDropEvent.

  //-------- Sven's write to pseudo global mime/app dirs start ---

  // Consider the following case:
  // User opened mime/apps dir and has not any local items =>
  // displayed are global items or global dirs with items.
  // Now he clicks on dir Images/. Global Images/ dir openes. Now
  // user wants to drag some new item  he got from friend to this
  // dir; He can't; this is a global directory. He is frustrated.

  // Fix: If url contains global mime/apps path, repair it to
  // point to local variant; create all needed dirs too.
  // If user is a root or can somehow write to globals
  // (whole KDE installed in his home dir), also do nothing.

  // Most of code is from kfmprops.cpp (= tested good).

#define GLOBALMIME kapp->kde_mimedir().data()
#define LOCALMIME (kapp->localkdedir() + "/share/mimelnk").data()

#define GLOBALAPPS kapp->kde_appsdir().data()
#define LOCALAPPS (kapp->localkdedir() + "/share/applnk").data()

  if (KfmGui::sumode)
    return;
  
  QString tryPath;
  tryPath = url->data(); // copy (hope deep)
  bool specialCase = false;

  // Some users CAN write to GLOBAL*.
  if (tryPath.contains(GLOBALMIME) && // if global mime..
      access (GLOBALMIME, W_OK) != 0) // ..and canot write
  {
    tryPath.remove(5, strlen(GLOBALMIME));
    tryPath.insert(5, LOCALMIME);
    specialCase = true;
  }
  else  if (tryPath.contains(GLOBALAPPS) && // if global apps..
	    access (GLOBALAPPS, W_OK) != 0) // ..and canot write
  {
    tryPath.remove(5, strlen(GLOBALAPPS));
    tryPath.insert(5, LOCALAPPS);
    specialCase = true;
  }
  // Ok repaired. Now we have to check/create nedded dir(s)

  if (specialCase)
  {
    QString path = &(url->data())[5];
    QDir lDir;
    // debug ("********SPECIAL CASE");
    if (path.find(kapp->kde_appsdir()) == 0) // kde_appsdir on start of path
    {
      path.remove(0, strlen(kapp->kde_appsdir())); //remove kde_appsdir
      lDir.setPath(LOCALAPPS);
    }
    else if (path.find(kapp->kde_mimedir()) == 0) // kde_mimedir on start of path
    {
      path.remove(0, strlen(kapp->kde_mimedir())); //remove kde_appsdir
      lDir.setPath(LOCALMIME);
    }
    else
    {
      debug ("HEEELP");
      return;
    }

    if (path[0] == '/')
      path.remove(0, 1); // remove /
    bool err;
    while (path.contains('/'))
    {
      int i = path.find('/'); // find separator
      if (!lDir.cd(path.left(i)))  // exists?
      {
	lDir.mkdir((path.left(i)));  // no, create
	if (!lDir.cd((path.left(i)))) // can cd to?
	{
	  err = true;                 // no flag it...
	  // debug ("Can't cd to  %s in %s", path.left(i).data(),
	  //	 lDir.absPath().data());
	  break;                      // and exit while
	}
	// Begin copy .directory if exists here.
	// This block can be commented out without problems
	// in case of problems.
	{
	  QFile tmp(kapp->kde_appsdir() +
		    "/" + path.left(i) + "/.directory");
	  //debug ("---- looking for: %s", tmp.name());
	  if (tmp.open( IO_ReadOnly))
	  {
	    //debug ("--- opened RO");
	    char *buff = new char[tmp.size()+10];
	    if (buff != 0)
	    {
	      if (tmp.readBlock(buff, tmp.size()) != -1)
	      {
		size_t tmpsize = tmp.size();
		//debug ("--- read");
		tmp.close();
		tmp.setName(lDir.absPath() + "/.directory");
		//debug ("---- copying to: %s", tmp.name());
		if (tmp.open(IO_ReadWrite))
		{
		  //debug ("--- opened RW");
		  if (tmp.writeBlock(buff, tmpsize) != -1)
		  {
		    //debug ("--- wrote");
		    tmp.close();
		  }
		  else
		  {
		    //debug ("--- removed");
		    tmp.remove();
		  }
		}                 // endif can open to write
	      }                   // endif can read
	      else     //coulnd't read
		tmp.close();

	      delete[] buff;
	    }                     // endif is alocated
	  }                       // can open to write
	}
	// End coping .directory file

      }
      path.remove (0, i);           // cded to;
      if (path[0] == '/')
	path.remove(0, 1); // remove / from path
    }

    // if it fails, kfmman will let us know
    url->setStr(tryPath.data());
  }                            // end if special case
  //-------- Sven's write to pseudo global mime/app dirs end ---
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
  system ( kfm.data() );                     // sven: tell shell to do it.

  exit (0);                                  // exit clean.
}

#include "kfmw.moc"

