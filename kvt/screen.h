
/*
 * This module is all new by Robert Nation
 * (nation@rocket.sanders.lockheed.com).
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 */

#ifndef SCREEN_H
#define SCREEN_H

#if defined(__GNUC__) || defined(__STRICT_ANSI__)
#define  inline  /*nothing*/
#endif /*__GNUC__*/

#ifndef COLOR
#define COLOR
#endif

typedef struct _win_info
{
  /* window width and height in pixels */
  int pwidth,pheight;
  /* window width and height in characters */
  int cwidth, cheight;
  /* font width and height in pixels */
  int fwidth, fheight;
  /* total lines to save in scroll-back buffer */
  int saved_lines;
  /* how far back we are in the scroll back buffer */
  int offset;
  /* high water mark of saved scrolled lines */
  int sline_top;
} WindowInfo;

typedef struct _screen_info
{
  /* All text including the scroll back buffer.
   * each line is a fixed length == cwidth+1
   * last character is 0 for non-wrapped lines,
   * 1 otherwise */
  unsigned char *text; 
  /* renditions, using flags defined later */
#ifdef COLOR
  unsigned int *rendition;
#else
  unsigned char *rendition;
#endif
  /* cursor position */
  int row,col;
  /* top and bottom scroll margins */
  int tmargin,bmargin;
  /* origin mode flag */
  int decom;
  /* auto-wrap flag */
  int wrap;
  /* need to wrap for next char? */
  int wrap_next;
  /* insert mode (vs. replace mode) */
  int insert;
  /* character set number (0,1) */
  int charset;
  int cursor_is_visible;		/* cursor visible? (Matthias)*/
} ScreenInfo;

#define NSCREENS 2

/* Screen refresh methods */
#define DONT_BOTHER 2     /* Window not visible at all! */
#define SLOW 1           /* No Bitblt */
#define FAST 0

/*  flags for scr_move()
 */
#define COL_RELATIVE	1	/* column movement is relative */
#define ROW_RELATIVE	2	/* row movement is relative */

/*  arguments to the screen delete functions
 */
#define END	0
#define START	1
#define ENTIRE	2

/* modes for scr_insert_delete_lines()
 */
#define INSERT -1
#define DELETE 1

/* modes for scr_move_up_down
 */
#define UP 1
#define DOWN -1

/*  rendition style flags.
 */
#define RS_NONE		0	/* Normal */
#define RS_BOLD		1	/* Bold face */
#define RS_ULINE	2	/* underline */
#define RS_CURSOR	4	/* cursor location */
#define RS_RVID		8	/* reverse video */
#define RS_GBFONT      16       /* UK character set */
#define RS_GRFONT      32       /* graphics character set */
#define RS_SELECTED    64       /* graphics character set */

/* Cursor on/off statuses 
 */
#define ON 1
#define OFF 0

#define SEL_SET 1
#define SEL_CLR 0

#define SELDIR_POS 1
#define SELDIR_NEG -1

#define SELECTION_CLEAR 0
#define SELECTION_BEGIN 1
#define SELECTION_INIT 2
#define SELECTION_ONGOING 3
#define SELECTION_COMPLETE 4

/* Multiselect. bmg */
#define SELECTION_MODE_CHAR 0
#define SELECTION_MODE_WORD 1
#define SELECTION_MODE_LINE 2

#define SELECT_WORD_START 0
#define SELECT_WORD_EXPAND 1

/* support of Linux-Color-Model */

#define COLOR_TYPE_ANSI 0
#define COLOR_TYPE_Linux 1
#define COLOR_TYPE_DEFAULT 2

void screen_init(void);
void scrn_reset(void);
void scr_power_on(void);
void set_charclass(const char *s);
void init_color_mode(int mode);
void set_color_mode(int mode);
int get_color_mode(void);
void scr_cursor_visible (int); /* Matthias */ 
void scr_backspace(void);
void scr_change_screen(int);
void scr_change_rendition(int,int);
void scr_get_size(int *,int *);
void scr_focus(int);
inline void scr_add_character(int c);
void scr_add_lines(unsigned char *c,int nl_count,int n);
void scr_move(int,int,int);
void scr_index(int);
void scr_save_cursor(void);
void scr_restore_cursor(void);
void scr_erase_line(int);
void scr_erase_char(int n);
void scr_erase_screen(int);
void scr_insert_delete_lines(int,int);
void scr_insert_delete_characters(int,int);
void scr_set_margins(int,int);
void scr_set_wrap(int);
void scr_set_decom(int);
void scr_set_insert(int);
void scr_move_to(int);
void scr_move_up_down(int);
void scr_make_selection(int);
void scr_paste_selection();
void scr_clear_selection(void);
void scr_extend_selection(int,int);
void scr_start_selection(unsigned int, int, int); /* arguments changed. bmg */
void scr_report_position(void);
void scr_refresh(int,int,int,int);
void scr_set_charset(int set,unsigned char a);
void scr_choose_charset(int set);
void screen_refresh(void);
inline void check_selection(int row, int col);
void scr_secure(void);
void set_font_style(void);
int  screen_scroll(int row1,int row2,int count);
void scr_set_tab(int value);
void scr_E(void);
void scr_rev_vid(int mode);
void mouse_report(XButtonEvent *ev, int release);
int scr_has_focus(); /* Leon, needed for clipboard */
#endif /* SCREEN_H */
