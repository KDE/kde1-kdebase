#include "kurl.h"
#include "kfmgui.h"
#include "kfm.h"

#include <qstrlist.h>
#include <kapp.h>
#include <kconfig.h>

KFM::KFM()
{
    kapp->setTopWidget( this );

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
}

#include "kfm.moc"
