// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de


#include "kfmserver.h"
#include "kfmdlg.h"
#include "kfmwin.h"
#include "root.h"
#include "kioserver.h"
#include "kfmprops.h"
#include "kiojob.h"

#include <qmsgbox.h>

KFMServer::KFMServer() : KfmIpcServer()
{
    connect( this, SIGNAL( newClient( KfmIpc * ) ), this, SLOT( slotNewClient( KfmIpc * ) ) );
}

void KFMServer::slotNewClient( KfmIpc * _client )
{
    connect( _client, SIGNAL( refreshDesktop() ), this, SLOT( slotRefreshDesktop() ) );
    connect( _client, SIGNAL( openURL( const char* ) ), this, SLOT( slotOpenURL( const char *) ) );    
    connect( _client, SIGNAL( refreshDirectory( const char* ) ), this, SLOT( slotRefreshDirectory( const char *) ) );    
    connect( _client, SIGNAL( openProperties( const char* ) ), this, SLOT( slotOpenProperties( const char *) ) );    
    connect( _client, SIGNAL( exec( const char*, const char* ) ),
	     this, SLOT( slotExec( const char *, const char*) ) );    
    connect( _client, SIGNAL( moveClient( const char*, const char* ) ), this,
	     SLOT( slotMoveClients( const char *, const char* ) ) );    
    connect( _client, SIGNAL( ask( int, int, const char*, const char* ) ), this,
	     SLOT( slotAsk( int, int, const char *, const char* ) ) );    
}

void KFMServer::slotAccept( KSocket * _sock )
{
    KfmIpc * i = new KFMClient( _sock );
    emit newClient( i );
}

void KFMServer::slotRefreshDesktop()
{
    KRootWidget::getKRootWidget()->update();
}

void KFMServer::slotAsk( int _x, int _y, const char *_src_urls, const char *_dest_url )
{
    new KFMAsk( this, _x, _y, _src_urls, _dest_url );
}

void KFMServer::slotMoveClients( const char *_src_urls, const char *_dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
    {
	dest = "file:";
	dest += QDir::homeDirPath().data();
	dest += "/Desktop/Trash/";
    }

    printf("Moving to '%s'\n",dest.data());
    
    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	printf("Appened '%s'\n",t.data());
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );
    printf("Appened '%s'\n",s.data());

    KIOJob *job = new KIOJob();
    job->move( urlList, dest.data() );
}

void KFMServer::slotCopyClients( const char *_src_urls, const char *_dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
    {
	dest = "file:";
	dest += QDir::homeDirPath().data();
	dest += "/Desktop/Trash/";
    }

    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );

    KIOJob *job = new KIOJob();
    job->copy( urlList, dest.data() );
}

void KFMServer::slotOpenURL( const char* _url )
{
    printf("KFMServer::Opening URL '%s'\n", _url );

    if ( _url[0] != 0 )
    {
	printf("There is an URL\n");
	
	QString url = _url;
	KURL u( _url );
	if ( u.isMalformed() )
	{
	    QMessageBox::message( "KFM Error", "Malformed URL\n" + url );
	    return;
	}
	printf("OK\n");
	
	url = u.url().copy();
	
	if ( url == "trash:/" )
	{
	    url = "file:";
	    url += QDir::homeDirPath().data();
	    url += "/Desktop/Trash/";
	}
	
	printf("Seaerching window\n");
	KFileWindow *w = KFileWindow::findWindow( url.data() );
	if ( w != 0L )
	{
	    printf("Window found\n");
	    w->show();
	    return;
	}
	
	printf("Opening new window\n");
	KFileWindow *f = new KFileWindow( 0L, 0L, url.data() );
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

void KFMServer::slotRefreshDirectory( const char* _url )
{
    printf("CMD: refeshDirectory '%s'\n",_url );
    
    QString tmp = _url;
    
    KURL u( _url );
    if ( tmp.right(1) == "/" )
	KIOServer::sendNotify( u.url() );
    else
    {
	printf("Sending something to '%s'\n",u.directoryURL());
	KIOServer::sendNotify( u.directoryURL() );
    }
}

void KFMServer::slotOpenProperties( const char* _url )
{
    new Properties( _url );
}

void KFMServer::slotExec( const char* _url, const char * _binding )
{
    // Attention this is a little hack by me (Matthias)
    QStrList sl;
    sl.append(_binding);   

    if ( _binding == 0L )
	KFileType::runBinding( _url );
    else
	KFileType::runBinding( _url, _binding, &sl );
}

KFMClient::KFMClient( KSocket *_sock ) : KfmIpc( _sock )
{
    connect( this, SIGNAL( copy( const char*, const char* ) ), this, SLOT( slotCopy( const char*, const char* ) ) );
    connect( this, SIGNAL( move( const char*, const char* ) ), this, SLOT( slotMove( const char*, const char* ) ) );
}

void KFMClient::slotCopy( const char *_src_url, const char * _dest_url )
{
    KIOJob * job = new KIOJob();

    connect( job, SIGNAL( finished( int ) ), this, SLOT( finished( int ) ) );

    job->copy( _src_url, _dest_url );    
}

void KFMClient::slotMove( const char *_src_urls, const char *_dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
    {
	dest = "file:";
	dest += QDir::homeDirPath().data();
	dest += "/Desktop/Trash/";
    }

    printf("Moving to '%s'\n",dest.data());
    
    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	printf("Appened '%s'\n",t.data());
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );
    printf("Appened '%s'\n",s.data());

    KIOJob *job = new KIOJob();
    connect( job, SIGNAL( finished( int ) ), this, SLOT( finished( int ) ) );
    job->move( urlList, dest.data() );
}

void KFMClient::finished( int _id )
{
    KfmIpc::finished();
}

// A Hack for KPanel
KFMAsk::KFMAsk( KFMServer *_server, int _x, int _y, const char *_src_urls, const char *_dest_url )
{
    server = _server;
    srcURLs = _src_urls;
    destURL = _dest_url;
 
    popupMenu = new QPopupMenu();
    
    popupMenu->insertItem( "Copy", this, SLOT( slotCopy() ) );
    popupMenu->insertItem( "Move", this, SLOT( slotMove() ) );
    
    QPoint p( _x, _y );
    popupMenu->popup( p );   
}

void KFMAsk::slotMove()
{
    delete popupMenu;
    server->slotMoveClients( srcURLs.data(), destURL.data() );
    delete this;
}

void KFMAsk::slotCopy()
{
    delete popupMenu;
    server->slotCopyClients( srcURLs.data(), destURL.data() );
    delete this;
}

KFMAsk::~KFMAsk()
{
}

#include "kfmserver.moc"








