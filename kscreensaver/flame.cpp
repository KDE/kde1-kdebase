
/*-
 * flame.c - recursive fractal cosmic flames.
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * 11-Aug-95: Got rid of polyominal since it was crashing xlock on some
 *            machines.
 * 01-Jun-95: This should look more like the original with some updates by
 *            Scott Graves.
 * 27-Jun-91: vary number of functions used.
 * 24-Jun-91: fixed portability problem with integer mod (%).
 * 06-Jun-91: Written. (received from Scott Graves, spot@cs.cmu.edu).
 */

// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>

#include <qlayout.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kbuttonbox.h>
#include "helpers.h"


#include "xlock.h"
#include <math.h>

#define MAXBATCH	10
#define MAXLEV		4
#define MAXKINDS	9

//ModeSpecOpt flameopts = {0, NULL, NULL, NULL};

typedef struct {
    double      f[2][3][MAXLEV];/* three non-homogeneous transforms */
    int         variation;
    int         max_levels;
    int         cur_level;
    int         snum;
    int         anum;
    int         width, height;
    int         num_points;
    int         total_points;
    int         pixcol;
    Window      win;
    XPoint      pts[MAXBATCH];
}           flamestruct;

static flamestruct flames[MAXSCREENS];

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

void
initflame(Window win)
{
    flamestruct *fs = &flames[screen];
    XWindowAttributes xwa;

    (void) XGetWindowAttributes(dsp, win, &xwa);
    fs->width = xwa.width;
    fs->height = xwa.height;

    fs->max_levels = batchcount;
    fs->win = win;

    XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
    XFillRectangle(dsp, win, Scr[screen].gc, 0, 0, fs->width, fs->height);

    if (!mono && Scr[screen].npixels > 2) {
	fs->pixcol = halfrandom(Scr[screen].npixels);
	XSetForeground(dsp, Scr[screen].gc, Scr[screen].pixels[fs->pixcol]);
    } else
	XSetForeground(dsp, Scr[screen].gc, WhitePixel(dsp, screen));

    fs->variation = LRAND() % MAXKINDS;

}

static      Bool
recurse(flamestruct *fs, double x, double y, int l)
{
    int         i;
    double      nx, ny;

    if (l == fs->max_levels) {
	fs->total_points++;
	if (fs->total_points > cycles)	/* how long each fractal runs */
	    return False;

	if (x > -1.0 && x < 1.0 && y > -1.0 && y < 1.0) {
	    fs->pts[fs->num_points].x = (int) ((fs->width / 2) * (x + 1.0));
	    fs->pts[fs->num_points].y = (int) ((fs->height / 2) * (y + 1.0));
	    fs->num_points++;
	    if (fs->num_points > MAXBATCH) {	/* point buffer size */
		XDrawPoints(dsp, fs->win, Scr[screen].gc, fs->pts,
			    fs->num_points, CoordModeOrigin);
		fs->num_points = 0;
	    }
	}
    } else {
	for (i = 0; i < fs->snum; i++) {
	    nx = fs->f[0][0][i] * x + fs->f[0][1][i] * y + fs->f[0][2][i];
	    ny = fs->f[1][0][i] * x + fs->f[1][1][i] * y + fs->f[1][2][i];
	    if (i < fs->anum) {
              switch (fs->variation) {
                case 0: /* sinusoidal */
		  nx = sin(nx);
		  ny = sin(ny);
                  break;
                case 1: /* complex */
                  {
                    double r2 = nx * nx + ny * ny + 1e-6;

                    nx = nx / r2;
                    ny = ny / r2;
                  }
                  break;
                case 2: /* bent */
                  if (nx < 0.0)
                    nx = nx * 2.0;
                  if (ny < 0.0)
                    ny = ny / 2.0;
                  break;
                case 3: /* swirl */
                  {
                    double r = (nx * nx + ny * ny);  /* times k here is fun */
                    double c1 = sin(r);
                    double c2 = cos(r);
                    double t = nx;
 
                    nx = c1 * nx - c2 * ny;
                    ny = c2 * t  + c1 * ny;
                  }
                  break;
                case 4: /* horseshoe */
                  {
                    double r = atan2(nx, ny);  /* times k here is fun */
                    double c1 = sin(r);
                    double c2 = cos(r);
                    double t = nx;
 
                    nx = c1 * nx - c2 * ny;
                    ny = c2 * t  + c1 * ny;
                  }
                  break;
                case 5: /* drape */
                  {
                    double t = atan2(nx, ny)/M_PI;

                    ny = sqrt(nx * nx + ny * ny) - 1.0;
                    nx = t;
                  }
                  break;
                case 6: /* broken */
                  if (nx >  1.0)
                    nx = nx - 1.0;
                  if (nx < -1.0)
                    nx = nx + 1.0;
                  if (ny >  1.0)
                    ny = ny - 1.0;
                  if (ny < -1.0)
                    ny = ny + 1.0;
                  break;
                case 7: /* spherical */
                  {
                    double r = 0.5 + sqrt(nx * nx + ny * ny + 1e-6);

                    nx = nx / r;
                    ny = ny / r; }
                    break;
                case 8: /*  */
                  nx = atan(nx)/M_PI_2;
                  ny = atan(ny)/M_PI_2;
                  break;
#if 0
/* core dumps on some machines, why not all? */
                case 9: /* complex sine */
                  {
                    double u = nx, v = ny;
                    double ev = exp(v);
                    double emv = exp(-v);
 
                    nx = (ev + emv) * sin(u) / 2.0;
                    ny = (ev - emv) * cos(u) / 2.0;
                  }
                  break;
                case 10: /* polynomial */
                  if (nx < 0)
                    nx = -nx * nx;
                  else
                    nx =  nx * nx;
                  if (ny < 0)
                    ny = -ny * ny;
                  else
                    ny =  ny * ny;
                  break;
#endif
                default:
		  nx = sin(nx);
		  ny = sin(ny);
              }
	    }
	    if (!recurse(fs, nx, ny, l + 1))
		return False;
	}
    }
    return True;
}


void
drawflame(Window win)
{
    flamestruct *fs = &flames[screen];

    int         i, j, k;
    static      int alt = 0;

    if (!(fs->cur_level++ % fs->max_levels)) {
	XClearWindow(dsp, fs->win);
	alt = !alt;
	SRAND( time(NULL) );
    } else {
	if (!mono && Scr[screen].npixels > 2) {
	    XSetForeground(dsp, Scr[screen].gc,
			   Scr[screen].pixels[fs->pixcol]);
	    if (--fs->pixcol < 0)
		fs->pixcol = Scr[screen].npixels - 1;
	}
    }

    /* number of functions */
    fs->snum = 2 + (fs->cur_level % (MAXLEV - 1));

    /* how many of them are of alternate form */
    if (alt)
	fs->anum = 0;
    else
	fs->anum = halfrandom(fs->snum) + 2;

    /* 6 coefs per function */
    for (k = 0; k < fs->snum; k++) {
	for (i = 0; i < 2; i++)
	    for (j = 0; j < 3; j++)
		fs->f[i][j][k] = ((double) (LRAND() & 1023) / 512.0 - 1.0);
    }
    fs->num_points = 0;
    fs->total_points = 0;
    (void) recurse(fs, 0.0, 0.0, 0);
    XDrawPoints(dsp, win, Scr[screen].gc,
		fs->pts, fs->num_points, CoordModeOrigin);
}

//-----------------------------------------------------------------------------

#include <qpushbt.h>
#include <qchkbox.h>
#include <qcolor.h>
#include <qmsgbox.h>
#include "kslider.h"

#include "flame.h"

#include "flame.moc"

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kFlameSaver *saver = NULL;

void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kFlameSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
	  delete saver; // fixed epirker, 12-Jul-97
	saver = NULL;
}

int setupScreenSaver()
{
	kFlameSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("Flame");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//-----------------------------------------------------------------------------

kFlameSaver::kFlameSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();

	colorContext = QColor::enterAllocContext();

	batchcount = maxLevels;
	cycles = numPoints;

	initXLock( gc );
	initflame( d );

	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kFlameSaver::~kFlameSaver()
{
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void kFlameSaver::setSpeed( int spd )
{
	timer.stop();
	speed = 100-spd;
	timer.start( speed );
}

void kFlameSaver::setLevels( int l )
{
	batchcount = maxLevels = l;
	initflame( d );
}

void kFlameSaver::setPoints( int p )
{
	cycles = numPoints = p;
	initflame( d );
}

void kFlameSaver::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = 100 - atoi( str );
	else
		speed = 50;

	str = config->readEntry( "MaxLevels" );
	if ( !str.isNull() )
		maxLevels = atoi( str );
	else
		maxLevels = 50;

	str = config->readEntry( "NumPoints" );
	if ( !str.isNull() )
		numPoints = atoi( str );
	else
		numPoints = 3000;
}

void kFlameSaver::slotTimeout()
{
	drawflame( d );
}

//-----------------------------------------------------------------------------

kFlameSetup::kFlameSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	speed = 50;

	readSettings();

	setCaption( glocale->translate("Setup KFlame") );

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
	tl11->addSpacing(5);

	label = new QLabel( glocale->translate("Max Levels:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new KSlider( KSlider::Horizontal, this );
	slider->setMinimumSize( 90, 20 );
	slider->setRange( 10, 110 );
	slider->setSteps( 25, 50 );
	slider->setValue( maxLevels );
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotLevels( int ) ) );
	tl11->addWidget(slider);
	tl11->addSpacing(5);


	label = new QLabel( glocale->translate("Num Points:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new KSlider( KSlider::Horizontal, this );
	slider->setMinimumSize( 90, 20 );
	slider->setRange( 1000, 11000 );
	slider->setSteps( 2500, 5000 );
	slider->setValue( numPoints );
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotPoints( int ) ) );
	tl11->addWidget(slider);
	tl11->addStretch(1);

	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new kFlameSaver( preview->winId() );
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

void kFlameSetup::readSettings()
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

	str = config->readEntry( "MaxLevels" );
	if ( !str.isNull() )
		maxLevels = atoi( str );
	else
		maxLevels = 50;

	str = config->readEntry( "NumPoints" );
	if ( !str.isNull() )
		numPoints = atoi( str );
	else
		numPoints = 5000;
}

void kFlameSetup::slotSpeed( int num )
{
	speed = num;

	if ( saver )
		saver->setSpeed( speed );
}

void kFlameSetup::slotLevels( int num )
{
	maxLevels = num;

	if ( saver )
		saver->setLevels( maxLevels );
}

void kFlameSetup::slotPoints( int num )
{
	numPoints = num;

	if ( saver )
		saver->setPoints( numPoints );
}

void kFlameSetup::slotOkPressed()
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

void kFlameSetup::slotAbout()
{
  QMessageBox::message(glocale->translate("About Flame"),
		       glocale->translate("Flame Version 3.3\n\nCopyright (c) 1991 by Patrick J. Naughton\n\nPorted to kscreensave by Martin Jones."), 
		       glocale->translate("OK"));
}


