/* See the file ChangeLog for log */

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
#include <qclipbrd.h>

#include <kapp.h>
#include <kwm.h>

#include "root.h"
#include "kfmprops.h"
#include "kfmdlg.h"
#include "kfmpaths.h"
#include "kfmexec.h"
#include "utils.h"
#include "config-kfm.h"
#include "kfmw.h"

#include <string.h>

#include <klocale.h>
#include <ksimpleconfig.h>

#define root KRootWidget::getKRootWidget()

KRootWidget* KRootWidget::pKRootWidget = 0L;

KRootWidget::KRootWidget( QWidget *parent, const char *name ) : QWidget( parent, name )
{

    gridwidth  = (oldgridwidth = DEFAULT_GRID_WIDTH);
    gridheight = (oldgridheight = DEFAULT_GRID_HEIGHT);

    KConfig *config = KApplication::getKApplication()->getConfig();

    if ( config )
    {
         config->setGroup("KFM Root Icons");
	 iconstyle = config->readNumEntry( "Style", 1 );
	 showHidden = config->readBoolEntry( "ShowHidden", DEFAULT_SHOW_HIDDEN_ROOT_ICONS );
	 //CT 12Nov1998
	 labelColor = config->readColorEntry( "Foreground", &DEFAULT_ICON_FG );
	 iconBgColor = config->readColorEntry( "Background", &DEFAULT_ICON_BG );

	 if ( labelColor == iconBgColor ) debug("Icon colors identical");
	 //CT

	 config->setGroup( "KFM Misc Defaults" ); // Why in that group ? Used for KRootWidget only...
	 gridwidth = config->readNumEntry( "GridWidth", DEFAULT_GRID_WIDTH );
         gridheight = config->readNumEntry( "GridHeight", DEFAULT_GRID_HEIGHT );
    }

    rootDropZone = new KDNDDropZone( this , DndURL );
    connect( rootDropZone, SIGNAL( dropAction( KDNDDropZone *) ),
	     this, SLOT( slotDropEvent( KDNDDropZone *) ) );
    KApplication::getKApplication()->setRootDropZone( rootDropZone );

    popupMenu = new QPopupMenu();
    connect( popupMenu, SIGNAL( activated( int )),
	     this, SLOT( slotPopupActivated( int )) );

    menuNew = new KNewMenu();

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

void KRootWidget::setRootGridParameters(int _gridwidth ,int _gridheight)
{
    oldgridwidth  = gridwidth;
    oldgridheight = gridheight;

    // better do some sanity checking ... -- Bernd
    if(_gridwidth < 0  || _gridwidth > DEFAULT_GRID_MAX)
        _gridwidth = DEFAULT_GRID_WIDTH;
    if(_gridheight < 0  || gridheight > DEFAULT_GRID_MAX)
        _gridheight = DEFAULT_GRID_HEIGHT;

    gridwidth     = _gridwidth;
    gridheight    = _gridheight;

    if(gridwidth != oldgridwidth || gridheight != oldgridheight)
        rearrangeIcons();
}

void KRootWidget::setRootIconStyle(int newiconstyle)
{
    if(newiconstyle != iconstyle)
    {
        iconstyle = newiconstyle;
        KRootIcon *icon;
        for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
            icon->update();
    }
}

//CT 12Nov 1998
void KRootWidget::setRootIconColors(QColor &fg, QColor &bg) {
    bool changed = false; 
    if (fg != labelColor) 
    { 
        changed = true;
        labelColor = fg;
    }
    if (bg != iconBgColor) 
    {
        changed = true;
        iconBgColor = bg;
    }
    if (!changed)
        return;
    KRootIcon *icon;
    for ( icon = icon_list.first(); icon != 0L; icon = icon_list.next() )
        icon->update();
}
//CT

void KRootWidget::showHiddenFiles( bool show )
{
    if( show != showHidden ){
        showHidden = show;
        update(); 
    }   
}

/** Reads and applies configuration from config file */
void KRootWidget::configure(KConfig * config) {
    config->setGroup("KFM Root Icons");
    bool show = config->readBoolEntry( "ShowHidden", DEFAULT_SHOW_HIDDEN_ROOT_ICONS );
    showHiddenFiles( show );

    setRootIconStyle( config->readNumEntry( "Style", 1 ) );
    QColor labelColor = config->readColorEntry( "Foreground", &DEFAULT_ICON_FG );
    QColor iconBgColor = config->readColorEntry( "Background", &DEFAULT_ICON_BG );

    setRootIconColors( labelColor, iconBgColor );

    config->setGroup( "KFM Misc Defaults" ); // Why in that group ? Used for KRootWidget only...
    int gridwidth = config->readNumEntry( "GridWidth", DEFAULT_GRID_WIDTH );
    int gridheight = config->readNumEntry( "GridHeight", DEFAULT_GRID_HEIGHT );

    setRootGridParameters(gridwidth, gridheight);
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
    unselectAllIcons();
}

void KRootWidget::openPopupMenu( QStrList &_urls, const QPoint &_point )
{
	// please note that this code is strongly related to
	// kfmman.cpp: void KFMManager::openPopupMenu( ... )

  if ( _urls.count() == 0 ){
    unselectAllIcons();
    return;
  }

    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	KURL u( s );
	if ( u.isMalformed() ){
	  unselectAllIcons();
	  return;
	}
    }

    popupMenu->clear();
    // store the mouse position. (Matthias)
    popupMenuPosition = QCursor::pos();

    bool isdir = (KIOServer::isDir( _urls ) == 1);
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
        /* Commented out the open with for directories. David. */
	/* id = popupMenu->insertItem( klocale->getAlias(ID_STRING_OPEN_WITH),
				    this, SLOT( slotPopupOpenWith() ) );
	popupMenu->insertSeparator();
        */

        id = popupMenu->insertItem( klocale->translate("&New"), menuNew );
        popupMenu->insertSeparator();

	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_OPEN), this,
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
    menuNew->setPopupFiles( _urls );

    QStrList bindings;
    QStrList bindings2;
    QStrList bindings3;
    QList<QPixmap> pixlist;
    QList<QPixmap> pixlist2;
    QList<QPixmap> pixlist3;

    // Get all bindings matching all files.
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	// If this is the first file in the list, assume that all bindings are ok
	if ( s == _urls.getFirst() )
	{
	    KMimeType::getBindings( bindings, pixlist, s, isdir );
	}
	// Take only bindings, matching all files.
	else
	{
	    bindings2.clear();
	    pixlist2.clear();
	    KMimeType::getBindings( bindings2, pixlist2, s, isdir );
	    char *b;
	    QPixmap *p = pixlist.first();
	    bindings3.clear();
	    pixlist3.clear();
	    // Look thru all bindings we have so far
	    for ( b = bindings.first(); b != 0L; b = bindings.next() )
	    {
		// Does the binding match this file, too ?
		if ( bindings2.find( b ) != -1 )
		{
		    // Keep these entries
		    bindings3.append( b );
		    pixlist3.append( p );
		}
		p = pixlist.next();
	    }
	    pixlist = pixlist3;
	    bindings = bindings3;
	}
	if( bindings.isEmpty() )
	    break;
    }

    // Add all bindings to the menu
    if ( !bindings.isEmpty() )
    {
	popupMenu->insertSeparator();

	char *str;
	QPixmap *p = pixlist.first();
	for ( str = bindings.first(); str != 0L; str = bindings.next() )
	{
	    if ( p != 0L && !p->isNull() )
		popupMenu->insertItem( *p, str );
	    else
		popupMenu->insertItem( str );
	    p = pixlist.next();
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

    QRect area = getWindowRegion();

    int gx = area.width() / gridwidth;
    int gy = area.height() / gridheight;

    int dx = (p.x() - dndStartPos.x());
    int dy = (p.y() - dndStartPos.y());

    // This bit of direction magnification makes the whole
    // procedure work more desirable. (Marcin Dalecki)
    // Without this change the previous calculations made the
    // icon movement *shamefully* broken! That little math......
    if (dx < 0)
        dx -= gridwidth / 2;
    else
        dx += gridwidth / 2;

    if (dy < 0)
        dy -= gridheight / 2;
    else
        dy += gridheight / 2;

    dx /= gridwidth;
    dy /= gridheight;


      /*
       * The following code
       *
       * - forces the formation of the icons to be kept, that means if you move
       *   three icons in a row you still have three in a row after moving
       *
       * - forces the order of the icons you move to be kept
       *
       * - doesn't allow the user to move icons off the screen
       *
       * I hope this makes it more intuitive to use       -- Stefan Westerfeld
       */

    // first, mark the icons as being moved, so that they do not occupy
    // the space for our move operation, where icons can only be moved
    // to free space (before this, you could never move an icon to the
    // position it was before

    char *s;
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
      KRootIcon* icon = findIcon( s );
      if( icon != 0L )
	icon->startmove();
    }

    // find a free space where the icons can be put
    int deltax, deltay, minscore = 10000000, score, mdx=0, mdy=0;

    for ( deltax = -3; deltax <= 3; deltax++)
    {
        for ( deltay = -3; deltay <= 3; deltay++ )
	{
	    score = deltax*deltax+deltay*deltay;

     	    for ( s = _urls.first(); s != 0L; s = _urls.next() )
	    {
  		KRootIcon* icon = findIcon( s );
          	if (icon)
  		{
  		    // do not let them drop off the screen

             	    int ix = icon->gridX() + dx + deltax;
             	    if ((ix < 0) || (ix >= gx)) score += 1000;
	
           	    int iy = icon->gridY() + dy + deltay;
           	    if ((iy < 0) || (iy >= gy)) score += 1000;

                    // do not allow them to be put upon other icons
           	    if ( isPlaceUsed(ix,iy) ) score += 1000;
		}
	    }
	    if (score < minscore)
	    {
		minscore = score;
		mdx = deltax;
		mdy = deltay;
	    }
	}
    }

    // if there is no free space, better don't even try to move ;)
    if (minscore >= 1000)
    {
	mdx = -dx;
	mdy = -dy;
    }

    // now really move them and make them "visible" again for space checking
    for ( s = _urls.first(); s != 0L; s = _urls.next() )
    {
	KRootIcon* icon = findIcon( s );
        if (icon)
	{
            int ix = icon->gridX() + dx + mdx;
            int iy = icon->gridY() + dy + mdy;

	    icon->setGridX( ix );
	    icon->setGridY( iy );
	    icon->move( area.x() + gridwidth * ix + ( gridwidth - icon->QWidget::width() ) / 2,
			area.y() + gridheight * iy + ( gridheight - icon->QWidget::height() ) );
	    icon->endmove();
	}
    }

    saveLayout();
    unselectAllIcons();
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

    QString file = kapp->localkdedir().copy();
    file += "/share/apps/kfm/desktop";

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
    QRect area = getWindowRegion();

    int gx = area.width() / gridwidth;
    int gy = area.height() / gridheight;

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
    QRect area = getWindowRegion();

    int gx = area.width() / gridwidth;
    int gy = area.height() / gridheight;

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
	if ( it.current()->gridX() == x && it.current()->gridY() == y
		&& !it.current()->isbeingmoved())
	    return true;

    return false;
}

void KRootWidget::rearrangeIcons()
{
  // Call this method after a gridwidth gridheight change
  // The algorithm is O(n^3) rather then the optimal O(n*log(n))
  // but I don't think anyone has a 1000 desktop icons and
  // it's better to be simple so that anyone can easily make
  // modifications.
  // -- Bernd

    QRect area = getWindowRegion();
    
    int my = area.height() / oldgridheight;
    int mx = area.width() / oldgridwidth;

    QList<KRootIcon> new_icon_list;
    new_icon_list.setAutoDelete(false);

    KRootIcon *icon = 0;

    // Create a sorted list of icons. This way the
    // rearrangement keeps the order of the icons somehow.
    int x, y;
    for ( x = 0; x <= mx; x++ )
    {
	for ( y = 0; y <= my; y++ )
	{
	  for (icon_list.first() ; (icon = icon_list.current()) ; icon_list.next() )
	    if ( icon->gridX() == x && icon->gridY() == y )
	    {
	      new_icon_list.append(icon);
	    }
	}
    }

    int ny = area.height() / gridheight;
    int nx = area.width() / gridwidth;

    int k = icon_list.count();

    // Let's check whether we would enter a valid state ...
    // This happens if there is not enough space for all icons.
    if ( ( k / ny ) >= nx )
    {
      // No good ..
      // The new grid dimension would move some icons off the current
      // visible desktop  --- let's recover.

      gridwidth = oldgridwidth;
      gridheight = oldgridheight;

      KConfig *config = KApplication::getKApplication()->getConfig();

      config->setGroup( "KFM Misc Defaults" );	
      config->writeEntry( "GridWidth", gridwidth );
      config->writeEntry( "GridHeight", gridheight );
      QMessageBox::warning( 0, klocale->translate( "Error"),
			    klocale->translate("Cannot execute request.\n"\
			     "New root grid dimensions would move some icons\n"\
			     "off the desktop.") );
      return;
    }

    int i;
    for(icon = new_icon_list.first(),i = 0; icon; icon = new_icon_list.next(),i++)
    {
	y = i % ny;
	x = i / ny;
	
	icon->setGridX( x );
	icon->setGridY( y );
	icon->move( area.x() + gridwidth * x +
		    ( gridwidth - icon->QWidget::width() ) / 2,
		    area.y() + gridheight * y +
		    ( gridheight - icon->QWidget::height() ) );
    }

    saveLayout();
}

void KRootWidget::update()
{
    if ( noUpdate )
	return;

    // Area where we can place icons
    QRect area = getWindowRegion();

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

	    QString file = ep->d_name;
	    KURL::encodeURL(file);
	    file.insert(0, desktopDir);

	    icon = findIcon( file.data() );
	    	
	    // This icon is missing on the screen, so we have to create it.
	    if ( icon == 0L )
	    {
              KURL u(file);
              QString tmp = u.filename();
              //ckeck if hidden files shall be shown
              if( tmp.left(1) != "." || showHidden ) 
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
		    icon->move( area.x() + gridwidth * x + ( gridwidth - icon->QWidget::width() ) / 2,
				area.y() + gridheight * y + ( gridheight - icon->QWidget::height() ) );
		    // This information is no longer needed
		    layoutList.remove( lay );
		}
		
		icon_list.append( icon );
		found_icons.append( icon );
	      }
	    }	
	    else  // Icon is already there
	    {
              KURL u(file);
              QString tmp = u.filename();
	      if( tmp.left(1) != "." || showHidden ) {
		icon->updatePixmap();
		found_icons.append( icon );
              }
	      else 
                //remove hidden files if they just got disabled
		icon_list.removeRef( icon );
	    }
	}
    }
    (void) closedir( dp );

    /**
     * Place icons as described in the config file
     */

    // Find correct places for the icons
    QString file = kapp->localkdedir().copy();
    file += "/share/apps/kfm/desktop";

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
		    icon->move( area.x() + gridwidth * x + ( gridwidth - icon->QWidget::width() ) / 2,
				area.y() + gridheight * y + ( gridheight - icon->QWidget::height() ) );
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
	if ( icon->gridX() == -1 || icon->gridY() == -1 || icon->gridX() * gridwidth + icon->QWidget::width() > area.width() ||
	     icon->gridY() * gridheight + icon->QWidget::height() > area.height() )
	{
	    QPoint p = findFreePlace();
	    icon->setGridX( p.x() );
	    icon->setGridY( p.y() );
	    icon->move( area.x() + gridwidth * p.x() + ( gridwidth - icon->QWidget::width() ) / 2,
			area.y() + gridheight * p.y() + ( gridheight - icon->QWidget::height() ) );
	}
    }

    saveLayout();
}

void KRootWidget::slotDropEvent( KDNDDropZone *_zone )
{
    // check if dropped data is an URL
    if ( _zone->getDataType() != DndURL )
    {
      QMessageBox::warning( 0, klocale->translate( "KFM Error" ),
			    klocale->translate("ERROR: You may only drop URLs") );
      return;
    }

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
    // -- sven: only copy from "file:/tmp/kmail*" start ---
    {
      char *s;
      bool attachment = false;
      for ( s = _zone->getURLList().first(); s != 0L; s = _zone->getURLList().next() )
	if (strstr (s, "file:/") != 0 && strstr(s, "tmp/kmail") != 0 )
	  attachment = true;
      if (attachment)
      {
	if (KIOServer::supports( desktopDir.data(), KIO_Write ) && !nested )
	{
	  popupMenu->insertItem( klocale->getAlias( ID_STRING_COPY ),
				 this, SLOT( slotDropCopy() ) );
	  popupMenu->popup( QPoint( dropFileX, dropFileY ) );
	}
	else
	  warning(klocale->translate("ERROR: Can not accept drop"));
	return;
      }
    }
    // -- sven: only copy from "file:/tmp/kmail*" end ---
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
    QRect area = getWindowRegion();
    int x = ( dropFileX - area.x() ) / gridwidth;
    int y = ( dropFileY - area.y() ) / gridheight;

    // Create a job
    KIOJob * job = new KIOJob();

    // Create layout hints. This way we know later, where to place the icons
    // for the files we want to copy now.
    char *s;
    for ( s = dropZone->getURLList().first(); s != 0L; s = dropZone->getURLList().next() )
    {
	QString tmp = KIOServer::getDestNameForCopy( s );
        KURL::encodeURL(tmp);
        tmp.insert(0, desktopDir);
	layoutList.append( new KRootLayout( tmp, x, y ) );
    }

    job->copy( dropZone->getURLList(), desktopDir.data() );
}

void KRootWidget::slotDropMove()
{
    // Calculate grid position for files
    QRect area = getWindowRegion();
    int x = ( dropFileX - area.x() ) / gridwidth;
    int y = ( dropFileY - area.y() ) / gridheight;

    // Create a job
    KIOJob * job = new KIOJob();

    // Create layout hints. This way we know later, where to place the icons
    // for the files we want to copy now.
    char *s;
    for ( s = dropZone->getURLList().first(); s != 0L; s = dropZone->getURLList().next() )
    {
	QString tmp = KIOServer::getDestNameForCopy( s );
        KURL::encodeURL(tmp);
        tmp.insert(0, desktopDir);
	layoutList.append( new KRootLayout( tmp, x, y ) );
    }

    job->move( dropZone->getURLList(), desktopDir.data() );
}

void KRootWidget::slotDropLink()
{
    // Calculate grid position for files
    QRect area = getWindowRegion();
    int x = ( dropFileX - area.x() ) / gridwidth;
    int y = ( dropFileY - area.y() ) / gridheight;

    // Create a job
    KIOJob * job = new KIOJob();

    // Create layout hints. This way we know later, where to place the icons
    // for the files we want to copy now.
    char *s;
    for ( s = dropZone->getURLList().first(); s != 0L; s = dropZone->getURLList().next() )
    {
	QString tmp = KIOServer::getDestNameForLink( s );
        KURL::encodeURL(tmp);
        tmp.insert(0, desktopDir);
	layoutList.append( new KRootLayout( tmp, x, y ) );
    }

    job->link( dropZone->getURLList(), desktopDir.data() );
    unselectAllIcons();
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
    OpenWithDlg l( klocale->translate("Open With:"), "", this, true );
    if ( l.exec() )
    {
      KMimeBind *bind = l.mimeBind();
      if ( bind )
      {
	const char *s;
	for( s = popupFiles.first(); s != 0L; s = popupFiles.next() )
	  bind->runBinding( s );
	unselectAllIcons();
	return;
      }

      QString pattern = l.getText();
      if ( pattern.length() == 0 ) {
	unselectAllIcons();
	return;
      }
    }
    else{
	unselectAllIcons();
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

    KMimeBind::runCmd( cmd.data() );
    unselectAllIcons();
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

    connect( p, SIGNAL( propertiesCancel() ), this,
	     SLOT( slotPropertiesCancel() ) );

    saveLayout();
}

void KRootWidget::slotPropertiesCancel(){

    unselectAllIcons();
}

void KRootWidget::slotPropertiesChanged( const char *_url, const char *_new_name )
{
    unselectAllIcons();
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
    QApplication::clipboard()->setText(popupFiles.first()); // first URL in the system clipboard 
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

    bool ok = !QMessageBox::warning( 0, klocale->translate("KFM Warning"),
				  klocale->translate("Do you really want to delete the selected file(s)?\n\nThere is no way to restore them."),
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
    unselectAllIcons();
}

void KRootWidget::slotPopupEmptyTrash()
{
    QString d = KFMPaths::TrashPath();
    QStrList trash;

    DIR *dp;
    struct dirent *ep;
    unselectAllIcons();

    dp = opendir( d );
    if ( dp )
    {
	while ( ( ep = readdir( dp ) ) != 0L )
	{
	    if ( strcmp( ep->d_name, "." ) != 0L && strcmp( ep->d_name, ".." ) != 0L && strcmp( ep->d_name, ".directory" ) != 0L )
	    {
		QString trashFile( ep->d_name );
		trashFile.detach();
		trashFile.prepend (d);
		KURL::encodeURL ( trashFile );    // make proper URL (Hen)
		trashFile.prepend ("file:");
		trash.append( trashFile.data() );
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

QRect KRootWidget::getWindowRegion()
{
    QRect area = KWM::getWindowRegion(KWM::currentDesktop());
    
    // check for mac menubar (ettrich)
    KConfig config;
    config.setGroup("KDE");
    if (config.readEntry("macStyle") == "on") 
        area.setTop(area.top() + 30);
    return area;
}

/*********************************************************************
 *
 * KRootIcon
 *
 *********************************************************************/

QPixmap *KRootIcon::link_pixmap = 0;
QPixmap *KRootIcon::ro_pixmap = 0;

KRootIcon::KRootIcon( const char *_url, int _x, int _y ) :
    KDNDWidget( 0L, 0L, WStyle_Customize | WStyle_NoBorder )
{
    grid_x = -1;
    grid_y = -1;

    beingmoved = false;
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
    KWM::setDecoration(winId(), KWM::desktopIcon);

    connect( kapp, SIGNAL( kdisplayFontChanged() ), this, SLOT( slotFontChanged() ) );

    if ( !link_pixmap || !ro_pixmap )
    {
	KIconLoader il;
	link_pixmap = new QPixmap;
	*link_pixmap = il.loadApplicationIcon( "link.xpm" );
	ro_pixmap = new QPixmap;
	*ro_pixmap = il.loadApplicationIcon( "readonly.xpm" );
    }
}

void KRootIcon::initToolTip()
{
    // Does not work due to a Qt bug.
    KMimeType *typ = KMimeType::getMagicMimeType( url.data() );
    QString com = typ->getComment( url.data() );
    com.detach();

    if ( !com.isEmpty() )
	QToolTip::add( this, com.data() );
}

void KRootIcon::initFilename()
{
    // first calculate the default
    file = url.mid( url.findRev( "/" ) + 1, url.length() );
    if ( file.find( ".kdelnk" ) == ((int)file.length()) - 7 )
	file = file.left( file.length() - 7 );
    file.detach();
    KURL::decodeURL( file );
    // This changes "%2f" to "/"
    // Just for a nicer display
    decodeFileName( file );

    // the following code is taken out of kbind.cpp, where nearly
    // the same is run before this.
    // I didn't want to introduce new member functions, so it exists
    // twice
    QString decoded = url;

    QString n = decoded.data() + 5;

    QDir dir(n); // no static method available
    if (dir.exists()) // a directoy
    {	
      n += "/.directory";

      QFile f( n );
      if ( !f.open( IO_ReadOnly ) )
	return;
      f.close();

      KConfig sc(n);
      sc.setGroup("KDE Desktop Entry");
      file = sc.readEntry("Name", file);
    }
}

void KRootIcon::init()
{
    initFilename();
    QPainter p;
    p.begin( this );

    // Select font
    bIsLink = FALSE;
    QFont myfont( font() );
    KURL u( url );
    if ( !u.isMalformed() && strcmp( u.protocol(), "file" ) == 0 && !u.hasSubProtocol() )
    {
      struct stat lbuff;
      lstat( u.path(), &lbuff );
      if ( S_ISLNK( lbuff.st_mode ) )
      {
	myfont.setItalic( TRUE );
	p.setFont( myfont );
	bIsLink = TRUE;
      }
    }

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

    if ( bIsLink )
      p2.setFont( myfont );

    if ( root->iconStyle() == 1 && !bSelected )
	p2.drawText( textXOffset, textYOffset, file );
    else
      p2.fillRect( textXOffset-1, textYOffset-ascent-1, width+2, ascent+descent+2, color1 );

    if ( pixmap->mask() == 0L )
	p2.fillRect( pixmapXOffset, pixmapYOffset, pixmap->width(), pixmap->height(), color1 );
    else
	p2.drawPixmap( pixmapXOffset, pixmapYOffset, *pixmap->mask() );

    if ( bIsLink && link_pixmap && link_pixmap->mask() != 0 )
    {
	p2.setRasterOp( OrROP );
	p2.drawPixmap( pixmapXOffset, pixmapYOffset, *link_pixmap->mask() );
    }

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

    if ( bIsLink && link_pixmap )
	bitBlt( this, pixmapXOffset, pixmapYOffset, link_pixmap );

    QPainter p;
    p.begin( this );

    int ascent = p.fontMetrics().ascent();
    int descent = p.fontMetrics().descent();
    int width = p.fontMetrics().width( file.data() );

    QColor fillColor;

    if ( bSelected )
	fillColor = kapp->selectColor;
    else
	fillColor = root->iconBackground();

    p.fillRect( textXOffset-1, textYOffset-ascent-1, width+2,
	ascent+descent+2, fillColor );

    if ( bSelected )
	p.setPen( kapp->selectTextColor );
    else
	p.setPen( root->labelForeground() );

    QFont myfont( font() );
    if ( bIsLink )
    {
      myfont.setItalic( TRUE );
      p.setFont( myfont );
    }
    p.drawText( textXOffset, textYOffset, file );

    if ( bSelected )
    {
//      p.setRasterOp( NotEraseROP );
//      p.fillRect( pixmapXOffset, pixmapYOffset, pixmap->width(), pixmap->height(), blue );
	QBrush b( kapp->selectColor, Dense4Pattern );
        p.fillRect( pixmapXOffset, pixmapYOffset, pixmap->width(), pixmap->height(), b );
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

	select(true);

    }
    else if ( _mouse->button() == RightButton )
    {
	QStrList list;

        if ( !bSelected )
        {
	  // The selected URL is not marked, so unmark the marked ones.
          root->unselectAllIcons();
	  list.append( url.data() );
	  select( true );
	}
	else
        {
	  root->getSelectedURLs( list );
	}

	QPoint p = mapToGlobal( _mouse->pos() );
	root->openPopupMenu( list, p );
    }
    else if ( _mouse->button() == MidButton ){
	root->unselectAllIcons();	
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

    // Use the destination of the link. This looks better
    // and is what the user expects
    if ( bIsLink )
    {
      KURL u( url );
      char buffer[ 1024 ];
      int n;
      if ( ( n = readlink( u.path(), buffer, 1023 ) ) > 0 )
      {
	QString u2 = "file:";
	buffer[n] = 0;
	u2 += buffer;
	root->openURL( u2 );
	pressed = false;
	repaint();
	return;
      }
    }

    root->openURL( url );
    pressed = false;
    repaint();

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
		QString tmp = kapp->kde_datadir().copy();
		tmp += "/kfm/pics/kmultiple.xpm";
		pixmap2.load( tmp );
		if ( pixmap2.isNull() )
		    warning("KFM: Could not find '%s'\n",tmp.data());
	    }

	    // Proceed with the dragging operation only if the icon in which
	    // the drag started was selected too.
	    bool klicked = false;
	    QStrList _urls;
	    root->getSelectedURLs(_urls);
	    for (char* s = _urls.first(); s; s = _urls.next()) {
	        if (root->findIcon(s) == this) {
	            klicked = true;
	            break;
	        }
	    }
	    if (!klicked)
	        return;
	}

	QPoint p = mapToGlobal( _mouse->pos() );
	QPoint p2 = mapToGlobal( QPoint( press_x, press_y ) );
	int dx = QWidget::x() - p2.x() + pixmapXOffset;
	int dy = QWidget::y() - p2.y() + pixmapYOffset;


	if ( !pixmap2.isNull() ) {
	    // Multiple URLs slected.
	    startDrag( new KDNDIcon( pixmap2, p.x() + dx, p.y() + dy ), data.data(), data.length(), DndURL, dx, dy );
	} else
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
    resizeEvent( 0L );	// <-- Added by Lars ( 04/06/98 ): fixes bug when changing icon
    repaint();
}

void KRootIcon::rename( const char *_new_name )
{
    int pos = url.findRev( "/" );
    // Should never happen
    if ( pos == -1 )
	return;
    url = url.left( pos + 1 );
    QString tmp = _new_name;
    KURL::encodeURL( tmp );
    url += tmp;

    init();
    setGeometry( x(), y(), width, height );
    repaint();
}

void KRootIcon::select( bool _select )
{
  if ( bSelected == _select )
    return;

  bSelected = _select;

  update();
}

void KRootIcon::update()
{
  init();
  // We changed the mask in init(), so update it here
  resizeEvent( 0L );

  repaint();
}

void KRootIcon::dropPopupMenu( KDNDDropZone *_zone, const char *_dest, const QPoint *_p )
{
printf("dropPopupMenu\n");

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
          // QMessageBox::warning( 0, klocale->translate("KFM Error"),
	          // 			  klocale->translate("Dont know what to do.") );
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
      //QMessageBox::warning( 0, klocale->translate("KFM Error"),
	      //		      klocale->translate("Dont know what to do.") );
	return;
    }

    // Show the popup menu
    popupMenu->popup( *_p );
}

void KRootIcon::slotDropCopy()
{
    KIOJob * job = new KIOJob;
    job->copy( dropZone->getURLList(), dropDestination.data() );
    root->unselectAllIcons();
}

void KRootIcon::slotDropMove()
{
    KIOJob * job = new KIOJob;
    job->move( dropZone->getURLList(), dropDestination.data() );
    root->unselectAllIcons();
}

void KRootIcon::slotDropLink()
{
    KIOJob * job = new KIOJob;
    job->link( dropZone->getURLList(), dropDestination.data() );
    root->unselectAllIcons();
}

void KRootIcon::slotFontChanged()
{
    update();
}

KRootIcon::~KRootIcon()
{
    if ( popupMenu )
	delete popupMenu;

    delete dropZone;
}

#include "root.moc"
