/*
 * This module is all new by Robert Nation
 * (nation@rocket.sanders.lockheed.com).
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 *
 * Design of this module was heavily influenced by the original xvt 
 * design of this module. See info relating to the original xvt elsewhere
 * in this package.
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <locale.h>
#include "rxvt.h"
#include "screen.h"
#include "command.h"
#include "xsetup.h"
#include "sbar.h"
#include "debug.h"

#define PROP_SIZE 1024	/* chunk size for retrieving the selection property */

/* need this for ACS_BOARD Matthias */ 
#define stipple_data_width 4
#define stipple_data_height 4
static unsigned char stipple_data_bits[] = {
   0x03, 0x03, 0x0c, 0x0c};    
static Pixmap stipple_data_pixmap;

/* needed for the qclipboard port (Matthias) */
extern void kvt_set_selection(char* s);
extern char* kvt_get_selection();


/* A few windows and GC's, etc */
extern Display	       *display;
extern Window	       vt_win;		/* vt100 window */
extern Window	       main_win;	/* parent window */
extern XFontStruct     *mainfont;

extern GC 	        gc;		/* GC for drawing text */
extern GC 	        rvgc;		/* GC for drawing text */
extern unsigned long	foreground;	/* foreground pixel value */
extern unsigned long	background;	/* background pixel value */
extern struct sbar_info sbar;

static int alloc_color( int color );
static void free_color( int color );

WindowInfo MyWinInfo;
ScreenInfo screens[NSCREENS];
ScreenInfo *cScreen = screens;

/* Data for save-screen */
int drow,dcol, save_row, save_col, save_charset_num;
char save_charset = 'B';

/* Stephan: a little bit hardcoded :) */
#ifndef COLOR
#define COLOR
#endif

#ifdef COLOR
#define RENDITION unsigned int
#else
#define RENDITION unsigned char
#endif
#define RENDSIZE sizeof(RENDITION)

/* This tells what's acutally on the screen */
unsigned char *displayed_text = NULL;
RENDITION *displayed_rend = NULL;

char charsets[2]={'B','B'};

int selanchor_row = -1000, selend_row = -1000;
int selanchor_col=0, selend_col=0;
int selstartx = 0,selstarty = 0;
int selection_screen = LOW;
int selected = SELECTION_CLEAR;
int select_mode = SELECTION_MODE_CHAR;	/* bmg */
int wordsel_left, wordsel_right;	/* bmg */
unsigned char *ch = NULL;
int current_screen = LOW;

/* this is a pointer to an array of size cwidth, with a 1 for any
 * location where there is a tab stop */
char *tabs = NULL;

/* file descriptor for child process */
extern int comm_fd;
void check_text(char *a);
int refresh_type = SLOW;

char *std_color_names[10] = 
{
  "",                  /* default foreground */
  "",                  /* default background */
  "#000000",
  "#ff0000",
  "#00ff00",
  "#ffff00",
  "#0000ff",
  "#ff00ff",
  "#00ffff",
  "#e0e0e0",
};

char *default_color_names[18] = 
{
  "",                  /* default foreground */
  "",                  /* default background */
  "#000000",
  "#cd0000",
  "#00cd00",
  "#cdcd00",
  "#0000cd",
  "#cd00cd",
  "#00cdcd",
  "#e5e5e5",
  "#4d4d4d",
  "#ff0000",
  "#00ff00",
  "#ffff00",
  "#0000ff",
  "#ff00ff",
  "#00ffff",
  "#ffffff"
};

 char *linux_color_names[18] = 
 {
   "",                  /* default foreground */
   "",                  /* default background */
   "#000000", 
   "#b21818", 
   "#18b218", 
   "#b26918", 
   "#1818b2", 
   "#b218b2", 
   "#18b2b2", 
   "#b2b2b2", 
   "#555555", 
   "#ff5555", 
   "#55ff55", 
   "#ffff55", 
   "#5555ff", 
   "#ff55ff", 
   "#55ffff", 
   "#ffffff" 
 }; 

#define STD_BOLD_COLOR (5)

int color_type = COLOR_TYPE_DEFAULT;
char **color_names = std_color_names;
char colors_loaded[18] = {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned long pixel_colors[18];
int fore_color = 0;
int back_color = 1;

/* set default rstyle colors */
#define DEFAULT_RSTYLE (1 << 16)

int screenNum = 0, rstyle=DEFAULT_RSTYLE, save_rstyle = DEFAULT_RSTYLE, focus = 0;

extern Colormap	colormap;

/* Array of flags; flagged characters are part of a word. bmg */
char charclass[256];

/* check if a char is in the charclass. bmg */
#define ischarclass(a) (charclass[(a)])




/* Matthias */
void memset2(RENDITION* rend, RENDITION rstyle, int n){
  int i;
  for (i=0; i<n; i++){
    *rend++ = rstyle;
  }
}



/***************************************************************************
 *  Perform any initialisation on the screen data structures.  
 *  Called just once
 *  at startup.  saved_lines is the number of saved lines.
 ***************************************************************************/
void screen_init(void)
{
  int i;
#ifdef DEBUG
  fprintf(stderr,"init");  
#endif

  for(i=0;i<NSCREENS;i++)
    {
      screens[i].text = NULL;
      screens[i].rendition = NULL;
      screens[i].row = 0;
      screens[i].col = 0;
      screens[i].charset = 0;
      screens[i].insert = 0;
      screens[i].wrap_next = 0;
      screens[i].wrap = 1;
      screens[i].decom = 0;
      screens[i].cursor_is_visible = 1;
    }
  fore_color = 0;
  back_color = 1;
  save_rstyle = rstyle = DEFAULT_RSTYLE;
  save_row = 0;
  save_col = 0;
  save_charset ='B';
  save_charset_num = 0;

  MyWinInfo.sline_top = 0;
  MyWinInfo.offset = 0;
  scrn_reset();
}

/**************************************************************************
 * set some additional characters that belog to a word. bmg 
 *************************************************************************/
void set_charclass(const char *s)
{
  int j;
  const unsigned char *i = (const unsigned char *) s;

  for (j=0; j<256; j++) {
    if (isalnum(j)) {
      charclass[j] = 1;
    } else {
      charclass[j] = 0;
    }
  }

  while(*i) {
    charclass[(int) *i] = 1;
    ++i;
  }
}

/**************************************************************************
 * initialize_color_mode. bmg
 *************************************************************************/
void init_color_mode(int mode)
{
  switch (mode) {
  case COLOR_TYPE_ANSI:
    color_type = COLOR_TYPE_ANSI;
    color_names = std_color_names;
    break;
  case COLOR_TYPE_DEFAULT:
    color_type = COLOR_TYPE_DEFAULT;
    color_names = default_color_names;
    break;
  case COLOR_TYPE_Linux:
    color_type = COLOR_TYPE_Linux;
    color_names = linux_color_names;
    if (!colors_loaded[STD_BOLD_COLOR]) {
      alloc_color(STD_BOLD_COLOR);
    }
    break;
  default:
    error("invalid colormode %d.", mode);
  }
}

/**************************************************************************
 * setup color_type. bmg
 *
 * extended for xterm-mode (Matthias)
 *************************************************************************/
void set_color_mode(int mode)
{
  int i;

  if (color_type == mode)
    return;

  for (i=2;i<18;i++){
    if (colors_loaded[i]){
      free_color(i);
    }
  }

  switch (mode) {
  case COLOR_TYPE_ANSI:
    color_type = COLOR_TYPE_ANSI;
    color_names = std_color_names;
    for (i=2; i<10; i++) {
      alloc_color(i);
    }
    break;
  case COLOR_TYPE_DEFAULT:
    color_type = COLOR_TYPE_DEFAULT;
    color_names = default_color_names;
    for (i=2; i<18; i++) 
      alloc_color(i);
    break;
  case COLOR_TYPE_Linux:
    color_type = COLOR_TYPE_Linux;
    color_names = linux_color_names;
    for (i=2; i<18; i++)
      alloc_color(i);
    if (colors_loaded[0] && !colors_loaded[STD_BOLD_COLOR]) {
      alloc_color(STD_BOLD_COLOR); /* foreground bold */
    }
    break;
  default:
    error("invalid colormode %d.", mode);
  }
}

int get_color_mode(void)
{
  return color_type;
}

/**************************************************************************
 *  Reset the screen - called whenever the screen needs to be repaired due
 *  to re-sizing or initialization.
 *************************************************************************/
void scrn_reset(void)
{
  static int previous_rows[NSCREENS] = {-1,-1};
  static int previous_cols[NSCREENS] = {-1,-1};
  unsigned char *new_screen;
  RENDITION *new_rend;
  int i,j,k,l,sl;

#ifdef DEBUG
  fprintf(stderr,"scrn_reset\n");
#endif
  if((previous_cols[0] == MyWinInfo.cwidth)&&
     (previous_cols[1] == MyWinInfo.cwidth)&&
     (previous_rows[0] == MyWinInfo.cheight + MyWinInfo.saved_lines)&&
     (previous_rows[1] == MyWinInfo.cheight))
    return;
#ifdef DEBUG
  fprintf(stderr,"scrn_reset continue\n");
#endif
  /* In case rows/columns are invalid */
  if(MyWinInfo.cwidth <= 0)
    MyWinInfo.cwidth = 80;
  if(MyWinInfo.cheight <= 0)
    MyWinInfo.cheight = 24;

  for(i=0;i<NSCREENS;i++)
    {
      screens[i].tmargin = 0;
      screens[i].bmargin = MyWinInfo.cheight -1;

      /* allocate correct amount of space for the screen + scroll-back
       * memory */
      if(i==1)
	sl = 0;
      else
	sl = MyWinInfo.saved_lines;
      new_screen = safemalloc((MyWinInfo.cheight+sl)*
			   (MyWinInfo.cwidth+1)*sizeof(char),"new_screen");
      new_rend = safemalloc((MyWinInfo.cheight+sl)*
			   (MyWinInfo.cwidth+1)*RENDSIZE,"new_rend");
      /* copy from old buffer to new buffer, as appropriate. */
      l = previous_rows[i];

      for(j=(MyWinInfo.cheight+sl-1);j>=0;j--)
	{
	  l--;
	  for(k=0;k<MyWinInfo.cwidth;k++)
	    {
	      if(k<previous_cols[i] && l>=0)
		{
		  new_screen[j*(MyWinInfo.cwidth+1)+k]=
		    screens[i].text[l*(previous_cols[i]+1)+k];
		  new_rend[j*(MyWinInfo.cwidth+1)+k]=
		    screens[i].rendition[l*(previous_cols[i]+1)+k];
		}
	      else
		{
		  /* In case there's nothing to copy .... */
		  new_screen[j*(MyWinInfo.cwidth+1)+k]=' ';
		  new_rend[j*(MyWinInfo.cwidth+1)+k]=DEFAULT_RSTYLE;
		}
	    }
    	  new_screen[j*(MyWinInfo.cwidth+1)+MyWinInfo.cwidth]=0;
	  new_rend[j*(MyWinInfo.cwidth+1)+MyWinInfo.cwidth]=DEFAULT_RSTYLE;  
	}
      if(screens[i].text != NULL)
	safefree(screens[i].text,"Screens.text","scrn_reset");
      if(screens[i].rendition != NULL)
	safefree(screens[i].rendition,"Screens.rend","scrn_reset");
      screens[i].text = new_screen;
      screens[i].rendition = new_rend;      

      /* Make sure the cursor is on the screen */
      if(previous_rows[i] > 0)
	screens[i].row = MyWinInfo.cheight -
	  (previous_rows[i] - sl - screens[i].row);
      if(screens[i].row < 0)
	screens[i].row = 0;
      if(screens[i].row >= MyWinInfo.cheight)
	screens[i].row = MyWinInfo.cheight - 1;

      if(screens[i].col < 0)
	screens[i].col = 0;
      if(screens[i].col >= MyWinInfo.cwidth)
	screens[i].col = MyWinInfo.cwidth - 1;
    }
  if(previous_rows[0] > 0)
    MyWinInfo.sline_top -= (MyWinInfo.cheight + MyWinInfo.saved_lines - 
			    previous_rows[0]);
  if(MyWinInfo.sline_top < 0)
    MyWinInfo.sline_top = 0;
  if(MyWinInfo.sline_top > MyWinInfo.saved_lines)
    MyWinInfo.sline_top = MyWinInfo.saved_lines;

  previous_cols[0] = MyWinInfo.cwidth;
  previous_cols[1] = MyWinInfo.cwidth;
  previous_rows[0] = MyWinInfo.cheight + MyWinInfo.saved_lines;
  previous_rows[1] = MyWinInfo.cheight;


  if(displayed_text != NULL)
    safefree(displayed_text,"deisplay_text","scrn_reset");
  if(displayed_rend != NULL)
    safefree(displayed_rend,"displayed_rend","scrn_reset");
  displayed_text = safemalloc(MyWinInfo.cheight*(MyWinInfo.cwidth+1)*
			   sizeof(char),"displayed_text");
  displayed_rend = safemalloc(MyWinInfo.cheight*(MyWinInfo.cwidth+1)*
			   RENDSIZE,"displayed_rend");
  for(j=0;j<(MyWinInfo.cheight);j++)
    {
      for(k=0;k<=MyWinInfo.cwidth;k++)
	{
	  displayed_text[j*(MyWinInfo.cwidth+1) + k] = ' ';
	  displayed_rend[j*(MyWinInfo.cwidth+1) + k] = DEFAULT_RSTYLE;
	}
    }
  /* Make sure the cursor is on the screen */
  if(save_row >= MyWinInfo.cheight)
    save_row = MyWinInfo.cheight-1;
  if(save_col >= MyWinInfo.cwidth)
    save_col = MyWinInfo.cwidth-1;

  if(tabs != NULL)
    safefree(tabs,"tabs","scrn_reset");

  tabs = safemalloc(sizeof(char)*MyWinInfo.cwidth,"tabs");

  for(i=0;i<MyWinInfo.cwidth;i++)
    {
      if((i%8) == 0)
	tabs[i] = 1;
      else
	tabs[i] = 0;
    }
  if(ch != NULL)
    free(ch);
  ch = (unsigned char *)malloc(MyWinInfo.cwidth + 1);

  tty_set_size(comm_fd,MyWinInfo.cwidth,MyWinInfo.cheight);
}

/************************************************************************
 * Restore power-on configuration
 * Clears screen, restores default fonts, etc.
 ************************************************************************/
void scr_power_on(void)
{
  int i,j,k;

#ifdef DEBUG
  fprintf(stderr,"Power On\n");
#endif
  fore_color = 0;
  back_color = 1;
  save_rstyle = rstyle = DEFAULT_RSTYLE;
  scr_change_screen(HIGH);
  scr_erase_screen(ENTIRE);
  scr_change_screen(LOW);
  scr_erase_screen(ENTIRE);
  XClearWindow(display,vt_win);
  for (i=0;i<NSCREENS;i++)
    {
      screens[i].tmargin = 0;
      screens[i].bmargin = MyWinInfo.cheight - 1;
      screens[i].row = 0;
      screens[i].col = 0;
      screens[i].wrap = 1;
      screens[i].decom = 0;
      screens[i].insert = 0;
      screens[i].charset = 0;
    }
  charsets[0]='B';
  charsets[1]='B';
  MyWinInfo.offset = 0;
  for(j=0;j<(MyWinInfo.cheight);j++)
    {
      for(k=0;k<=MyWinInfo.cwidth;k++)
	{
	  displayed_text[j*(MyWinInfo.cwidth+1) + k] = ' ';
	  displayed_rend[j*(MyWinInfo.cwidth+1) + k] = DEFAULT_RSTYLE;
	}
      displayed_text[j*(MyWinInfo.cwidth+1) +MyWinInfo.cwidth] = 0;
    }
  save_row = 0;
  save_col = 0;
  save_charset = 'B';
  save_charset_num = 0;
  scrn_reset();
  screen_refresh();
}


/* Matthias: */ 
void scr_cursor_visible (int mode)
{
   cScreen->cursor_is_visible = mode;
}


/*************************************************************************
 *  Handle a backspace
 ************************************************************************/
void scr_backspace(void)
{
#ifdef DEBUG
  check_text("backspace");
#endif
  if(selected)check_selection(cScreen->row,cScreen->col);
  if (cScreen->col == 0 && cScreen->row > 0) 
    {
      cScreen->row--;
      cScreen->col = MyWinInfo.cwidth - 1;
    }
  else if (cScreen->wrap_next ){
    /* Matthias */ 
    int x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1);
    cScreen->text[x + MyWinInfo.cwidth] = '\0';
    cScreen->wrap_next = 0;
  }
  else
    scr_move(-1,0,COL_RELATIVE|ROW_RELATIVE);
}


/**************************************************************************
 *  Change between the alternate and the main screens
 **************************************************************************/
void scr_change_screen(int direction)
{
  int x,y,r,i,j;
  unsigned int t;

  MyWinInfo.offset = 0;
#ifdef DEBUG
  check_text("screen");
#endif
  if(current_screen == direction)
    return;
  
  current_screen = direction;
  /* swap screens */
  for(i=0;i< MyWinInfo.cheight;i++)
    {
      x = i*(MyWinInfo.cwidth+1);
      y = (i+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1);
      for(j=0;j < MyWinInfo.cwidth;j++)
	{
	  t = screens[0].text[y];
	  screens[0].text[y] = screens[1].text[x];
	  screens[1].text[x] = t;
	  t = screens[0].rendition[y];
	  screens[0].rendition[y] = screens[1].rendition[x];
	  screens[1].rendition[x] = t;
	  x++;
	  y++;
	}
    }
  r = screens[0].row;
  screens[0].row = screens[1].row;
  screens[1].row = r;
  r = screens[0].col;
  screens[0].col = screens[1].col;
  screens[1].col = r;

  r = screens[0].decom;
  screens[0].decom = screens[1].decom;
  screens[1].decom = r;

  r = screens[0].wrap;
  screens[0].wrap = screens[1].wrap;
  screens[1].wrap = r;
  r = screens[0].wrap_next;
  screens[0].wrap_next = screens[1].wrap_next;
  screens[1].wrap_next = r;
  r = screens[0].insert;
  screens[0].insert = screens[1].insert;
  screens[1].insert = r;
  r = screens[0].charset;
  screens[0].charset = screens[1].charset;
  screens[1].charset = r;
}

/**************************************************************************
 *  Change the rendition style.
 **************************************************************************/
void scr_change_rendition(int mode,int style)
{
#ifdef DEBUG
  check_text("change_rend");
#endif

  if(mode == 0)
    rstyle |= style;
  else
    {
      rstyle &= ~style;
      /* the next line seems to be buggy, so I commented it out. (Matthias) */ 
/*       rstyle = (rstyle & 0x0ff) | (fore_color <<8) | (back_color <<16);   */


      if(style == ~RS_NONE)
	{
	  /* the next two lines where commented out in rxvt-2.08. Why ?? (Matthias) */ 
	  /* guess I know why now. Matthias */ 
/*     	  fore_color = 0;     */
/*     	  back_color = 1;    */
	  rstyle = DEFAULT_RSTYLE;
	}
    }
  set_font_style(); 
}

/***************************************************************************
 *  Sets the rstyle parameter to reflect the selected font 
 **************************************************************************/
void set_font_style(void)
{
#ifdef DEBUG
  check_text("font_style");
#endif
  rstyle &= ~RS_GRFONT;
  rstyle &= ~RS_GBFONT;
  if(charsets[cScreen->charset] == 'A')
    rstyle |= RS_GBFONT;
  else if(charsets[cScreen->charset] == '0')
    rstyle |= RS_GRFONT;
}

/*************************************************************************
 *  Return the width and height of the screen.
 *************************************************************************/
void scr_get_size(int *width_p,int *height_p)
{

#ifdef DEBUG
  check_text("get_size");
#endif
  *width_p = MyWinInfo.cwidth;
  *height_p = MyWinInfo.cheight;
}


/**************************************************************************
 *  Indicate a change of keyboard focus.
 **************************************************************************/
void scr_focus(int is_in)
{
#ifdef DEBUG
  check_text("focus");
#endif

  focus = is_in;
}


/**************************************************************************
 *  Secure the keyboard
 **************************************************************************/
void scr_secure(void)
{
  static int state = 0;

#ifdef DEBUG
  check_text("secure");
#endif

  if(!state)
    {
      if(XGrabKeyboard(display,main_win,True,GrabModeAsync,
		       GrabModeAsync,CurrentTime) == GrabSuccess)
	{
	  /* enter reverse video */
	  /* no more needed. Matthias */ 
/* 	  tgc = rvgc; */
/* 	  rvgc = gc; */
/* 	  gc = tgc; */
/* 	  temp = pixel_colors[0]; */
/* 	  pixel_colors[0] = pixel_colors[1]; */
/* 	  pixel_colors[1] = temp; */
/* 	  XSetWindowBackground(display,vt_win,foreground); */
 	  state = 1; 
	}
      }
    else
      {
	XUngrabKeyboard(display,CurrentTime);
	  /* no more needed. Matthias */ 
/* 	tgc = rvgc; */
/* 	rvgc = gc; */
/* 	gc = tgc; */
/* 	temp = pixel_colors[0]; */
/* 	pixel_colors[0] = pixel_colors[1]; */
/* 	pixel_colors[1] = temp; */
/* 	XSetWindowBackground(display,vt_win,background); */
	state = 0;
      }  

  XClearWindow(display,vt_win);

  scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);
}


void scr_add_lines(unsigned char *c,int nl_count,int n)
{
  int i,nl,x;
  char c_tmp; /* Matthias */ 
#ifdef DEBUG
  check_text("add_lines");
#endif

  if(selected)check_selection(cScreen->row,cScreen->col);
      
  nl = cScreen->row + nl_count - cScreen->bmargin;

  if((nl_count > 0)&&(nl>0))
    {
      if((cScreen->tmargin == 0)&&
	 (cScreen->bmargin == MyWinInfo.cheight - 1))
	{
	  screen_scroll(cScreen->tmargin,cScreen->bmargin,nl);
	  cScreen->row -= nl;
	  if(cScreen->row < -MyWinInfo.saved_lines)
	    cScreen->row = -MyWinInfo.saved_lines;
	}
    }
  if(cScreen->col >= MyWinInfo.cwidth)
    cScreen->col =  MyWinInfo.cwidth-1;

  /* some debug stuff from Matthias */ 
/*   printf("scr_add (%d,%d)='",cScreen->row, cScreen->col ); */
/*   for (i=0;i<n;i++) printf("%c", c[i]); */
/*   printf("'\n"); */

  x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) +
    (cScreen->col);
  for(i=0;i<n;i++)
    {
      /***********************************************************************
       *  Adds a single character at the current cursor location
       *  new lines in the string.
       ***********************************************************************/

      MyWinInfo.offset = 0;
      c_tmp = c[i];
      switch (c_tmp)
	{
	case 127:
	  break;
	case '\n':
	  /* 2 lines Matthias */ 
 	  x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) +MyWinInfo.cwidth;
  	  cScreen->text[x] = '\0';

	  cScreen->wrap_next = 0;
	  if (cScreen->row == cScreen->bmargin)
	    screen_scroll(cScreen->tmargin,cScreen->bmargin,1);
	  else if (cScreen->row < MyWinInfo.cheight - 1)
	    cScreen->row++;
	  x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) +
	    (cScreen->col);
	  break;
	case '\r':
	  cScreen->col = 0;
	  cScreen->wrap_next = 0;
 	  x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) +MyWinInfo.cwidth;
 	  cScreen->text[x] = '\0'; /* Matthias */ 
	  x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) +
	    (cScreen->col);
	  break;
	case '\t':
	  if (cScreen->col < MyWinInfo.cwidth - 1) 
	    {
	      cScreen->col++;
	      while((tabs[cScreen->col] == 0)&&(cScreen->col < MyWinInfo.cwidth-1))
		cScreen->col++;
	      x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) +
		(cScreen->col);

	    }
	  break;
	default:
	  if (cScreen->wrap_next) 
	    {
	      /* 2 lines Matthias */ 
	      x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) +MyWinInfo.cwidth;
 	      cScreen->text[x] = '\n';  
	      
	      if (cScreen->row == cScreen->bmargin)
		screen_scroll(cScreen->tmargin,cScreen->bmargin,1);
	      else if (cScreen->row < MyWinInfo.cheight - 1)
		cScreen->row++;      
	      cScreen->col = 0;
	      x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) +
		(cScreen->col);
	      cScreen->wrap_next = 0;
	    }
	  if (cScreen->insert) 
	    scr_insert_delete_characters(1,INSERT);
	  cScreen->text[x] = c[i];

/*  	  displayed_text[cScreen->row*(MyWinInfo.cwidth+1)+cScreen->col] = '\0';  */
/* 	  displayed_rend[cScreen->row*(MyWinInfo.cwidth+1)+cScreen->col] = 255; */
	  cScreen->rendition[x++] = rstyle;
	  cScreen->col++;
	  if(cScreen->col == MyWinInfo.cwidth)
	    {
	      cScreen->col = MyWinInfo.cwidth -1;
	      x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) +
		(cScreen->col);
	      cScreen->wrap_next = cScreen->wrap;
	    }
	}
    }
}

/****************************************************************************
 * 
 * Scroll the text on the screen
 *  Scroll count lines from row1 to row2 inclusive.  row1 should be <= row2.
 *  scrolling is up for a +ve count and down for a -ve count.
 *
 *************************************************************************/
int screen_scroll(int row1,int row2,int count)
{
  int i;
  unsigned char *dest1,*source1;
  RENDITION *dest2,*source2;

  selanchor_row -= count;
  selend_row -= count;

  if(count > 0)
    {
      /* if the line scrolls off the top of the screen, shift the entire scroll
       * back buffer too. */
      if((row1 == 0)&&(current_screen == LOW))
	{
	  row1 = -MyWinInfo.saved_lines;
	  MyWinInfo.sline_top += count;
	  if(MyWinInfo.sline_top > MyWinInfo.saved_lines)
	    MyWinInfo.sline_top = MyWinInfo.saved_lines;
	}

      if(row2 - row1 +1 < count)
	{
	  count = row2 - row1 +1;
	}
      dest1 =&cScreen->text[(row1+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1)];
      source1 = &cScreen->text[(row1+MyWinInfo.saved_lines+count)*
			   (MyWinInfo.cwidth+1)];
      dest2 =&cScreen->rendition[(row1+MyWinInfo.saved_lines)*
					 (MyWinInfo.cwidth+1)];
      source2 = &cScreen->rendition[(row1+MyWinInfo.saved_lines+count)*
					    (MyWinInfo.cwidth+1)];
      /* Forward overlapping memcpy's are probably OK, but backwards
       * doesn't work with SunOS 4.1.3,
       * so this is OK, but for the next section, do it one line
       * at a time. */
      if((row2-count-row1+1) > 0)
	{
	  memcpy(dest1,source1,(MyWinInfo.cwidth+1)*(row2-count-row1+1));
	  dest1+=(MyWinInfo.cwidth+1)*(row2-count-row1+1);
	  memcpy(dest2,source2,
		 (MyWinInfo.cwidth+1)*(row2-count-row1+1)*RENDSIZE);
	  dest2+=(MyWinInfo.cwidth+1)*(row2-count-row1+1);
	}

      /* copy blank lines in at the bottom */
      memset(dest1,' ',(MyWinInfo.cwidth+1)*count);
      memset2(dest2,DEFAULT_RSTYLE,(MyWinInfo.cwidth+1)*count);

      dest1--;
      dest1 += MyWinInfo.cwidth + 1; /* Matthias */ 
      for(i=0;i<(count+1);i++)
	{
	  *dest1 = 0;
	  dest1 += MyWinInfo.cwidth+1;
	}
    }
  else
    {
      dest1 =&cScreen->text[(row2+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1)];
      source1 = &cScreen->text[(row2+MyWinInfo.saved_lines+count)*
			       (MyWinInfo.cwidth+1)];
      dest2 =&cScreen->rendition[(row2+MyWinInfo.saved_lines)*
				 (MyWinInfo.cwidth+1)];
      source2 =&cScreen->rendition[(row2+MyWinInfo.saved_lines+count)*
				    (MyWinInfo.cwidth+1)];
      for(i=row2;i>=row1-count;i--)
	{
	  memcpy(dest1,source1,MyWinInfo.cwidth);
	  dest1 -= (MyWinInfo.cwidth+1);
	  source1 -= (MyWinInfo.cwidth+1);
	  memcpy(dest2,source2,MyWinInfo.cwidth*RENDSIZE);
	  dest2 -= (MyWinInfo.cwidth+1);
	  source2 -= (MyWinInfo.cwidth+1);
	}
      /* copy blank lines in at the top */
      for(;i>=row1;i--)
	{
	  memset(dest1,' ',MyWinInfo.cwidth);
	  memset2(dest2,DEFAULT_RSTYLE,MyWinInfo.cwidth);
	  dest2 -= (MyWinInfo.cwidth+1);
	  *(dest1+MyWinInfo.cwidth) = 0;
	  dest1 -= (MyWinInfo.cwidth+1);
	}
    }
  return count;
}

/***************************************************************************
 *
 * Find a word for word-select-mode
 * 
 ***************************************************************************/

static void scr_word_selection(int row, int col)	/* bmg */
{
  int i;
  unsigned char *ptr;

  ptr = cScreen->text+(row+MyWinInfo.saved_lines-MyWinInfo.offset)*
    (MyWinInfo.cwidth+1);
  
  if (!ischarclass(*(ptr+col))) {
    selanchor_col = selend_col = col; 
  } else {
    if (col>0) {
      for (i=col-1;; --i) {
	if (i<0 || !ischarclass(*(ptr+i))) {
	  selanchor_col = i+1;
	  break;
	}
      }
    } else {
      selanchor_col = col;
    }
    
    if (col<MyWinInfo.cwidth) {
      for (i=col+1;; ++i) {
	if (i==MyWinInfo.cwidth || !ischarclass(*(ptr+i))) {
	  selend_col = i-1;
	  break;
	}
      }
    } else {
      selend_col = col;
    }
  }
  wordsel_right = selend_col;
  wordsel_left = selanchor_col;
}

/***************************************************************************
 *
 * Expand selection in word-select-mode
 *
 ***************************************************************************/

static void scr_word_expand(int row, int col)	/* bmg */
{
  int i, dir;
  unsigned char *ptr;

  ptr = cScreen->text+(row+MyWinInfo.saved_lines-MyWinInfo.offset)*
    (MyWinInfo.cwidth+1);

  dir = (selanchor_row>row ? -1 : 
	 (selanchor_row<row ? 1 :
	  (wordsel_left>col ? -1 : 
	   (wordsel_right<col ? 1 : 0))));
  
  if (!ischarclass(*(ptr+col))) {
    switch(dir) {
    case -1:
      selend_col = col;
      selanchor_col = wordsel_right;
      return;
    case 1:
      selend_col = col;
      selanchor_col = wordsel_left;
      return;
    }
  }

  switch (dir) {
  case -1:
    if (col>0) {
      for (i=col-1;; --i) {
	if (i<0 || !ischarclass(*(ptr+i))) {
	  selend_col = i+1;
	  break;
	}
      }
    } else {
      selend_col = col;
    }
    selanchor_col = wordsel_right;
    break;
  case 0:
    selanchor_col = wordsel_left;
    selend_col = wordsel_right;
    break;
  case 1:
    if (col<MyWinInfo.cwidth) {
      for (i=col+1;; ++i) {
	if (i==MyWinInfo.cwidth || !ischarclass(*(ptr+i))) {
	  selend_col = i-1;
	  break;
	}
      }
    } else {
      selend_col = col;
    }
    selanchor_col = wordsel_left;
  }
}

/*****************************************************************************
 *
 * If (row,col) is within a selected region of text, remove the selection
 *
 ****************************************************************************/
inline void  check_selection(int row, int col)
{
  int val,a,b,c,d;
#ifdef DEBUG
  check_text("selectiob");
#endif

  if(current_screen != selection_screen)
    return;

  val = (row - selanchor_row)*(row-selend_row);

  if(val < 0)
    scr_clear_selection();
  else if (val == 0)
    {
      /* We're on the same row as the start or end of selection */
      if((selanchor_row < selend_row)||
	 ((selanchor_row == selend_row)&&(selanchor_col < selend_col)))
	{
	  a = selanchor_row;
	  b = selend_row;
	  c = selanchor_col;
	  d = selend_col;
	}
      else
	{
	  a = selend_row;
	  b = selanchor_row;
	  c = selend_col;	     
	  d = selanchor_col;
	}
      if((row == a)&&(row == b))
	{
	  if((col >=c)&&(col <=d))
	    scr_clear_selection();
	}
      else if((row == a)&&(col >=c))
	scr_clear_selection();
      else if((row ==b)&&(col <=d))
	scr_clear_selection();
    }
  return;
}



/*************************************************************************
 *  Move the cursor to a new position.  The relative argument is a pair of
 *  flags that specify relative rather than absolute motion.
 *************************************************************************/
void scr_move(int x,int y,int relative)
{
  MyWinInfo.offset = 0;
  cScreen->col = (relative & COL_RELATIVE) ? cScreen->col + x : x;
  if (cScreen->col < 0)
    cScreen->col = 0;
  if (cScreen->col >= MyWinInfo.cwidth)
    cScreen->col = MyWinInfo.cwidth - 1;

  /* Matthias */ 
  if (cScreen->wrap_next){
    x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) + MyWinInfo.cwidth;
    cScreen->text[x] = '\0'; 
    cScreen->wrap_next = 0;
  }


  if (relative & ROW_RELATIVE) 
    {
      if (y > 0) 
	{
	  if((cScreen->row<=cScreen->bmargin)&&
	     (cScreen->row+y > cScreen->bmargin))
	    cScreen->row = cScreen->bmargin;
	  else
	    cScreen->row += y;
	}
      else if (y < 0) 
	{
	  if((cScreen->row>=cScreen->tmargin)&&
	     (cScreen->row+y<cScreen->tmargin))
	    cScreen->row = cScreen->tmargin;
	  else
	    cScreen->row += y;
	}
    }
  else
    {
      if (cScreen->decom) 
	{
	  cScreen->row = y + cScreen->tmargin;
	  if (cScreen->row > cScreen->bmargin)
	    cScreen->row = cScreen->bmargin;
	}
      else
	cScreen->row = y;
    }
  if (cScreen->row < 0)
    cScreen->row = 0;
  if (cScreen->row >= MyWinInfo.cheight)
    cScreen->row = MyWinInfo.cheight - 1;
  cScreen->wrap_next = 0;
}


/*************************************************************************
 *  Move the cursor down one line and scroll if necessary.
 *************************************************************************/
void scr_index(int direction)
{
  MyWinInfo.offset = 0;
#ifdef DEBUG
  check_text("index");
#endif

  /* Matthias */ 
  if (cScreen->wrap_next){
    int x;
    x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) + MyWinInfo.cwidth;
    cScreen->text[x] = '\0';
    cScreen->wrap_next = 0;
  }

  if ((cScreen->row == cScreen->bmargin)&&(direction == 1))
    {
      screen_scroll(cScreen->tmargin,cScreen->bmargin,1);
    }
  else if ((cScreen->row == cScreen->tmargin)&&(direction = -1))
    screen_scroll(cScreen->tmargin,cScreen->bmargin,-1);
  else
    cScreen->row += direction;
  cScreen->wrap_next = 0;
  if(selected)check_selection(cScreen->row,cScreen->row);
}


/***************************************************************************
 *  Save the cursor position and rendition style.
 ****************************************************************************/
void scr_save_cursor(void)
{
  save_row = cScreen->row;
  save_col = cScreen->col;
  save_rstyle = rstyle;
  save_charset_num = cScreen->charset;
  save_charset = charsets[cScreen->charset];
}


/***************************************************************************
 *  Restore the cursor position and rendition style.
 ****************************************************************************/
void scr_restore_cursor(void)
{
  cScreen->row = save_row;
  cScreen->col = save_col;
/*   rstyle = (save_rstyle & 0x0ff) | (fore_color <<8) | (back_color <<16); */
  rstyle = save_rstyle; /* this is more reasonable. Matthias */ 
  cScreen->charset = save_charset_num;
  charsets[cScreen->charset] = save_charset;
  set_font_style();
}

/***************************************************************************
 *  erase part or the whole of a line
 ****************************************************************************/
void scr_erase_line(int mode)
{
  unsigned char *starttext;
  RENDITION *startrend;
  
#ifdef DEBUG
  check_text("erase line");
#endif

  MyWinInfo.offset = 0;
  starttext = &cScreen->text[(cScreen->row+MyWinInfo.saved_lines)*
			     (MyWinInfo.cwidth+1)];
  startrend = &cScreen->rendition[(cScreen->row+MyWinInfo.saved_lines)*
				  (MyWinInfo.cwidth+1)];

  /* Matthias */ 
  if (cScreen->wrap_next){
    int x;
    x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) + MyWinInfo.cwidth;
    cScreen->text[x] = '\0';
    cScreen->wrap_next = 0;
  }

  switch (mode) 
    {
    case START :
#ifdef DEBUG
      check_text("erase line start");
#endif
      /* erase to start of line */
      memset(starttext,' ',cScreen->col+1);
      memset2(startrend,DEFAULT_RSTYLE,(cScreen->col+1));
      break;
    case END :
#ifdef DEBUG
      check_text("erase line end");
#endif
      /* erase to end of line */
      memset(starttext+cScreen->col,' ',MyWinInfo.cwidth - cScreen->col);
      memset2(startrend+cScreen->col, rstyle & ~RS_CURSOR,
	     (MyWinInfo.cwidth - cScreen->col));
      break;
    case ENTIRE :
#ifdef DEBUG
      check_text("erase line entire");
#endif
      memset(starttext,' ',MyWinInfo.cwidth);
      memset2(startrend,rstyle & ~RS_CURSOR,MyWinInfo.cwidth);
      break;
    default :
#ifdef DEBUG
      check_text("erase line none");
#endif
      return;
    }
#ifdef DEBUG
  check_text("erase line done");
#endif
  if(selected)check_selection(cScreen->row,cScreen->row);
  cScreen->wrap_next = 0;
}


/* Matthias */
void scr_erase_char(int n){
  unsigned char *starttext;
  /* int back_color, fore_color; */
  RENDITION *startrend;
  /* int end,i; */
  if (n < 1)
    return;
  MyWinInfo.offset = 0;
  
  starttext = &cScreen->text[(cScreen->row+MyWinInfo.saved_lines)*
			     (MyWinInfo.cwidth+1) + cScreen->col];
  startrend = &cScreen->rendition[(cScreen->row+MyWinInfo.saved_lines)*
				  (MyWinInfo.cwidth+1) + cScreen->col];

  if (n + cScreen->col > MyWinInfo.cwidth)
    n = MyWinInfo.cwidth - cScreen->col;
  
  memset(starttext,' ',n);
  memset2(startrend, rstyle & ~RS_CURSOR, n);
}
 


/***************************************************************************
 *  erase part or the whole of the screen
 ***************************************************************************/
void scr_erase_screen(int mode)
{
  int j,x,a,b,startr,startc,endr,endc;

#ifdef DEBUG
  check_text("erase screen");
#endif
  switch(mode)
    {
    case START:
      startr = 0;
      startc = 0;
      endr = cScreen->row;
      endc = cScreen->col;
      break;
    case END:
      startr = cScreen->row;
      startc = cScreen->col;
      endr = MyWinInfo.cheight -1;
      endc = MyWinInfo.cwidth -1;
      break;
    case ENTIRE:
      startr = 0;
      startc = 0;
      endr = MyWinInfo.cheight -1;
      endc = MyWinInfo.cwidth -1;
      break;
    default:
      startr = 0;
      startc = 0;
      endr = -1;
      endc = -1;
      break;
    }
  for(j=startr;j<=(endr);j++)
    {
      if(j == startr)
	a = startc;
      else
	a = 0;
      if(j==endr)
	b = endc;
      else
	b = MyWinInfo.cwidth-1;
      x=(j+MyWinInfo.saved_lines-MyWinInfo.offset)*
	(MyWinInfo.cwidth+1)+a;
      memset(&cScreen->text[x],' ',b-a+1);
      /* memset2(&cScreen->rendition[x],DEFAULT_RSTYLE,(b-a+1)); */
      memset2(&cScreen->rendition[x],rstyle & ~RS_CURSOR,(b-a+1));
    }
}


/**************************************************************************
 *  Fill the screen with E's
 **************************************************************************/
void scr_E(void)
{
  int j,x;

#ifdef DEBUG
  check_text("E");
#endif
  for(j=0;j<(MyWinInfo.cheight);j++)
    {
      x=(j+MyWinInfo.saved_lines-MyWinInfo.offset)*
	(MyWinInfo.cwidth+1);
      memset(&cScreen->text[x],'E',MyWinInfo.cwidth);
      memset2(&cScreen->rendition[x],DEFAULT_RSTYLE,MyWinInfo.cwidth);
    }
} 



/**************************************************************************
 *  Insert or delete count lines and scroll
 *  if insdel = 1, then delete lines
 *                 scroll up the bottom of the screen to fill the gap
 *  if insdel = -1, then insert lines
 *                 scroll down the lower lines
 *  other values of insdel yield undefined results
 **************************************************************************/
void scr_insert_delete_lines(int count, int insdel)
{
#ifdef DEBUG
  check_text("ins del lines");
#endif

  if (cScreen->row > cScreen->bmargin)
    return;

  if ((insdel==DELETE)&&(count > cScreen->bmargin - cScreen->row + 1))
    return;

  if((insdel==INSERT)&&(count > cScreen->bmargin - cScreen->row + 1))
    count = cScreen->bmargin - cScreen->row + 1;  

  /* Matthias */ 
  if (cScreen->wrap_next){
    int x;
    x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) + MyWinInfo.cwidth;
    cScreen->text[x] = '\0';
    cScreen->wrap_next = 0;
  }

  MyWinInfo.offset = 0;
  screen_scroll(cScreen->row,cScreen->bmargin,insdel*count);
  cScreen->wrap_next = 0;
}


/***************************************************************************
 *  Insert or Delete count characters from the current position.
 *  if insdel == 1, delete chars
 *  if insdel == -1, insert chars
 ****************************************************************************/
void scr_insert_delete_characters(int count, int insdel)
{
  unsigned char *starttext,*endtext;
  RENDITION *startrend, *endrend;

#ifdef DEBUG
  check_text("insert_del");
#endif
  if (count > MyWinInfo.cwidth - cScreen->col)
    count = MyWinInfo.cwidth - cScreen->col;
  if (count <= 0)
    return;
  
  MyWinInfo.offset = 0;
  if(selected)check_selection(cScreen->row,cScreen->col);

  starttext = &cScreen->text[(cScreen->row+MyWinInfo.saved_lines)*
			     (MyWinInfo.cwidth+1) + (cScreen->col)];
  startrend = &cScreen->rendition[(cScreen->row+MyWinInfo.saved_lines)*
				  (MyWinInfo.cwidth+1) + (cScreen->col)];

  /* Matthias */ 
  {
    int x;
    x = (cScreen->row+MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) + MyWinInfo.cwidth;
    cScreen->text[x] = '\0';
    cScreen->wrap_next = 0;
  }
  

  if(insdel == DELETE)
    {
      while(( *(starttext+count) != 0)&&(*starttext != 0))
	{
	  *starttext = *(starttext+count);
	  *startrend = *(startrend+count);
	  starttext++;
	  startrend++;
	}

      while( *starttext != 0)
	{
	  *starttext = ' ';
	  *startrend = DEFAULT_RSTYLE;
	  starttext++;
	  startrend++;
	}
    }
  else
    {
      /* INSERT count characters */
      /* find end of line */

      endtext=&cScreen->text[(cScreen->row+MyWinInfo.saved_lines)*
			     (MyWinInfo.cwidth+1) + (MyWinInfo.cwidth-1)];
      *(endtext+1) = 0;
      endrend=&cScreen->rendition[(cScreen->row+MyWinInfo.saved_lines)*
				  (MyWinInfo.cwidth+1)+(MyWinInfo.cwidth-1)];
      while(endtext - count >= starttext)
	{
	  *endtext = *(endtext-count);
	  *endrend = *(endrend-count);
	  endtext--;
	  endrend--;
	}
      while(endtext>=starttext)
	{
	  *endtext = ' ';
	  *endrend = DEFAULT_RSTYLE;
	  endtext--;
	  endrend--;
	}
    }
  cScreen->wrap_next = 0;

}


/***************************************************************************
 *  Attempt to set the top and bottom scroll margins.
 ***************************************************************************/
void scr_set_margins(int top,int bottom)
{
  if (top < 0)
    top = 0;
  if (bottom >= MyWinInfo.cheight)
    bottom = MyWinInfo.cheight - 1;
  if (top > bottom)
    return;
  
  cScreen->tmargin = top;
  cScreen->bmargin = bottom;
  scr_move(0,0,0);
}


/***************************************************************************
 *  Set or unset automatic wrapping.
 ****************************************************************************/
void scr_set_wrap(int mode)
{
  cScreen->wrap = (mode == HIGH);
}


/***************************************************************************
 *  Set or unset margin origin mode.
 ****************************************************************************/
void scr_set_decom(int mode)
{
  cScreen->decom = (mode == HIGH);
  scr_move(0,0,0);
}


/***************************************************************************
 *  Set or unset automatic insert mode.
 ****************************************************************************/
void scr_set_insert(int mode)
{
  cScreen->insert = (mode == HIGH);
}


/***************************************************************************
 *  Move the display so that line represented by scrollbar value y is at the 
 *  top of the screen.
 ****************************************************************************/
void scr_move_to(int y)
{
  int lnum;

#ifdef DEBUG
  check_text("moveto");
#endif
  y = sbar.height - 1 - y;
  lnum = y * (MyWinInfo.cheight + MyWinInfo.sline_top - 1) / (sbar.height - 1);
  MyWinInfo.offset = lnum - MyWinInfo.cheight + 1;
  if (MyWinInfo.offset > MyWinInfo.sline_top)
    MyWinInfo.offset = MyWinInfo.sline_top;
  if (MyWinInfo.offset < 0)
    MyWinInfo.offset = 0;
}


/***************************************************************************
 *  Scroll the screen up a little less than a full window
 *  Direction = 1 for scroll up,
 *            =-1 for scroll down.
 ****************************************************************************/
void scr_move_up_down(int direction)
{
#ifdef DEBUG
  check_text("move up_donw");
#endif
  MyWinInfo.offset = MyWinInfo.offset + direction*MyWinInfo.cheight*0.8;
  if (MyWinInfo.offset > MyWinInfo.sline_top)
    MyWinInfo.offset = MyWinInfo.sline_top;
  if (MyWinInfo.offset < 0)
    MyWinInfo.offset = 0;  

  /* do some refreshing here (Matthias) */ 
/*  scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);*/
  screen_refresh();
}


/***************************************************************************
 *  Make the selection currently delimited by the selection end markers.
 ***************************************************************************/
void scr_make_selection(int time)
{
  unsigned char *str, *ptr;
  int i, total,j,a,b,tx;
  int start_row, start_col, end_row, end_col;
  
  if((selected == SELECTION_BEGIN)||
     (selected == SELECTION_INIT))
    {
      scr_clear_selection();
      selanchor_col = (selstartx - MARGIN)/MyWinInfo.fwidth;
      selanchor_row = (selstarty - MARGIN)/MyWinInfo.fheight;
      selend_col = (selstartx - MARGIN)/MyWinInfo.fwidth;
      selend_row = (selstarty - MARGIN)/MyWinInfo.fheight;
      selected = SELECTION_COMPLETE;
      return;
    }
  
  if(selected != SELECTION_ONGOING)
    return;

  selected = SELECTION_COMPLETE;

  
  selection_screen = current_screen;
  /* Set start_row, col and end_row, col to point to the 
   * selection endpoints. */
  if(((selend_col <= selanchor_col)&&(selend_row == selanchor_row))||
     (selend_row < selanchor_row))
    {
      start_row = selend_row;
      start_col = selend_col;
      end_row = selanchor_row;
      end_col = selanchor_col;
    }
  else
    {
      end_row = selend_row;
      end_col = selend_col;
      start_row = selanchor_row;
      start_col = selanchor_col;
    }

  total = (end_row - start_row +1)*(MyWinInfo.cwidth+1);
  str = (unsigned char *)safemalloc(total + 1,"sel_text");

  ptr = str;
  /* save all points between start and end with selection flag */
  for(j=start_row; j <= end_row ;j++)
    {
      a=0;
      b = MyWinInfo.cwidth -1 ;
      if(j == start_row)
	  a = start_col;
      if(j == end_row)
	  b = end_col;

      tx=(j+MyWinInfo.saved_lines-MyWinInfo.offset)*
	(MyWinInfo.cwidth+1);
      if(((tx + a) < 0)||
	 ((tx+b)>=(MyWinInfo.saved_lines+MyWinInfo.cheight)*
	  (MyWinInfo.cwidth+1)))
	{
	  scr_clear_selection();
	  return;
	}
      for(i = a ; i <= b ; i++)
	{
	  *ptr++ =  cScreen->text[tx+i];
	}
      /* some modifications by me  Matthias */ 
      if(b == (MyWinInfo.cwidth-1)&& !cScreen->text [tx + MyWinInfo.cwidth])
	{
	  ptr--;
	  while(ptr >= str && *ptr == ' ' )
	    ptr--;
	  ptr++;
	  *ptr++ = '\n';
	}
    }  
  *ptr = 0;

  kvt_set_selection(str);
  safefree(str, "schwachsinn", "sel_text" );
  return;
}




/***************************************************************************
 *  Request the current primary selection
 ***************************************************************************/
void scr_paste_selection()
{
  int i = 0;
  char* s = kvt_get_selection();
  if (!s)
    return;

  /* this is a hack for stupid clients like pine who seem to prefer \r over \n grrr...(Matthias) */
  for (;s[i];i++){
    if (s[i]=='\n')
      s[i]='\r';
  }

  send_string(s,strlen(s));

}


/***************************************************************************
 *  Clear the current selection.
 ***************************************************************************/
void scr_clear_selection(void)
{
  int j,x,i;

  selected = SELECTION_CLEAR;

  for(j = 0 ; j < MyWinInfo.saved_lines + MyWinInfo.cheight ;j++)
    {
      x=j*(MyWinInfo.cwidth+1);
      for(i = 0 ; i < (MyWinInfo.cwidth +1) ; i++)
	cScreen->rendition[x+i] &= ~RS_SELECTED;
    }  

  for(j = 0 ; j < MyWinInfo.cheight ;j++)
    {
      x=j*(MyWinInfo.cwidth+1);
      for(i = 0 ; i < (MyWinInfo.cwidth +1) ; i++)
	cScreen->rendition[x+i] &= ~RS_SELECTED;
    }  

  selanchor_row = selanchor_col = selend_row = selend_col = 0;
}

/***************************************************************************
 *  Mark selected points (used by scr_extend_selection)
 ****************************************************************************/
static void scr_set_clr_sel(int op_flg,int start_row,int start_col,
			    int end_row,int end_col)
{
  int j,tx,a,b,i; /*,c,d;*/
  for(j=start_row; j <= end_row ;j++)
    {
      a=0;
      b = MyWinInfo.cwidth -1 ;
      if(j == start_row)
	a = start_col;
      if(j == end_row)
	b = end_col;

      tx=(j+MyWinInfo.saved_lines-MyWinInfo.offset)*
	(MyWinInfo.cwidth+1);
      for(i = a ; i <= b ; i++)
	{
	  if (op_flg == SEL_SET)
	    cScreen->rendition[tx+i] |= RS_SELECTED;
          else
	    cScreen->rendition[tx+i] &= ~RS_SELECTED;
	}
    }  
}

/***************************************************************************
 *  Extend the selection.
 ****************************************************************************/
void scr_extend_selection(int x,int y)
{
  /* int clr_start_row, clr_start_col,clr_end_row,clr_end_col; */
  /* int set_start_row, set_start_col,set_end_row,set_end_col; */
  int old_selend_row, old_selend_col;
  int old_direction,direction;
  
  if(selected == SELECTION_INIT)
    {
      scr_clear_selection();
      selanchor_col = selend_col = (selstartx - MARGIN)/MyWinInfo.fwidth;
      selanchor_row = selend_row = (selstarty - MARGIN)/MyWinInfo.fheight;
      selected = SELECTION_BEGIN;
    }
/*  if(selected == SELECTION_CLEAR) {
      scr_start_selection(x,y);
      return;
    }*/

  if(selected == SELECTION_COMPLETE)
    {
      selected = SELECTION_ONGOING;
    }
  if((selected != SELECTION_ONGOING)&&(selected != SELECTION_BEGIN))
    return;

  /* Remember old selection for virtual removal		*/
  old_selend_row = selend_row;
  old_selend_col = selend_col;
  
  /* Figure out where new selection is			*/

  selend_col = (x - MARGIN)/MyWinInfo.fwidth;
  selend_row = (y - MARGIN)/MyWinInfo.fheight;

  /* Handle `select a line'-mode and `select a word'-mode here. bmg */ 
  switch(select_mode) {
  case SELECTION_MODE_LINE:
    selend_col = MyWinInfo.cwidth-1;
    break;
  case SELECTION_MODE_WORD:
    scr_word_expand(selend_row, selend_col);
    break;
  }
    
  if(selend_col >= MyWinInfo.cwidth){
    /* Matthias: correct last column */ 
    if (selend_row < selanchor_row){
      selend_col = 0;
      selend_row++;
    }
    else
      selend_col = MyWinInfo.cwidth -1;
  }

  if(selend_row >= MyWinInfo.cheight)
    selend_row = MyWinInfo.cheight -1;
  if(selend_col < 0){
    /* Matthias: correct first column */ 
    if (selend_row > selanchor_row){
      selend_col = MyWinInfo.cwidth -1;
      selend_row--;
    }
    else 
      selend_col = 0;
  }
  if(selend_row < 0)
    selend_row = 0;

  if(((selend_col != selanchor_col)||
     (selend_row != selanchor_row))&&
     (selected == SELECTION_BEGIN))
    selected = SELECTION_ONGOING;

  /* If new selection is same as old selection just return	*/
  if ( (selend_row==old_selend_row) && (selend_col==old_selend_col) )
    return;

  /* determine direction of old selection				*/
  if(((old_selend_col <= selanchor_col)&&(old_selend_row == selanchor_row))||
     (old_selend_row < selanchor_row))
    old_direction = SELDIR_NEG ;
  else 
    old_direction = SELDIR_POS ;

  /* determine direction of new selection				*/
  if(((selend_col <= selanchor_col)&&(selend_row == selanchor_row))||
     (selend_row < selanchor_row))
    direction = SELDIR_NEG ;
  else
    direction = SELDIR_POS ;

  /* If old and new direction are different, clear old, set new		*/
  if (direction!=old_direction)
    {
      if (old_direction==SELDIR_NEG)
        {
          scr_set_clr_sel(SEL_CLR,old_selend_row,old_selend_col,
			  selanchor_row,selanchor_col);
          scr_set_clr_sel(SEL_SET,selanchor_row,selanchor_col,
			  selend_row,selend_col);
        }
      else
        {
          scr_set_clr_sel(SEL_CLR,selanchor_row,selanchor_col,
			  old_selend_row,old_selend_col);
          scr_set_clr_sel(SEL_SET,selend_row,selend_col,
			  selanchor_row,selanchor_col);
        }
    }
  else
    {
      if (old_direction==SELDIR_NEG)
        {
          if ( (old_selend_row < selend_row) ||
	      ((old_selend_row==selend_row)&&(old_selend_col<selend_col)) )
            {
              scr_set_clr_sel(SEL_CLR,old_selend_row,old_selend_col,
			      selend_row,selend_col);
              scr_set_clr_sel(SEL_SET,selend_row,selend_col,
			      selend_row,selend_col);
            }
          else
            {
              scr_set_clr_sel(SEL_SET,selend_row,selend_col,
			      old_selend_row,old_selend_col);
            }
        }
      else
        {
          if ( (selend_row < old_selend_row) ||
	      ((selend_row==old_selend_row)&&(selend_col<old_selend_col)) )
            {
              scr_set_clr_sel(SEL_CLR,selend_row,selend_col,
			      old_selend_row,old_selend_col);
              scr_set_clr_sel(SEL_SET,selend_row,selend_col,
			      selend_row,selend_col);
            }
          else
            {
              scr_set_clr_sel(SEL_SET,old_selend_row,old_selend_col,
			      selend_row,selend_col);
            }
        }
    }
}


/***************************************************************************
 *  start a selection using the specified unit.
 ***************************************************************************/
/* changed for multiclicks. bmg */

void scr_start_selection(unsigned int clicks, int x,int y)
{
  switch (clicks) {
  case 1:
    selstartx = x;
    selstarty = y;
    selected = SELECTION_INIT;
    select_mode = SELECTION_MODE_CHAR;
    break;
  case 2:
    scr_word_selection((y - MARGIN)/MyWinInfo.fheight,
		       (x - MARGIN)/MyWinInfo.fwidth);
    scr_set_clr_sel(SEL_SET, selanchor_row, selanchor_col, 
		    selend_row, selend_col);
    selected = SELECTION_ONGOING;
    select_mode = SELECTION_MODE_WORD;
    break;
  case 3:
    selanchor_row = (y - MARGIN)/MyWinInfo.fheight;
    selanchor_col = 0;
    selend_row = selanchor_row;
    selend_col = MyWinInfo.cwidth-1 ;
    scr_set_clr_sel(SEL_SET, selanchor_row, selanchor_col, 
		    selend_row, selend_col);
    selected = SELECTION_ONGOING;
    select_mode = SELECTION_MODE_LINE;
    break;
  }
}


/***************************************************************************
 *  Report the current cursor position.
 ****************************************************************************/
void scr_report_position(void)
{
  cprintf("\033[%d;%dR",cScreen->row + 1,cScreen->col + 1);
}


/***************************************************************************
 *  Set a font
 ****************************************************************************/
void scr_set_charset(int set, unsigned char a)
{
  charsets[set] = a;
  set_font_style();
}


/***************************************************************************
 * choose a font
 **************************************************************************/
void scr_choose_charset(int set)
{
  cScreen->charset = set;
  set_font_style();
}

/**************************************************************************
 * Repaint the box delimited by row1 to row2 and col1 to col2 of the displayed
 * screen from the backup screen.
 *************************************************************************/
void scr_refresh(int x,int y,int width,int height)
{
  int r1,r2,c1,c2,j;
#ifdef DEBUG
  check_text("scr_refresh");  
#endif
  c1=(x - MARGIN)/MyWinInfo.fwidth;
  r1 =(y - MARGIN)/MyWinInfo.fheight;
  c2 = c1 + width/MyWinInfo.fwidth + 1;
  r2 = r1 + height/MyWinInfo.fheight + 1;

  if(r1 < 0) r1 = 0;
  if(c1 < 0) c1 = 0;
  if(r2 < 0) r2 = 0;
  if(c2 < 0) c2 = 0;

  if(c1 >= MyWinInfo.cwidth) c1 = MyWinInfo.cwidth-1;
  if(c2 >= MyWinInfo.cwidth) c2 = MyWinInfo.cwidth-1;

  if(r2 >= MyWinInfo.cheight) r2 = MyWinInfo.cheight-1;
  if(r1 >= MyWinInfo.cheight) r1 = MyWinInfo.cheight-1;

  /* hmmm, normal expose events give a blank spot on the screen,
   * but graphics expose gives garbage. */
  /* Oops! Even normal expose events don't always have blank
   * screen underneath! */
  for(j=r1;j<=r2;j++)
    {
      memset(&displayed_text[j*(MyWinInfo.cwidth+1)+c1],0,c2-c1+1);
      memset2(&displayed_rend[j*(MyWinInfo.cwidth+1)+c1],
	     DEFAULT_RSTYLE,(c2-c1+1));
    }
}


/**************************************************************************
 * Actually draws to the X window 
 * For X related speed-ups, this is a good place to fiddle.
 * The arrays displayed_text and displayed_rend contain what I 
 * believe is currently shown on the screen. The arrays in cScreen contain
 * what should be displayed. This routine can decide how to refresh the
 * screen. Calls in command.c decide when to screen_refresh.
 **************************************************************************/
void screen_refresh()
{
  /* extern int font_num; */
  char force_next,fn;
  int j,k,x1,y1,x,n,k0,xrow,x2,xrow2,k1;
#ifdef USE_XCOPYAREA
  int l,trow,trow2,count,j0;
#endif
  int rval, fore, back;
  unsigned long newgcm = 0;
  XGCValues newgcv;

  GC thisGC;
  int xcursor = 0;
  static int d_xcursor = 0;
  int outline = False;
#ifdef DEBUG
  check_text("screen_refresh");  
#endif


  /* Window is not visible at all, don't update */
  if(refresh_type == DONT_BOTHER)
    return;

  /* take care about the scrollbar, too (Matthias) */
  sbar_show(MyWinInfo.cheight+MyWinInfo.sline_top-1,MyWinInfo.offset,
	    MyWinInfo.offset + MyWinInfo.cheight -1);

   if(d_xcursor < (MyWinInfo.cheight*(MyWinInfo.cwidth+1))) 
     displayed_rend[d_xcursor] = 255 | DEFAULT_RSTYLE; 

#ifdef USE_XCOPYAREA
  if((refresh_type != SLOW))
    {
      fprintf(stderr, "should not happen with kvt\n");
      /* scroll using bitblt whereever possible.  */
      /* a dirty approximation will ignore the rendition field here, and
       * fix it up later. */
      for(j=0;j<MyWinInfo.cheight;j++)
	displayed_text[j*(MyWinInfo.cwidth+1)+MyWinInfo.cwidth] = 0;  
      for(j=0;j<MyWinInfo.cheight;j++)
	{
	  xrow=(j+MyWinInfo.saved_lines-MyWinInfo.offset)*(MyWinInfo.cwidth+1);
	  xrow2 = j*(MyWinInfo.cwidth+1);
	  if(strcmp(&displayed_text[xrow2],&cScreen->text[xrow])!=0)
	    {
	      /* look for a similar line */
	      for(k=0;k<MyWinInfo.cheight;k++)
		{
		  xrow=(j+MyWinInfo.saved_lines-MyWinInfo.offset)*
		    (MyWinInfo.cwidth+1);
		  xrow2 = k*(MyWinInfo.cwidth+1);
		  
		  j0=j;
		  if(strcmp(&displayed_text[xrow2],&(cScreen->text[xrow]))==0)
		    {
		      trow = xrow + MyWinInfo.cwidth+1;
		      trow2 = xrow2 + MyWinInfo.cwidth+1;
		      count = 1;
		      j++;
		      while((j<MyWinInfo.cheight)&&
			    (strcmp(&displayed_text[trow2],
				    &(cScreen->text[trow]))==0))
			{
			  trow += MyWinInfo.cwidth+1;
			  trow2 += MyWinInfo.cwidth+1;		      
			  count++;
			  j++;
			}
		      j--;
		      XCopyArea(display,vt_win,vt_win,gc,
				MARGIN,k*MyWinInfo.fheight+MARGIN,
				MyWinInfo.pwidth,
				count*MyWinInfo.fheight,
				MARGIN,j0*MyWinInfo.fheight+MARGIN);

		      /* Forward overlapping memcpy's are probably OK, 
		       * but backwards doesn't work on SunOS 4.1.3 */
		      k *=  (MyWinInfo.cwidth+1);
		      j0 *=  (MyWinInfo.cwidth+1);
		      if(k>j0)
			{
			  for(l = 0; l< count;l++)
			    {
			      memcpy(&displayed_text[j0], &displayed_text[k],
				     (MyWinInfo.cwidth+1));
			      memcpy(&displayed_rend[j0], &displayed_rend[k],
				     (MyWinInfo.cwidth+1)*RENDSIZE);
			      k += MyWinInfo.cwidth+1;
			      j0 += MyWinInfo.cwidth+1;
			    }
			}
		      else
			{
			  k += (count-1)*(MyWinInfo.cwidth+1);
			  j0 += (count-1)*(MyWinInfo.cwidth+1);
			  for(l = count-1; l>=0;l--)
			    {
			      memcpy(&displayed_text[j0], &displayed_text[k],
				     (MyWinInfo.cwidth+1));
			      memcpy(&displayed_rend[j0], &displayed_rend[k],
				     (MyWinInfo.cwidth+1)*RENDSIZE);
			      k -= MyWinInfo.cwidth+1;
			      j0 -= MyWinInfo.cwidth+1;
			    }
			}
		      k=MyWinInfo.cheight;
		    }
		}
	    }
	}
    }
#endif

  xcursor = (cScreen->row + MyWinInfo.saved_lines)*(MyWinInfo.cwidth+1) 
    + cScreen->col;
  if (cScreen->cursor_is_visible) /* Matthias */  
    cScreen->rendition[xcursor] |= RS_CURSOR;
  d_xcursor = (cScreen->row + MyWinInfo.offset);
  if(d_xcursor >= MyWinInfo.cheight)
    {
      d_xcursor = 0;
      cScreen->rendition[xcursor] &= ~RS_CURSOR;
    }
  d_xcursor = d_xcursor * (MyWinInfo.cwidth+1)  + cScreen->col;


  /* For a first cut, do it one character at a time */
  for(j=0;j<(MyWinInfo.cheight);j++)
    {
      xrow=(j+MyWinInfo.saved_lines-MyWinInfo.offset)*(MyWinInfo.cwidth+1);
      xrow2 = j*(MyWinInfo.cwidth+1);
      y1 = j*MyWinInfo.fheight + MARGIN + mainfont->ascent;
      force_next = 0;
      for(k=0;k<MyWinInfo.cwidth;k++)
	{	
	  x=xrow+k;
	  x2 = xrow2+k;
	  fn = force_next;
	  force_next = 0;
	  /* replaced force_next with fn here to fix screen garbage. Mathias */ 
	  if(fn ||(displayed_text[x2]!=cScreen->text[x])||
	     (displayed_rend[x2]!=cScreen->rendition[x]))
	    {
	      if(displayed_rend[x2] & RS_BOLD)
		{
		  force_next = 1;
		}

	      rval = cScreen->rendition[x];
	      ch[0] = cScreen->text[x];
	      displayed_text[x2]= cScreen->text[x];
	      displayed_rend[x2]= cScreen->rendition[x];
	      x++;
	      k0=k+1;
	      k1 = k+2;
	      n=1;


	      while((k0<MyWinInfo.cwidth)&&
		    ((displayed_text[xrow2+k0]!=cScreen->text[x])||
		     (((k0+1)<MyWinInfo.cwidth)&&
		      (displayed_text[xrow2+k0+1]!= cScreen->text[x+1]))||
		     (cScreen->rendition[x] != displayed_rend[xrow2+k0]))&&
		    (cScreen->rendition[x]==rval))
		{
		  displayed_text[xrow2+k0]= cScreen->text[x];
		  displayed_rend[xrow2+k0++]= cScreen->rendition[x];
		  ch[n++] = cScreen->text[x++];
		  if(displayed_rend[x2++] & RS_BOLD)
		    {
		      force_next = 1;
		    }
		}

	      outline = False;
	      thisGC = gc;
	      
	      fore = ( rval >>8 ) & ( 0x0ff);
	      back = ( rval >>16 ) & ( 0x0ff);
	      
	      rval = rval & 0x0ff;
	      if(rval != 0)
		{
		  if((rval & RS_RVID)&&(rval & RS_SELECTED))
		    {
		      rval &= ~RS_RVID;
		      rval &= ~RS_SELECTED;
		    }
		  
		  if((!focus)&&(rval & RS_CURSOR))
		    {
		      outline = True;
		      rval &= ~RS_CURSOR;
		    }

		  if((rval & RS_CURSOR)&&(rval &(RS_RVID|RS_SELECTED)))
		    {
		      rval &= ~RS_CURSOR;
		      rval &= ~RS_RVID;
		      rval &= ~RS_SELECTED;
		    }

		  if (rval & (RS_RVID | RS_SELECTED | RS_CURSOR)) 
		    thisGC = rvgc;

		  if(rval & RS_GBFONT)
		    {
		      for(x=0;x<n;x++)
			if (ch[x] == '#') ch[x] = '\036';
		    }
		  if(rval & RS_GRFONT)
		    {
/* 		      printf("check GRFONT '"); */
		      for(x=0;x<n;x++){
/* 			printf("%c", ch[x]); */
			/* handle 0 and h --- see below (Matthias) */
			if (ch[x] == '0'){
			  ch[x] = ' ';
			}
			else if (ch[x] == 'h'){
			  ch[x] = ' ';
			}
			if (ch[x] >=0x5f && ch[x] <=0x7e){
			  ch[x] = (ch[x] == 0x5f) ? 0x7f : ch[x] - 0x5f;
			}
		      }
/* 		      printf("\n"); */
		    }
		}
#ifdef COLOR
	      
	      if (rval & (RS_RVID | RS_SELECTED | RS_CURSOR)) {
		switch (color_type){
		case COLOR_TYPE_Linux:
		  if (rval & RS_BOLD)
		    newgcv.foreground = pixel_colors[back==1?0:back+8];
		  else 
		    newgcv.foreground = pixel_colors[back];
		  newgcv.background = pixel_colors[fore];
		  break;
		case COLOR_TYPE_ANSI:
		  newgcv.foreground = pixel_colors[back];
		  newgcv.background = pixel_colors[fore];
		  break;
		default: /* COLOR_TYPE_DEFAULT */
		  newgcv.foreground = pixel_colors[back];
		  newgcv.background = pixel_colors[fore<3?fore:fore+8];
		  break;
		}
		  
	      } else {

		switch (color_type){
		case COLOR_TYPE_Linux:
		  if (rval & RS_BOLD){
		    if (fore >= 2) {
		      newgcv.foreground = pixel_colors[fore+8];
		    } else {
		      newgcv.foreground = pixel_colors[STD_BOLD_COLOR];
		    }
		  } else {
		    newgcv.foreground = pixel_colors[fore];
		  }
		  newgcv.background = pixel_colors[back];
		  break;
		case COLOR_TYPE_ANSI:
		  newgcv.foreground = pixel_colors[fore];
		  newgcv.background = pixel_colors[back];
		  break;
		default: /* COLOR_TYPE_DEFAULT */
		  newgcv.foreground = pixel_colors[fore<3?fore:fore+8];
		  newgcv.background = pixel_colors[back];
		  break;
		}
	      }

	      newgcm = GCBackground | GCForeground;
	      XChangeGC(display,thisGC,newgcm,&newgcv);
#endif
	      x1 = k*MyWinInfo.fwidth + MARGIN;
	      
  	      XDrawImageString(display,vt_win,thisGC,x1,y1,ch,n);  

	      if(rval != 0) {
		if (rval & RS_BOLD && color_type != COLOR_TYPE_Linux) {
		  XDrawString(display,vt_win,thisGC,x1+1,y1,ch,n);
		}	      
		  
		/* On the smallest font, underline overwrites the next row */
		if ((rval & RS_ULINE)&&(mainfont->descent > 1))
		  XDrawLine(display,vt_win,thisGC,x1,y1+1,
			    x1+n*MyWinInfo.fwidth-1,y1+1);
		  
		/* support for ACS_BOARD and ACS_BLOCK (Matthias) */ 
		if(rval & RS_GRFONT) {
		  for(x=0;x<n;x++){
		    if (displayed_text[xrow2 + k + x] == '0'){
		      XSetFillStyle(display, thisGC, FillSolid); 
		      XFillRectangle(display, vt_win, thisGC, 
				     x1 + x*MyWinInfo.fwidth,
				     y1 - mainfont->ascent,
				     MyWinInfo.fwidth,
				     MyWinInfo.fheight);
		    }
		    else if (displayed_text[xrow2 + k + x] == 'h'){
		      if (!stipple_data_pixmap){
			stipple_data_pixmap = XCreateBitmapFromData(
								    display, vt_win, 
								    stipple_data_bits,
								    stipple_data_width, 
								    stipple_data_height);
		      }
		      XSetStipple(display, thisGC, stipple_data_pixmap);
		      XSetFillStyle(display, thisGC, FillStippled);
		      XFillRectangle(display, vt_win, thisGC, 
				     x1 + x*MyWinInfo.fwidth,
				     y1 - mainfont->ascent,
				     MyWinInfo.fwidth,
				     MyWinInfo.fheight); 
		      XSetFillStyle(display, thisGC, FillSolid); 
		    }
		  }
		}
	      }
#ifdef COLOR
	      if(newgcm != 0)
		{
		  if(thisGC == rvgc)
		    {
		      newgcv.foreground = pixel_colors[1];
		      newgcv.background = pixel_colors[0];
		    }
		  else
		    {
		      newgcv.foreground = pixel_colors[0];
		      newgcv.background = pixel_colors[1];
		    }
		  XChangeGC(display,thisGC,newgcm,&newgcv);
		  newgcm = 0;
		}
#endif
	      if(outline)
		{
		  XDrawRectangle(display,vt_win,thisGC,x1,
				 y1-mainfont->ascent,
				 MyWinInfo.fwidth-1,MyWinInfo.fheight-1);
		}
	      if(k0-1>k)k=k0-1;
	    }
	}
    }
  if (cScreen->cursor_is_visible)  /* Matthias */
    cScreen->rendition[xcursor] &= ~RS_CURSOR;
}

/**************************************************************************
 * If value == 1, sets a new tab stop at the current location
 * If value == 0, cleats any tab stops at the current location
 * If value == -1, clears all tabs
 *************************************************************************/
void scr_set_tab(int value)
{
  int i;

#ifdef DEBUG
  check_text("set_tab");  
#endif

  if(value < 0)
    {
      for(i=0;i<MyWinInfo.cwidth;i++)
	tabs[i] = 0;
    }
  else
    {
      if(cScreen->col < MyWinInfo.cwidth)
	{
	  tabs[cScreen->col] = value;
	}
    }
}

/***************************************************************************
 * Does some simple checking about the integrity of the screen data structures
 ***************************************************************************/
#ifdef DEBUG
void check_text(char *a)
{
  int i;
  static char *previous = "a";

  fprintf(stderr,"%s\n",a);
  for(i=0; i< (MyWinInfo.cheight + MyWinInfo.saved_lines);i++)
    if(cScreen->text[i*(MyWinInfo.cwidth+1)+MyWinInfo.cwidth] != 0)
      {
	fprintf(stderr,"%s: %s Help! violation on row %d\n",a,previous,i);
	exit(0);
      }
  check_all_mem("check_text",a);
  previous = a;


  
}
#endif


/******************************************************************************
 * Toggle reverse video settings 
 *****************************************************************************/
void scr_rev_vid(int mode)
{
  static int current = LOW;
  GC t;
#ifdef DEBUG
  check_text("rev_vid");  
#endif

  if(current == mode)
    return;
  else
    {
      /* switch modes */
      t = rvgc;
      rvgc = gc;
      gc = t;
      if(mode == HIGH)
	XSetWindowBackground(display,vt_win,foreground);
      else
	XSetWindowBackground(display,vt_win,background);
      XClearWindow(display,vt_win);

      scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);
    }
  current = mode;
}

/***************************************************************************
 * Free a color. [bmg]
 **************************************************************************/
void free_color(int color)
{
  XFreeColors(display, colormap, &pixel_colors[color], 1, 0);
  colors_loaded[color] = 0;
}

/***************************************************************************
 * Allocate a color. [bmg]
 **************************************************************************/
int alloc_color(int color)
{
  XColor new_color;

  if (XParseColor(display, colormap, color_names[color], &new_color) == 0) {
    error("invalid color %s", color_names[color]);
    return(0);
  } else if (XAllocColor(display, colormap, &new_color) == 0) {
    error("can't allocate color %s", color_names[color]);
    return(0);
  } else {
    pixel_colors[color] = new_color.pixel;
    colors_loaded[color] = 1;
    return(1);
  }
}

/***************************************************************************
 * Set the text foreground color
 **************************************************************************/
void scr_fore_color(int color)
{
#ifdef DEBUG
  check_text("set_color");  
#endif
  color -= 30;
  
  /*   if((color <0)||(color > 7)) */ /* Matthias */ 
  if((color <-2)||(color > 7))
    return;
  
  if(colors_loaded[color+2]) {
    fore_color = color+2;
    rstyle = (rstyle &(0xffff00ff) ) | (fore_color <<8);
  } else if (alloc_color(color+2)) {
    fore_color = color+2;
    rstyle = (rstyle &(0xffff00ff) ) | (fore_color <<8);
  }
  if (color_type != COLOR_TYPE_ANSI && color >= 0 && 
      !colors_loaded[color+10]) {
    alloc_color(color+10);
  }
}  

/***************************************************************************
 * Set the text background color
 **************************************************************************/
void scr_back_color(int color)
{
#ifdef DEBUG
  check_text("set_back_color");  
#endif
  color -= 40;

  /*   if((color <0)||(color > 7)) */ /* Matthias */ 
  if((color <-2)||(color > 7))
    return;


  if(colors_loaded[color+2]) {
    back_color = color+2;
    rstyle = (rstyle &(0xff00ffff) ) | (back_color <<16);
  } else if (alloc_color(color+2)) {
    back_color = color+2;
    rstyle = (rstyle &(0xff00ffff) ) | (back_color <<16);
  }
  if (color_type != COLOR_TYPE_ANSI && color >=0 && 
      !colors_loaded[color+10]) {
    alloc_color(color+10);
  }
}

/***************************************************************************
 * Report mouse event to application
 **************************************************************************/
#define ButtonNumber(x) ((x) == AnyButton ? 3 : ((x) - Button1))
#define KeyState(x) ((((x)&(ShiftMask|ControlMask))+(((x)&Mod1Mask)?2:0))<<2)

void mouse_report (XButtonEvent * ev, int release)
{
   int y = (ev->y - MARGIN)/MyWinInfo.fheight;
   int x = (ev->x - MARGIN)/MyWinInfo.fwidth;
   cprintf ("\033[M%c%c%c",
              ((release > 0) ? '#' : (040 + ButtonNumber (ev->button) +
			KeyState (ev->state))),
              (041 + x),
              (041 + y));
}

/****************************************************************************
 * Say whether this window is currently in focus
 ***************************************************************************/
int scr_has_focus()
{
   return focus;
}
