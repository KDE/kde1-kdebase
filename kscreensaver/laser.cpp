#ifdef lint
static char sccsid[] = "@(#)laser.c	3.3 95/09/26 xlockmore";
#endif
/*-
 * laser.c - laser for xlockmore
 *
 * Copyright (c) 1995 Pascal Pensa <pensa@aurora.unice.fr>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 */
// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>

#include "xlock.h"

#define MINREDRAW 3  /* Number of redrawn on each frame */
#define MAXREDRAW 8

#define MINLASER  2  /* Laser number */
#define MAXLASER  10 

#define MINWIDTH  2  /* Laser ray width range */
#define MAXWIDTH 40

#define MINSPEED  2  /* Speed range */
#define MAXSPEED 17

#define MINDIST  10  /* Minimal distance from edges */

#define COLORSTEP 2  /* Laser color step */

#define MAX_RAND(max) (LRAND() % (max))
#define RANGE_RAND(min,max) ((min) + LRAND() % ((max) - (min)))

//ModeSpecOpt laseropts = {0, NULL, NULL, NULL};

typedef enum {
  TOP, RIGHT, BOTTOM, LEFT 
} border;

typedef struct {
  int       bx;			/* border x */
  int       by;			/* border y */
  border    bn;			/* active border */
  int       dir;		/* direction */
  int       speed;		/* laser velocity from MINSPEED to MAXSPEED */
  int       sx[MAXWIDTH];       /* x stack */
  int       sy[MAXWIDTH];       /* x stack */
  XGCValues gcv;		/* for color */
} laserstruct;

typedef struct {
  int		width;
  int		height;
  int           cx;		/* center x */
  int           cy;		/* center y */
  int           lw;             /* laser width */
  int           ln;             /* laser number */
  int           lr;             /* laser redraw */
  int           sw;             /* stack width */
  int           so;             /* stack offset */
  int		time;           /* up time */
  GC		stippled_GC;
  laserstruct  *laser;
} lasersstruct;

static lasersstruct lasers[MAXSCREENS];

static XGCValues gcv_black;     /* for black color */

void
initlaser(Window win)
{
  int i, c = 0;
  XGCValues gcv;
  Screen *scr;
  XWindowAttributes xgwa;
  lasersstruct *lp = &lasers[screen];

  (void) XGetWindowAttributes(dsp, win, &xgwa);
  scr = ScreenOfDisplay(dsp, screen);
  lp->width = xgwa.width;
  lp->height = xgwa.height;
  lp->time = 0;

  if (batchcount >= MAXLASER)
    lp->ln = RANGE_RAND(MINLASER, MAXLASER);
  else
    lp->ln = batchcount;
  /* Absolute minimum */
  if (lp->ln < MINLASER)
    lp->ln = MINLASER;

  if (!lp->laser) {
    lp->laser = (laserstruct *) malloc(MAXLASER * sizeof(laserstruct));

    gcv.foreground = WhitePixelOfScreen(scr);
    gcv.background = BlackPixelOfScreen(scr); 
    gcv_black.foreground = BlackPixelOfScreen(scr);

    lp->stippled_GC = XCreateGC(dsp, win,
			      GCForeground|GCBackground, &gcv);
  }

  XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
  XFillRectangle(dsp, win, Scr[screen].gc, 0, 0, lp->width, lp->height);


  if (MINDIST < lp->width - MINDIST)
    lp->cx = RANGE_RAND(MINDIST, lp->width - MINDIST);
  else
    lp->cx = RANGE_RAND(0, lp->width);
  if (MINDIST < lp->height - MINDIST)
    lp->cy = RANGE_RAND(MINDIST, lp->height - MINDIST);
  else
    lp->cy = RANGE_RAND(0, lp->height);
  lp->lw = RANGE_RAND(MINWIDTH, MAXWIDTH);
  lp->lr = RANGE_RAND(MINREDRAW, MAXREDRAW);
  lp->sw = 0;
  lp->so = 0;

  if (!mono && Scr[screen].npixels > 2)
    c = LRAND() % Scr[screen].npixels;

  for (i = 0; i < lp->ln; i++) {
    laserstruct *l = &lp->laser[i];

    l->bn = (border) MAX_RAND(4);

    switch (l->bn) {
      case TOP:
        l->bx = MAX_RAND(lp->width);
        l->by = 0;
        break;
      case RIGHT:
        l->bx = lp->width;
        l->by = MAX_RAND(lp->height);
        break;
      case BOTTOM:
        l->bx = MAX_RAND(lp->width);
        l->by = lp->height;
        break;
      case LEFT:
        l->bx = 0;
        l->by = MAX_RAND(lp->height);
    }

    l->dir = MAX_RAND(2);
    l->speed = ((RANGE_RAND(MINSPEED,MAXSPEED) * lp->width) / 1000) + 1;
    if (!mono && Scr[screen].npixels > 2) {
      l->gcv.foreground = Scr[screen].pixels[c];
      c = (c + COLORSTEP) % Scr[screen].npixels;
    } else
      l->gcv.foreground = WhitePixel(dsp, screen);
  }
}

static void
drawlaser_once(Window win)
{
  lasersstruct *lp = &lasers[screen];
  int i;

  for(i=0; i<lp->ln; i++){
    laserstruct *l = &lp->laser[i];

    if(lp->sw >= lp->lw){
      XChangeGC (dsp, lp->stippled_GC, GCForeground, &gcv_black);
      XDrawLine (dsp, win, lp->stippled_GC,
		 lp->cx, lp->cy,
		 l->sx[lp->so], l->sy[lp->so]);
    }

    if(l->dir){
      switch(l->bn){
      case TOP:
	l->bx -= l->speed;
	if(l->bx < 0){
	  l->by = - l->bx;
	  l->bx = 0;
	  l->bn = LEFT;
	}
	break;
      case RIGHT:
	l->by -= l->speed;
	if(l->by < 0){
	  l->bx =  lp->width + l->by;
	  l->by = 0;
	  l->bn = TOP;
	}
	break;
      case BOTTOM:
	l->bx += l->speed;
	if(l->bx >= lp->width){
	  l->by = lp->height - l->bx % lp->width;
	  l->bx = lp->width;
	  l->bn = RIGHT;
	}
	break;
      case LEFT:
	l->by += l->speed;
	if(l->by >= lp->height){
	  l->bx = l->by % lp->height;
	  l->by = lp->height;
	  l->bn = BOTTOM;
	}
      }
    }
    else{
      switch(l->bn){
      case TOP:
	l->bx += l->speed;
	if(l->bx >= lp->width){
	  l->by = l->bx % lp->width;
	  l->bx = lp->width;
	  l->bn = RIGHT;
	}
	break;
      case RIGHT:
	l->by += l->speed;
	if(l->by >= lp->height){
	  l->bx = lp->width - l->by % lp->height;
	  l->by = lp->height;
	  l->bn = BOTTOM;
	}
	break;
      case BOTTOM:
	l->bx -=  l->speed;
	if(l->bx < 0){
	  l->by = lp->height + l->bx;
	  l->bx = 0;
	  l->bn = LEFT;
	}
	break;
      case LEFT:
	l->by -=  l->speed;
	if(l->by < 0){
	  l->bx =  - l->bx;
	  l->by = 0;
	  l->bn = TOP;
	}
      }
    }

    XChangeGC (dsp, lp->stippled_GC, GCForeground, &l->gcv);
    XDrawLine (dsp, win, lp->stippled_GC, lp->cx, lp->cy, l->bx, l->by);

    l->sx[lp->so] = l->bx;
    l->sy[lp->so] = l->by;

  }

  if(lp->sw < lp->lw)
    ++lp->sw;

  lp->so = (lp->so + 1) % lp->lw;
}

void
drawlaser(Window win)
{
  lasersstruct *lp = &lasers[screen];
  int i;

  for(i=0; i<lp->lr; i++)
    drawlaser_once(win);

  if (++lp->time > cycles)
    initlaser(win);
}

//-----------------------------------------------------------------------------

void laser_cleanup()
{
	lasersstruct *lp = &lasers[screen];

	free( lp->laser );
	lp->laser = NULL;
}

//-----------------------------------------------------------------------------

#include <qpushbt.h>
#include <qchkbox.h>
#include <qcolor.h>
#include <qmsgbox.h>
#include "kslider.h"

#include "laser.h"

#include "laser.moc"

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"


// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kLaserSaver *saver = NULL;

void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kLaserSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	kLaserSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("Laser");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//-----------------------------------------------------------------------------

kLaserSaver::kLaserSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();

	colorContext = QColor::enterAllocContext();

	initXLock( gc );
	initlaser( d );

	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kLaserSaver::~kLaserSaver()
{
	timer.stop();
	laser_cleanup();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void kLaserSaver::setSpeed( int spd )
{
	speed = 100-spd;
	timer.changeInterval( speed );
}

void kLaserSaver::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = 100 - atoi( str );
	else
		speed = 50;
}

void kLaserSaver::slotTimeout()
{
	drawlaser( d );
}

//-----------------------------------------------------------------------------

kLaserSetup::kLaserSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	speed = 50;

	readSettings();

	setCaption( glocale->translate("Setup KLaser") );

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
	slider->setRange( 0, 100 );
	slider->setSteps( 25, 50 );
	slider->setValue( speed );
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotSpeed( int ) ) );
	tl11->addWidget(slider);
	tl11->addStretch(1);

	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new kLaserSaver( preview->winId() );
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

void kLaserSetup::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = atoi( str );

	if ( speed > 100 )
		speed = 100;
	else if ( speed < 50 )
		speed = 50;
}

void kLaserSetup::slotSpeed( int num )
{
	speed = num;

	if ( saver )
		saver->setSpeed( speed );
}

void kLaserSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString sspeed;
	sspeed.setNum( speed );
	config->writeEntry( "Speed", sspeed );

	config->sync();
	accept();
}

void kLaserSetup::slotAbout()
{
	QMessageBox::message(glocale->translate("About Laser"),
			     glocale->translate("Laser Version 3.3\n\nCopyright (c) 1995 by Pascal Pensa\n\nPorted to kscreensave by Martin Jones."),
			     glocale->translate("OK"));
}

