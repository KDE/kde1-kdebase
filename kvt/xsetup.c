/*  Copyright 1992 John Bovey, University of Kent at Canterbury.
 *
 *  You can do what you like with this source code as long as
 *  you don't try to make money out of it and you include an
 *  unaltered copy of this message (including the copyright).
 */
/*
 * This module has heavily modifiedby R. Nation
 * (nation@rocket.sanders.lockheed.com).
 * No additional restrictions are applied
 *
 * Additional modifications by Garrett D'Amore (garrett@netcom.com).
 * No additional restrictions are applied.
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 */

/* LICENSE changed: all authors including but not limited to John Bovey
 * aggreed to distribute their code under the terms of the GPL
 */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include "rxvt.h"
#include "command.h"
#include "xsetup.h"
#include "screen.h"
#include "sbar.h"
#include "debug.h"
#include "kvt_version.h"

#include <config.h>

#ifdef HAVE_NCURSES_H
/* #include <ncurses.h> */
#else
/* #include <curses.h> */
#endif
#ifdef HAVE_SETUPTERM
#ifndef setupterm
/* this is a hack to avoid complaining about term.h include. Anybody wants
 * to hack configure, so it detects *where* term.h is and include it here?
 */
extern int setupterm(const char *,int,int *);
#endif
#endif

#ifdef GREEK_KBD	
#include "grkelot.h"
#endif

/* Stephan: a little bit hardcoded :) */
#if !defined PRINT_PIPE
#define PRINT_PIPE
#endif


extern WindowInfo MyWinInfo;

/* some functions for further options. Matthias */ 
/* needed for vt_size option */ 
extern void kvt_set_fontnum(char *);
/* needed for no_menubar option */ 
extern void kvt_set_menubar(int);
/* needed for no_scrollbar option */ 
extern void kvt_set_scrollbar(int);
/* needed for font switching */ 
extern void kvt_set_size_increment(int, int);

extern void clean_exit( int r );


#define XVT_CLASS	"XTerm"

#define VT_EVENTS	(	ExposureMask |\
				ButtonPressMask |\
				ButtonReleaseMask |\
				Button1MotionMask |\
				Button3MotionMask \
			)

#define MW_EVENTS	(	KeyPressMask |\
				FocusChangeMask |\
				StructureNotifyMask |\
			        VisibilityChangeMask \
			)

#define SB_EVENTS	(	ExposureMask |\
				Button2MotionMask |\
				Button1MotionMask |\
				Button3MotionMask |\
				ButtonReleaseMask |\
				ButtonPressMask \
			)
#define SB_ARROW_EVENTS (	ExposureMask |\
				EnterWindowMask|\
				LeaveWindowMask |\
				ButtonPressMask \
			)

/*  External global variables that are initialised at startup. */
Display		*display;
Window		vt_win=0;		/* vt100 window */
Window		main_win;	/* parent window */
Colormap	colormap;
XFontStruct	*mainfont;	/* main font structure */
int		mainfontfid;	/* main font FID */
GC 		gc;		/* GC for drawing text */
GC 		rvgc;		/* GC for drawing text */
unsigned long	foreground;	/* foreground pixel value */
unsigned long	background;	/* background pixel value */

struct sbar_info sbar;

char		*xvt_name;	/* the name the program is run under */
char		*window_name;	/* window name for titles etc. */
char		*icon_name;	/* name to display in the icon */
int		screen;		/* the X screen number */
char            *terminal;	/* contents of the TERM variable */
XColor		background_color;
XColor		foreground_color;
extern unsigned long pixel_colors[10];

char *fg_string_tmp = NULL;
char *bg_string_tmp = NULL;


extern KeySym SecureKeysym;
extern KeySym BigFontKeysym;
extern KeySym SmallFontKeysym;
extern KeySym PageUpKeysym;
extern KeySym PageDownKeysym;
#ifdef GREEK_KBD
extern KeySym GreekSwitchKeysym; 
extern int GreekMode;
#endif

static char *termlist[] = {
    "kvt-color",
    "xterm-color",
    "kvt",
    "xterm",
    "vt100",
    "ansi",
    "dump",
    0
};

XSizeHints sizehints = {
	PMinSize | PResizeInc | PBaseSize | PWinGravity,
	0, 0, 80, 24,	/* x, y, width and height */
	1, 1,		/* Min width and height */
	0, 0,		/* Max width and height */
	1, 1,		/* Width and height increments */
	{0, 0}, {0, 0},	/* Aspect ratio - not used */
	2 * MARGIN, 2 * MARGIN,		/* base size */
	NorthWestGravity                /* gravity */
};

int console = 0;

extern unsigned char mask;
int MetaHandling = ESCAPE;
int login_shell = 0;

char *display_name = NULL;
char *geom_string = "80x24";

#ifdef PRINT_PIPE
char *print_pipe = "lpr";
#endif

char *reg_fonts[NUM_FONTS]=
{"6x13", "5x7", "6x10", "7x13", "9x15", "10x20"};
int reg_font_handles[NUM_FONTS];

int font_num = DEFAULT_FONT;

/* cursor global, since it is needed several times below. (Matthias). */ 
Cursor cursor = 0;
Cursor mcursor = 0;	/* mouse accept cursor */
int cursor_mouse = 0;	/* use mouse cursor */

#define NORMAL_CURSOR	XC_xterm
#define MOUSE_CURSOR	XC_left_ptr



extern char *command;
int fat_sbar;
char iconic = 0;
#ifdef MAPALERT
int map_alert = 0 ;
#endif

static void create_xwindow(int,char **);
static void extract_fonts_and_geometry(char *,int,char *);
void extract_colors(char *, char *);
void extract_resources(void);

XErrorHandler RxvtErrorHandler(Display *, XErrorEvent *);


void set_geom_string(char *string)
{
  geom_string = string;
}

/*  Open the display, initialise the rDB resources database and create the
 *  window.  If title is non null then it is used as the window and icon title.
 *  iargc and iargv are the original argc, argv so the can be written to a
 *  COMMAND resource.
 */
int init_display(int argc,char **argv)
{
  XGCValues gcv;
  int i, len;
  char *display_string;
  char *terminal = NULL;
  display_name = getenv("DISPLAY");

  screen = DefaultScreen(display);
  colormap = DefaultColormap(display, screen);

  extract_resources();

  /* now get all the options */
  for(i=1;i<argc;i++) {
    if((strcmp(argv[i],"-vt_geometry")==0)&&(i+1<argc)) {
      geom_string = argv[++i];
#ifdef PRINT_PIPE
    } else if((strcmp(argv[i],"-print-pipe")==0)&&(i+1<argc)) {
      print_pipe = argv[++i];
#endif
    } else if((strcmp(argv[i],"-vt_fg")==0)&&(i+1<argc)) {
      fg_string_tmp = argv[++i];
    } else if((strcmp(argv[i],"-vt_bg")==0)&&(i+1<argc)) {
      bg_string_tmp = argv[++i];
    } else if((strcmp(argv[i],"-vt_font")==0)&&(i+1<argc)) {
      reg_fonts[DEFAULT_FONT] = argv[++i];
      reg_font_handles[DEFAULT_FONT] = 0;
    } else if((strcmp(argv[i],"-vt_size")==0)&&(i+1<argc)) {
      kvt_set_fontnum (argv[++i]);
    } else if(strcmp(argv[i],"-no_menubar")==0) {
      kvt_set_menubar(0);
    } else if(strcmp(argv[i],"-no_scrollbar")==0) {
      kvt_set_scrollbar(0);
    } else if(strcmp(argv[i], "-linux")==0) {
      fg_string_tmp = "#b2b2b2";
      bg_string_tmp = "#000000";
      init_color_mode(COLOR_TYPE_Linux);
    } else if(strcmp(argv[i],"-C")==0) {
      console = 1;
    } else if((strcmp(argv[i],"-T")==0)&&(i+1<argc)) {
      window_name = argv[++i]; 
    } else if((strcmp(argv[i],"-tn")==0)&&(i+1<argc)) {
      ++i; 
      terminal = safemalloc(strlen(argv[i])+6, "terminal");
      sprintf(terminal, "TERM=%s", argv[i]);
      putenv(terminal);
    } else if((strcmp(argv[i],"-n")==0)&&(i+1<argc)) {
      icon_name = argv[++i]; 
    } else if(strcmp(argv[i],"-7")==0) {
      mask = 0x7f;
    } else if(strcmp(argv[i],"-8")==0) {
      mask = 0xff;
    } else if(strcmp(argv[i],"-ls")==0) {
      login_shell = 1;
    } else if(strcmp(argv[i],"-ls-")==0) {
      login_shell = 0;
    } else if((strncmp(argv[i],"-meta",5)==0) &&(i+1<argc)) {
	if(strcasecmp(argv[i+1],"escape")==0) {
	    MetaHandling = ESCAPE;
	} else if(strcasecmp(argv[i+1],"8thbit")==0) {
	    MetaHandling = BIT;
	} else {
	    MetaHandling = 0;
	}
	i++;
#ifdef GREEK_KBD
    } else if (strcmp(argv[i],"-grk9")==0) {
      GreekMode = GREEK_ELOT928;
    } else if (strcmp(argv[i],"-grk4")==0) {
      GreekMode = GREEK_IBM437;
#endif       
    } else if((strcmp(argv[i],"-pageup")==0)&&(i+1<argc)) {
      PageUpKeysym = XStringToKeysym(argv[++i]);	
    } else if((strcmp(argv[i],"-pagedown")==0)&&(i+1<argc)) {
      PageDownKeysym = XStringToKeysym(argv[++i]);
    } else if((strcmp(argv[i],"-sl")==0)&&(i+1<argc)) {
      sscanf(argv[++i],"%d\n",&MyWinInfo.saved_lines);
      if(MyWinInfo.saved_lines < 0 ) {
	MyWinInfo.saved_lines = 0;
      }
    } else if (strcmp(argv[i],"-no_font_hack")==0) {
	/* turn off font hack - done in main */
    } else {
      return(0);
    }
  }
  
  /* if no terminal has been set by -tn option, set one */ 
  if (!terminal) {
# ifdef HAVE_SETUPTERM
    char **ptr = termlist;
    int i;
    while(*ptr) {
    	if ((setupterm(*ptr, 1, &i)) != -1 && (i == 1)) {
	    terminal = safemalloc(strlen(*ptr)+6, "terminal");
	    sprintf(terminal, "TERM=%s", *ptr);
	    break;
	}
	ptr++;
    }
# else
    terminal = safemalloc(11, "terminal");
    sprintf(terminal, "TERM=xterm");
# endif
    putenv(terminal);
  }

  /* to make sure that the GCs are not changed */ 
  gc = 0;
  rvgc = 0;

  /* changed DEFAULT_FONT to font_num. (Matthias) */ 
  extract_fonts_and_geometry(reg_fonts[font_num], reg_font_handles[font_num],
	geom_string);
  extract_colors(fg_string_tmp, bg_string_tmp);

  create_xwindow(argc,argv);
  
  /*  Create the graphics contexts.
   */
  gcv.foreground = background;
  gcv.background = foreground;
  gcv.font = mainfont->fid;
  rvgc = XCreateGC(display,main_win,GCForeground|GCBackground|GCFont,&gcv);
  gcv.foreground = foreground;
  gcv.background = background;
  gc = XCreateGC(display,main_win,GCForeground|GCBackground|GCFont,&gcv);
  

  /*  Add a DISPLAY entry to the environment, incase we were started
   * with rxvt -display term:0.0
   */
  len = strlen(XDisplayString(display));
  display_string = safemalloc(len+10,"display_string");
  sprintf(display_string,"DISPLAY=%s",XDisplayString(display));
  putenv(display_string);

  /*  initialise the screen data structures.
   */
  screen_init();
  sbar_init();

  XSetErrorHandler((XErrorHandler)RxvtErrorHandler);
  return(1);
}

void adjust_cursor(int mode)
{
	if (mode) {
		cursor_mouse = 1;
		XDefineCursor(display,vt_win,mcursor);
	} else {
		cursor_mouse = 0;
		XDefineCursor(display,vt_win,cursor);
	}
}

/*  Extract the resource fields that are needed to open the window.
 */

/* new functions since this stuff must be setable from Qt. (Matthias) */ 
void extract_colors( char *fg_string, char *bg_string){
  /*  Do the foreground, and background colors.
   */
  XGCValues gcv;
  XSetWindowAttributes winattr;
  int baw = 0;
  
  /* changed this error handling to black-on-white defaults. Matthias. */ 

  if (XParseColor(display,colormap,fg_string,&foreground_color) == 0){
    error("invalid foreground color %s.",fg_string);
    baw = 1;
  }

  if (!baw && XParseColor(display,colormap,bg_string,&background_color) == 0){
    error("invalid background color %s.",bg_string);
    baw = 1;
  }

  if (!baw && XAllocColor(display,colormap,&foreground_color) == 0){
    error("can't allocate color %s.",fg_string);
    baw = 1;
  }
  else
    foreground = foreground_color.pixel;

  if (!baw && XAllocColor(display,colormap,&background_color) == 0){
    error("can't allocate color %s.",bg_string);
    baw = 1;
  }
  else
    background = background_color.pixel;

  if (baw){
    /* black-on-white default */ 
    error ("will use black-on-white instead\n");
    foreground = XBlackPixel(display, screen);
    background = XWhitePixel(display, screen);
  }
  


  pixel_colors[0] = foreground;
  pixel_colors[1] = background;

  /* take care about the cursor, too. (Matthias) */ 
  if (cursor){
    /* I have to create a new cursor here. Otherwise the
       recolering will need a window-leave/window-enter to
       become visible ... (Matthias) */
    cursor = XCreateFontCursor(display,NORMAL_CURSOR);
    XRecolorCursor(display,cursor,&foreground_color,&background_color);
    if (!cursor_mouse) XDefineCursor(display,vt_win,cursor);
  }
  if (mcursor){
    /* I have to create a new cursor here. Otherwise the
       recolering will need a window-leave/window-enter to
       become visible ... (Matthias) */
    mcursor = XCreateFontCursor(display,MOUSE_CURSOR);
    XRecolorCursor(display,mcursor,&foreground_color,&background_color);
    if (cursor_mouse) XDefineCursor(display,vt_win,mcursor);
  }

  /* change the GCs to the new colors. (Matthias) */
  if (gc && rvgc){
    gcv.foreground = background;
    gcv.background = foreground;
    XChangeGC(display,rvgc,GCForeground|GCBackground,&gcv);
    gcv.foreground = foreground;
    gcv.background = background;
    XChangeGC(display,gc,GCForeground|GCBackground,&gcv);
    
    /* don't forget the window attributres (Matthias) */ 
    winattr.background_pixel = background;
    XChangeWindowAttributes(display, vt_win, CWBackPixel, &winattr);
  }
}



static void extract_fonts_and_geometry(char *font_string, int font_id,
	char *geom_string)
{
  int x, y, width, height;
  int flags;

  /*  First get the font since we need it to set the size.
   */
  if (font_id) {
	mainfont = XQueryFont(display, font_id);
	mainfontfid = 1;
  } else {
	if ((mainfont = XLoadQueryFont(display,font_string)) == NULL) 
	    {
	      error("can't access font %s\n",font_string);
              
              if ((mainfont = XLoadQueryFont(display,"fixed")) == NULL) 
	          error("can't access fixed font\n");
              
              clean_exit(1);
             }

	mainfontfid = 0;
  }


/*   This is an rxvt bug. (Matthias ) */
/*   sizehints.width_inc = XTextWidth(mainfont,"M",1); */
  /* better: */ 
  sizehints.width_inc = mainfont->max_bounds.width;

  sizehints.height_inc = mainfont->ascent + mainfont->descent;

  /* use the size increments for the kvt-widget, too.
    (Matthias) */ 
  kvt_set_size_increment(sizehints.width_inc, sizehints.height_inc);


  flags = XParseGeometry(geom_string,&x,&y,&width,&height);
  if (flags & WidthValue) 
    {
      sizehints.width = width;
      sizehints.flags |= USSize;
    }
  if (flags & HeightValue) 
    {
      sizehints.height = height;
      sizehints.flags |= USSize;
    }

  MyWinInfo.fheight = sizehints.height_inc;
  MyWinInfo.fwidth = sizehints.width_inc;
  MyWinInfo.cwidth = sizehints.width;
  MyWinInfo.cheight = sizehints.height;
  MyWinInfo.pwidth = MyWinInfo.cwidth*MyWinInfo.fwidth;
  MyWinInfo.pheight = MyWinInfo.cheight*MyWinInfo.fheight;

  sizehints.width = sizehints.width * sizehints.width_inc +
    sizehints.base_width;
  sizehints.height = sizehints.height * sizehints.height_inc + 
    sizehints.base_height;
  sizehints.min_width = sizehints.width_inc + sizehints.base_width;
  sizehints.min_height = sizehints.height_inc + sizehints.base_height;
  if (flags & XValue) 
    {
      if (flags & XNegative)
	{
	  x = DisplayWidth(display,screen) + x - sizehints.width - 2;
	  sizehints.win_gravity = NorthEastGravity;
	}
      sizehints.x = x;
      sizehints.flags |= USPosition;
    }
  if (flags & YValue) 
    {
      if (flags & YNegative)
	{
	  y = DisplayHeight(display,screen) + y - sizehints.height - 2;
	  sizehints.win_gravity = SouthWestGravity;
	  if((flags&XValue)&&(flags&XNegative))
	    sizehints.win_gravity = SouthEastGravity;	
	}
      sizehints.y = y;
      sizehints.flags |= USPosition;
    }
}


/*  Open and map the window.
 */
XClassHint class;
static void create_xwindow(int argc,char **argv)
{
  XWMHints wmhints;
  
  /* cursor is now defined on top of this file. (Matthias) */
/*   Cursor cursor; */

  /* no longer needed. Matthias */   
/*   main_win = XCreateSimpleWindow(display,DefaultRootWindow(display), */
/* 				 sizehints.x,sizehints.y, */
/* 				 sizehints.width,sizehints.height, */
  /* 				 1,foreground,background); */
/*  XResizeWindow(display, main_win, sizehints.width, sizehints.height+3); */
  
  change_window_name(window_name);
  change_icon_name(icon_name);
  class.res_name = xvt_name;
  class.res_class = XVT_CLASS;
  wmhints.input = True;

   if(iconic) 
     wmhints.initial_state = IconicState; 
   else 
     wmhints.initial_state = NormalState; 
   wmhints.flags = InputHint | StateHint; 
   /* no longer needed. Matthias */ 
/*    XSetWMProperties(display,main_win,NULL,NULL,argv,argc,   */
/* 		    &sizehints,&wmhints,&class);   */


  XSelectInput(display,main_win,MW_EVENTS);
  
  /* we do no longer need a special vt_win. Matthias */ 
  /* vt_win = main_win; */ 
   vt_win = XCreateSimpleWindow(display,main_win,0,0, 
 			       sizehints.width, 
 			       sizehints.height,0,foreground,background); 
  cursor = XCreateFontCursor(display,NORMAL_CURSOR);
  mcursor = XCreateFontCursor(display,MOUSE_CURSOR);
  XRecolorCursor(display,cursor,&foreground_color,&background_color);
  XRecolorCursor(display,mcursor,&foreground_color,&background_color);
  if (cursor_mouse) XDefineCursor(display,vt_win,mcursor);
  else XDefineCursor(display,vt_win,cursor);
  XSelectInput(display,vt_win,VT_EVENTS);
  
  XMapWindow(display,vt_win);
}

/*  Redraw the whole window after an exposure or size change.
 */
void resize_window(int width, int height)
{
  Window root;
  int x, y;
  extern int current_screen;
  static Bool first = True;
  unsigned int nw,nh, border_width, depth,prev_screen;
  XEvent dummy;
  static int old_width=-1,old_height=-1;
  if(width ==0)
    {
      while (XCheckTypedWindowEvent (display, main_win, ConfigureNotify, &dummy)); 
      XGetGeometry(display,main_win,&root,&x,&y,&width,&height,&border_width,
		   &depth);
    }
  nw = (width - 2*MARGIN )/MyWinInfo.fwidth;
  nh = (height - 2*MARGIN )/MyWinInfo.fheight;

  if((first)||(nw != MyWinInfo.cwidth)||(nh != MyWinInfo.cheight)||
     (old_width != width)||(old_height!=height))
    {
      old_width = width;
      old_height = height;
      prev_screen = current_screen;
      /* scrn_reset will only work if I'm on the low screen! */
      if(!first)
	{
	  scr_clear_selection();
	  scr_change_screen(LOW);
	}

      MyWinInfo.cheight = nh;
      MyWinInfo.cwidth = nw;
      MyWinInfo.pwidth = MyWinInfo.cwidth*MyWinInfo.fwidth;
      MyWinInfo.pheight = MyWinInfo.cheight*MyWinInfo.fheight;
      
      XResizeWindow(display,vt_win,width,height);
      XClearWindow(display,vt_win);
      XSync(display,0);  
      scrn_reset();
      if(!first)
	scr_change_screen(prev_screen);
    }
  first = False;
}

/* yet defined in main.C   (Matthias) */ 
/*  Change the window name displayed in the title bar. */
/* */
/* void change_window_name(char *str) */
/* { */
/*   XTextProperty name; */

/*     if (XStringListToTextProperty(&str,1,&name) == 0)  */
/*     { */
/*       error("cannot allocate window name"); */
/*       return; */
/*     } */
/*   XSetWMName(display,main_win,&name); */
/*   XFree(name.value); */
/* } */

/* yet defined in main.C   (Matthias) */ 
/* Change the icon name. */
/*  */
/* void */
/* change_icon_name(str) */
/* char *str; */
/* { */
/*   XTextProperty name; */
  
/*   if (XStringListToTextProperty(&str,1,&name) == 0)  */
/*     { */
/*       error("cannot allocate icon name"); */
/*       return; */
/*     } */
/*   XSetWMIconName(display,main_win,&name); */
/*   XFree(name.value); */
/* } */

/*  Print an error message.
 */
void error(char *fmt,...)
{
  va_list args;
  
  va_start(args,fmt);

  fprintf(stderr,"%s: ",xvt_name);
  vfprintf(stderr,fmt,args);
  va_end(args);
  fprintf(stderr,"\n");
}

XErrorHandler RxvtErrorHandler(Display *dpy, XErrorEvent *event)
{
  clean_exit(1);
  return 0;
}

/****************************************************************************
 * 
 * Switch to a new font
 *
 ***************************************************************************/

/* new function. Matthias. */ 
void LoadNewFont(){
  int w,h;
  if (mainfontfid > 0) {
	/* pre-allocated by main.C - queried by XQueryFont */
	XFreeFontInfo(NULL, mainfont, 1);
  } else {
	/* it was allocated by XLoadQueryFont */
	XFreeFont(display,mainfont);
  }
  if (reg_font_handles[font_num] > 0) {
	/* a font was pre-allocated */
	mainfont = XQueryFont(display,reg_font_handles[font_num]);
	mainfontfid = 1;
  } else {
	if ((mainfont = XLoadQueryFont(display,reg_fonts[font_num]))==NULL) 
	    {
	      error("can't access font %s\n",reg_fonts[font_num]);
	      mainfont = XLoadQueryFont(display,reg_fonts[DEFAULT_FONT]);
	    }
	mainfontfid = 0;
  }
  XSetFont(display,gc,mainfont->fid);
  XSetFont(display,rvgc,mainfont->fid);
  MyWinInfo.fheight = mainfont->ascent + mainfont->descent;
/*   This is an rxvt bug. (Matthias ) */
/*   MyWinInfo.fwidth = XTextWidth(mainfont,"M",1); */

  /* better: */ 
  MyWinInfo.fwidth = mainfont->max_bounds.width;


  w = MyWinInfo.cwidth*MyWinInfo.fwidth + sizehints.base_width;
  h = MyWinInfo.cheight*MyWinInfo.fheight + sizehints.base_height;
  sizehints.width_inc = MyWinInfo.fwidth;
  sizehints.height_inc = MyWinInfo.fheight;

  /* use the size increments for the kvt-widget, too.
    (Matthias) */ 
  kvt_set_size_increment(sizehints.width_inc, sizehints.height_inc);

  sizehints.width = w;
  sizehints.height = h;
  sizehints.min_width = sizehints.width_inc + sizehints.base_width;
  sizehints.min_height = sizehints.height_inc + sizehints.base_height;
  sizehints.flags = PMinSize | PResizeInc | PBaseSize | PWinGravity;
/*   XSetWMNormalHints(display,main_win,&sizehints); */
  XResizeWindow(display, main_win, w, h);
  MyWinInfo.pwidth = MyWinInfo.cwidth*MyWinInfo.fwidth;
  MyWinInfo.pheight = MyWinInfo.cheight*MyWinInfo.fheight;
/*  XClearWindow(display,vt_win);*/
  XClearArea(display, vt_win, 0, 0, w, h, True);
  XSync(display,0); 
}

void NewFont(int direction)
{

  if(direction == BIGGER)
    {
      font_num++;
      if(font_num >= NUM_FONTS)
	{
	  font_num = NUM_FONTS-1;
	  return;
	}
    }

  if(direction == SMALLER)
    {
      font_num--;
      if(font_num <0)
	{
	  font_num = 0;
	  return;
	}
    }
  
  LoadNewFont();
}
