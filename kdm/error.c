/* $XConsortium: error.c,v 1.16 94/04/17 20:03:38 gildea Exp $ */
/* $Id: error.c,v 1.9 1999/01/07 03:58:44 steffen Exp $ */
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
 * error.c
 *
 * Log display manager errors to a file as
 * we generally do not have a terminal to talk to
 * or use syslog if it exists
 */

# include "dm.h"
# include <stdio.h>
#if NeedVarargsPrototypes
# include <stdarg.h>
#else
/* this type needs to be big enough to contain int or pointer */
typedef long Fmtarg_t;
#endif

#if NeedVarargsPrototypes && !HAVE_VSYSLOG
#undef USE_SYSLOG
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

/*VARARGS1*/
void 
LogInfo(
#if NeedVarargsPrototypes
    char * fmt, ...)
#else
    fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
    Fmtarg_t arg1, arg2, arg3, arg4, arg5, arg6;
#endif
{
#ifndef USE_SYSLOG
    fprintf (stderr, "xdm info (pid %d): ", (int)getpid());
#endif
#if NeedVarargsPrototypes
    {
	va_list args;
	va_start(args, fmt);
#  ifdef USE_SYSLOG
	vsyslog (LOG_INFO, fmt, args);
#  else
	vfprintf (stderr, fmt, args);
#  endif
	va_end(args);
    }
#else
#  ifdef USE_SYSLOG
    syslog (LOG_INFO, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#  else
    fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#  endif
#endif
#ifdef USE_SYSLOG
    fflush (stderr);
#endif
}

/*VARARGS1*/
int
LogError (
#if NeedVarargsPrototypes
    char * fmt, ...)
#else
    fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
    Fmtarg_t arg1, arg2, arg3, arg4, arg5, arg6;
#endif
{
#ifndef USE_SYSLOG
    fprintf (stderr, "xdm error (pid %d): ", (int)getpid());
#endif
#if NeedVarargsPrototypes
    {
	va_list args;
	va_start(args, fmt);
#  ifdef USE_SYSLOG
	vsyslog (LOG_ERR, fmt, args);
#  else
	vfprintf (stderr, fmt, args);
#  endif
	va_end(args);
    }
#else
#  ifdef USE_SYSLOG
    syslog (LOG_ERR, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#  else
    fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#  endif
#endif
#ifdef USE_SYSLOG
    return 0;
#else
    return fflush (stderr);
#endif
}

/*VARARGS1*/
void
LogPanic (
#if NeedVarargsPrototypes
    char * fmt, ...)
#else
    fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
    Fmtarg_t arg1, arg2, arg3, arg4, arg5, arg6;
#endif
{
#ifndef USE_SYSLOG
    fprintf (stderr, "xdm panic (pid %d): ", (int)getpid());
#endif
#if NeedVarargsPrototypes
    {
	va_list args;
	va_start(args, fmt);
#  ifdef USE_SYSLOG
	vsyslog (LOG_EMERG, fmt, args);
#  else
	vfprintf (stderr, fmt, args);
#  endif
	va_end(args);
    }
#else
#  ifdef USE_SYSLOG
    syslog (LOG_EMERG, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#  else
    fprintf (fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#  endif
#endif
#ifdef USE_SYSLOG
    fflush (stderr);
#endif
    exit (1);
}

/*VARARGS1*/
int
LogOutOfMem (
#if NeedVarargsPrototypes
    char * fmt, ...)
#else
    fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
    Fmtarg_t arg1, arg2, arg3, arg4, arg5, arg6;
#endif
{
#ifdef USE_SYSLOG
    char fmt1[256];
    sprintf(fmt1, "out of memory: %s", fmt);
#else
    fprintf (stderr, "kdm: out of memory in routine ");
#endif
#if NeedVarargsPrototypes
    {
	va_list args;
	va_start(args, fmt);
#  ifdef USE_SYSLOG
	vsyslog (LOG_ALERT, fmt1, args);
#  else
	vfprintf (stderr, fmt, args);
#  endif
	va_end(args);
    }
#else
#  ifdef USE_SYSLOG
    syslog(LOG_ALERT, fmt1, arg1, arg2, arg3, arg4, arg5, arg6);
#  else
    fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#  endif
#endif
#ifdef USE_SYSLOG
    return 0;
#else
    return fflush (stderr);
#endif
}

int
Panic (mesg)
char	*mesg;
{
#ifdef USE_SYSLOG
    syslog(LOG_EMERG, mesg);
#else
    int	i;

    i = creat ("/dev/console", 0666);
    write (i, "panic: ", 7);
    write (i, mesg, strlen (mesg));
#endif
    exit (1);
}


/*VARARGS1*/
int
Debug (
#if NeedVarargsPrototypes
    char * fmt, ...)
#else
    fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
    Fmtarg_t arg1, arg2, arg3, arg4, arg5, arg6;
#endif
{
    if (debugLevel > 0)
    {
#if NeedVarargsPrototypes
	va_list args;
	va_start(args, fmt);
#  ifdef USE_SYSLOG
	vsyslog (LOG_DEBUG, fmt, args);
#  endif
	vprintf (fmt, args);
	va_end(args);
#else
#  ifdef USE_SYSLOG
	syslog (LOG_DEBUG, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#  endif
	printf (fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
#ifndef USE_SYSLOG
	fflush (stdout);
#endif
    }
    return 0;
}

void
InitErrorLog ()
{
#ifdef USE_SYSLOG
	openlog("kdm", LOG_PID, LOG_DAEMON);
#else
	int	i;
	if (errorLogFile[0]) {
		i = creat (errorLogFile, 0666);
		if (i != -1) {
			if (i != 2) {
				dup2 (i, 2);
				close (i);
			}
		} else
			LogError ("Cannot open errorLogFile %s\n", errorLogFile);
	}
#endif
}
