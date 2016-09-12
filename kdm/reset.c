/* $XConsortium: reset.c,v 1.11 94/04/17 20:03:42 keith Exp $ */
/* $Id: reset.c,v 1.5 1998/09/30 21:20:08 bieker Exp $ */
/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * pseudoReset -- pretend to reset the server by killing all clients
 * with windows.  It will reset the server most of the time, unless
 * a client remains connected with no windows.
 */

# include	"dm.h"
# include	<X11/Xlib.h>
# include	<signal.h>

/*ARGSUSED*/
static int
ignoreErrors (dpy, event)
Display	*dpy;
XErrorEvent	*event;
{
	return Debug ("ignoring error\n");
}

/*
 * this is mostly bogus -- but quite useful.  I wish the protocol
 * had some way of enumerating and identifying clients, that way
 * this code wouldn't have to be this kludgy.
 */

/*
 * The Property code is taken from kwm.
 * This should only be a temporary solution, but
 * it is necessary to check for windows with
 * the property KDE_DESKTOP_WINDOW so that
 * kdm doesn't kill itself!!
 *
 * Sat Oct 18 07:44:56 1997 -- Steffen Hansen
 */
static int _getprop(Display* dpy, Window w, Atom a, Atom type, long len, unsigned char **p){
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;

  status = XGetWindowProperty(dpy, w, a, 0L, len, False, type, 
			      &real_type, &format, &n , &extra, p);
  if (status != Success || *p == 0)
    return -1;
  if (n == 0)
    XFree((void*) *p);
  return n;
}

/* Small modification to be C */
static int getSimpleProperty(Display* dpy, Window w, Atom a, long *result){
  long *p = 0;

  if (_getprop(dpy, w, a, a, 1L, (unsigned char**)&p) <= 0){
    return FALSE;
  }

  *result = p[0];
  XFree((char *) p);
  return TRUE;
}

static
void killWindows (dpy, window)
Display	*dpy;
Window	window;
{
	Window	root, parent, *children;
	int	child;
	unsigned int nchildren = 0;
	/* This prop. indicates that its a kdm window! */
	Atom a = XInternAtom(dpy, "KDE_DESKTOP_WINDOW", False);
 
	/* We cannot loop, when we want to keep the stupid window...*.
	while (XQueryTree (dpy, window, &root, &parent, &children, &nchildren)
	       && nchildren > 0)
	*/
	XQueryTree (dpy, window, &root, &parent, &children, &nchildren);
	if( nchildren > 0)
	{
		for (child = 0; child < nchildren; child++) {
		        long result = 0;
			getSimpleProperty(dpy, children[child],a,&result);
			if( !result) {
			  /* not kdm window, kill it */
			  Debug ("XKillClient 0x%x\n", children[child]);
			  XKillClient (dpy, children[child]);
			}
		}
		XFree ((char *)children);
	}
}

static Jmp_buf	resetJmp;

/* ARGSUSED */
static SIGVAL
abortReset (n)
    int n;
{
	Longjmp (resetJmp, 1);
}

/*
 * this display connection better not have any windows...
 */
 
void pseudoReset (dpy)
Display	*dpy;
{
	Window	root;
	int	screen;

	if (Setjmp (resetJmp)) {
		LogError ("pseudoReset timeout\n");
	} else {
		(void) Signal (SIGALRM, abortReset);
		(void) alarm (30);
		XSetErrorHandler (ignoreErrors);
		for (screen = 0; screen < ScreenCount (dpy); screen++) {
			Debug ("pseudoReset screen %d\n", screen);
			root = RootWindow (dpy, screen);
			killWindows (dpy, root);
		}
		Debug ("before XSync\n");
		XSync (dpy, False);
		(void) alarm (0);
	}
	Signal (SIGALRM, SIG_DFL);
	XSetErrorHandler ((int (*)()) 0);
	Debug ("pseudoReset done\n");
}

