//-----------------------------------------------------------------------------
//
// kscience - screen saver for KDE
//
// Copyright (c)  Rene Beutler 1998
//

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>

#include <kapp.h> 
#include <kcharsets.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qmsgbox.h> 
#include <qlistbox.h>
#include <qchkbox.h>
#include <qbttngrp.h>
#include <qradiobt.h>

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"

#include "kslider.h"
#include "science.h"
#include "science.moc"

#include <X11/Xutil.h>

#define SCI_DEFAULT_MODE          0
#define SCI_DEFAULT_MOVEX         6
#define SCI_DEFAULT_MOVEY         8
#define SCI_DEFAULT_SIZE         15
#define SCI_DEFAULT_INTENSITY     4
#define SCI_DEFAULT_SPEED        70
#define SCI_DEFAULT_INVERSE   false
#define SCI_DEFAULT_GRAVITY   false
#define SCI_DEFAULT_HIDE      false
#define SCI_MAX_SPEED           100
#define SCI_MAX_MOVE             20

#undef i18n
#define i18n(x) glocale->translate(x)

static KScienceSaver *saver = 0;
extern KLocale *glocale;

#define PIXDIR   KApplication::getKApplication()->kde_datadir() + "/kscreensaver/"

struct {
	char name[32];
	bool inverseEnable;
	} modeInfo[MAX_MODES];

enum { MODE_WHIRL=0, MODE_CURVATURE, MODE_SPHERE, MODE_WAVE, MODE_EXPONENTIAL, MODE_CONTRACTION };

void initModeInfo()
{
	strncpy( modeInfo[MODE_WHIRL].name, i18n( "Whirl" ), 32 );
	modeInfo[MODE_WHIRL].inverseEnable = true;

	strncpy( modeInfo[MODE_SPHERE].name, i18n( "Sphere" ), 32 );
	modeInfo[MODE_SPHERE].inverseEnable = true;

	strncpy( modeInfo[MODE_EXPONENTIAL].name, i18n( "Exponential" ), 32 );
	modeInfo[MODE_EXPONENTIAL].inverseEnable = false;

	strncpy( modeInfo[MODE_CONTRACTION].name, i18n( "Contraction" ), 32 );
	modeInfo[MODE_CONTRACTION].inverseEnable = false;

	strncpy( modeInfo[MODE_WAVE].name, i18n( "Wave" ), 32 );
	modeInfo[MODE_WAVE].inverseEnable = false;

	strncpy( modeInfo[MODE_CURVATURE].name, i18n( "Curvature" ), 32 );
	modeInfo[MODE_CURVATURE].inverseEnable = true;
}

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
	return i18n( "Science" );
}

//-----------------------------------------------------------------------------
// Prepare Dialog
// 

KPrepareDlg::KPrepareDlg( QWidget *parent ) : QWidget( parent )
{
	save = 0;
	QPixmap pmap( PIXDIR + "kscience-small.gif" );
	w = ( pmap.isNull() ) ? 300 : 400;
	h = 99;

	x = ( QApplication::desktop()->width()  - w ) >> 1;
	y = ( QApplication::desktop()->height() - h ) >> 1;
     
	// save screen
	Display *dsp = qt_xdisplay();
        Window rootwin = RootWindow( dsp, qt_xscreen() );
        save = XGetImage( dsp, rootwin, x, y, w, h, AllPlanes, ZPixmap);
	saver->myAssert( save != 0, "unable to grab screen" );
        bpl = save->bytes_per_line;
	
        frame = new QFrame( parent );
        frame->setFrameStyle( QFrame::Panel | QFrame::Raised );
        frame->setLineWidth( 2 );

	QFont font( "helvetica", 18 );
	KApplication::getKApplication()->getCharsets()->setQFont(font);

	label = new QLabel( 0, frame );
	label->setAlignment( AlignCenter );
	label->setFont( font );
	label->show();

	if( !pmap.isNull() )
	{
		QWidget *image = new QWidget( frame );
		image->setGeometry( 7, 7, 110, 85 );
		image->setBackgroundPixmap( pmap );
		label->setGeometry( 120, 25, 240, 30 );
	}
	else
		label->setGeometry( 30, 35, 240, 30 );

        frame->setGeometry( x, y, w, h );	
	frame->raise();
}          

KPrepareDlg::~KPrepareDlg()
{
	frame->close(true);
	if( save )
		XDestroyImage( save );
}                                                           

void KPrepareDlg::show()
{
	frame->raise();
	frame->show();
	kapp->processEvents();
}

void KPrepareDlg::hide()
{
	frame->hide();
}

void KPrepareDlg::setText( const char *msg )
{
	if( msg != 0 )
	{
		label->setText( msg );	
		kapp->processEvents();
	}
}


//-----------------------------------------------------------------------------
// KPreviewWidget
//

KPreviewWidget::KPreviewWidget( QWidget *parent ) :
                QWidget ( parent ) { }

void KPreviewWidget::paintEvent( QPaintEvent *event )
{
	if( saver != 0 )
		saver->do_refresh( event->rect() );
}

void KPreviewWidget::notifySaver( KScienceSaver *s )
{
	saver = s;
}

//-----------------------------------------------------------------------------
// Screen Saver
//

KScienceSaver::KScienceSaver( Drawable drawable, bool s, bool gP ) : kScreenSaver( drawable )
{
	xRootWin = buffer = 0;
	dlg = 0;

	moveOn = true;
	grabPixmap = gP;
	setup = s;
	showDialog = !setup && 	(RootWindow( qt_xdisplay(), qt_xscreen() ) != drawable) &&
                     width >= 320 && height >= 200;

	if( showDialog )
	{
		dlg = new KPrepareDlg( QWidget::find( drawable ) );
		myAssert( dlg != 0, "unable to create prepare dialog" );
	}

	vx = vy = 0.0;
	readSettings();

	if( !grabPixmap )
	{
		grabRootWindow();
		initialize();
		do_refresh( QRect ( 0, 0, width, height ) );
	}

	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
	timer.start( SCI_MAX_SPEED - speed[mode] );
}

KScienceSaver::~KScienceSaver()
{
	timer.stop();
	releaseLens();
	if ( xRootWin )
		XDestroyImage( xRootWin );
	if( dlg )
		delete dlg;
}

void KScienceSaver::myAssert( bool term, char *eMsg )
{
	if( !term ) {
		fprintf(stderr, "Error in KScreensaver - mode Science: %s\n", eMsg);
		releaseLens();
		exit(-1);
	}
}

void KScienceSaver::initialize()
{
	initLens();
	signed int ws = (signed int) (width -  diam);
	signed int hs = (signed int) (height - diam);

	srandom( (int) time( (time_t *) NULL ) );
	x = (double) ( (ws > 0) ? (random() % ws ) : 0 );
	y = (double) ( (hs > 0) ? (random() % hs ) : 0 );

	xcoord = (int) x;
	ycoord = (int) y;		

	switch( bpp ) {
		case 1 : applyLens = &applyLens8bpp;  break;
		case 2 : applyLens = &applyLens16bpp; break;
		case 3 : applyLens = &applyLens24bpp; break;
		case 4 : applyLens = &applyLens32bpp; break;
		default: myAssert( false, "unsupported colordepth "\
		                   "(only 8, 16, 24, 32 bpp supported)" );
	}
}

void KScienceSaver::initWhirlLens()
{
	double dx, dy, r, phi, intens;
	T32bit *off;
	T32bit xo, yo;

	intens = double( intensity[mode] + 1) / 5.0;
	if( inverse[mode] ) 
		intens = -intens;

	for(int y = side-1; y >= 0; y--)
	{
		dy = y - origin;
		off = offset[y] = (T32bit *) malloc(sizeof(T32bit) * side);
		myAssert( off != 0, "too few memory" );
		for(int x = side-1; x >= 0; x--)
		{
		    dx = x - origin;
		    r = sqrt( dx*dx + dy*dy );

		    if( r < radius )
		    {
			    if ( dx == 0.0 )
				    phi = (dy > 0.0) ? M_PI_2 :-(M_PI_2);
			    else
				    phi = atan2( dy, dx );
			    phi +=  intens * ( radius - r ) / ( r+7.0 );
			    xo = (T32bit) ( origin + r*cos( phi ) - x );
			    yo = (T32bit) ( origin + r*sin( phi ) - y );
			    off[x] = xo*bpp + yo*imgnext;				
		    } 		 			
		    else
			if( hideBG[mode] )
				off[x] = (border-y)*imgnext + (border-x)*bpp;
			else
				off[x] = 0;
		}
        }
}

void KScienceSaver::initSphereLens()
{
	double dx, dy, r, xr, yr, phi, intens;
	T32bit *off;
	T32bit xo, yo;

	intens = 1.0 - double( intensity[mode] ) / 20.0;

	if( inverse[mode] )
		intens = -intens;

	for(int y = side-1; y >= 0; y--)
	{
		dy = y - origin;
		off = offset[y] = (T32bit *) malloc(sizeof(T32bit) * side);
		myAssert( off != 0, "too few memory" );
		for(int x = side-1; x >= 0; x--)
		{
		    dx = x - origin;
		    r = sqrt( dx*dx + dy*dy );

		if( r < radius )
		{
			xr = (double) radius*cos(asin(dy/radius));
			yr = (double) radius*cos(asin(dx/radius));
			phi = (xr != 0.0) ? asin(dx/xr) : 0.0;
			xo = (T32bit) (origin + intens*2.0*phi*xr / M_PI - x);
			phi = (yr != 0.0) ? asin(dy/yr) : 0.0;
			yo = (T32bit) (origin + intens*2.0*phi*yr / M_PI - y);
			off[x] = xo*bpp + yo*imgnext;
		} 		 			
		else 
			if( hideBG[mode] )
				off[x] = (border-y)*imgnext + (border-x)*bpp;
			else
				off[x] = 0;
		}
        }
}

void KScienceSaver::initExponentialLens()
{
	double dx, dy, r, rnew, f, intens;
	T32bit *off;
	T32bit xo, yo;

	if( mode == MODE_EXPONENTIAL )
		intens = - (0.1 + 0.8 * double( intensity[mode] + 2) / 10.0);
	else
		intens = 0.9 - 0.8 * double( intensity[mode] ) / 10.0;

	for(int y = side-1; y >= 0; y--)
	{
		dy = y - origin;
		off = offset[y] = (T32bit *) malloc(sizeof(T32bit) * side);
		myAssert( off != 0, "too few memory" );
		for(int x = side-1; x >= 0; x--)
		{
		    dx = x - origin;
		    r = sqrt( dx*dx + dy*dy );

		    if( r < radius )
		    {
			    if( r == 0.0 )
				    f = 0.0;
			    else
			    {
				    rnew = radius*(pow(r, intens) /  pow(radius, intens));
				    f = double ((int)rnew % radius) / r;
			    }
			    xo = (T32bit) ( origin + f*dx - x );
			    yo = (T32bit) ( origin + f*dy - y );
			    off[x] = xo*bpp + yo*imgnext;			
		    }
		    else
			if( hideBG[mode] )
				off[x] = (border-y)*imgnext + (border-x)*bpp;
			else
				off[x] = 0;
		}
        }
}

void KScienceSaver::initCurvatureLens()
{ 
	double dx, dy, r, f, intens; 
	T32bit *off; 
	T32bit xo, yo; 
 
	intens = (double) radius*intensity[mode] / 20.0; 
	if( inverse[mode] ) intens = -intens;
  
	for(int y = side-1; y >= 0; y--)
	{
		dy = y - origin;
		off = offset[y] = (T32bit *) malloc(sizeof(T32bit) * side);
		myAssert( off != 0, "too few memory" );
		for(int x = side-1; x >= 0; x--)
		{
		    dx = x - origin;
		    r = sqrt( dx*dx + dy*dy );

		    if( r < radius ) 
		    { 
			    if( r == 0.0 )
				    f = 0.0;
			    else
				    f = (r - intens * sin(M_PI * r/(double)radius)) / r;
			    xo = (T32bit) ( origin + f*dx - x );
			    yo = (T32bit) ( origin + f*dy - y );
			    off[x] = xo*bpp + yo*imgnext;			
		    } 
		    else
			if( hideBG[mode] )
				off[x] = (border-y)*imgnext + (border-x)*bpp;
			else
				off[x] = 0;
		} 
	} 
} 

void KScienceSaver::initWaveLens()
{ 
	double dx, dy, r, rnew, f, intens, k; 
	T32bit *off; 
	T32bit xo, yo; 
 
	intens = (double) intensity[mode] + 1.0; 
	k = (intensity[mode] % 2) ? -12.0 : 12.0;
 
	for(int y = side-1; y >= 0; y--)
	{
		dy = y - origin;
		off = offset[y] = (T32bit *) malloc(sizeof(T32bit) * side);
		myAssert( off != 0, "too few memory" );
		for(int x = side-1; x >= 0; x--)
		{
		    dx = x - origin;
		    r = sqrt( dx*dx + dy*dy );

		    if( r < radius ) 
		    { 
			    if( r == 0.0 )
				    f = 0.0;
			    else
			    {
				    rnew = r - k * sin( M_PI * intens * r/(double)radius);
				    f = double ((int)rnew % radius) / r;
			    }
			    xo = (T32bit) ( origin + f*dx - x );
			    yo = (T32bit) ( origin + f*dy - y );
			    off[x] = xo*bpp + yo*imgnext;			
		    } 
		    else
			if( hideBG[mode] )
				off[x] = (border-y)*imgnext + (border-x)*bpp;
			else
				off[x] = 0;
		} 
	} 
} 

void KScienceSaver::initLens()
{
	if( showDialog && dlg ) {
		dlg->setText( i18n( "rendering lens..." ) );
		dlg->show();
		kapp->processEvents();		
	}

	int min = (width < height) ? width : height;
	border = 1 + SCI_MAX_MOVE;

	radius = (size[mode] * min) / 100;
	if( radius<<1 == min ) radius--;
	diam = radius << 1;
	myAssert( diam < min, "assertion violated: diam < min" );
	origin = radius + border;
	side  = origin << 1;

	buffer = XSubImage( xRootWin, 0, 0, side, side );
        myAssert( buffer != 0, "can't allocate pixmap" );                             

	offset = (T32bit **) malloc( sizeof(T32bit *) * side );
	myAssert( offset != 0, "too few memory" );

	switch( mode ) {
		case MODE_WHIRL: 	initWhirlLens();  break;
		case MODE_SPHERE: 	initSphereLens(); break;
		case MODE_EXPONENTIAL:
		case MODE_CONTRACTION: 	initExponentialLens(); break;
		case MODE_CURVATURE:    initCurvatureLens(); break;
		case MODE_WAVE: 	initWaveLens(); break;
		default: myAssert( false, "internal error (wrong mode in initLens() )" );
	}

	if( showDialog && dlg )
		dlg->hide();
}

void KScienceSaver::releaseLens()
{
	if( offset != 0 ) {
		for(int i=0; i<side; i++) 
    			if( offset[i] != 0 ) free( offset[i] );
    		free( offset );
		offset = 0;
	}
	if( buffer != 0 ) {
		XDestroyImage( buffer );
		buffer = 0;
	}
}

void KScienceSaver::setMode( int m )
{
	timer.stop();

	releaseLens();
	int old = mode;
	mode = m;
	vx = copysign( moveX[mode], vx );
	vy = copysign( moveY[mode], vy );
	int dm = diam;
	initLens();
	if( hideBG[old] ^ hideBG[m] )
		do_refresh( QRect( 0, 0, width, height ) );
	else
		if( diam < dm )
		{
			do_refresh( QRect( (int) x+diam, (int) y,      dm-diam, diam    ) );
			do_refresh( QRect( (int) x,      (int) y+diam, dm,      dm-diam ) );
		}
	
	timer.start( SCI_MAX_SPEED - speed[mode] );
}

void KScienceSaver::setMoveX( int s )
{
	timer.stop();
	
	moveX[mode] = s;
	vx = copysign( moveX[mode], vx );
	
	timer.start( SCI_MAX_SPEED - speed[mode] );
}

void KScienceSaver::setMoveY( int s )
{
	timer.stop();
	
	moveY[mode] = s;
	vy = copysign( moveY[mode], vy );
	
	timer.start( SCI_MAX_SPEED - speed[mode] );
}

void KScienceSaver::setMove( bool s )
{
	moveOn = s;
}

void KScienceSaver::setSize( int s )
{
	timer.stop();

	releaseLens();
	int dm = diam;
	size[mode] = s;
	initLens();
	if( diam < dm )
	{
		do_refresh( QRect( (int) x+diam, (int) y,      dm-diam, diam    ) );
		do_refresh( QRect( (int) x,      (int) y+diam, dm,      dm-diam ) );
	}

	timer.start( SCI_MAX_SPEED - speed[mode] );
}

void KScienceSaver::setSpeed( int s )
{
	speed[mode] = s;

	timer.changeInterval( SCI_MAX_SPEED - speed[mode] );
}

void KScienceSaver::setIntensity( int i )
{
	timer.stop();
	
	releaseLens();
	intensity[mode] = i;
	initLens();

	timer.start( SCI_MAX_SPEED - speed[mode]);
}

void KScienceSaver::setInverse( bool b )
{
	timer.stop();

	releaseLens();
	inverse[mode] = b;
	initLens();

	timer.start( SCI_MAX_SPEED - speed[mode]);
}

void KScienceSaver::setGravity( bool b )
{
	timer.stop();

	releaseLens();
	gravity[mode] = b;
	vy = copysign( moveY[mode], vy );
	initLens();

	timer.start( SCI_MAX_SPEED - speed[mode]);
}

void KScienceSaver::setHideBG( bool b )
{
	timer.stop();
	
	releaseLens();
	hideBG[mode] = b;
	initLens();	
	do_refresh( QRect( 0, 0, width, height ) );
		
	timer.start( SCI_MAX_SPEED - speed[mode]);	
}

void KScienceSaver::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
        QString sMode;
  
	config->setGroup( "Settings" );
	mode = config->readNumEntry( "ModeNr", SCI_DEFAULT_MODE );

	for(int i=0; i < MAX_MODES; i++) 
	{
		sMode.setNum( i );
		config->setGroup( "Mode" + sMode ); 
		moveX[i]     = config->readNumEntry(  "MoveX",     SCI_DEFAULT_MOVEX);
		moveY[i]     = config->readNumEntry(  "MoveY",     SCI_DEFAULT_MOVEY);
		size[i]      = config->readNumEntry(  "Size",      SCI_DEFAULT_SIZE);
		speed[i]     = config->readNumEntry(  "Speed",     SCI_DEFAULT_SPEED);
		intensity[i] = config->readNumEntry(  "Intensity", SCI_DEFAULT_INTENSITY);
		inverse[i]   = config->readBoolEntry( "Inverse",   SCI_DEFAULT_INVERSE);
		gravity[i]   = config->readBoolEntry( "Gravity",   SCI_DEFAULT_GRAVITY);
		hideBG[i]    = config->readBoolEntry( "HideBG",    SCI_DEFAULT_HIDE);
	}

	vx = copysign( moveX[mode], vx );
	vy = copysign( moveY[mode], vy );
}              

void KScienceSaver::do_refresh( const QRect & rect )
{
	if( grabPixmap ) 
		return;
	rect.normalize();

	if( hideBG[mode] )
	{
		XSetWindowBackground( qt_xdisplay(), d, black.pixel() );
		XClearArea( qt_xdisplay(), d, rect.left(), rect.top(), 
                            rect.width(), rect.height(), false );
	}
	else 
	{
		myAssert( xRootWin != 0, "root window not grabbed" );
		XPutImage( qt_xdisplay(), d, gc, xRootWin, 
		           rect.left(), rect.top(),
                           rect.left(), rect.top(), 
                           rect.width(), rect.height() );
	}
	         
}

void KScienceSaver::slotTimeout()
{
	if( grabPixmap ) {
		if( !QWidget::find(d)->isActiveWindow() )
			return;
		grabPreviewWidget();
		grabPixmap = false;
		initialize();
		if( hideBG[mode] )
			do_refresh( QRect ( 0, 0, width, height ) );
	}

	signed int oldx = xcoord, oldy = ycoord;

	if( gravity[mode] ) {
		double h = double(y+1.0) / double(height-diam);
		if( h > 1.0 ) h = 1.0;
		vy = sqrt( h ) * ( (vy > 0.0) ? moveY[mode] : -moveY[mode] );
	}
	myAssert( abs((int)rint(vy)) <= border, "assertion violated: vy <= border" );

	if( moveOn )
	{
		x += vx;
		y += vy;
	}

	if( x <= 0.0 ) { 
		vx = -vx; 
		x = 0.0; 
	}
	if( (unsigned int) x + diam >= width) { 
		vx = -vx; 
		myAssert( width-diam > 0, "assertion violated: width-diam > 0" );
		x = (double) (width - diam - 1); 
	}
	if( y <= 0.0 ) { 
		vy = -vy; 
		y = 0.0; 
	}
	if( (unsigned int) y + diam >= height ) { 
		vy = -vy; 
		myAssert( height - diam > 0, "assertion violated: height-diam > 0" );
		y = (double) (height - diam - 1); 
	}

	xcoord = (int) x ;
	ycoord = (int) y ;
	signed int dx = (signed int) xcoord - oldx;
	signed int dy = (signed int) ycoord - oldy;
	signed int xs, ys, xd, yd, w, h;

	if( dx > 0 ) {
		w = diam+dx;
		xd = oldx;		
		xs = border-dx;
		if( dy > 0 ) {
			h = diam+dy;
			yd = oldy;
			ys = border-dy;
		}
		else {
			h = diam-dy;
			yd = ycoord;
			ys = border;
		}
	}
	else {
		w = diam-dx;
		xd = xcoord;
		xs = border;
		if( dy > 0 ) {
			h = diam+dy;
			yd = oldy;
			ys = border-dy;
		} else {
			h = diam-dy;
			yd = ycoord;
			ys = border;
		}
	}

	if( (unsigned int) xd + w >= width  ) w = width  - xd - 1;
	if( (unsigned int) yd + h >= height ) h = height - yd - 1;

//printf("%d: (dx: %3d, dy: %3d), diam: %3d, (xc: %3d, yc: %3d), (xs: %3d, ys: %3d), (xd: %3d, yd: %3d), (w: %3d, h: %3d)\n", mode, dx, dy, diam, xcoord, ycoord, xs, ys, xd, yd, w, h);	
	myAssert( dx <= border && dy <=border, "assertion violated: dx or dy <= border");
	myAssert( xcoord >= 0 && ycoord >= 0, "assertion violated: xcoord, ycoord >= 0 ");
	myAssert( (unsigned int) xd+w < width, "assertion violated: xd+w < width" );
	myAssert( (unsigned int) yd+h < height, "assertion violated: yd+h < height" );

	if( hideBG[mode] )
		blackPixel( xcoord, ycoord );
	(this->*applyLens)(xs, ys, xd, yd, w, h);
	XPutImage( qt_xdisplay(), d, gc, buffer, 0, 0, xd, yd, w, h );
	if( hideBG[mode] )
		blackPixelUndo( xcoord, ycoord );
}

void KScienceSaver::grabRootWindow()
{
	Display *dsp = qt_xdisplay();
	Window rootwin = RootWindow( dsp, qt_xscreen() );

	if( showDialog && dlg ) 
	{
		dlg->setText( i18n( "grabbing screen..." ) );
		dlg->show();
		kapp->processEvents();		
	}

	// grab contents of root window
	if( xRootWin )
		XDestroyImage( xRootWin );

	xRootWin = XGetImage( dsp, rootwin, 0, 0, width,
	                      height, AllPlanes, ZPixmap);
	myAssert( xRootWin, "unable to grab root window\n" );

	imgnext = xRootWin->bytes_per_line;
	bpp = ( xRootWin->bits_per_pixel ) >> 3;

	// remove dialog
	if( showDialog && dlg )
	{
		char *p = xRootWin->data + imgnext*dlg->y + bpp*dlg->x;
		char *q = dlg->save->data;
		for( int y=0; y<dlg->h; y++ )
		{
			memcpy(p, q, bpp*dlg->w);
			p += imgnext;
			q += dlg->bpl;
		}
	}
}

void KScienceSaver::grabPreviewWidget()
{
	myAssert( QWidget::find(d)->isActiveWindow(), "can't grab preview widget: dialog not active()" );

	if( xRootWin )
		XDestroyImage( xRootWin );

	Display *dsp = qt_xdisplay();
	xRootWin = XGetImage( dsp, d, 0, 0, width, height, AllPlanes, ZPixmap);
	myAssert( xRootWin, "unable to grab preview window\n" );

	imgnext = xRootWin->bytes_per_line;
	bpp = ( xRootWin->bits_per_pixel ) >> 3;		
}

void KScienceSaver::blackPixel( int x, int y )
{
	unsigned char black = (char) BlackPixel( qt_xdisplay(), qt_xscreen() );
	unsigned int adr = x*bpp + y*imgnext;

	for(int i=0; i<bpp; i++) {
		blackRestore[i] = xRootWin->data[adr];
		xRootWin->data[adr++] = black;
	}
}

void KScienceSaver::blackPixelUndo( int x, int y )
{
	unsigned int adr = x*bpp + y*imgnext;
	for(int i=0; i<bpp; i++)
		xRootWin->data[adr++] = blackRestore[i];
}

// hm....

void KScienceSaver::applyLens8bpp(int xs, int ys, int xd, int yd, int w, int h)
{
	T32bit *off;
	char *img1, *img2, *data;
	signed int ix, iy, datanext = buffer->bytes_per_line - w;

	img1 = xRootWin->data + xd + yd*imgnext;
	data = buffer->data;
	for(iy = ys; iy < ys+h; iy++)
	{
		off = offset[iy] + xs;
		img2 = img1;
		for(ix = w; ix > 0; ix--)
			*data++ = img2++[*off++];
		img1 += imgnext;
		data += datanext;
	} 

}

void KScienceSaver::applyLens16bpp(int xs, int ys, int xd, int yd, int w, int h)
{
	T32bit *off;
	char *img1, *img2, *data;
	int ix, iy, datanext = buffer->bytes_per_line - (w << 1);

	img1 = xRootWin->data + (xd << 1) + yd*imgnext;
	data = buffer->data;
	for(iy = ys; iy < ys+h; iy++)
	{
		off = offset[iy] + xs;
		img2 = img1;
		for(ix = w; ix > 0; ix--) {
			*data++ = img2++[*off];
			*data++ = img2++[*off++];
		}
		img1 += imgnext;
		data += datanext;
	} 
}
	
void KScienceSaver::applyLens24bpp(int xs, int ys, int xd, int yd, int w, int h)
{
	T32bit *off;
	char *img1, *img2, *data;
	signed int ix, iy, datanext = buffer->bytes_per_line - 3*w;

	img1 = xRootWin->data + 3*xd + yd*imgnext;
	data = buffer->data;
	for(iy = ys; iy < ys+h; iy++)
	{
		off = offset[iy] + xs;
		img2 = img1;
		for(ix = w; ix > 0; ix--) {
			*data++ = img2++[*off];
			*data++ = img2++[*off];
			*data++ = img2++[*off++];
		}
		img1 += imgnext;
		data += datanext;
	} 
}
                      
void KScienceSaver::applyLens32bpp(int xs, int ys, int xd, int yd, int w, int h)
{
	T32bit *off;
	char *img1, *img2, *data;
	signed int ix, iy, datanext = buffer->bytes_per_line - (w << 2);

	img1 = xRootWin->data + (xd << 2) + yd*imgnext;
	data = buffer->data;
	for(iy = ys; iy < ys+h; iy++)
	{
		off = offset[iy] + xs;
		img2 = img1;
		for(ix = w; ix > 0; ix--) {
			*data++ = img2++[*off];
			*data++ = img2++[*off];
			*data++ = img2++[*off];
			*data++ = img2++[*off++];
		}
		img1 += imgnext;
		data += datanext;
	} 
}


//-----------------------------------------------------------------------------

KScienceSetup::KScienceSetup(  QWidget *parent, const char *name ) : 
               QDialog( parent, name, TRUE )  
{
	saver = 0;

	readSettings();
	initModeInfo();

	setCaption( i18n("Setup Science") );

	QLabel *label;
	QPushButton *button;
	QCheckBox *cbox;
	KSlider *sb;
	
	QVBoxLayout *lt  = new QVBoxLayout( this, 10, 10);
	QHBoxLayout *ltu = new QHBoxLayout() ;
	lt->addLayout( ltu );
	QVBoxLayout *ltm = new QVBoxLayout( 5 );
	ltu->addLayout( ltm );
	ltu->addSpacing( 5 );
	QVBoxLayout *ltc = new QVBoxLayout( 5 );
	ltu->addLayout( ltc );
	ltu->addSpacing( 5 );
	
	// mode
	label = new QLabel( i18n("Mode:"), this );
	min_size( label );
	ltm->addWidget( label );

	QListBox *c = new QListBox( this );
	for(int i = 0; i<MAX_MODES; i++)
		c->insertItem( modeInfo[i].name );
	c->setCurrentItem( mode );
	c->adjustSize();
	c->setFixedHeight( 5 * c->fontMetrics().height() );
	connect( c, SIGNAL( highlighted( int ) ), SLOT( slotMode( int ) ) );
	ltm->addWidget( c, 2 );
	ltm->addSpacing( 5 );

	// inverse
	cbox = checkInverse = new QCheckBox( i18n("Inverse"), this );
	cbox->setEnabled( modeInfo[mode].inverseEnable );
	cbox->setChecked( inverse[mode] );
	cbox->adjustSize();
	cbox->setMinimumSize( cbox->sizeHint() );
	connect( cbox, SIGNAL( clicked() ), SLOT( slotInverse() ) );
	ltm->addWidget( cbox );
	ltm->addSpacing( 5 );
	
	// gravity 
	cbox = checkGravity = new QCheckBox( i18n("Gravity"), this );
	cbox->setChecked( gravity[mode] );
	cbox->adjustSize();
	cbox->setMinimumSize( cbox->sizeHint() );
	connect( cbox, SIGNAL( clicked() ), SLOT( slotGravity() ) );
	ltm->addWidget( cbox );
	ltm->addSpacing( 5 );
		
	// hide background
	cbox = checkHideBG = new QCheckBox( i18n("Hide Background"), this );
	cbox->setChecked( hideBG[mode] );
	cbox->adjustSize();
	cbox->setMinimumSize( cbox->sizeHint() );
	connect( cbox, SIGNAL( clicked() ), SLOT( slotHideBG() ) );
	ltm->addWidget( cbox );		
	ltm->addStretch( 1 );
		
	// size 	
	label = new QLabel( i18n("Size:"), this );
	min_size( label );
	ltc->addWidget( label );
		
	sb = slideSize = new KSlider( KSlider::Horizontal, this );
	sb->setMinimumSize( 90, 20 );
	sb->setRange( 9, 50 );
	sb->setSteps( 5, 20 );
	sb->setValue( size[mode] );
	connect( sb, SIGNAL( sliderMoved( int ) ), SLOT( slotSize( int ) ) );
	connect( sb, SIGNAL( sliderPressed()    ), SLOT( slotSliderPressed() ) );
	connect( sb, SIGNAL( sliderReleased()   ), SLOT( slotSliderReleased() ) );

	ltc->addWidget( sb );
	ltc->addSpacing( 3 );

	// intensity
	label = new QLabel( i18n("Intensity:"), this );
	min_size( label );
	ltc->addWidget( label );
		
	sb = slideIntensity = new KSlider( KSlider::Horizontal, this );
	sb->setMinimumSize( 90, 20 );
	sb->setRange( 0, 10 );
	sb->setSteps( 1, 5 );
	sb->setValue( intensity[mode] );
	connect( sb, SIGNAL( sliderMoved( int ) ), SLOT( slotIntensity( int )) );
	connect( sb, SIGNAL( sliderPressed()    ), SLOT( slotSliderPressed() ) );
	connect( sb, SIGNAL( sliderReleased()   ), SLOT( slotSliderReleased() ) );
	ltc->addWidget( sb );
	ltc->addSpacing( 3 );

	// speed
	label = new QLabel( i18n("Speed:"), this );
	min_size( label );
	ltc->addWidget( label );
	
	sb = slideSpeed = new KSlider( KSlider::Horizontal, this );
	sb->setMinimumSize( 90, 20 );
	sb->setRange( 0, SCI_MAX_SPEED );
	sb->setSteps( 10, 50 );
	sb->setValue( speed[mode] );	
	connect( sb, SIGNAL( sliderMoved( int ) ), SLOT( slotSpeed( int ) ) );
	ltc->addWidget( sb );
	ltc->addSpacing( 3 );

	// motion
	label = new QLabel( i18n("Motion:"), this );
	min_size( label );
	ltc->addWidget( label );

	QHBoxLayout *ltcm = new QHBoxLayout() ;
	ltc->addLayout( ltcm );
	
	sb = slideMoveX = new KSlider( KSlider::Horizontal, this );
	sb->setMinimumSize( 40, 20 );
	sb->setRange( 0, SCI_MAX_MOVE );
	sb->setSteps( 5, SCI_MAX_MOVE );
	sb->setValue( moveX[mode] );
	connect( sb, SIGNAL( sliderMoved( int ) ), SLOT( slotMoveX( int ) ) );
	ltcm->addWidget( sb );

	sb = slideMoveY = new KSlider( KSlider::Horizontal, this );
	sb->setMinimumSize( 40, 20 );
	sb->setRange( 0, SCI_MAX_MOVE );
	sb->setSteps( 5, SCI_MAX_MOVE );
	sb->setValue( moveY[mode] );
	connect( sb, SIGNAL( sliderMoved( int ) ), SLOT( slotMoveY( int ) ) );
	ltcm->addWidget( sb );		

	ltc->addStretch( 1 );

	// preview
	preview = new KPreviewWidget( this );
	preview->setFixedSize( 220, 170 );
	QPixmap p( PIXDIR + "kscience.gif" );
	if( p.isNull() )
		preview->setBackgroundColor( black );
	else
		preview->setBackgroundPixmap( p );
	preview->show();	// otherwise saver does not get correct size
	ltu->addWidget( preview );

	// buttons
	KButtonBox *bbox = new KButtonBox( this );
	button = bbox->addButton( i18n("About") );
	connect( button, SIGNAL( clicked() ), SLOT( slotAbout() ) );
	bbox->addStretch( 1 );

	button = bbox->addButton( i18n("OK") );
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = bbox->addButton( i18n("Cancel") );
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
	bbox->layout();
	lt->addWidget( bbox );
	
	lt->freeze();
	// let the preview window display before creating the saver
	kapp->processEvents();

	saver = new KScienceSaver( preview->winId(), true, !p.isNull() );
	preview->notifySaver( saver );
}

KScienceSetup::~KScienceSetup()
{
	delete saver;		// be sure to delete this first
}

void KScienceSetup::updateSettings()
{
	// update dialog
	slideMoveX    ->setValue(   moveX[mode]     );
	slideMoveY    ->setValue(   moveY[mode]     );
	slideSize     ->setValue(   size[mode]      );
	slideSpeed    ->setValue(   speed[mode]     );
	slideIntensity->setValue(   intensity[mode] );
	checkInverse  ->setEnabled( modeInfo[mode].inverseEnable );
	checkInverse  ->setChecked( inverse[mode]   );
	checkGravity  ->setChecked( gravity[mode]   );
	checkHideBG   ->setChecked( hideBG[mode]    );
}

// read settings from config file
void KScienceSetup::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
        QString sMode;
  
	config->setGroup( "Settings" );
	mode = config->readNumEntry( "ModeNr", SCI_DEFAULT_MODE );

	for(int i=0; i < MAX_MODES; i++) 
	{
		sMode.setNum( i );
		config->setGroup( "Mode" + sMode ); 
		moveX[i]     = config->readNumEntry(  "MoveX",     SCI_DEFAULT_MOVEX);
		moveY[i]     = config->readNumEntry(  "MoveY",     SCI_DEFAULT_MOVEY);
		size[i]      = config->readNumEntry(  "Size",      SCI_DEFAULT_SIZE);
		speed[i]     = config->readNumEntry(  "Speed",     SCI_DEFAULT_SPEED);
		intensity[i] = config->readNumEntry(  "Intensity", SCI_DEFAULT_INTENSITY);
		inverse[i]   = config->readBoolEntry( "Inverse",   SCI_DEFAULT_INVERSE);
		gravity[i]   = config->readBoolEntry( "Gravity",   SCI_DEFAULT_GRAVITY);
		hideBG[i]    = config->readBoolEntry( "HideBG",    SCI_DEFAULT_HIDE);
	}
}  

void KScienceSetup::slotMode( int m )
{
	mode = m;

	if( saver )
		saver->setMode( mode );

	updateSettings();
}

void KScienceSetup::slotInverse( )
{
	inverse[mode] = checkInverse->isChecked();

	if( saver )
		saver->setInverse( inverse[mode] );
}

void KScienceSetup::slotGravity( )
{
	gravity[mode] = checkGravity->isChecked();
	
	if( saver )
		saver->setGravity( gravity[mode] );
}

void KScienceSetup::slotHideBG( )
{
	hideBG[mode] = checkHideBG->isChecked();
	
	if( saver )
		saver->setHideBG( hideBG[mode] );
}

void KScienceSetup::slotMoveX( int x )
{
	moveX[mode] = x;
	
	if( saver )
		saver->setMoveX( x );
}

void KScienceSetup::slotMoveY( int y )
{
	moveY[mode] = y;
	
	if( saver )
		saver->setMoveY( y );
}

   
void KScienceSetup::slotSize( int s )
{
	size[mode] = s;

	if( saver )
		saver->setSize( s );
}

void KScienceSetup::slotSpeed( int s )
{
	speed[mode] = s;

	if( saver )
		saver->setSpeed( s );
}
                       
void KScienceSetup::slotIntensity( int i )
{
	intensity[mode] = i;

	if( saver )
		saver->setIntensity( i );
}

void KScienceSetup::slotSliderPressed()
{
	if( saver )
		saver->setMove( false );
}

void KScienceSetup::slotSliderReleased()
{
	if( saver )
		saver->setMove( true );
}

// Ok pressed - save settings and exit
void KScienceSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	QString sSize, sSpeed, sIntensity, sMode;

	config->setGroup( "Settings" );
	config->writeEntry( "ModeNr", mode );

	for(int i=0; i<MAX_MODES; i++)
	{
		sMode.setNum( i );
		config->setGroup( "Mode" + sMode );
		config->writeEntry( "MoveX",     moveX[i]     );
		config->writeEntry( "MoveY",     moveY[i]     );
		config->writeEntry( "Size",      size[i]      );
		config->writeEntry( "Speed",     speed[i]     );
		config->writeEntry( "Intensity", intensity[i] );
		config->writeEntry( "Inverse",   inverse[i]   );
		config->writeEntry( "Gravity",   gravity[i]   );
		config->writeEntry( "HideBG",    hideBG[i]    );
	}

	config->sync();

	accept();
}

void KScienceSetup::slotAbout()
{
	QString about;

	about.sprintf( "%s 0.26.5\n\n%s Rene Beutler (1998)\nrbeutler@g26.ethz.ch",
	               i18n( "Science Version"),
	               i18n( "written by" ) );
	QMessageBox::message( i18n("About Science"), 
	                      (const char *) about,
	                      i18n("OK") );
}
