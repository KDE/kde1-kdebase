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
#include <qclipbrd.h>
#include <qtooltip.h>

#include <kconfig.h>
#include <kapp.h>
#include <kwm.h>
 
#include <kcursor.h>

#include "kfmview.h"
#include "kfmdlg.h"
#include "kfmprops.h"
#include "kbutton.h"
#include "kfmpaths.h"
#include "config-kfm.h"
#include "utils.h"
#include "kfmw.h"
#include "kfmexec.h"
#include "root.h"
#include "kcookiejar.h"

#include <klocale.h>
#include <kstring.h>
#include <kcharsets.h>

// constants used when dragging a selection rectange outside the kfm window
#define AUTOSCROLL_DELAY	150
#define AUTOSCROLL_STEP		20

#define IconList KRootWidget::pKRootWidget->icon_list

QStrList *KfmView::clipboard;
bool KfmView::stackLock = false;

KfmView::KfmView( KfmGui *_gui, QWidget *parent, const char *name, KfmView *_parent_view )
    : KHTMLView( parent, name, 0, _parent_view )
{
    rectStart = false;
    dPainter = 0L;
    
    htmlCache = new HTMLCache();
    if(!_parent_view)
    {
	backStack = new QStack<SavedPage>;
	forwardStack = new QStack<SavedPage>;
	backStack->setAutoDelete( false );
	forwardStack->setAutoDelete( false );
    }
    else
    {
	backStack = _parent_view->getBackStack();
	forwardStack = _parent_view->getForwardStack();
    }

    connect( htmlCache, SIGNAL( urlLoaded( const char*, const char *) ),
	     this, SLOT( slotImageLoaded( const char*, const char* ) ) );
    connect( this, SIGNAL( imageRequest( const char * ) ), htmlCache, SLOT( slotURLRequest( const char * ) ) );
    connect( this, SIGNAL( cancelImageRequest( const char * ) ),
	     htmlCache, SLOT( slotCancelURLRequest( const char * ) ) );
    connect( this, SIGNAL( popupMenu( KHTMLView *, const char *, const QPoint & ) ),
	     this, SLOT( slotPopupMenu2( KHTMLView *, const char *, const QPoint & ) ) );
    connect( getKHTMLWidget(), SIGNAL( scrollVert( int ) ),
	SLOT( slotUpdateSelect(int) ) );

    connect( this, SIGNAL( goUp() ), this, SLOT( slotUp() ) );
    connect( this, SIGNAL( goRight() ), this, SLOT( slotForward() ) );
    connect( this, SIGNAL( goLeft() ), this, SLOT( slotBack() ) );
    connect ( getKHTMLWidget(), SIGNAL( redirect( int , const char *) ),
	      this, SLOT( slotRedirect( int, const char * )) );
    connect( &redirectTimer, SIGNAL(timeout()), 
	     this, SLOT(slotDelayedRedirect2()));


    gui = _gui;
 
    dropZone = 0L;
    popupMenu = 0L;
    
    popupMenuEvent = false;
    stackLock = false;

    // ignoreMouseRelease = false; // Stephan: Just guessed. It was undefined

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
    slotStop();

    delete manager;
 
    delete htmlCache;
    
    if(getParentView() == 0)
    {
	delete backStack;
	delete forwardStack;
    }
    // debugT("Deleted\n");

    // Save HTTP Cookies
    if (cookiejar)
    {
      QString cookieFile = kapp->localkdedir().copy();
      cookieFile += "/share/apps/kfm/cookies";
      cookiejar->saveCookies( cookieFile.data() );
    }
}

void KfmView::setHTMLWidgetOptions(){

  int fSize;
  QString stdName;
  QString fixedName;

  QColor bgColor;
  QColor textColor;
  QColor linkColor;
  QColor vLinkColor;

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

  QString aStr = config->readEntry( "DefaultCharset" );
  if(!aStr.isEmpty())
      kapp->getCharsets()->setDefault(aStr);


  bool changeCursor = config->readNumEntry("ChangeCursor",false);
  bool underlineLinks = config->readNumEntry("UnderlineLinks",true);

  KHTMLWidget* htmlview;
  htmlview=getKHTMLWidget();
  htmlview->setFixedFont( fixedName);
  htmlview->setStandardFont( stdName );
  htmlview->setDefaultFontBase( fSize );
  htmlview->setUnderlineLinks(underlineLinks);
  if(changeCursor)
    htmlview->setURLCursor( KCursor::handCursor() );
  else
    htmlview->setURLCursor( KCursor::arrowCursor() );

  config->setGroup( "KFM HTML Defaults" );	
  bgColor = config->readColorEntry( "BgColor", &HTML_DEFAULT_BG_COLOR );
  textColor = config->readColorEntry( "TextColor", &HTML_DEFAULT_TXT_COLOR );
  linkColor = config->readColorEntry( "LinkColor", &HTML_DEFAULT_LNK_COLOR );
  vLinkColor = config->readColorEntry( "VLinkColor", &HTML_DEFAULT_VLNK_COLOR);

  bool forceDefaults = config->readBoolEntry( "ForceDefaultColors", false );
  view->setForceDefault( forceDefaults );
  
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

void KfmView::setUnderlineLinks( const bool uline ){
  view->setUnderlineLinks( uline );
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

	QString dir (QDir::homeDirPath());

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
    config->setGroup( "KFM Misc Defaults" );
    QString term = config->readEntry( "Terminal", DEFAULT_TERMINAL );

    QString dir ( QDir::homeDirPath() );
    
    KURL u( manager->getURL() );
    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	dir = u.path();
    
    QString cmd;
    cmd << "cd \"" << dir << "\" ; " << term << "&";
    system( cmd.data() );
}

void KfmView::slotStop()
{
    KfmView *v;

    // MRJ: cancel any file requests before the htmlCache is stopped.
    cancelAllRequests();
    for( v = childViewList.first(); v != 0; v = childViewList.next())
	v->slotStop();

    manager->stop();
    htmlCache->stop();
}

void KfmView::slotReload()
{
  stackLock=true;
  if (!manager->getURL().isEmpty())
    manager->openURL( manager->getURL(), true );
  stackLock=false;
}

void KfmView::slotUpdateView( bool _reload )
{
    if ( isFrame() )
    {
	KfmView *v;
	for ( v = childViewList.first(); v != 0L; v = childViewList.next() )
	    v->slotUpdateView( _reload );
    }
    else {
        if (!manager->getURL().isEmpty())
            manager->openURL( manager->getURL().data(), _reload );
    }
}

void KfmView::slotMountNotify()
{
    KURL u( manager->getURL().data() );
    
    if ( u.isLocalFile() )
	manager->openURL( manager->getURL().data(), true );
}

void KfmView::slotFilesChanged( const char *_url )
{
    //-------- Sven's refresh bindings if mime/app dirs changed start ---
    // Few defines:
#define GLOBALMIME kapp->kde_mimedir().data()
#define LOCALMIME (kapp->localkdedir() + "/share/mimelnk").data()

#define GLOBALAPPS kapp->kde_appsdir().data()
#define LOCALAPPS (kapp->localkdedir() + "/share/applnk").data()

    // Refreshing.
    if (strstr (_url, LOCALMIME) != 0 ||  // local Mimes or ...
	strstr (_url, LOCALAPPS) != 0 )   // local apps  written to?
    {
      // gui->slotRescanBindings(); // newly created mimetypes are invalid...
      return;
    }
    // Note:
    // This updates if user wrote into local mime/apps dir
    // If user or root wrote to global dirs (without this redirection)
    // There will be no updateing; I belive that it might be what
    // they want - to make massive changes to globals and then
    // update manualy.
    // If you don't agree, add check for GLOBALMIME and GLOBALAPPS.
    // Also I must call gui->slotRescanBindings(), although it updates
    // templates and krootwm which are changed in this case. The
    // reason for that is that I wan't to update all windows,
    // but I cannot get the list of them (Because it usees protected
    // windowlist, instead of public one from KTW. OK, in that case
    // updateView should be virtual, and so on and on...)
    
    // David, if I cause some serious shit here, I am sorry :-)
    // I wish you all merry, merry Xmass, and very happy New Year.

    //-------- Sven's refresh bindings if mime/app dirs changed end ---
    QString u1 = _url;
    if ( u1.right( 1 ) != "/" )
	u1 += "/";
    
    QString u2 = manager->getURL().data();
    u2.detach();
    if ( u2.right( 1 ) != "/" )
	u2 += "/";

    if ( u1 == u2 )
    {
	stackLock=true;
	manager->openURL( manager->getURL().data(), true );
	stackLock=false;
    }
}

void KfmView::slotDropEnterEvent( KDNDDropZone * )
{
}

void KfmView::slotDropLeaveEvent( KDNDDropZone * )
{
}

void KfmView::slotDropEvent( KDNDDropZone *_zone )
{
      // check if dropped data is an URL
      if ( _zone->getDataType() != DndURL )
      {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: You may only drop URLs") );
	return;
      }
      
      QPoint p = view->mapFromGlobal( QPoint( _zone->getMouseX(), _zone->getMouseY() ) );
      const char *url = view->getURL( p );
 
      // Dropped over an object or not ?
      if ( url == 0L )
	// dropped over white ground
	url = manager->getURL();
      //-------- Sven's write to pseudo global mime/app dirs start ---

      // Consider the following case:
      // User opened mime/apps dir and has not any local items =>
      // displayed are global items or global dirs with items.
      // Now he clicks on dir Images/. Global Images/ dir openes. Now
      // user wants to drag some new item  he got from friend to this
      // dir; He can't; this is a global directory. He is frustrated.

      // the code is moved to kfm.cpp: static Kfm::setUpDest (QString *)

      QString tryPath(url);      // copy (hope deep)
      if (!getGUI()->sumode) // if not root...
      {
	Kfm::setUpDest(&tryPath);  // check/modify...
	url = tryPath.data();      // return back
      }
      //-------- Sven's write to pseudo global mime/app dirs end ---
      
    KURL u( url );
    if ( u.isMalformed() )
    {
	QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: Drop over malformed URL") );
	return;
    }
    
    // Check whether we drop a directory on itself or one of its children
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
        // Commented out useless warning. David.
	// QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
	// 		      klocale->translate("ERROR: You dropped some URL over itself") );
	return;
    }
    
    QPoint p2( _zone->getMouseX(), _zone->getMouseY() );
    manager->dropPopupMenu( _zone, url, &p2, ( nested == 0 ? false : true ) );

}

void KfmView::copySelection()
{
	// clear the internal clipboard first.
    clipboard->clear();

    // Is there any text selected ?	
    if ( getActiveView()->isTextSelected() )
    {
        QString txt;
		getActiveView()->getSelectedText ( txt );		
        if (!txt.isEmpty())
		{			
            clipboard->append(txt);
			KApplication::clipboard()->setText( txt );
		}
	}
	// If not what about URL(s) ?	
	else
	{
		getActiveView()->getSelected( (*clipboard) );
		// if user clicked on the background w/o selecting anything else.	
		if (clipboard->isEmpty() && popupMenuEvent )
		{
			clipboard->append ( getURL() );
			KApplication::clipboard()->setText( getURL() );
    }
		else
			// Always copy the last selected URL into system clipboard ...
			KApplication::clipboard()->setText( clipboard->last() );
	}
}

void KfmView::slotCopy()
{
	copySelection ();
}

void KfmView::slotCut()
{
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
    // Check whether we drop a directory on itself or one of its children
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
	// Popup menu generated this Event! (Dawit A.)
	popupMenuEvent = true;
    // Show the popup Menu for the given URLs
	debug ( "popup state enabled" );
    manager->openPopupMenu( _urls, _point, _current_dir );
}

void KfmView::slotOpenWith()
{
    QStrList popupFiles = new QStrList();
    getActiveView()->getSelected ( popupFiles ); // get selected URL(s)
    if ( popupFiles.isEmpty() && popupMenuEvent )
{
		popupFiles.append ( getURL() );
    }
    OpenWithDlg l( klocale->translate("Open With:"), "", this, true );
    if ( l.exec() )
    {
      KMimeBind *bind = l.mimeBind();
      if ( bind )
      {
	const char *s;
	for( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
	  bind->runBinding( s );
	return;
      }
      QString pattern = l.getText();
      if ( pattern.length() == 0 )
	return;
    }
    else
      return;
    
    printf("KfmView::slotPopupOpenWith starts openWithOldApplication(%s)\n", l.getText());
    KURL u(popupFiles.first());
    if (u.isLocalFile())
    {
        QString udir(u.directory());
        udir.detach();
        KURL::decodeURL(udir); // I hate KURL, you never know when it's encoded ... David.
        openWithOldApplication( l.getText(), popupFiles, udir ); 
    }
    else  openWithOldApplication( l.getText(), popupFiles );
}              

void KfmView::slotProperties()
{
	QStrList popupFiles = new QStrList();
	getActiveView()->getSelected ( popupFiles );
	if ( popupFiles.isEmpty() && popupMenuEvent )
	{
		popupFiles.append ( getURL() );
	}
    if ( popupFiles.count() != 1 )
    {
	warning(klocale->translate("ERROR: Can not open properties for multiple files"));
	return;
    }

    (void)new Properties( popupFiles.first() );
}

//----------------------------------------------------------------------------
void KfmView::slotSaveLocalProperties()
{
  
  int isADir;
  // Check if this is a dir. If it is writable, write it.
  const char *_url = manager->getURL();

  isADir = strncmp (_url, "file:", 5);
    
  if (!isADir)
  {
    // No way do find out is it writable; have to check:
    if (access(&_url[5], W_OK)==0)
    {
      QString configname(&_url[5]);
      configname.append("/.directory");
      KSimpleConfig cfg(configname.data());
      gui->writeProperties((KConfig *) &cfg); // will sync on end
      return;
    }
  }

  //Dir not writable or not a file: url. See if it is bookmarked
  KBookmark *bm = gui->getBookmarkManager()->findBookmark(_url);
  if (bm)
  {
    KSimpleConfig cfg(bm->file());
    gui->writeProperties((KConfig *) &cfg); // will sync on end
    return;
  }

  //Dir not writable or not a file: url. See if it is on desktop
  if (KRootWidget::pKRootWidget) // if there is one. DF.
  {
      QString s1, s2, kurl;
      s1 = _url;
      if (s1.right( 1 ) == "/")
	s1.resize(s1.length());
      
      for (KRootIcon *i = IconList.first(); i; i = IconList.next())
      {
	  kurl = i->getURL();
	  KURL::decodeURL(kurl); // Decode kdelnk filename
          if (kurl.contains(".kdelnk"))
          {
              debug ("Got a kdelnk: %s", &(kurl.data())[5]);
              KSimpleConfig cfg(&(kurl.data())[5], true); //RO, #inline, so it's fast
	      cfg.setGroup("KDE Desktop Entry");
	      
	      s2 = cfg.readEntry("URL").data();
	      if (s2.right( 1 ) == "/")
		s2.resize(s2.length());

              if (s1 == s2)
	      {
		KSimpleConfig wcfg(&(kurl.data())[5]); //rw
		gui->writeProperties((KConfig *) &wcfg); // will sync on end
		return;
              }
	  }
      }
  }
  // Not found or not writable:
  if (!isADir )
    QMessageBox::warning(0, klocale->translate( "KFM Error" ),
                         klocale->translate("Can not save properties because\nthis directory is neither writable nor bookmarked.\n\n Bookmark this location first, or make kdelnk to it on desktop.") );

  else 
    QMessageBox::warning(0, klocale->translate( "KFM Error" ),
                         klocale->translate("Can not save properties because\nthis URL is neither bookmarked nor on desktop.\n\n Bookmark this location first, or make kdelnk to it on desktop."));
}
//----------------------------------------------------------------------------

void KfmView::slotBookmarks()
{
    char *s;
    QStrList popupFiles = new QStrList();
    getActiveView()->getSelected ( popupFiles ); // get the selected URL(s)
    if ( popupFiles.isEmpty() && popupMenuEvent )
    {
       popupFiles.append ( getURL() );
    }
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	gui->addBookmark( s, s );
}

void KfmView::slotEmptyTrashBin()
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
	    if ( strcmp( ep->d_name, "." ) != 0L && strcmp( ep->d_name, ".." ) != 0L && strcmp( ep->d_name, ".directory" ) != 0L )
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

void KfmView::slotNewView()
{
	char *s;
    QStrList popupFiles = new QStrList();
    getActiveView()->getSelected ( popupFiles ); // get the selected URL(s)
	for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	{
	KfmGui *m = new KfmGui( 0L, 0L, s );
	m->show();
    }
}

const char * KfmView::getURL()
{
    return manager->getURL();
}

//------------------------------------------------------------------------
void KfmView::checkLocalProperties (const char *_url)
{
  
  // Here we read properties. We check if this is a dir with .directory
  // then if this is the bookmarked url, last if this is on the desktop

  if (!KRootWidget::pKRootWidget) // No desktop : the window is being restored
      // by session management. Then use SM values, not local properties.
      return;

  // check if this options are enabled:
  if (!pkfm->isURLPropesEnabled ())
    return;
  
  int isADir;
  // Check if this is a dir. If it is writable, write it.

  isADir = strncmp (_url, "file:", 5);
    
  if (!isADir)
  {
    if (access(&_url[5], R_OK)==0) // we need if we can read (sven)
    {
      QString configname(&_url[5]);
      configname.append("/.directory");
      if (access(configname.data(), F_OK)==0)
      {
        KSimpleConfig cfg(configname.data(), true); // be read only
        gui->loadProperties((KConfig *) &cfg);
        return;
      }
    }
  }

  //.directory not readable or not found. See if it is bookmarked
  KBookmark *bm = gui->getBookmarkManager()->findBookmark(_url);
  if (bm)
  {
    KSimpleConfig cfg(bm->file(), true); //RO
    gui->loadProperties((KConfig *) &cfg); // will sync on end
    return;
  }

  // .directory not readable or not found and not bookmarked.
  // See if it is on desktop
  
  for (KRootIcon *i = IconList.first(); i; i = IconList.next())
  {
    QString kurl = i->getURL();
    KURL::decodeURL(kurl); // Decode kdelnk filename
    if (kurl.contains(".kdelnk"))
      {
          KSimpleConfig cfg(&(kurl.data())[5], true); // RO, #inline, so it's fast
          cfg.setGroup("KDE Desktop Entry");
          if (!strcmp (cfg.readEntry("URL").data(), _url))
          {
              gui->loadProperties((KConfig *) &cfg); // will sync on end
              return;
          }
      }
  }
  // Not found or not readable:
  // Tell gui that there is no URL properties!!!!
  gui->setHasLocal(false);
}
//------------------------------------------------------------------------


void KfmView::openURL( const char *_url, bool _refresh, int _xoffset, int _yoffset )
{
    redirectTimer.stop();
    checkLocalProperties (_url);
    emit newURL( _url );
    manager->openURL( _url, _refresh, _xoffset, _yoffset );
}

void KfmView::openURL( const char *_url )
{
    redirectTimer.stop();

    // dirty hack to get the restoring of frames working correctly
    if( strncmp( _url, "restored:", 9 ) == 0 )
    {
	_url += 9;
	emit newURL(_url);
	return;
    }

    checkLocalProperties (_url);
    emit newURL( _url );
    manager->openURL( _url );
}

void KfmView::openURL( const char *_url, const char *_data )
{
    redirectTimer.stop();

    checkLocalProperties (_url);
    emit newURL( _url );
    manager->openURL( _url, FALSE, 0, 0, _data );
}

void KfmView::pushURLToHistory()
{
    if ( stackLock )
	return;
    
    // printf("pushing to stack\n");
    SavedPage *p = saveYourself();
    if(!p) return;

    backStack->push( p );
    forwardStack->setAutoDelete( true );
    forwardStack->clear();
    forwardStack->setAutoDelete( false );
	
    emit historyUpdate( true, false );
}

void KfmView::setUpURL( const char *_url )
{
  m_strUpURL = _url;

  //CT 16Dec1998 handle View menu
  bool has_upURL = !( m_strUpURL == "/"  || m_strUpURL.isEmpty() );
  gui->enableToolbarButton( 0, has_upURL );
}

void KfmView::setHTMLMode(bool bHtmlMode)
{
  gui->handleViewMenu(bHtmlMode);
}

void KfmView::slotUp()
{
  if ( m_strUpURL.isEmpty() )
    return;
  
  openURL( m_strUpURL, false );
}

void KfmView::slotForward()
{
    if ( forwardStack->isEmpty() )
	return;

    SavedPage *s = forwardStack->pop();
    // try to find the corresponding htmlview. If only this view
    // (the one that changes saves itself, we will reduce flicker
    // a lot...
    
    // we are the top level widget, since we get signals from kfmgui
    KHTMLView *top = (KHTMLView *)this;
    KHTMLView *v;
    for ( v = viewList->first(); v != 0; v = viewList->next() )
    {
	if ( strcmp( v->getFrameName(), s->frameName ) == 0 ) 
	    if( top != v->findView( "_top" ) )
		continue;
	    else
		break;
    }
    if( !v ) v = top;

    SavedPage *p = v->saveYourself();
    backStack->push( p );
    
    if ( forwardStack->isEmpty() )
	emit historyUpdate( true, false );
    else
	emit historyUpdate( true, true );
    
    stackLock = true;
    restore( s );
    stackLock = false;

    delete s;
}

void KfmView::slotBack()
{
    if ( backStack->isEmpty() )
	return;

    SavedPage *s = backStack->pop();
    // try to find the corresponding htmlview. If only this view
    // (the one that changes & saves itself), we will reduce flicker
    // a lot...
    
    // we are the top level widget, since we get signals from kfmgui
    KHTMLView *top = (KHTMLView *)this;
    KHTMLView *v;
    for ( v = viewList->first(); v != 0; v = viewList->next() )
    {
	if ( strcmp( v->getFrameName(), s->frameName ) == 0 ) 
	    if( top != v->findView( "_top" ) )
		continue;
	    else
		break;
    }
    if( !v ) v = top;

    SavedPage *p = v->saveYourself();
    forwardStack->push( p );
    
    if ( backStack->isEmpty() )
	emit historyUpdate( false, true );
    else
	emit historyUpdate( true, true );    

    stackLock = true;
    restore( s );
    stackLock = false;

    delete s;
}

void KfmView::slotRedirect( int _delay, const char * _url )
{
    if ( _delay == 0 )
    {
	stackLock = true;
	openURL( _url );
	stackLock = false;
    } 
    else
    {
	redirectURL = _url;
	redirectDelay = _delay;
	connect( this, SIGNAL( documentDone(KHTMLView *)),
		 this, SLOT( slotDelayedRedirect(KHTMLView *) ) );
    }
}

void KfmView::slotDelayedRedirect(KHTMLView *)
{
    disconnect( this, SIGNAL( documentDone(KHTMLView *)),
		this, SLOT( slotDelayedRedirect(KHTMLView *) ) );
    redirectTimer.start(1000*redirectDelay, true);
}

void KfmView::slotDelayedRedirect2()
{
    stackLock = true;
    openURL( redirectURL );
    stackLock = false;
    redirectURL = "";
    redirectDelay = 0;
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
        KHTMLView *v;
        v = findView( _target);
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
        QString surl =  _url;
        // Delete a trailing '/', for KURL::filename() and for lstat
        surl.detach();
        if ( surl.right(1) == "/" )
            surl.truncate( surl.length() - 1 );
                   
	KMimeType *typ = KMimeType::getMagicMimeType( _url );
	if ( typ )
	    com = typ->getComment( _url );

        KURL url (surl);
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
	text = decodedName.copy(); // copy to change it
	text2 = text;
	text2.detach();
	
        if ( url.isLocalFile() )
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
	  else if ( S_ISDIR( buff.st_mode ) )
          {
	      text += "/  ";
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

void KfmView::slotFormSubmitted( const char *_method, const char *_url, const char *_data )
{   
    // debugT("Form Submitted '%s'\n", _url );
    
    KURL u1( manager->getURL() );
    KURL u2( u1, _url );
    if (strcasecmp(_method, "GET")==0)
    {
       // GET
       QString search_part = u2.searchPart();
       if (!search_part.isNull())
       {
           u2.setSearchPart(search_part + "&" + _data);		
       }
       else
       {
           u2.setSearchPart(_data);		
       }
       
       openURL( u2.url().data() );
    }
    else
    {
       // POST

       openURL( u2.url().data() , _data);    
    }
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
	popupMenuEvent = false;
    
    // Select by drawing a rectangle
    if ( _url == 0L && _mouse->button() == LeftButton )
    {
        // Do not do it on native HTML pages (Dawit A.)
        if ( manager->isHTML() ) return false;
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
	getKHTMLWidget()->setMarker( _url );
	// ignoreMouseRelease = true;
	return true;
    }
    else if ( _url != 0L && _mouse->button() == LeftButton &&
	      ( _mouse->state() & ShiftButton ) == ShiftButton )
    {
        selectedURL = _url;
        getKHTMLWidget()->selectFromMarker( _url );
        return true;
    }
    else if ( _url != 0L && _mouse->button() == LeftButton )
    {
	// We can not do much here, since we dont know whether
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
	// select( 0L, false );  Why are we deselecting URL's on a right click ??? (Dawit A.)
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
    else if ( !selectedURL.isEmpty() && _mouse->button() == LeftButton &&
	 ( _mouse->state() & ShiftButton ) == ShiftButton )
    {
	// This is already done, so we jusrt consume this event
	selectedURL = "";
	return true;
    }
    // The user pressed the mouse over an URL, did no DND and released it
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
    }
    else
    {
	// pixmap.load( KMimeType::getPixmapFileStatic( l.first() ) );
	QString dir = kapp->kde_datadir().copy();
	dir += "/kfm/pics/kmultiple.xpm";
	pixmap.load( dir );
	// TODO  Nice icon for multiple files
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
  QStrList *list = Kfm::history();
  if ( list->find( _url ) != -1 )
    return true;
  
  return false;
}

QStack<SavedPage> *KfmView::getBackStack()
{
    return backStack;
}

QStack<SavedPage> *KfmView::getForwardStack()
{
    return forwardStack;
}

const char * KfmView::getJobURL() 
{
    return manager->getJobURL(); 
}

#include "kfmview.moc"
