//----------------------------------------------------------------------------
// This module contains code to interface original xlock savers to
// kscreensave
//

#include <qapp.h>
#include "xlock.h"


int screen;
Display *dsp;
perscreen Scr[MAXSCREENS];
int batchcount = 100;
int cycles = 100;
Bool mono = 0;
Bool allowroot = 0;
char *ProgramName;
/*
Dr. Park's algorithm published in the Oct. '88 ACM
"Random Number Generators: Good Ones Are Hard To Find"
His version available at ftp://cs.wm.edu/pub/rngs.tar
Present form by many authors.
*/

static int Seed = 1;       /* This is required to be 32 bits long */

/*
 *      Given an integer, this routine initializes the RNG seed.
 */
void SetRNG(long s)
{
	Seed = (int) s;
}

/*
 *      Returns an integer between 0 and 2147483647, inclusive.
 */
long LongRNG()
{
	if ((Seed = Seed % 44488 * 48271 - Seed / 44488 * 3399) < 0)
		Seed += 2147483647;
	return (long) (Seed - 1);
}

unsigned long
allocpixel(Colormap cmap, char *name, char *def)
{
	XColor      col;
	XColor      tmp;
	XParseColor(dsp, cmap, name, &col);
	if (!XAllocColor(dsp, cmap, &col))
	{
		fprintf(stderr, "couldn't allocate: %s, using %s instead\n", name, def);
		XAllocNamedColor(dsp, cmap, def, &col, &tmp);
	}

	return col.pixel;
}

void initXLock( GC gc )
{
	SetRNG( time(NULL) );

	dsp = qt_xdisplay();
	screen = qt_xscreen();

	Screen *scr = ScreenOfDisplay(dsp, screen);

	Scr[0].gc = gc;
	Scr[0].npixels = NUMCOLORS;
	Scr[0].cmap = None;

	Colormap    cmap = DefaultColormapOfScreen(scr);
	Scr[0].bgcol = allocpixel(cmap, "background", "White");
	Scr[0].bgcol = allocpixel(cmap, "foreground", "Black");

	QColor color;

	for ( int i = 0; i < NUMCOLORS; i++ )
	{
		color.setHsv( i * 360 / NUMCOLORS, 255, 255 );
		Scr[0].pixels[i] = color.alloc();
	}
}

