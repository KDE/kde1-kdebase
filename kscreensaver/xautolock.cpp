//----------------------------------------------------------------------------
//
// KDE screensavers
//
// This module is a heavily modified xautolock.
// The orignal copyright notice follows
//

/*****************************************************************************
 *
 * xautolock
 * =========
 *
 * Authors   :  S. De Troch (SDT) + M. Eyckmans (MCE)
 *
 * Date      :  22/07/90
 *
 * ---------------------------------------------------------------------------
 *
 * Copyright 1990, 1992-1995 by S. De Troch and MCE.
 *
 * Permission to use, copy, modify and distribute this software and the
 * supporting documentation without fee is hereby granted, provided that
 *
 *  1 : Both the above copyright notice and this permission notice
 *      appear in all copies of both the software and the supporting
 *      documentation.
 *  2 : No financial profit is made out of it.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *****************************************************************************/



/*
 *  Have a guess what this does...
 *  ==============================
 *
 *  Warning for swm & tvtwm users : xautolock should *not* be compiled
 *  with vroot.h, because it needs to know the real root window.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(hpux) || defined (__hpux)
#ifndef _HPUX_SOURCE
#define _HPUX_SOURCE
#endif /* _HPUX_SOURCE */
#endif /* hpux || __hpux */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef VMS
#include <ssdef.h>    
#include <processes.h>  /* really needed? */
#endif /* VMS */

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#ifdef HAVE_SYS_M_WAIT_H
#include <sys/m_wait.h>
#endif 

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#undef TrueColor

#include <qapp.h>
#include "xautolock.h"

/*
 *  Usefull macros and customization stuff
 *  ======================================
 */
#define PP(x)                      x

#ifdef VMS
#define ALL_OK                     1       /* for use by exit ()           */
#define PROBLEMS                   SS$_ABORT 
                                           /* for use by exit ()           */
#else /* VMS */
#define ALL_OK                     0       /* for use by exit ()           */
#define PROBLEMS                   1       /* for use by exit ()           */
#endif /* VMS */

#define FALSE                      0       /* as it says                   */
#define TRUE                       1       /* as it says                   */
#define MINUTES                    10      /* default ...                  */
#define CREATION_DELAY             30      /* should be > 10 and
                                              < min (45,(MIN_MINUTES*30))  */
#define CORNER_SIZE                10      /* size in pixels of the
                                              force-lock areas             */
#define CORNER_DELAY               5       /* number of seconds to wait
                                              before forcing a lock        */
#define TIME_CHANGE_LIMIT         120      /* if the time changes by more
                                              than x secs then we will
                                              assume someone has changed
                                              date or machine has suspended */


#ifndef HasVFork
#define vfork                      fork
#endif /* HasVFork */

#define Error0(str)                fprintf (stderr, str)
#define SetTrigger(delta)          trigger = time ((time_t*) NULL) + delta

static caddr_t                     ch_ptr;  /* this is dirty */
#define Skeleton(t,s)              (ch_ptr = (Caddrt) malloc ((Unsigned) s), \
                                      (ch_ptr == (Caddrt) NULL)              \
                                    ? (Error0 ("Out of memory.\n"),          \
                                       exit (PROBLEMS),                      \
                                       /*NOTREACHED*/ (t*) NULL              \
                                      )                                      \
                                    : (t*) ch_ptr                            \
                                   )                                         \

#define New(tp)                    Skeleton (tp, sizeof (tp))



/*
 *  New types
 *  =========
 */
#if defined (apollo) || defined (news1800) 
typedef int                        (*XErrorHandler) PP((Display*,
                                                        XErrorEvent*));
#endif /* apollo || news1800 */

#if defined (news1800) || defined (sun386) 
typedef int                        pid_t;
#endif /* news1800  || sun386*/

#ifdef VMS
typedef long                       pid_t;
#endif /* VMS */

#define Void                       void     /* no typedef because of VAX */
typedef int                        Int;
typedef char                       Char;
typedef char*                      String;
typedef int                        Boolean;
typedef caddr_t                    Caddrt;
typedef unsigned int               Unsigned;
typedef unsigned long              Huge;

typedef struct QueueItem_
        {
          Window                   window;        /* as it says          */
          time_t                   creationtime;  /* as it says          */
          struct QueueItem_*       next;          /* as it says          */
          struct QueueItem_*       prev;          /* as it says          */
        } aQueueItem, *QueueItem;

typedef struct Queue_
        {
          struct QueueItem_*       head;          /* as it says          */
          struct QueueItem_*       tail;          /* as it says          */
        } aQueue, *Queue;


/*
 *  Function declarations
 *  =====================
 */
#if defined(news1800) 
extern Void*    malloc                PP((Unsigned));
#endif /* news1800 */
 
static int      EvaluateCounter       PP((Display*));
static int      QueryPointer          PP((Display*));
static Void     ProcessEvents         PP((Display*, Queue));
static Queue    NewQueue              PP((Void));
static Void     AddToQueue            PP((Queue, Window));
static Void     ProcessQueue          PP((Queue, Display*, time_t));
static Void     SelectEvents          PP((Display*, Window, Boolean));


/*
 *  Global variables
 *  ================
 */
static time_t        trigger = 0;            /* as it says                 */
static time_t        time_limit = MINUTES;   /* as it says                 */
static Int           corner_size = CORNER_SIZE; /* as it says              */
static time_t        corner_delay = CORNER_DELAY; /* as it says            */
static CornerAction  corners[4] = { IGNORE, IGNORE, IGNORE, IGNORE };
                                             /* default CornerActions      */

/*
 *  Functions related to the window queue
 *  =====================================
 *
 *  Function for creating a new queue
 *  ---------------------------------
 */
static Queue  NewQueue ()

{
  Queue  queue;  /* return value */

  queue = New (aQueue);
  queue->tail = New (aQueueItem);
  queue->head = New (aQueueItem);

  queue->tail->next = queue->head;
  queue->head->prev = queue->tail;
  queue->tail->prev = queue->head->next = (QueueItem) NULL;

  return queue;
}


/*
 *  Function for adding an item to a queue
 *  --------------------------------------
 */
static Void  AddToQueue (Queue queue, Window window)
{
  QueueItem  newq;  /* new item */

  newq = New (aQueueItem);

  newq->window = window;
  newq->creationtime = time ((time_t*) NULL);
  newq->next = queue->tail->next;
  newq->prev = queue->tail;
  queue->tail->next->prev = newq;
  queue->tail->next = newq;
}

/*
 *  Function for processing those entries that are old enough
 *  ---------------------------------------------------------
 */
static Void  ProcessQueue (Queue queue, Display *d, time_t age)
{
  QueueItem  current;  /* as it says */
  time_t     now;      /* as it says */

  time (&now);
  current = queue->head->prev;

  while ( current->prev && current->creationtime + age < now )
  {
    SelectEvents (d, current->window, False);
    current = current->prev;
    free (current->next);
  }

  current->next = queue->head;
  queue->head->prev = current;
}


static Void  FreeQueue( Queue queue )
{
  QueueItem  current;  /* as it says */

  current = queue->head->prev;

  while ( current->prev )
  {
	  current = current->prev;
	  free(current->next);
  }

  free(current);
  free(queue);
}


/*
 *  Functions related to (the lack of) user activity
 *  ================================================
 *
 *  Function for processing the event queue
 *  ---------------------------------------
 */
static Void  ProcessEvents (Display *d, Queue queue)
{
  XEvent  event;  /* as it says */

 /*
  *  Read whatever is available for reading.
  */
  while (XPending (d))
  {
    if (XCheckMaskEvent (d, SubstructureNotifyMask, &event))
    {
      if (event.type == CreateNotify)
      {
        AddToQueue (queue, event.xcreatewindow.window);
      }
    }
    else
    {
      XNextEvent (d, &event);
    }


   /*
    *  Reset the counter if and only if the event is a KeyPress
    *  event *and* was not generated by XSendEvent ().
    */
    if ( event.type == KeyPress && !event.xany.send_event )
    {
      SetTrigger (time_limit);
    }
  }


 /*
  *  Check the window queue for entries that are older than
  *  CREATION_DELAY seconds.
  */
  ProcessQueue (queue, d, (time_t) CREATION_DELAY);
}


/*
 *  Function for monitoring pointer movements
 *  -----------------------------------------
 */
static int  QueryPointer (Display *d)
{
  Window           dummy_w;            /* as it says                    */
  Int              dummy_c;            /* as it says                    */
  Unsigned         mask;               /* modifier mask                 */
  Int              root_x;             /* as it says                    */
  Int              root_y;             /* as it says                    */
  time_t           now;                /* as it says                    */
  Int              i;                  /* loop counter                  */
  static Window    root;               /* root window the pointer is on */
  static Screen*   screen;             /* screen the pointer is on      */
  static Unsigned  prev_mask = 0;      /* as it says                    */
  static Int       prev_root_x = -1;   /* as it says                    */
  static Int       prev_root_y = -1;   /* as it says                    */
  static Boolean   first_call = TRUE;  /* as it says                    */


 /*
  *  Have a guess...
  */
  if (first_call)
  {
    first_call = FALSE;
    root = DefaultRootWindow (d);
    screen = ScreenOfDisplay (d, DefaultScreen (d));
  }


 /*
  *  Find out whether the pointer has moved. Using XQueryPointer for this
  *  is gross, but it also is the only way never to mess up propagation
  *  of pointer events.
  *
  *  Remark : Unlike XNextEvent(), XPending () doesn't notice if the
  *           connection to the server is lost. For this reason, earlier
  *           versions of xautolock periodically called XNoOp (). But
  *           why not let XQueryPointer () do the job for us, since
  *           we now call that periodically anyway?
  */
  if (!XQueryPointer (d, root, &root, &dummy_w, &root_x, &root_y,
                      &dummy_c, &dummy_c, &mask))
  {
   /*
    *  Pointer has moved to another screen, so let's find out which one.
    */
    for (i = -1; ++i < ScreenCount (d); ) 
    {
      if (root == RootWindow (d, i)) 
      {
        screen = ScreenOfDisplay (d, i);
        break;
      }
    }
  }

  if (   root_x == prev_root_x
      && root_y == prev_root_y
      && mask == prev_mask
     )
  {
    time (&now);
    if ( now >= ( trigger - time_limit ) + corner_delay )
    {
     /*
      *  If the pointer has not moved since the previous call and 
      *  is inside one of the 4 corners, we act according to the
      *  contents of the "corners" array.
      *
      *  If root_x and root_y are less than zero, don't lock even if
      *  FORCE_LOCK is set in the upper-left corner. Why? 'cause
      *  on initial server startup, IF the pointer is never moved,
      *  XQueryPointer returns values less than zero (only some
      *  servers, Openwindows 2.0 and 3.0 in particular).
      */
      if ( root_x <= corner_size && root_x >= 0
              && root_y <= corner_size && root_y >= 0)
      {
          return corners[0];
      }
      else if ( root_x >= WidthOfScreen  (screen) - corner_size - 1
              && root_y <= corner_size)
      {
          return corners[1];
      }
      else if ( root_x <= corner_size
              && root_y >= HeightOfScreen (screen) - corner_size - 1 )
      {
          return corners[2];
      }
      else if ( root_x >= WidthOfScreen  (screen) - corner_size - 1
              && root_y >= HeightOfScreen (screen) - corner_size - 1 )
      {
  		return corners[3];
      }
    }
  }
  else
  {
    prev_root_x = root_x;
    prev_root_y = root_y;
    prev_mask = mask;
    SetTrigger (time_limit);
  }

  return 0;
}


/*
 *  Function for deciding whether to lock
 *  -------------------------------------
 */
static int  EvaluateCounter (Display *)
{
  time_t         now = 0;                /* as it says  */

 /*
  *  Now trigger the notifier if required. 
  */
  time (&now);

 /*
  *  Finally fire up the locker if time has come. 
  */
  if (now >= trigger)
  {
      SetTrigger (time_limit);
	  return TRUE;
  }

  return FALSE;
}

/*
 *  Function for selecting events on a tree of windows
 *  --------------------------------------------------
 */
static Void  SelectEvents (Display *d, Window window, Boolean substructure_only)
{
  Window             root;              /* root window of this window */
  Window             parent;            /* parent of this window      */
  Window*            children;          /* children of this window    */
  Unsigned           nof_children = 0;  /* number of children         */
  Unsigned           i;                 /* loop counter               */
  XWindowAttributes  attribs;           /* attributes of the window   */


 /*
  *  Start by querying the server about parent and child windows.
  */
  if (!XQueryTree (d, window, &root, &parent, &children, &nof_children))
  {
    return;
  }


 /*
  *  Build the appropriate event mask. The basic idea is that we don't
  *  want to interfere with the normal event propagation mechanism if
  *  we don't have to.
  */
  if (substructure_only)
  {
    XSelectInput (d, window, SubstructureNotifyMask);
  }
  else
  {
    if (parent == None)  /* the *real* rootwindow */
    {
      attribs.all_event_masks = 
        attribs.do_not_propagate_mask = KeyPressMask;
    }
    else if (XGetWindowAttributes (d, window, &attribs) == 0)
    {
      return;
    }

    XSelectInput (d, window,   SubstructureNotifyMask
                             | (  (  attribs.all_event_masks
                                   | attribs.do_not_propagate_mask)
                                & KeyPressMask));
  }


 /*
  *  Now do the same thing for all children.
  */
  for (i = 0; i < nof_children; ++i)
  {
    SelectEvents (d, children[i], substructure_only);
  }

  if (nof_children) XFree ((Char*) children);
}


int catchFalseAlarms( Display *, XErrorEvent * )
{
	return 0;
}

void setCorners( const char *cstring )
{
	char p;

	for ( int i = 0; i < 4; i++ )
	{
		p = cstring[i];
		if ( p == 0 )
			break;

		switch ( p )
		{
			case 'l':
				corners[i] = FORCE_LOCK;
				break;

			case 's':
				corners[i] = FORCE_SAVE;
				break;

			default:
				corners[i] = IGNORE;
		}
	}
}

volatile int timeoutNow = FALSE;

void forceTimeout()
{
	timeoutNow = TRUE;
}

Queue  windowQueue;
Window hiddenWin;        /* hidden window    */

void initAutoLock()
{
  Display*              d;          /* display pointer  */
  Window                r;          /* root window      */
  Int                   s;          /* screen index     */
  XSetWindowAttributes  attribs;    /* for dummy window */
  int (*oldHandler)(Display *, XErrorEvent *);

  d = qt_xdisplay();

  oldHandler = XSetErrorHandler( catchFalseAlarms );
  XSync (d, 0);

  windowQueue = NewQueue ();

  for (s = -1; ++s < ScreenCount (d); )
  {
    AddToQueue (windowQueue, r = RootWindowOfScreen (ScreenOfDisplay (d, s)));
    SelectEvents (d, r, True);
  }

 /*
  *  Get ourselves a dummy window in order to allow display and/or
  *  session managers etc. to use XKillClient() on us (e.g. xdm when
  *  not using XDMCP).
  * 
  *  I'm not sure whether the window needs to be mapped for xdm, but
  *  the default set up Sun uses for OpenWindows and olwm definitely
  *  requires it to be mapped.
  */
  attribs.override_redirect = True;
  hiddenWin = XCreateWindow (d, DefaultRootWindow (d), -100, -100, 1, 1, 0,
                CopyFromParent, InputOnly, CopyFromParent, CWOverrideRedirect,
				&attribs);

  XMapWindow (d, hiddenWin );

  XSetErrorHandler( oldHandler );
}

void cleanupAutoLock()
{
  int (*oldHandler)(Display *, XErrorEvent *);
  oldHandler = XSetErrorHandler( catchFalseAlarms );

  FreeQueue( windowQueue );
  XDestroyWindow( qt_xdisplay(), hiddenWin );
  XSetErrorHandler( oldHandler );
}


/*
 *  Main function
 *  -------------
 */
int waitTimeout( int timeout )
{
  Display*              d;          /* display pointer  */
  int rv;
  int (*oldHandler)(Display *, XErrorEvent *);
  time_t now, prev;

  time_limit = timeout;

  d = qt_xdisplay();

  oldHandler = XSetErrorHandler( catchFalseAlarms );

  SetTrigger (time_limit);

  time(&prev);

 /*
  *  Main event loop.
  */
  while ( 1 )
  {
    if ( timeoutNow )
	{
	    // this happens, when you send the screensaver a SIGUSR1
	    // this emits, that you want to lock the screen!
	     rv = FORCE_LOCK;
		timeoutNow = FALSE;
		break;
	}
    ProcessEvents (d, windowQueue);
    rv = QueryPointer (d);
	if ( rv != IGNORE )
		break;

    time(&now);

    if ((now > prev && now - prev > TIME_CHANGE_LIMIT) ||
        (prev > now && prev - now > TIME_CHANGE_LIMIT+1))
    {
      /* the time has changed in one large jump.  This could be because the
         date was changed, or the machine was suspended.  We'll just
         reset the triger. */
      SetTrigger (time_limit);
    }

    prev = now;
      
    if ( EvaluateCounter (d) )
	{
		rv = IGNORE;
		break;
	}

   /*
    *  It seems that, on some operating systems (VMS to name just one),
    *  sleep () can be vastly inaccurate: sometimes 60 calls to sleep (1)
    *  add up to only 30 seconds or even less of sleeping. Therefore,
    *  as of patchlevel 9 we no longer rely on it for keeping track of
    *  time. The only reason why we still call it, is to make  xautolock
    *  (which after all uses a busy-form-of-waiting algorithm), less
    *  processor hungry.
    */
    sleep (1);
  }

  XSetErrorHandler( oldHandler );

  return rv;
}

