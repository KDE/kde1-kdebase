/*-
 * bat.c - A bouncing bat for xlock, the X Window System lockscreen.
 *
 * Copyright (c) 1988 by Sun Microsystems
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * 18-Sep-95: 5 bats now in color (patol@info.isbiel.ch)
 * 20-Sep-94: 5 bats instead of bouncing balls, based on bounce.c
 *            (patol@info.isbiel.ch)
 * 2-Sep-93: bounce version (David Bagley bagleyd@hertz.njit.edu)
 * 1986: Sun Microsystems
 */

/* 
 * original copyright
 * **************************************************************************
 * Copyright 1988 by Sun Microsystems, Inc. Mountain View, CA.
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Sun or MIT not be used in advertising
 * or publicity pertaining to distribution of the software without specific
 * prior written permission. Sun and M.I.T. make no representations about the
 * suitability of this software for any purpose. It is provided "as is"
 * without any express or implied warranty.
 *
 * SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL SUN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ***************************************************************************
 */

/* Ported to kscreensave:
   July 1997, Emanuel Pirker <epirker@edu.uni-klu.ac.at>
   Contact me if something doesn't work correctly!
   Last revised: 11-Jul-97
*/

// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>

#include "xlock.h"
#include <math.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#define MINSPEED 0
#define MAXSPEED 100
#define DEFSPEED 50
#define MINBATCH 0
#define MAXBATCH 20
#define DEFBATCH 5

#if HAVE_XPM
#if 1
#include <X11/xpm.h>
#else
#include <xpm.h>
#endif
#include "pixmaps/bat-0.xpm"
#include "pixmaps/bat-1.xpm"
#include "pixmaps/bat-2.xpm"
#include "pixmaps/bat-3.xpm"
#include "pixmaps/bat-4.xpm"
#endif

#include "bitmaps/bat-0.xbm"
#include "bitmaps/bat-1.xbm"
#include "bitmaps/bat-2.xbm"
#include "bitmaps/bat-3.xbm"
#include "bitmaps/bat-4.xbm"

#define MAX_STRENGTH 24
#define FRICTION 15
#define PENETRATION 0.4
#define SLIPAGE 4
#define TIME 32

#define ORIENTS 8
#define ORIENTCYCLE 32
#define CCW 1
#define CW (ORIENTS-1)
#define DIR(x)	(((x)>=0)?CCW:CW)
#define SIGN(x)	(((x)>=0)?1:-1)
#define ABS(x)	(((x)>=0)?x:-(x))

//ModeSpecOpt bat_opts = {0, NULL, NULL, NULL};

static XImage bimages[] =
{
	{0, 0, 0, XYBitmap, 0, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, 0, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, 0, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, 0, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, 0, LSBFirst, 8, LSBFirst, 8, 1}
};
static XImage *images[ORIENTS / 2 + 1];

typedef struct {
	int         x, y, xlast, ylast;
	int         spincount, spindelay, spindir, orient;
	int         vx, vy, vang;
	int         mass, size, sizex, sizey;
	unsigned long color;
} batstruct;

typedef struct {
	int         width, height;
	int         nbats;
	int         restartnum;
	batstruct   bats[MAXBATCH];
} bouncestruct;

static bouncestruct bounces[MAXSCREENS];

static void checkCollision(int a_bat);
static void drawabat(Window win, batstruct * bat);
static void movebat(batstruct * bat);
static void flapbat(batstruct * bat, int dir, int *vel);
static int  collide(int a_bat);
static void XEraseImage(Display * display, Window win, GC gc, int x, int y, int xlast, int ylast, int xsize, int ysize);

static      unsigned int first = 1;

static unsigned char *bits[] =
{
	bat0_bits, bat1_bits, bat2_bits, bat3_bits, bat4_bits
};

#if HAVE_XPM
static char **pixs[] =
{
	bat0, bat1, bat2, bat3, bat4
};

#endif

static void
init_images()
{
	int         i;

#if HAVE_XPM
	int         xpm_ret = 0;

	if (!mono && Scr[screen].npixels > 2)
		for (i = 0; i <= ORIENTS / 2; i++)
			xpm_ret += XpmCreateImageFromData(dsp, pixs[i], &(images[i]),
				   (XImage **) NULL, (XpmAttributes *) NULL);
	if (mono || Scr[screen].npixels <= 2 || xpm_ret != 0)
#endif
		for (i = 0; i <= ORIENTS / 2; i++) {
			bimages[i].data = (char *) bits[i];
			bimages[i].width = bat0_width;
			bimages[i].height = bat0_height;
			bimages[i].bytes_per_line = (bat0_width + 7) / 8;
			images[i] = &(bimages[i]);
		}
}

void
initbat(Window win)
{
	bouncestruct *bp = &bounces[screen];
	int         i;
	XWindowAttributes xwa;

	if (first) {
		init_images();
		first = 0;
	}
	XGetWindowAttributes(dsp, win, &xwa);
	bp->width = xwa.width;
	bp->height = xwa.height;
	bp->restartnum = TIME;

	bp->nbats = batchcount;
	if (bp->nbats < 1)
		bp->nbats = 1;
	//if (!bp->bats)
	//	bp->bats = (batstruct *) malloc(bp->nbats * sizeof (batstruct));
	i = 0;
	while (i < bp->nbats) {
		if (bat0_width > bp->width / 2 || bat0_height > bp->height / 2) {
			bp->bats[i].sizex = 7;
			bp->bats[i].sizey = 3;
			bp->bats[i].size = (bp->bats[i].sizex + bp->bats[i].sizey) / 2;
		} else {
			bp->bats[i].sizex = bat0_width;
			bp->bats[i].sizey = bat0_height;
			bp->bats[i].size = (bp->bats[i].sizex + bp->bats[i].sizey) / 2;
		}
		bp->bats[i].vx = ((LRAND() & 1) ? -1 : 1) * (LRAND() % MAX_STRENGTH + 1);
		bp->bats[i].x = (bp->bats[i].vx >= 0) ? 0 : bp->width - bp->bats[i].sizex;
		bp->bats[i].y = LRAND() % (bp->height / 2);
		if (i == collide(i)) {
			if (!mono && Scr[screen].npixels > 2)
				bp->bats[i].color = Scr[screen].pixels[LRAND() % Scr[screen].npixels];
			else
				bp->bats[i].color = WhitePixel(dsp, screen);
			bp->bats[i].xlast = -1;
			bp->bats[i].ylast = 0;
			bp->bats[i].spincount = 1;
			bp->bats[i].spindelay = 1;
			bp->bats[i].vy = ((LRAND() & 1) ? -1 : 1) * (LRAND() % MAX_STRENGTH);
			bp->bats[i].spindir = 0;
			bp->bats[i].vang = 0;
			bp->bats[i].orient = LRAND() % ORIENTS;
			i++;
		} else
			bp->nbats--;
	}
	XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
	XFillRectangle(dsp, win, Scr[screen].gc, 0, 0, bp->width, bp->height);
}

static void
checkCollision(int a_bat)
{
	bouncestruct *bp = &bounces[screen];
	int         i, amount, spin, d, size;
	double      x, y;

	for (i = 0; i < bp->nbats; i++) {
		if (i != a_bat) {
			x = (double) (bp->bats[i].x - bp->bats[a_bat].x);
			y = (double) (bp->bats[i].y - bp->bats[a_bat].y);
			d = (int) sqrt(x * x + y * y);
			size = (bp->bats[i].size + bp->bats[a_bat].size) / 2;
			if (d > 0 && d < size) {
				amount = size - d;
				if (amount > PENETRATION * size)
					amount = (int)(PENETRATION * size);
				bp->bats[i].vx += (int)(amount * x / d);
				bp->bats[i].vy += (int)(amount * y / d);
				bp->bats[i].vx -= bp->bats[i].vx / FRICTION;
				bp->bats[i].vy -= bp->bats[i].vy / FRICTION;
				bp->bats[a_bat].vx -= (int)(amount * x / d);
				bp->bats[a_bat].vy -= (int)(amount * y / d);
				bp->bats[a_bat].vx -= bp->bats[a_bat].vx / FRICTION;
				bp->bats[a_bat].vy -= bp->bats[a_bat].vy / FRICTION;
				spin = (bp->bats[i].vang - bp->bats[a_bat].vang) /
					(2 * size * SLIPAGE);
				bp->bats[i].vang -= spin;
				bp->bats[a_bat].vang += spin;
				bp->bats[i].spindir = DIR(bp->bats[i].vang);
				bp->bats[a_bat].spindir = DIR(bp->bats[a_bat].vang);
				if (!bp->bats[i].vang) {
					bp->bats[i].spindelay = 1;
					bp->bats[i].spindir = 0;
				} else
					bp->bats[i].spindelay = (int)(M_PI * bp->bats[i].size /
						(ABS(bp->bats[i].vang)) + 1);
				if (!bp->bats[a_bat].vang) {
					bp->bats[a_bat].spindelay = 1;
					bp->bats[a_bat].spindir = 0;
				} else
					bp->bats[a_bat].spindelay = (int)(M_PI * bp->bats[a_bat].size /
						(ABS(bp->bats[a_bat].vang)) + 1);
				return;
			}
		}
	}
}

void
drawbat(Window win)
{
	bouncestruct *bp = &bounces[screen];
	int         i;

	for (i = 0; i < bp->nbats; i++) {
		drawabat(win, &bp->bats[i]);
		movebat(&bp->bats[i]);
	}
	for (i = 0; i < bp->nbats; i++)
		checkCollision(i);
	if (!(LRAND() % TIME))	/* Put some randomness into the time */
		bp->restartnum--;
	if (!bp->restartnum)
		initbat(win);
}

static void
drawabat(Window win, batstruct * bat)
{
	if (bat->sizex < bat0_width) {
		if (bat->xlast != -1) {
			XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
			XFillRectangle(dsp, win, Scr[screen].gc,
			     bat->xlast, bat->ylast, bat->sizex, bat->sizey);
		}
		XSetForeground(dsp, Scr[screen].gc, bat->color);
		XFillRectangle(dsp, win, Scr[screen].gc,
			       bat->x, bat->y, bat->sizex, bat->sizey);
	} else {
		XSetForeground(dsp, Scr[screen].gc, bat->color);
		XPutImage(dsp, win, Scr[screen].gc,
			  images[(bat->orient > ORIENTS / 2) ? ORIENTS - bat->orient : bat->orient],
			  0, 0, bat->x, bat->y, bat->sizex, bat->sizey);
		if (bat->xlast != -1) {
			XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
			XEraseImage(dsp, win, Scr[screen].gc,
				    bat->x, bat->y, bat->xlast, bat->ylast, bat->sizex, bat->sizey);
		}
	}
}

static void
movebat(batstruct * bat)
{
	bouncestruct *bp = &bounces[screen];

	bat->xlast = bat->x;
	bat->ylast = bat->y;
	bat->x += bat->vx;
	if (bat->x > (bp->width - bat->sizex)) {
		/* Bounce off the right edge */
		bat->x = 2 * (bp->width - bat->sizex) - bat->x;
		bat->vx = -bat->vx + bat->vx / FRICTION;
		flapbat(bat, 1, &bat->vy);
	} else if (bat->x < 0) {
		/* Bounce off the left edge */
		bat->x = -bat->x;
		bat->vx = -bat->vx + bat->vx / FRICTION;
		flapbat(bat, -1, &bat->vy);
	}
	bat->vy++;
	bat->y += bat->vy;
	if (bat->y >= (bp->height + bat->sizey)) {	/* Don't see bat bounce */
		/* Bounce off the bottom edge */
		bat->y = (bp->height - bat->sizey);
		bat->vy = -bat->vy + bat->vy / FRICTION;
		flapbat(bat, -1, &bat->vx);
	}			/* else if (bat->y < 0) { */
	/* Bounce off the top edge */
	/*bat->y = -bat->y;
	   bat->vy = -bat->vy + bat->vy / FRICTION;
	   flapbat(bat, 1, &bat->vx);
	   } */
	if (bat->spindir) {
		bat->spincount--;
		if (!bat->spincount) {
			bat->orient = (bat->spindir + bat->orient) % ORIENTS;
			bat->spincount = bat->spindelay;
		}
	}
}

static void
flapbat(batstruct * bat, int dir, int *vel)
{
	*vel -= (int)((*vel + SIGN(*vel * dir) * bat->spindelay * ORIENTCYCLE /
		 (M_PI * bat->size)) / SLIPAGE);
	if (*vel) {
		bat->spindir = DIR(*vel * dir);
		bat->vang = *vel * ORIENTCYCLE;
		bat->spindelay = (int)(M_PI * bat->size / (ABS(bat->vang)) + 1);
	} else
		bat->spindir = 0;
}

static int
collide(int a_bat)
{
	bouncestruct *bp = &bounces[screen];
	int         i, d, x, y;

	for (i = 0; i < a_bat; i++) {
		x = (bp->bats[i].x - bp->bats[a_bat].x);
		y = (bp->bats[i].y - bp->bats[a_bat].y);
		d = (int) sqrt((double) (x * x + y * y));
		if (d < (bp->bats[i].size + bp->bats[a_bat].size) / 2)
			return i;
	}
	return i;
}

/* This stops some flashing, could be more efficient */
static void
XEraseImage(Display * display, Window win, GC gc, int x, int y, int xlast, int ylast, int xsize, int ysize)
{
	if (ylast < y) {
		if (y < ylast + ysize)
			XFillRectangle(display, win, gc, xlast, ylast, xsize, y - ylast);
		else
			XFillRectangle(display, win, gc, xlast, ylast, xsize, ysize);
	} else if (ylast > y) {
		if (y > ylast - ysize)
			XFillRectangle(display, win, gc, xlast, y + ysize, xsize, ylast - y);
		else
			XFillRectangle(display, win, gc, xlast, ylast, xsize, ysize);
	}
	if (xlast < x) {
		if (x < xlast + xsize)
			XFillRectangle(display, win, gc, xlast, ylast, x - xlast, ysize);
		else
			XFillRectangle(display, win, gc, xlast, ylast, xsize, ysize);
	} else if (xlast > x) {
		if (x > xlast - xsize)
			XFillRectangle(display, win, gc, x + xsize, ylast, xlast - x, ysize);
		else
			XFillRectangle(display, win, gc, xlast, ylast, xsize, ysize);
	}
}

// --------------------------------------------------------------------

#include <qpushbt.h>
#include <qchkbox.h>
#include <qcolor.h>
#include <qmsgbox.h>
#include <qlayout.h>
#include <kbuttonbox.h>
#include "kslider.h"
#include "helpers.h"

#include "bat.h"

#include "bat.moc"

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kBatSaver *saver = NULL;

void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kBatSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	kBatSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("Bat");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//-----------------------------------------------------------------------------

kBatSaver::kBatSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();

	colorContext = QColor::enterAllocContext();

	batchcount = maxLevels;

	initXLock( gc );
	initbat( d );

	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kBatSaver::~kBatSaver()
{
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void kBatSaver::setSpeed( int spd )
{
	timer.stop();
	speed = MAXSPEED - spd;
	timer.start( speed );
}

void kBatSaver::setLevels( int l )
{
	batchcount = maxLevels = l;
	initbat( d );
}

void kBatSaver::readSettings()
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

void kBatSaver::slotTimeout()
{
    drawbat( d );
}

//-----------------------------------------------------------------------------

kBatSetup::kBatSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	speed = 50;

	readSettings();

	setCaption( glocale->translate("Setup KBat") );

	QLabel *label;
	QPushButton *button;
	KSlider *slider;
	
	QVBoxLayout *tl = new QVBoxLayout(this, 10);
	QHBoxLayout *tl1 = new QHBoxLayout;
	tl->addLayout(tl1);

	QVBoxLayout *tl11 = new QVBoxLayout(5);
	tl1->addLayout(tl11);
	label = new QLabel( glocale->translate("Speed:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new KSlider( KSlider::Horizontal, this );
	slider->setFixedHeight(20);
	slider->setMinimumWidth(90);
	slider->setRange( MINSPEED, MAXSPEED );
	slider->setSteps( (MAXSPEED-MINSPEED)/4, (MAXSPEED-MINSPEED)/2 );
	slider->setValue( speed );
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotSpeed( int ) ) );
	tl11->addWidget(slider);
	tl11->addSpacing(15);

	label = new QLabel( glocale->translate("Number of bats:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new KSlider( KSlider::Horizontal, this );
	slider->setFixedHeight(20);
	slider->setMinimumWidth(90);
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
	saver = new kBatSaver( preview->winId() );
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

void kBatSetup::readSettings()
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

void kBatSetup::slotSpeed( int num )
{
	speed = num;

	if ( saver )
		saver->setSpeed( speed );
}

void kBatSetup::slotLevels( int num )
{
	maxLevels = num;

	if ( saver )
		saver->setLevels( maxLevels );
}

void kBatSetup::slotOkPressed()
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

void kBatSetup::slotAbout()
{
	QMessageBox::message(glocale->translate("About Bat"), 
			     glocale->translate("Bat\n\nCopyright (c) 1986 by Sun Microsystems\n\nPorted to kscreensave by Emanuel Pirker."), 
			     glocale->translate("OK"));
}


