#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#include <qpopmenu.h>
#include <qpainter.h>
#include <qapp.h>
#include <qkeycode.h>
#include <qaccel.h>
#include <qpushbt.h>
#include <qdir.h>
#include <qstrlist.h>
#include <qregexp.h>
#include <qmsgbox.h>
#include <qtooltip.h>

#include <kconfig.h>
#include <kapp.h>
#include <kwm.h>

#include "kfmview.h"
#include "kfmdlg.h"
#include "kfmprops.h"
#include "kbutton.h"
#include "kfmpaths.h"
#include "config-kfm.h"
#include "utils.h"
#include "kfm.h"
#include "kfmexec.h"

#include <klocale.h>
#include <kstring.h>

// constants used when dragging a selection rectange outside the kfm window
#define AUTOSCROLL_DELAY	150
#define AUTOSCROLL_STEP		20


QStrList *KfmView::clipboard;

KfmView::KfmView( KfmGui *_gui, QWidget *parent, const char *name, KHTMLView *_parent_view )
    : KHTMLView( parent, name, 0, _parent_view )
{
    rectStart = false;
    dPainter = 0L;
    
    htmlCache = new HTMLCache();

    connect( htmlCache, SIGNAL( urlLoaded( const char*, const char *) ),
	     this, SLOT( slotImageLoaded( const char*, const char* ) ) );
    connect( this, SIGNAL( imageRequest( const char * ) ), htmlCache, SLOT( slotURLRequest( const char * ) ) );
    connect( this, SIGNAL( cancelImageRequest( const char * ) ),
	     htmlCache, SLOT( slotCancelURLRequest( const char * ) ) );
    connect( this, SIGNAL( popupMenu( KHTMLView *, const char *, const QPoint & ) ),
	     this, SLOT( slotPopupMenu2( KHTMLView *, const char *, const QPoint & ) ) );
    connect( getKHTMLWidget(), SIGNAL( scrollVert( int ) ),
	SLOT( slotUpdateSelect(int) ) );
    
    gui = _gui;
 
    dropZone = 0L;
    popupMenu = 0L;
    
    stackLock = false;

    // ignoreMouseRelease = false; // Stephan: Just guessed. It was undefined

    backStack.setAutoDelete( false );
    forwardStack.setAutoDelete( false );

    childViewList.setAutoDelete( false );

    manager = new KFMManager( this );

    dropZone = new KDNDDropZone( view , DndURL );
    connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ),
	     this, SLOT( slotDropEvent( KDNDDropZone *) ) );
    connect( dropZone, SIGNAL( dropEnter( KDNDDropZone *) ),
	     this, SLOT( slotDropEnterEvent( KDNDDropZone *) ) );
    connect( dropZone, SIGNAL( dropLeave( KDNDDropZone *) ),
	     this, SLOT( slotDropLeaveEvent( KDNDDropZone *) ) );

    connect( KIOServer::getKIOServer(), SIGNAL( notify( const char * ) ), this,
    	     SLOT( slotFilesChanged( const char * ) ) );
    connect( KIOServer::getKIOServer(), SIGNAL( mountNotify() ), this, SLOT( slotMountNotify() ) );

    getKHTMLWidget()->setFocusPolicy( QWidget::StrongFocus );
    setHTMLWidgetOptions();
}

KHTMLView* KfmView::newView( QWidget *_parent, const char *_name, int )
{
    KfmView *v = new KfmView( gui, _parent, _name, this );
    childViewList.append( v );
    return v;
}

KfmView::~KfmView()
{
    // debugT("Deleting KfmView\n");
    
    if ( dropZone != 0L )
	delete dropZone;
    dropZone = 0L;

    // MRJ: make sure all requests are cancelled before the cache is deleted
    cancelAllRequests();
    
    delete manager;
 
    delete htmlCache;
    
    // debugT("Deleted\n");
}

void KfmView::setHTMLWidgetOptions(){

  int fSize;
  QString stdName;
  QString fixedName;

  QColor bgColor;
  QColor textColor;
  QColor linkColor ;
  QColor vLinkColor ;

  KConfig *config = KApplication::getKApplication()->getConfig();
  config->setGroup( "KFM HTML Defaults" );		
  
  QString fs = config->readEntry( "BaseFontSize" );
  if ( !fs.isEmpty() )
    {
      fSize = fs.toInt();
      if ( fSize < 3 )
	fSize = 3;
      else if ( fSize > 5 )
	fSize = 5;
    }
  else
    fSize = 3;


  stdName = config->readEntry( "StandardFont" );
  if ( stdName.isEmpty() )
    stdName = DEFAULT_VIEW_FONT;

  fixedName = config->readEntry( "FixedFont" );
  if ( fixedName.isEmpty() )
    fixedName = DEFAULT_VIEW_FIXED_FONT;

  bool changeCursor = (bool) config->readNumEntry("ChangeCursor",0);

  KHTMLWidget* htmlview;
  htmlview=getKHTMLWidget();
  htmlview->setFixedFont( fixedName);
  htmlview->setStandardFont( stdName );
  htmlview->setDefaultFontBase( fSize );
  if(changeCursor)
    htmlview->setURLCursor( upArrowCursor);
  else
    htmlview->setURLCursor( arrowCursor );

  config->setGroup( "KFM HTML Defaults" );	
  bgColor = config->readColorEntry( "BgColor", &HTML_DEFAULT_BG_COLOR );
  textColor = config->readColorEntry( "TextColor", &HTML_DEFAULT_TXT_COLOR );
  linkColor = config->readColorEntry( "LinkColor", &HTML_DEFAULT_LNK_COLOR );
  vLinkColor = config->readColorEntry( "VLinkColor", &HTML_DEFAULT_VLNK_COLOR);
  
  setDefaultTextColors(textColor,linkColor,vLinkColor);
  setDefaultBGColor(bgColor);

}

void KfmView::setDefaultTextColors( const QColor& textc,const QColor& linkc,
				    const QColor& vlinkc){


  view->setDefaultTextColors( 
			     textc, 
			     linkc,
			     vlinkc 
			     );

  manager->setDefaultTextColors( 
			     textc, 
			     linkc,
			     vlinkc 
			     );


}

void KfmView::setDefaultBGColor( const QColor& bgcolor ){

  view->setDefaultBGColor( bgcolor );
  manager->setDefaultBGColor( bgcolor );

}

void KfmView::begin( const char *_url, int _x_offset, int _y_offset )
{
    // Delete all frames in this view
    childViewList.clear();
    KHTMLView::begin( _url, _x_offset, _y_offset );
}

void KfmView::splitWindow()
{
    // A bad Hack.
    // Use a CGI script instead!
    view->begin( manager->getURL() );
    view->parse();
    view->write( "<HTML><HEAD><TITLE>" );
    view->write( manager->getURL() );
    view->write( "</TITLE></HEAD><FRAMESET COLS=\"50%,50%\"><FRAME SRC=\"" );
    view->write( manager->getURL() );
    view->write( "\"><FRAME SRC=\"" );
    view->write( manager->getURL() );
    view->write( "\"></FRAMESET></HTML>" );
    view->end();
}

void KfmView::slotRun()
{
    // HACK: The command is not executed in the directory
    // we are in currently. KWM does not support that yet
    KWM::sendKWMCommand("execute");  

    /* QString url2 = "";
    if ( manager->getURL() )
	url2 = manager->getURL();
    
    DlgLineEntry l( klocale->translate("Run:"), "", this, true );
    if ( l.exec() )
    {
	QString exec = l.getText();
	exec = exec.stripWhiteSpace();
	// Exit if the user did not enter an URL
	if ( exec.data()[0] == 0 )
	    return;

	QString dir = getenv( "HOME" );

	if ( !url2.isEmpty() )
	{
	    KURL u( url2 );
	    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
		dir = u.path();
	}
	
        QString cmd;    
	cmd.sprintf( "cd %s; %s &", dir.data(), exec.data() );
	system( cmd.data() );
    } */
}

void KfmView::slotTerminal()
{
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup( "Terminal" );
    QString term = "kvt";
    term = config->readEntry( "Terminal", term );

    QString dir = getenv( "HOME" );
    
    KURL u( manager->getURL() );
    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	dir = u.path();
    
    QString cmd;
    cmd << "cd " << dir << " ; " << term << "&";
    system( cmd.data() );
}

void KfmView::slotStop()
{
    // MRJ: cancel any file requests before the htmlCache is stopped.
    cancelAllRequests();
    manager->stop();
    htmlCache->stop();
}

void KfmView::slotReload()
{
    manager->openURL( manager->getURL(), true );
}

void KfmView::slotUpdateView( bool _reload )
{
    if ( isFrame() )
    {
	KfmView *v;
	for ( v = childViewList.first(); v != 0L; v = childViewList.next() )
	    v->slotUpdateView( _reload );
    }
    else
	manager->openURL( manager->getURL(), _reload );
}

void KfmView::slotMountNotify()
{
    KURL u( manager->getURL().data() );
    
    if ( strcmp( u.protocol(), "file:" ) == 0 && !u.hasSubProtocol() )
	manager->openURL( manager->getURL().data(), true );
}

void KfmView::slotFilesChanged( const char *_url )
{
    QString u1 = _url;
    if ( u1.right( 1 ) != "/" )
	u1 += "/";
    
    QString u2 = manager->getURL().data();
    u2.detach();
    if ( u2.right( 1 ) != "/" )
	u2 += "/";

    if ( u1 == u2 )
	manager->openURL( manager->getURL().data(), true );
}

void KfmView::slotDropEnterEvent( KDNDDropZone * )
{
}

void KfmView::slotDropLeaveEvent( KDNDDropZone * )
{
}

void KfmView::slotDropEvent( KDNDDropZone *_zone )
{
    QPoint p = view->mapFromGlobal( QPoint( _zone->getMouseX(), _zone->getMouseY() ) );
    const char *url = view->getURL( p );
 
    // Dropped over an object or not ?
    if ( url == 0L )
	// dropped over white ground
	url = manager->getURL();

    KURL u( url );
    if ( u.isMalformed() )
    {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: Drop over malformed URL") );
	return;
    }
    
    // Check wether we drop a directory on itself or one of its children
    int nested = 0;
    QStrList list( _zone->getURLList() );
    char *s;
    for ( s = list.first(); s != 0L; s = list.next() )
    {
	int j;
	if ( ( j = testNestedURLs( s, url ) ) )
	    if ( j == -1 || ( j > nested && nested != -1 ) )
		nested = j;
	}
    
    if ( nested == -1 )
    {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: Malformed URL") );
	return;
    }
    if ( nested == 2 )
    {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: You dropped some URL over itself") );
	return;
    }
    
    QPoint p2( _zone->getMouseX(), _zone->getMouseY() );
    manager->dropPopupMenu( _zone, url, &p2, ( nested == 0 ? false : true ) );
}




void KfmView::slotCopy()
{
    clipboard->clear();
    view->getSelected( (*clipboard) );
}

void KfmView::slotTrash()
{
    QStrList marked;
    
    view->getSelected( marked );

    KIOJob * job = new KIOJob;

    QString dest = "file:" + KFMPaths::TrashPath();
    job->move( marked, dest );
}

void KfmView::slotDelete()
{
    QStrList marked;
    
    view->getSelected( marked );

    bool ok = !QMessageBox::information( this, klocale->translate("KFM Warning"), 
				   klocale->translate("Do you really want to delete the selected file(s)?\n\nThere is no way to restore them."), 
				    klocale->translate("Yes"), 
				    klocale->translate("No") );
    
    if ( ok )
    {
      KIOJob * job = new KIOJob;
      job->del( marked );
    }
}

void KfmView::slotPaste()
{
    // Check wether we drop a directory on itself or one of its children
    int nested = 0;
    char *s;
    for ( s = clipboard->first(); s != 0L; s = clipboard->next() )
    {
	int j;
	if ( ( j = testNestedURLs( s, manager->getURL() ) ) )
	    if ( j == -1 || ( j > nested && nested != -1 ) )
		nested = j;
    }
    
    if ( nested == -1 )
    {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: Malformed URL") );
	return;
    }
    if ( nested == 2 )
    {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: You dropped a URL over itself") );
	return;
    }
    if ( nested == 1 )
    {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: You dropped a directory over one of its children") );
	return;
    }

    KIOJob * job = new KIOJob;
    job->copy( (*clipboard), manager->getURL().data() );
}

void KfmView::slotPopupMenu( QStrList &_urls, const QPoint &_point, bool _current_dir )
{
    // Show the popup Menu for the given URLs
    manager->openPopupMenu( _urls, _point, _current_dir );
}

void KfmView::slotPopupOpenWith()
{
    DlgLineEntry l( klocale->translate("Open With:"), "", this, true );
    if ( l.exec() )
    {
	QString pattern = l.getText();
	if ( pattern.length() == 0 )
	    return;
    }
    else
      return;
    
    openWithOldApplication( l.getText(), popupFiles );
}              

void KfmView::slotPopupProperties()
{
    if ( popupFiles.count() != 1 )
    {
	warning(klocale->translate("ERROR: Can not open properties for multiple files"));
	return;
    }

    (void)new Properties( popupFiles.first() );
}

void KfmView::slotPopupBookmarks()
{
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	gui->addBookmark( s, s );
}

void KfmView::slotPopupEmptyTrashBin()
{
    QString d = KFMPaths::TrashPath();
    QStrList trash;
    
    DIR *dp;
    struct dirent *ep;
    dp = opendir( d );
    if ( dp )
    {
	// Create list of all trash files
	while ( ( ep = readdir( dp ) ) != 0L )
	{
	    if ( strcmp( ep->d_name, "." ) != 0L && strcmp( ep->d_name, ".." ) != 0L )
	    {
	      QString trashFile( ep->d_name );
	      trashFile.detach();
	      trashFile.prepend (d);
	      KURL::encodeURL ( trashFile );   // make proper URL (Hen)
	      trashFile.prepend ("file:");
	      trash.append( trashFile.data() );
	    }
	}
	closedir( dp );
    }
    else
    {
      QMessageBox::warning( 0, klocale->translate("KFM Error"), 
			    klocale->translate("Could not access Trash Bin") );
	return;
    }
    
    // Delete all trash files
    KIOJob * job = new KIOJob;
    job->del( trash );
}

void KfmView::slotPopupCopy()
{
    clipboard->clear();
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	clipboard->append( s );

    // DEBUG
    for( s = clipboard->first(); s != 0L; s = clipboard->next() )
      printf("CLIPBOARD: %s\n",s);
}

void KfmView::slotPopupPaste()
{
    if ( popupFiles.count() != 1 )
    {
	QMessageBox::warning( 0, klocale->translate("KFM Error"), 
			      klocale->translate("Can not paste into multiple directories") );
	return;
    }
    
    // Check wether we drop a directory on itself or one of its children
    int nested = 0;
    char *s;
    for ( s = clipboard->first(); s != 0L; s = clipboard->next() )
    {
	int j;
	if ( ( j = testNestedURLs( s, manager->getURL() ) ) )
	    if ( j == -1 || ( j > nested && nested != -1 ) )
		nested = j;
	}
    
    if ( nested == -1 )
    {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: Malformed URL") );
	return;
    }
    if ( nested == 2 )
    {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: You dropped a URL over itself") );
	return;
    }
    if ( nested == 1 )
    {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: You dropped a directory over one of its children") );
	return;
    }

    KIOJob * job = new KIOJob;
    job->copy( (*clipboard), popupFiles.first() );
}

void KfmView::slotPopupTrash()
{
    // This function will emit a signal that causes us to redisplay the
    // contents of our directory if neccessary.
    KIOJob * job = new KIOJob;
    
    QString dest = "file:" + KFMPaths::TrashPath();
 
    job->setOverWriteExistingFiles( TRUE );
    job->move( popupFiles, dest );
}

void KfmView::slotPopupDelete()
{
    // Is the user really sure ?
    bool ok = !QMessageBox::warning( 0, klocale->translate("KFM Warning"), 
				  klocale->translate("Do you really want to delete the selected file(s)?\n\nThere is no way to restore them."), 
				  klocale->translate("Yes"), 
				  klocale->translate("No") );
    if ( ok )
    {
	// Store the decoded URLs here.
	QStrList list;
	char *s;
	for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	{
	    list.append( s );
	}

	KIOJob * job = new KIOJob;	
	job->del( list );
    }
}

void KfmView::slotPopupNewView()
{
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	KfmGui *m = new KfmGui( 0L, 0L, s );
	m->show();
    }
}

void KfmView::slotPopupCd()
{
    if ( popupFiles.count() != 1 )
    {
	warning(klocale->translate("ERROR: Can not change to multiple directories"));
	return;
    }
    
    openURL( popupFiles.first() );
}

const char * KfmView::getURL()
{
    return manager->getURL();
}

void KfmView::openURL( const char *_url, bool _refresh, int _xoffset, int _yoffset )
{
    emit newURL( _url );
    manager->openURL( _url, _refresh, _xoffset, _yoffset );
}

void KfmView::openURL( const char *_url )
{
    emit newURL( _url );
    manager->openURL( _url );
}

void KfmView::pushURLToHistory()
{
    if ( stackLock )
	return;
    
    HistoryEntry *h = new HistoryEntry;
    h->url = getKHTMLWidget()->getDocumentURL().url();
    h->xOffset = xOffset();
    h->yOffset = yOffset();
    
    backStack.push( h );
    forwardStack.setAutoDelete( true );
    forwardStack.clear();
    forwardStack.setAutoDelete( false );
	
    emit historyUpdate( true, false );
}

void KfmView::setUpURL( const char *_url )
{
  m_strUpURL = _url;

  if ( m_strUpURL.isEmpty() )
    gui->enableToolbarButton( 0, false );
  else
    gui->enableToolbarButton( 0, true );
}

void KfmView::slotUp()
{
  if ( m_strUpURL.isEmpty() )
    return;
  
  openURL( m_strUpURL, false );
}

void KfmView::slotForward()
{
    if ( forwardStack.isEmpty() )
	return;

    HistoryEntry *h = new HistoryEntry;
    h->url = getKHTMLWidget()->getDocumentURL().url();
    h->xOffset = xOffset();
    h->yOffset = yOffset();
    backStack.push( h );
    
    HistoryEntry *s = forwardStack.pop();
    if ( forwardStack.isEmpty() )
	emit historyUpdate( true, false );
    else
	emit historyUpdate( true, true );
    
    stackLock = true;
    openURL( s->url, false, s->xOffset, s->yOffset );
    stackLock = false;

    delete s;
}

void KfmView::slotBack()
{
    if ( backStack.isEmpty() )
	return;

    HistoryEntry *h = new HistoryEntry;
    h->url = getKHTMLWidget()->getDocumentURL().url();
    h->xOffset = xOffset();
    h->yOffset = yOffset();
    forwardStack.push( h );
    
    HistoryEntry *s = backStack.pop();
    if ( backStack.isEmpty() )
	emit historyUpdate( false, true );
    else
	emit historyUpdate( true, true );    

    stackLock = true;
    openURL( s->url, false, s->xOffset, s->yOffset );
    stackLock = false;

    delete s;
}

void KfmView::slotURLSelected( const char *_url, int _button, const char *_target )
{
    // Security
    KURL u1( _url );
    KURL u2( manager->getURL() );
    if ( ( strcmp( u1.protocol(), "file" ) == 0 || strcmp( u1.protocol(), "cgi" ) == 0 ) &&
	 strcmp( u2.protocol(), "file" ) != 0 && strcmp( u2.protocol(), "cgi" ) != 0 )
    {
	QMessageBox::critical( (QWidget*)0L, klocale->translate( "KFM Security Alert" ),
			       klocale->translate( "This page is untrusted\nbut it contains a link to your local file system." ) );
	return;
    }
    
    if ( !_url )
	return;
    
    if ( _target != 0L && _target[0] != 0 && _button == LeftButton )
    {
	KHTMLView *v = findView( _target );
	if ( v )
	{
	    v->openURL( _url );
	    return;
	}
	else
	{
	    KfmGui *m = new KfmGui( 0L, 0L, _url );
	    m->show();
	    return;
	}
    }

    if ( _button == MidButton )
    {
	KURL base( manager->getURL() );
	KURL u( base, _url );
	QString url = u.url();

	KFMExec *e = new KFMExec();
	e->openURL( url );
	// KfmGui *m = new KfmGui( 0L, 0L, url.data() );
	// m->show();
    }
    
    if ( _button == LeftButton )
	openURL( _url );
}

void KfmView::setPopupFiles( QStrList &_files )
{
    popupFiles.copy( _files );
}

void KfmView::slotOnURL( const char *_url )
{
    if ( _url == 0L )
    {
	// gui->slotSetStatusBarURL( activeManager->getURL() );
	gui->slotSetStatusBar( "" );
    }
    else
    {
        QString com;

	KMimeType *typ = KMimeType::findType( _url );      
	if ( typ )
	    com = typ->getComment( _url );

        KURL url (_url);
	if ( url.isMalformed() )
        {
	  gui->slotSetStatusBar( _url );
	  return;
	}

	QString decodedPath( url.path() );
	QString decodedName( url.filename() );
	
        struct stat buff;
        stat( decodedPath, &buff );

        struct stat lbuff;
        lstat( decodedPath, &lbuff );
        QString text;
	QString text2;
	text = decodedName;
	text2 = text;
	text2.detach();
	
        if ( strcmp( url.protocol(), "file") == 0 )
        {
          if (S_ISLNK( lbuff.st_mode ) )
          {
	    QString tmp;
	    if ( com.isNull() )
	      tmp = klocale->translate( "Symbolic Link");
	    else
	      tmp.sprintf(klocale->translate("%s (Link)"), com.data() );
            char buff_two[1024];
            text += "->";
            int n = readlink ( decodedPath, buff_two, 1022);
            if (n == -1)
            {
               gui->slotSetStatusBar( text2 + "  " + tmp );
               return;
            }
	    buff_two[n] = 0;
	    text += buff_two;
	    text += "  ";
	    text += tmp.data();
	  }
	  else if ( S_ISREG( buff.st_mode ) )
          {
	     text += " ";
	     if (buff.st_size < 1024)
	       text.sprintf( "%s (%ld %s)", 
			     text2.data(), (long) buff.st_size,
			     klocale->translate("bytes"));
	     else
             {
	       float d = (float) buff.st_size/1024.0;
	       text.sprintf( "%s (%.2f K)", text2.data(), d);
	     }
	     text += "  ";
	     text += com.data();
	  }
	  else
	  {
	      text += "  ";
	      text += com.data();
	  }	  
	  gui->slotSetStatusBar( text );
	}
        else
	{
	    QString decodedURL( _url );
            KURL::decodeURL(decodedURL);	    
            gui->slotSetStatusBar( decodedURL );
	}
    }
}

void KfmView::slotFormSubmitted( const char *, const char *_url )
{   
    // debugT("Form Submitted '%s'\n", _url );
    
    KURL u1( manager->getURL() );
    KURL u2( u1, _url );
    
    // debugT("Complete is '%s'\n", u2.url().data() );
    
    openURL( u2.url().data() );    
}

KfmView* KfmView::getActiveView()
{
  KHTMLView *v = getSelectedView();
  if ( !v )
    return this; 
  if ( v->isA( "KfmView" ) )
    return (KfmView*)v;
  return this;
} 

// quote special HTML-characters and write to widget
// according to RFC 1866,Hypertext Markup Language 2.0,
// 9.7.1. Numeric and Special Graphic Entity Set
// -> should go to KHTMLView
// (Hen - Henner Zeller, zeller@think.de)
void KfmView::writeHTMLquoted (const char * text)
{
	// worst case: just quotes which expand to the
	// six characters '&quot;'
	char *out_text = new char [ 6 * strlen (text) + 1 ];
	char * outpos = out_text;

	for ( /* nope */ ; *text ; text++) {
		*outpos = '\0';
		switch (*text) {
		case '&' : 
			strcat (out_text, "&amp;"); 
			outpos += 5;
			break;
		case '<':
			strcat (out_text, "&lt;"); 
			outpos += 4;
			break;
		case '>':
			strcat (out_text, "&gt;"); 
			outpos += 4;
			break;
		case '"':
			strcat (out_text, "&quot;"); 
			outpos += 6;
			break;
		default:
			*outpos = *text;
			outpos++;
		}
	}
	*outpos = '\0';
	this->write( out_text );
	delete [] out_text;
}

bool KfmView::mousePressedHook( const char *_url, const char *, QMouseEvent *_mouse, bool _isselected )
{
    rectStart = false;
    selectedURL = "";
    
    // Select by drawing a rectangle
    if ( _url == 0L && _mouse->button() == LeftButton && !manager->isHTML() )
    {
	select( 0L, false );
	rectStart = true;            // Say it is start of drag
	rectX1 = _mouse->pos().x();   // get start position
	rectY1 = _mouse->pos().y() + yOffset();
	rectX2 = rectX1;
	rectY2 = rectY1;
	if ( !dPainter )
	    dPainter = new QPainter;

	return true;
    }
    // Select a URL with Ctrl Button
    else if ( _url != 0L && _mouse->button() == LeftButton &&
	      ( _mouse->state() & ControlButton ) == ControlButton )
    {   
	selectByURL( 0L, _url, !_isselected );
	selectedURL = _url;
	// ignoreMouseRelease = true;
	return true;
    }
    else if ( _url != 0L && _mouse->button() == LeftButton )
    {
	// We can not do much here, since we dont know wether
	// this may be the start of some DND action.
	selectedURL = _url;
	return true;
    }
    // Context Menu
    else if ( _url != 0L && _mouse->button() == RightButton )
    {   
	QPoint p = mapToGlobal( _mouse->pos() );
	
	// A popupmenu for a list of URLs or just for one ?
	QStrList list;
	getSelected( list );
    
	if ( list.find( _url ) != -1 )
	    slotPopupMenu( list, p );
	else
	{
	    // The selected URL is not marked, so unmark the marked ones.
	    select( 0L, false );
	    selectByURL( 0L, _url, true );
	    list.clear();
	    list.append( _url );
	    slotPopupMenu( list, p );
	}

	return true;
    }
    // Context menu for background ?
    else if ( _url == 0L && _mouse->button() == RightButton )
    {      
	QPoint p = mapToGlobal( _mouse->pos() );
	
	select( 0L, false );
	QStrList list;
	list.append( manager->getURL() );
	slotPopupMenu( list, p, true );
	return true;
    }
    
    return false;
}

bool KfmView::mouseMoveHook( QMouseEvent *_mouse )
{
    if ( rectStart )
    {
	int x = _mouse->pos().x();
	int y = _mouse->pos().y() + yOffset();
	
	if ( !dPainter )
	{
	    // debugT ("KFileView::mouseMoveEvent: no painter\n");
	    return true;
	}

	if ( !getKHTMLWidget()->isAutoScrollingY() && bandVisible )
	{
	    dPainter->begin( view );
	    dPainter->setRasterOp (NotROP);
	    dPainter->drawRect(rectX1, rectY1-yOffset(), rectX2-rectX1,
		 rectY2-rectY1);
	    dPainter->end();
	    bandVisible = false;
	}
	
	int x1 = rectX1;
	int y1 = rectY1;    
	if ( x1 > x )
	{
	    int tmp = x1;
	    x1 = x;
	    x = tmp;
	}
	if ( y1 > y )
	{
	    int tmp = y1;
	    y1 = y;
	    y = tmp;
	}    
	
	QRect rect( x1, y1, x - x1 + 1, y - y1 + 1 );
	select( 0L, rect );
	
	rectX2 = _mouse->pos().x();
	rectY2 = _mouse->pos().y() + yOffset();
	
	if (_mouse->pos().y() > height() )
	    getKHTMLWidget()->autoScrollY( AUTOSCROLL_DELAY, AUTOSCROLL_STEP );
	else if ( _mouse->pos().y() < 0 )
	    getKHTMLWidget()->autoScrollY( AUTOSCROLL_DELAY, -AUTOSCROLL_STEP );
	else
	    getKHTMLWidget()->stopAutoScrollY();

	if ( !getKHTMLWidget()->isAutoScrollingY() )
	{
	    dPainter->begin( view );
	    dPainter->setRasterOp (NotROP);
	    dPainter->drawRect(rectX1, rectY1-yOffset(), rectX2-rectX1,
		rectY2-rectY1);
	    dPainter->end();
	    bandVisible = true;
	}
	
	return true;
    }
  
    return false;
}

bool KfmView::mouseReleaseHook( QMouseEvent *_mouse )
{
    /* if ( ignoreMouseRelease )
    {
	ignoreMouseRelease = false;
	return true;
    } */

    // make sure autoScroll is off
    if ( _mouse->button() == LeftButton && rectStart )
	getKHTMLWidget()->stopAutoScrollY();

    if ( !selectedURL.isEmpty() && _mouse->button() == LeftButton &&
	 ( _mouse->state() & ControlButton ) == ControlButton )
    {
	// This is already done, so we jusrt consume this event
	selectedURL = "";
	return true;
    }
    // The user pressed the mouse over an URL, did no DND and released it
    else if ( !selectedURL.isEmpty() && _mouse->button() == LeftButton )
    {
	QStrList list;
	getSelected( list );

	// The user selected the first icon
	if ( list.count() == 0 )
	{
	    selectByURL( 0L, selectedURL, true );
	    selectedURL = "";
	    return false;
	}
	// The user selected one of the icons that are already selected
	// if ( list.find( selectedURL ) != -1 )
	// return ;

	// The user selected one icon => deselect the other ones if there are any
	select( 0L, false );
	selectByURL( 0L, selectedURL, true );
	selectedURL = "";
	return false;
    }
   
    if ( rectStart )
    {
	rectX2 = _mouse->pos().x();
	rectY2 = _mouse->pos().y() + yOffset();
	if ( ( rectX2 == rectX1 ) && ( rectY2 == rectY1 ) )
	{
	    select( 0L, false );
	    // debugT ("KFileView::mouseReleaseEvent: it was just a dream... just a dream.\n");
	}
	else
	{
	    /* ...find out which objects are in(tersected with) rectangle and
	       select them, but remove rectangle first. */
	    if ( bandVisible )
	    {
		dPainter->begin( view );
		dPainter->setRasterOp (NotROP);
		dPainter->drawRect (rectX1, rectY1-yOffset(), rectX2-rectX1,
		    rectY2-rectY1);
		dPainter->end();
		bandVisible = false;
	    }
	    if ( rectX2 < rectX1 )
	    {
		int tmp = rectX1;
		rectX1 = rectX2;
		rectX2 = tmp;
	    }
	    if ( rectY2 < rectY1 )
	    {
		int tmp = rectY1;
		rectY1 = rectY2;
		rectY2 = tmp;
	    }
	    
	    QRect rect( rectX1, rectY1, rectX2 - rectX1 + 1, rectY2 - rectY1 + 1 );
	    select( 0L, rect );
	}
	
	rectStart = false;
	delete dPainter;
	dPainter = 0L;
	
	return true;
	
    }
    
    return false;
}

bool KfmView::dndHook( const char *_url, QPoint &_p )
{
    selectedURL = "";

    if ( _url == 0L )
	return true;
    
    QStrList l;
    getSelected( l );
    // Did the user drag an icon that was not selected ?
    if ( l.find( _url ) == -1 )
    {
	l.clear();
	l.append( _url );
    }
    
    QPixmap pixmap;
        
    // Do we drag multiple files ?
    if ( l.count() == 1 )
    {
	pixmap.load( KMimeType::getPixmapFileStatic( l.first() ) );
	/* KMimeType *typ = KMimeType::findType( l.first() );
	pixmap = typ->getPixmap( l.first() ); */
    }
    else
    {
	// pixmap.load( KMimeType::getPixmapFileStatic( l.first() ) );
	QString dir = kapp->kde_datadir();
	dir += "/kfm/pics/kmultiple.xpm";
	pixmap.load( dir );
	// TODO  Nice icon for multiple files
	/* KMimeType *typ = KMimeType::findType( l.first() );
	pixmap = typ->getPixmap( l.first() ); */
    }
    
    // Put all selected files in one line separated with newlines
    char *s;
    QString tmp = "";
    for ( s = l.first(); s != 0L; s = l.next() )
    {
	tmp += s;
	tmp += "\n";
    }
    // Delete the trailing newline
    QString data = tmp.stripWhiteSpace();
    
    // Offset of the mouse pointer relative to the upper left corner of the icon
    int dx = - pixmap.width() / 2;
    int dy = - pixmap.height() / 2;
    
    view->startDrag( new KDNDIcon( pixmap, _p.x() + dx, _p.y() + dy ), 
		     data.data(), data.length(), DndURL, dx, dy );
    
    return true;
}

void KfmView::slotUpdateSelect( int )
{
    if ( !rectStart )
	return;

    int x1 = rectX1;
    int y1 = rectY1;    

    QPoint point = QCursor::pos();
    point = mapFromGlobal( point );
    if ( point.y() > height() )
	point.setY( height() );
    else if ( point.y() < 0 )
	point.setY( 0 );

    int x = rectX2 = point.x();
    int y = rectY2 = point.y() + yOffset();

    if ( x1 > x )
    {
	int tmp = x1;
	x1 = x;
	x = tmp;
    }
    if ( y1 > y )
    {
	int tmp = y1;
	y1 = y;
	y = tmp;
    }    
    
    QRect rect( x1, y1, x - x1 + 1, y - y1 + 1 );
    select( 0L, rect );
}

void KfmView::slotPopupMenu2( KHTMLView *, const char *_url, const QPoint &_point )
{
  // A popupmenu for a list of URLs or just for one ?
  QStrList list;
  getSelected( list );
    
  if ( list.find( _url ) != -1 )
    slotPopupMenu( list, _point );
  else
  {
    // The selected URL is not marked, so unmark the marked ones.
    select( 0L, false );
    selectByURL( 0L, _url, true );
    list.clear();
    list.append( _url );
    slotPopupMenu( list, _point );
  }
}

bool KfmView::URLVisited( const char *_url )
{
  QStrList *list = KFM::history();
  if ( list->find( _url ) != -1 )
    return true;
  
  return false;
}

#include "kfmview.moc"
