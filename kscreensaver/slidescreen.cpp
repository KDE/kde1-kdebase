/* xscreensaver, Copyright (c) 1992, 1995, 1996, 1997
 *  Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

//#include <stdio.h>
//#include <math.h>

//=============================================================

// xscreensaver includes:

#include "xlock.h"

extern "C" {

#include "xs_visual.h"

}

//================================================================

// Qt includes:

#include <qpushbt.h>
//#include <qchkbox.h>
#include <qcolor.h>
#include <qmsgbox.h>
//#include <qcombobox.h>

#include "kslider.h"
#include "slidescreen.h"

#include "slidescreen.moc"

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"


//================================================================

static int grid_size;
static int pix_inc;
static int hole_x, hole_y;
static int bitmap_w, bitmap_h;
static int xoff, yoff;
static int grid_w, grid_h;
static int ssdelay, ssdelay2;
static GC gc;
static int max_width, max_height;
static unsigned long fg, bg;
static int border;

static XImage *save;

void draw_slide(Display *dpy, Window window);

static void
init_slide (Display *dpy, Window window, kSlideScreenSaver *kss)
{
  int i;
  XWindowAttributes xgwa;
  Colormap cmap;
  Visual *visual;

  XGetWindowAttributes (dpy, window, &xgwa);
  //grab_screen_image (xgwa.screen, window);

#if 1
  // save screen
  Display *dsp = qt_xdisplay();
  Window rootwin = RootWindow( dsp, qt_xscreen() );
  save = XGetImage( dsp, rootwin, 0, 0, xgwa.width, xgwa.height, AllPlanes, ZPixmap);
#endif

  XGetWindowAttributes (dpy, window, &xgwa);  /* re-retrieve colormap */
  cmap = xgwa.colormap;
  visual = xgwa.visual;
  max_width = xgwa.width;
  max_height = xgwa.height;

  ssdelay = 50000;
  ssdelay2= 1000000;
  //grid_size= 70;
  grid_size= max_width/14;
  pix_inc= 10;
  border= grid_size > 20 ? 4: 1;

  {
    XColor fgc, bgc;
    //char *fgs = get_string_resource("background", "Background");
    //char *bgs = get_string_resource("foreground", "Foreground");
    Bool fg_ok, bg_ok;
    //if (!XParseColor (dpy, cmap, fgs, &fgc))
      XParseColor (dpy, cmap, "black", &bgc);
    //if (!XParseColor (dpy, cmap, bgs, &bgc))
      XParseColor (dpy, cmap, "gray", &fgc);

    fg_ok = XAllocColor (dpy, cmap, &fgc);
    bg_ok = XAllocColor (dpy, cmap, &bgc);

    /* If we weren't able to allocate the two colors we want from the
       colormap (which is likely if the screen has been grabbed on an
       8-bit SGI visual -- don't ask) then just go through the map
       and find the closest color to the ones we wanted, and use those
       pixels without actually allocating them.
     */
    if (fg_ok)
      fg = fgc.pixel;
    else
      fg = 0;

    if (bg_ok)
      bg = bgc.pixel;
    else
      bg = 1;

    if (!fg_ok || bg_ok)
      {
	unsigned long fgd = ~0;
	unsigned long bgd = ~0;
	int max = visual_cells (xgwa.screen, visual);
	XColor *all = (XColor *) calloc(sizeof (*all), max);
	for (i = 0; i < max; i++)
	  {
	    all[i].flags = DoRed|DoGreen|DoBlue;
	    all[i].pixel = i;
	  }
	XQueryColors (dpy, cmap, all, max);
	for(i = 0; i < max; i++)
	  {
	    long rd, gd, bd;
	    unsigned long d;
	    if (!fg_ok)
	      {
		rd = (all[i].red   >> 8) - (fgc.red   >> 8);
		gd = (all[i].green >> 8) - (fgc.green >> 8);
		bd = (all[i].blue  >> 8) - (fgc.blue  >> 8);
		if (rd < 0) rd = -rd;
		if (gd < 0) gd = -gd;
		if (bd < 0) bd = -bd;
		d = (rd << 1) + (gd << 2) + bd;
		if (d < fgd)
		  {
		    fgd = d;
		    fg = all[i].pixel;
		    if (d == 0)
		      fg_ok = True;
		  }
	      }

	    if (!bg_ok)
	      {
		rd = (all[i].red   >> 8) - (bgc.red   >> 8);
		gd = (all[i].green >> 8) - (bgc.green >> 8);
		bd = (all[i].blue  >> 8) - (bgc.blue  >> 8);
		if (rd < 0) rd = -rd;
		if (gd < 0) gd = -gd;
		if (bd < 0) bd = -bd;
		d = (rd << 1) + (gd << 2) + bd;
		if (d < bgd)
		  {
		    bgd = d;
		    bg = all[i].pixel;
		    if (d == 0)
		      bg_ok = True;
		  }
	      }

	    if (fg_ok && bg_ok)
	      break;
	  }
	XFree(all);
      }
  }


  if (ssdelay < 0) ssdelay = 0;
  if (ssdelay2 < 0) ssdelay2 = 0;
  if (pix_inc < 1) pix_inc = 1;
  if (grid_size < 1) grid_size = 1;
}

void draw_slide(Display *dpy, Window window)
{
  XGCValues gcv;
  XWindowAttributes xgwa;
  Drawable d;
  long gcflags;
  int i;

  gcv.foreground = fg;
  gcv.function = GXcopy;
  gcv.subwindow_mode = IncludeInferiors;
  gcflags = GCForeground |GCFunction;

#if 0
  if (use_subwindow_mode_p(xgwa.screen, window)) /* see grabscreen.c */
    gcflags |= GCSubwindowMode;
#endif
  gc = XCreateGC (dpy, window, gcflags, &gcv);

  XGetWindowAttributes (dpy, window, &xgwa);
  bitmap_w = xgwa.width;
  bitmap_h = xgwa.height;

  grid_w = bitmap_w / grid_size;
  grid_h = bitmap_h / grid_size;
  hole_x = random () % grid_w;
  hole_y = random () % grid_h;
  xoff = (bitmap_w - (grid_w * grid_size)) / 2;
  yoff = (bitmap_h - (grid_h * grid_size)) / 2;

  d = window;

  XPutImage(dpy, d, gc, save, 0, 0, 0, 0, max_width, max_height);

  if (border)
    {
      int half = border/2;
      int half2 = (border & 1 ? half+1 : half);
      XSetForeground(dpy, gc, bg);
      for (i = 0; i < bitmap_w; i += grid_size)
	{
	  int j;
	  for (j = 0; j < bitmap_h; j += grid_size)
	    XDrawRectangle (dpy, d, gc,
			    xoff+i+half2, yoff+j+half2,
			    grid_size-border-1, grid_size-border-1);
	}

      XSetForeground(dpy, gc, fg);
      for (i = 0; i <= bitmap_w; i += grid_size)
	XFillRectangle (dpy, d, gc, xoff+i-half, yoff, border, bitmap_h);
      for (i = 0; i <= bitmap_h; i += grid_size)
	XFillRectangle (dpy, d, gc, xoff, yoff+i-half, bitmap_w, border);
    }

  if (xoff)
    {
      XFillRectangle (dpy, d, gc, 0, 0, xoff, bitmap_h);
      XFillRectangle (dpy, d, gc, bitmap_w - xoff, 0, xoff, bitmap_h);
    }
  if (yoff)
    {
      XFillRectangle (dpy, d, gc, 0, 0, bitmap_w, yoff);
      XFillRectangle (dpy, d, gc, 0, bitmap_h - yoff, bitmap_w, yoff);
    }

  //XSync (dpy, True);
  XFlush (dpy);
  if (ssdelay2) usleep (ssdelay2 * 2);
 for (i = 0; i < grid_size; i += pix_inc)
   {
     XPoint points [3];
     points[0].x = xoff + grid_size * hole_x;
     points[0].y = yoff + grid_size * hole_y;
     points[1].x = points[0].x + grid_size;
     points[1].y = points[0].y;
     points[2].x = points[0].x;
     points[2].y = points[0].y + i;
     XFillPolygon (dpy, window, gc, points, 3, Convex, CoordModeOrigin);

     points[1].x = points[0].x;
     points[1].y = points[0].y + grid_size;
     points[2].x = points[0].x + i;
     points[2].y = points[0].y + grid_size;
     XFillPolygon (dpy, window, gc, points, 3, Convex, CoordModeOrigin);

     points[0].x = points[1].x + grid_size;
     points[0].y = points[1].y;
     points[2].x = points[0].x;
     points[2].y = points[0].y - i;
     XFillPolygon (dpy, window, gc, points, 3, Convex, CoordModeOrigin);

     points[1].x = points[0].x;
     points[1].y = points[0].y - grid_size;
     points[2].x = points[1].x - i;
     points[2].y = points[1].y;
     XFillPolygon (dpy, window, gc, points, 3, Convex, CoordModeOrigin);

     //XSync (dpy, True);
     XFlush (dpy);
     if (ssdelay) usleep (ssdelay);
   }

  XFillRectangle (dpy, window, gc,
		  xoff + grid_size * hole_x,
		  yoff + grid_size * hole_y,
		  grid_size, grid_size);
}

static void
slide1 (Display *dpy, Window window)
{
  /* this code is a total kludge, but who cares, it works... */
 int i, x, y, ix, iy, dx, dy, dir, w, h, size, inc;
 static int last = -1;
 do {
   dir = random () % 4;
   switch (dir)
     {
     case 0: dx = 0,  dy = 1;  break;
     case 1: dx = -1, dy = 0;  break;
     case 2: dx = 0,  dy = -1; break;
     case 3: dx = 1,  dy = 0;  break;
     default: abort ();
     }
 } while (dir == last ||
	  hole_x + dx < 0 || hole_x + dx >= grid_w ||
	  hole_y + dy < 0 || hole_y + dy >= grid_h);
 if (grid_w > 1 && grid_h > 1)
   last = (dir == 0 ? 2 : dir == 2 ? 0 : dir == 1 ? 3 : 1);

 switch (dir)
   {
   case 0: size = 1 + (random()%(grid_h - hole_y - 1)); h = size; w = 1; break;
   case 1: size = 1 + (random()%hole_x); 	        w = size; h = 1; break;
   case 2: size = 1 + (random()%hole_y);	        h = size; w = 1; break;
   case 3: size = 1 + (random()%(grid_w - hole_x - 1)); w = size; h = 1; break;
   default: abort ();
   }

 if (dx == -1) hole_x -= (size - 1);
 else if (dy == -1) hole_y -= (size - 1);

 ix = x = xoff + (hole_x + dx) * grid_size;
 iy = y = yoff + (hole_y + dy) * grid_size;
 inc = pix_inc;
 for (i = 0; i < grid_size; i += inc)
   {
     int fx, fy, tox, toy;
     if (inc + i > grid_size)
       inc = grid_size - i;
     tox = x - dx * inc;
     toy = y - dy * inc;

     fx = (x < 0 ? 0 : x > max_width  ? max_width  : x);
     fy = (y < 0 ? 0 : y > max_height ? max_height : y);
     tox = (tox < 0 ? 0 : tox > max_width  ? max_width  : tox);
     toy = (toy < 0 ? 0 : toy > max_height ? max_height : toy);

     XCopyArea (dpy, window, window, gc,
		fx, fy,
		grid_size * w, grid_size * h,
		tox, toy);

     x -= dx * inc;
     y -= dy * inc;
     switch (dir)
       {
       case 0: XFillRectangle (dpy, window, gc,
			       ix, y + grid_size * h, grid_size * w, iy - y);
	 break;
       case 1: XFillRectangle (dpy, window, gc, ix, iy, x - ix, grid_size * h);
	 break;
       case 2: XFillRectangle (dpy, window, gc, ix, iy, grid_size * w, y - iy);
	 break;
       case 3: XFillRectangle (dpy, window, gc,
			       x + grid_size * w, iy, ix - x, grid_size * h);
	 break;
       }

     //XSync (dpy, True);
     XFlush (dpy);
     if (ssdelay) usleep (ssdelay);
   }
 switch (dir)
   {
   case 0: hole_y += size; break;
   case 1: hole_x--; break;
   case 2: hole_y--; break;
   case 3: hole_x += size; break;
   }
}


//----------------------------------------------------------------------------

void slide_cleanup()
{
#if 1
	if( save )
		XDestroyImage( save );
#endif
}

//----------------------------------------------------------------------------

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kSlideScreenSaver *saver = NULL;

void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kSlideScreenSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	kSlideScreenSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("SlideScreen");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//----------------------------------------------------------------------------

kSlideScreenSaver::kSlideScreenSaver( Drawable drawable ) : kScreenSaver( drawable )
{
    restart = true;

	readSettings();

	colorContext = QColor::enterAllocContext();

    initXLock( this->gc );	// needed by all xlock ports
    init_slide (dsp, d, this);

	timer.start( 200, TRUE );		// single shot timer makes smoother animation
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kSlideScreenSaver::~kSlideScreenSaver()
{
	timer.stop();
	slide_cleanup();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void kSlideScreenSaver::expose(int x, int y, int width, int height)
{
    // This saver needs a complete restart when an area is exposed
    restart = true;
    timer.stop();
    timer.start( 200, TRUE );
}

void kSlideScreenSaver::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );
}

void kSlideScreenSaver::slotTimeout()
{
    if ( restart )
    {
        draw_slide(qt_xdisplay(), d);
        restart = false;
    }
    slide1 (dsp, d);
	timer.start( ssdelay2/1000, TRUE );
}

//----------------------------------------------------------------------------

kSlideScreenSetup::kSlideScreenSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	//number = 15;

	//readSettings();
	setCaption( glocale->translate("Setup KSlideScreen") );


	//QLabel *label;
	QPushButton *button;
	//KSlider *slider;

	QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
	QHBoxLayout *tl1 = new QHBoxLayout;
	tl->addLayout(tl1);
	QVBoxLayout *tl11 = new QVBoxLayout(5);
	tl1->addLayout(tl11);

#if 0
	label = new QLabel( glocale->translate("Number:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new KSlider( KSlider::Horizontal, this );
	slider->setMinimumSize( 90, 20 );
	slider->setRange( 5, 55 );
	slider->setSteps( 5, 10 );
	slider->setValue( number );
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotNumber( int ) ) );
	tl11->addWidget(slider);
	tl11->addSpacing(5);

	label = new QLabel( glocale->translate("Mode:"), this);
	min_size(label);
	tl11->addWidget(label, 0, 0);

	QComboBox *combo = new QComboBox( this );
	combo->insertItem(  glocale->translate("Balls"), 0 );
	combo->insertItem(  glocale->translate("Lines"), 1 );
	combo->insertItem(  glocale->translate("Polygons"), 2 );
	combo->insertItem(  glocale->translate("Tails"), 3 );
	if (strcmp(mode, "Lines") == 0)
	  combo->setCurrentItem( 1 );
	else if (strcmp(mode, "Polygons") == 0)
	  combo->setCurrentItem( 2 );
	else if (strcmp(mode, "Tails") == 0)
	  combo->setCurrentItem( 3 );
	min_width(combo);
	fixed_height(combo);
	tl11->addWidget(combo, 0, 1);
	connect( combo, SIGNAL( activated( const char * ) ),
			SLOT( slotMode( const char * ) ) );

	QCheckBox *cb = new QCheckBox( glocale->translate("Glow"), this );
	min_size(cb);
	cb->setChecked( glow );
	connect( cb, SIGNAL( toggled( bool ) ), SLOT( slotGlow( bool ) ) );
	tl11->addWidget(cb);
	tl11->addStretch(1);

	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new kSlideScreenSaver( preview->winId() );
	tl1->addWidget(preview);
#endif

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


void kSlideScreenSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	config->sync();

	accept();
}

void kSlideScreenSetup::slotAbout()
{
	QMessageBox::message(glocale->translate("About SlideScreen"), 
			     glocale->translate("SlideScreen Version 1.1\n\n"
				"Copyright (c) 1992-1997 by Jamie Zawinski <jwz@jwz.org>\n\n"
				"Ported to kscreensaver by:\n\n"
				"Tom Vijlbrief <tom.vijlbrief@knoware.nl> oct 1998"),
			     glocale->translate("OK"));
}

