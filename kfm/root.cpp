#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <errno.h>

#include <qdir.h>
#include <qtooltip.h>
#include <qrect.h>
#include <qmsgbox.h>

#include <kapp.h>

#include "root.h"
#include "kfmprops.h"
#include "kfmdlg.h"
#include "kfmpaths.h"
#include "kfmexec.h"
#include <config-kfm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <string.h>

#include <klocale.h>

#define root KRootWidget::getKRootWidget()

KRootWidget* KRootWidget::pKRootWidget = 0L;

KRootWidget::KRootWidget( QWidget *parent=0, const char *name=0 ) : QWidget( parent, name )
{
    KConfig *config = KApplication::getKApplication()->getConfig();

    if ( config )
    {
         config->setGroup("KFM Root Icons");
	 iconstyle = config->readNumEntry( "Style", 1 );
	 QString bg = config->readEntry( "Background" );
	 if ( bg.isNull() )
	   bg = "black";
         iconBgColor.setNamedColor( bg.data() );
	 QString fg = config->readEntry( "Foreground" );
	 if ( fg.isNull() )
	   fg = "white";
         labelColor.setNamedColor( fg.data() );
    }

    rootDropZone = new KDNDDropZone( this , DndURL );
    connect( rootDropZone, SIGNAL( dropAction( KDNDDropZone *) ),
	     this, SLOT( slotDropEvent( KDNDDropZone *) ) );
    KApplication::getKApplication()->setRootDropZone( rootDropZone );

    popupMenu = new QPopupMenu();
    connect( popupMenu, SIGNAL( activated( int )), this, SLOT( slotPopupActivated( int )) );
    
    noUpdate = false;
    
    pKRootWidget = this;
    
    desktopDir = "file:" + KFMPaths::DesktopPath();
    
    connect( KIOServer::getKIOServer(), SIGNAL( notify( const char *) ),
    	     this, SLOT( slotFilesChanged( const char *) ) );
    connect( KIOServer::getKIOServer(), SIGNAL( mountNotify() ), 
	     this, SLOT( update() ) );

    icon_list.setAutoDelete( true );
    layoutList.setAutoDelete( true );

    update();
}

bool KRootWidget::isBindingHardcoded( const char *_txt )
{
    if ( strcmp( klocale->getAlias(ID_STRING_CD), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_NEW_VIEW), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_COPY), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_DELETE), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_MOVE_TO_TRASH), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_PASTE), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_OPEN_WITH), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_CUT), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_MOVE), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_PROP), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_LINK), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_OPEN), _txt ) == 0 )
	return true;
    if ( strcmp( klocale->getAlias(ID_STRING_TRASH), _txt ) == 0 )
	return true;

    return false;
}

void KRootWidget::slotPopupActivated( int _id )
{
    if ( popupMenu->text( _id ) == 0 )
	return;
    
    // Text of the menu entry
    QString txt = popupMenu->text( _id );
    
    // Is this some KFM internal stuff ?
    if ( isBindingHardcoded( txt ) )
	return;

    // Loop over all selected files
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	// debugT("Exec '%s'\n", s );
	// Run the action 'txt' on every single file
	KMimeBind::runBinding( s, txt );    
    }
}

void KRootWidget::openPopupMenu( QStrList &_urls, const QPoint &_point )
{
    if ( _urls.count() == 0 )
	return;
    
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	KURL u( s );
	if ( u.isMalformed() )
	    return;
    }
    
    popupMenu->clear();
    // store the mouse position. (Matthias)
    popupMenuPosition = QCursor::pos();   
    
    QStrList bindings;
    bindings.setAutoDelete( true );
    QStrList bindings2;
    bindings2.setAutoDelete( true );
    
    // char buffer[ 1024 ];
    
    bool isdir = KIOServer::isDir( _urls );
    bool istrash = KIOServer::isTrash( _urls );
    
    if ( istrash )
    {
	bool isempty = true;
        DIR *dp;
        struct dirent *ep;
        dp = opendir( _urls.first() + 5 );
	if ( dp )
	{
	    ep=readdir( dp );
	    ep=readdir( dp );      // ignore '.' and '..' dirent
	    if ( readdir( dp ) == 0L ) // third file is NULL entry -> empty directory
		isempty = false;
	    closedir( dp );
	}

	int id;
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_OPEN), this, 
				    SLOT( slotPopupNewView() ) );
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_TRASH), this, 
				    SLOT( slotPopupEmptyTrash() ) );
	if ( !isempty )
	    popupMenu->setItemEnabled( id, false );
    }
    else if ( isdir )
    {
	int id;
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_NEW_VIEW), this, 
				    SLOT( slotPopupNewView() ) );
	id = popupMenu->insertItem(  klocale->getAlias(ID_STRING_COPY), this, 
				    SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem(  klocale->getAlias(ID_STRING_MOVE_TO_TRASH),  this, 
				    SLOT( slotPopupTrash() ) );
	id = popupMenu->insertItem(  klocale->getAlias(ID_STRING_DELETE), 
				     this, SLOT( slotPopupDelete() ) );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_OPEN_WITH),
				    this, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_COPY), this, 
				    SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_MOVE_TO_TRASH),  this, 
				    SLOT( slotPopupTrash() ) );
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_DELETE), this, 
				    SLOT( slotPopupDelete() ) );
    }
    
    popupFiles.copy( _urls );
    
    // Get all bindings matching all files.
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	// If this is the first file in the list, assume that all bindings are ok
	if ( s == _urls.getFirst() )
	{
	    KMimeType::getBindings( bindings, s, isdir );
	}
	// Take only bindings, matching all files.
	else
	{
	    KMimeType::getBindings( bindings2, s, isdir );
	    char *b;
	    // Look thru all bindings we have so far
	    for ( b = bindings.first(); b != 0L; b = bindings.next() )
		// Does the binding match this file, too ?
		if ( bindings2.find( b ) == -1 )
		    // If not, delete the binding
		    bindings.removeRef( b );
	}
    }
	
    if ( !bindings.isEmpty() )
    {
	popupMenu->insertSeparator();

	char *str;
	for ( str = bindings.first(); str != 0L; str = bindings.next() )
	{
	    popupMenu->insertItem( str );
	}
    }

    if ( _urls.count() == 1 )
    {
	popupMenu->insertSeparator();
	popupMenu->insertItem( klocale->getAlias(ID_STRING_PROP), 
			       this, SLOT( slotPopupProperties() ) );
    }
    
    popupMenu->popup( _point );
}

void KRootWidget::openURL( const char *_url )
{
    KFMExec *e = new KFMExec();
    e->openURL( _url );
}

void KRootWidget::moveIcons( QStrList &_urls, QPoint &p )
{
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	KRootIcon *icon = findIcon( s );
	if ( icon != 0L )
	{
	    icon->move( icon->x() + p.x() - icon->getDndStartPos().x(),
			icon->y() + p.y() - icon->getDndStartPos().y() );
	}
    }

    saveLayout();
}

void KRootWidget::selectIcons( QRect &_rect )
{
  KRootIcon *icon;
  for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
    if ( icon->geometry().intersects( _rect ) )
      icon->select( TRUE );
}

void KRootWidget::unselectAllIcons( )
{
  KRootIcon *icon;
  for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
    icon->select( FALSE );
}

void KRootWidget::getSelectedURLs( QStrList &_list )
{
  KRootIcon *icon;
  for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
      if ( icon->isSelected() )
	  _list.append( icon->getURL() );
}

QString KRootWidget::getSelectedURLs()
{
    QStrList list;
    getSelectedURLs( list );
    
    QString erg( "" );

    bool first = TRUE;
    char *s;
    for ( s = list.first(); s != 0L; s = list.next() )
    {
	if ( !first )
	    erg += "\n";
	erg += s;
	first = FALSE;
    }
    
    return erg;
}

void KRootWidget::saveLayout()
{
    KRootIcon *icon;

    QString file = QDir::homeDirPath().data();
    file += "/.kde/share/apps/kfm/desktop";
    
    FILE *f = fopen( file.data(), "w" );
    if ( f != 0 )
    {
	
	for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
	    fprintf( f, "%s;%i;%i;%i;\n",icon->getURL(),icon->getType(),icon->saveX(),icon->saveY() );
	
	fclose( f );
    }
}

void KRootWidget::loadLayout()
{
    layoutList.clear();
    
    QString file = QDir::homeDirPath().data();
    file += "/.kde/share/apps/kfm/desktop";
 
    FILE *f = fopen( file.data(), "r" );
    if ( f != 0 )
    {
	char buffer[1024];
	while ( !feof( f ) )
	{
	    buffer[ 0 ] = 0;
	    fgets( buffer, 1024, f );
	    if ( buffer[ 0 ] != 0 )
	    {
		const char *p = buffer;
		char *p2 = strchr( p, ';' );
		*p2++ = 0;
		QString u = p;
		p = p2;
		p2 = strchr( p, ';' );
		*p2++ = 0;
		QString t = p;
		p = p2;
		p2 = strchr( p, ';' );
		*p2++ = 0;
		QString x = p;
		p = p2;
		p2 = strchr( p, ';' );
		*p2++ = 0;
		QString y = p;
		KRootLayout *l = new KRootLayout( u.data(), atoi( t.data() ), atoi( x.data() ), atoi( y.data() ) );
		layoutList.append( l );
	    }
	}

	fclose( f );
    }
}

KRootIcon* KRootWidget::findIcon( const char *_url )
{
    KRootIcon *icon;
    
    for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
    {
	if ( strcmp( icon->getURL(), _url ) == 0 )
	    return icon;
    }

    return 0L;
}

QPoint KRootWidget::findLayout( const char *_url )
{
    KRootLayout *icon;
    
    for ( icon = layoutList.first(); icon != 0L; icon = layoutList.next() )
    {
	if ( strcmp( icon->getURL(), _url ) == 0 )
	    return QPoint( icon->getX(), icon->getY() );
    }

    return QPoint( 0, 0 );
}

QPoint KRootWidget::findFreePlace( int _width, int _height )
{
    int dwidth = XDisplayWidth( KApplication::getKApplication()->getDisplay(), 0 );
    int dheight = XDisplayHeight( KApplication::getKApplication()->getDisplay(), 0 );

    KRootIcon *icon;

    for ( int x = 0; x <= dwidth - ROOT_GRID_WIDTH; x += ROOT_GRID_WIDTH )
    {
	for ( int y = 80; y <= dheight - ROOT_GRID_HEIGHT; y += ROOT_GRID_HEIGHT )
	{	
	    bool isOK = true;
	    
	    for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
	    {
		QRect r1( x, y, _width, _height );
		QRect r2( icon->x(), icon->y(), icon->QWidget::width(),icon->QWidget::height() );
		if ( r1.intersects( r2 ) == false )
		{
		    QRect r3( x + ( ROOT_GRID_WIDTH - _width ) / 2, y, _width, _height );
		    if ( r2.intersects( r3 ) == true )
			isOK = false;
		}
		else
		    isOK = false;
	    }
	    if ( isOK )
		return QPoint( x + ( ROOT_GRID_WIDTH - _width ) / 2, y );
	}
    }

    return QPoint( 0, 0 );
}

void KRootWidget::sortIcons()
{
    KRootIcon *icon;

    for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
	if ( ( icon->x() - ( ROOT_GRID_WIDTH - icon->QWidget::width() ) / 2 )  % ROOT_GRID_WIDTH != 0 ||
	     icon->y() % ROOT_GRID_HEIGHT != 0 )
	    icon->move( -200, -200 );
	
    QListIterator<KRootIcon> it( icon_list );    
    for ( ; it.current(); ++it )
	if ( it.current()->y() == -200 )
	{
	    QPoint p = findFreePlace( it.current()->QWidget::width(), it.current()->QWidget::height() );
	    it.current()->move( p );
	}
    
    saveLayout();
}

/*
 * TODO: Wenn ein Icon auf dem Schirm aber nicht im Dateisystem ist
 * muss es geloescht werden
 */
void KRootWidget::update()
{
    if ( noUpdate )
	return;
    
    loadLayout();
    
    KURL u ( KFMPaths::DesktopPath() ); // KURL::KURL adds a file:

    if ( u.isMalformed() )
    {
	warning(klocale->translate("Internal Error: desktopDir is malformed"));
	return;
    }
    
    DIR *dp;
    struct dirent *ep;
    
    dp = opendir( u.path() );
    
    if ( dp == NULL )
    {
	// debugT("'%s'\n",u.path());
	warning(klocale->translate("ERROR: Could not read desktop directory '%s'"), desktopDir.data());
	exit(1);
    }
    
    QList<KRootIcon> found_icons;
    found_icons.setAutoDelete( false );

    // go thru all files in ~/Desktop.
    while ( ( ep = readdir( dp ) ) != 0L )
    {
	// We are not interested in '.' and '..'
	if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
	{   
	    KRootIcon *icon;
	    // bool ok = false;

	    QString file = desktopDir;
	    file.detach();
	    file += ep->d_name;

	    icon = findIcon( file.data() );
	    	    
	    // This icon is missing on the screen.
	    if ( icon == 0L )
	    {
		KRootIcon *icon = 0L;
		// KMimeType *typ = KMimeType::getMagicMimeType( file.data() );
		QPoint p = findLayout( file );
		if ( p.x() == 0 && p.y() == 0 )
		{
		    icon = new KRootIcon( file, -200, -200 );
		    p = findFreePlace( icon->QWidget::width(), icon->QWidget::height() );
		    icon->move( p );
		}
		else
		    icon = new KRootIcon( file, p.x(), p.y() );
		icon_list.append( icon );
		found_icons.append( icon );
	    }	    
	    else  // Icon is already there
	    {
		icon->updatePixmap(); 
		found_icons.append( icon );
	    }
	}
    }
    (void) closedir( dp );
    
    KRootIcon *icon;
    // Delete all icons on the screen that appear not in the file system.
    for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
    {
	if ( found_icons.findRef( icon ) == -1 )
	    icon_list.remove( icon );
    }
}

void KRootWidget::slotDropEvent( KDNDDropZone *_zone )
{
    dropZone = _zone;
    dropFileX = _zone->getMouseX();
    dropFileY = _zone->getMouseY();
    
    popupMenu->clear();

    QStrList & list = _zone->getURLList();
    char *s;
    for ( s = list.first(); s != 0L; s = list.next() )
    {
	KURL u( s );
	if ( !u.isMalformed() )
	{
	    if ( strcmp( u.directoryURL(), desktopDir.data() ) == 0 )
	    {
		QPoint p( _zone->getMouseX(), _zone->getMouseY() );
		moveIcons( list, p );
		return;
	    }
	}
    }
    
    int id = 1;
    // Ask wether we can read from the dropped URL.
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( desktopDir.data(), KIO_Write ) )
	id = popupMenu->insertItem( "Copy", this, SLOT( slotDropCopy() ) );
    // Ask wether we can read from the URL and delete it afterwards
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( _zone->getURLList(), KIO_Delete ) &&
	 KIOServer::supports( desktopDir.data(), KIO_Write ) )
	id = popupMenu->insertItem( "Move", this, SLOT( slotDropMove() ) );
    // We can link everything on the local file system
    if ( KIOServer::supports( desktopDir.data(), KIO_Link ) )
	id = popupMenu->insertItem( "Link", this, SLOT( slotDropLink() ) );
    if ( id == -1 )
    {
        warning(klocale->translate("ERROR: Can not accept drop"));
	return;
    }

    popupMenu->popup( QPoint( dropFileX, dropFileY ) );
}

void KRootWidget::dropUpdateIcons( int _x, int _y )
{
    KURL u ( desktopDir.data() );
    if ( u.isMalformed() )
    {
	warning(klocale->translate("Internal Error: desktopDir is malformed"));
	exit(2);
    }
    
    DIR *dp;
    struct dirent *ep;
    // debugT("Opening '%s'\n",u.path());
    
    dp = opendir( u.path() );
    if ( dp == NULL )
    {
        warning(klocale->translate("ERROR: Could not read desktop directory"));
	noUpdate = false;
	return;
    }
    
    QList<KRootIcon> found_icons;
    found_icons.setAutoDelete( false );

    // go thru all files in ~/Desktop. Search for new files. We assume that they
    // are a result of the drop. This may be wrong, but it is the best method yet.
    while ( ( ep = readdir( dp ) ) != 0L )
    {
	// We are not interested in '.' and '..'
	if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
	{   
	    KRootIcon *icon;
	    // bool ok = false;

	    QString file = desktopDir;
	    file.detach();
	    file += ep->d_name;

	    icon = findIcon( file.data() );
	    	    
	    // This icon is missing on the screen.
	    if ( icon == 0L )
	    {
		KMimeType *typ = KMimeType::getMagicMimeType( file.data() );
		// KMimeType *typ = KMimeType::findType( file.data() );
		QPixmap *pix = typ->getPixmap( file.data() );
		KRootIcon *icon = new KRootIcon( file, _x - pix->width() / 2, _y - pix->height() / 2 );
		icon_list.append( icon );
		found_icons.append( icon );
	    }	    
	}
    }
    (void) closedir( dp );

    saveLayout();
}

//--------------------------------------------
// KIORootJob
//--------------------------------------------

KIORootJob::KIORootJob( KRootWidget *_root, int _x, int _y ) : KIOJob()
{
    rootWidget = _root;
    dropFileX = _x;
    dropFileY = _y;
}

void KIORootJob::slotDropNotify( int , const char *_url )
{
    QString u = _url;
    u.detach();
    if ( u.right( 1 ) != "/" )
	u += "/";
    
    if ( rootWidget->desktopDir != _url )
	KIOServer::sendNotify( _url );
    else
	rootWidget->dropUpdateIcons( dropFileX, dropFileY );
}

//--------------------------------------------

void KRootWidget::slotDropCopy()
{
    // Create a job
    KIORootJob * job = new KIORootJob( this, dropFileX, dropFileY );
    
    // While copying, signals are emitted that cause the desktop to be
    // updated. We want to avoid this, so tell the job, not to emit such
    // signals to everybody. Instead the function, connected to the 'notify' signal of the
    // job will do check wether the notify will cause the KRootWidget to update or
    // not. if not, the notify will be made public, if yet a procedure will update
    // the icons.
    job->noGlobalNotify();
    connect( job, SIGNAL( notify( int, const char* ) ), job, SLOT( slotDropNotify( int, const char* ) ) );    

    job->copy( dropZone->getURLList(), desktopDir.data() );    
}

void KRootWidget::slotDropMove()
{
    // Create a job
    KIORootJob * job = new KIORootJob( this, dropFileX, dropFileY );
    
    job->noGlobalNotify();
    connect( job, SIGNAL( notify( int, const char* ) ), job, SLOT( slotDropNotify( int, const char* ) ) );    

    job->move( dropZone->getURLList(), desktopDir.data() );
}

void KRootWidget::slotDropLink()
{
    // Create a job
    KIORootJob * job = new KIORootJob( this, dropFileX, dropFileY );
    
    job->noGlobalNotify();
    connect( job, SIGNAL( notify( int, const char* ) ), job, SLOT( slotDropNotify( int, const char* ) ) );    

    job->link( dropZone->getURLList(), desktopDir.data() );
}

void KRootWidget::slotFilesChanged( const char *_url )
{
    QString tmp = _url;
    if ( tmp.right(1) != "/" )
	tmp += "/";
    
    if ( tmp == desktopDir )
    {
	update();
	return;
    }
    
    // Calculate the path of the trash bin
    QString d = KFMPaths::TrashPath();

    KURL u( _url );
    if ( u.isMalformed() )
	return;
   
     QString path = u.path();
     if ( path.right(1) != "/" )
     	 path += "/";
    // Update the icon for the trash bin ?
    if ( strcmp( u.protocol(), "file" ) == 0L && 
	 KFMPaths::TrashPath() == path )
    {
	KRootIcon *icon;
	// Find the icon representing the trash bin
	for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
	{
	  /* Stephan: I don't know, why Torben did this in this way:
	    QString t = icon->getURL();
	    if ( t.right(1) != "/" )
		t += "/";
	    if ( t.right( 7 ) == "/Trash/" )
		icon->updatePixmap();
		*/
	  // Stephan: my replacement
	  path = icon->getURL();
	  if ( path.right(1) != "/" )
	    path += "/";
	  QString path2 = QString("file:") + KFMPaths::TrashPath();
	  if ( path2 == path )
	    icon->updatePixmap();
	}
    }
}

void KRootWidget::slotPopupOpenWith()
{
    DlgLineEntry l( "Open With:", "", this, true );
    if ( l.exec() )
    {
	QString pattern = l.getText();
	if ( pattern.length() == 0 )
	    return;
    }
    else
	return;
    
    QString cmd;
    cmd = l.getText();
    cmd += " ";

    QString tmp;
    
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	cmd += "\"";
	KURL file = popupFiles.first();    
	if ( strcmp( file.protocol(), "file" ) == 0L )
	{
	    tmp = KIOServer::shellQuote( file.path() ).copy();
	    cmd += tmp.data();
	}
	else
	{
	    tmp = KIOServer::shellQuote( file.url().data() ).copy();
	    cmd += tmp.data();
	}
	cmd += "\" ";
    }
    
    debugT("Executing '%s'\n", cmd.data());    
    KMimeBind::runCmd( cmd.data() );
}              

void KRootWidget::slotPopupProperties()
{
    if ( popupFiles.count() != 1 )
    {
	warning(klocale->translate("ERROR: Can not open properties for multiple files") );
	return;
    }
    
    Properties *p = new Properties( popupFiles.first() );
    connect( p, SIGNAL( propertiesChanged( const char *, const char * ) ), this,
	     SLOT( slotPropertiesChanged( const char *, const char* ) ) );
    saveLayout();
}

void KRootWidget::slotPropertiesChanged( const char *_url, const char *_new_name )
{
    // Check for renamings.
    if ( _new_name != 0L )
    {
	KRootIcon *icon = findIcon( _url );
	if ( icon == 0L )
	    return;
	icon->rename( _new_name );
	saveLayout();
    }
}

void KRootWidget::slotPopupCopy()
{
    KfmView::clipboard.clear();
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	KfmView::clipboard.append( s );
}

void KRootWidget::slotPopupTrash()
{
    KIOJob * job = new KIOJob;
    
    QString dest = "file:" + KFMPaths::TrashPath();
    job->move( popupFiles, dest );
}

void KRootWidget::slotPopupDelete()
{
    KIOJob * job = new KIOJob;
 
    bool ok = QMessageBox::warning( 0, klocale->translate("KFM Warning"), 
				  klocale->translate("Do you really want to delete the selected file(s)?\n\nThere is no way to restore them."), 
				  klocale->translate("No"), 
				  klocale->translate("Yes") );
    
    if ( ok )
	job->del( popupFiles );
}

void KRootWidget::slotPopupNewView()
{
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	KfmGui *m = new KfmGui( 0L, 0L, s );
	m->show();
    }
}

void KRootWidget::slotPopupEmptyTrash()
{
    QString d = KFMPaths::TrashPath();
    QStrList trash;
    
    DIR *dp;
    struct dirent *ep;
    dp = opendir( d );
    if ( dp )
    {
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
	QMessageBox::warning( 0, klocale->translate( "KFM Error"),
			      klocale->translate( "Could not access Trash Bin") );
	return;
    }
    
    KIOJob * job = new KIOJob;
    job->del( trash );
}

/*********************************************************************
 *
 * KRootIcon
 *
 *********************************************************************/

KRootIcon::KRootIcon( const char *_url, int _x, int _y ) :
    KDNDWidget( 0L, 0L, WStyle_Customize | WStyle_Tool | WStyle_NoBorder )
{
    bSelected = FALSE;
  
    popupMenu = new QPopupMenu();
    
    url = _url;
    url.detach();

    KMimeType *typ = KMimeType::getMagicMimeType( url );
    pixmap = typ->getPixmap( url );
    
    init();
    initToolTip();
    
    dropZone = new KDNDDropZone( this , DndURL);
    connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ), this, SLOT( slotDropEvent( KDNDDropZone *) ) );

    // connect( drop_zone, SIGNAL( dropEnter( KDNDDropZone *) ), this, SLOT( slotDropEnterEvent( KDNDDropZone *) ) );
    // connect( drop_zone, SIGNAL( dropLeave( KDNDDropZone *) ), this, SLOT( slotDropLeaveEvent( KDNDDropZone *) ) );

    setGeometry( _x - pixmapXOffset, _y, width, height );
    show();
    lower();
}

void KRootIcon::initToolTip()
{
    // Does not work due to a Qt bug.
    /*    KFileType *typ = KFileType::findType( url.data() );
    QString com = typ->getComment( url.data() );
    com.detach();
    if ( !com.isNull() )
	QToolTip::add( this, com.data() ); */
}

void KRootIcon::init()
{
    QPainter p;
    p.begin( this );
    
    int pos = url.findRev( "/" );
    file = url.mid( pos + 1, url.length() );
    if ( file.find( ".kdelnk" ) == ((int)file.length()) - 7 )
	file = file.left( file.length() - 7 );
    file.detach();
    
    int ascent = p.fontMetrics().ascent();
    int descent = p.fontMetrics().descent();
    int width = p.fontMetrics().width( file.data() );

    int w = width;
    if ( pixmap->width() > w )
    {
	w = pixmap->width() + 8;
	textXOffset = ( w - width ) / 2;
	pixmapXOffset = 4;
    }
    else
    {
	w += 8;
	textXOffset = 4;
	pixmapXOffset = ( w - pixmap->width() ) / 2;
    }

    pixmapYOffset = 2;
    textYOffset = pixmap->height() + 5 + ascent + 2;

    p.end();

    mask.resize( w, pixmap->height() + 5 + ascent + descent + 4 );
    mask.fill( color0 );
    QPainter p2;
    p2.begin( &mask );
    p2.setFont( font() );
    
    if ( root->iconStyle() == 1 && !bSelected )
       p2.drawText( textXOffset, textYOffset, file );
    else
       p2.fillRect( textXOffset-1, textYOffset-ascent-1, width+2, ascent+descent+2, color1 );     

    if ( pixmap->mask() == 0L )
	p2.fillRect( pixmapXOffset, pixmapYOffset, pixmap->width(), pixmap->height(), color1 );
    else
	bitBlt( &mask, pixmapXOffset, pixmapYOffset, pixmap->mask() );
    
    p2.end();

    if ( bSelected )
      setBackgroundColor( black );
    else
      setBackgroundColor( root->iconBackground() );     

    this->width = w;
    this->height = pixmap->height() + 5 + ascent + descent + 4;
}

void KRootIcon::slotDropEvent( KDNDDropZone *_zone )
{	
    QPoint p( _zone->getMouseX(), _zone->getMouseY() );

    QStrList & list = _zone->getURLList();

    // Dont drop icons on themselves.
    if ( list.count() != 0 )
    {
	char *s;
	for ( s = list.first(); s != 0L; s = list.next() )
	{
	    if ( strcmp( s, url.data() ) == 0 )
	    {   
		root->moveIcons( list, p );
		return;
	    }
	}
    }

    if ( KIOServer::isTrash( url.data() ) )
    {
	KIOJob * job = new KIOJob;

	QString dest = "file:" + KFMPaths::TrashPath();
	job->move( list, dest );
	return;
    }
    
    dropPopupMenu( _zone, url.data(), &p );
}

void KRootIcon::resizeEvent( QResizeEvent * )
{
    XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0, mask.handle(), ShapeSet );
}

void KRootIcon::paintEvent( QPaintEvent * ) 
{
    bitBlt( this, pixmapXOffset, pixmapYOffset, pixmap );
  
    QPainter p;
    p.begin( this );
  
    p.setPen( root->labelForeground() );   
    p.drawText( textXOffset, textYOffset, file );
    
    if ( bSelected )
    {
      p.setRasterOp( NotEraseROP );
      p.fillRect( pixmapXOffset, pixmapYOffset, pixmap->width(), pixmap->height(), blue );
    }
    
    p.end();
}

void KRootIcon::mousePressEvent( QMouseEvent *_mouse )
{
    // Does the user want to select the icon ?
    if ( _mouse->button() == LeftButton && ( _mouse->state() & ControlButton ) == ControlButton )
    {
      select( !bSelected );
    }
    else if ( _mouse->button() == LeftButton )
    {
	// This might be the start of a drag. So we save
	// the position and set a flag.
	pressed = true;
	press_x = _mouse->pos().x();
	press_y = _mouse->pos().y();    
	dndStartPos = mapToGlobal( QPoint( press_x, press_y ) );
    }
    else if ( _mouse->button() == RightButton )
    {
	QStrList list;

        if ( !bSelected )
        {    
	  root->unselectAllIcons();
	  list.append( url.data() );
	}
	else
        {
	  root->getSelectedURLs( list );
	}
	
	QPoint p = mapToGlobal( _mouse->pos() );
	root->openPopupMenu( list, p );
    }
}

void KRootIcon::mouseDoubleClickEvent( QMouseEvent * )
{
    // root->getManager()->openURL( url.data() );
    pressed = false;
}

/*
 * Called, if the widget initiated a drag, after the mouse has been released.
 */
void KRootIcon::dragEndEvent()
{
    // Used to prevent dndMouseMoveEvent from initiating a new drag before
    // the mouse is pressed again.
    pressed = false;
}

void KRootIcon::dndMouseReleaseEvent( QMouseEvent *_mouse )
{
    if (pressed == false)
      return;

    if ( _mouse->button() != LeftButton || ( _mouse->state() & ControlButton ) == ControlButton )
      return;

    root->unselectAllIcons();
    
    root->openURL( url.data() );                          

    pressed = false;
}

void KRootIcon::dndMouseMoveEvent( QMouseEvent *_mouse )
{
    if ( !pressed )
	return;
    
    int x = _mouse->pos().x();
    int y = _mouse->pos().y();

    if ( abs( x - press_x ) > Dnd_X_Precision || abs( y - press_y ) > Dnd_Y_Precision )
    {
	QString data = root->getSelectedURLs();

	QPixmap pixmap2;
	// No URL selected ?
	if ( data.isEmpty() )
	    data = url;
	else 
	{
	    // Multiple URLs ?
	    if ( data.find( '\n' ) != -1 )
	    {
		QString tmp = kapp->kdedir();
		tmp += "/share/apps/kfm/pics/kmultiple.xpm";
		pixmap2.load( tmp );
		if ( pixmap2.isNull() )
		    warning("KFM: Could not find '%s'\n",tmp.data());
	    }
	}
	
	QPoint p = mapToGlobal( _mouse->pos() );
	QPoint p2 = mapToGlobal( QPoint( press_x, press_y ) );
	int dx = QWidget::x() - p2.x() + pixmapXOffset;
	int dy = QWidget::y() - p2.y() + pixmapYOffset;

	// Multiple URLs ?
	if ( !pixmap2.isNull() )
	    startDrag( new KDNDIcon( pixmap2, p.x() + dx, p.y() + dy ), data.data(), data.length(), DndURL, dx, dy );
	else
	    startDrag( new KDNDIcon( *pixmap, p.x() + dx, p.y() + dy ), data.data(), data.length(), DndURL, dx, dy );
    }
}

void KRootIcon::updatePixmap()
{
    QPixmap *tmp = pixmap;
    
    KMimeType *typ = KMimeType::getMagicMimeType( url.data() );
    pixmap = typ->getPixmap( url );
    
    if ( pixmap == tmp )
	return;

    QToolTip::remove( this );
    initToolTip();
    
    init();
    setGeometry( x(), y(), width, height );
    repaint();
}

void KRootIcon::rename( const char *_new_name )
{
    int pos = url.findRev( "/" );
    // Should never happen
    if ( pos == -1 )
	return;
    url = url.left( pos + 1 );
    url += _new_name;

    init();
    setGeometry( x(), y(), width, height );
    repaint();    
}

void KRootIcon::select( bool _select )
{
  if ( bSelected == _select )
    return;

  bSelected = _select;
  
  init();
  // We changed the mask in init(), so update it here
  resizeEvent( 0L );

  repaint();
}

void KRootIcon::dropPopupMenu( KDNDDropZone *_zone, const char *_dest, const QPoint *_p )
{
    dropDestination = _dest;
    dropDestination.detach();
    
    dropZone = _zone;
    
    // debugT(" Drop with destination %s\n", _dest );
    
    KURL u( _dest );
    
    // Perhaps an executable ?
    // So lets ask wether we can be shure that it is no directory
    // We can rely on this, since executables are on the local hard disk
    // and KIOServer can query the local hard disk very quickly.
    if ( KIOServer::isDir( _dest ) == 0 )
    {
	// Executables or only of interest on the local hard disk
	if ( strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
	{
	    KMimeType *typ = KMimeType::getMagicMimeType( _dest );
	    if ( typ->runAsApplication( _dest, &(_zone->getURLList() ) ) )
		return;
	}
	else
	{
	    // We did not find some binding to execute
	    QMessageBox::warning( 0, klocale->translate("KFM Error"),
				  klocale->translate("Dont know what to do.") );
	    return;
	}
    }
    
    popupMenu->clear();
    
    int id = -1;
    // Ask wether we can read from the dropped URL.
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( _dest, KIO_Write ) )
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_COPY), 
				    this, SLOT( slotDropCopy() ) );
    // Ask wether we can read from the URL and delete it afterwards
    if ( KIOServer::supports( _zone->getURLList(), KIO_Move ) &&
	 KIOServer::supports( _dest, KIO_Write ) )
	id = popupMenu->insertItem(  klocale->getAlias(ID_STRING_MOVE), 
				     this, SLOT( slotDropMove() ) );
    // Ask wether we can link the URL 
    if ( KIOServer::supports( _dest, KIO_Link ) )
	id = popupMenu->insertItem(  klocale->getAlias(ID_STRING_LINK), 
				     this, SLOT( slotDropLink() ) );
    if ( id == -1 )
    {
	QMessageBox::warning( 0, klocale->translate("KFM Error"), 
			      klocale->translate("Dont know what to do") );
	return;
    }

    // Show the popup menu
    popupMenu->popup( *_p );
}

void KRootIcon::slotDropCopy()
{
    KIOJob * job = new KIOJob;
    job->copy( dropZone->getURLList(), dropDestination.data() );
}

void KRootIcon::slotDropMove()
{
    KIOJob * job = new KIOJob;
    job->move( dropZone->getURLList(), dropDestination.data() );
}

void KRootIcon::slotDropLink()
{
    KIOJob * job = new KIOJob;
    job->link( dropZone->getURLList(), dropDestination.data() );
}

KRootIcon::~KRootIcon()
{
    if ( popupMenu )
	delete popupMenu;
    
    delete dropZone;
}

#include "root.moc"
