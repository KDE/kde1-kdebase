//-----------------------------------------------------------------------------
//
// kscience - screen saver for KDE
//
// Copyright (c)  Rene Beutler 1998
//                                                 

#include <stdlib.h>
#include <math.h>

#include <kapp.h> 

#include <qlabel.h>
#include <qpushbt.h>
#include <qmsgbox.h> 

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"

#include "kslider.h"
#include "science.h"
#include "science.moc"

#define SCIENCE_DEFAULT_SIZE       15
#define SCIENCE_DEFAULT_INTENSITY   8
#define SCIENCE_DEFAULT_SPEED      70
#define SCIENCE_MAX_SPEED         100

static KScienceSaver *saver = 0;
extern KLocale *glocale;

//-----------------------------------------------------------------------------
// standard screen saver interface functions
//
void startScreenSaver( Drawable d )
{
	if ( saver )
		return;

	saver = new KScienceSaver( d );
}               

void stopScreenSaver()
{
	if ( saver )
		delete saver;

	saver = 0;
}

int setupScreenSaver()
{
	KScienceSetup dlg;
	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate( "Science" );
}


//-----------------------------------------------------------------------------
   
KScienceSaver::KScienceSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	xRootWin = 0;

	grabRootWindow();
	
	readSettings();

       	refresh = true;
	initLens();

	switch( bpp ) {
		case 1 : applyLens = &applyLens8bpp;  break;
		case 2 : applyLens = &applyLens16bpp; break;
		case 3 : applyLens = &applyLens24bpp; break;
		case 4:  applyLens = &applyLens32bpp; break;
		default: myAssert( false, "unsupported colordepth "\
		                   "(only 8, 16 and 24 bpp supported)" );
	}

	XPutImage( qt_xdisplay(), d, gc, xRootWin, 0, 0, 0, 0, 
	           width, height);

	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
	timer.start( SCIENCE_MAX_SPEED - speed );
}

KScienceSaver::~KScienceSaver()
{
	timer.stop();
	releaseLens();
	if ( xRootWin )
	    free( xRootWin );
}

void KScienceSaver::myAssert( bool term, char *eMsg )
{
	if( !term ) {
		fprintf(stderr, "Error in Science: %s\n", eMsg);
		releaseLens();
		exit(-1);
	}
}

void KScienceSaver::initLens()
{
	int x, y, radius, diam, d, min;
	double dx, dy, r, alpha, intens;
	T32bit xo, yo;

	min = (width < height) ? width : height;
	d = (vx > vy) ? vx : vy;
	radius = (size * min)/ 100 - d;
	side = diam = ( radius+d ) << 1;

	intens = double( intensity ) / 10.0;

	offset = (T32bit **) malloc( sizeof( T32bit) * diam );
	myAssert( offset != 0, "too few memory" );

	Window rootwin;

	Display *dsp = qt_xdisplay();           
	rootwin = RootWindow( dsp, qt_xscreen() );  
	buffer = XGetImage( dsp, rootwin, 0, 0, diam, diam, AllPlanes, ZPixmap );
	myAssert( buffer != 0, "can't allocate pixmap" );
	bpp = (buffer->bits_per_pixel) >> 3;

	for(y = 0; y < diam; y++)
	{
		offset[y] = (T32bit *) malloc(sizeof(T32bit) * diam); 
		myAssert( offset[y] != 0, "too few memory" );
		for(x = 0; x < diam; x++)
		{
			dx = x - ( radius+d );
			dy = y - ( radius+d );
			r = sqrt( dx*dx + dy*dy );
			if( r < radius )
			{
				if ( dx == 0.0 )
					alpha = (dy > 0.0) ? M_PI_2 :-(M_PI_2);
 					alpha = atan2( dy, dx );
				alpha +=  intens * ( radius-r ) / ( r+7.0 );
				xo = (T32bit) ( radius + r*cos( alpha ) - x );
				yo = (T32bit) ( radius + r*sin( alpha ) - y );
				offset[y][x] = (xo + xRootWidth*yo) * bpp;
			} 		 			
			else {
				offset[y][x] =0;
 			}
		}
        }
}

void KScienceSaver::releaseLens()
{
	if( offset != 0 ) {
		for(int i=0; i<side; i++) 
    			if( offset[i] != 0 ) free( offset[i] );
	}
	if( buffer != 0 ) free( buffer );
}

void KScienceSaver::setSize( int s )
{
	timer.stop();

	releaseLens();
	size = s;
	initLens();
	refresh = true;

	timer.start( SCIENCE_MAX_SPEED - speed );
}

void KScienceSaver::setSpeed( int s )
{
	speed = s;

	timer.changeInterval( SCIENCE_MAX_SPEED - speed );
}

void KScienceSaver::setIntensity( int i )
{
	timer.stop();
	
	releaseLens();
	intensity = i;
	initLens();

	timer.start( SCIENCE_MAX_SPEED - speed);
}

void KScienceSaver::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
        config->setGroup( "Settings" );

	size      = SCIENCE_DEFAULT_SIZE;
	speed     = SCIENCE_DEFAULT_SPEED;
	intensity = SCIENCE_DEFAULT_INTENSITY;

        QString sSize, sSpeed, sIntensity;
        sSize      = config->readEntry( "Size"  );
	sSpeed     = config->readEntry( "Speed" );
	sIntensity = config->readEntry( "Intensity" );

 	if ( !sSize.isNull() )
		size = sSize.toInt();

	if ( !sSpeed.isNull() )
		speed = sSpeed.toInt();

	if ( !sIntensity.isNull() )
		intensity = sIntensity.toInt();

	vx = 2;
	vy = 1;
	x = 100;
	y = 10;
}              

void KScienceSaver::do_refresh()
{
	refresh = true;
}

void KScienceSaver::slotTimeout()
{
	if( refresh ) {
		myAssert( xRootWin != 0, "root window not grabbed" );
		XPutImage( qt_xdisplay(), d, gc, xRootWin, 0, 0, 0, 0, 
		           width, height);
		refresh = false;
	}

	x += vx;
	y += vy;

	if( x <= 0 ) { 
		vx = -vx; 
		x = 0; 
	}
	if( x >= (signed) width - side ) { 
		vx = -vx; 
		x = width - side; 
	}
	if( y <= 0 ) { 
		vy = -vy; 
		y = 0; 
	}
	if( y >= (signed) height - side ) { 
		vy = -vy; 
		y = height - side; 
	}

	(this->*applyLens)();
	XPutImage( qt_xdisplay(), d, gc, buffer, 0, 0, x, y, side, side);  
}

void KScienceSaver::grabRootWindow()
{
    Display *dsp;
    Window rootwin;

    // grab contents of root window
    if( xRootWin )
	free( xRootWin);

    dsp = qt_xdisplay();
    rootwin = RootWindow( dsp, qt_xscreen() );
    xRootWidth = QApplication::desktop()->width();
    xRootHeight = QApplication::desktop()->height();
    xRootWin = XGetImage( dsp, rootwin, 0, 0, xRootWidth,
	    xRootHeight, AllPlanes, ZPixmap);

    if( !xRootWin ) {
	fprintf(stderr, "KScreensaver: unable to grab root window\n");
	exit(-1);
    }
}

// yes I know... this could be much smaller.
// but remember: this is the innermost loop!

void KScienceSaver::applyLens8bpp()
{
T32bit *off;
	char *img1, *img2, *data;
	signed int ix, iy, datanext = buffer->bytes_per_line - bpp*side;

	img1 = xRootWin->data + ( x + y*xRootWidth )*bpp;
	data = buffer->data;
	for(iy = 0; iy < side; iy++)
	{
		off = offset[iy];
		img2 = img1;
		for(ix = side; ix > 0; ix--)
			*data++ = img2++[*off++];
		img1 += xRootWidth*bpp;
		data += datanext;
	} 
}

void KScienceSaver::applyLens16bpp()
{
	T32bit *off;
	char *img1, *img2, *data;
	int ix, iy, datanext = buffer->bytes_per_line - bpp*side;

	img1 = xRootWin->data + ( x + y*xRootWidth )*bpp;
	data = buffer->data;
	for(iy = 0; iy < side; iy++)
	{
		off = offset[iy];
		img2 = img1;
		for(ix = side; ix > 0; ix--) {
			*data++ = img2++[*off];
			*data++ = img2++[*off++];
		}
		img1 += xRootWidth*bpp;
		data += datanext;
	} 
}
	
void KScienceSaver::applyLens24bpp()
{
	T32bit *off;
	char *img1, *img2, *data;
	signed int ix, iy, datanext = buffer->bytes_per_line - bpp*side;

	img1 = xRootWin->data + ( x + y*xRootWidth )*bpp;
	data = buffer->data;
	for(iy = 0; iy < side; iy++)
	{
		off = offset[iy];
		img2 = img1;
		for(ix = side; ix > 0; ix--) {
			*data++ = img2++[*off];
			*data++ = img2++[*off];
			*data++ = img2++[*off++];
		}
		img1 += xRootWidth*bpp;
		data += datanext;
	} 
}
                      
void KScienceSaver::applyLens32bpp()
{
	T32bit *off;
	char *img1, *img2, *data;
	signed int ix, iy, datanext = buffer->bytes_per_line - bpp*side;

	img1 = xRootWin->data + ( x + y*xRootWidth )*bpp;
	data = buffer->data;
	for(iy = 0; iy < side; iy++)
	{
		off = offset[iy];
		img2 = img1;
		for(ix = side; ix > 0; ix--) {
			*data++ = img2++[*off];
			*data++ = img2++[*off];
			*data++ = img2++[*off];
			*data++ = img2++[*off++];
		}
		img1 += xRootWidth*bpp;
		data += datanext;
	} 
}


//-----------------------------------------------------------------------------

KScienceSetup::KScienceSetup(  QWidget *parent, const char *name ) : 
               QDialog( parent, name, TRUE )  
{
	saver = 0;

	readSettings();

	setCaption( glocale->translate("Setup Science") );

	QLabel *label;
	QPushButton *button;
	KSlider *sb;
	
	QVBoxLayout *lt  = new QVBoxLayout( this, 10, 10);
	QHBoxLayout *ltu = new QHBoxLayout;
	lt->addLayout( ltu );
	QVBoxLayout *ltc = new QVBoxLayout( 5 );
	ltu->addLayout( ltc );

	// size 	
	label = new QLabel( glocale->translate("Size:"), this );
	min_size( label );
	ltc->addWidget( label );
		
	sb = new KSlider( KSlider::Horizontal, this );
	sb->setMinimumSize( 90, 20 );
	sb->setRange( 10, 50 );
	sb->setSteps( 5, 25 );
	sb->setValue( size );
	connect( sb, SIGNAL( valueChanged( int ) ), SLOT( slotSize( int ) ) );
	ltc->addWidget( sb );
	ltc->addSpacing( 5 );

	// intensity
	label = new QLabel( glocale->translate("Intensity:"), this );
	min_size( label );
	ltc->addWidget( label );
		
	sb = new KSlider( KSlider::Horizontal, this );
	sb->setMinimumSize( 90, 20 );
	sb->setRange( 1, 21 );
	sb->setSteps( 2, 10 );
	sb->setValue( intensity );
	connect( sb, SIGNAL( valueChanged( int ) ), SLOT( slotIntensity( int )) );
	ltc->addWidget( sb );
	ltc->addSpacing( 5 );

	// speed
	label = new QLabel( glocale->translate("Speed:"), this );
	min_size( label );
	ltc->addWidget( label );
	
	sb = new KSlider( KSlider::Horizontal, this );
	sb->setMinimumSize( 90, 20 );
	sb->setRange( 0, SCIENCE_MAX_SPEED );
	sb->setSteps( 10, 50 );
	sb->setValue( speed );	
	connect( sb, SIGNAL( valueChanged( int ) ), SLOT( slotSpeed( int ) ) );
	ltc->addWidget( sb );


	ltc->addStretch( 1 );
	
	// preview
	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();	// otherwise saver does not get correct size
	ltu->addWidget( preview );

	// buttons
	KButtonBox *bbox = new KButtonBox( this );
	button = bbox->addButton( glocale->translate("About") );
	connect( button, SIGNAL( clicked() ), SLOT( slotAbout() ) );
	bbox->addStretch( 1 );

	button = bbox->addButton( glocale->translate("Ok") );
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = bbox->addButton( glocale->translate("Cancel") );
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
	bbox->layout();
	lt->addWidget( bbox );
	
	lt->freeze();

	// let the preview window display before creating the saver
	kapp->processEvents();
	
	saver = new KScienceSaver( preview->winId() );
}

// read settings from config file
void KScienceSetup::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
        config->setGroup( "Settings" );

	size      = SCIENCE_DEFAULT_SIZE;
	speed     = SCIENCE_DEFAULT_SPEED;
	intensity = SCIENCE_DEFAULT_INTENSITY;

        QString sSize, sSpeed, sIntensity;
        sSize      = config->readEntry( "Size"  );
	sSpeed     = config->readEntry( "Speed" );
	sIntensity = config->readEntry( "Intensity" );

 	if ( !sSize.isNull() )
		size = sSize.toInt();

	if ( !sSpeed.isNull() )
		speed = sSpeed.toInt();

	if( !sIntensity.isNull() )
		intensity = sIntensity.toInt();
}  
   
void KScienceSetup::slotSize( int s )
{
	size = s;

	if( saver )
		saver->setSize( s );
}

void KScienceSetup::slotSpeed( int s )
{
	speed = s;

	if( saver )
		saver->setSpeed( speed );
}
                       
void KScienceSetup::slotIntensity( int i )
{
	intensity = i;

	if( saver )
		saver->setIntensity( intensity );
}

// Ok pressed - save settings and exit
void KScienceSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString sSize, sSpeed, sIntensity;

	sSize.setNum( size );
	config->writeEntry( "Size", sSize );

	sSpeed.setNum( speed );
	config->writeEntry( "Speed", sSpeed );

	sIntensity.setNum( intensity );
	config->writeEntry( "Intensity", sIntensity );

	config->sync();

	accept();
}

void KScienceSetup::slotAbout()
{
	QMessageBox::message( glocale->translate("About Science"), 
	                      glocale->translate("Science Version 0.2beta\n\n"\
	                      "written by Rene Beutler 1998\n"\
                              "rbeutler@g26.ethz.ch"), 
		glocale->translate("Ok") );

	if( saver )
		saver->do_refresh();
}

void KScienceSetup::paintEvent( QPaintEvent * )
{
	if( saver )
		saver->do_refresh();
}

