#ifndef __XLOCK_XLOCK_H__
#define __XLOCK_XLOCK_H__

/*-
 * @(#)xlock.h	4.00 97/01/01 xlockmore 
 *
 * xlock.h - external interfaces for new modes and SYSV OS defines.
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 *
 * Changes of David Bagley <bagleyd@bigfoot.com>
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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* out of main.cpp */
#define MODE_NONE               0
#define MODE_INSTALL    1
#define MODE_SETUP              2
#define MODE_PREVIEW    3
#define MODE_TEST               4


#ifndef MAXSCREENS
#define MAXSCREENS        3
#endif
#ifndef NUMCOLORS
#define NUMCOLORS         64
#endif
#ifndef MESSAGELINES
#define MESSAGELINES      40
#endif
#define PASSLENGTH        120
#define FALLBACK_FONTNAME "fixed"
#ifndef DEF_MFONT
#define DEF_MFONT "-*-times-*-*-*-*-18-*-*-*-*-*-*-*"
#endif
#ifndef DEF_PROGRAM		/* Try the -o option ;) */
#define DEF_PROGRAM "fortune -s"
#endif

#ifndef DEF_NONE3D
#define DEF_NONE3D "Black"
#endif
#ifndef DEF_RIGHT3D
#define DEF_RIGHT3D "Red"
#endif
#ifndef DEF_LEFT3D
#define DEF_LEFT3D "Blue"
#endif
#ifndef DEF_BOTH3D
#define DEF_BOTH3D "Magenta"
#endif

#ifndef DEF_ICONW
#define DEF_ICONW         64	/* Age old default */
#endif
#ifndef DEF_ICONH
#define DEF_ICONH         64
#endif

#define MINICONW          1	/* Will not see much */
#define MINICONH          1

#ifndef MAXICONW
#define MAXICONW          256	/* Want users to know the screen is locked */
#endif
#ifndef MAXICONH
#define MAXICONH          256	/* by a particular user */
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef ABS
#define ABS(a)  ((a<0)?(-(a)):(a))
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
#if VMS
#include <unixlib.h>
#endif
#include <sys/types.h>
#if 0
#ifndef uid_t
#define uid_t int
#endif
#ifndef gid_t
#define gid_t int
#endif
#ifndef pid_t
#define pid_t int
#endif
#ifndef size_t
#define size_t unsigned
#endif
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_MEMORY_H
#include <memory.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if HAVE_DIRENT_H
#ifdef USE_XVMSUTILS
#if 0
#include "../xvmsutils/unix_types.h"
#include "../xvmsutils/dirent.h"
#else
#include <X11/unix_types.h>
#include <X11/dirent.h>
#endif
#else /* !USE_XVMSUTILS */
#include <dirent.h>
#endif /* !USE_XVMSUTILS */
#else
#define dirent direct
#define NAMELEN(dirent) (dirent)->d_namelen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif
#ifndef MAXNAMLEN
#define MAXNAMLEN       256	/* maximum filename length */
#endif
#ifndef DIRBUF
#define DIRBUF          512	/* buffer size for fs-indep. dirs */
#endif

typedef struct {
	GC          gc;		/* graphics context for animation */
	int         npixels;	/* number of valid entries in pixels */
	Colormap    cmap;	/* current colormap */
	unsigned long pixels[NUMCOLORS];	/* pixel values in the colormap */
	unsigned long bgcol, fgcol;	/* background and foreground pixel values */
	unsigned long rightcol, leftcol;	/* 3D color pixel values */
	unsigned long nonecol, bothcol;
} perscreen;

#define t_String        0
#define t_Float         1
#define t_Int           2
#define t_Bool          3

typedef struct {
	caddr_t    *var;
	char       *name;
	char       *classname;
	char       *def;
	int         type;
} argtype;

typedef struct {
	char       *opt;
	char       *desc;
} OptionStruct;

typedef struct {
	int         numopts;
	XrmOptionDescRec *opts;
	int         numvarsdesc;
	argtype    *vars;
	OptionStruct *desc;
} ModeSpecOpt;

/* this must follow definition of ModeSpecOpt */
#include "mode.h"

#define IS_XBMDONE 1		/* Only need one mono image */
#define IS_XBM 2
#define IS_XBMFILE 3
#define IS_XPM 4
#define IS_XPMFILE 5
#define IS_RASTERFILE 6

extern void getResources(int argc, char **argv);
extern unsigned long allocPixel(Display * display, Colormap cmap,
				char *name, char *def);
extern void setColormap(Display * display, Window window, Colormap map,
			Bool inwindow);
extern void reserveColors(ModeInfo * mi, Colormap cmap,
			  unsigned long *blackpix, unsigned long *whitepix);
extern void fixColormap(Display * display, Window window,
			int screen, float saturation,
	  Bool mono, Bool install, Bool inroot, Bool inwindow, Bool verbose);
extern int  visualClassFromName(char *name);
extern void showVisualInfo(XVisualInfo * wantVis);
#ifdef USE_GL
extern int getVisual(ModeInfo * mi, XVisualInfo * wantVis, int mono);
#endif

extern unsigned long seconds(void);
extern void finish(void);
extern void error(char *s1,...);
extern void warning(char *s1,...);

#ifndef HAVE_STRDUP 
extern char *strdup(char *);
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

#endif /* __XLOCK_XLOCK_H__ */
