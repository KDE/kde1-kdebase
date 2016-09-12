
    /*

    User view for kdm. Classes KDMViewItem, KDMViewItemList and KDMView
    $Id: kdmview.h,v 1.6 1998/09/04 15:33:02 bieker Exp $

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
 

#ifndef KDMVIEW_H
#define KDMVIEW_H

# include "kdm-config.h"

#include <qlist.h>
#include <qtablevw.h>
#include <qpainter.h>
#include <qpixmap.h>

class KDMView;

class KDMViewItem {
public:
     KDMViewItem(const char *s, const QPixmap p )
	  : pm(p)
     { setText( s ); }
     virtual ~KDMViewItem() {}; 
     virtual const char    *text()   const { return txt; }
     virtual const QPixmap *pixmap() const { return &pm; }
     enum State { Normal, Selected, Hilighted};
protected:
     virtual void paint( QPainter * , KDMView* , enum State = Normal);
     virtual int height( const KDMView * ) const;
     virtual int width( const KDMView * ) const;
     virtual void setText( const char* text) { txt = text;}
     virtual void setPixmap( const QPixmap pix) { pm = pix;}
private:
     QString txt;
     QPixmap pm;
     friend class KDMView;
};

class KVItemList : public QList<KDMViewItem>
{
public:
    ~KVItemList();
    int compareItems( GCI i1, GCI i2);
};

class KDMView : public QTableView {
Q_OBJECT
public:
     KDMView( QWidget *parent = 0, const char* name = 0, WFlags f = 0);
     ~KDMView();
     void insertItem( const KDMViewItem*, int index = -1, bool sorted = false);
     void insertItemList( KVItemList* );
     void paintCell(class QPainter *, int, int);
     bool setCurrentItem( int index);
     bool setCurrentItem( int, int);
     void clear();
     int currentItem() const;
     int currentRow() const { return cur_row;}
     int currentCol() const { return cur_col;}
     void setCurrentRow( int);
     void setCurrentCol( int);
     virtual void show();
     QSize sizeHint() const;
     int sizeHintHeight( int) const;
     int minimumHeight() const { return cellHeight()+2*frameWidth();}
signals:
     void selected( int);
protected:
     void resizeEvent( QResizeEvent *);
     void mousePressEvent( QMouseEvent *);
     void mouseReleaseEvent( QMouseEvent *); 
     void focusInEvent( QFocusEvent *);
     void focusOutEvent( QFocusEvent *);
     void keyPressEvent( QKeyEvent *);
     //void updateItem( int, bool clear = TRUE);
private:
     void calcDimen();
     bool checkIndex( int, int);
     int cur_row;
     int cur_col;
     KVItemList* itemList;
};

#endif /* KDMVIEW_H */
