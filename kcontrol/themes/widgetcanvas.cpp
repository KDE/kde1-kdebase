//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//
// Converted to a kcc module by Matthias Hoelzer 1997
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <qgrpbox.h>
#include <qbttngrp.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbt.h>
#include <qfiledlg.h>
#include <qslider.h>
#include <qradiobt.h>
#include <qmsgbox.h>
#include <qscrbar.h>
#include <qdrawutl.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <kapp.h>
#include <kmsgbox.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <kcolordlg.h>

#include "kwmcom.h"
#include "widgetcanvas.h"
#include "widgetcanvas.moc"

#define SUPPLIED_SCHEMES 5
#define SCROLLBAR_SIZE 16

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


int dropError(Display *, XErrorEvent *)
{
  return 0;
}

WidgetCanvas::WidgetCanvas( QWidget *parent)
  : QWidget( parent  )
{
}

void WidgetCanvas::paintEvent(QPaintEvent *)
{
  bitBlt( this, 0,0, &smplw ); 
}

void WidgetCanvas::mousePressEvent( QMouseEvent *me )
{
  for ( int i = 0; i < MAX_HOTSPOTS; i++ )
    if ( hotspots[i].rect.contains( me->pos() ) )
    {
      emit widgetSelected( hotspots[i].number );
      return;
    }
}

void WidgetCanvas::paletteChange(const QPalette &)
{
  drawSampleWidgets();
}


void WidgetCanvas::drawSampleWidgets()
{
  int textLen;
  int highlightVal, lowlightVal;
	
  // Calculate the highlight and lowloght from contrast value and create
  // color group from color scheme.
	
  highlightVal=100+(2*contrast+4)*16/10;
  lowlightVal=100+(2*contrast+4)*10;
	
  QColorGroup cg( textColor, backgroundColor, 
		  backgroundColor.light(highlightVal),
		  backgroundColor.dark(lowlightVal), 
		  backgroundColor.dark(120),
		  textColor, windowColor );
	
  QColor col = QWidget::backgroundColor(); 

  // We will need this brush.
	
  QBrush brush(SolidPattern);
  brush.setColor( backgroundColor );

  // Create a scrollbar and redirect drawing into a temp. pixmap to save a
  // lot of fiddly drawing later.

  QScrollBar *vertScrollBar = new QScrollBar( QScrollBar::Vertical, this );
  vertScrollBar->setGeometry( 400, 400, SCROLLBAR_SIZE, height());
  vertScrollBar->setRange( 0,  0 );
  vertScrollBar->setPalette( QPalette(cg,cg,cg));
  vertScrollBar->show();
    
  QPixmap pm( vertScrollBar->width(), vertScrollBar->height() );
  pm.fill( backgroundColor );
  QPainter::redirect( vertScrollBar, &pm );
  vertScrollBar->repaint();
  QPainter::redirect( vertScrollBar, 0 );
  vertScrollBar->hide();
	
  // Initialize the pixmap which we draw sample widgets into.

  smplw.resize(width(), height());
  smplw.fill( parentWidget()->backgroundColor() );
	
  // Actually start painting in 
	
  QPainter paint( &smplw );
	
  // Inactive window
	                          
  qDrawWinPanel ( &paint, 0, 0, width()-18, height(), cg, FALSE,
		  &brush);
    
  paint.setBrush( inactiveTitleColor );
  paint.setPen( inactiveTitleColor );
  paint.drawRect( 5, 5, width()-30, 20 );
    
  paint.setFont( QFont("Helvetica", 12, QFont::Bold) );
  paint.setPen( inactiveTextColor );
  paint.drawText( (width()-25)/2-40, 20, klocale->translate("Inactive window") );
  textLen = paint.fontMetrics().width(  klocale->translate("Inactive window") );
    
  hotspots[0] = HotSpot( QRect( (width()-25)/2-40, 8, textLen, 14 ), 1 ); // inactive text
  hotspots[1] = HotSpot( QRect( 5, 5, width()-30, 20 ), 0 ); // inactive title

  // Active window
    
  qDrawWinPanel ( &paint, 20, 25+5, width()-20, height(), cg, FALSE,
		  &brush);
    
  paint.setBrush( activeTitleColor );paint.setPen( activeTitleColor );
  paint.drawRect( 25, 30+5, width()-32, 20 ); 
    
  paint.setFont( QFont("Helvetica", 12, QFont::Bold) );
  paint.setPen( activeTextColor );
  paint.drawText( 30+5+width()-32-(width()-32)/2-50, 45+5,  klocale->translate("Active window") );
  textLen = paint.fontMetrics().width(  klocale->translate("Active window" ));
    
  hotspots[2] = HotSpot( QRect( 30+5+width()-32-(width()-32)/2-50, 38, textLen, 14 ), 3 ); // Active text
  hotspots[3] = HotSpot( QRect( 25, 35, width()-32, 20 ), 2 ); // Active title

  // Menu bar
  
  qDrawShadePanel ( &paint, 25, 55, width()-32, 28, cg, FALSE, 2, &brush);
    
  paint.setFont( QFont("Helvetica", 12, QFont::Normal) );
  paint.setPen(textColor );
  textLen = paint.fontMetrics().width( klocale->translate("File") );
  qDrawShadePanel ( &paint, 30, 52+5+2, textLen + 10, 21, cg, FALSE, 2, &brush);
  paint.drawText( 35, 69+5, klocale->translate("File") );

  hotspots[4] = HotSpot( QRect( 35, 62, textLen, 14 ), 5 ); 
  hotspots[5] = HotSpot( QRect( 27, 52+5, 33, 21 ), 4 ); 
    
  paint.setFont( QFont("Helvetica", 12, QFont::Normal) );
  paint.setPen( textColor );
  paint.drawText( 35 + textLen + 20, 69+5, klocale->translate("Edit") );
  textLen = paint.fontMetrics().width( klocale->translate("Edit") );

  hotspots[6] = HotSpot( QRect( 65, 62, textLen, 14 ), 5 ); // text

  // Frame and window contents
    
  brush.setColor( windowColor );
  qDrawShadePanel ( &paint, 25, 80+5-4, width()-7-25-2, 
		    height(), cg, TRUE, 2, &brush);
    
  paint.setFont( QFont("Helvetica", 14, QFont::Normal) );
  paint.setPen( windowTextColor );
  paint.drawText( 200, 127-10, klocale->translate( "Window text") );
  textLen = paint.fontMetrics().width( klocale->translate("Window text") );

  hotspots[7] = HotSpot( QRect( 200, 113-10, textLen, 14 ), 9 ); // window text
  hotspots[8] = HotSpot( QRect( 116, 87, width()-138, height()-82-5 ), 8 ); // window bg
    
  // Scrollbar
    
  paint.drawPixmap(width()-35+27-16-2,80+5-2,pm);
    
  // Menu
  
  brush.setColor( backgroundColor );
  qDrawShadePanel ( &paint, 30, 80, 84, height(), cg, FALSE, 2, &brush);

  paint.setFont( QFont("Helvetica", 12, QFont::Normal) );
  paint.setPen( lightGray.dark() );
  paint.drawText( 38, 97, klocale->translate("Disabled") );
    
  qDrawShadePanel ( &paint, 32, 101, 80, 25, cg, FALSE, 2,
		    &brush);
   
  paint.setFont( QFont("Helvetica", 12, QFont::Normal) );
  paint.setPen( textColor );
  paint.drawText( 38, 119, klocale->translate("Selected") );
  textLen = paint.fontMetrics().width( klocale->translate("Selected") );

  hotspots[10] = HotSpot( QRect( 38, 105, textLen, 14 ), 5 ); 
  hotspots[11] = HotSpot( QRect( 28, 101, 78, 21 ), 4 ); 
    
  // Valance

  qDrawShadePanel ( &paint, 0, height()-17,width(), 17, cg, FALSE, 2,
		    &brush);
  paint.setPen( col.light() );paint.setBrush( col.light() );
  paint.drawRect( 1, height()-17, width()-3, 15);
	
  // Stop the painting
	
  hotspots[12] = HotSpot( QRect( 0, 0, width(), height() ), 4 );
	
  repaint( FALSE );          
}
