// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de


#include "kfmserver.h"
#include "kfmdlg.h"
#include "kfmgui.h"
#include "root.h"
#include "kioserver.h"
#include "kfmprops.h"
#include "kiojob.h"
#include <config-kfm.h>

#include <qmsgbox.h>

#include <stdlib.h>
#include <time.h>

QString KFMClient::password;

KFMServer::KFMServer() : KfmIpcServer()
{
    // Create the password file if it does not exist
    QString fn = getenv( "HOME" );
    fn += "/.kfm/magic";
    FILE *f = fopen( fn.data(), "rb" );
    if ( f == 0L )
    {
	FILE *f = fopen( fn.data(), "wb" );
	if ( f == 0L )
	{
	    QMessageBox::message("KFM Error", "Could not create ~/.kfm/magic" );
	    return;
	}
	
	QString pass;
	pass.sprintf("%i",time(0L));
	fwrite( pass.data(), 1, pass.length(), f );
	fclose( f );

	QMessageBox::message("KFM Warning", "Please change the password in\n\r~/.kfm/magic" );
    }
    else
	fclose( f );
}

void KFMServer::slotAuthorized( KFMClient * _client )
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
    connect( _client, SIGNAL( sortDesktop() ), this, SLOT( slotSortDesktop() ) );
}

void KFMServer::slotAccept( KSocket * _sock )
{
    KfmIpc * i = new KFMClient( _sock );
    connect( i, SIGNAL( authorized( KFMClient * ) ), this, SLOT( slotAuthorized( KFMClient * ) ) );
    emit newClient( i );
}

void KFMServer::slotSortDesktop()
{
    debugT("JOB: sortDesktop\n");
    KRootWidget::getKRootWidget()->sortIcons();
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

    debugT("Moving to '%s'\n",dest.data());
    
    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	debugT("Appened '%s'\n",t.data());
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );
    debugT("Appened '%s'\n",s.data());

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
    debugT("KFMServer::Opening URL '%s'\n", _url );

    if ( _url[0] != 0 )
    {
	debugT("There is an URL\n");
	
	QString url = _url;
	KURL u( _url );
	if ( u.isMalformed() )
	{
	    QMessageBox::message( "KFM Error", "Malformed URL\n" + url );
	    return;
	}
	debugT("OK\n");
	
	url = u.url().copy();
	
	if ( url == "trash:/" )
	{
	    url = "file:";
	    url += QDir::homeDirPath().data();
	    url += "/Desktop/Trash/";
	}
	
	debugT("Seaerching window\n");
	KfmGui *w = KfmGui::findWindow( url.data() );
	if ( w != 0L )
	{
	    debugT("Window found\n");
	    w->show();
	    return;
	}
	
	debugT("Opening new window\n");
	KfmGui *f = new KfmGui( 0L, 0L, url.data() );
	f->show();
	return;
    }
    
    QString home = "file:";
    home += QDir::homeDirPath().data();
    DlgLineEntry l( "Open Location:", home.data(), KRootWidget::getKRootWidget(), TRUE );
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
	    debugT("ERROR: Malformed URL\n");
	    return;
	}

	KfmGui *f = new KfmGui( 0L, 0L, url.data() );
	f->show();
    }
}

void KFMServer::slotRefreshDirectory( const char* _url )
{
    debugT("CMD: refeshDirectory '%s'\n",_url );
    
    QString tmp = _url;
    
    KURL u( _url );
    if ( tmp.right(1) == "/" )
	KIOServer::sendNotify( u.url() );
    else
    {
	debugT("Sending something to '%s'\n",u.directoryURL());
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
	KMimeType::runBinding( _url );
    else
	KMimeType::runBinding( _url, _binding, &sl );
}

KFMClient::KFMClient( KSocket *_sock ) : KfmIpc( _sock )
{
    bAuth = TRUE;
    
    connect( this, SIGNAL( auth( const char* ) ), this, SLOT( slotAuth( const char* ) ) );
}

void KFMClient::slotAuth( const char *_password )
{
    if ( KFMClient::password.isNull() )
    {
	QString fn = getenv( "HOME" );
	fn += "/.kfm/magic";
	FILE *f = fopen( fn.data(), "rb" );
	if ( f == 0L )
	{
	    QMessageBox::message( "KFM Error", "You dont have the file ~/.kfm/magic\n\rAuthorization failed" );
	    return;
	}
	char buffer[ 1024 ];
	char *p = fgets( buffer, 1023, f );
	fclose( f );
	if ( p == 0L )
	{
	    QMessageBox::message( "KFM Error", "The file ~/.kfm/magic is corrupted\n\rAuthorization failed" );
	    return;
	}
	KFMClient::password = buffer;
    }
    if ( KFMClient::password != _password )
    {
	QMessageBox::message( "KFM Error", "Someone tried to authorize himself\nusing a wrong password" );
	bAuth = FALSE;
	return;
    }

    bAuth = TRUE;
    connect( this, SIGNAL( copy( const char*, const char* ) ),
	     this, SLOT( slotCopy( const char*, const char* ) ) );
    connect( this, SIGNAL( move( const char*, const char* ) ), 
	     this, SLOT( slotMove( const char*, const char* ) ) );

    emit authorized( this );
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

    debugT("Moving to '%s'\n",dest.data());
    
    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	debugT("Appened '%s'\n",t.data());
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );
    debugT("Appened '%s'\n",s.data());

    KIOJob *job = new KIOJob();
    connect( job, SIGNAL( finished( int ) ), this, SLOT( finished( int ) ) );
    job->move( urlList, dest.data() );
}

void KFMClient::finished( int )
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








