#include "kfinder.h"
#include <qscrbar.h>
#include <kapp.h>

#include "config-kfm.h"

QPixmap* KFinder::openPixmap = 0L;
QPixmap* KFinder::closePixmap = 0L;

KFinder::KFinder( QWidget *_parent, QStrList *_headers, QArray<int> *_sizes ) : QWidget( _parent )
{
    xOffset = 0;
    
    getOpenPixmap();
    getClosePixmap();

    buttonList.setAutoDelete( TRUE );
    
    sizeList = 0L;
    currentSizeList = 0L;
    
    if ( _headers )
    {
	sizeList = new unsigned int[ _headers->count() ];
	currentSizeList = new unsigned int[ _headers->count() ];
    }
    else
    {
	sizeList = new unsigned int[ 1 ];
	currentSizeList = new unsigned int[ 1 ];
    }
    
    if ( _headers )
	for ( unsigned int i = 0; i < _headers->count(); i++ )
	{
	    KFinderButton *but;
	    buttonList.append( but = new KFinderButton( _headers->at( i ), this ) );
	    connect( but, SIGNAL( pressed( KFinderButton* ) ), this, SLOT( slotButtonPressed( KFinderButton* ) ) );

	    if ( _sizes )
	    {
		currentSizeList[i] = (*_sizes)[i];
		sizeList[ i ] = (*_sizes)[i];
	    }
	    else
	    {
		// Just a guess
		currentSizeList[i] = 150;
		sizeList[ i ] = 150;
	    }
	}

    sort = 0;

    finderWin = new KFinderWin( this );
    connect( finderWin->horizontalScrollBar(), SIGNAL( valueChanged( int ) ), this, SLOT( slotScrolled( int ) ) );
    // Dirty Hack, sorry
    QScrollBar *s = (QScrollBar*)finderWin->horizontalScrollBar();
    s->setTracking( TRUE );

    initColors();

}

void KFinder::initColors(){

    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup( "KFM HTML Defaults" );	

    bgColor = config->readColorEntry( "BgColor", &white );
    textColor = config->readColorEntry( "LinkColor", &black );

    if(finderWin)
      finderWin->setBackgroundColor( bgColor );

}

void KFinder::setColors(QColor bgcolor,QColor textcolor){

  bgColor = bgcolor;
  if(finderWin)
    finderWin->setBackgroundColor(bgColor);
  textColor = textcolor;

}

QColor KFinder::getTextColor(){
  
  return textColor;

}

void KFinder::slotButtonPressed( KFinderButton *_but )
{
    sort = buttonList.findRef( _but );
    QPushButton *but;
    for ( but = buttonList.first(); but != 0L; but = buttonList.next() )
	if ( but != _but )
	    but->setOn( FALSE );
    finderWin->repaint();

    emit buttonPressed( sort );
}
    
void KFinder::resizeEvent( QResizeEvent * )
{
    int x = -xOffset;
    unsigned int y = 0;

    // Which size do we need minimum ?
    int w = 0;
    for ( unsigned int j = 0; j < buttonList.count(); j++ )
	w += sizeList[ j ];
    
    // Is there space left ?
    if ( w <= width() )
    {
	int i = 0;
	QPushButton *but;
	for ( but = buttonList.first(); but != 0L; but = buttonList.next() )
	{
	    if ( but == buttonList.getLast() )
	    {
		but->setGeometry( x, 0, width() - x, 20 );
		currentSizeList[i] = width() - x;
	    }
	    else
	    {
		but->setGeometry( x, 0, width() * sizeList[ i ] / w, 20 );
		currentSizeList[i] = width() * sizeList[ i ] / w;
		x += width() * sizeList[ i++ ] / w;
	    }
	}
    }
    // To few space available
    else
    {
	int i = 0;
	QPushButton *but;
	for ( but = buttonList.first(); but != 0L; but = buttonList.next() )
	{
	    but->setGeometry( x, 0, sizeList[ i ], 20 );
	    currentSizeList[ i ] = sizeList[ i ];
	    x += sizeList[ i++ ];
	}
    }
    
    if ( buttonList.count() != 0 )
	y += 20;
    
    finderWin->setGeometry( 0, y, width(), height() - y );
}

void KFinder::changeTree( KFinderNode *_node )
{
    finderWin->changeTree( _node );
}

void KFinder::updateTree( bool _repaint )
{
    finderWin->updateTree( _repaint );
}

void KFinder::slotScrolled( int _value )
{
    xOffset = _value;
    resizeEvent( 0L );
}

QPixmap& KFinder::getOpenPixmap()
{
    QString f = kapp->kde_datadir().copy();
    f += "/kfm/pics/open.xpm";
    
    if ( openPixmap == 0L )
    {
	openPixmap = new QPixmap();
	openPixmap->load( f );
    }

    return *openPixmap;
}

QPixmap& KFinder::getClosePixmap()
{
    QString f = kapp->kde_datadir().copy();
    f += "/kfm/pics/close.xpm";

    if ( closePixmap == 0L )
    {
	closePixmap = new QPixmap();
	closePixmap->load( f );
    }

    return *closePixmap;
}

void KFinder::emitDrop( QStrList &_urls, const QPoint &_point )
{
    emit drop( _urls, _point );
}

KFinderItem* KFinder::first() { return finderWin->first(); }
KFinderItem* KFinder::next() { return finderWin->next(); }

void KFinder::itemList( QList<KFinderItem> &_list )
{
    _list.clear();
    
    QList<KFinderNode> nodeList;
    nodeList.setAutoDelete( FALSE );
    nodeList.append( finderWin->node );
    
    KFinderNode *n;
    KFinderItem *item;
 
    QListIterator<KFinderNode> it(nodeList);
    for ( ; it.current(); ++it )
    {   
	n = it.current();
	for ( item = n->first(); item != 0L; item = n->next() )
	{
	    _list.append( item );
	    nodeList.append( item->node() );
	}
    }
}

KFinder::~KFinder()
{
    if ( finderWin )
	delete finderWin;
    if ( sizeList )
	delete []sizeList;
    if ( currentSizeList )
	delete []currentSizeList;
}

KFinderWin::KFinderWin( KFinder *_finder ) : QTableView( _finder )
{
    node = 0L;
    
    itemList.setAutoDelete( FALSE );

    
    finder = _finder;

    setNumCols( finder->buttonList.count()!=0 ? finder->buttonList.count() : 1 );
    setNumRows( 0 );    
    setCellHeight( CELL_HEIGHT );
    setTableFlags( Tbl_autoScrollBars | /* Tbl_clipCellPainting | */ Tbl_smoothScrolling );

    dropZone = new KDNDDropZone( this , DndURL );
    connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ),
	     this, SLOT( slotDropEvent( KDNDDropZone *) ) );
}


void KFinderWin::slotDropEvent( KDNDDropZone *_zone )
{
    QPoint p = QPoint( _zone->getMouseX(), _zone->getMouseY() );
    
    QPoint p2 = mapFromGlobal( p );
    int row = findRow( p2.y() );
    if ( row == -1 )
    {
	finder->emitDrop( _zone->getURLList(), p );
	return;
    }
    
    itemList.at( row )->dropEvent( _zone->getURLList(), p );
}

void KFinderWin::changeTree( KFinderNode *_node )
{
    node = _node;

    itemList.clear();    
    if ( node )
	node->fillItemList( itemList, 0 );
    setNumRows( itemList.count() );
}

void KFinderWin::updateTree( bool _repaint )
{
    if ( node == 0L )
	return;
    
    itemList.clear();    
    node->fillItemList( itemList, 0 );
    setNumRows( itemList.count() );
    if ( _repaint )
	repaint();
}

void KFinderWin::setOffsets( int _xpos, int _ypos )
{
    if ( _ypos > maxYOffset() )
	setYOffset( maxYOffset() );
    else
	setYOffset( _ypos );
	
    if ( _xpos > maxXOffset() )
	setXOffset( maxXOffset() );
    else
	setXOffset( _xpos );
}

void KFinderWin::offsets( int &_xpos, int &_ypos )
{
    _ypos = yOffset();
    _xpos = xOffset();
}

void KFinderWin::paintCell( QPainter * _painter, int _row, int _col )
{
    /* if ( finder->sort == _col || finder->buttonList.count() == 0 )
    {
	_painter->fillRect( 0, 0, cellWidth( _col ), cellHeight(), lightGray );
    }
    else
    {
	QColor c( 230, 230, 230 );
	_painter->fillRect( 0, 0, cellWidth( _col ), cellHeight(), c );
    }

    _painter->setPen( white );
    _painter->drawLine( 0, cellHeight() - 1, width(), cellHeight() - 1 ); */
    
    itemList.at( _row )->paintCell( _painter, _col );
}

int KFinderWin::cellWidth( int _column )
{
    if ( finder->buttonList.count() == 0 )
	return viewWidth();
    
    return finder->currentSizeList[ _column ];
}

void KFinderWin::mousePressEvent( QMouseEvent *_ev )
{
    int row = findRow( _ev->pos().y() );
    if ( row == -1 )
	return;

    QPoint p( _ev->pos().x() - xOffset(), _ev->pos().y() );
    QPoint p2( mapToGlobal( _ev->pos() ) );
    QMouseEvent ev( Event_MouseButtonPress, p, _ev->button(), _ev->state() );
    
    itemList.at( row )->mousePressEvent( &ev, p2 );
}

KFinderWin::~KFinderWin()
{
    if ( dropZone )
	delete dropZone;
}

KFinderItem::KFinderItem( KFinder *_finder )
{
    bOpened = FALSE;
    finderNode = new KFinderNode;
    level = 0;
        
    finder = _finder;
}

void KFinderItem::mousePressEvent( QMouseEvent *_ev, const QPoint &_globalPoint )
{
  pressed( _ev, _globalPoint );

  finder->updateTree();
}

void KFinderItem::fillItemList( QList<KFinderItem>& _list, int _level  )
{
    if ( !bOpened )
	return;
    
    if ( finderNode )
	finderNode->fillItemList( _list, _level + 1 );
}

KFinderItem::~KFinderItem()
{
    if ( finderNode )
	delete finderNode;
}

KFinderNode::KFinderNode()
{
    bAutoDelete = TRUE;
    itemList.setAutoDelete( bAutoDelete );
}

void KFinderNode::fillItemList( QList<KFinderItem>& _list, int _level )
{
    KFinderItem *item;
    for ( item = itemList.first(); item != 0L; item = itemList.next() )
    {
	item->setLevel( _level );
	_list.append( item );
	item->fillItemList( _list, _level );
    }
}

KFinderButton::KFinderButton( const char *_text, KFinder *_finder ) : QPushButton( _text, _finder )
{
    connect( this, SIGNAL( pressed() ), this, SLOT( slotPressed() ) );
    setToggleButton( TRUE );
}

void KFinderButton::slotPressed()
{
    emit pressed( this );
}

#include "kfinder.moc"
