/*-
 * lissie.c - The Lissajous worm by Alexander Jolk
 *            <ub9x@rz.uni-karlsruhe.de>
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * 01-May-96: written.
 */

/* Ported to kscreensave:
   July 1997, Emanuel Pirker <epirker@edu.uni-klu.ac.at>
   Last Revised: 10-Jul-97
   In case of problems contact me, not the original author!
*/
// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xlock.h"
#include <math.h>

#define MAXSPEED 100
#define MINSPEED 0
#define DEFSPEED 50
#define MINBATCH 1
#define MAXBATCH 101
#define DEFBATCH 1
#define MINCYCLES 0
#define MAXCYCLES 2000
#define DEFCYCLES 1000

#define Lissie(n)\
  if (lissie->xs[(n)] >= 0 && lissie->ys[(n)] >= 0 \
      && lissie->xs[(n)] <= gp->width && lissie->ys[(n)] <= gp->height) {\
    if (lissie->ri < 2)\
     XDrawPoint(dsp, win, Scr[screen].gc, lissie->xs[(n)], lissie->ys[(n)]);\
    else\
     XDrawArc(dsp, win, Scr[screen].gc,\
       lissie->xs[(n)] - lissie->ri / 2, lissie->ys[(n)] - lissie->ri / 2,\
       lissie->ri, lissie->ri,\
       0, 23040);\
   }

#define FLOATRAND(min,max)	((min)+(LRAND()/MAXRAND)*((max)-(min)))
#define INTRAND(min,max)     ((min)+(LRAND()%((max)-(min)+1)))

#define MINDT  0.01
#define MAXDT  0.15

#define MAXLISSIELEN  100
#define MINLISSIELEN  10


//ModeSpecOpt lissie_opts = {0, NULL, NULL, NULL};

typedef struct {
	double
	            tx, ty, dtx, dty;
	int
	            xi, yi, ri, rx, ry, len, pos, xs[MAXLISSIELEN], ys[MAXLISSIELEN];
	int         color;
} lissiestruct;

typedef struct {
	int         width, height;
	int         nlissies;
	lissiestruct lissie[MAXBATCH];
} lissstruct;

static lissstruct lisss[MAXSCREENS];

static int  loopcount;

static void initlissie(Window win, lissiestruct * lissie);
static void drawlissie(Window win, lissiestruct * lissie);

void
init_lissie(Window win)
{
	lissstruct *gp = &lisss[screen];
	unsigned char ball;

	XWindowAttributes xwa;

	(void) XGetWindowAttributes(dsp, win, &xwa);
	gp->width = xwa.width;
	gp->height = xwa.height;

	gp->nlissies = batchcount;
	if (gp->nlissies < 1)
		gp->nlissies = 1;
	else if (gp->nlissies > 1000)
		gp->nlissies = 1000;

	loopcount = 0;

	//if (!gp->lissie)    made a static structure, epirker 10-jul-97
	//	gp->lissie = (lissiestruct *) calloc(gp->nlissies, sizeof (lissiestruct));
       
	XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
	XFillRectangle(dsp, win, Scr[screen].gc, 0, 0, gp->width, gp->height);
	for (ball = 0; ball < (unsigned char) gp->nlissies; ball++)
		initlissie(win, &gp->lissie[ball]);

}

static void
initlissie(Window win, lissiestruct * lissie)
{
	lissstruct *gp = &lisss[screen];

	if (!mono && Scr[screen].npixels > 2)
		lissie->color = LRAND() % Scr[screen].npixels;
	else
		lissie->color = WhitePixel(dsp, screen);
	/* Initialize parameters */
	lissie->ri = INTRAND(0, min(gp->width, gp->height) / 8);
	lissie->xi = INTRAND(gp->width / 4 + lissie->ri,
			     gp->width * 3 / 4 - lissie->ri);
	lissie->yi = INTRAND(gp->height / 4 + lissie->ri,
			     gp->height * 3 / 4 - lissie->ri);
	lissie->rx = INTRAND(gp->width / 4,
		   min(gp->width - lissie->xi, lissie->xi)) - 2 * lissie->ri;
	lissie->ry = INTRAND(gp->height / 4,
		  min(gp->height - lissie->yi, lissie->yi)) - 2 * lissie->ri;
	lissie->len = INTRAND(MINLISSIELEN, MAXLISSIELEN - 1);
	lissie->pos = 0;

	lissie->tx = FLOATRAND(0, 2 * M_PI);
	lissie->ty = FLOATRAND(0, 2 * M_PI);
	lissie->dtx = FLOATRAND(MINDT, MAXDT);
	lissie->dty = FLOATRAND(MINDT, MAXDT);

	/* Draw lissie */
	drawlissie(win, lissie);
}

void
draw_lissie(Window win)
{
	lissstruct *gp = &lisss[screen];
	register unsigned char ball;

	if (++loopcount > cycles)
		init_lissie(win);
	else
		for (ball = 0; ball < (unsigned char) gp->nlissies; ball++)
			drawlissie(win, &gp->lissie[ball]);
}

static void
drawlissie(Window win, lissiestruct * lissie)
{
	lissstruct *gp = &lisss[screen];
	int         p = (++lissie->pos) % MAXLISSIELEN;
	int         oldp = (lissie->pos - lissie->len + MAXLISSIELEN) % MAXLISSIELEN;
	//printf("Lissie with %d batchcount and %d cycles.\n",batchcount,cycles); fflush(stdout);
	/* Let time go by ... */
	lissie->tx += lissie->dtx;
	lissie->ty += lissie->dty;
	if (lissie->tx > 2 * M_PI)
		lissie->tx -= 2 * M_PI;
	if (lissie->ty > 2 * M_PI)
		lissie->ty -= 2 * M_PI;

	/* slightly vary both (x/y) speeds for amusement */
	lissie->dtx += FLOATRAND(-MINDT / 5.0, MINDT / 5.0);
	lissie->dty += FLOATRAND(-MINDT / 5.0, MINDT / 5.0);
	if (lissie->dtx < MINDT)
		lissie->dtx = MINDT;
	else if (lissie->dtx > MAXDT)
		lissie->dtx = MAXDT;
	if (lissie->dty < MINDT)
		lissie->dty = MINDT;
	else if (lissie->dty > MAXDT)
		lissie->dty = MAXDT;

	lissie->xs[p] = (int)(lissie->xi + sin(lissie->tx) * lissie->rx);//(int) to get rid of
	lissie->ys[p] = (int)(lissie->yi + sin(lissie->ty) * lissie->ry);// warnings, epirker

	/* Mask */
	XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
	Lissie(oldp);

	/* Redraw */
	if (!mono && Scr[screen].npixels > 2) {
		XSetForeground(dsp, Scr[screen].gc, Scr[screen].pixels[lissie->color]);
		if (++lissie->color >= Scr[screen].npixels)
			lissie->color = 0;
	} else
		XSetForeground(dsp, Scr[screen].gc, WhitePixel(dsp, screen));
	Lissie(p);
}

//-----------------------------------------------------------------------------

#include <qpushbt.h>
#include <qchkbox.h>
#include <qcolor.h>
#include <qmsgbox.h>
#include "kslider.h"

#include "lissie.h"

#include "lissie.moc"

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"


// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kLissieSaver *saver = NULL;

void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kLissieSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
	  delete saver;   // FIX
	saver = NULL;
}

int setupScreenSaver()
{
	kLissieSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("Lissie");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//-----------------------------------------------------------------------------

kLissieSaver::kLissieSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();

	colorContext = QColor::enterAllocContext();

	batchcount = maxLevels;
	cycles = numPoints;
	
	initXLock( gc );
	init_lissie( d );

	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kLissieSaver::~kLissieSaver()
{
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void kLissieSaver::setSpeed( int spd )
{
	timer.stop();
	speed = MAXSPEED - spd;
	timer.start( speed );
}

void kLissieSaver::setLevels( int l )
{
  batchcount = maxLevels = l;
  init_lissie( d );
}

void kLissieSaver::setPoints( int p )
{
	cycles = numPoints = p;
	init_lissie( d );
}

void kLissieSaver::readSettings()
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

	str = config->readEntry( "NumPoints" );
	if ( !str.isNull() )
		numPoints = atoi( str );
	else
		numPoints = DEFCYCLES;
}

void kLissieSaver::slotTimeout()
{
	draw_lissie( d );
}

//-----------------------------------------------------------------------------

kLissieSetup::kLissieSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{

	readSettings();

	setCaption( glocale->translate("Setup KLissie") );

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
	tl11->addSpacing(10);

	label = new QLabel( glocale->translate("Num of Lissies:"), this );
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
	tl11->addSpacing(10);

	label = new QLabel( glocale->translate("Cycles:"), this );
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
	saver = new kLissieSaver( preview->winId() );
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

void kLissieSetup::readSettings()
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

	str = config->readEntry( "NumPoints" );
	if ( !str.isNull() )
		numPoints = atoi( str );
	else
		numPoints = DEFCYCLES;
}

void kLissieSetup::slotSpeed( int num )
{
	speed = num;

	if ( saver )
		saver->setSpeed( speed );
}

void kLissieSetup::slotLevels( int num )
{
	maxLevels = num;

	if ( saver )
		saver->setLevels( maxLevels );
}

void kLissieSetup::slotPoints( int num )
{
	numPoints = num;

	if ( saver )
		saver->setPoints( numPoints );
}

void kLissieSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString sspeed;
	sspeed.setNum( speed );
	config->writeEntry( "Speed", sspeed );

	QString slevels;
	slevels.setNum( maxLevels );
	config->writeEntry( "MaxLevels", slevels );

	QString spoints;
	spoints.setNum( numPoints );
	config->writeEntry( "NumPoints", spoints );

	config->sync();
	accept();
}

void kLissieSetup::slotAbout()
{
	QMessageBox::message(glocale->translate("About Lissie"), 
			     glocale->translate("Lissie\n\nCopyright (c) 1996 by Alexander Jolk\n\nPorted to kscreensave by Emanuel Pirker."), 
			     glocale->translate("OK"));
}


