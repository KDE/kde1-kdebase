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

#include <Kconfig.h>
#include <kapp.h>

#include "kfmview.h"
#include "kfmdlg.h"
#include "kfmprops.h"
#include "kbutton.h"
#include "root.h"
#include "kfmpaths.h"
#include <config-kfm.h>

#include <klocale.h>

QStrList KfmView::clipboard;

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

    gui = _gui;
 
    dropZone = 0L;
    popupMenu = 0L;
    
    stackLock = false;

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
}

KHTMLView* KfmView::newView( QWidget *_parent, const char *_name, int )
{
    KfmView *v = new KfmView( gui, _parent, _name, this );
    childViewList.append( v );
    return v;
}

KfmView::~KfmView()
{
    debugT("Deleting KfmView\n");
    
    if ( dropZone != 0L )
	delete dropZone;
    dropZone = 0L;
    
    delete manager;
 
    delete htmlCache;
    
    debugT("Deleted\n");
}

void KfmView::begin( const char *_url = 0L, int _x_offset = 0, int _y_offset = 0 )
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
    QString dir = getenv( "HOME" );
    
    KURL u( manager->getURL() );
    if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	dir = u.path();
    
    QString cmd;
    cmd.sprintf( "cd %s; kcli &", dir.data() );
    system( cmd.data() );
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
    cmd.sprintf( "cd %s; %s &", dir.data(), term.data() );
    system( cmd.data() );
}

void KfmView::slotStop()
{
    manager->stop();
}

void KfmView::slotReload()
{
    manager->openURL( manager->getURL(), true );
}

void KfmView::slotUpdateView()
{
    if ( isFrame() )
    {
	KfmView *v;
	for ( v = childViewList.first(); v != 0L; v = childViewList.next() )
	    v->slotUpdateView();
    }
    else
	manager->openURL( manager->getURL(), true );
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

    debugT("Comparing '%s' to '%s'\n",u1.data(), u2.data() );
    if ( u1 == u2 )
	manager->openURL( manager->getURL().data(), true );
    debugT("Changed\n");
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
 
    // Dropped over an object ?
    if ( url != 0L )
    {
	KURL u( url );
	if ( u.isMalformed() )
	{
	    warning( klocale->translate("ERROR: Drop over malformed URL"));
	    return;
	}
	
	// Clear out symlinks if we are on the local hard disk
	QString canonical = url;
	if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	{
	    // Get the canonical path.
	    QDir dir( u.path() );
	    canonical = dir.canonicalPath();
	    if ( canonical.isEmpty() )
		canonical = u.path();
	    fprintf(stderr,"u='%s' can='%s'\n",u.path(),dir.canonicalPath().data());
	}
	
	// Check wether we drop a file on itself
	QStrList list( _zone->getURLList() );
	char *s;
	for ( s = list.first(); s != 0L; s = list.next() )
	{
	    fprintf(stderr,"U2='%s' U1='%s'\n",s,canonical.data());
	    QString url2( s );
	    // replace all symlinks if we are on the local hard disk
	    KURL u2( s );
	    if ( !u2.isMalformed() )
	    {
		if ( strcmp( u2.protocol(), "file" ) == 0 && !u2.hasSubProtocol() )
		{
		    // Get the canonical path.
		    QDir dir2( u2.path() );
		    url2 = dir2.canonicalPath();
		    if ( url2.isEmpty() )
			url2 = u2.path();
		    fprintf(stderr,"2. u='%s' can='%s'\n",u2.path(),dir2.canonicalPath().data());
		}
	    }
	    
	    fprintf(stderr,"U2='%s' U1='%s'\n",url2.data(),canonical.data());
	    
	    // Are both symlinks equal ?
	    if ( strcmp( url2, canonical ) == 0 )
	    {
		QMessageBox::message( klocale->translate( "KFM Error" ),
				      klocale->translate( "You dropped some file over itself" ) );
		return;
	    }
	}
	
	debugT(" Dropped over object\n");
		
	QPoint p( _zone->getMouseX(), _zone->getMouseY() );
	manager->dropPopupMenu( _zone, url, &p );
    }
    else // dropped over white ground
    {
	debugT("Dropped over white\n");
	
	QPoint p( _zone->getMouseX(), _zone->getMouseY() );
	manager->dropPopupMenu( _zone, manager->getURL(), &p );
    }
}

void KfmView::slotCopy()
{
    clipboard.clear();
    view->getSelected( clipboard );
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

    KIOJob * job = new KIOJob;
    bool ok = QMessageBox::query(  klocale->translate("KFM Warning"), 
				   klocale->translate("Dou you really want to delete the files?\n\nThere is no way to restore them"), 
				    klocale->translate("Yes"), 
				    klocale->translate("No") );
    
    if ( ok )
	job->del( marked );
}

void KfmView::slotPaste()
{
    KIOJob * job = new KIOJob;
    job->copy( clipboard, manager->getURL().data() );
}

void KfmView::slotPopupMenu( QStrList &_urls, const QPoint &_point )
{
    // Show the popup Menu for the given URLs
    manager->openPopupMenu( _urls, _point );
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

    QString cmd;
    cmd = l.getText();
    cmd += " ";

    QString tmp;
    
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	cmd += "\"";
	KURL file = s;    
	
	if ( strcmp( file.protocol(), "file" ) == 0L && !file.hasSubProtocol() )
	{
	    QString decoded( file.path() );
	    KURL::decodeURL( decoded );
	    decoded = KIOServer::shellQuote( decoded ).data();
	    cmd += decoded.data();
	}
	else
	{
	    QString decoded( s );
	    KURL::decodeURL( decoded );
	    decoded = KIOServer::shellQuote( decoded ).data();
	    cmd += decoded.data();
	}
	cmd += "\" ";
    }
    debugT("Executing '%s'\n", cmd.data());
    
    KMimeBind::runCmd( cmd.data() );
}              

void KfmView::slotPopupProperties()
{
    if ( popupFiles.count() != 1 )
    {
	warning(klocale->translate("ERROR: Can not open properties for multiple files"));
	return;
    }

    new Properties( popupFiles.first() );
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
		QString t = "file:" + d + ep->d_name;
		trash.append( t.data() );
	    }
	}
	closedir( dp );
    }
    else
    {
      QMessageBox::message( klocale->translate("KFM Error"), 
			    klocale->translate("Could not access Trash Bin") );
	return;
    }
    
    // Delete all trash files
    KIOJob * job = new KIOJob;
    job->del( trash );
}

void KfmView::slotPopupCopy()
{
    clipboard.clear();
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	clipboard.append( s );
}

void KfmView::slotPopupPaste()
{
    if ( popupFiles.count() != 1 )
    {
	QMessageBox::message( klocale->translate("KFM Error"), 
			      klocale->translate("Can not paste in multiple directories") );
	return;
    }
    
    KIOJob * job = new KIOJob;
    job->copy( clipboard, popupFiles.first() );
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
    // This function will emit a signal that causes us to redisplay the
    // contents of our directory if neccessary.
    KIOJob * job = new KIOJob;
    
    // Is the user really shure ?
    bool ok = QMessageBox::query( klocale->translate("KFM Warning"), 
				  klocale->translate("Dou you really want to delete the files?\n\nThere is no way to restore them"), 
				  klocale->translate("Yes"), 
				  klocale->translate("No") );
    if ( ok )
    {
	// Store the decoded URLs here.
	QStrList list;
	char *s;
	for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	{
	    // Decode the URL
	    QString str( s );
	    KURL::decodeURL( str );
	    list.append( str );
	}
	
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

void KfmView::openURL( const char *_url )
{
    manager->openURL( _url );
}

void KfmView::slotURLToStack( const char *_url )
{
    if ( stackLock )
	return;
    
    debugT("Moving '%s' on STACK\n",_url);
    
    QString *s = new QString( _url );
    s->detach();
    backStack.push( s );
    forwardStack.setAutoDelete( true );
    forwardStack.clear();
    forwardStack.setAutoDelete( false );
	
    emit historyUpdate( true, false );
}

void KfmView::slotForward()
{
    if ( forwardStack.isEmpty() )
	return;

    QString *s2 = new QString( manager->getURL() );
    s2->detach();
    backStack.push( s2 );

    QString *s = forwardStack.pop();
    if ( forwardStack.isEmpty() )
	emit historyUpdate( true, false );
    else
	emit historyUpdate( true, true );
    
    stackLock = true;
    openURL( s->data() );
    stackLock = false;

    delete s;
}

void KfmView::slotBack()
{
    if ( backStack.isEmpty() )
	return;
    
    QString *s2 = new QString( manager->getURL() );
    s2->detach();
    forwardStack.push( s2 );
    
    QString *s = backStack.pop();
    if ( backStack.isEmpty() )
	emit historyUpdate( false, true );
    else
	emit historyUpdate( true, true );    

    stackLock = true;
    openURL( s->data() );
    stackLock = false;

    delete s;
}

void KfmView::slotURLSelected( const char *_url, int _button, const char *_target )
{
    debugT("######### Click '%s' target='%s'\n",_url,_target);
 
    if ( !_url )
	return;
    
    if ( _target != 0L && _target[0] != 0 && _button == LeftButton )
    {
	KHTMLView *v = KHTMLView::findView( _target );
	if ( v )
	{
	    debugT("Found Frame\n");
	    v->openURL( _url );
	    return;
	}
	else
	{
	    debugT("New GUI\n");
	    KfmGui *m = new KfmGui( 0L, 0L, _url );
	    debugT("New GUI2\n");
	    m->show();
	    debugT("New GUI3\n");
	    return;
	}
    }
    
    if ( _button == MidButton && KIOServer::isDir( _url ) )
    {
	KURL base( manager->getURL() );
	KURL u( base, _url );
	QString url = u.url();

	KfmGui *m = new KfmGui( 0L, 0L, url.data() );
	m->show();
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
	KURL::decodeURL( decodedPath );
	QString decodedName( url.filename() );
	KURL::decodeURL( decodedName );
	
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
	    KURL::decodeURL( decodedURL );
	    gui->slotSetStatusBar( decodedURL );
	}
    }
}

void KfmView::slotFormSubmitted( const char *, const char *_url )
{   
    debugT("Form Submitted '%s'\n", _url );
    
    KURL u1( manager->getURL() );
    KURL u2( u1, _url );
    
    debugT("Complete is '%s'\n", u2.url().data() );
    
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

bool KfmView::mousePressedHook( const char *_url, const char *, QMouseEvent *_mouse, bool _isselected )
{
    // Select by drawing a rectangle
    if ( _url == 0L && _mouse->button() == LeftButton )
    {
	select( 0L, false );
	rectStart = true;            // Say it is start of drag
	rectX1 = _mouse->pos().x();   // get start position
	rectY1 = _mouse->pos().y();
	rectX2 = rectX1;
	rectY2 = rectY1;
	if ( !dPainter )
	    dPainter = new QPainter;
	debugT ("KFileView::mousePressEvent: starting a rectangle (w/Painter)\n");   
	return true;
    }
    // Select a URL with Ctrl Button
    else if ( _url != 0L && _mouse->button() == LeftButton &&
	      ( _mouse->state() & ControlButton ) == ControlButton )
    {   
	selectByURL( 0L, _url, !_isselected );
	ignoreMouseRelease = true;
	return true;
    }
    else if ( _url != 0L && _mouse->button() == LeftButton )
    {
	QStrList list;
	getSelected( list );

	// The user selected the first icon
	if ( list.count() == 0 )
	{
	    selectByURL( 0L, _url, true );
	    return false;
	}
	// The user selected one of the icons that are already selected
	if ( list.find( _url ) != -1 )
	    return false;
	// The user selected another icon => deselect the selected ones
	select( 0L, false );
	selectByURL( 0L, _url, true );
	return false;
    }
    // Context Menu
    else if ( _url != 0L && _mouse->button() == RightButton )
    {   
	QPoint p = mapToGlobal( _mouse->pos() );
	
	// A popupmenu for a list of URLs or just for one ?
	QStrList list;
	getSelected( list );
	char* s;
	for ( s = list.first(); s != 0L; s = list.next() )
	    debugT(" Entry '%s'\n",s);
    
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
	slotPopupMenu( list, p );
	return true;
    }
    
    return false;
}

bool KfmView::mouseMoveHook( QMouseEvent *_mouse )
{
    if ( rectStart )
    {
	int x = _mouse->pos().x();
	int y = _mouse->pos().y();
	
	if ( !dPainter )
	{
	    debugT ("KFileView::mouseMoveEvent: no painter\n");
	    return true;
	}
	dPainter->begin( view );
	dPainter->setRasterOp (NotROP);
	dPainter->drawRect (rectX1, rectY1, rectX2-rectX1, rectY2-rectY1);
	dPainter->end();
	
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
	
	debugT ("KFileView::dndMouseMoveEvent: end of draging; x1,y1,x2,y2 = %d,%d,%d,%d\n",
		x1, y1, x, y);
	QRect rect( x1, y1, x - x1 + 1, y - y1 + 1 );
	select( 0L, rect );
	
	rectX2 = _mouse->pos().x();
	rectY2 = _mouse->pos().y();
	
	dPainter->begin( view );
	dPainter->setRasterOp (NotROP);
	dPainter->drawRect (rectX1, rectY1, rectX2-rectX1, rectY2-rectY1);
	dPainter->end();
	
	return true;
    }
  
    return false;
}

bool KfmView::mouseReleaseHook( QMouseEvent *_mouse )
{
    if ( ignoreMouseRelease )
    {
	ignoreMouseRelease = false;
	return true;
    }
    
    if ( rectStart )
    {
	rectX2 = _mouse->pos().x();
	rectY2 = _mouse->pos().y();
	if ( ( rectX2 == rectX1 ) && ( rectY2 == rectY1 ) )
	{
	    select( 0L, false );
	    debugT ("KFileView::mouseReleaseEvent: it was just a dream... just a dream.\n");
	}
	else
	{
	    /* ...find out which objects are in(tersected with) rectangle and select
	       them, but remove rectangle first. */
	    dPainter->begin( view );
	    dPainter->setRasterOp (NotROP);
	    dPainter->drawRect (rectX1, rectY1, rectX2-rectX1, rectY2-rectY1);
	    dPainter->end();
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
	    
	    debugT ("KFileView::dndMouseReleaseEvent: end of draging; x1,y1,x2,y2 = %d,%d,%d,%d\n",
		    rectX1, rectY1, rectX2, rectY2);
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
    if ( _url == 0L )
	return true;
    
    QStrList l;
    getSelected( l );
    
    QPixmap pixmap;
    
    // Is anything selected at all ?
    if ( l.count() == 0 )
	l.append( _url );
    
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
	QString dir = kapp->kdedir();
	dir += "/share/apps/kfm/pics/kmultiple.xpm";
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

#include "kfmview.moc"
