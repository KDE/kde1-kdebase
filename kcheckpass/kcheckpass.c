/*****************************************************************
 *
 *	kcheckpass - Simple password checker
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *	kcheckpass is a simple password checker. Just invoke and
 *      send it the password on stdin.
 *
 *	If the password was accepted, the program exits with 0;
 *	if it was rejected, it exits with 1. Any other exit
 *	code signals an error.
 *
 *	It's hopefully simple enough to allow it to be setuid
 *	root.
 *
 *	Compile with -DHAVE_VSYSLOG if you have vsyslog().
 *	Compile with -DHAVE_PAM if you have a PAM system,
 *	and link with -lpam -ldl.
 *	Compile with -DHAVE_SHADOW if you have a shadow
 *	password system.
 *
 *	Copyright (C) 1998, Caldera, Inc.
 *	Released under the GNU General Public License
 *
 *	Olaf Kirch <okir@caldera.de>      General Framework and PAM support
 *	Christian Esken <esken@kde.org>   Shadow and /etc/passwd support
 *
 *      Other parts were taken from kscreensaver's passwd.cpp.
 *
 *****************************************************************/

#include "kcheckpass.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <memory.h>

#ifndef __FreeBSD__
#include <malloc.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <time.h>

/* Define this if you want kcheckpass to accept options
 * (They don't do anything useful right now) */
#undef ACCEPT_OPTIONS

/*****************************************************************
 * Set to 1 if stdin is a tty
 *****************************************************************/
static int	havetty = 0;
#ifdef ACCEPT_OPTIONS
static int	debug = 0;
#endif

/*****************************************************************
 * Output a message to syslog (and to stderr as well, if available)
 *****************************************************************/
void
message(const char *fmt, ...)
{
  va_list		ap;

  va_start(ap, fmt);
  if (havetty) {
    vfprintf(stderr, fmt, ap);
  } else {
#ifndef HAVE_VSYSLOG
    char	buffer[1024];

    /* Not sure what's less portable -- vsyslog or
     * vsnprintf :-/
     */
#ifdef HAVE_VSNPRINTF
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
#else
    vsprintf(buffer, fmt, ap);
#endif
    syslog(LOG_NOTICE, buffer);
#else
    vsyslog(LOG_NOTICE, fmt, ap);
#endif
      }
}

/*****************************************************************
 * Usage message
 *****************************************************************/
static void
usage(int exitval)
{
  message("usage: kcheckpass %s\n"
	  "    Obtains the password from standard input and checks it\n"
	  "    against that of the invoking user.\n"
	  "    Exit codes:\n"
	  "        0 success\n"
	  "        1 invalid password\n"
          "        2 cannot read password database\n"
	  "    Anything else tells you something's badly hosed.\n",
#ifdef ACCEPT_OPTIONS
	" [-dh]"
#else
	""
#endif
	);
  exit(exitval);
}

/*****************************************************************
 * Main program
 *****************************************************************/
int
main(int argc, char **argv)
{
  char		*login, passbuffer[1024], *passwd;
  struct passwd	*pw;
  int		status, c;
  uid_t		uid;

  openlog("kcheckpass", LOG_PID, LOG_AUTH);


  /* Make sure stdout/stderr are open */
  for (c = 1; c <= 2; c++) {
    if (fcntl(c, F_GETFL) == -1) {
      int	nfd;

      if ((nfd = open("/dev/null", O_WRONLY)) < 0) {
        message("cannot open /dev/null: %s\n", strerror(errno));
	exit(10);
      }
      if (c != nfd) {
	dup2(nfd, c);
	close(nfd);
      }
    }
  }

  havetty = isatty(0);

#ifndef ACCEPT_OPTIONS
  if (argc != 1)
    usage(10);
#else
  while ((c = getopt(argc, argv, "d")) != -1) {
    switch (c) {
    case 'd':
      debug = 1;
      break;
    case 'h':
      usage(0);
    default:
      message("Unknown option %c\n", c);
      usage(10);
    }
  }
#endif

  uid = getuid();
  if (!(pw = getpwuid(uid))) {
    message("Unknown user (uid %d)\n", uid);
    exit(10);
  }
  if ((login = strdup(pw->pw_name)) == NULL) {
    message("Out of memory on strdup'ing user name \"%.100s\"\n", pw->pw_name);
    exit(10);
  }

  /* If we have a tty, use getpass.
   * Otherwise, just snarf the password from stdin (we don't
   * want getpass to blurt anything to stderr).
   */
  passwd = 0;
  if (havetty) {
    passwd = getpass("Password: ");
  } else {
    int	passlen;

    passlen = read(0, passbuffer, sizeof(passbuffer)-1);
    if (passlen >= 0) {
      passbuffer[passlen] = '\0';
      if (passbuffer[passlen-1] == '\n')
	passbuffer[--passlen] = '\0';
      passwd = passbuffer;
    }
  }
  if (passwd == 0) {
    message("Can't read password: %s\n", strerror(errno));
    exit(10);
  }

  /* Now do the fandango */
  status = authenticate(login, passwd);

  /* Clear password buffer */
  memset(passbuffer, 0, sizeof(passbuffer));

  if ( status == 0 ) {
    /* failure */
    time_t	now = time(NULL);
    message("authentication failure for user %s [uid %d]\n",
	    login, uid);

    do {
      sleep (1); /* <<< Security: Don't undermine the shadow system */
    } while (time(NULL) < now + 1);
    exit(1);
  }
  if ( status == 2 ) {
    /* Cannot read password database (e.g. not SUID on shadow systems) */
    exit(2);
  }
  exit(0);
}

/*****************************************************************
  The real authentication methods are in separate source files.
  Look in checkpass_*.cpp
*****************************************************************/
