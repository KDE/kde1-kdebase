#include <kurl.h>
#include <sys/stat.h>

#include <limits.h>

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <kapp.h>
#include <qmsgbox.h>
#include <qdir.h>

#include "kfmtree.h"
#include "kioserver.h"
#include "kiojob.h"
#include "kbind.h"
#include "config-kfm.h"
#include "kfmview.h"
#include "kfmprops.h"
#include "kfmgui.h"
#include "kfmpaths.h"
#include "utils.h"
#include "kfmdlg.h"
#include "kintlist.h"


KFMDirTree::KFMDirTree( QWidget *_parent, KfmGui *_gui ) : KFinder( _parent )
{
    connect( KIOServer::getKIOServer(), SIGNAL( notify( const char * ) ), this,
    	     SLOT( slotDirectoryChanged( const char * ) ) );

    gui = _gui;
    popupMenu = new QPopupMenu();

    menuNew = new KNewMenu();
   
    lastSelectedItem = 0L;
}

KFMDirTree::~KFMDirTree()
{
    delete menuNew;
    delete popupMenu;
}

void KFMDirTree::fill()
{
    QString desktop( KFMPaths::DesktopPath() );
    if ( desktop.right(1) == "/" )
      desktop.truncate( desktop.length() - 1 );

    KFMDirTreeItem *item = new KFMDirTreeItem( this, "/", FALSE );
    node.append( item );
    item = new KFMDirTreeItem( this, QDir::homeDirPath(), FALSE );
    node.append( item );
    item = new KFMDirTreeItem( this, desktop, FALSE );
    slotSelectItem( item );
    node.append( item );

    changeTree( &node );
}

// new slot for updating tree view Sep 5 rjakob
// _url is an encoded URL
void KFMDirTree::slotshowDirectory(const char *_url )
{
    QString tmp2;
    bool found;
    int loops;
    bool urlBelowBasePath = false;

    int ypos, xpos, myindex;
    finderWin->offsets( xpos, ypos );

    KURL u( _url );
    if ( u.isMalformed() )
	return;

    finderWin->setAutoUpdate( false );

    KFinderItem *item;
    KFMDirTreeItem *kfmitem;
    KFMDirTreeItem *selectedSubTree = 0L;

    QString tmp (u.url());
    if ( tmp.right(1) == "/" )
        tmp.truncate(tmp.length() - 1);
    loops=0;

    int comparelength=tmp.length();

    if( lastSelectedItem )
    {
      QString basePath(lastSelectedItem->getURL());
      if( basePath.right(1) == "/" )
        basePath.truncate(basePath.length() - 1);
      int curLevel = lastSelectedItem->getLevel();
      int truncPos;
      // truncate curLevel levels from the url
      for( truncPos = basePath.length(); (truncPos >= 0) && curLevel; truncPos--)
	if( (basePath[truncPos] == '/') && !(--curLevel))
	  break;
    
      // now we get the path of the top most item of the subtree the contains
      // the currently selecteditem
      // the three subtrees are Root, My Home and Desktop
      basePath.truncate( truncPos );
      // check if the url is in the same subtree as the currently selected item
      urlBelowBasePath = basePath == tmp.left(basePath.length());
 
      // get the root item for the currently selected subtree
      if( urlBelowBasePath ) 
      {
	for( item = node.first(); item != 0L; item = node.next() )
	{
	  selectedSubTree = (KFMDirTreeItem*)item;
	  QString nodeURL( selectedSubTree->getURL() );
	  if( nodeURL.right(1) == "/" )
	    nodeURL.truncate(nodeURL.length() - 1);
	  if( nodeURL == basePath )
	    break;
	}
      }
    }

    while(1) {

        myindex=0;

        found=false;

	// if url is in the same subtree just skip the preceding subtrees
	// just to speed it up a little
        if( urlBelowBasePath )
	  for ( item = first(); item != selectedSubTree; item = next() );
	else
	  item = first();

        for ( ; item != 0L; item = next() ) {
            kfmitem = (KFMDirTreeItem*)item;
            myindex++;

            tmp2 = kfmitem->getURL();
	    if ( tmp2.right(1) == "/" )
	      tmp2.truncate(tmp2.length() - 1);

	    QString dummy = tmp.left(comparelength);

            if ( !qstrncmp(tmp,tmp2,comparelength) && dummy == tmp2 ) {
	        kfmitem->setOpen(true);
    	        updateTree(false);
	        found=true;
	        if (comparelength<(int)tmp.length()) {
		    comparelength=tmp.length();
		    break;  // break the for() loop
		    }
                setBranchVisible( kfmitem );
		slotSelectItem( kfmitem ); 
                finderWin->setAutoUpdate( true );
    	        finderWin->repaint();
	        return;
	    }
        }
        if (found) continue;
        if (comparelength==1) {
	    debug("should never happen!\n");
	    break;
	    }
        comparelength--;
        while( (comparelength>0) && (tmp.at(comparelength-1)!='/') )
    	    comparelength--;
        comparelength--;
        if (comparelength<=0) comparelength=1;
        if (++loops>128) break;     // race condition ?
        }
    debug("\"%s\" not opened\n",tmp.data());

    finderWin->setAutoUpdate( true );
}

void KFMDirTree::slotDirectoryChanged( const char *_url )
{
    KURL u( _url );
    if ( u.isMalformed() )
	return;
    
    // Prepare the path in a way that allows quick comparison
    // with the return value of KFMDirTreeItem::getURL
    QString tmp( u.url() );
    if ( tmp.right(1) == "/" )
	tmp.truncate( tmp.length() - 1 );

    // Do we display this URL ?
    KFinderItem *item;
    KFMDirTreeItem *kfmitem;
    for ( item = first(); item != 0L; item = next() )
    {
	kfmitem = (KFMDirTreeItem*)item;

	if ( tmp == kfmitem->getURL() )
	{
	    update();
	    return;
	}
    }
}

void KFMDirTree::update()
{
    finderWin->setAutoUpdate( false );

    int ypos, xpos;
    finderWin->offsets( xpos, ypos );

    bool show_dots = gui->isShowDot();
    
    // Get a list of all visisble items
    QList<KFinderItem> openList;
    openList.setAutoDelete( FALSE);
    itemList( openList );

    // remebmer the currently selected Item
    QString curSelectedURL;
    int curSelectedLevel = 0;

    // Find all open items
    QStrList openURLList;
    QDict<KIntList> openURLLevelDict;
    openURLLevelDict.setAutoDelete( true );
    KFinderItem *item;
    KFMDirTreeItem *kfmitem;

    for ( item = openList.first(); item != 0L; item = openList.next() )
    {
	kfmitem = (KFMDirTreeItem*)item;
	if ( kfmitem->isOpen() )
	{
	    // Ignore opened directories that start with "." if
	    // the user does not want to see the dot files.
	    if ( *kfmitem->getURL() != '.' || show_dots ) {
		openURLList.append( kfmitem->getURL() );

                KIntList* levelList;
                int* level = new int;
                *level = kfmitem->getLevel();
                if( !(levelList = openURLLevelDict.find( kfmitem->getURL() )) ) {
                    levelList = new KIntList;
                    levelList->setAutoDelete( true );
                    openURLLevelDict.insert( kfmitem->getURL(), levelList );
                }

                levelList->append( level );
            }
	}
        if( kfmitem->isSelected() )
	{
	  curSelectedURL = kfmitem->getURL();
	  curSelectedLevel = kfmitem->getLevel();
	}
    }

    // Clean the tree
    node.clear();
    slotSelectItem( 0L );
    // Fill the tree with level 0
    fill();
    
    // Reopen every directory that was open before
    QList<KFinderNode> nodeList;
    nodeList.setAutoDelete( FALSE );
    nodeList.append( &node );
    KFinderNode *n;

    QListIterator<KFinderNode> it(nodeList);
    for ( ; it.current(); ++it )
    {   
	n = it.current();
	for ( item = n->first(); item != 0L; item = n->next() )
	{
	    kfmitem = (KFMDirTreeItem*)item;
	    // Was this item open ?
	    if ( openURLList.find( kfmitem->getURL() ) != -1 )
	    {
                KIntList* levelList;
                int level = kfmitem->getLevel();
                if( (levelList = openURLLevelDict.find(kfmitem->getURL())) ) {
                    if( levelList->find( &level ) != -1 ) {
                        // Open it again
       		        item->setOpen( TRUE );
		        // Traverse its items, too
		        nodeList.append( item->node() );
                    }
                } 
                else
                  debug("kfmtree.cpp: should never happen!!!");
	    }
            if( curSelectedURL == kfmitem->getURL() && 
                curSelectedLevel == kfmitem->getLevel() )
              slotSelectItem( kfmitem );
	}
    }

    // Initialize but dont paint
    updateTree( FALSE );

    // Adjust scrollbars to original position if possible
    finderWin->setOffsets( xpos, ypos );
    
    finderWin->setAutoUpdate( true );

    finderWin->repaint();
}

void KFMDirTree::openDropMenu( const char *_dest, QStrList &_urls, const QPoint &_p )
{
    dropDestination = _dest;
    dropDestination.detach();
    dropSource.copy( _urls );
    
    popupMenu->clear();

    int id = -1;
    // Ask wether we can read from the dropped URL.
    if ( KIOServer::supports( _urls, KIO_Read ) &&
	 KIOServer::supports( _dest, KIO_Write ) )
	id = popupMenu->insertItem(  klocale->translate("Copy"), 
				     this, SLOT( slotDropCopy() ) );
    // Ask wether we can read from the URL and delete it afterwards
    if ( KIOServer::supports( _urls, KIO_Move ) &&
	 KIOServer::supports( _dest, KIO_Write ) )
	id = popupMenu->insertItem(  klocale->translate("Move"),
				     this, SLOT( slotDropMove() ) );
    // Ask wether we can link the URL 
    if ( KIOServer::supports( _dest, KIO_Link ) )
	id = popupMenu->insertItem(  klocale->translate("Link"), 
				     this, SLOT( slotDropLink() ) );
    if ( id == -1 )
    {
	QMessageBox::warning( 0, klocale->translate("KFM Error"),
			       klocale->translate("Dont know what to do.") );
	return;
    }

    // Show the popup menu
    popupMenu->popup( _p );
}

void KFMDirTree::slotDropCopy()
{
    KIOJob * job = new KIOJob;
    job->copy( dropSource, dropDestination.data() );
}

void KFMDirTree::slotDropMove()
{
    KIOJob * job = new KIOJob;
    job->move( dropSource, dropDestination.data() );
}

void KFMDirTree::slotDropLink()
{
    KIOJob * job = new KIOJob;
    job->link( dropSource, dropDestination.data() );
}

void KFMDirTree::openPopupMenu( const char *_url, const QPoint &_point )
{
    popupMenu->clear();

    // Store for later use
    popupDir = _url;

    if ( KIOServer::isTrash( _url ) )
    {
	int id;
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_CD), 
				    this, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_NEW_VIEW), 
				    this, SLOT( slotPopupNewView() ) );
	popupMenu->insertSeparator();    
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_TRASH), 
				    this, SLOT( slotPopupEmptyTrashBin() ) );
    } 
    else
    {
	int id;
        id = popupMenu->insertItem( klocale->translate("&New"), menuNew );
        popupMenu->insertSeparator();

	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_CD),
				    this, SLOT( slotPopupCd() ) );
	id = popupMenu->insertItem( klocale->getAlias(ID_STRING_NEW_VIEW), 
				    this, SLOT( slotPopupNewView() ) );
	popupMenu->insertSeparator();    
	if ( KIOServer::supports( _url, KIO_Read ) )
	    id = popupMenu->insertItem( klocale->translate("Copy"), 
					this, SLOT( slotPopupCopy() ) );
	if ( KIOServer::supports( _url, KIO_Write ) && KfmView::clipboard->count() != 0 )
	    id = popupMenu->insertItem( klocale->translate("Paste"), 
					this, SLOT( slotPopupPaste() ) );
	if ( KIOServer::supports( _url, KIO_Move ) )
	    id = popupMenu->insertItem( klocale->translate("Move to Trash"),  
					this, SLOT( slotPopupTrash() ) );
	if ( KIOServer::supports( _url, KIO_Delete ) )
	    id = popupMenu->insertItem( klocale->translate("Delete"),  
					this, SLOT( slotPopupDelete() ) );
    }

    popupMenu->insertItem( klocale->translate("Add To Bookmarks"), 
			   this, SLOT( slotPopupBookmarks() ) );
    popupMenu->insertSeparator();    
    popupMenu->insertItem( klocale->translate("Properties"),
			   this, SLOT( slotPopupProperties() ) );
    popupMenu->popup( _point );
    menuNew->setPopupFiles ( _url );
}

void KFMDirTree::slotPopupNewView()
{
    KfmGui *m = new KfmGui( 0L, 0L, popupDir );
    m->show();
}

void KFMDirTree::slotPopupCd()
{
    emit urlSelected( popupDir, LeftButton );
}

void KFMDirTree::slotPopupProperties()
{
    (void)new Properties( popupDir );
}

void KFMDirTree::slotPopupBookmarks()
{
    gui->addBookmark( popupDir, popupDir );
}

void KFMDirTree::slotPopupEmptyTrashBin()
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

void KFMDirTree::slotPopupCopy()
{
    KfmView::clipboard->clear();
    KfmView::clipboard->append( popupDir );
}

void KFMDirTree::slotPopupPaste()
{
    // Check wether we drop a directory on itself or one of its children
    int nested = 0;
    char *s;
    for ( s = KfmView::clipboard->first(); s != 0L; s = KfmView::clipboard->next() )
    {
	int j;
	if ( ( j = testNestedURLs( s, popupDir ) ) )
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
    job->copy( (*KfmView::clipboard), popupDir );
}

void KFMDirTree::slotPopupTrash()
{
    // This function will emit a signal that causes us to redisplay the
    // contents of our directory if neccessary.
    KIOJob * job = new KIOJob;
    
    QString dest = "file:" + KFMPaths::TrashPath();
 
    job->setOverWriteExistingFiles( TRUE );
    QStrList list;
    list.append( popupDir );
    job->move( list, dest );
}


void KFMDirTree::slotPopupDelete()
{   
    // Is the user really sure ?
    bool ok = !QMessageBox::warning( 0, klocale->translate("KFM Warning"), 
				  klocale->translate("Do you really want to delete the selected file(s)?\n\nThere is no way to restore them."), 
				  klocale->translate("Yes"), 
				  klocale->translate("No") );
    if ( ok )
    {
	QString str( popupDir.data() );
	KURL::decodeURL( str );
	KIOJob * job = new KIOJob;
	job->del( str );
    }
}

void KFMDirTree::emitUrlSelected( const char *_url, int _button )
{
    emit urlSelected( _url, _button );
}

void KFMDirTree::slotSelectItem( KFMDirTreeItem* item )
{
  if( lastSelectedItem )
    lastSelectedItem->setSelected( false );

    lastSelectedItem = item;

  if( item )
    lastSelectedItem->setSelected( true );
}

KFMDirTreeItem::KFMDirTreeItem( KFMDirTree *_finder, const char *_url, bool _isfile  ) : KFinderItem( _finder )
{
    dirTree = _finder;
    
    bFilled = FALSE;
    bIsFile = _isfile;
    bIsSelected = false;
    
    QString tmp = _url;
    if ( tmp.right(1) == "/" && tmp != "/" && tmp.right(2) != ":/" )
	tmp.truncate( tmp.length() - 1 );
    KURL u( tmp );
    url = u.url(); // returns a URL with file: and encoded
   
    QString home( QDir::homeDirPath() );
    if ( home.right(1) == "/" )
	home.truncate( home.length() -1 );
    QString desk( KFMPaths::DesktopPath().data() );
    if ( desk.right(1) == "/" )
	desk.truncate( home.length() -1 );
    
    if ( home == u.path() )
	name = klocale->translate( "My Home" );
    else if ( desk == u.path() )
	name = klocale->translate( "Desktop" );    
    else if ( strcmp( u.path(), "/" ) == 0 )
	name = klocale->translate( "Root" );
    else
	name = u.filename();

    // Find the correct icon
    QString pixmapFile(KMimeType::getPixmapFileStatic( url, TRUE ) );
    // Is the icon cached ?
    folder_normal = KMimeType::pixmapCache->find( pixmapFile );
    // If not => create a new icon
    if ( folder_normal == 0L )
    {
	folder_normal = new QPixmap();
	folder_normal->load( pixmapFile );
	KMimeType::pixmapCache->insert( pixmapFile, folder_normal );
    }    
    pixmap = folder_normal;

    QString folder_openedFile( KApplication::kde_icondir() );
    if( folder_openedFile.right(1) != "/" )
      folder_openedFile.append("/");
    folder_openedFile.append("mini/folder_open.xpm");

    folder_opened = KMimeType::pixmapCache->find( folder_openedFile );
    // If not => create a new icon
    if ( folder_opened == 0L )
    {
	folder_opened = new QPixmap();
	folder_opened->load( folder_openedFile );
	KMimeType::pixmapCache->insert( folder_openedFile , folder_opened );
    }    
}

void KFMDirTreeItem::paintCell( QPainter *_painter, int _col )
{
    int x = 0;
    
    x += ( PIXMAP_WIDTH + 6 ) * level;

    if ( _col == 0 )
      {
	if ( !bIsFile )
	  {
	    if ( bOpened )
	      _painter->drawPixmap( QPoint( x + 4, 
	      ( CELL_HEIGHT - PIXMAP_HEIGHT ) / 2 ), KFinder::getOpenPixmap() );
	    else
	      _painter->drawPixmap( QPoint( x + 4, 
	      ( CELL_HEIGHT - PIXMAP_HEIGHT ) / 2 ), KFinder::getClosePixmap() );
	}
	
	x += PIXMAP_WIDTH + 4;
	
	QFontMetrics fm = _painter->fontMetrics();

	_painter->drawPixmap( QPoint( x + 6, ( CELL_HEIGHT - PIXMAP_HEIGHT ) / 2 ), 
			      *pixmap );

	_painter->setPen( dirTree->getTextColor() );

	QString text = name;
	decodeFileName(text); // for directories with endoded '/' in their name
	_painter->drawText( x + 6 + PIXMAP_WIDTH + 6,
	    ( CELL_HEIGHT - fm.ascent() - fm.descent() ) / 2 + fm.ascent(), text );
    }
}

int KFMDirTreeItem::width() const
{
    int w = ( PIXMAP_WIDTH + 6 ) * level + 2 * PIXMAP_WIDTH + 18;

	QFontMetrics fm( finder->font() );

    w += fm.width( name );

    return w;
}

void KFMDirTreeItem::pressed( QMouseEvent *_ev, const QPoint &_globalPoint  )
{
    if ( _ev->button() == RightButton )
    {
	dirTree->openPopupMenu( url, _globalPoint );
	return;
    }
      
    if ( bIsFile )
    {
	dirTree->emitUrlSelected( url, _ev->button() );
	return;
    }
    
    int x = ( PIXMAP_WIDTH + 6 ) * level;
    if ( _ev->pos().x() >= x + PIXMAP_WIDTH + 4 )
    {
        dirTree->slotSelectItem( this );
	dirTree->emitUrlSelected( url, _ev->button() );
	return;
    }

    setOpen( !bOpened );
}

void KFMDirTreeItem::setOpen( bool _open )
{
    if ( bOpened == _open )
	return;

    if ( bFilled )
    {
	bOpened = _open;
	return;
    }
    
    bOpened = _open;
    
    KURL u( url );
    
    DIR *dp = 0L;
    struct dirent *ep;
    struct stat buff;

    dp = 0L;
    dp = opendir( u.path() );
    if ( dp == 0L )
    {
	warning( "Could not enter directory %s", url.data() );
	return;
    }
    
    bool show_dots = dirTree->getGui()->isShowDot();
    
    QStrList sort_list;

    while ( ( ep = readdir( dp ) ) != 0L )
    {
	QString name(ep->d_name);
	if ( name != "." && name != ".." )
	{
	    if ( *ep->d_name != '.' || show_dots )
		sort_list.inSort( ep->d_name );
	}
    }
    
    closedir( dp );    

    const char *s;
    for ( s = sort_list.first(); s != 0L; s = sort_list.next() )
    {
	QString fname ( u.path() );
	if (fname.right(1) != "/") fname += "/";
	fname += s;
	 
	if (stat( fname, &buff )) { warning("Could not stat %s",fname.data()); }

	// For symlinks, follow them to stat the real file or directory
	if ( S_ISLNK( buff.st_mode ) ) { lstat( fname, &buff ); }

	if ( S_ISDIR( buff.st_mode ) )
	{
	    // create item, using encoded path
	    KFMDirTreeItem *item = new KFMDirTreeItem( dirTree, fname, FALSE );
	    item->setLevel( getLevel() + 1 );
	    finderNode->append( item );
	}
    }
    
    bFilled = TRUE;    
}

void KFMDirTreeItem::setSelected( bool enable )
{
  bIsSelected = enable;
  if( enable )
    pixmap = folder_opened;
  else
    pixmap = folder_normal;
}

bool KFMDirTreeItem::isSelected( void )
{
  return bIsSelected;
}

void KFMDirTreeItem::dropEvent( QStrList &_urls, const QPoint &_point )
{
    dirTree->openDropMenu( url, _urls, _point );
}

KFMDirTreeItem::~KFMDirTreeItem()
{
}

#include "kfmtree.moc"
