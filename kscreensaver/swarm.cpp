/*-
 * swarm.c - swarm of bees for xlock, the X Window System lockscreen.
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * Revision History:
 * 31-Aug-90: Adapted from xswarm by Jeff Butterworth. (butterwo@ncsc.org)
 */

/* Ported to kscreensave:
   July 1997, Emanuel Pirker <epirker@edu.uni-klu.ac.at>
   Contact me in case of problems, not the original author!
   Last revised: 10-Jul-97
*/
// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>

#define MAXSPEED 100
#define MINSPEED 0
#define DEFSPEED 50
#define MAXBATCH 200
#define MINBATCH 0
#define DEFBATCH 20

#include "xlock.h"

#define TIMES	4		/* number of time positions recorded */
#define BEEACC	2		/* acceleration of bees */
#define WASPACC 5		/* maximum acceleration of wasp */
#define BEEVEL	12		/* maximum bee velocity */
#define WASPVEL 10		/* maximum wasp velocity */

/* Macros */
#define X(t,b)	(sp->x[(t)*sp->beecount+(b)])
#define Y(t,b)	(sp->y[(t)*sp->beecount+(b)])
#define balance_rand(v)	((LRAND()%(v))-((v)/2))		/* random number around 0 */

//ModeSpecOpt swarm_opts = {0, NULL, NULL, NULL};

typedef struct {
	int         pix;
	int         width;
	int         height;
	int         border;	/* wasp won't go closer than this to the edge */
	int         beecount;	/* number of bees */
	XSegment    segs[MAXBATCH];	/* bee lines */
	XSegment    old_segs[MAXBATCH];	/* old bee lines */
	short       x[MAXBATCH*TIMES];
	short       y[MAXBATCH*TIMES];		/* bee positions x[time][bee#] */
	short       xv[MAXBATCH];
	short       yv[MAXBATCH];		/* bee velocities xv[bee#] */
	short       wx[3];
	short       wy[3];
	short       wxv;
	short       wyv;
} swarmstruct;

static swarmstruct swarms[MAXSCREENS];

void
initswarm(Window win)
{
	swarmstruct *sp = &swarms[screen];
	int         b;
	XWindowAttributes xwa;

	sp->beecount = batchcount;
	(void) XGetWindowAttributes(dsp, win, &xwa);
	sp->width = xwa.width;
	sp->height = xwa.height;

	sp->border = (sp->width + sp->height) / 50;

	/* Clear the background. */
	XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
	XFillRectangle(dsp, win, Scr[screen].gc, 0, 0, sp->width, sp->height);

	/*  Now static data structures. epirker */
	//if (!sp->segs) {
	//sp->segs = (XSegment *) malloc(sizeof (XSegment) * sp->beecount);
	//sp->old_segs = (XSegment *) malloc(sizeof (XSegment) * sp->beecount);
	//sp->x = (short *) malloc(sizeof (short) * sp->beecount * TIMES);
	//sp->y = (short *) malloc(sizeof (short) * sp->beecount * TIMES);
	//sp->xv = (short *) malloc(sizeof (short) * sp->beecount);
	//sp->yv = (short *) malloc(sizeof (short) * sp->beecount);
	//}
	/* Initialize point positions, velocities, etc. */

	/* wasp */
	sp->wx[0] = sp->border + LRAND() % (sp->width - 2 * sp->border);
	sp->wy[0] = sp->border + LRAND() % (sp->height - 2 * sp->border);
	sp->wx[1] = sp->wx[0];
	sp->wy[1] = sp->wy[0];
	sp->wxv = 0;
	sp->wyv = 0;

	/* bees */
	for (b = 0; b < sp->beecount; b++) {
		X(0, b) = LRAND() % sp->width;
		X(1, b) = X(0, b);
		Y(0, b) = LRAND() % sp->height;
		Y(1, b) = Y(0, b);
		sp->xv[b] = balance_rand(7);
		sp->yv[b] = balance_rand(7);
	}
}



void
drawswarm(Window win)
{
	swarmstruct *sp = &swarms[screen];
	int         b;

	/* <=- Wasp -=> */
	/* Age the arrays. */
	sp->wx[2] = sp->wx[1];
	sp->wx[1] = sp->wx[0];
	sp->wy[2] = sp->wy[1];
	sp->wy[1] = sp->wy[0];
	/* Accelerate */
	sp->wxv += balance_rand(WASPACC);
	sp->wyv += balance_rand(WASPACC);

	/* Speed Limit Checks */
	if (sp->wxv > WASPVEL)
		sp->wxv = WASPVEL;
	if (sp->wxv < -WASPVEL)
		sp->wxv = -WASPVEL;
	if (sp->wyv > WASPVEL)
		sp->wyv = WASPVEL;
	if (sp->wyv < -WASPVEL)
		sp->wyv = -WASPVEL;

	/* Move */
	sp->wx[0] = sp->wx[1] + sp->wxv;
	sp->wy[0] = sp->wy[1] + sp->wyv;

	/* Bounce Checks */
	if ((sp->wx[0] < sp->border) || (sp->wx[0] > sp->width - sp->border - 1)) {
		sp->wxv = -sp->wxv;
		sp->wx[0] += sp->wxv;
	}
	if ((sp->wy[0] < sp->border) || (sp->wy[0] > sp->height - sp->border - 1)) {
		sp->wyv = -sp->wyv;
		sp->wy[0] += sp->wyv;
	}
	/* Don't let things settle down. */
	sp->xv[LRAND() % sp->beecount] += balance_rand(3);
	sp->yv[LRAND() % sp->beecount] += balance_rand(3);

	/* <=- Bees -=> */
	for (b = 0; b < sp->beecount; b++) {
		int         distance, dx, dy;

		/* Age the arrays. */
		X(2, b) = X(1, b);
		X(1, b) = X(0, b);
		Y(2, b) = Y(1, b);
		Y(1, b) = Y(0, b);

		/* Accelerate */
		dx = sp->wx[1] - X(1, b);
		dy = sp->wy[1] - Y(1, b);
		distance = abs(dx) + abs(dy);	/* approximation */
		if (distance == 0)
			distance = 1;
		sp->xv[b] += (dx * BEEACC) / distance;
		sp->yv[b] += (dy * BEEACC) / distance;

		/* Speed Limit Checks */
		if (sp->xv[b] > BEEVEL)
			sp->xv[b] = BEEVEL;
		if (sp->xv[b] < -BEEVEL)
			sp->xv[b] = -BEEVEL;
		if (sp->yv[b] > BEEVEL)
			sp->yv[b] = BEEVEL;
		if (sp->yv[b] < -BEEVEL)
			sp->yv[b] = -BEEVEL;

		/* Move */
		X(0, b) = X(1, b) + sp->xv[b];
		Y(0, b) = Y(1, b) + sp->yv[b];

		/* Fill the segment lists. */
		sp->segs[b].x1 = X(0, b);
		sp->segs[b].y1 = Y(0, b);
		sp->segs[b].x2 = X(1, b);
		sp->segs[b].y2 = Y(1, b);
		sp->old_segs[b].x1 = X(1, b);
		sp->old_segs[b].y1 = Y(1, b);
		sp->old_segs[b].x2 = X(2, b);
		sp->old_segs[b].y2 = Y(2, b);
	}

	XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
	XDrawLine(dsp, win, Scr[screen].gc,
		  sp->wx[1], sp->wy[1], sp->wx[2], sp->wy[2]);
	XDrawSegments(dsp, win, Scr[screen].gc, sp->old_segs, sp->beecount);

	XSetForeground(dsp, Scr[screen].gc, WhitePixel(dsp, screen));
	XDrawLine(dsp, win, Scr[screen].gc,
		  sp->wx[0], sp->wy[0], sp->wx[1], sp->wy[1]);
	if (!mono && Scr[screen].npixels > 2) {
		XSetForeground(dsp, Scr[screen].gc, Scr[screen].pixels[sp->pix]);
		if (++sp->pix >= Scr[screen].npixels)
			sp->pix = 0;
	}
	XDrawSegments(dsp, win, Scr[screen].gc, sp->segs, sp->beecount);
}

//-----------------------------------------------------------------------------

#include <qpushbt.h>
#include <qchkbox.h>
#include <qcolor.h>
#include <qmsgbox.h>
#include "kslider.h"

#include "swarm.h"

#include "swarm.moc"
#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"


// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kSwarmSaver *saver = NULL;

void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kSwarmSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	kSwarmSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("Swarm");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//-----------------------------------------------------------------------------

kSwarmSaver::kSwarmSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();

	colorContext = QColor::enterAllocContext();

	batchcount = maxLevels;

	initXLock( gc );
	initswarm( d );

	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kSwarmSaver::~kSwarmSaver()
{
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void kSwarmSaver::setSpeed( int spd )
{
	timer.stop();
	speed = MAXSPEED - spd;
	timer.start( speed );
}

void kSwarmSaver::setLevels( int l )
{
	batchcount = maxLevels = l;
	initswarm( d );
}

void kSwarmSaver::readSettings()
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

void kSwarmSaver::slotTimeout()
{
	drawswarm( d );
}

//-----------------------------------------------------------------------------

kSwarmSetup::kSwarmSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	readSettings();

	setCaption( glocale->translate("Setup KSwarm") );

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

	label = new QLabel( glocale->translate("Number of Bees:"), this );
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
	saver = new kSwarmSaver( preview->winId() );
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

void kSwarmSetup::readSettings()
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

void kSwarmSetup::slotSpeed( int num )
{
	speed = num;

	if ( saver )
		saver->setSpeed( speed );
}

void kSwarmSetup::slotLevels( int num )
{
	maxLevels = num;

	if ( saver )
		saver->setLevels( maxLevels );
}

void kSwarmSetup::slotOkPressed()
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

void kSwarmSetup::slotAbout()
{
	QMessageBox::message(glocale->translate("About Swarm"),
			     glocale->translate("Swarm\n\nCopyright (c) 1991 by Patrick J. Naughton\n\nPorted to kscreensave by Emanuel Pirker."),
			     glocale->translate("OK"));
}
