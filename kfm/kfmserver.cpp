// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#include <qdir.h>

#include "kfmserver.h"
#include "kfmdlg.h"
#include "kfmgui.h"
#include "root.h"
#include "kioserver.h"
#include "kfmprops.h"
#include "kiojob.h"
#include "kfmpaths.h"
#include "kfmexec.h"
#include "config-kfm.h"

#include <klocale.h>
#include <kstring.h>

#include <qmsgbox.h>

#include <stdlib.h>
#include <time.h>

QString* KFMClient::password = 0L;

KFMServer::KFMServer() : KfmIpcServer()
{
    // Create the password file if it does not exist
    QString fn = getenv( "HOME" );
    fn += "/.kde/share/apps/kfm/magic";
    FILE *f = fopen( fn.data(), "rb" );
    if ( f == 0L )
    {
	FILE *f = fopen( fn.data(), "wb" );
	if ( f == 0L )
	{
	    QMessageBox::warning( 0, i18n( "KFM Error" ),
				  i18n( "Could not create ~/.kde/share/apps/kfm/magic" ) );
	    return;
	}
	
	QString pass;
	pass.sprintf("%i",time(0L));
	fwrite( pass.data(), 1, pass.length(), f );
	fclose( f );

	QMessageBox::warning( (QWidget*)0L, i18n( "KFM Warning" ),
			      i18n( "Please change the password in\n~/.kde/share/apps/kfm/magic" ) );
    }
    else
	fclose( f );
}

void KFMServer::slotAccept( KSocket * _sock )
{
    KfmIpc * i = new KFMClient( _sock, this );
    emit newClient( i );
}

#define root KRootWidget::getKRootWidget()

void KFMServer::slotSelectRootIcons( int _x, int _y, int _w, int _h, bool _add )
{
    if ( root )
    {
	if ( !_add )
	    root->unselectAllIcons();
	QRect r( _x, _y, _w, _h );
	root->selectIcons( r );
    }
}

void KFMServer::slotSortDesktop()
{
    if ( QMessageBox::warning( (QWidget*)0L, i18n( "KFM Warning" ),
			  i18n( "Do you really want to rearrange your icons ?" ),
			  i18n( "Yes" ), i18n( "No" ) ) != 0 )
	return;
    
    if ( root )
	KRootWidget::getKRootWidget()->rearrangeIcons();
}

void KFMServer::slotRefreshDesktop()
{
    if ( root )
	KRootWidget::getKRootWidget()->update();
}

void KFMServer::slotMoveClients( const char *_src_urls, const char *_dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
	dest = "file:" + KFMPaths::TrashPath();

    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );

    KIOJob *job = new KIOJob();
    if ( urlList.count() == 1 )
      job->move( urlList.first(), dest.data() );
    else
      job->move( urlList, dest.data() );
}

void KFMServer::slotCopyClients( const char *_src_urls, const char *_dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
        dest = "file:" + KFMPaths::TrashPath();

    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );

    KIOJob *job = new KIOJob();
    if ( urlList.count() == 1 )
      job->copy( urlList.first(), dest.data() );
    else
      job->copy( urlList, dest.data() );
}

void KFMServer::slotOpenURL( const char* _url )
{
    if ( _url[0] != 0 )
    {
	QString url = _url;
	KURL u( _url );
	if ( u.isMalformed() )
	{
	    QString tmp;
	    ksprintf(&tmp,"Malformed URL \n%s", url.data());
	    QMessageBox::warning( (QWidget*)0L, 
				  i18n( "KFM Error" ), tmp );
	    return;
	}

	if ( url == "trash:/" )
	    url = "file:" + KFMPaths::TrashPath();
	
	KfmGui *w = KfmGui::findWindow( url.data() );
	if ( w != 0L )
	{
	    w->show();
	    return;
	}

	KFMExec* exec = new KFMExec;
	exec->openURL( url );
	return;
    }
    
    QString home = "file:";
    home += QDir::homeDirPath().data();
    DlgLineEntry l( i18n( "Open Location:" ), home.data(), KRootWidget::getKRootWidget(), true );
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
	    return;
	}

	KfmGui *f = new KfmGui( 0L, 0L, url.data() );
	f->show();
    }
}

void KFMServer::slotRefreshDirectory( const char* _url )
{   
    QString tmp = _url;
    
    KURL u( _url );
    if ( tmp.right(1) == "/" )
	KIOServer::sendNotify( u.url() );
    else
    {
	KIOServer::sendNotify( u.directoryURL() );
    }
}

void KFMServer::slotOpenProperties( const char* _url )
{
    new Properties( _url );
}

void KFMServer::slotExec( const char* _url, const char * _documents )
{
    KURL u( _url );
    if ( u.isMalformed() )
    {
	QString msg;
	ksprintf(&msg, i18n( "The URL\n%s\nis malformed" ), 
		 _url);
	QMessageBox::warning( (QWidget*)0, 
			      i18n( "KFM Error" ), msg );
	return;
    }
    
    if ( _documents == 0L && _url != 0L )
    {
	KFMExec *e = new KFMExec;
	e->openURL( _url );
	return;
    }
    
    QString s( _documents );
    QStrList urlList;
        
    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );
    KMimeType *typ = KMimeType::getMagicMimeType( _url );
    typ->runAsApplication( _url, &urlList );
}

KFMClient::KFMClient( KSocket *_sock, KFMServer *_server ) : KfmIpc( _sock )
{
    bAuth = FALSE;
    server = _server;
    
    connect( this, SIGNAL( auth( const char* ) ), this, SLOT( slotAuth( const char* ) ) );
}

void KFMClient::slotAuth( const char *_password )
{
    if ( KFMClient::password == 0L )
	KFMClient::password = new QString;
    
    if ( KFMClient::password->isNull() )
    {
	QString fn = getenv( "HOME" );
	fn += "/.kde/share/apps/kfm/magic";
	FILE *f = fopen( fn.data(), "rb" );
	if ( f == 0L )
	{
	    QMessageBox::warning( (QWidget*)0, i18n( "KFM Error" ),
				  i18n( "You dont have the file ~/.kde/share/apps/kfm/magic\nAuthorization failed" ) );
	    return;
	}
	char buffer[ 1024 ];
	char *p = fgets( buffer, 1023, f );
	fclose( f );
	if ( p == 0L )
	{
	    QMessageBox::warning( (QWidget*)0, i18n( "KFM Error" ),
				  i18n( "The file ~/.kde/share/apps/kfm/magic is corrupted\nAuthorization failed" ) );
	    return;
	}
	*( KFMClient::password ) = buffer;
    }
    if ( *( KFMClient::password ) != _password )
    {
	QMessageBox::warning( (QWidget*)0, i18n( "KFM Error" ),
			      i18n( "Someone tried to authorize himself\nusing a wrong password" ) );
	bAuth = false;
	return;
    }

    bAuth = true;
    connect( this, SIGNAL( list( const char* ) ),
	     this, SLOT( slotList( const char* ) ) );
    connect( this, SIGNAL( copy( const char*, const char* ) ),
	     this, SLOT( slotCopy( const char*, const char* ) ) );
    connect( this, SIGNAL( move( const char*, const char* ) ), 
	     this, SLOT( slotMove( const char*, const char* ) ) );
    connect( this, SIGNAL( copyClient( const char*, const char* ) ),
	     server, SLOT( slotCopyClients( const char*, const char* ) ) );
    connect( this, SIGNAL( moveClient( const char*, const char* ) ), 
	     server, SLOT( slotMoveClients( const char*, const char* ) ) );
    connect( this, SIGNAL( refreshDesktop() ), server, SLOT( slotRefreshDesktop() ) );
    connect( this, SIGNAL( openURL( const char* ) ), server, SLOT( slotOpenURL( const char *) ) );    
    connect( this, SIGNAL( refreshDirectory( const char* ) ), server, SLOT( slotRefreshDirectory( const char *) ) );    
    connect( this, SIGNAL( openProperties( const char* ) ), server, SLOT( slotOpenProperties( const char *) ) );    
    connect( this, SIGNAL( exec( const char*, const char* ) ),
	     server, SLOT( slotExec( const char *, const char*) ) );    
    connect( this, SIGNAL( sortDesktop() ), server, SLOT( slotSortDesktop() ) );
    connect( this, SIGNAL( selectRootIcons( int, int, int, int, bool ) ),
	     server, SLOT( slotSelectRootIcons( int, int, int, int, bool ) ) );
}

void KFMClient::slotCopy( const char *_src_urls, const char * _dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
	dest = "file:" + KFMPaths::TrashPath();

    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );

    KIOJob *job = new KIOJob();
    connect( job, SIGNAL( finished( int ) ), this, SLOT( finished( int ) ) );
    if ( urlList.count() == 1 )
      job->copy( urlList.first(), dest.data() );
    else
      job->copy( urlList, dest.data() );
}

void KFMClient::slotMove( const char *_src_urls, const char *_dest_url )
{
    QString s = _src_urls;
    s.detach();
    QStrList urlList;
    
    QString dest = _dest_url;
    if ( dest == "trash:/" )
	dest = "file:" + KFMPaths::TrashPath();

    int i;
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );

    KIOJob *job = new KIOJob();
    connect( job, SIGNAL( finished( int ) ), this, SLOT( finished( int ) ) );
    if ( urlList.count() == 1 )
      job->move( urlList.first(), dest.data() );
    else
      job->move( urlList, dest.data() );
}

void KFMClient::slotList( const char *_url )
{
  KIOJob *job = new KIOJob();
  
  connect( job, SIGNAL( newDirEntry( int, KIODirectoryEntry* ) ),
	   this, SLOT( newDirEntry( int, KIODirectoryEntry* ) ) );
  connect( job, SIGNAL( error( int, const char* ) ),
	   this, SLOT( slotError( int, const char* ) ) );
  connect( job, SIGNAL( finished( int ) ), this, SLOT( finished( int ) ) );
  job->list( _url );
}

void KFMClient::slotError( int _kioerror, const char *_text )
{
  KfmIpc::error( _kioerror, _text );
}

void KFMClient::newDirEntry( int, KIODirectoryEntry * _entry )
{
  KfmIpc::dirEntry( _entry->getName(), _entry->getAccess(), _entry->getOwner(),
		    _entry->getGroup(), _entry->getCreationDate(), _entry->getSize() );
}

void KFMClient::finished( int )
{
  KfmIpc::finished();
}

#include "kfmserver.moc"
