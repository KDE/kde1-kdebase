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

/* Simulation of a pair of quasi-gravitational fields, maybe sorta kinda
   a little like the strong and weak electromagnetic forces.  Derived from
   a Lispm screensaver by John Pezaris <pz@mit.edu>.  Mouse control and
   viscosity added by "Philip Edward Cutone, III" <pc2d+@andrew.cmu.edu>.

   John sez:

   The simulation started out as a purely accurate gravitational simulation,
   but, with constant simulation step size, I quickly realized the field being
   simulated while grossly gravitational was, in fact, non-conservative.  It
   also had the rather annoying behavior of dealing very badly with colliding
   orbs.  Therefore, I implemented a negative-gravity region (with two
   thresholds; as I read your code, you only implemented one) to prevent orbs
   from every coming too close together, and added a viscosity factor if the
   speed of any orb got too fast.  This provides a nice stable system with
   interesting behavior.

   I had experimented with a number of fields including the van der Waals
   force (very interesting orbiting behavior) and 1/r^3 gravity (not as
   interesting as 1/r^2).  An even normal viscosity (rather than the
   thresholded version to bleed excess energy) is also not interesting.
   The 1/r^2, -1/r^2, -10/r^2 thresholds proved not only robust but also
   interesting -- the orbs never collided and the threshold viscosity fixed
   the non-conservational problem.

   Philip sez:
   > An even normal viscosity (rather than the thresholded version to
   > bleed excess energy) is also not interesting.

   unless you make about 200 points.... set the viscosity to about .8
   and drag the mouse through it.   it makes a nice wave which travels
   through the field.

   And (always the troublemaker) Joe Keane <jgk@jgk.org> sez:

   Despite what John sez, the field being simulated is always conservative.
   The real problem is that it uses a simple hack, computing acceleration
   *based only on the starting position*, instead of a real differential
   equation solver.  Thus you'll always have energy coming out of nowhere,
   although it's most blatant when balls get close together.  If it were
   done right, you wouldn't need viscosity or artificial limits on how
   close the balls can get.
 */

#include <stdio.h>
#include <math.h>

//=============================================================

// xscreensaver includes:

#include "xlock.h"

extern "C" {
#include "xs_colors.h"
}

//================================================================

// Qt includes:

#include <qpushbt.h>
#include <qchkbox.h>
#include <qcolor.h>
#include <qmsgbox.h>
#include <qcombobox.h>

#include "kslider.h"
#include "attraction.h"

#include "attraction.moc"

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"


//================================================================

struct ball {
  double x, y;
  double vx, vy;
  double dx, dy;
  double mass;
  int size;
  int pixel_index;
  int hue;
};

static struct ball *balls;
static int npoints;
static int threshold;
static int attrdelay;
static int global_size;
static int segments;
static Bool glow_p;
static Bool orbit_p;

static Bool mono_p;

static XPoint *point_stack;
static int point_stack_size, point_stack_fp;
static XColor *colors;
static int ncolors;
static int fg_index;
static int color_shift;

/*flip mods for mouse interaction*/
static Bool mouse_p;
int mouse_x, mouse_y, mouse_mass, root_x, root_y;
static double viscosity;

static enum object_mode {
  ball_mode, line_mode, polygon_mode, spline_mode, spline_filled_mode,
  tail_mode
} mode;

static GC draw_gc, erase_gc;

#define MAX_SIZE 16

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static inline double frand(double m)
{
  // return ((double)random()) / RAND_MAX * m;
  // Commented out by David. RAND_MAX might not be accurate...
  return ((double)(random()%30000)/30000.0) * m;
}

static void
init_balls (Display *dpy, Window window, kAttractionSaver *kas)
{
  int i;
  XWindowAttributes xgwa;
  XGCValues gcv;
  int xlim, ylim, midx, midy, r, vx, vy;
  double th;
  Colormap cmap;
  XGetWindowAttributes (dpy, window, &xgwa);
  xlim = xgwa.width;
  ylim = xgwa.height;
  cmap = xgwa.colormap;
  midx = xlim/2;
  midy = ylim/2;
  //r = get_integer_resource ("radius", "Integer");
  r= 0;
  if (r <= 0 || r > min (xlim/2, ylim/2))
    r = min (xlim/2, ylim/2) - 50;
  //vx = get_integer_resource ("vx", "Integer");
  //vy = get_integer_resource ("vy", "Integer");
  vx= vy= 0;
  //npoints = get_integer_resource ("points", "Integer");
  npoints = kas->getNumber();
  if (npoints < 1)
    npoints = 3 + (random () % 5);
  balls = (struct ball *) malloc (npoints * sizeof (struct ball));
  //segments = get_integer_resource ("segments", "Integer");
  segments = 500;
  if (segments < 0) segments = 1;
  //threshold = get_integer_resource ("threshold", "Integer");
  threshold= 100;
  if (threshold < 0) threshold = 0;
  //attrdelay = get_integer_resource ("delay", "Integer");
  attrdelay= 10000;
  if (attrdelay < 0) attrdelay = 0;
  //global_size = get_integer_resource ("size", "Integer");
  global_size = 0;
  if (global_size < 0) global_size = 0;
  //glow_p = get_boolean_resource ("glow", "Boolean");
  glow_p= kas->getGlow();
  //orbit_p = get_boolean_resource ("orbit", "Boolean");
  orbit_p= false;
  //color_shift = get_integer_resource ("colorShift", "Integer");
  color_shift = 3;
  if (color_shift <= 0) color_shift = 5;

  /*flip mods for mouse interaction*/
  //mouse_p = get_boolean_resource ("mouse", "Boolean");
  mouse_p= false;
  //mouse_mass = get_integer_resource ("mouseSize", "Integer");
  mouse_mass= 10;
  mouse_mass =  mouse_mass *  mouse_mass * 10;

  //viscosity = get_float_resource ("viscosity", "Float");
  viscosity = 1;

  const char *mode_str = kas->getMode();
  if (! mode_str) mode = ball_mode;
  else if (!strcmp (mode_str, "Balls")) mode = ball_mode;
  else if (!strcmp (mode_str, "Lines")) mode = line_mode;
  else if (!strcmp (mode_str, "Polygons")) mode = polygon_mode;
  else if (!strcmp (mode_str, "Tails")) mode = tail_mode;
#if 0
  else if (!strcmp (mode_str, "splines")) mode = spline_mode;
  else if (!strcmp (mode_str, "filled-splines")) mode = spline_filled_mode;
#endif
  else
    mode= ball_mode;

  if (mode != ball_mode && mode != tail_mode) glow_p = False;
  
  if (mode == polygon_mode && npoints < 3)
    mode = line_mode;

  //ncolors = 200;
  ncolors = NUMCOLORS;
  if (ncolors < 2) ncolors = 2;
  if (ncolors <= 2) mono_p = True;
  colors = 0;

  if (!mono_p)
    {
      fg_index = 0;
      switch (mode)
	{
	case ball_mode:
	  if (glow_p)
	    {
	      int H = random() % 360;
	      double S1 = 0.25;
	      double S2 = 1.00;
	      double V = frand(0.25) + 0.75;
	      colors = (XColor *) malloc(sizeof(*colors) * (ncolors+1));
	      make_color_ramp (dpy, cmap, H, S1, V, H, S2, V, colors, &ncolors,
			       False, True, False);
	    }
	  else
	    {
	      ncolors = npoints;
	      colors = (XColor *) malloc(sizeof(*colors) * (ncolors+1));
	      make_random_colormap (dpy, xgwa.visual, cmap, colors, &ncolors,
				    True, True, False, True);
	    }
	  break;
	case line_mode:
	case polygon_mode:
	case spline_mode:
	case spline_filled_mode:
	case tail_mode:
	  colors = (XColor *) malloc(sizeof(*colors) * (ncolors+1));
	  make_smooth_colormap (dpy, xgwa.visual, cmap, colors, &ncolors,
				True, False, True);
	  break;
	default:
	  abort ();
	}
    }

  if (!mono_p && ncolors <= 2)
    {
      if (colors) free (colors);
      colors = 0;
      mono_p = True;
    }

  if (mode != ball_mode)
    {
      int size = (segments ? segments : 1);
      point_stack_size = size * (npoints + 1);
      point_stack = (XPoint *) calloc (point_stack_size, sizeof (XPoint));
      point_stack_fp = 0;
    }

  gcv.line_width = (mode == tail_mode
		    ? (global_size ? global_size : (MAX_SIZE * 2 / 3))
		    : 1);
  gcv.cap_style = (mode == tail_mode ? CapRound : CapButt);

  if (mono_p)
    //gcv.foreground = get_pixel_resource("foreground", "Foreground", dpy, cmap);
    gcv.foreground = WhitePixel (dpy, DefaultScreen (dpy));
  else
    gcv.foreground = colors[fg_index].pixel;
  draw_gc = XCreateGC (dpy, window, GCForeground|GCLineWidth|GCCapStyle, &gcv);

  //gcv.foreground = get_pixel_resource("background", "Background", dpy, cmap);
  gcv.foreground= BlackPixel (dpy, DefaultScreen (dpy));
  erase_gc = XCreateGC (dpy, window, GCForeground|GCLineWidth|GCCapStyle,&gcv);


#define rand_size() min (MAX_SIZE, 8 + (random () % (MAX_SIZE - 9)))

  if (orbit_p && !global_size)
    /* To orbit, all objects must be the same mass, or the math gets
       really hairy... */
    global_size = rand_size ();

  th = frand (M_PI+M_PI);
  for (i = 0; i < npoints; i++)
    {
      int new_size = (global_size ? global_size : rand_size ());
      balls [i].dx = 0;
      balls [i].dy = 0;
      balls [i].size = new_size;
      balls [i].mass = (new_size * new_size * 10);
      balls [i].x = midx + r * cos (i * ((M_PI+M_PI) / npoints) + th);
      balls [i].y = midy + r * sin (i * ((M_PI+M_PI) / npoints) + th);
      if (! orbit_p)
	{
	  balls [i].vx = vx ? vx : ((6.0 - (random () % 11)) / 8.0);
	  balls [i].vy = vy ? vy : ((6.0 - (random () % 11)) / 8.0);
	}
      if (mono_p || mode != ball_mode)
	balls [i].pixel_index = -1;
      else if (glow_p)
	balls [i].pixel_index = 0;
      else
	balls [i].pixel_index = random() % ncolors;
    }

  if (orbit_p)
    {
      double a = 0;
      double v;
      //double v_mult = get_float_resource ("vMult", "Float");
      //if (v_mult == 0.0) v_mult = 1.0;
      double v_mult = 1.0;

      for (i = 1; i < npoints; i++)
	{
          double _2ipi_n = (2 * i * M_PI / npoints);
	  double x = r * cos (_2ipi_n);
	  double y = r * sin (_2ipi_n);
	  double distx = r - x;
	  double dist2 = (distx * distx) + (y * y);
          double dist = sqrt (dist2);
          double a1 = ((balls[i].mass / dist2) *
                       ((dist < threshold) ? -1.0 : 1.0) *
                       (distx / dist));
	  a += a1;
	}
      if (a < 0.0)
	{
	  fprintf (stderr, "%s: domain error: forces on balls too great\n",
		   "kscreensaver");
	  exit (-1);
	}
      v = sqrt (a * r) * v_mult;
      for (i = 0; i < npoints; i++)
	{
	  double k = ((2 * i * M_PI / npoints) + th);
	  balls [i].vx = -v * sin (k);
	  balls [i].vy =  v * cos (k);
	}
    }

  if (mono_p) glow_p = False;

  //XClearWindow (dpy, window);
  //XSetForeground(dpy, gc, BlackPixel(dsp, screen));
  XFillRectangle(dpy, window, erase_gc, 0, 0, xlim, ylim);
}

static void
compute_force (int i, double *dx_ret, double *dy_ret)
{
  int j;
  double x_dist, y_dist, dist, dist2;
  *dx_ret = 0;
  *dy_ret = 0;
  for (j = 0; j < npoints; j++)
    {
      if (i == j) continue;
      x_dist = balls [j].x - balls [i].x;
      y_dist = balls [j].y - balls [i].y;
      dist2 = (x_dist * x_dist) + (y_dist * y_dist);
      dist = sqrt (dist2);
	      
      if (dist > 0.1) /* the balls are not overlapping */
	{
	  double new_acc = ((balls[j].mass / dist2) *
			    ((dist < threshold) ? -1.0 : 1.0));
	  double new_acc_dist = new_acc / dist;
	  *dx_ret += new_acc_dist * x_dist;
	  *dy_ret += new_acc_dist * y_dist;
	}
      else
	{		/* the balls are overlapping; move randomly */
	  *dx_ret += (frand (10.0) - 5.0);
	  *dy_ret += (frand (10.0) - 5.0);
	}
    }

  if (mouse_p)
    {
      x_dist = mouse_x - balls [i].x;
      y_dist = mouse_y - balls [i].y;
      dist2 = (x_dist * x_dist) + (y_dist * y_dist);
      dist = sqrt (dist2);
	
      if (dist > 0.1) /* the balls are not overlapping */
	{
	  double new_acc = ((mouse_mass / dist2) *
			    ((dist < threshold) ? -1.0 : 1.0));
	  double new_acc_dist = new_acc / dist;
	  *dx_ret += new_acc_dist * x_dist;
	  *dy_ret += new_acc_dist * y_dist;
	}
      else
	{		/* the balls are overlapping; move randomly */
	  *dx_ret += (frand (10.0) - 5.0);
	  *dy_ret += (frand (10.0) - 5.0);
	}
    }
}

static void
run_balls (Display *dpy, Window window)
{
  int last_point_stack_fp = point_stack_fp;
  static int tick = 500, xlim, ylim;
  static Colormap cmap;
  int i;

  /*flip mods for mouse interaction*/
  Window  root1, child1;
  unsigned int mask;
  if (mouse_p)
    {
      XQueryPointer(dpy, window, &root1, &child1,
		    &root_x, &root_y, &mouse_x, &mouse_y, &mask);
    }

  if (tick++ == 500)
    {
      XWindowAttributes xgwa;
      XGetWindowAttributes (dpy, window, &xgwa);
      tick = 0;
      xlim = xgwa.width;
      ylim = xgwa.height;
      cmap = xgwa.colormap;
    }

  /* compute the force of attraction/repulsion among all balls */
  for (i = 0; i < npoints; i++)
    compute_force (i, &balls[i].dx, &balls[i].dy);

  /* move the balls according to the forces now in effect */
  for (i = 0; i < npoints; i++)
    {
      double old_x = balls[i].x;
      double old_y = balls[i].y;
      double new_x, new_y;
      int size = balls[i].size;
      balls[i].vx += balls[i].dx;
      balls[i].vy += balls[i].dy;

      /* don't let them get too fast: impose a terminal velocity
         (actually, make the medium have friction) */
      if (balls[i].vx > 10)
	{
	  balls[i].vx *= 0.9;
	  balls[i].dx = 0;
	}
      else if (viscosity != 1)
	{
	  balls[i].vx *= viscosity;
	}

      if (balls[i].vy > 10)
	{
	  balls[i].vy *= 0.9;
	  balls[i].dy = 0;
	}
      else if (viscosity != 1)
	{
	  balls[i].vy *= viscosity;
	}

      balls[i].x += balls[i].vx;
      balls[i].y += balls[i].vy;

      /* bounce off the walls */
      if (balls[i].x >= (xlim - balls[i].size))
	{
	  balls[i].x = (xlim - balls[i].size - 1);
	  if (balls[i].vx > 0)
	    balls[i].vx = -balls[i].vx;
	}
      if (balls[i].y >= (ylim - balls[i].size))
	{
	  balls[i].y = (ylim - balls[i].size - 1);
	  if (balls[i].vy > 0)
	    balls[i].vy = -balls[i].vy;
	}
      if (balls[i].x <= 0)
	{
	  balls[i].x = 0;
	  if (balls[i].vx < 0)
	    balls[i].vx = -balls[i].vx;
	}
      if (balls[i].y <= 0)
	{
	  balls[i].y = 0;
	  if (balls[i].vy < 0)
	    balls[i].vy = -balls[i].vy;
	}

      new_x = balls[i].x;
      new_y = balls[i].y;

      if (!mono_p)
	{
	  if (mode == ball_mode)
	    {
	      if (glow_p)
		{
		  /* make color saturation be related to particle
		     acceleration. */
		  double limit = 0.5;
		  double s, fraction;
		  double vx = balls [i].dx;
		  double vy = balls [i].dy;
		  if (vx < 0) vx = -vx;
		  if (vy < 0) vy = -vy;
		  fraction = vx + vy;
		  if (fraction > limit) fraction = limit;

		  s = 1 - (fraction / limit);
		  balls[i].pixel_index = (int) (ncolors * s);
		}
	      XSetForeground (dpy, draw_gc,
			      colors[balls[i].pixel_index].pixel);
	    }
	}

      if (mode == ball_mode)
	{
	  XFillArc (dpy, window, erase_gc, (int) old_x, (int) old_y,
		    size, size, 0, 360*64);
	  XFillArc (dpy, window, draw_gc,  (int) new_x, (int) new_y,
		    size, size, 0, 360*64);
	}
      else
	{
	  point_stack [point_stack_fp].x = (short) new_x;
	  point_stack [point_stack_fp].y = (short) new_y;
	  point_stack_fp++;
	}
    }

  /* draw the lines or polygons after computing all points */
  if (mode != ball_mode)
    {
      point_stack [point_stack_fp].x = (short) balls [0].x; /* close the polygon */
      point_stack [point_stack_fp].y = (short) balls [0].y;
      point_stack_fp++;
      if (point_stack_fp == point_stack_size)
	point_stack_fp = 0;
      else if (point_stack_fp > point_stack_size) /* better be aligned */
	abort ();
      if (!mono_p)
	{
	  static int tick = 0;
	  if (tick++ == color_shift)
	    {
	      tick = 0;
	      fg_index = (fg_index + 1) % ncolors;
	      XSetForeground (dpy, draw_gc, colors[fg_index].pixel);
	    }
	}
    }

  switch (mode)
    {
    case ball_mode:
      break;
    case line_mode:
      if (segments > 0)
	XDrawLines (dpy, window, erase_gc, point_stack + point_stack_fp,
		    npoints + 1, CoordModeOrigin);
      XDrawLines (dpy, window, draw_gc, point_stack + last_point_stack_fp,
		  npoints + 1, CoordModeOrigin);
      break;
    case polygon_mode:
      if (segments > 0)
	XFillPolygon (dpy, window, erase_gc, point_stack + point_stack_fp,
		      npoints + 1, (npoints == 3 ? Convex : Complex),
		      CoordModeOrigin);
      XFillPolygon (dpy, window, draw_gc, point_stack + last_point_stack_fp,
		    npoints + 1, (npoints == 3 ? Convex : Complex),
		    CoordModeOrigin);
      break;
    case tail_mode:
      {
	for (i = 0; i < npoints; i++)
	  {
	    int index = point_stack_fp + i;
	    int next_index = (index + (npoints + 1)) % point_stack_size;
	    XDrawLine (dpy, window, erase_gc,
		       point_stack [index].x,
		       point_stack [index].y,
		       point_stack [next_index].x,
		       point_stack [next_index].y);

	    index = last_point_stack_fp + i;
	    next_index = (index - (npoints + 1)) % point_stack_size;
	    if (next_index < 0) next_index += point_stack_size;
	    if (point_stack [next_index].x == 0 &&
		point_stack [next_index].y == 0)
	      continue;
	    XDrawLine (dpy, window, draw_gc,
		       point_stack [index].x,
		       point_stack [index].y,
		       point_stack [next_index].x,
		       point_stack [next_index].y);
	  }
      }
      break;
/*
    case spline_mode:
    case spline_filled_mode:
      {
	static spline *s = 0;
	if (! s) s = make_spline (npoints);
	if (segments > 0)
	  {
	    for (i = 0; i < npoints; i++)
	      {
		s->control_x [i] = point_stack [point_stack_fp + i].x;
		s->control_y [i] = point_stack [point_stack_fp + i].y;
	      }
	    compute_closed_spline (s);
	    if (mode == spline_filled_mode)
	      XFillPolygon (dpy, window, erase_gc, s->points, s->n_points,
			    (s->n_points == 3 ? Convex : Complex),
			    CoordModeOrigin);
	    else
	      XDrawLines (dpy, window, erase_gc, s->points, s->n_points,
			  CoordModeOrigin);
	  }
	for (i = 0; i < npoints; i++)
	  {
	    s->control_x [i] = point_stack [last_point_stack_fp + i].x;
	    s->control_y [i] = point_stack [last_point_stack_fp + i].y;
	  }
	compute_closed_spline (s);
	if (mode == spline_filled_mode)
	  XFillPolygon (dpy, window, draw_gc, s->points, s->n_points,
			(s->n_points == 3 ? Convex : Complex),
			CoordModeOrigin);
	else
	  XDrawLines (dpy, window, draw_gc, s->points, s->n_points,
		      CoordModeOrigin);
      }
      break;
*/
    default:
      abort ();
    }

  // XSync (dpy, True);
}

//----------------------------------------------------------------------------

void attr_cleanup()
{
    if (balls) {
      free(balls);
      balls= 0;
    }
    if (colors) {
      free(colors);
      colors= 0;
    }
    if (point_stack) {
      free(point_stack);
      point_stack= 0;
    }
}

//----------------------------------------------------------------------------

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kAttractionSaver *saver = NULL;

void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kAttractionSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	kAttractionSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("Attraction");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//----------------------------------------------------------------------------

kAttractionSaver::kAttractionSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();

	colorContext = QColor::enterAllocContext();

	initXLock( gc );	// needed by all xlock ports
        init_balls (dsp, d, this);

	timer.start( 10, TRUE );		// single shot timer makes smoother animation
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kAttractionSaver::~kAttractionSaver()
{
	timer.stop();
	attr_cleanup();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void kAttractionSaver::setNumber( int num )
{
	attr_cleanup();
	number = num;
        init_balls (dsp, d, this);
}

void kAttractionSaver::setGlow( bool c )
{
	attr_cleanup();
	glow = c;
        init_balls (dsp, d, this);
}

void kAttractionSaver::setMode( const char *m)
{
	attr_cleanup();
	mode = m;
        init_balls (dsp, d, this);
}

void kAttractionSaver::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Number" );
	if ( !str.isNull() )
		number = atoi( str );
	else
		number = 15;

	str = config->readEntry( "Glow" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		glow = TRUE;
	else
		glow = FALSE;

	str = config->readEntry( "Mode" );
	if ( !str.isNull() )
		mode = str;
        else
		mode = "Balls";
}

void kAttractionSaver::slotTimeout()
{
        run_balls (dsp, d);
	timer.start( 10, TRUE );
}

//----------------------------------------------------------------------------

kAttractionSetup::kAttractionSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	number = 15;

	readSettings();
	setCaption( glocale->translate("Setup KAttraction") );

	QLabel *label;
	QPushButton *button;
	KSlider *slider;

	QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
	QHBoxLayout *tl1 = new QHBoxLayout;
	tl->addLayout(tl1);
	QVBoxLayout *tl11 = new QVBoxLayout(5);
	tl1->addLayout(tl11);

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
	saver = new kAttractionSaver( preview->winId() );
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

void kAttractionSetup::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Number" );
	if ( !str.isNull() )
		number = atoi( str );

	if ( number > 50 )
		number = 50;
	else if ( number < 5 )
		number = 15;

	str = config->readEntry( "Glow" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		glow = TRUE;
	else
		glow = FALSE;

	str = config->readEntry( "Mode" );
	if ( !str.isNull() )
		mode = str;
        else
		mode = "Balls";
}

void kAttractionSetup::slotNumber( int num )
{
	number = num;

	if ( saver )
		saver->setNumber( number );
}

void kAttractionSetup::slotGlow( bool c )
{
	glow = c;
	if ( saver )
		saver->setGlow( glow );
}

void kAttractionSetup::slotMode( const char * m)
{
	mode= m;
	if ( saver )
		saver->setMode( m );
}

void kAttractionSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString snumber;
	snumber.setNum( number );
	config->writeEntry( "Number", snumber );

	config->writeEntry( "Glow", glow ? "yes" : "no");

	config->writeEntry( "Mode", mode);

	config->sync();

	accept();
}

void kAttractionSetup::slotAbout()
{
	QMessageBox::message(glocale->translate("About Attraction"), 
			     glocale->translate("Attraction Version 1.0\n\n"
				"Copyright (c) 1992-1997 by Jamie Zawinski <jwz@jwz.org>\n\n"
				"Ported to kscreensaver by:\n\n"
				"Tom Vijlbrief <tom.vijlbrief@knoware.nl> oct 1998"),
			     glocale->translate("OK"));
}

