#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>

#include <qdir.h>
#include <qtooltip.h>
#include <qrect.h>
#include <qmsgbox.h>

#include <kapp.h>

#include "root.h"
#include "kfmprops.h"
#include "kfmdlg.h"
#include "kfmpaths.h"
#include <config-kfm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <string.h>

#define root KRootWidget::getKRootWidget()

KRootWidget* KRootWidget::pKRootWidget;

KRootManager::KRootManager( KRootWidget *_root ) : KFileManager( 0L )
{
    rootWidget = _root;
}

void KRootManager::openPopupMenu( QStrList &_urls, const QPoint &_point )
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
	id = popupMenu->insertItem( "Open", rootWidget, 
				    SLOT( slotPopupNewView() ) );
	id = popupMenu->insertItem( "Empty Trash Bin", rootWidget, 
				    SLOT( slotPopupEmptyTrash() ) );
	if ( !isempty )
	    popupMenu->setItemEnabled( id, false );
    }
    else if ( isdir )
    {
	int id;
	id = popupMenu->insertItem( "New View", rootWidget, 
				    SLOT( slotPopupNewView() ) );
	id = popupMenu->insertItem( "Copy", rootWidget, 
				    SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Move to Trash",  rootWidget, 
				    SLOT( slotPopupTrash() ) );
	id = popupMenu->insertItem( "Delete",  rootWidget, 
				    SLOT( slotPopupDelete() ) );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( "Open With", rootWidget, 
				    SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( "Copy", rootWidget, 
				    SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Move to Trash",  rootWidget, 
				    SLOT( slotPopupTrash() ) );
	id = popupMenu->insertItem( "Delete", rootWidget, 
				    SLOT( slotPopupDelete() ) );
    }
    
    rootWidget->setPopupFiles( _urls );
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
	popupMenu->insertItem( "Properties", rootWidget, SLOT( slotPopupProperties() ) );
    }
    
    popupMenu->popup( _point );
}

KRootWidget::KRootWidget( QWidget *parent=0, const char *name=0 ) : QWidget( parent, name )
{
    debugT("1\n");
    KConfig *config = KApplication::getKApplication()->getConfig();
    debugT("2\n");
    if ( config )
    {
         debugT("3\n");
         config->setGroup("KFM Root Icons");
         debugT("4\n");
	 iconstyle = config->readNumEntry( "Style", 1 );
         debugT("5\n");
	 QString bg = config->readEntry( "Background" );
	 if ( bg.isNull() )
	   bg = "black";
	 debugT("5b '%s'\n",bg.data());
         iconBgColor.setNamedColor( bg.data() );
         debugT("6\n");
	 QString fg = config->readEntry( "Foreground" );
	 if ( fg.isNull() )
	   fg = "white";
         labelColor.setNamedColor( fg.data() );
	 debugT("7\n");
    }

    rootDropZone = new KDNDDropZone( this , DndURL );
    connect( rootDropZone, SIGNAL( dropAction( KDNDDropZone *) ),
	     this, SLOT( slotDropEvent( KDNDDropZone *) ) );
    KApplication::getKApplication()->setRootDropZone( rootDropZone );

    manager = new KRootManager( this );
    popupMenu = new QPopupMenu();
    
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

void KRootWidget::moveIcons( QStrList &_urls, QPoint &p )
{
    debugT("1\n");
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	debugT("2\n");
	KRootIcon *icon = findIcon( s );
	if ( icon != 0L )
	{
	    debugT("3\n");
	    icon->move( icon->x() + p.x() - icon->getDndStartPos().x(),
			icon->y() + p.y() - icon->getDndStartPos().y() );
	}
	debugT("4\n");
    }

    debugT("5\n");
    saveLayout();
    debugT("6\n");
}

QString KRootWidget::getSelectedURLs()
{
    QString erg;

    // TODO: Liste aller selektierten URLs zurueckgeben
    return QString( erg );
}

void KRootWidget::saveLayout()
{
    KRootIcon *icon;

    QString file = QDir::homeDirPath().data();
    file += "/.desktop";
    
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
    file += "/.desktop";
 
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
		debugT("I am here\n");
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
		/* StringTokenizer st( s, ";" );
		QString u = st.nextToken();
		QString t = st.nextToken();
		QString x = st.nextToken();
		QString y = st.nextToken(); */
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

    debugT("Searching free place %i,%i\n",_width,_height);
    
    for ( int x = 0; x <= dwidth - ROOT_GRID_WIDTH; x += ROOT_GRID_WIDTH )
    {
	for ( int y = 80; y <= dheight - ROOT_GRID_HEIGHT; y += ROOT_GRID_HEIGHT )
	{	
	    bool isOK = true;
	    
	    for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
	    {
		debugT("Testing %i|%i against %i|%i '%s'\n",x,y,icon->x(),icon->y(),icon->getURL() );
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
	    debugT("Moving '%s'\n",it.current()->getURL() );
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
	debugT("Internal Error: desktopDir is malformed\n");
	return;
    }
    
    DIR *dp;
    struct dirent *ep;
    
    dp = opendir( u.path() );
    debugT("%i : %i %i %i %i %i %i\n",errno, EACCES,EMFILE,ENFILE,ENOENT,ENOMEM,ENOTDIR);
    
    if ( dp == NULL )
    {
	debugT("'%s'\n",u.path());
	debugT("ERROR: Could not read desktop directory '%s'\nRun 'setupKFM'\n", desktopDir.data());
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
		KRootIcon *icon;
		KMimeType *typ = KMimeType::findType( file.data() );
		QPoint p = findLayout( file );
		if ( p.x() == 0 && p.y() == 0 )
		{
		    icon = new KRootIcon( typ->getPixmapFile( file.data() ), file, -200, -200 );
		    p = findFreePlace( icon->QWidget::width(), icon->QWidget::height() );
		    icon->move( p );
		}
		else
		    icon = new KRootIcon( typ->getPixmapFile( file.data() ), file, p.x(), p.y() );
		icon_list.append( icon );
		found_icons.append( icon );
	    }	    
	    else  // Icon is already there
	    {
		// Perhaps the pixmap changed. This may only happen to
		// *.kdelnk files.
		// if ( file.right( 7 ) == ".kdelnk" )
		// {
		//KConfig *config = KFileType::openKFMConfig( file.data() );
		    /* if ( config != 0L )
		    {
			QString typ = config->readEntry( "Type" );
			if ( typ == "FSDevice" ) */
		    icon->updatePixmap(); 
		    // delete config;
			// }
		    // }
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
	debugT("ERROR: Can not accept drop\n");
	return;
    }

    popupMenu->popup( QPoint( dropFileX, dropFileY ) );
}

void KRootWidget::dropUpdateIcons( int _x, int _y )
{
    KURL u ( desktopDir.data() );
    if ( u.isMalformed() )
    {
	debugT("Internal Error: desktopDir is malformed\n");
	exit(2);
    }
    
    DIR *dp;
    struct dirent *ep;
    debugT("Opening '%s'\n",u.path());
    
    dp = opendir( u.path() );
    if ( dp == NULL )
    {
	debugT("ERROR: Could not read desktop directory\n");
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
		KMimeType *typ = KMimeType::findType( file.data() );
		QPixmap pix = typ->getPixmap( file.data() );
		KRootIcon *icon = new KRootIcon( typ->getPixmapFile( file.data() ), file, _x - pix.width() / 2,
						 _y - pix.height() / 2 );
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

void KRootWidget::setPopupFiles( QStrList &_files )
{
    popupFiles.copy( _files );
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
    
    KMimeType::runCmd( cmd.data() );
}              

void KRootWidget::slotPopupProperties()
{
    if ( popupFiles.count() != 1 )
    {
	debugT("ERROR: Can not open properties for multiple files\n");
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
    debugT(":::::::::::::::::::: COPY ::::::::::::::::::::\n");
    
    KfmView::clipboard.clear();
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	KfmView::clipboard.append( s );
}

void KRootWidget::slotPopupTrash()
{
    KIOJob * job = new KIOJob;
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
      debugT("&&> '%s'\n",s);
    
    QString dest = "file:" + KFMPaths::TrashPath();
    job->move( popupFiles, dest );
}

void KRootWidget::slotPopupDelete()
{
    KIOJob * job = new KIOJob;
 
    bool ok = QMessageBox::query( "KFM Warning", "Dou you really want to delete the files?\n\r\n\rThere is no way to restore them", "Yes", "No" );
    
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
	QMessageBox::message( "KFM Error", "Could not access Trash Bin" );
	return;
    }
    
    KIOJob * job = new KIOJob;
    job->del( trash );
}

KRootIcon::KRootIcon( const char *_pixmap_file, QString &_url, int _x, int _y ) :
    KDNDWidget( 0L, 0L, WStyle_Customize | WStyle_Tool | WStyle_NoBorder )
{
    url = _url;
    url.detach();
    pixmap.load( _pixmap_file );
    pixmapFile = _pixmap_file;
    pixmapFile.detach();
    
    init();
    initToolTip();
    
    dropZone = new KDNDDropZone( this , DndURL);
    connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ), this, SLOT( slotDropEvent( KDNDDropZone *) ) );

    // connect( drop_zone, SIGNAL( dropEnter( KDNDDropZone *) ), this, SLOT( slotDropEnterEvent( KDNDDropZone *) ) );
    // connect( drop_zone, SIGNAL( dropLeave( KDNDDropZone *) ), this, SLOT( slotDropLeaveEvent( KDNDDropZone *) ) );

    setGeometry( _x - pixmapXOffset, _y, width, height );
    lower();
    show();
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
    if ( pixmap.width() > w )
    {
	w = pixmap.width() + 8;
	textXOffset = ( w - width ) / 2;
	pixmapXOffset = 4;
    }
    else
    {
	w += 8;
	textXOffset = 4;
	pixmapXOffset = ( w - pixmap.width() ) / 2;
    }

    pixmapYOffset = 2;
    textYOffset = pixmap.height() + 5 + ascent + 2;

    p.end();

    mask.resize( w, pixmap.height() + 5 + ascent + descent + 4 );
    mask.fill( color0 );
    QPainter p2;
    p2.begin( &mask );
    p2.setFont( font() );
    
    if ( root->iconStyle() == 1 )
       p2.drawText( textXOffset, textYOffset, file );
    else
       p2.fillRect( textXOffset-1, textYOffset-ascent-1, width+2, ascent+descent+2, color1 );     

    if ( pixmap.mask() == 0L )
	p2.fillRect( pixmapXOffset, pixmapYOffset, pixmap.width(), pixmap.height(), color1 );
    else
	bitBlt( &mask, pixmapXOffset, pixmapYOffset, pixmap.mask() );
    
    p2.end();
    
    setBackgroundColor( root->iconBackground() );     

    this->width = w;
    this->height = pixmap.height() + 5 + ascent + descent + 4;
}

void KRootIcon::slotDropEvent( KDNDDropZone *_zone )
{	
    debugT("-1 Drop Event\n");
    
    QPoint p( _zone->getMouseX(), _zone->getMouseY() );

    debugT("-2\n");
    
    QStrList & list = _zone->getURLList();

    debugT("-3\n");
    // Dont drop icons on themselves.
    if ( list.count() != 0 )
    {
	debugT("-4\n");
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
    
    debugT("-5\n");
    root->getManager()->dropPopupMenu( _zone, url.data(), &p );
    debugT("-6\n");
}

void KRootIcon::resizeEvent( QResizeEvent * )
{
    XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0, mask.handle(), ShapeSet );
}


void KRootIcon::paintEvent( QPaintEvent * ) 
{
    bitBlt( this, pixmapXOffset, pixmapYOffset, &pixmap );

    QPainter p;
    p.begin( this );

    p.setPen( root->labelForeground() );   
    p.drawText( textXOffset, textYOffset, file );

    p.end();
}

void KRootIcon::mousePressEvent( QMouseEvent *_mouse )
{
    if ( _mouse->button() == LeftButton )
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
	QPoint p = mapToGlobal( _mouse->pos() );
	QStrList list;
	list.append( url.data() );
	root->getManager()->openPopupMenu( list, p );
    }
}

void KRootIcon::mouseDoubleClickEvent( QMouseEvent * )
{
    // root->getManager()->openURL( url.data() );
    pressed = false;
    debugT ("Doubleclick!\n");     
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

void KRootIcon::dndMouseReleaseEvent( QMouseEvent * )
{
    if (pressed == false)
      return;
    root->getManager()->openURL( url.data() );                          

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
	
	if ( data.isNull() )
	    data = url;
	
	QPoint p = mapToGlobal( _mouse->pos() );
	QPoint p2 = mapToGlobal( QPoint( press_x, press_y ) );
	int dx = QWidget::x() - p2.x() + pixmapXOffset;
	int dy = QWidget::y() - p2.y() + pixmapYOffset;
	startDrag( new KDNDIcon( pixmap, p.x() + dx, p.y() + dy ), url.data(), url.length(), DndURL, dx, dy );
    }
}

/*
 * Called when we start dragging Icons from the desktop around and release
 * the mouse button over the desktop again. In this case I just move
 * the icons from their original place to the new one.
 */
/* void KRootIcon::rootDropEvent( int _x, int _y )
{
    QPoint p = mapToGlobal( QPoint( press_x, press_y ) );
    root->moveIcons( (const char *)dndData, p.x() - _x, p.y() - _y );

    KDNDWidget::rootDropEvent();
}*/

void KRootIcon::updatePixmap()
{
    KMimeType *typ = KMimeType::findType( url.data() );
    QString f = typ->getPixmapFile( url.data() );
    if ( f == pixmapFile )
	return;

    QToolTip::remove( this );
    initToolTip();
    
    debugT("Changing from '%s' to '%s\n",pixmapFile.data(),f.data());
    
    pixmapFile = f.data();
    pixmapFile.detach();
    pixmap = typ->getPixmap( url.data() );
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

KRootIcon::~KRootIcon()
{
    delete dropZone;
}

#include "root.moc"
