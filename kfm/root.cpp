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

KRootWidget* KRootWidget::pKRootWidget;

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
	debugT("Exec '%s'\n", s );
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
    e->openURL( _url, FALSE );

    /*
    // _url has "file" protocol.
    // It would be possible to handle the whole stuff
    // with KFMExec. But KFMExec is not optimized for local
    // files, so we try on our own here first.
    KURL u( _url );
    if ( u.isMalformed() )
    {
	QString tmp;
	tmp.sprintf("Malformed URL\n\r%s", _url );
	QMessageBox::message( "KFM Error", tmp );
	return;
    }

    // A link to the web in form of a *.kdelnk file ?
    QString path = u.path();
    if ( !u.hasSubProtocol() && strcmp( u.protocol(), "file" ) == 0 && path.right(7) == ".kdelnk" )
    {
	KURL::decodeURL( path );
    
	// Try tp open the *.kdelnk file
	QFile file( path );
	if ( file.open( IO_ReadOnly ) )
	{
	    QTextStream pstream( &file );
	    KConfig config( &pstream );
	    config.setGroup( "KDE Desktop Entry" );
	    QString typ = config.readEntry( "Type" );
	    // Is it a link ?
	    if ( typ == "Link" )
	    {
		// Is there a URL ?
		QString u2 = config.readEntry( "URL" );
		if ( !u2.isEmpty() )
		{
		    // It is a link and we have a URL => Give the job away
		    KFMExec *e = new KFMExec( u2 );
		    e->openURL( u2, _reload );
		    return;
		}
		else
		{
		    // The *.kdelnk file is broken
		    QMessageBox::message( "KFM Error", "The file does not contain a URL" );
		    return;
		}
	    }
	    file.close();
	}
    }

    // Is it really a directory ?
    if ( KIOServer::isDir( _url ) == 1 )
    {
	// Open a new window
	KfmGui *m = new KfmGui( 0L, 0L, _url );
	m->show();
	return;
    }
    
    // We have a real file here.
    // So try to run the default binding on it.
    if ( KMimeType::runBinding( _url ) )
	return;

    // We dont know what to do with the file, so
    // ask the user what we should do
    DlgLineEntry l( "Open With:", "", 0L, true );
    debugT("OPENING DLG\n");
    if ( l.exec() )
    {
	QString pattern = l.getText();
	if ( pattern.isEmpty() )
	    return;

	QString decoded( tryURL );
	KURL::decodeURL( decoded );
	decoded = KIOServer::shellQuote( decoded ).data();
	    
	QString cmd;
	cmd = l.getText();
	cmd += " ";
	cmd += "\"";
	cmd += decoded;
	cmd += "\"";
	debugT("Executing stuff '%s'\n", cmd.data());
	    
	KMimeType::runCmd( cmd.data() );
    }
    */
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
	warning(klocale->translate("Internal Error: desktopDir is malformed"));
	return;
    }
    
    DIR *dp;
    struct dirent *ep;
    
    dp = opendir( u.path() );
    debugT("%i : %i %i %i %i %i %i\n",errno, EACCES,EMFILE,ENFILE,ENOENT,ENOMEM,ENOTDIR);
    
    if ( dp == NULL )
    {
	debugT("'%s'\n",u.path());
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
    debugT("Opening '%s'\n",u.path());
    
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
 
    bool ok = QMessageBox::query( klocale->translate("KFM Warning"), 
				  klocale->translate("Dou you really want to delete the files?\n\nThere is no way to restore them"), 
				  klocale->translate("Yes"), 
				  klocale->translate("No") );
    
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
	QMessageBox::message( klocale->translate( "KFM Error"),
			      klocale->translate( "Could not access Trash Bin") );
	return;
    }
    
    KIOJob * job = new KIOJob;
    job->del( trash );
}

KRootIcon::KRootIcon( const char *_pixmap_file, QString &_url, int _x, int _y ) :
    KDNDWidget( 0L, 0L, WStyle_Customize | WStyle_Tool | WStyle_NoBorder )
{
    bSelected = FALSE;
  
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
    
    if ( root->iconStyle() == 1 && !bSelected )
       p2.drawText( textXOffset, textYOffset, file );
    else
       p2.fillRect( textXOffset-1, textYOffset-ascent-1, width+2, ascent+descent+2, color1 );     

    if ( pixmap.mask() == 0L )
	p2.fillRect( pixmapXOffset, pixmapYOffset, pixmap.width(), pixmap.height(), color1 );
    else
	bitBlt( &mask, pixmapXOffset, pixmapYOffset, pixmap.mask() );
    
    p2.end();

    if ( bSelected )
      setBackgroundColor( black );
    else
      setBackgroundColor( root->iconBackground() );     

    this->width = w;
    this->height = pixmap.height() + 5 + ascent + descent + 4;
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
    bitBlt( this, pixmapXOffset, pixmapYOffset, &pixmap );
  
    QPainter p;
    p.begin( this );
  
    p.setPen( root->labelForeground() );   
    p.drawText( textXOffset, textYOffset, file );
    
    if ( bSelected )
    {
      p.setRasterOp( NotEraseROP );
      p.fillRect( pixmapXOffset, pixmapYOffset, pixmap.width(), pixmap.height(), blue );
    }
    
    p.end();
}

void KRootIcon::mousePressEvent( QMouseEvent *_mouse )
{
    // Does the user want to select the icon ?
    if ( _mouse->button() == LeftButton && ( _mouse->state() & ControlButton ) == ControlButton )
    {
      printf("Selecting\n");
      select( !bSelected );
    }
    else if ( _mouse->button() == LeftButton )
    {
        root->unselectAllIcons();

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
    
    debugT(" Drop with destination %s\n", _dest );
    
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
	    // Run the executable with the dropped 
	    // files as arguments
	    if ( KMimeBind::runBinding( _dest, 
					klocale->getAlias(ID_STRING_OPEN), 
					&(_zone->getURLList() ) ) )
		// Everything went fine
		return;
	    else
	    {
		// We did not find some binding to execute
	      QMessageBox::message( klocale->translate("KFM Error"),
				    klocale->translate("Dont know what to do.") );
		return;
	    }
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
	QMessageBox::message( klocale->translate("KFM Error"), 
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
    delete dropZone;
}

#include "root.moc"
