/* xscreensaver, Copyright (c) 1998 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * Blue Screen of Death: the finest in personal computer emulation.
 * Concept cribbed from Stephen Martin <smartin@mks.com>;
 * this version written by jwz, 4-Jun-98.
 *
 *   TODO:
 *      -  Should simulate a Unix kernel panic and reboot.
 *      -  Making various boot noises would be fun, too.
 *      -  Maybe scatter some random bits across the screen,
 *         to simulate corruption of video ram?
 *      -  Should randomize the various hex numbers printed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#ifndef __FreeBSD__
#include <time.h>
#endif

#include "bsod.h"

#include <qpainter.h>
#include <qstring.h>
#include <qwindowdefs.h>

#include <kapp.h>
#include <kconfig.h>

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

#ifdef HAVE_XPM
# include <X11/xpm.h>
# include "amiga.xpm"
#endif

#include "atari.xbm"
#include "mac.xbm"

#ifndef isupper
# define isupper(c)  ((c) >= 'A' && (c) <= 'Z')
#endif
#ifndef _tolower
# define _tolower(c)  ((c) - 'A' + 'a')
#endif

extern KLocale *glocale;

#define progname "bsod"
#define MAXDELAY 50
#define MINDELAY 3
#define DEFDELAY 10

extern "C" {
  char *get_string_resource (const char*, char*);
  int get_integer_resource (char*, char*);
  unsigned int get_pixel_resource (const char*, char*, Colormap);
  static Bool macsbug (Window window, int delay);
}

char *
get_string_resource (const char *res_name, char *res_class)
{
  KConfig *config = KApplication::getKApplication()->getConfig();
  config->setGroup("Default");
  return (char*)config->readEntry(res_name,config->readEntry(res_class,0)).data();
}

int 
get_integer_resource (char *res_name, char *res_class)
{
  int val;
  char c, *s = get_string_resource (res_name, res_class);
  char *ss = s;
  if (!s) return 0;

  while (*ss && *ss <= ' ') ss++;			/* skip whitespace */

  if (ss[0] == '0' && (ss[1] == 'x' || ss[1] == 'X'))	/* 0x: parse as hex */
    {
      if (1 == sscanf (ss+2, "%x %c", &val, &c))
	{
	  //free (s);
	  return val;
	}
    }
  else							/* else parse as dec */
    {
      if (1 == sscanf (ss, "%d %c", &val, &c))
	{
	  //free (s);
	  return val;
	}
    }

  fprintf (stderr, "%s: %s must be an integer, not %s.\n",
	   progname, res_name, s);
  return 0;
}

unsigned int
get_pixel_resource (const char *res_name, char *res_class,
		    Colormap cmap)
{
  XColor color;
  char *s = get_string_resource (res_name, res_class);
  char *s2;
  if (!s) goto DEFAULT;

  for (s2 = s + strlen(s) - 1; s2 > s; s2--)
    if (*s2 == ' ' || *s2 == '\t')
      *s2 = 0;
    else
      break;

  if (! XParseColor (qt_xdisplay(), cmap, s, &color))
    {
      fprintf (stderr, "%s: can't parse color %s\n", progname, s);
      goto DEFAULT;
    }
  if (! XAllocColor (qt_xdisplay(), cmap, &color))
    {
      fprintf (stderr, "%s: couldn't allocate color %s\n", progname, s);
      goto DEFAULT;
    }
  //free (s);
  return color.pixel;
 DEFAULT:
  //if (s) free (s);
  return ((strlen(res_class) >= 10 &&
	   !strcmp ("Background", res_class + strlen(res_class) - 10))
	  ? BlackPixel (qt_xdisplay(), DefaultScreen (qt_xdisplay()))
	  : WhitePixel (qt_xdisplay(), DefaultScreen (qt_xdisplay())));
}

static void
draw_string (Window window, GC gc, XGCValues *gcv,
	     XFontStruct *font,
	     int xoff, int yoff,
	     int win_width, int win_height,
	     const char *string, int _delay)
{
  int x, y;
  int width = 0, height = 0, cw = 0;
  int char_width, line_height;

  const char *s = string;
  const char *se = string;

  /* This pretty much assumes fixed-width fonts */
  char_width = (font->per_char
		? font->per_char['n'-font->min_char_or_byte2].width
		: font->min_bounds.width);
  line_height = font->ascent + font->descent + 1;

  while (1)
    {
      if (*s == '\n' || !*s)
	{
	  height++;
	  if (cw > width) width = cw;
	  cw = 0;
	  if (!*s) break;
	}
      else
	cw++;
      s++;
    }

  x = (win_width - (width * char_width)) / 2;
  y = (win_height - (height * line_height)) / 2;

  if (x < 0) x = 2;
  if (y < 0) y = 2;

  x += xoff;
  y += yoff;

  se = s = string;
  while (1)
    {
      if (*s == '\n' || !*s)
	{
	  int off = 0;
	  Bool flip = False;

	  if (*se == '@' || *se == '_')
	    {
	      if (*se == '@') flip = True;
	      se++;
	      off = (char_width * (width - (s - se))) / 2;
	    }

	  if (flip)
	    {
	      XSetForeground(qt_xdisplay(), gc, gcv->background);
	      XSetBackground(qt_xdisplay(), gc, gcv->foreground);
	    }

	  if (s != se)
	    XDrawImageString(qt_xdisplay(), window, gc, x+off, y+font->ascent, se, s-se);

	  if (flip)
	    {
	      XSetForeground(qt_xdisplay(), gc, gcv->foreground);
	      XSetBackground(qt_xdisplay(), gc, gcv->background);
	    }

	  se = s;
	  y += line_height;
	  if (!*s) break;
	  se = s+1;

	  if (_delay)
	    {
	      XSync(qt_xdisplay(), False);
	      usleep(_delay);
	    }
	}
      s++;
    }
}


static Pixmap
double_pixmap(GC gc, Visual *visual, int depth, Pixmap pixmap,
	     int pix_w, int pix_h)
{
  int x, y;
  Pixmap p2 = XCreatePixmap(qt_xdisplay(), pixmap, pix_w*2, pix_h*2, depth);
  XImage *i1 = XGetImage(qt_xdisplay(), pixmap, 0, 0, pix_w, pix_h, ~0L, ZPixmap);
  XImage *i2 = XCreateImage(qt_xdisplay(), visual, depth, ZPixmap, 0, 0,
			    pix_w*2, pix_h*2, 8, 0);
  i2->data = (char *) calloc(i2->height, i2->bytes_per_line);
  for (y = 0; y < pix_h; y++)
    for (x = 0; x < pix_w; x++)
      {
	unsigned long p = XGetPixel(i1, x, y);
	XPutPixel(i2, x*2,   y*2,   p);
	XPutPixel(i2, x*2+1, y*2,   p);
	XPutPixel(i2, x*2,   y*2+1, p);
	XPutPixel(i2, x*2+1, y*2+1, p);
      }
  free(i1->data); i1->data = 0;
  XDestroyImage(i1);
  XPutImage(qt_xdisplay(), p2, gc, i2, 0, 0, 0, 0, i2->width, i2->height);
  free(i2->data); i2->data = 0;
  XDestroyImage(i2);
  XFreePixmap(qt_xdisplay(), pixmap);
  return p2;
}


/* Sleep for N seconds and return False.  But if a key or mouse event is
   seen, discard all pending key or mouse events, and return True.
 */
static Bool
bsod_sleep(int seconds)
{
  XEvent event;
  int q = seconds * 4;
  int mask = KeyPressMask|ButtonPressMask;
  do
    {
      XSync(qt_xdisplay(), False);
      if (XCheckMaskEvent(qt_xdisplay(), mask, &event))
	{
	  while (XCheckMaskEvent(qt_xdisplay(), mask, &event))
	    ;
	  stopScreenSaver();
	  return True;
	}
      if (q > 0)
	{
	  q--;
	  usleep(250000);
	}
    }
  while (q > 0);

  return False; 
}


static Bool
windows (Window window, int delay, Bool w95p)
{
  XGCValues gcv;
  XWindowAttributes xgwa;
  char *fontname;
  const char *def_font = "fixed";
  XFontStruct *font;
  GC gc;
  KConfig *config = KApplication::getKApplication()->getConfig();

  const char *w95 =
    ("@Windows\n"
     "A fatal exception 0E has occured at F0AD:42494C4C\n"
     "the current application will be terminated.\n"
     "\n"
     "* Press any key to terminate the current application.\n"
     "* Press CTRL+ALT+DELETE again to restart your computer.\n"
     "  You will lose any unsaved information in all applications.\n"
     "\n"
     "\n"
     "_Press any key to continue");

  const char *wnt =
    ("*** STOP: 0x0000001E (0x80000003,0x80106fc0,0x8025ea21,0xfd6829e8)\n"
   "Unhandled Kernel exception c0000047 from fa8418b4 (8025ea21,fd6829e8)\n"
   "\n"
   "Dll Base Date Stamp - Name             Dll Base Date Stamp - Name\n"
   "80100000 2be154c9 - ntoskrnl.exe       80400000 2bc153b0 - hal.dll\n"
   "80258000 2bd49628 - ncrc710.sys        8025c000 2bd49688 - SCSIPORT.SYS \n"
   "80267000 2bd49683 - scsidisk.sys       802a6000 2bd496b9 - Fastfat.sys\n"
   "fa800000 2bd49666 - Floppy.SYS         fa810000 2bd496db - Hpfs_Rec.SYS\n"
   "fa820000 2bd49676 - Null.SYS           fa830000 2bd4965a - Beep.SYS\n"
   "fa840000 2bdaab00 - i8042prt.SYS       fa850000 2bd5a020 - SERMOUSE.SYS\n"
   "fa860000 2bd4966f - kbdclass.SYS       fa870000 2bd49671 - MOUCLASS.SYS\n"
   "fa880000 2bd9c0be - Videoprt.SYS       fa890000 2bd49638 - NCC1701E.SYS\n"
   "fa8a0000 2bd4a4ce - Vga.SYS            fa8b0000 2bd496d0 - Msfs.SYS\n"
   "fa8c0000 2bd496c3 - Npfs.SYS           fa8e0000 2bd496c9 - Ntfs.SYS\n"
   "fa940000 2bd496df - NDIS.SYS           fa930000 2bd49707 - wdlan.sys\n"
   "fa970000 2bd49712 - TDI.SYS            fa950000 2bd5a7fb - nbf.sys\n"
   "fa980000 2bd72406 - streams.sys        fa9b0000 2bd4975f - ubnb.sys\n"
   "fa9c0000 2bd5bfd7 - usbser.sys         fa9d0000 2bd4971d - netbios.sys\n"
   "fa9e0000 2bd49678 - Parallel.sys       fa9f0000 2bd4969f - serial.SYS\n"
   "faa00000 2bd49739 - mup.sys            faa40000 2bd4971f - SMBTRSUP.SYS\n"
   "faa10000 2bd6f2a2 - srv.sys            faa50000 2bd4971a - afd.sys\n"
   "faa60000 2bd6fd80 - rdr.sys            faaa0000 2bd49735 - bowser.sys\n"
   "\n"
   "Address dword dump Dll Base                                      - Name\n"
   "801afc20 80106fc0 80106fc0 00000000 00000000 80149905 : "
     "fa840000 - i8042prt.SYS\n"
   "801afc24 80149905 80149905 ff8e6b8c 80129c2c ff8e6b94 : "
     "8025c000 - SCSIPORT.SYS\n"
   "801afc2c 80129c2c 80129c2c ff8e6b94 00000000 ff8e6b94 : "
     "80100000 - ntoskrnl.exe\n"
   "801afc34 801240f2 80124f02 ff8e6df4 ff8e6f60 ff8e6c58 : "
     "80100000 - ntoskrnl.exe\n"
   "801afc54 80124f16 80124f16 ff8e6f60 ff8e6c3c 8015ac7e : "
     "80100000 - ntoskrnl.exe\n"
   "801afc64 8015ac7e 8015ac7e ff8e6df4 ff8e6f60 ff8e6c58 : "
     "80100000 - ntoskrnl.exe\n"
   "801afc70 80129bda 80129bda 00000000 80088000 80106fc0 : "
     "80100000 - ntoskrnl.exe\n"
   "\n"
   "Kernel Debugger Using: COM2 (Port 0x2f8, Baud Rate 19200)\n"
   "Restart and set the recovery options in the system control panel\n"
   "or the /CRASHDEBUG system start option. If this message reappears,\n"
   "contact your system administrator or technical support group."
     );

  if (!config->readBoolEntry((w95p? "doWindows" : "doNT"), true))
    return False;

  XGetWindowAttributes (qt_xdisplay(), window, &xgwa);

  fontname = (char *)(config->readEntry((xgwa.height > 600
				   ? (w95p
				      ? "windows95.font2"
				      : "windowsNT.font2")
				   : (w95p
				      ? "windows95.font"
				      : "windowsNT.font")),
"-*-courier-bold-r-*-*-*-120-*-*-m-*-*-*")).data();
  if (!fontname) fontname = (char *)def_font;
  font = XLoadQueryFont (qt_xdisplay(), fontname);
  if (!font) font = XLoadQueryFont (qt_xdisplay(), def_font);
  if (!font) exit(-1);
/*  if (fontname && fontname != def_font)
    free (fontname);*/

  gcv.font = font->fid;
  gcv.foreground = get_pixel_resource((w95p
				       ? "windows95.foreground"
				       : "windowsNT.foreground"),
				      "Windows.Foreground",
				      xgwa.colormap);
  gcv.background = get_pixel_resource((w95p
				       ? "windows95.background"
				       : "windowsNT.background"),
				      "Windows.Background",
				      xgwa.colormap);
  XSetWindowBackground(qt_xdisplay(), window, gcv.background);
  XClearWindow(qt_xdisplay(), window);

  gc = XCreateGC(qt_xdisplay(), window, GCFont|GCForeground|GCBackground,
&gcv);

  if (w95p)
    draw_string(window, gc, &gcv, font,
		0, 0, xgwa.width, xgwa.height, w95, 0);
  else
    draw_string(window, gc, &gcv, font, 0, 0, 10, 10, wnt, 750);

  XFreeGC(qt_xdisplay(), gc);
  XSync(qt_xdisplay(), False);
  bsod_sleep(delay);
  XClearWindow(qt_xdisplay(), window);
  XFreeFont(qt_xdisplay(), font);
  return True;
}

/* SCO OpenServer 5 panic, by Tom Kelly <tom@ancilla.toronto.on.ca>
 */
static Bool
sco (Window window, int delay)
{
  XGCValues gcv;
  XWindowAttributes xgwa;
  char *fontname;
  const char *def_font = "fixed";
  XFontStruct *font;
  GC gc;
  int lines = 1;
  const char *s;
  KConfig *config = KApplication::getKApplication()->getConfig();

  const char *sco_panic =
    ("Unexpected trap in kernel mode:\n"
     "\n"
     "cr0 0x80010013     cr2  0x00000014     cr3 0x00000000  tlb  0x00000000\n"
     "ss  0x00071054    uesp  0x00012055     efl 0x00080888  ipl  0x00000005\n"
     "cs  0x00092585     eip  0x00544a4b     err 0x004d4a47  trap 0x0000000E\n"
     "eax 0x0045474b     ecx  0x0042544b     edx 0x57687920  ebx  0x61726520\n"
     "esp 0x796f7520     ebp  0x72656164     esi 0x696e6720  edi  0x74686973\n"
     "ds  0x3f000000     es   0x43494c48     fs  0x43525343  gs   0x4f4d4b53\n"
     "\n"
     "PANIC: k_trap - kernel mode trap type 0x0000000E\n"
     "Trying to dump 5023 pages to dumpdev hd (1/41), 63 pages per '.'\n"
     "...............................................................................\n"
     "5023 pages dumped\n"
     "\n"
     "\n"
     "**   Safe to Power Off   **\n"
     "           - or -\n"
     "** Press Any Key to Reboot **\n"
    );

  if (!config->readBoolEntry("doSCO", true))
    return False;

  for (s = sco_panic; *s; s++) if (*s == '\n') lines++;

  XGetWindowAttributes (qt_xdisplay(), window, &xgwa);

  fontname = get_string_resource ((xgwa.height > 600
				   ? "sco.font2"
				   : "sco.font"),
				  "SCO.Font");
  if (!fontname || !*fontname) fontname = (char *)def_font;
  font = XLoadQueryFont (qt_xdisplay(), fontname);
  if (!font) font = XLoadQueryFont (qt_xdisplay(), def_font);
  if (!font) exit(-1);

  gcv.font = font->fid;
  gcv.foreground = get_pixel_resource(("sco.foreground"),
				      "SCO.Foreground",
				      xgwa.colormap);
  gcv.background = get_pixel_resource(("sco.background"),
				      "SCO.Background",
				      xgwa.colormap);
  XSetWindowBackground(qt_xdisplay(), window, gcv.background);
  XClearWindow(qt_xdisplay(), window);

  gc = XCreateGC(qt_xdisplay(), window, GCFont|GCForeground|GCBackground, &gcv);

  draw_string(window, gc, &gcv, font,
	      10, xgwa.height - (lines * (font->ascent + font->descent + 1)),
	      10, 10,
	      sco_panic, 0);
  XFreeGC(qt_xdisplay(), gc);
  XSync(qt_xdisplay(), False);
  bsod_sleep(delay);
  XClearWindow(qt_xdisplay(), window);
  XFreeFont(qt_xdisplay(), font);
  return True;
}


/* Linux (sparc) panic, by Tom Kelly <tom@ancilla.toronto.on.ca>
 */
static Bool
sparc_linux (Window window, int delay)
{
  XGCValues gcv;
  XWindowAttributes xgwa;
  char *fontname;
  const char *def_font = "fixed";
  XFontStruct *font;
  GC gc;
  int lines = 1;
  const char *s;
  KConfig *config = KApplication::getKApplication()->getConfig();

  const char *linux_panic =
    ("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
	"Unable to handle kernel paging request at virtual address f0d4a000\n"
	"tsk->mm->context = 00000014\n"
	"tsk->mm->pgd = f26b0000\n"
	"              \\|/ ____ \\|/\n"
	"              \"@'/ ,. \\`@\"\n"
	"              /_| \\__/ |_\\\n"
	"                 \\__U_/\n"
	"gawk(22827): Oops\n"
	"PSR: 044010c1 PC: f001c2cc NPC: f001c2d0 Y: 00000000\n"
	"g0: 00001000 g1: fffffff7 g2: 04401086 g3: 0001eaa0\n"
	"g4: 000207dc g5: f0130400 g6: f0d4a018 g7: 00000001\n"
	"o0: 00000000 o1: f0d4a298 o2: 00000040 o3: f1380718\n"
	"o4: f1380718 o5: 00000200 sp: f1b13f08 ret_pc: f001c2a0\n"
	"l0: efffd880 l1: 00000001 l2: f0d4a230 l3: 00000014\n"
	"l4: 0000ffff l5: f0131550 l6: f012c000 l7: f0130400\n"
	"i0: f1b13fb0 i1: 00000001 i2: 00000002 i3: 0007c000\n"
	"i4: f01457c0 i5: 00000004 i6: f1b13f70 i7: f0015360\n"
	"Instruction DUMP:\n"
    );

  if (!config->readBoolEntry("doSparcLinux", true))
    return False;

  for (s = linux_panic; *s; s++) if (*s == '\n') lines++;

  XGetWindowAttributes (qt_xdisplay(), window, &xgwa);

  fontname = get_string_resource ((xgwa.height > 600
				   ? "sparclinux.font2"
				   : "sparclinux.font"),
				  "SparcLinux.Font");
  if (!fontname || !*fontname) fontname = (char *)def_font;
  font = XLoadQueryFont (qt_xdisplay(), fontname);
  if (!font) font = XLoadQueryFont (qt_xdisplay(), def_font);
  if (!font) exit(-1);

  gcv.font = font->fid;
  gcv.foreground = get_pixel_resource(("sparclinux.foreground"),
				      "SparcLinux.Foreground",
				      xgwa.colormap);
  gcv.background = get_pixel_resource(("sparclinux.background"),
				      "SparcLinux.Background",
				      xgwa.colormap);
  XSetWindowBackground(qt_xdisplay(), window, gcv.background);
  XClearWindow(qt_xdisplay(), window);

  gc = XCreateGC(qt_xdisplay(), window, GCFont|GCForeground|GCBackground, &gcv);

  draw_string(window, gc, &gcv, font,
	      10, xgwa.height - (lines * (font->ascent + font->descent + 1)),
	      10, 10,
	      linux_panic, 0);
  XFreeGC(qt_xdisplay(), gc);
  XSync(qt_xdisplay(), False);
  bsod_sleep(delay);
  XClearWindow(qt_xdisplay(), window);
  XFreeFont(qt_xdisplay(), font);
  return True;
}

static Bool
amiga (Window window, int delay)
{
  XGCValues gcv;
  XWindowAttributes xgwa;
  char *fontname;
  const char *def_font = "fixed";
  XFontStruct *font;
  GC gc, gc2;
  int height;
  unsigned long fg, bg, bg2;
  Pixmap pixmap = 0;
  int pix_w, pix_h;
  KConfig *config = KApplication::getKApplication()->getConfig();

  const char *string =
    ("_Software failure.  Press left mouse button to continue.\n"
     "_Guru Meditation #00000003.00C01570");

  if (!config->readBoolEntry("doAmiga", true))
    return False;

  XGetWindowAttributes (qt_xdisplay(), window, &xgwa);

  fontname = get_string_resource ((xgwa.height > 600
				   ? "amiga.font2" : "amiga.font"),
				  "Amiga.Font");
  if (!fontname || !*fontname) fontname = (char *)def_font;
  font = XLoadQueryFont (qt_xdisplay(), fontname);
  if (!font) font = XLoadQueryFont (qt_xdisplay(), def_font);
  if (!font) exit(-1);

  gcv.font = font->fid;
  fg = gcv.foreground = get_pixel_resource("amiga.foreground",
					   "Amiga.Foreground",
					   xgwa.colormap);
  bg = gcv.background = get_pixel_resource("amiga.background",
					   "Amiga.Background",
					   xgwa.colormap);
  bg2 = get_pixel_resource("amiga.background2", "Amiga.Background",
			   xgwa.colormap);
  XSetWindowBackground(qt_xdisplay(), window, bg2);
  XClearWindow(qt_xdisplay(), window);

  gc = XCreateGC(qt_xdisplay(), window, GCFont|GCForeground|GCBackground, &gcv);
  gcv.background = fg; gcv.foreground = bg;
  gc2 = XCreateGC(qt_xdisplay(), window, GCFont|GCForeground|GCBackground, &gcv);

  height = (font->ascent + font->descent) * 6;

#ifdef HAVE_XPM
  {
    XpmAttributes xpmattrs;
    int result;
    xpmattrs.valuemask = 0;

# ifdef XpmCloseness
    xpmattrs.valuemask |= XpmCloseness;
    xpmattrs.closeness = 40000;
# endif
# ifdef XpmVisual
    xpmattrs.valuemask |= XpmVisual;
    xpmattrs.visual = xgwa.visual;
# endif
# ifdef XpmDepth
    xpmattrs.valuemask |= XpmDepth;
    xpmattrs.depth = xgwa.depth;
# endif
# ifdef XpmColormap
    xpmattrs.valuemask |= XpmColormap;
    xpmattrs.colormap = xgwa.colormap;
# endif

    result = XpmCreatePixmapFromData(qt_xdisplay(), window, amiga_hand,
				     &pixmap, 0 /* mask */, &xpmattrs);
    if (!pixmap || (result != XpmSuccess && result != XpmColorError))
      pixmap = 0;
    pix_w = xpmattrs.width;
    pix_h = xpmattrs.height;
  }
#endif /* HAVE_XPM */

  if (pixmap && xgwa.height > 600)	/* scale up the bitmap */
    {
      pixmap = double_pixmap(gc, xgwa.visual, xgwa.depth,
			     pixmap, pix_w, pix_h);
      pix_w *= 2;
      pix_h *= 2;
    }

  if (pixmap)
    {
      int x = (xgwa.width - pix_w) / 2;
      int y = ((xgwa.height - pix_h) / 2);
      XCopyArea(qt_xdisplay(), pixmap, window, gc, 0, 0, pix_w, pix_h, x, y);

      XSync(qt_xdisplay(), False);
      bsod_sleep(2);

      XCopyArea(qt_xdisplay(), pixmap, window, gc, 0, 0, pix_w, pix_h, x, y + height);
      XClearArea(qt_xdisplay(), window, 0, 0, xgwa.width, y + height, False);
      XFreePixmap(qt_xdisplay(), pixmap);
    }

  XFillRectangle(qt_xdisplay(), window, gc2, 0, 0, xgwa.width, height);
  draw_string(window, gc, &gcv, font, 0, 0, xgwa.width, height, string,0);

  {
    GC gca = gc;
    while (delay > 0)
      {
	XFillRectangle(qt_xdisplay(), window, gca, 0, 0, xgwa.width, font->ascent);
	XFillRectangle(qt_xdisplay(), window, gca, 0, 0, font->ascent, height);
	XFillRectangle(qt_xdisplay(), window, gca, xgwa.width-font->ascent, 0,
		       font->ascent, height);
	XFillRectangle(qt_xdisplay(), window, gca, 0, height-font->ascent, xgwa.width,
		       font->ascent);
	gca = (gca == gc ? gc2 : gc);
	XSync(qt_xdisplay(), False);
	if (bsod_sleep(1))
	  break;
	delay--;
      }
  }

  XFreeGC(qt_xdisplay(), gc);
  XFreeGC(qt_xdisplay(), gc2);
  XSync(qt_xdisplay(), False);
  XClearWindow(qt_xdisplay(), window);
  XFreeFont(qt_xdisplay(), font);
  return True;
}


/* Atari ST, by Marcus Herbert <rhoenie@nobiscum.de>
   Marcus had this to say:

	Though I still have my Atari somewhere, I hardly remember
	the meaning of the bombs. I think 9 bombs was "bus error" or
	something like that.  And you often had a few bombs displayed
	quickly and then the next few ones coming up step by step.
	Perhaps somebody else can tell you more about it..  its just
	a quick hack :-}
 */
static Bool
atari (Window window, int delay)
{
	
  XGCValues gcv;
  XWindowAttributes xgwa;
  const char *def_font = "fixed";
  XFontStruct *font;
  GC gc;
  Pixmap pixmap = 0;
  int pix_w = atari_width;
  int pix_h = atari_height;
  int offset;
  int i, x, y;
  KConfig *config = KApplication::getKApplication()->getConfig();

  if (!config->readBoolEntry("doAtari", true))
    return False;

  XGetWindowAttributes (qt_xdisplay(), window, &xgwa);

  font = XLoadQueryFont (qt_xdisplay(), def_font);
  if (!font) exit(-1);
                
  gcv.font = font->fid;
  gcv.foreground = get_pixel_resource("atari.foreground", "Atari.Foreground",
				      xgwa.colormap);
  gcv.background = get_pixel_resource("atari.background", "Atari.Background",
				      xgwa.colormap);

  XSetWindowBackground(qt_xdisplay(), window, gcv.background);
  XClearWindow(qt_xdisplay(), window);

  gc = XCreateGC(qt_xdisplay(), window, GCFont|GCForeground|GCBackground, &gcv);

  pixmap = XCreatePixmapFromBitmapData(qt_xdisplay(), window, (char *)atari_bits,
				       pix_w, pix_h,
				       gcv.foreground, gcv.background,
				       xgwa.depth);
  pixmap = double_pixmap(gc, xgwa.visual, xgwa.depth,
			 pixmap, pix_w, pix_h);
  pix_w *= 2;
  pix_h *= 2;

  offset = pix_w + 2;
  x = 5;
  y = (xgwa.height - (xgwa.height / 5));
  if (y < 0) y = 0;

  for (i=0 ; i<7 ; i++) {
    XCopyArea(qt_xdisplay(), pixmap, window, gc, 0, 0, pix_w, pix_h,
	      (x + (i*offset)), y);
  }  
  
  for (i=7 ; i<10 ; i++) {
    bsod_sleep(1);
    XCopyArea(qt_xdisplay(), pixmap, window, gc, 0, 0, pix_w, pix_h,
	      (x + (i*offset)), y);
  }

  XFreePixmap(qt_xdisplay(), pixmap);
  XFreeGC(qt_xdisplay(), gc);
  XSync(qt_xdisplay(), False);
  bsod_sleep(delay);
  XClearWindow(qt_xdisplay(), window);
  XFreeFont(qt_xdisplay(), font);
  return True;
}


static Bool
mac (Window window, int delay)
{
  XGCValues gcv;
  XWindowAttributes xgwa;
  char *fontname;
  const char *def_font = "fixed";
  XFontStruct *font;
  GC gc;
  Pixmap pixmap = 0;
  int pix_w = mac_width;
  int pix_h = mac_height;
  int offset = mac_height * 4;
  int i;
  KConfig *config = KApplication::getKApplication()->getConfig();

  const char *string = ("0 0 0 0 0 0 0 F\n"
			"0 0 0 0 0 0 0 3");

  if (!config->readBoolEntry("doMac", true))
    return False;

  XGetWindowAttributes (qt_xdisplay(), window, &xgwa);

  fontname = get_string_resource ("mac.font", "Mac.Font");
  if (!fontname || !*fontname) fontname = (char *)def_font;
  font = XLoadQueryFont (qt_xdisplay(), fontname);
  if (!font) font = XLoadQueryFont (qt_xdisplay(), def_font);
  if (!font) exit(-1);

  gcv.font = font->fid;
  gcv.foreground = get_pixel_resource("mac.foreground", "Mac.Foreground",
				      xgwa.colormap);
  gcv.background = get_pixel_resource("mac.background", "Mac.Background",
				      xgwa.colormap);
  XSetWindowBackground(qt_xdisplay(), window, gcv.background);
  XClearWindow(qt_xdisplay(), window);

  gc = XCreateGC(qt_xdisplay(), window, GCFont|GCForeground|GCBackground, &gcv);

  pixmap = XCreatePixmapFromBitmapData(qt_xdisplay(), window, (char *) mac_bits,
				       mac_width, mac_height,
				       gcv.foreground,
				       gcv.background,
				       xgwa.depth);

  for(i = 0; i < 2; i++)
    {
      pixmap = double_pixmap(gc, xgwa.visual, xgwa.depth,
			     pixmap, pix_w, pix_h);
      pix_w *= 2; pix_h *= 2;
    }

  {
    int x = (xgwa.width - pix_w) / 2;
    int y = (((xgwa.height + offset) / 2) -
	     pix_h -
	     (font->ascent + font->descent) * 2);
    if (y < 0) y = 0;
    XCopyArea(qt_xdisplay(), pixmap, window, gc, 0, 0, pix_w, pix_h, x, y);
    XFreePixmap(qt_xdisplay(), pixmap);
  }

  draw_string(window, gc, &gcv, font, 0, 0,
	      xgwa.width, xgwa.height + offset, string, 0);

  XFreeGC(qt_xdisplay(), gc);
  XSync(qt_xdisplay(), False);
  bsod_sleep(delay);
  XClearWindow(qt_xdisplay(), window);
  XFreeFont(qt_xdisplay(), font);
  return True;
}

static Bool
macsbug (Window window, int delay)
{
  XGCValues gcv;
  XWindowAttributes xgwa;
  char *fontname;
  const char *def_font = "fixed";
  XFontStruct *font;
  GC gc, gc2;
  KConfig *config = KApplication::getKApplication()->getConfig();

  int char_width, line_height;
  int col_right, row_top, row_bottom, page_right, page_bottom, body_top;
  int xoff, yoff;

  const char *left = ("    SP     \n"
		      " 04EB0A58  \n"
		      "58 00010000\n"
		      "5C 00010000\n"
		      "   ........\n"
		      "60 00000000\n"
		      "64 000004EB\n"
		      "   ........\n"
		      "68 0000027F\n"
		      "6C 2D980035\n"
		      "   ....-..5\n"
		      "70 00000054\n"
		      "74 0173003E\n"
		      "   ...T.s.>\n"
		      "78 04EBDA76\n"
		      "7C 04EBDA8E\n"
		      "   .S.L.a.U\n"
		      "80 00000000\n"
		      "84 000004EB\n"
		      "   ........\n"
		      "88 00010000\n"
		      "8C 00010000\n"
		      "   ...{3..S\n"
		      "\n"
		      "\n"
		      " CurApName \n"
		      "  Finder   \n"
		      "\n"
		      " 32-bit VM \n"
		      "SR Smxnzvc0\n"
		      "D0 04EC0062\n"
		      "D1 00000053\n"
		      "D2 FFFF0100\n"
		      "D3 00010000\n"
		      "D4 00010000\n"
		      "D5 04EBDA76\n"
		      "D6 04EBDA8E\n"
		      "D7 00000001\n"
		      "\n"
		      "A0 04EBDA76\n"
		      "A1 04EBDA8E\n"
		      "A2 A0A00060\n"
		      "A3 027F2D98\n"
		      "A4 027F2E58\n"
		      "A5 04EC04F0\n"
		      "A6 04EB0A86\n"
		      "A7 04EB0A58");
  const char *bottom = ("  _A09D\n"
			"     +00884    40843714     #$0700,SR         "
			"                  ; A973        | A973\n"
			"     +00886    40843765     *+$0400           "
			"                                | 4A1F\n"
			"     +00888    40843718     $0004(A7),([0,A7[)"
			"                  ; 04E8D0AE    | 66B8");

#if 0
  const char *body = ("Bus Error at 4BF6D6CC\n"
		      "while reading word from 4BF6D6CC in User data space\n"
		      " Unable to access that address\n"
		      "  PC: 2A0DE3E6\n"
		      "  Frame Type: B008");
#else
  const char * body = ("PowerPC unmapped memory exception at 003AFDAC "
						"BowelsOfTheMemoryMgr+04F9C\n"
		      " Calling chain using A6/R1 links\n"
		      "  Back chain  ISA  Caller\n"
		      "  00000000    PPC  28C5353C  __start+00054\n"
		      "  24DB03C0    PPC  28B9258C  main+0039C\n"
		      "  24DB0350    PPC  28B9210C  MainEvent+00494\n"
		      "  24DB02B0    PPC  28B91B40  HandleEvent+00278\n"
		      "  24DB0250    PPC  28B83DAC  DoAppleEvent+00020\n"
		      "  24DB0210    PPC  FFD3E5D0  "
						"AEProcessAppleEvent+00020\n"
		      "  24DB0132    68K  00589468\n"
		      "  24DAFF8C    68K  00589582\n"
		      "  24DAFF26    68K  00588F70\n"
		      "  24DAFEB3    PPC  00307098  "
						"EmToNatEndMoveParams+00014\n"
		      "  24DAFE40    PPC  28B9D0B0  DoScript+001C4\n"
		      "  24DAFDD0    PPC  28B9C35C  RunScript+00390\n"
		      "  24DAFC60    PPC  28BA36D4  run_perl+000E0\n"
		      "  24DAFC10    PPC  28BC2904  perl_run+002CC\n"
		      "  24DAFA80    PPC  28C18490  Perl_runops+00068\n"
		      "  24DAFA30    PPC  28BE6CC0  Perl_pp_backtick+000FC\n"
		      "  24DAF9D0    PPC  28BA48B8  Perl_my_popen+00158\n"
		      "  24DAF980    PPC  28C5395C  sfclose+00378\n"
		      "  24DAF930    PPC  28BA568C  free+0000C\n"
		      "  24DAF8F0    PPC  28BA6254  pool_free+001D0\n"
		      "  24DAF8A0    PPC  FFD48F14  DisposePtr+00028\n"
		      "  24DAF7C9    PPC  00307098  "
						"EmToNatEndMoveParams+00014\n"
		      "  24DAF780    PPC  003AA180  __DisposePtr+00010");
#endif

  const char *s;
  int body_lines = 1;

  if (!config->readBoolEntry("doMacsBug", true))
    return False;

  for (s = body; *s; s++) if (*s == '\n') body_lines++;

  XGetWindowAttributes (qt_xdisplay(), window, &xgwa);

  fontname = get_string_resource ((xgwa.height > 850
				   ? "macsbug.font3"
				   : (xgwa.height > 700
				      ? "macsbug.font2"
				      : "macsbug.font")),
				  "MacsBug.Font");
  if (!fontname || !*fontname) fontname = (char *)def_font;
  font = XLoadQueryFont (qt_xdisplay(), fontname);
  if (!font) font = XLoadQueryFont (qt_xdisplay(), def_font);
  if (!font) exit(-1);

  gcv.font = font->fid;
  gcv.foreground = get_pixel_resource("macsbug.foreground",
				      "MacsBug.Foreground",
				      xgwa.colormap);
  gcv.background = get_pixel_resource("macsbug.background",
				      "MacsBug.Background",
				      xgwa.colormap);

  gc = XCreateGC(qt_xdisplay(), window, GCFont|GCForeground|GCBackground, &gcv);

  gcv.foreground = gcv.background;
  gc2 = XCreateGC(qt_xdisplay(), window, GCForeground, &gcv);

  XSetWindowBackground(qt_xdisplay(), window,
		       get_pixel_resource("macsbug.borderColor",
					  "MacsBug.BorderColor",
					  xgwa.colormap));
  XClearWindow(qt_xdisplay(), window);

  char_width = (font->per_char
		? font->per_char['n'-font->min_char_or_byte2].width
		: font->min_bounds.width);
  line_height = font->ascent + font->descent + 1;

  col_right = char_width * 12;
  page_bottom = line_height * 47;

  if (page_bottom > xgwa.height) page_bottom = xgwa.height;

  row_bottom = page_bottom - line_height;
  row_top = row_bottom - (line_height * 4);
  page_right = col_right + (char_width * 88);
  body_top = row_top - (line_height * body_lines);

  page_bottom += 2;
  row_bottom += 2;
  body_top -= 4;

  xoff = (xgwa.width - page_right) / 2;
  yoff = (xgwa.height - page_bottom) / 2;
  if (xoff < 0) xoff = 0;
  if (yoff < 0) yoff = 0;

  XFillRectangle(qt_xdisplay(), window, gc2, xoff, yoff, page_right, page_bottom);

  draw_string(window, gc, &gcv, font, xoff, yoff, 10, 10, left, 0);
  draw_string(window, gc, &gcv, font, xoff+col_right, yoff+row_top,
	      10, 10, bottom, 0);

  XFillRectangle(qt_xdisplay(), window, gc, xoff + col_right, yoff, 2, page_bottom);
  XDrawLine(qt_xdisplay(), window, gc,
	    xoff+col_right, yoff+row_top, xoff+page_right, yoff+row_top);
  XDrawLine(qt_xdisplay(), window, gc,
	    xoff+col_right, yoff+row_bottom, xoff+page_right, yoff+row_bottom);
  XDrawRectangle(qt_xdisplay(), window, gc,  xoff, yoff, page_right, page_bottom);

  if (body_top > 4)
    body_top = 4;

  draw_string(window, gc, &gcv, font,
	      xoff + col_right + char_width, yoff + body_top, 10, 10, body,
	      500);

  while (delay > 0)
    {
      XDrawLine(qt_xdisplay(), window, gc,
		xoff+col_right+(char_width/2)+2, yoff+row_bottom+3,
		xoff+col_right+(char_width/2)+2, yoff+page_bottom-3);
      XSync(qt_xdisplay(), False);
      usleep(666666L);
      XDrawLine(qt_xdisplay(), window, gc2,
		xoff+col_right+(char_width/2)+2, yoff+row_bottom+3,
		xoff+col_right+(char_width/2)+2, yoff+page_bottom-3);
      XSync(qt_xdisplay(), False);
      usleep(333333L);
      if (bsod_sleep(0))
	break;
      delay--;
    }

  XFreeGC(qt_xdisplay(), gc);
  XFreeGC(qt_xdisplay(), gc2);
  XClearWindow(qt_xdisplay(), window);
  XFreeFont(qt_xdisplay(), font);
  return True;
}

void BSODSaver::draw_bsod (Window win)
{
      int i=-1;
      i = (random()%8);
      switch (i) {
	case 0: windows(win, delay, True); break;
	case 1: windows(win, delay, False); break;
	case 2: amiga(win, delay); break;
	case 3: mac(win, delay); break;
	case 4: macsbug(win, delay); break;
	case 5: sco(win, delay); break;
	case 6: sparc_linux(win, delay); break;
	case 7: atari(win, delay); break;
	default: abort(); break;
      }

}

static BSODSaver *saver=NULL;

void BSODSaver::readSettings ()
{
QString str;
KConfig *config = KApplication::getKApplication()->getConfig();
config->setGroup("Default");
config->writeEntry("Windows95.font", config->readEntry("Windows.font", "-*-courier-bold-r-*-*-*-120-*-*-m-*-*-*"));
config->writeEntry("Windows95.font2", config->readEntry("Windows.font2","-*-courier-bold-r-*-*-*-180-*-*-m-*-*-*"));
config->writeEntry("Windows95.foreground", config->readEntry("Windows.foreground", "White"));
config->writeEntry("Windows95.background", config->readEntry("Windows.background", "Blue"));
config->writeEntry("WindowsNT.font", config->readEntry("Windows.font", "-*-courier-bold-r-*-*-*-120-*-*-m-*-*-*"));
config->writeEntry("WindowsNT.font2", config->readEntry("Windows.font2","-*-courier-bold-r-*-*-*-180-*-*-m-*-*-*"));
config->writeEntry("WindowsNT.foreground", config->readEntry("Windows.foreground", "White"));
config->writeEntry("WindowsNT.background", config->readEntry("Windows.background", "Blue"));
config->writeEntry("Amiga.font", config->readEntry("Amiga.font", "-*-courier-bold-r-*-*-*-120-*-*-m-*-*-*"));
config->writeEntry("Amiga.font2", config->readEntry("Amiga.font2", "-*-courier-bold-r-*-*-*-180-*-*-m-*-*-*"));
config->writeEntry("Amiga.foreground", config->readEntry("Amiga.foreground", "Red"));
config->writeEntry("Amiga.background", config->readEntry("Amiga.background", "Black"));
config->writeEntry("Amiga.background2", config->readEntry("Amiga.background2", "White"));
config->writeEntry("MacsBug.font", config->readEntry("MacsBug.font", "-*-courier-medium-r-*-*-*-100-*-*-m-*-*-*"));
config->writeEntry("MacsBug.font2", config->readEntry("MacsBug.font2", "-*-courier-bold-r-*-*-*-120-*-*-m-*-*-*"));
config->writeEntry("MacsBug.font3", config->readEntry("MacsBug.font3", "-*-courier-bold-r-*-*-*-140-*-*-m-*-*-*"));
config->writeEntry("MacsBug.foreground", config->readEntry("MacsBug.foreground", "Black"));
config->writeEntry("MacsBug.background", config->readEntry("MacsBug.background", "White"));
config->writeEntry("MacsBug.borderColor", config->readEntry("MacsBug.borderColor", "#AAAAAA"));
config->writeEntry("Mac.font", config->readEntry("Mac.font", "-*-courier-bold-r-*-*-*-120-*-*-m-*-*-*"));
config->writeEntry("Mac.foreground", config->readEntry("Mac.foreground", "PaleTurquoise1"));
config->writeEntry("Mac.background", config->readEntry("Mac.background", "Black"));
config->writeEntry("SCO.font", config->readEntry("SCO.font", "-*-courier-bold-r-*-*-*-120-*-*-m-*-*-*"));
config->writeEntry("SCO.font2", config->readEntry("SCO.font2", "-*-courier-bold-r-*-*-*-140-*-*-m-*-*-*"));
config->writeEntry("SCO.foreground", config->readEntry("SCO.foreground",
"White"));
config->writeEntry("SCO.background", config->readEntry("SCO.background", "Black"));
config->writeEntry("Atari.foreground", config->readEntry("Atari.foreground", "Black"));
config->writeEntry("Atari.background", config->readEntry("Atari.background", "White"));
config->writeEntry("SparcLinux.font", config->readEntry("SparcLinux.font", "-*-courier-bold-r-*-*-*-120-*-*-m-*-*-*"));
config->writeEntry("SparcLinux.font2", config->readEntry("SparcLinux.font2", "-*-courier-bold-r-*-*-*-140-*-*-m-*-*-*"));
config->writeEntry("SparcLinux.foreground", config->readEntry("SparcLinux.foreground", "White"));
config->writeEntry("SparcLinux.background", config->readEntry("SparcLinux.background", "Black"));
str = config->readEntry("Delay");
if ( !str.isNull() )
	delay = (unsigned int)atoi( str );
else
	delay = DEFDELAY;

config->sync();
}

BSODSaver::BSODSaver (Drawable d)
 : kScreenSaver(d)
{
#ifdef __FreeBSD__
	srandomdev();
#else
	srandom(time(0));
#endif
	timer = new QTimer(this);
	readSettings();
	timer->start(delay);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void BSODSaver::timeout ()
{
	draw_bsod(d);
}

BSODSaver::~BSODSaver ()
{
	timer->stop();
	delete timer;
}

void startScreenSaver (Drawable d)
{
        if (saver)
                return;
        saver = new BSODSaver(d);
}
   
void stopScreenSaver ()
{
        if (saver) {
          delete saver;   // FIX
	}
        saver = NULL;
} 

int setupScreenSaver ()
{
	return 0;
}

const char *getScreenSaverName ()
{
	return glocale->translate("Black Screen of Death");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

#include "bsod.moc"
