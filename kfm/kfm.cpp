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

KFM *KFM::pKfm = 0L;

KFM::KFM()
{
    pKfm = this;
    
    kapp->setTopWidget( this );

    HTMLCache::load();
    
    if ( KfmGui::rooticons )
    {
	kapp->enableSessionManagement( TRUE );
	kapp->setWmCommand( "" );
	
	connect( kapp, SIGNAL( saveYourself() ), this, SLOT( slotSave() ) );

	KConfig *config = kapp->getConfig();
	config->setGroup( "SM" );
	bool flag = config->hasKey( "URLs" );
	
	QStrList urlList;
	int n = config->readListEntry( "URLs", urlList );
	printf("N=%i\n",n);
	
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
		printf("URL='%s'\n",urlList.at( i - 1 ) );
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

void KFM::slotTouch()
{
  QString tmp;
  tmp.sprintf("touch /tmp/kfm_%i%s",(int)getpid(),displayName().data() );
  system( tmp.data() );
  tmp.sprintf("touch /tmp/kio_%i%s",(int)getpid(),displayName().data() );
  system( tmp.data() );
}

KFM::~KFM()
{
}
    
void KFM::slotSave()
{
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

#include "kfm.moc"
