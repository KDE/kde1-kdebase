#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>

#include <qdir.h>
#include <qtooltip.h>
#include <qrect.h>
#include <qmsgbox.h>

#include <kapp.h>
#include <kwm.h>

#include "root.h"
#include "kfmprops.h"
#include "kfmdlg.h"
#include "kfmpaths.h"
#include "kfmexec.h"
#include "utils.h"
#include "config-kfm.h"

#include <string.h>

#include <klocale.h>

#define root KRootWidget::getKRootWidget()

KRootWidget* KRootWidget::pKRootWidget = 0L;

KRootWidget::KRootWidget( QWidget *parent, const char *name ) : QWidget( parent, name )
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
    if ( desktopDir.right(1) != "/" )
	desktopDir += "/";
    
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
    if ( strcmp( klocale->getAlias(ID_STRING_ADD_TO_BOOMARKS), _txt ) == 0 )
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
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_OPEN_WITH), 
				    this, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_NEW_VIEW), this, 
				    SLOT( slotPopupNewView() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem(  klocale->getAlias(ID_STRING_COPY), this, 
				    SLOT( slotPopupCopy() ) );
	if ( KIOServer::supports( _urls, KIO_Write ) && KfmView::clipboard->count() != 0 )
	    id = popupMenu->insertItem( klocale->getAlias( ID_STRING_PASTE ), 
					this, SLOT( slotPopupPaste() ) );
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
    if ( _urls.first() == 0L )
	return;
    
    QRect area = KWM::getWindowRegion(KWM::currentDesktop());
    
    // Calculate offset in grid units
    int x = ( p.x() - dndStartPos.x() ) / GRID_WIDTH;
    int y = ( p.y() - dndStartPos.y() ) / GRID_HEIGHT;
    // Do we have to round up/down ?
    int hx = p.x() - dndStartPos.x();
    if ( hx < 0 ) hx *= -1;
    int hy = p.y() - dndStartPos.y();
    if ( hy < 0 ) hy *= -1;
    if ( hx % GRID_WIDTH > GRID_WIDTH/2 )
    {
	if ( p.x() - dndStartPos.x() < 0 )
	    x--;
	else
	    x++;
    }
    if ( hy % GRID_HEIGHT > GRID_HEIGHT/2 )
    {
	if ( p.y() - dndStartPos.y() < 0 )
	    y--;
	else
	    y++;
    }
    
    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	KRootIcon* icon = findIcon( s );
	if ( icon != 0L )
	{
	    int ix = icon->gridX() + x;
	    int iy = icon->gridY() + y;
	    if ( isPlaceUsed( ix, iy ) )
	    {
		QPoint p = findFreePlace( ix, iy );
		ix = p.x();
		iy = p.y();
	    }
	    icon->setGridX( ix );
	    icon->setGridY( iy );
	    icon->move( area.x() + GRID_WIDTH * ix + ( GRID_WIDTH - icon->QWidget::width() ) / 2,
			area.y() + GRID_HEIGHT * iy + ( GRID_HEIGHT - icon->QWidget::height() ) );
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
	    fprintf( f, "%s;%i;%i\n",icon->getURL(),icon->gridX(),icon->gridY() );
	
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

KRootLayout* KRootWidget::findLayout( const char *_url )
{
    KRootLayout *lay;
    
    for ( lay = layoutList.first(); lay != 0L; lay = layoutList.next() )
    {
	if ( strcmp( lay->getURL(), _url ) == 0 )
	    return lay;
    }
    
    return 0L;
}

QPoint KRootWidget::findFreePlace()
{
    // Matthias
    // use the window area to layout the icons
    QRect area = KWM::getWindowRegion(KWM::currentDesktop());

    int gx = area.width() / GRID_WIDTH;
    int gy = area.height() / GRID_HEIGHT;
    
    bool ok = false;
    int x = 0, y = 0;
    while ( !ok && x < gx )
    {
	y = 0;
	while ( !ok && y < gy )
	{
	    bool found = false;
	    QListIterator<KRootIcon> it( icon_list );
	    for ( ; it.current(); ++it )
		if ( it.current()->gridX() == x && it.current()->gridY() == y )
		    found = true;
	    // No icon on this place ?
	    if ( !found )
		return QPoint( x, y );
	    y++;
	}
	x++;
    }
    
    return QPoint( 0, 0 );
}

QPoint KRootWidget::findFreePlace( int x, int y )
{
    // Matthias
    // use the window area to layout the icons
    QRect area = KWM::getWindowRegion(KWM::currentDesktop());

    int gx = area.width() / GRID_WIDTH;
    int gy = area.height() / GRID_HEIGHT;
    
    int offset;
    for ( offset = 1; offset < gx; offset++ )
    {
	int j;
	if ( x - offset >= 0 && x - offset < gx )
	    for ( j = y - offset; j <= y + offset; j++ )
		if ( !isPlaceUsed( x - offset, j ) && j > 0 && j < gy )
		    return QPoint( x - offset, j );
	if ( x + offset >= 0 && x + offset < gx )
	    for ( j = y - offset; j <= y + offset; j++ )
		if ( !isPlaceUsed( x + offset, j ) && j > 0 && j < gy )
		    return QPoint( x + offset, j );
	if ( y - offset >= 0 && y - offset < gy )
	    for ( j = x - offset + 1; j <= x + offset - 1; j++ )
		if ( !isPlaceUsed( j, y - offset ) && j > 0 && j < gx )
		    return QPoint( j, y - offset );
	if ( y + offset >= 0 && y + offset < gy )
	    for ( j = x - offset + 1; j <= x + offset - 1; j++ )
		if ( !isPlaceUsed( j, y + offset ) && j > 0 && j < gx )
		    return QPoint( j, y + offset );
    }
    
    return QPoint( 0, 0 );
}

bool KRootWidget::isPlaceUsed( int x, int y )
{
    QListIterator<KRootIcon> it( icon_list );
    for ( ; it.current(); ++it )
	if ( it.current()->gridX() == x && it.current()->gridY() == y )
	    return true;

    return false;
}

void KRootWidget::arrangeIcons()
{
    // Matthias
    // use the free window area  ( no panel ) to layout the icons
    QRect area = KWM::getWindowRegion(KWM::currentDesktop());

    int max_icons = icon_list.count();
    int my = area.height() / GRID_HEIGHT;
    int gx = max_icons / my;
    int gy = max_icons % my;
    
    /**
     * Find first not used position from left to right and top
     * to bottom and move the right/bottom most icon to this
     * position.
     */

    int x, y;
    for ( x = 0; x <= gx; x++ )
    {
	int gy2 = my;
	if ( x == gx ) // last line
	    gy2 = gy;
	for ( y = 0; y < gy2; y++ )
	{
	    bool found = false;
	    QListIterator<KRootIcon> it2( icon_list );
	    for ( ; it2.current(); ++it2 )
		if ( it2.current()->gridX() == x && it2.current()->gridY() == y )
		    found = true;
	    
	    // No icon on this place ?
	    if ( !found )
	    {
		// Find right and bottom most icon
		int maxx = x, maxy = y;
		KRootIcon *icon = 0;
		QListIterator<KRootIcon> it3( icon_list );
		for ( ; it3.current(); ++it3 )
		{
		    if ( it3.current()->gridX() > maxx )
		    {
			icon = it3.current();
			maxx = it3.current()->gridX();
			maxy = it3.current()->gridY();
		    }
		    else if ( it3.current()->gridX() == maxx && it3.current()->gridY() > maxy )
		    {
			icon = it3.current();
			maxy = it3.current()->gridY();
		    }
		}
		// Move the right and bottom most icon
		if ( icon )
		{
		    icon->setGridX( x );
		    icon->setGridY( y );
		    icon->move( area.x() + GRID_WIDTH * x + ( GRID_WIDTH - icon->QWidget::width() ) / 2,
				area.y() + GRID_HEIGHT * y + ( GRID_HEIGHT - icon->QWidget::height() ) );
		}
	    }
	}
    }

    saveLayout();
}

void KRootWidget::update()
{
    if ( noUpdate )
	return;

    // Area where we can place icons
    QRect area = KWM::getWindowRegion(KWM::currentDesktop());
    
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
	warning(klocale->translate("ERROR: Could not read desktop directory '%s'"), desktopDir.data());
	exit(1);
    }

    /**
     * Create every icon that is not on the screen but on the file system
     */

    QList<KRootIcon> found_icons;
    found_icons.setAutoDelete( false );

    // go thru all files in ~/Desktop.
    while ( ( ep = readdir( dp ) ) != 0L )
    {
	// We are not interested in '.' and '..'
	if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
	{   
	    KRootIcon *icon;

	    QString file = desktopDir;
	    file.detach();
	    file += ep->d_name;

	    icon = findIcon( file.data() );
	    	    
	    // This icon is missing on the screen, so we have to create it.
	    if ( icon == 0L )
	    {
		KRootIcon *icon = 0L;
		icon = new KRootIcon( file, -200, -200 );
		// Do we have some information about where to locate the new icon ?
		KRootLayout *lay = findLayout( file );
		if ( lay )
		{
		    int x = lay->getX();
		    int y = lay->getY();
		    // is the location already used ?
		    if ( isPlaceUsed( x, y ) )
		    {
			// Find a free place next to this location
			QPoint p = findFreePlace( x, y );
			x = p.x();
			y = p.y();
		    }
		    
		    icon->setGridX( x );
		    icon->setGridY( y );
		    icon->move( area.x() + GRID_WIDTH * x + ( GRID_WIDTH - icon->QWidget::width() ) / 2,
				area.y() + GRID_HEIGHT * y + ( GRID_HEIGHT - icon->QWidget::height() ) );
		    // This information is no longer needed
		    layoutList.remove( lay );
		}
		
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

    /**
     * Place icons as described in the config file
     */

    // Find correct places for the icons
    QString file = QDir::homeDirPath().data();
    file += "/.kde/share/apps/kfm/desktop";
 
    FILE *f = fopen( file.data(), "r" );
    if ( f != 0 )
    {
	// Loop over every entry
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
		int x = atoi( p );
		p = p2;
		int y = atoi( p );

		// Do we have such an icon
		KRootIcon* icon = findIcon( u );
		if ( icon )
		{
		    icon->setGridX( x );
		    icon->setGridY( y );
		    icon->move( area.x() + GRID_WIDTH * x + ( GRID_WIDTH - icon->QWidget::width() ) / 2,
				area.y() + GRID_HEIGHT * y + ( GRID_HEIGHT - icon->QWidget::height() ) );
		}
	    }
	}
	fclose( f );
    }

    /**
     * Delete all icons on the screen that appear not in the file system.
     */

    QList<KRootIcon> remove_list;
    KRootIcon *icon;
    for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
	if ( found_icons.findRef( icon ) == -1 )
	    remove_list.append( icon );

    for ( icon = remove_list.first(); icon != 0L; icon = remove_list.next() )
	icon_list.remove( icon );

    /**
     * Correct the position of all icons which are not on the visible part of the screen.
     */
    
    // Find every icon that is not on the screen.
    for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
    {
	// Is the icon misplaced ?
	if ( icon->gridX() == -1 || icon->gridY() == -1 || icon->gridX() * GRID_WIDTH + icon->QWidget::width() > area.width() ||
	     icon->gridY() * GRID_HEIGHT + icon->QWidget::height() > area.height() )
	{
	    QPoint p = findFreePlace();
	    icon->setGridX( p.x() );
	    icon->setGridY( p.y() );
	    icon->move( area.x() + GRID_WIDTH * p.x() + ( GRID_WIDTH - icon->QWidget::width() ) / 2,
			area.y() + GRID_HEIGHT * p.y() + ( GRID_HEIGHT - icon->QWidget::height() ) );
	}
    }

    saveLayout();
}

void KRootWidget::slotDropEvent( KDNDDropZone *_zone )
{
    dropZone = _zone;
    dropFileX = _zone->getMouseX();
    dropFileY = _zone->getMouseY();
    
    popupMenu->clear();

    QStrList & list = _zone->getURLList();
    char *s = list.first();
    KURL u( s );
    if ( !u.isMalformed() )
    {
	// Did we just move root icons ?
	if ( strcmp( u.directoryURL(), desktopDir.data() ) == 0 )
	{
	    QPoint p( _zone->getMouseX(), _zone->getMouseY() );
	    moveIcons( list, p );
	    return;
	}
    }

    bool nested = false;
    // Test wether the destination includes the source
    QString url( KIOServer::canonicalURL( KFMPaths::DesktopPath() ) );
    for ( s = list.first(); s != 0L; s = list.next() )
    {
	if ( !KIOServer::testDirInclusion( s, url ) )
	    nested = true;
    }
	    
    int id = 1;
    // Ask wether we can read from the dropped URL.
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( desktopDir.data(), KIO_Write ) && !nested )
	id = popupMenu->insertItem( klocale->getAlias( ID_STRING_COPY ), this, SLOT( slotDropCopy() ) );
    // Ask wether we can read from the URL and delete it afterwards
    if ( KIOServer::supports( _zone->getURLList(), KIO_Read ) &&
	 KIOServer::supports( _zone->getURLList(), KIO_Delete ) &&
	 KIOServer::supports( desktopDir.data(), KIO_Write ) && !nested )
	id = popupMenu->insertItem( klocale->getAlias( ID_STRING_MOVE ), this, SLOT( slotDropMove() ) );
    // We can link everything on the local file system
    if ( KIOServer::supports( desktopDir.data(), KIO_Link ) )
	id = popupMenu->insertItem( klocale->getAlias( ID_STRING_LINK ), this, SLOT( slotDropLink() ) );
    if ( id == -1 )
    {
        warning(klocale->translate("ERROR: Can not accept drop"));
	return;
    }

    popupMenu->popup( QPoint( dropFileX, dropFileY ) );
}

void KRootWidget::slotDropCopy()
{
    // Calculate grid position for files
    QRect area = KWM::getWindowRegion(KWM::currentDesktop());
    int x = ( dropFileX - area.x() ) / GRID_WIDTH;
    int y = ( dropFileY - area.y() ) / GRID_HEIGHT;

    // Create a job
    KIOJob * job = new KIOJob();

    // Create layout hints. This way we know later, where to place the icons
    // for the files we want to copy now.
    char *s;
    for ( s = dropZone->getURLList().first(); s != 0L; s = dropZone->getURLList().next() )
    {
	QString tmp( desktopDir.data() );
	tmp += KIOServer::getDestNameForCopy( s );
	layoutList.append( new KRootLayout( tmp, x, y ) );
    }
    
    job->copy( dropZone->getURLList(), desktopDir.data() );    
}

void KRootWidget::slotDropMove()
{
    // Calculate grid position for files
    QRect area = KWM::getWindowRegion(KWM::currentDesktop());
    int x = ( dropFileX - area.x() ) / GRID_WIDTH;
    int y = ( dropFileY - area.y() ) / GRID_HEIGHT;

    // Create a job
    KIOJob * job = new KIOJob();

    // Create layout hints. This way we know later, where to place the icons
    // for the files we want to copy now.
    char *s;
    for ( s = dropZone->getURLList().first(); s != 0L; s = dropZone->getURLList().next() )
    {
	QString tmp( desktopDir.data() );
	tmp += KIOServer::getDestNameForCopy( s );
	layoutList.append( new KRootLayout( tmp, x, y ) );
    }
    
    job->move( dropZone->getURLList(), desktopDir.data() );
}

void KRootWidget::slotDropLink()
{
    // Calculate grid position for files
    QRect area = KWM::getWindowRegion(KWM::currentDesktop());
    int x = ( dropFileX - area.x() ) / GRID_WIDTH;
    int y = ( dropFileY - area.y() ) / GRID_HEIGHT;

    // Create a job
    KIOJob * job = new KIOJob();

    // Create layout hints. This way we know later, where to place the icons
    // for the files we want to copy now.
    char *s;
    for ( s = dropZone->getURLList().first(); s != 0L; s = dropZone->getURLList().next() )
    {
	QString tmp( desktopDir.data() );
	tmp += KIOServer::getDestNameForLink( s );
	layoutList.append( new KRootLayout( tmp, x, y ) );
    }
    
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

void KRootWidget::slotPopupPaste()
{
    if ( popupFiles.count() != 1 )
    {
	QMessageBox::warning( (QWidget*)0, klocale->translate("KFM Error"), 
			      klocale->translate("Can not paste into multiple directories") );
	return;
    }
    
    // Check wether we drop a directory on itself or one of its children
    int nested = 0;
    char *s;
    for ( s = KfmView::clipboard->first(); s != 0L; s = KfmView::clipboard->next() )
    {
	int j;
	if ( ( j = testNestedURLs( s, popupFiles.first() ) ) )
	    if ( j == -1 || ( j > nested && nested != -1 ) )
		nested = j;
	}
    
    if ( nested == -1 )
    {
	QMessageBox::warning( (QWidget*)0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: Malformed URL") );
	return;
    }
    if ( nested == 2 )
    {
	QMessageBox::warning( (QWidget*)0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: You dropped a URL over itself") );
	return;
    }
    if ( nested == 1 )
    {
	QMessageBox::warning( (QWidget*)0, klocale->translate( "KFM Error" ),
			      klocale->translate("ERROR: You dropped a directory over one of its children") );
	return;
    }

    KIOJob * job = new KIOJob;
    job->copy( (*KfmView::clipboard), popupFiles.first() );
}

void KRootWidget::slotPopupCopy()
{
    KfmView::clipboard->clear();
    char *s;
    for ( s = popupFiles.first(); s != 0L; s = popupFiles.next() )    
	KfmView::clipboard->append( s );
}

void KRootWidget::slotPopupTrash()
{
    KIOJob * job = new KIOJob;
    
    QString dest = "file:" + KFMPaths::TrashPath();
    job->setOverWriteExistingFiles( true );
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
    grid_x = -1;
    grid_y = -1;
    
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

    // Matthias
    // keep root icons lowered
    KWM::setDecoration(winId(), 1024);
    
    connect( kapp, SIGNAL( kdisplayFontChanged() ), this, SLOT( slotFontChanged() ) );
}

void KRootIcon::initToolTip()
{
    // Does not work due to a Qt bug.
    KMimeType *typ = KMimeType::findType( url.data() );
    QString com = typ->getComment( url.data() );
    com.detach();
    if ( !com.isNull() )
	QToolTip::add( this, com.data() );
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
	root->dndStartPos = mapToGlobal( QPoint( press_x, press_y ) );
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

void KRootIcon::slotFontChanged()
{
    init();
}

KRootIcon::~KRootIcon()
{
    if ( popupMenu )
	delete popupMenu;
    
    delete dropZone;
}

#include "root.moc"
