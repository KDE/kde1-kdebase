// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include "kfmipc.h"
#include "kfmdlg.h"
#include "kfmwin.h"
#include "root.h"
#include "kio.h"
#include "kfmprops.h"

void ipcKFM::refreshDesktop()
{
    printf("Refresh desktop\n");
    KRootWidget::getKRootWidget()->update();
}

void ipcKFM::openURL(char* _url)
{
    printf("Opening URL '%s'\n", _url );

    if ( _url[0] != 0 )
    {
	KFileWindow *w = KFileWindow::findWindow( _url );
	if ( w != 0L )
	{
	    w->show();
	    return;
	}
	
	KFileWindow *f = new KFileWindow( 0L, 0L, _url );
	f->show();
	return;
    }
    
    QString home = "file:";
    home += QDir::homeDirPath().data();
    DlgLineEntry l( "Open Location:", home.data(), KRootWidget::getKRootWidget() );
    if ( l.exec() )
    {
	QString url = l.getText();
	// Exit if the user did not enter an URL
	if ( url.data()[0] == 0 )
	    return;
	// Root directory?
	if ( url.data()[0] == '/' )
	{
	    url = "file:";
	    url += l.getText();
	}
	// Home directory?
	else if ( url.data()[0] == '~' )
	{
	    url = "file:";
	    url += QDir::homeDirPath().data();
	    url += l.getText() + 1;
	}
	
	KURL u( url.data() );
	if ( u.isMalformed() )
	{
	    printf("ERROR: Malformed URL\n");
	    return;
	}

	KFileWindow *f = new KFileWindow( 0L, 0L, url.data() );
	f->show();
    }
}

void ipcKFM::refreshDirectory(char* _url)
{
    KIOManager::getKIOManager()->sendNotify( _url );
}

void ipcKFM::openProperties(char* _url)
{
    new Properties( _url );
}
