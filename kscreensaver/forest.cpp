/*-
 * forest.c - forest for xlockmore
 *
 * Copyright (c) 1995 Pascal Pensa <pensa@aurora.unice.fr>
 *
 * Original idea : Guillaume Ramey <ramey@aurora.unice.fr>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 */

/* Ported to kscreensave:
   July 1997, Emanuel Pirker <epirker@edu.uni-klu.ac.at>
   In case of problems contact me, not the original author!
*/
// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define MAXSPEED 100
#define MINSPEED 0
#define DEFSPEED 50
#define MINCYCLES 10
#define MAXCYCLES 790
#define DEFCYCLES 100

#include "xlock.h"
#include <math.h>

#define MINHEIGHT  20		/* Tree height range */
#define MAXHEIGHT  40

#define MINANGLE   15		/* (degree) angle between soon */
#define MAXANGLE   35
#define RANDANGLE  15		/* (degree) Max random angle from default */

#define REDUCE     90		/* Height % from father */

#define ITERLEVEL  10		/* Tree iteration */

#define COLORSPEED  2		/* Color increment */

/* degree to radian */
#define DEGTORAD(x) (((float)(x)) * 0.017453293)

#define PIONTWO 1.5707963

#define MAX_RAND(max) (LRAND() % (max))
#define RANGE_RAND(min,max) ((min) + LRAND() % ((max) - (min)))

//ModeSpecOpt forest_opts = {0, NULL, NULL, NULL};

typedef struct {
	int         width;
	int         height;
	int         time;	/* up time */
} foreststruct;

static foreststruct forests[MAXSCREENS];

static void
draw_tree(Window win, short int x, short int y, short int len, float a, float as, short int c, short int level)
				/* Father's end */
				/* Length */
				/* color */
				/* Height level */
				/* Father's angle */
				/* Father's angle step */
{
	short       x_1, y_1, x_2, y_2;
	float       a1, a2;

	/* left */

	a1 = a + as + DEGTORAD(MAX_RAND(2 * RANDANGLE) - RANDANGLE);

	x_1 = x + (short) (COSF(a1) * ((float) len));
	y_1 = y + (short) (SINF(a1) * ((float) len));

	/* right */

	a2 = a - as + DEGTORAD(MAX_RAND(2 * RANDANGLE) - RANDANGLE);

	x_2 = x + (short) (COSF(a2) * ((float) len));
	y_2 = y + (short) (SINF(a2) * ((float) len));

	if (!mono && Scr[screen].npixels > 2) {
		XSetForeground(dsp, Scr[screen].gc, Scr[screen].pixels[c]);
		c = (c + COLORSPEED) % Scr[screen].npixels;
	} else
		XSetForeground(dsp, Scr[screen].gc, WhitePixel(dsp, screen));

	XDrawLine(dsp, win, Scr[screen].gc, x, y, x_1, y_1);
	XDrawLine(dsp, win, Scr[screen].gc, x, y, x_2, y_2);

	if (level < 2) {
		XDrawLine(dsp, win, Scr[screen].gc, x + 1, y, x_1 + 1, y_1);
		XDrawLine(dsp, win, Scr[screen].gc, x + 1, y, x_2 + 1, y_2);
	}
	len = (len * REDUCE * 10) / 1000;

	if (level < ITERLEVEL) {
		draw_tree(win, x_1, y_1, len, a1, as, c, level + 1);
		draw_tree(win, x_2, y_2, len, a2, as, c, level + 1);
	}
}

void
initforest(Window win)
{
  foreststruct *fp = &forests[screen];
  XWindowAttributes xwa;
  
  (void) XGetWindowAttributes(dsp, win, &xwa);
  fp->width = xwa.width;
  fp->height = xwa.height;
  fp->time = 0;
  
  XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
  XFillRectangle(dsp, win, Scr[screen].gc, 0, 0, fp->width, fp->height);
}

void
drawforest(Window win)
{
  short       x, y, x_2, y_2, len, c = 0;
  float       a, as;
  foreststruct *fp = &forests[screen];

  if (fp->time == cycles) {
    fp->time = 0;
    initforest(win);
  }
  x = RANGE_RAND(0, fp->width);
  y = RANGE_RAND(0, fp->height + MAXHEIGHT);
  a = -PIONTWO + DEGTORAD(MAX_RAND(2 * RANDANGLE) - RANDANGLE);
  as = DEGTORAD(RANGE_RAND(MINANGLE, MAXANGLE));
  len = ((RANGE_RAND(MINHEIGHT, MAXHEIGHT) * (fp->width / 20)) / 50) + 2;
  
  if (!mono && Scr[screen].npixels > 2) {
    c = MAX_RAND(Scr[screen].npixels);
    XSetForeground(dsp, Scr[screen].gc, Scr[screen].pixels[c]);
    c = (c + COLORSPEED) % Scr[screen].npixels;
  } else
    XSetForeground(dsp, Scr[screen].gc, WhitePixel(dsp, screen));
  
  x_2 = x + (short) (COSF(a) * ((float) len));
  y_2 = y + (short) (SINF(a) * ((float) len));
  
  XDrawLine(dsp, win, Scr[screen].gc, x, y, x_2, y_2);
  XDrawLine(dsp, win, Scr[screen].gc, x + 1, y, x_2 + 1, y_2);
  
  draw_tree(win, x_2, y_2, (len * REDUCE) / 100, a, as, c, 1);
  fp->time++;
}

//---------------------------------------------------

#include <qpushbt.h>
#include <qchkbox.h>
#include <qcolor.h>
#include <qmsgbox.h>
#include "kslider.h"

#include "forest.h"

#include "forest.moc"

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"


// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kForestSaver *saver = NULL;

void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kForestSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	kForestSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("Forest");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//-----------------------------------------------------------------------------

kForestSaver::kForestSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();

	colorContext = QColor::enterAllocContext();

	cycles = numPoints;

	initXLock( gc );
	initforest( d );

	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kForestSaver::~kForestSaver()
{
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void kForestSaver::setSpeed( int spd )
{
	timer.stop();
	speed = MAXSPEED - spd;
	timer.start( speed );
}

void kForestSaver::setPoints( int p )
{
	cycles = numPoints = p;
	initforest( d );
}

void kForestSaver::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = MAXSPEED - atoi( str );
	else
		speed = DEFSPEED;

	str = config->readEntry( "NumPoints" );
	if ( !str.isNull() )
		numPoints = atoi( str );
	else
		numPoints = DEFCYCLES;
}

void kForestSaver::slotTimeout()
{
	drawforest( d );
}

//-----------------------------------------------------------------------------

kForestSetup::kForestSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	readSettings();

	setCaption( glocale->translate("Setup KForest") );

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

	label = new QLabel( glocale->translate("Num of Trees:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new KSlider( KSlider::Horizontal, this );
	slider->setMinimumSize( 90, 20 );
	slider->setRange( MINCYCLES, MAXCYCLES );
	slider->setSteps( (MAXCYCLES-MINCYCLES)/4, (MAXCYCLES-MINCYCLES)/2 );
	slider->setValue( numPoints );
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotPoints( int ) ) );
	tl11->addWidget(slider);
	tl11->addStretch(1);

	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new kForestSaver( preview->winId() );
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

void kForestSetup::readSettings()
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

	str = config->readEntry( "NumPoints" );
	if ( !str.isNull() )
		numPoints = atoi( str );
	else
		numPoints = DEFCYCLES;
}

void kForestSetup::slotSpeed( int num )
{
	speed = num;

	if ( saver )
		saver->setSpeed( speed );
}

void kForestSetup::slotPoints( int num )
{
	numPoints = num;

	if ( saver )
		saver->setPoints( numPoints );
}

void kForestSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString sspeed;
	sspeed.setNum( speed );
	config->writeEntry( "Speed", sspeed );

	QString spoints;
	spoints.setNum( numPoints );
	config->writeEntry( "NumPoints", spoints );

	config->sync();
	accept();
}

void kForestSetup::slotAbout()
{
	QMessageBox::message(glocale->translate("About Forest"),
			     glocale->translate("Forest\n\nCopyright (c) 1995 by Pascal Pensa\n\nPorted to kscreensave by Emanuel Pirker."), 
			     glocale->translate("OK"));
}


