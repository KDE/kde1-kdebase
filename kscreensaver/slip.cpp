
/*-
 * slip.c - lots of blits
 *
 * Copyright (c) 1992 by Scott Draves (spot@cs.cmu.edu)
 *
 * See xlock.c for copying information.
 * 01-Dec-95: Patched for VMS <joukj@alpha.chem.uva.nl>.
 */

/* Ported to kscreensave:
   July 1997, Emanuel Pirker <epirker@edu.uni-klu.ac.at>
   Last revised: 10-Jul-97
   Please contact me in case of problems, bugs etc!
*/
// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>


#define MINSPEED 0
#define MAXSPEED 100
#define DEFSPEED 100
#define MINBATCH 0
#define MAXBATCH 100
#define DEFBATCH 10

#include "xlock.h"
#include <math.h>

//ModeSpecOpt slip_opts = {0, NULL, NULL, NULL};

typedef struct {
	int         color;
	int         width, height;

	int         nblits_remaining;
	int         blit_width, blit_height;
	int         mode;
	int         first_time;
} slipstruct;
static slipstruct slips[MAXSCREENS];

static short
halfrandom(int mv)
{
	static short lasthalf = 0;
	unsigned long r;

	if (lasthalf) {
		r = lasthalf;
		lasthalf = 0;
	} else {
		r = LRAND();
		lasthalf = r >> 16;
	}
	return r % mv;
}

static int
erandom(int mv)
{
	static int  stage = 0;
	static unsigned long r;
	int         res;

	if (0 == stage) {
		r = LRAND();
		stage = 7;
	}
	res = r & 0xf;
	r = r >> 4;
	stage--;
	if (res & 8)
		return res & mv;
	else
		return -(res & mv);
}

void
initslip(Window win)
{
	slipstruct *s = &slips[screen];

	XWindowAttributes xwa;

	(void) XGetWindowAttributes(dsp, win, &xwa);
	s->width = xwa.width;
	s->height = xwa.height;
	s->color = Scr[screen].npixels > 2;

	s->blit_width = s->width / 25;
	s->blit_height = s->blit_width;
	s->nblits_remaining = 0;
	s->mode = 0;
	s->first_time = 1;

	/* no "NoExpose" events from XCopyArea wanted */
	XSetGraphicsExposures(dsp, Scr[screen].gc, False);
}

static void
prepare_screen(Window win, slipstruct * s)
{
	int         i, n, w = s->width / 20;
	int         not_solid = halfrandom(10);

	if (s->first_time || !halfrandom(5)) {
		XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
		XFillRectangle(dsp, win, Scr[screen].gc, 0, 0, s->width, s->height);
		n = 300;
	} else {
		if (halfrandom(5))
			return;
		if (halfrandom(5))
			n = 100;
		else
			n = 2000;
	}

	XSetForeground(dsp, Scr[screen].gc,
		       Scr[screen].pixels[halfrandom(Scr[screen].npixels)]);

	for (i = 0; i < n; i++) {
		if (not_solid)
			XSetForeground(dsp, Scr[screen].gc,
			Scr[screen].pixels[halfrandom(Scr[screen].npixels)]);
		XFillRectangle(dsp, win, Scr[screen].gc,
			       halfrandom(s->width - w),
			       halfrandom(s->height - w),
			       w, w);
	}
	s->first_time = 0;
}

static int
quantize(double d)
{
  int         i = (int)floor(d);  //(int) included to get rid of warning
	double      f = d - i;

	if ((LRAND() & 0xff) < f * 0xff)
		i++;
	return i;
}


void
drawslip(Window win)
{
	slipstruct *s = &slips[screen];
	GC          gc = Scr[screen].gc;

	int         xi = halfrandom(s->width - s->blit_width);
	int         yi = halfrandom(s->height - s->blit_height);
	double      x, y, dx = 0, dy = 0, t, s1, s2;
	
	if (0 == s->nblits_remaining--) {
	  static    int  lut[] =
	  {0, 0, 0, 1, 1, 1, 2};
	  
	  prepare_screen(win, s);
	  s->nblits_remaining = batchcount *
	    (2000 + halfrandom(1000) + halfrandom(1000));
	  if (s->mode == 2)
	    s->mode = halfrandom(2);
	  else
	    s->mode = lut[halfrandom(7)];
	}
	x = (2 * xi + s->blit_width) / (double) s->width - 1;
	y = (2 * yi + s->blit_height) / (double) s->height - 1;
	
	/* (x,y) is in biunit square */
	switch (s->mode) {
	case 0:
	  dx = x;
	  dy = y;
	  
	  if (dy < 0) {
	    dy += 0.04;
	    if (dy > 0)
	      dy = 0.00;
	  }
	  if (dy > 0) {
	    dy -= 0.04;
	    if (dy < 0)
	      dy = 0.00;
	  }
	  t = dx * dx + dy * dy + 1e-10;
	  s1 = 2 * dx * dx / t - 1;
	  s2 = 2 * dx * dy / t;
	  
	  dx = s1 * 5;
	  dy = s2 * 5;
	  break;
	case 1:
	  dx = erandom(3);
	  dy = erandom(3);
	  break;
	case 2:
	  dx = x * 3;
	  dy = y * 3;
	  break;
	}
	{
	  int         qx = xi + quantize(dx), qy = yi + quantize(dy);
	  int         wrap;
	  
	  if (!(qx < 0 || qy < 0 ||
		qx >= s->width - s->blit_width ||
		qy >= s->height - s->blit_height)) {
	    
	    XCopyArea(dsp, win, win, gc, xi, yi,
		      s->blit_width, s->blit_height,
		      qx, qy);
	    
	    switch (s->mode) {
	    case 0:
	      /* wrap */
	      wrap = s->width - (2 * s->blit_width);
	      if (qx > wrap)
		XCopyArea(dsp, win, win, gc, qx, qy,
			  s->blit_width, s->blit_height,
			  qx - wrap, qy);
	      
	      if (qx < 2 * s->blit_width)
		XCopyArea(dsp, win, win, gc, qx, qy,
			  s->blit_width, s->blit_height,
			  qx + wrap, qy);
	      
	      wrap = s->height - (2 * s->blit_height);
	      if (qy > wrap)
		XCopyArea(dsp, win, win, gc, qx, qy,
			  s->blit_width, s->blit_height,
			  qx, qy - wrap);
	      
	      if (qy < 2 * s->blit_height)
		XCopyArea(dsp, win, win, gc, qx, qy,
			  s->blit_width, s->blit_height,
			  qx, qy + wrap);
	      break;
	    case 1:
	    case 2:
	      break;
	    }
	  } 
	}
}

//-----------------------------------------------------------------------------

#include <qpushbt.h>
#include <qchkbox.h>
#include <qcolor.h>
#include <qmsgbox.h>
#include "kslider.h"

#include "slip.h"

#include "slip.moc"

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kSlipSaver *saver = NULL;

void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kSlipSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	kSlipSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("Slip");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//-----------------------------------------------------------------------------

kSlipSaver::kSlipSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();

	colorContext = QColor::enterAllocContext();

	batchcount = maxLevels;

	initXLock( gc );
	initslip( d );
	
	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kSlipSaver::~kSlipSaver()
{
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void kSlipSaver::setSpeed( int spd )
{
	timer.stop();
	speed = MAXSPEED - spd;
	timer.start( speed );
}

void kSlipSaver::setLevels( int l )
{
	batchcount = maxLevels = l;
	initslip( d );
}

void kSlipSaver::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = MAXSPEED - atoi( str );
	else
		speed = DEFSPEED;

	str = config->readEntry( "MaxLevels" );
	if ( !str.isNull() )
		maxLevels = atoi( str );
	else
		maxLevels = DEFBATCH;

}

void kSlipSaver::slotTimeout()
{
	drawslip( d );
}

//-----------------------------------------------------------------------------

kSlipSetup::kSlipSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	readSettings();

	setCaption( glocale->translate("Setup KSlip") );

	QLabel *label;
	QPushButton *button;
	KSlider *slider;

	QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
	QHBoxLayout *tl1 = new QHBoxLayout;
	tl->addLayout(tl1);
	QVBoxLayout *tl11 = new QVBoxLayout(5);
	tl1->addLayout(tl11);	

	label = new QLabel( glocale->translate("Speed:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new KSlider( KSlider::Horizontal, this );
	slider->setMinimumSize( 90, 20 );
	slider->setRange( MINSPEED, MAXSPEED );
	slider->setSteps( (MAXSPEED-MINSPEED)/4, (MAXSPEED-MINSPEED)/2 );
	slider->setValue( speed );
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotSpeed( int ) ) );
	tl11->addWidget(slider);
	tl11->addSpacing(5);

	label = new QLabel( glocale->translate("Batchcount:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new KSlider( KSlider::Horizontal, this );
	slider->setMinimumSize( 90, 20 );
	slider->setRange( MINBATCH, MAXBATCH );
	slider->setSteps( (MAXBATCH-MINBATCH)/4, (MAXBATCH-MINBATCH)/2 );
	slider->setValue( maxLevels );
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotLevels( int ) ) );
	tl11->addWidget(slider);
	tl11->addStretch(1);

	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new kSlipSaver( preview->winId() );
	tl1->addWidget(preview);

	KButtonBox *bbox = new KButtonBox(this);	
	button = bbox->addButton( glocale->translate("About"));
	connect( button, SIGNAL( clicked() ), SLOT(slotAbout() ) );
	bbox->addStretch(1);

	button = bbox->addButton( glocale->translate("OK"));	
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = bbox->addButton(glocale->translate("Cancel"));
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
	bbox->layout();
	tl->addWidget(bbox);

	tl->freeze();
}

void kSlipSetup::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = atoi( str );

	if ( speed > MAXSPEED )
		speed = MAXSPEED;
	else if ( speed < MINSPEED )
		speed = MINSPEED;

	str = config->readEntry( "MaxLevels" );
	if ( !str.isNull() )
		maxLevels = atoi( str );
	else
		maxLevels = DEFBATCH;

}

void kSlipSetup::slotSpeed( int num )
{
	speed = num;

	if ( saver )
		saver->setSpeed( speed );
}

void kSlipSetup::slotLevels( int num )
{
	maxLevels = num;

	if ( saver )
		saver->setLevels( maxLevels );
}

void kSlipSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString sspeed;
	sspeed.setNum( speed );
	config->writeEntry( "Speed", sspeed );

	QString slevels;
	slevels.setNum( maxLevels );
	config->writeEntry( "MaxLevels", slevels );

	config->sync();
	accept();
}

void kSlipSetup::slotAbout()
{
	QMessageBox::message(glocale->translate("About Slip"), 
			     glocale->translate("Slip\n\nCopyright (c) 1991 by Scott Draves\n\nPorted to kscreensave by Emanuel Pirker."),
			     glocale->translate("OK"));
}


