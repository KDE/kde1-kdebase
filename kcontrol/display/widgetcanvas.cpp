//
// A special widget which draws a sample of KDE widgets
// It is used to preview color schemes
//
// Copyright (c)  Mark Donohoe 1998
//

#include "widgetcanvas.h"
#include "widgetcanvas.moc"

WidgetCanvas::WidgetCanvas( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
}

void WidgetCanvas::paintEvent(QPaintEvent *)
{
	bitBlt( this, 0, 0, &smplw ); 
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
	
	QColorGroup cg( txt, back, 
    				back.light(highlightVal),
    				back.dark(lowlightVal), 
    				back.dark(120),
    				txt, window );
	
	QColor col = QWidget::backgroundColor(); 

	// We will need this brush.
	
	QBrush brush(SolidPattern);
	brush.setColor( back );

	// Create a scrollbar and redirect drawing into a temp. pixmap to save a
	// lot of fiddly drawing later.

	QScrollBar *vertScrollBar = new QScrollBar( QScrollBar::Vertical, this );
	vertScrollBar->setStyle( MotifStyle );
    vertScrollBar->setGeometry( 400, 400, SCROLLBAR_SIZE, height());
    vertScrollBar->setRange( 0,  0 );
    vertScrollBar->setPalette( QPalette(cg,cg,cg));
    vertScrollBar->show();
    
    QPixmap pm( vertScrollBar->width(), vertScrollBar->height() );
    pm.fill( back );
    QPainter::redirect( vertScrollBar, &pm );
    vertScrollBar->repaint();
    QPainter::redirect( vertScrollBar, 0 );
    vertScrollBar->hide();
	
	// Initialize the pixmap which we draw sample widgets into.

	smplw.resize(width(), height());
	//smplw.fill( parentWidget()->back() );
	smplw.fill( parentWidget()->colorGroup().mid() );
	
	// Actually start painting in 
	
	QPainter paint( &smplw );
	
	// Inactive window
	                          
    qDrawWinPanel ( &paint, 15, 5, width()-48, height(), cg, FALSE,
	&brush);
    
    paint.setBrush( iaTitle );
    paint.setPen( iaTitle );
    //paint.drawRect( 20, 10, width()-60, 20 );
	
	KPixmap pmTitle;
	pmTitle.resize( width()-60, 20 );
	pmTitle.gradientFill( iaTitle, iaBlend, FALSE );
	paint.drawPixmap( 20, 10, pmTitle ); 
   
    QFont fnt("Helvetica", 12, QFont::Bold);
    KApplication::getKApplication()->getCharsets()->setQFont(fnt);
    paint.setFont( fnt );
    paint.setPen( iaTxt );
    paint.drawText( 25, 25, i18n("Inactive window") );
	textLen = paint.fontMetrics().width(  i18n("Inactive window") );
    
	int spot = 0;
	hotspots[ spot++ ] =
	HotSpot( QRect( 25, 25-14, textLen, 14 ), 1 ); // inactive text
	
	hotspots[ spot++ ] =
	HotSpot( QRect( 20, 10, (width()-60)/2, 20 ), 0 ); // inactive title
	
	hotspots[ spot++ ] =
	HotSpot( QRect( 20+(width()-60)/2, 10,
			(width()-60)/2, 20 ), 2 ); // inactive blend

    // Active window
    
    qDrawWinPanel ( &paint, 20, 25+5, width()-40, height(), cg, FALSE,
    &brush);
    
    paint.setBrush( aTitle );paint.setPen( aTitle );
    paint.drawRect( 25, 30+5, width()-52, 20 );
	
	pmTitle.resize( width()-52, 20 );
	pmTitle.gradientFill( aTitle, aBlend, FALSE );
	paint.drawPixmap( 25, 35, pmTitle ); 
    
    paint.setFont( fnt );
    paint.setPen( aTxt );
    paint.drawText( 35, 50,  i18n("Active window") );
	textLen = paint.fontMetrics().width(  i18n("Active window" ));
    
	hotspots[ spot++ ] =
	HotSpot( QRect( 35, 50-14, textLen, 14 ), 4 ); // Active text
	hotspots[ spot ++] =
	HotSpot( QRect( 25, 35, (width()-52)/2, 20 ), 3 ); // Active title
	hotspots[ spot ++] =
	HotSpot( QRect( 25+(width()-52)/2, 35,
			(width()-52)/2, 20 ), 5 ); // Active title

    // Menu bar
  
    qDrawShadePanel ( &paint, 25, 55, width()-52, 28, cg, FALSE, 2, &brush);
   
    fnt.setBold(FALSE);
    paint.setFont( fnt );
    paint.setPen(txt );
    textLen = paint.fontMetrics().width( i18n("File") );
    qDrawShadePanel ( &paint, 30, 59, textLen + 10, 21, cg, FALSE, 2, &brush);
    paint.drawText( 35, 74, i18n("File") );

    hotspots[ spot++ ] =
	HotSpot( QRect( 35, 62, textLen, 14 ), 7 ); 
    hotspots[ spot++ ] =
	HotSpot( QRect( 27, 57, 33, 21 ), 6 ); 
    
    paint.setFont( fnt );
    paint.setPen( txt );
    paint.drawText( 35 + textLen + 20, 69+5, i18n("Edit") );
    textLen = paint.fontMetrics().width( i18n("Edit") );

    hotspots[ spot++ ] = HotSpot( QRect( 65, 62, textLen, 14 ), 7 ); // text

    // Frame and window contents
    
    brush.setColor( window );
    qDrawShadePanel ( &paint, 25, 80+5-4, width()-7-45-2, 
		      height(), cg, TRUE, 2, &brush);
    
    fnt.setPointSize(12);
    paint.setFont( fnt );
    paint.setPen( windowTxt );
    paint.drawText( 200, 127-20, i18n( "Window text") );
    textLen = paint.fontMetrics().width( i18n("Window text") );

    hotspots[ spot++ ] =
	HotSpot( QRect( 200, 113-20, textLen, 14 ), 11 ); // window text
   
    
	paint.setBrush( select );paint.setPen( select );
    paint.drawRect ( 120, 115, width()-175, 
		      height() );
    
    fnt.setPointSize(12);
    paint.setFont( fnt );
    paint.setPen( selectTxt );
    paint.drawText( 200, 135, i18n( "Selected text") );
    textLen = paint.fontMetrics().width( i18n("Selected text") );
	
	hotspots[ spot++ ] =
	HotSpot( QRect( 200, 121, textLen, 14 ), 9 ); // select text
    hotspots[ spot++ ] =
	HotSpot( QRect( 120, 115, width()-175, height() ), 8 ); // select bg
	
	 hotspots[ spot++ ] =
	HotSpot( QRect( 116, 87, width()-138, height()-82-5 ), 10 ); // window bg
	
    // Scrollbar
    
	paint.drawPixmap(width()-55+27-16-2,80+5-2,pm);
    
    // Menu
  
    brush.setColor( back );
    qDrawShadePanel ( &paint, 30, 80, 84, height(), cg, FALSE, 2, &brush);

    fnt.setPointSize(12);
    paint.setFont( fnt );
    paint.setPen( txt );
    paint.drawText( 38, 97, i18n("New") );
	
	hotspots[ spot++ ] =
	HotSpot( QRect( 38, 83, textLen, 14 ), 7 ); 
    //hotspots[ spot++ ] =
	//HotSpot( QRect( 28, 97, 78, 21 ), 6 ); 
    
    //qDrawShadePanel ( &paint, 32, 101, 80, 25, cg, FALSE, 2,
    //&brush);
   
    paint.setFont( fnt );
    paint.drawText( 38, 119, i18n("Open") );
    textLen = paint.fontMetrics().width( i18n("Open") );
	
	hotspots[ spot++ ] =
	HotSpot( QRect( 38, 105, textLen, 14 ), 7 ); 
    hotspots[ spot++ ] =
	HotSpot( QRect( 28, 101, 78, 21 ), 6 ); 
	
	paint.setFont( fnt );
    paint.setPen( lightGray.dark() );
    paint.drawText( 38, 141, i18n("Save") );
    textLen = paint.fontMetrics().width( i18n("Save") );

    
    
    // Valance

	qDrawWinPanel ( &paint, 0, 0, width(), height(),
	parentWidget()->colorGroup(), TRUE, 0);

    //qDrawShadePanel ( &paint, 0, height()-17,width(), 17, cg, FALSE, 2,
    //&brush);
    //paint.setPen( col.light() );paint.setBrush( col.light() );
    //paint.drawRect( 1, height()-17, width()-3, 15);
	
	// Stop the painting
	
	hotspots[ spot++ ]
	= HotSpot( QRect( 0, 0, width(), height() ), 6 );
	
	repaint( FALSE );          
}
