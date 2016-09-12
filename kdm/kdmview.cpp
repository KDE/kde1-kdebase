    /*

    User view for kdm. Inspired by QListBox
    $Id: kdmview.cpp,v 1.7 1998/09/04 15:33:09 bieker Exp $

    Copyright (C) 1997, 1998 Steffen Hansen
                             stefh@mip.ou.dk


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */
 

#include "kdmview.h"
#include <math.h>
#include <qwmatrix.h>
#include <qkeycode.h>

int KVItemList::compareItems( GCI i1, GCI i2)
{
    KDMViewItem *lbi1 = (KDMViewItem *)i1;
    KDMViewItem *lbi2 = (KDMViewItem *)i2;
    return strcmp( lbi1->text(), lbi2->text() );
}

/* My g++ gives internal compiler errors if i don't make a destructor */
KVItemList::~KVItemList()
{
}

/*****************************KDMViewItem****************************/

void
KDMViewItem::paint( QPainter* p, KDMView* v, enum State s)
{
     QFontMetrics fm = p->fontMetrics();
     int pixx = (width( v) - pm.width())/2;
     int txtx = (width( v) - fm.width( text()))/2;
     p->drawPixmap( pixx, 0, pm);
     QColorGroup g = v->colorGroup();
     if( s == Selected) {
	  QColor   fc;                             // fill color
	  GUIStyle style = v->style();
	  switch( style) {
	  case WindowsStyle:
	       fc = darkBlue;                      // !!!hardcoded
	       p->drawWinFocusRect( txtx - 3, pm.height(), 
				    fm.width( text())+ 5,
				    fm.lineSpacing() + 4);
	       if( v->hasFocus()) {
		    p->fillRect( txtx - 1, pm.height() + 2, 
				 fm.width( text()) + 1, 
				 fm.lineSpacing(), fc);
		    p->setPen( g.base() );
	       }
	       break;
	  default: /* Motif */
	       fc = g.text();
	       p->fillRect( txtx - 1, pm.height() + 2, 
			    fm.width( text()) + 1, 
			    fm.lineSpacing(), fc);
	       if( v->hasFocus()) {
		    p->drawRect( txtx - 3, pm.height(), 
				 fm.width( text())+ 5,
				 fm.lineSpacing() + 4);    
	       }
	       p->setBackgroundColor( g.text() );
	       p->setPen( g.base() );
	       break;
	  }
     } 
     p->drawText( txtx, pm.height()+fm.lineSpacing()-1, text());
     p->setBackgroundColor( g.base() );
     p->setPen( g.text() );
}

int
KDMViewItem::height( const KDMView* kv) const
{
     return pm.height() + kv->fontMetrics().lineSpacing() + 6;
}

int
KDMViewItem::width( const KDMView* kv) const
{
     return QMAX( pm.width(), kv->fontMetrics().width( text()) + 6);
}


/*****************************KDMView********************************/


KDMView::KDMView( QWidget* parent, const char* name, WFlags f)
     :QTableView( parent, name, f)
{
     setTableFlags( Tbl_autoScrollBars| Tbl_smoothScrolling | 
		    Tbl_clipCellPainting );
     switch ( style() ) {
     case WindowsStyle:
     case MotifStyle:
	  setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
	  setBackgroundColor( colorGroup().base() );
	  break;
     default:
	  setFrameStyle( QFrame::Panel | QFrame::Plain );
	  setLineWidth( 1 );
     }
     itemList = new KVItemList;
     setFocusPolicy( StrongFocus );
     setNumCols( 1 );
     cur_col = cur_row = -1;
     setCurrentRow( -1);
     setCurrentCol( -1);
}

KDMView::~KDMView()
{
     itemList->clear();
     delete itemList;
}

bool
KDMView::checkIndex( int row, int col)
{
     return (unsigned int)(row * numCols() + col)<itemList->count();
}

int
KDMView::currentItem() const
{
     return cur_row * numCols() + cur_col;
}

bool
KDMView::setCurrentItem( int index)
{
     int col = index % numCols();
     int row = index / numCols();
     return setCurrentItem( row, col);
}

bool
KDMView::setCurrentItem( int row, int col)
{
     if( checkIndex( row, col)) {
	  int old_col = cur_col;
	  int old_row = cur_row;
	  int delta;
	  rowYPos( row, &delta);
	  delta += cellHeight(row);
	  cur_col = col;
	  cur_row = row;
	  if( row > lastRowVisible() ) {
	       // Below bottom
	       setTopCell( row - lastRowVisible() + topCell());
	  } else if( (row == lastRowVisible()) 
		     || (  delta > viewHeight())) {
	       // At bottom or partially below
	       if( rowIsVisible(row) )
		    setYOffset( yOffset() + delta 
				- viewHeight());
	       else
		    setTopCell( row);
	  } else if( row <= topCell()) {
	       // At top or over
	       setTopCell( row);
	  }
	  updateCell( old_row, old_col);
	  updateCell( cur_row, cur_col);
	  return true;
     }
     return false;
}

void
KDMView::clear()
{
     setNumCols( /*numCols*/ 1 );
     cur_col = cur_row = -1;
     itemList->clear();
}

void
KDMView::setCurrentRow(int i)
{
     cur_row = i;
} 

void
KDMView::setCurrentCol(int i)
{
     cur_col = i;
} 


void 
KDMView::paintCell( QPainter *p, int row, int col )
{
     if( !checkIndex( row, col))
	  return;
    KDMViewItem *kvi = itemList->at( row * numCols() + col );
    if ( !kvi ) {
	 return;
    }
    QWMatrix m, oldm;
    oldm = p->worldMatrix();
    m.translate( (cellWidth() - kvi->width(this))/2.0, 
		 (cellHeight() - kvi->height(this)));
    p->setWorldMatrix( m, true);
    if( cur_row == row && cur_col == col)
	 kvi->paint( p , this, KDMViewItem::Selected);
    else
	 kvi->paint( p, this);
    p->setWorldMatrix( oldm);
}

void
KDMView::insertItem( const KDMViewItem* item, int index, bool sorted)
{
     index = 0;
     if( sorted)
	  itemList->inSort( item);
     else
	  itemList->insert( index, item );
     // constant width and height
     int w = item->width( this );
     int h = item->height( this );

     if ( w > cellWidth() )
	  setCellWidth( w );
     if ( w > cellHeight() )
	  setCellHeight( h );
     //updateTableSize();
     if ( autoUpdate() )
	  repaint();
     //setNumRows( itemList->count()/numCols());
     calcDimen();
}

void
KDMView::insertItemList( KVItemList* itlist)
{
     itemList = itlist;
     int max_width = 0;
     int max_height = 0;
     for( KDMViewItem* i = itemList->first(); i;i = itemList->next()) {
	  max_width = QMAX( max_width, i->width( this));
	  max_height = QMAX( max_height, i->height( this));
     }
     setCellWidth( max_width);
     setCellHeight( max_height);
     //updateTableSize();
     calcDimen();
}

void 
KDMView::calcDimen()
{
     setNumCols( viewWidth()/(cellWidth()?cellWidth():1));
     /*if( numCols() * cellWidth() < viewWidth() - cellWidth()*0.75) 
	  setNumCols( numCols()+1);*/
     if( numCols() == 0)
	  setNumCols( 1);
     setNumRows( itemList->count()/numCols());
     if( numRows() == 0 || itemList->count()%numCols() != 0)
	  setNumRows( numRows() + 1);
}

QSize
KDMView::sizeHint() const
{
     return QSize( (int)sqrt( itemList->count())*cellWidth()+2*frameWidth(), 
		   (int)sqrt( itemList->count())*cellHeight()+2*frameWidth());
}

int
KDMView::sizeHintHeight( int width) const
{
     int w = width - 2*frameWidth();
     if( w <= 0 ) w = 1;
     int s = (cellHeight()*cellWidth()*itemList->count())/w;
     int h = (cellHeight()>0?cellHeight():1);
     int delta = h - (s % h);
     return s+delta+2*frameWidth();
}

void 
KDMView::show()
{
     calcDimen();
     QTableView::show();
}

void 
KDMView::resizeEvent( QResizeEvent* e)
{
     QTableView::resizeEvent( e);
     calcDimen();
}

void 
KDMView::focusInEvent( QFocusEvent * )
{
     if ( currentRow() < 0 && numRows() > 0 ) {
	  setCurrentRow(0);
     }
     if( currentCol() < 0 && numCols() > 0) {
	  setCurrentCol(0);
     }
     updateCell( currentRow(), currentCol(), false); //show focus 
}

void 
KDMView::focusOutEvent( QFocusEvent * )
{
     updateCell( cur_row, cur_col); //show focus out
}

void 
KDMView::mousePressEvent( QMouseEvent *e )
{
    int row = findRow( e->pos().y() );
    int col = findCol( e->pos().x() );
    setCurrentItem( row, col);
}

void
KDMView::mouseReleaseEvent( QMouseEvent * )
{
     if ( currentItem() >= 0 ) {
	  emit selected( currentItem());
     }
}

void 
KDMView::keyPressEvent( QKeyEvent* e)
{
     int row = cur_row;
     int col = cur_col;
     if( currentItem() < 0)
	  setCurrentItem(0);
     switch( e->key()) {
     case Key_Return:
     case Key_Enter:
	  emit selected( currentItem());
	  break;
     case Key_Left:
	  setCurrentItem( currentItem()-1);
	  break;
     case Key_Right:
	  setCurrentItem( currentItem()+1);
	  break;
     case Key_Down:
	  row++;
	  setCurrentItem( row, col);
	  break;
     case Key_Up:
	  row--;
	  setCurrentItem( row, col);
	  break;
     }
}

#include "kdmview.moc"
