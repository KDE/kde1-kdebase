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

#include <qlabel.h>
#include <qpushbt.h>
#include <qmsgbox.h> 
#include <qlistbox.h>
#include <qchkbox.h>

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"

#include "kslider.h"
#include "science.h"
#include "science.moc"

#include <X11/Xutil.h>

#define SCI_DEFAULT_MODE          1
#define SCI_DEFAULT_MOVEX         6
#define SCI_DEFAULT_MOVEY         8
#define SCI_DEFAULT_SIZE         15
#define SCI_DEFAULT_INTENSITY     4
#define SCI_DEFAULT_SPEED        70
#define SCI_DEFAULT_INVERSE   false
#define SCI_DEFAULT_GRAVITY   false
#define SCI_MAX_SPEED           100
#define SCI_MAX_MOVE             20

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
	xRootWin = buffer = 0;

	grabRootWindow();

	vx = vy = 0.0;
	readSettings();
	initLens();

	signed int ws = (signed int) (width -  diam);
	signed int hs = (signed int) (height - diam);

	srand( (int) time( (time_t *) NULL ) );
	x = (double) ( (ws > 0) ? (rand() % ws ) : 0 );
	y = (double) ( (hs > 0) ? (rand() % hs ) : 0 );

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

	XPutImage( qt_xdisplay(), d, gc, xRootWin, 0, 0, 0, 0, width, height);
       	refresh = false;

	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
	timer.start( SCI_MAX_SPEED - speed[mode] );
}

KScienceSaver::~KScienceSaver()
{
	timer.stop();
	releaseLens();
	if ( xRootWin )
		XDestroyImage( xRootWin );
}

void KScienceSaver::myAssert( bool term, char *eMsg )
{
	if( !term ) {
		fprintf(stderr, "Error in KScreensaver - mode Science: %s\n", eMsg);
		releaseLens();
		exit(-1);
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
			else {
				off[x] = 0;
 			}
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
				xr = radius*cos(asin(dy/radius));
				yr = radius*cos(asin(dx/radius));
                                phi = (xr != 0.0) ? asin(dx/xr) : 0.0;
                                xo = (T32bit) (origin + intens*2.0*phi*xr / M_PI - x);
                                phi = (yr != 0.0) ? asin(dy/yr) : 0.0;
                                yo = (T32bit) (origin + intens*2.0*phi*yr / M_PI - y);
                                off[x] = xo*bpp + yo*imgnext;
			} 		 			
			else {
				off[x] = 0;
 			}
		}
        }
}

void KScienceSaver::initLens()
{
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

	offset = (T32bit **) malloc( sizeof( T32bit) * side );
	myAssert( offset != 0, "too few memory" );

	switch( mode ) {
		case 0: initWhirlLens();  break;
		case 1: initSphereLens(); break;
		default: myAssert( false, "internal error (wrong mode in initLens() )" );
	}
}

void KScienceSaver::releaseLens()
{
	if( offset != 0 ) {
		for(int i=0; i<side; i++) 
    			if( offset[i] != 0 ) free( offset[i] );
    		free( offset );
	}
	if( buffer != 0 ) free( buffer );
	buffer = 0;
}

void KScienceSaver::setMode( int m )
{
	timer.stop();

	releaseLens();
	mode = m;
	vx = copysign( moveX[mode], vx );
	vy = copysign( moveY[mode], vy );
	initLens();
	refresh = true;

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

void KScienceSaver::setSize( int s )
{
	timer.stop();

	releaseLens();
	size[mode] = s;
	initLens();
	refresh = true;

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
	initLens();

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
	}

	vx = copysign( moveX[mode], vx );
	vy = copysign( moveY[mode], vy );
}              

void KScienceSaver::do_refresh()
{
	refresh = true;
}

void KScienceSaver::slotTimeout()
{
	signed int oldx = xcoord, oldy = ycoord;

	if( refresh ) {
		myAssert( xRootWin != 0, "root window not grabbed" );
		XPutImage( qt_xdisplay(), d, gc, xRootWin, 0, 0, 0, 0, 
		           width, height);
		refresh = false;
	}

	if( gravity[mode] ) {
		double h = double(y+1.0) / double(height-diam);
		if( h > 1.0 ) h = 1.0;
		vy = sqrt( h ) * ( (vy > 0.0) ? moveY[mode] : -moveY[mode] );
	}
	myAssert( abs((int)rint(vy)) <= border, "assertion violated: vy <= border" );

	x += vx;
	y += vy;

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

	myAssert( dx <= border && dy <=border, "assertion violated: dx or dy <= border");
	myAssert( xcoord >= 0 && ycoord >= 0, "assertion violated: xcoord, ycoord >= 0 ");
	myAssert( (unsigned int) xd+w < width, "assertion violated: xd+w < width" );
	myAssert( (unsigned int) yd+h < height, "assertion violated: yd+h < height" );

	(this->*applyLens)(xs, ys, xd, yd, w, h);
	XPutImage( qt_xdisplay(), d, gc, buffer, 0, 0, xd, yd, w, h );
}

void KScienceSaver::grabRootWindow()
{
	// grab contents of root window
	if( xRootWin )
		XDestroyImage( xRootWin );

	Display *dsp = qt_xdisplay();
	Window rootwin = RootWindow( dsp, qt_xscreen() );

	xRootWin = XGetImage( dsp, rootwin, 0, 0, width,
	                      height, AllPlanes, ZPixmap);
	myAssert( xRootWin, "unable to grab root window\n" );

	imgnext = xRootWin->bytes_per_line;
	bpp = ( xRootWin->bits_per_pixel ) >> 3;
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

	setCaption( glocale->translate("Setup Science") );

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
	label = new QLabel( glocale->translate("Mode:"), this );
	min_size( label );
	ltm->addWidget( label );

	static const char *items[MAX_MODES+1] = { 
             glocale->translate( "Whirl"  ), 
	     glocale->translate( "Sphere" ) , 0 };

	QListBox *c = new QListBox( this );
	c->insertStrList( items );
	c->setCurrentItem( mode );
	c->adjustSize();
	c->setMinimumSize( c->size() );
	connect( c, SIGNAL( highlighted( int ) ), SLOT( slotMode( int ) ) );
	ltm->addWidget( c, 2 );
	ltm->addSpacing( 5 );

	// inverse
	cbox = checkInverse = new QCheckBox( glocale->translate("Inverse"), this );
	cbox->setChecked( inverse[mode] );
	cbox->adjustSize();
	cbox->setMinimumSize( cbox->sizeHint() );
	connect( cbox, SIGNAL( clicked() ), SLOT( slotInverse() ) );
	ltm->addWidget( cbox );
	ltm->addSpacing( 5 );
	
	// gravity 
	cbox = checkGravity = new QCheckBox( glocale->translate("Gravity"), this );
	cbox->setChecked( gravity[mode] );
	cbox->adjustSize();
	cbox->setMinimumSize( cbox->sizeHint() );
	connect( cbox, SIGNAL( clicked() ), SLOT( slotGravity() ) );
	ltm->addWidget( cbox );
	ltm->addSpacing( 10 );
	
	ltm->addStretch( 1 );
		
	// size 	
	label = new QLabel( glocale->translate("Size:"), this );
	min_size( label );
	ltc->addWidget( label );
		
	sb = slideSize = new KSlider( KSlider::Horizontal, this );
	sb->setMinimumSize( 90, 20 );
	sb->setRange( 9, 50 );
	sb->setSteps( 5, 20 );
	sb->setValue( size[mode] );
	connect( sb, SIGNAL( sliderMoved( int ) ), SLOT( slotSize( int ) ) );
	ltc->addWidget( sb );
	ltc->addSpacing( 3 );

	// intensity
	label = new QLabel( glocale->translate("Intensity:"), this );
	min_size( label );
	ltc->addWidget( label );
		
	sb = slideIntensity = new KSlider( KSlider::Horizontal, this );
	sb->setMinimumSize( 90, 20 );
	sb->setRange( 0, 10 );
	sb->setSteps( 1, 5 );
	sb->setValue( intensity[mode] );
	connect( sb, SIGNAL( sliderMoved( int ) ), SLOT( slotIntensity( int )) );
	ltc->addWidget( sb );
	ltc->addSpacing( 3 );

	// speed
	label = new QLabel( glocale->translate("Speed:"), this );
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
	label = new QLabel( glocale->translate("Motion:"), this );
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

void KScienceSetup::updateSettings()
{
	// update dialog
	slideMoveX    ->setValue(   moveX[mode]     );
	slideMoveY    ->setValue(   moveY[mode]     );
	slideSize     ->setValue(   size[mode]      );
	slideSpeed    ->setValue(   speed[mode]     );
	slideIntensity->setValue(   intensity[mode] );
	checkInverse  ->setChecked( inverse[mode]   );
	checkGravity  ->setChecked( gravity[mode]   );
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
	}

	config->sync();

	accept();
}

void KScienceSetup::slotAbout()
{
	QMessageBox::message( glocale->translate("About Science"), 
	                      glocale->translate("Science Version 0.24.2 beta\n\n"\
	                      "written by Rene Beutler 1998\n"\
                              "rbeutler@g26.ethz.ch"), 
	                      glocale->translate("Ok") );
}

void KScienceSetup::paintEvent( QPaintEvent * )
{
	if( saver )
		saver->do_refresh();
}

