#include <stdlib.h>
#include <qmsgbox.h>

#include "kfmtree.h"

/*************************************************
 *
 * CLASS KFMTree
 *
 ************************************************/

KFMTree::KFMTree( QWidget *_parent ) : KTreeList( _parent )
{
}

KFMTree::~KFMTree()
{
}
    
void KFMTree::mousePressEvent( QMouseEvent *_e )
{
    // find out which row has been clicked
    
    QPoint mouseCoord = _e->pos();
    int itemClicked = findRow( mouseCoord.y() );
    
    // if a valid row was not clicked, do nothing
    
    if( itemClicked == -1 ) 
	return;

    KTreeListItem *item = itemAt( itemClicked );
    if( !item )
	return;
  
    // translate mouse coord to cell coord
  
    int  cellX, cellY;
    colXPos( 0, &cellX );
    rowYPos( itemClicked, &cellY );
    QPoint cellCoord( mouseCoord.x() - cellX, mouseCoord.y() - cellY );

    if ( _e->button() == RightButton )
    {
	// hit test item
 
	if ( item->boundingRect(fontMetrics()).contains(cellCoord) )
	{
	    QPoint p = mapToGlobal( _e->pos() );
	    emit showPopup( itemClicked, p );
	}
    }
    else if ( _e->button() == MidButton )
    {
    }
    else
	KTreeList::mousePressEvent( _e );
}

/*************************************************
 *
 * CLASS KFMTreeView
 *
 ************************************************/

KFMTreeView::KFMTreeView( QWidget *parent ) : QWidget(parent )
{
  oldPath.setAutoDelete(TRUE);

  // load the pixmaps we'll be using

  QString d = getenv( "KDEDIR" );
  d += "/lib/pics/closed.gif";
  closedPixmap = new QPixmap();
  closedPixmap->load( d.data() );
  if ( closedPixmap->isNull() )
      QMessageBox::message( "KFM Error", "Could not find\n" + d );
  
  d = getenv( "KDEDIR" );
  d += "/lib/pics/open.gif";
  openPixmap = new QPixmap();
  openPixmap->load( d.data() );
  if ( openPixmap->isNull() )
      QMessageBox::message( "KFM Error", "Could not find\n" + d );
  
  // set up child widgets
  
  dirTree = new KFMTree(this);
  dirTree->setExpandLevel(1);
    
  // when an item in the directory tree is highlighted, selected or 
  // expanded, notify us so we can do something
  
  connect(dirTree, SIGNAL(highlighted(int)), this, SLOT(slotDirHighlighted(int)));
  connect(dirTree, SIGNAL(expanded(int)), this, SLOT(slotDirExpanded(int)));
  connect(dirTree, SIGNAL(selected(int)), this, SLOT(slotDirSelected(int)));
  connect(dirTree, SIGNAL(showPopup(int, QPoint&)), this, SLOT(slotShowPopup(int, QPoint&)));
  
  // set initial size
  resize(300, 400);
  
  // initializeTree();  
}

KFMTreeView::~KFMTreeView()
{
  // delete the pixmaps
  // child widgets will be deleted automatically
  
  delete closedPixmap;
  delete openPixmap;
}
      
void KFMTreeView::slotDirExpanded( int index )
{
  advanceReadDirectories(index);
}

// here's where we change the folder pixmaps and list the
// files in the highlighted directory

void KFMTreeView::slotDirHighlighted( int index )
{
    if ( !oldPath.isEmpty() )
    {
	dirTree->changeItem(0, closedPixmap, &oldPath);
	oldPath.clear();
    }

    oldPath = *dirTree->itemPath(index);
    dirTree->changeItem(0, openPixmap, &oldPath);

    /* QString dirPath;
    KPath pathCopy = oldPath;
    while( !pathCopy.isEmpty() )
    {
	dirPath.prepend(*pathCopy.pop());
	dirPath.prepend("/");
    }

    int i = 0;
    while ( dirPath.data()[i] == '/' )
	i++;
    QString d = "file:/";
    d += dirPath.data() + i;
    
    emit showDir( d.data() ); */
}

void KFMTreeView::slotShowPopup( int index, QPoint &_point )
{

    if ( !oldPath.isEmpty() )
    {
	dirTree->changeItem(0, closedPixmap, &oldPath);
	oldPath.clear();
    }

    oldPath = *dirTree->itemPath(index);
    dirTree->changeItem(0, openPixmap, &oldPath);
    QString dirPath;
    KPath pathCopy = oldPath;
    while( !pathCopy.isEmpty() )
    {
	dirPath.prepend(*pathCopy.pop());
	dirPath.prepend("/");
    }

    int i = 0;
    while ( dirPath.data()[i] == '/' )
	i++;
    QString d = "file:/";
    d += dirPath.data() + i;
    
    emit popupMenu( d.data(), _point );
    
    printf("!!!!!!! POPUP %s\n",d.data());
}

void KFMTreeView::slotDirSelected(int index)
{
    dirTree->expandOrCollapseItem( index );

    oldPath = *dirTree->itemPath(index);
    dirTree->changeItem(0, openPixmap, &oldPath);
    QString dirPath;
    KPath pathCopy = oldPath;
    while( !pathCopy.isEmpty() )
    {
	dirPath.prepend(*pathCopy.pop());
	dirPath.prepend("/");
    }

    int i = 0;
    while ( dirPath.data()[i] == '/' )
	i++;
    QString d = "file:/";
    d += dirPath.data() + i;
    
    emit showDir( d.data() );
    
    printf("!!!!!!! SHOW %s\n",d.data());
}

// reads directories one level deeper in advance of tree expansion

void KFMTreeView::advanceReadDirectories( int index )
{
    KTreeListItem *parentItem = dirTree->itemAt( index );
    KPath *treePath = dirTree->itemPath( index );
    KTreeListItem *childItem = parentItem->getChild();

    while( childItem )
    {
	if ( childItem->hasChild() )
	    return; // already read!!!

	const char *childDir = childItem->getText();
	KPath treePathCopy = *treePath;
	QString filePath = childDir;

	while( !treePathCopy.isEmpty() )
	{
	    filePath.prepend("/");
	    filePath.prepend(*treePathCopy.pop());
	}

	treePath->push( new QString(childDir) );

	QDir dir( filePath );
	const QStrList *dirs = dir.entryList(QDir::Dirs | QDir::Readable);

	if ( dirs && dirs->count() != 2 )
	{ // prevent core dumps in /proc!
	    QStrListIterator i(*dirs);
	    for( ; i.current(); ++i )
	    {
		QString currDir(i.current());
		if(currDir == "." || currDir == "..")
		    continue;
		dirTree->addChildItem(currDir, closedPixmap, treePath);
	    }
	}

	childItem = childItem->getSibling();
	treePath->pop();
    }

    delete treePath;
}
      
// reads root directory and fills the tree list to two levels deep

void KFMTreeView::initializeTree()
{
    QDir rootDir = QDir::root();
    QString rootName = rootDir.absPath();
    dirTree->insertItem(rootName, closedPixmap, -1);
    KPath path;
    path.push(&rootName);
    const QStrList *dirs = rootDir.entryList( QDir::Dirs | QDir::Readable );

    if( dirs->count() == 2 )
	return;

    QStrListIterator i(*dirs);

    for ( ; i.current(); ++i )
    {
	QString currDir(i.current());
	if ( currDir == "." || currDir == "..")
	    continue;

	dirTree->addChildItem( currDir, closedPixmap, &path );
	QDir subDir( rootDir.absFilePath( currDir ) );
	const QStrList *subDirs = subDir.entryList( QDir::Dirs | QDir::Readable );
	if ( subDirs->count() == 2 )
	    continue;
	QStrListIterator j( *subDirs );
	path.push( new QString( currDir ) );

	for( ; j.current(); ++j )
	{
	    QString currSubDir( j.current() );
	    if ( currSubDir != "." && currSubDir != ".." )
		dirTree->addChildItem( currSubDir, closedPixmap, &path );
	}

	QString *junk = path.pop();
	delete junk;
    }
}

// if the main window is resized, adjust the widget layout

void KFMTreeView::resizeEvent(QResizeEvent *e)
{
    dirTree->setGeometry( 0, 0, width(), height() );
}

#include "kfmtree.moc"
