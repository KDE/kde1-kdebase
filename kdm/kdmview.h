//                              -*- Mode: C++ -*- 
// Title            : kdmview.h
// 
// Description      : User view for kdm. Classes KDMViewItem, 
//                    KDMViewItemList and KDMView
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:46:59 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Tue Sep  9 18:32:26 1997
// Update Count     : 8
// Status           : Unknown, Use with caution!
// 

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
