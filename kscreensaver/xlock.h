#ifndef __XLOCK_H__
#define __XLOCK_H__

/*-
 * @(#)xlock.h	3.3 95/09/24 xlockmore 
 *
 * xlock.h - external interfaces for new modes and SYSV OS defines.
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 *
 * Changes of David Bagley <bagleyd@source.asset.com>
 * 12-May-95: Added defines for SunOS's Adjunct password file
 *            Dale A. Harris <rodmur@ecst.csuchico.edu>
 * 18-Nov-94: Modified for QNX 4.2 w/ Metrolink X server from Brian Campbell
 *            <brianc@qnx.com>.
 * 11-Jul-94: added Bool flag: inwindow, which tells xlock to run in a
 *            window from Greg Bowering <greg@cs.adelaide.edu.au>
 * 11-Jul-94: patch for Solaris SYR4 from Chris P. Ross <cross@eng.umd.edu>
 * 28-Jun-94: Reorganized shadow stuff
 * 24-Jun-94: Reorganized
 * 22-Jun-94: Modified for VMS
 *            <Anthony.D.Clarke@Support.Hatfield.Raytheon.bae.eurokom.ie>
 * 17-Jun-94: patched shadow passwords and bcopy and bzero for SYSV from
 *            <reggers@julian.uwo.ca>
 * 21-Mar-94: patched the patch for AIXV3 and HP from
 *            <R.K.Lloyd@csc.liv.ac.uk>.
 * 01-Dec-93: added patch for AIXV3 from
 *            (Tom McConnell, tmcconne@sedona.intel.com) also added a patch
 *            for HP-UX 8.0.
 *
 */

#include <qapp.h> // hack for qt-1.2
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>

// added for kscreensave
void initXLock( GC gc );


#define MAXSCREENS        1
#define NUMCOLORS         64
#ifndef KERBEROS
#define PASSLENGTH        64
#else
#define PASSLENGTH        120
#endif
#define FALLBACK_FONTNAME "fixed"
#ifndef DEF_MFONT
#define DEF_MFONT "-*-times-*-*-*-*-18-*-*-*-*-*-*-*"
#endif
#ifndef DEF_PROGRAM  /* Try the -o option ;) */
#define DEF_PROGRAM "fortune -s"
#endif

#define ICONW             64
#define ICONH             64

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#if defined VMS || defined __QNX__
#ifdef VMS
/*#define VMS_PLAY*/
#include <unixlib.h>
#endif
#endif

#include <math.h>

#ifndef M_E
#define M_E    2.7182818284590452354
#endif
#ifndef M_PI
#define M_PI   3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#if !defined (news1800) && !defined (sun386)
#include <stdlib.h>
#if !defined (apollo) && !defined (VMS)
#include <unistd.h>
#include <memory.h>
#endif
#endif
#include <stdio.h>
#include <string.h>
 
typedef struct {
  GC            gc;                /* graphics context for animation */
  int           npixels;           /* number of valid entries in pixels */
  Colormap      cmap;              /* current colormap */
  unsigned long pixels[NUMCOLORS]; /* pixel values in the colormap */
  unsigned long bgcol, fgcol;      /* background and foreground pixel values */
} perscreen;

/* This stuff moved here from resource.c for the mode-specific options. */
#define t_String        0
#define t_Float         1
#define t_Int           2
#define t_Bool          3

typedef struct {
    caddr_t    *var;
    char       *name;
    char       *arg_class;
    char       *def;
    int         type;
} argtype;

typedef struct {
    char       *opt;
    char       *desc;
} OptionStruct;

typedef struct {
    int              numopts;
    XrmOptionDescRec *opts;
    argtype          *vars;
    OptionStruct     *desc;
} ModeSpecOpt;
/* End moved from resource.c */

extern perscreen Scr[MAXSCREENS];
extern Display *dsp;
extern int  screen;

extern char  *ProgramName;
extern char  *fontname;
extern char  *background;
extern char  *foreground;
extern char  *text_name;
extern char  *text_pass;
extern char  *text_info;
extern char  *text_valid;
extern char  *text_invalid;
extern char  *geometry;
extern float saturation;
extern int   nicelevel;
extern int   delay;
extern int   batchcount;
extern int   cycles;
extern int   timeout;
extern int   lockdelay;
#if defined(HAS_RPLAY) || defined(VMS_PLAY)
extern char  *locksound;
extern char  *infosound;
extern char  *validsound;
/*extern char  *invalidsound;*/
#endif
#ifdef AUTO_LOGOUT
extern int   forceLogout;
#endif
#ifdef LOGOUT_BUTTON
extern int   enable_button;
extern char  *logoutButtonLabel;
extern char  *logoutButtonHelp;
extern char  *logoutFailedString;
#endif
extern Bool  usefirst;
extern Bool  mono;
extern Bool  nolock;
extern Bool  allowroot;
extern Bool  enablesaver;
extern Bool  allowaccess;
extern Bool  grabmouse;
extern Bool  echokeys;
extern Bool  verbose;
extern Bool  inwindow;
extern Bool  inroot;
extern Bool  timeelapsed;
extern Bool  install;
extern int   onepause;

/* For modes with text, marquee & nose */
extern char *program;
extern char *messagesfile;
extern char *messagefile;
extern char *message;
extern char *mfont;

extern void  (*callback) ();
extern void  (*init) ();

extern void GetResources();
extern void set_colormap();
extern void fix_colormap();
#ifdef __STDC__
extern void error(char *, ...);
#else
extern void error();
#endif
extern void alarm_ctl();
extern long seconds();

/* For modes with text, marquee & nose */
extern XFontStruct *get_font();
extern char *get_words();
extern void init_words();
extern int is_ribbon();

#ifdef LESS_THAN_AIX3_2
#undef NULL
#define NULL 0
#endif /* LESS_THAN_AIX3_2 */

#ifdef VMS
#define OLD_EVENT_LOOP
#endif

#if defined(__STDC__) && (defined(__hpux) && defined(_PA_RISC1_1))
#define MATHF
#endif
#ifdef MATHF
#define SINF(n) sinf(n)
#define COSF(n) cosf(n)
#define FABSF(n) fabsf(n)
#else
#define SINF(n) ((float)sin((double)(n)))
#define COSF(n) ((float)cos((double)(n)))
#define FABSF(n) ((float)fabs((double)(n)))
#endif

/*** random number generator ***/
/* We can always use random and srandom.
   They are provided by fakes.cpp if they're not present */
#define SRAND srandom
#define LRAND random
#ifndef MAXRAND
#define MAXRAND (2147483648.0)
#endif

#define NRAND(X) ((int)(LRAND()%(X)))

#endif /* __XLOCK_H__ */
