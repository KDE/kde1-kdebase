#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>

#include <qdir.h>
#include <qtooltip.h>

#include <kapp.h>

#include "root.h"
#include "kfmprops.h"

#define root KRootWidget::getKRootWidget()

KRootWidget* KRootWidget::pKRootWidget;

KRootManager::KRootManager( KRootWidget *_root ) : KFileManager( 0L, 0L )
{
    rootWidget = _root;
}

void KRootManager::openPopupMenu( QStrList &_urls, const QPoint &_point )
{
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
    bindings.setAutoDelete( TRUE );
    QStrList bindings2;
    bindings2.setAutoDelete( TRUE );
    
    char buffer[ 1024 ];
    
    bool isdir = KIOServer::isDir( _urls );

    if ( isdir )
    {
	int id;
	id = popupMenu->insertItem( "New View", rootWidget, SLOT( slotPopupNewView() ) );
	id = popupMenu->insertItem( "Copy", rootWidget, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Delete",  rootWidget, SLOT( slotPopupDelete() ) );
    }
    else
    {
	int id;
	id = popupMenu->insertItem( "Copy", rootWidget, SLOT( slotPopupCopy() ) );
	id = popupMenu->insertItem( "Delete", rootWidget, SLOT( slotPopupDelete() ) );
    }
    
    rootWidget->setPopupFiles( _urls );
    popupFiles.copy( _urls );
    
    // Get all bindings matching all files.
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	// If this is the first file in the list, assume that all bindings are ok
	if ( s == _urls.getFirst() )
	{
	    KFileType::getBindings( bindings, s, isdir );
	}
	// Take only bindings, matching all files.
	else
	{
	    KFileType::getBindings( bindings2, s, isdir );
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
    rootDropZone = new KDNDDropZone( this , DndURL );
    connect( rootDropZone, SIGNAL( dropAction( KDNDDropZone *) ), this, SLOT( slotDropEvent( KDNDDropZone *) ) );
    KApplication::getKApplication()->setRootDropZone( rootDropZone );

    manager = new KRootManager( this );
    popupMenu = new QPopupMenu();
    
    noUpdate = FALSE;
    
    pKRootWidget = this;
    
    desktopDir = "file:";
    desktopDir += QDir::homeDirPath().data();
    desktopDir += "/Desktop/";

    connect( KIOServer::getKIOServer(), SIGNAL( notify( const char *) ),
    	     this, SLOT( slotFilesChanged( const char *) ) );
    connect( KIOServer::getKIOServer(), SIGNAL( mountNotify() ), this, SLOT( update() ) );

    icon_list.setAutoDelete( true );
    layoutList.setAutoDelete( true );
    
    update();
}

void KRootWidget::moveIcons( QStrList &_urls, QPoint &p )
{
    printf("1\n");
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	printf("2\n");
	KRootIcon *icon = findIcon( s );
	if ( icon != 0L )
	{
	    printf("3\n");
	    icon->move( icon->x() + p.x() - icon->getDndStartPos().x(),
			icon->y() + p.y() - icon->getDndStartPos().y() );
	}
	printf("4\n");
    }

    printf("5\n");
    saveLayout();
    printf("6\n");
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
    layoutList.remove();
    
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
		QString s = (const char*)buffer;	    
		StringTokenizer st( s, ";" );
		QString u = st.nextToken();
		QString t = st.nextToken();
		QString x = st.nextToken();
		QString y = st.nextToken();
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

/*
 * TODO: Wenn ein Icon auf dem Schirm aber nicht im Dateisystem ist
 * muss es geloescht werden
 */
void KRootWidget::update()
{
    if ( noUpdate )
	return;
    
    loadLayout();
    
    KURL u ( desktopDir.data() );
    if ( u.isMalformed() )
    {
	printf("Internal Error: desktopDir is malformed\n");
	return;
    }
    
    DIR *dp;
    struct dirent *ep;
    
    dp = opendir( u.path() );
    printf("%i : %i %i %i %i %i %i\n",errno, EACCES,EMFILE,ENFILE,ENOENT,ENOMEM,ENOTDIR);
    
    if ( dp == NULL )
    {
	printf("'%s'\n",u.path());
	printf("ERROR: Could not read desktop directory '%s'\nRun 'setupKFM'\n", desktopDir.data());
	exit(1);
    }
    
    QList<KRootIcon> found_icons;
    found_icons.setAutoDelete( FALSE );

    // go thru all files in ~/Desktop.
    while ( ep = readdir( dp ) )
    {
	// We are not interested in '.' and '..'
	if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
	{   
	    KRootIcon *icon;
	    bool ok = false;

	    QString file = desktopDir;
	    file.detach();
	    file += ep->d_name;

	    icon = findIcon( file.data() );
	    	    
	    // This icon is missing on the screen.
	    if ( icon == 0L )
	    {
		KFileType *typ = KFileType::findType( file.data() );
		QPoint p = findLayout( file );
		// TODO: Wenn ein Directory, dann folder.ppm und DndDir
		KRootIcon *icon = new KRootIcon( typ->getPixmapFile( file.data() ), file, p.x(), p.y() );
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
	printf("ERROR: Can not accept drop\n");
	return;
    }

    popupMenu->popup( QPoint( dropFileX, dropFileY ) );
}

void KRootWidget::dropUpdateIcons( int _x, int _y )
{
    KURL u ( desktopDir.data() );
    if ( u.isMalformed() )
    {
	printf("Internal Error: desktopDir is malformed\n");
	exit(2);
    }
    
    DIR *dp;
    struct dirent *ep;
    printf("Opening '%s'\n",u.path());
    
    dp = opendir( u.path() );
    if ( dp == NULL )
    {
	printf("ERROR: Could not read desktop directory\n");
	noUpdate = FALSE;
	return;
    }
    
    QList<KRootIcon> found_icons;
    found_icons.setAutoDelete( FALSE );

    // go thru all files in ~/Desktop. Search for new files. We assume that they
    // are a result of the drop. This may be wrong, but it is the best method yet.
    while ( ep = readdir( dp ) )
    {
	// We are not interested in '.' and '..'
	if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
	{   
	    KRootIcon *icon;
	    bool ok = false;

	    QString file = desktopDir;
	    file.detach();
	    file += ep->d_name;

	    icon = findIcon( file.data() );
	    	    
	    // This icon is missing on the screen.
	    if ( icon == 0L )
	    {
		KFileType *typ = KFileType::findType( file.data() );
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

void KIORootJob::slotDropNotify( int _id, const char *_url )
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
	update();
}

void KRootWidget::setPopupFiles( QStrList &_files )
{
    popupFiles.copy( _files );
}

void KRootWidget::slotPopupProperties()
{
    if ( popupFiles.count() != 1 )
    {
	printf("ERROR: Can not open properties for multiple files\n");
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
    KFileWindow::clipboard.clear();
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	KFileWindow::clipboard.append( s );
}

void KRootWidget::slotPopupDelete()
{
    KIOJob * job = new KIOJob;
    job->del( popupFiles );
}

void KRootWidget::slotPopupNewView()
{
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
    {
	KFileWindow *m = new KFileWindow( 0L, 0L, s );
	m->show();
    }
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
    if ( file.find( ".kdelnk" ) == file.length() - 7 )
	file = file.left( file.length() - 7 );
    file.detach();
    
    int ascent = p.fontMetrics().ascent();
    int descent = p.fontMetrics().descent();
    int width = p.fontMetrics().width( file.data() );

    int w = width;
    if ( pixmap.width() > w )
    {
	w = pixmap.width() + 4;
	textXOffset = ( w - width ) / 2;
	pixmapXOffset = 2;
    }
    else
    {
	w += 4;
	textXOffset = 2;
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
    
    p2.drawText( textXOffset, textYOffset, file );

    if ( pixmap.mask() == 0L )
	p2.fillRect( pixmapXOffset, pixmapYOffset, pixmap.width(), pixmap.height(), color1 );
    else
	bitBlt( &mask, pixmapXOffset, pixmapYOffset, pixmap.mask() );
    
    p2.end();
    
    setBackgroundColor( white );    

    this->width = w;
    this->height = pixmap.height() + 5 + ascent + descent + 4;
}

void KRootIcon::slotDropEvent( KDNDDropZone *_zone )
{	
    printf("-1 Drop Event\n");
    
    QPoint p( _zone->getMouseX(), _zone->getMouseY() );

    printf("-2\n");
    
    QStrList & list = _zone->getURLList();

    printf("-3\n");
    // Dont drop icons on themselves.
    if ( list.count() != 0 )
    {
	printf("-4\n");
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
    
    printf("-5\n");
    root->getManager()->dropPopupMenu( _zone, url.data(), &p );
    printf("-6\n");
}

void KRootIcon::resizeEvent( QResizeEvent * )
{
    XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0, mask.handle(), ShapeSet );
}


void KRootIcon::paintEvent( QPaintEvent *_event ) 
{
    bitBlt( this, pixmapXOffset, pixmapYOffset, &pixmap );

    QPainter p;
    p.begin( this );

    p.setPen( white );
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

void KRootIcon::mouseDoubleClickEvent( QMouseEvent *_mouse )
{
    root->getManager()->openURL( url.data() );
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
    KFileType *typ = KFileType::findType( url.data() );
    QString f = typ->getPixmapFile( url.data() );
    if ( f == pixmapFile )
	return;

    QToolTip::remove( this );
    initToolTip();
    
    printf("Changing from '%s' to '%s\n",pixmapFile.data(),f.data());
    
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
