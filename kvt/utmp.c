/* $Id$
 *
 * Revision 1.5 08/09/1993 stempien
 * Something was wrong with the Linux support!
 * I fixed it using the provided utmp manipulation routines.
 * I also surrounded many varables that were not needed for Linux 
 * in the BSD defines to reduce the memory needed to run.
 * I didn't touch the Sun part of the code so it should still work.
 *
 * $Log$
 * Revision 1.7.4.3  1999/06/08 09:09:22  hoelzer
 * Fixed several security holes.
 *
 * Revision 1.7.4.2  1999/02/17 18:54:08  leonw
 * A non-readable password file made kvt segfault. This behaviour now produces an error message and a clean exit. Not that I know why anyone would want to have a non-readable passwd file...
 *
 * Revision 1.7.4.1  1999/02/11 16:10:42  leonw
 * Some bugfixes include 1) cleaning tty permissions when killed. 2) MacOS style menubar does not resize kvt anymore 3) Testing for the fixed font when no other font can be found.
 *
 * Revision 1.7  1998/10/24 20:17:41  bieker
 * Use the test for sys/param.h
 *
 * Revision 1.6  1998/10/23 16:49:34  bieker
 * Enables utmp on FreeBSD.. I home sys/param.h is a common file on most
 * systems. :)
 *
 * Revision 1.5  1997/08/10 13:43:56  berberic
 * [bmg] removed `#define BSD' because it prevents compilation on
 *       non-BSD-systems
 *
 * Revision 1.4  1997/08/02 12:33:07  kulow
 * Coolo: some cleanups in kvt.
 *
 * Revision 1.3  1997/05/27 12:25:09  ettrich
 * Matthias: Small fixes for FreeBSD (Marc van Kempten)
 *
 * Revision 1.2  1997/04/23 20:00:17  kulow
 * updated to kvt-0.14
 *
 * Revision 1.4  1993/08/09  11:54:15  lipka
 * now works both on Linux and SunOs 4.1.3.
 * Brians clean-ups incorporated
 *
 * Revision 1.3  1993/08/09  07:16:42  lipka
 * nearly correct (running) linux-version.
 * Delete-Window is still a problem
 *
 * Revision 1.1  1993/08/07  18:03:53  lipka
 * Initial revision
 *
 * Clean-ups according to suggestions of Brian Stempien (stempien@cs.wmich.edu)
 *
 *    provides:
 *    int utmp_end(), returns eof(UTMP)
 *    void makeutent(), stamps _current_ process to UTMP
 *    void clean_exit(), to clear a record in utmp
 *        (void) cleanutent(pos)
 *
 *    Bugs: UTMP should be locked from call to utmp_end() 'til
 *          makeutent() (?).
 *          Maybe the child should tell the parent, where the entry is made.
 *          Tested only on Linux.
 *
 *          Gives weird inital idle times. They go away after one uses the 
 *          window, but......
 *
 */
/* There must be a more elegant way than this HUGE ifdef to do this */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>
#include <sys/time.h>

#include <utmp.h>
#include <unistd.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <pwd.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>

#ifndef TTYTAB
#define TTYTAB "/etc/ttys"
#endif

void cleanutent(void);
void makeutent(char *);

/* for cleanutent, marks whether or not the entry has been made */
char madeutent=0;

/****************************************************************************
 * Makes sure that the utmp entry is removed when we exit 
 ***************************************************************************/
void clean_exit(int r) 
{
#ifndef SVR4
  extern char ttynam[];
  extern struct stat ttyfd_stat;

  // regain root privileges
  seteuid(0);

  chmod(ttynam,ttyfd_stat.st_mode);
  chown(ttynam,ttyfd_stat.st_uid,ttyfd_stat.st_gid);
  
  // drop root privileges again
  seteuid(getuid());

#endif
  if(madeutent)
    cleanutent();
  if (r >= 0) exit(r);
}

#ifndef UTMP_SUPPORT
/***************************************************************************
 * Dummy routines to use if utmp support is not wanted/needed 
 **************************************************************************/
void makeutent(char *ttyname) 
{
  return;
}
void cleanutent() 
{
  return;
}

#else /* UTMP_SUPPORT */
/**************************************************************************
 *
 * Code to make utmp entries really starts here !
 *
 **************************************************************************/
int utmp_pos; /* position of utmp-stamp */


/*
 * on Sparcs login/logouts are logged at /var/adm/wtmp
 * but talk(d)/finger only look at /etc/utmp
 */
#ifndef UTMP
#define UTMP "/etc/utmp"
#endif

#ifndef USER_PROCESS
#define USER_PROCESS 7
#endif

#ifndef DEAD_PROCESS
#define DEAD_PROCESS 8
#endif

/**************************************************************************
 *
 * The code was pretty ugly when BSD and SYSV style utmp stuff was mixed up,
 * so I just made a huge ifdef for BSD, and another for SYSV
 *
 *************************************************************************/
#ifdef BSD

/**************************************************************************
 * get_tslot() - grabbed from xvt-1.0 - modified by David Perry
 *
 *  Look up the tty name in the etc/ttytab file and return a slot number
 * that can be used to access the utmp file.  We cannot use ttyslot()
 * because the tty name is not that of fd 0.
 *************************************************************************/
int get_tslot(char *ttynam)
{
  FILE *fs;
  char buf[200], name[200];
  int i;
  
  if ((fs = fopen(TTYTAB,"r")) == NULL)
    return(-1);
  i = 1;
  while (fgets(buf,200,fs) != NULL) 
    {
      if (*buf == '#')
	continue;
      if (sscanf(buf,"%s",name) != 1)
	continue;
      if (strcmp(ttynam,name) == 0) 
	{
	  fclose(fs);
	  return(i);
	}
      i++;
    }
  fclose(fs);
  return(-1);
}

/****************************************************************************
 * write a utmp entry to the utmp file 
 ***************************************************************************/
int write_utmp(char *ttyname, struct utmp * u) 
{
  FILE *utmp;
  if((utmp = fopen(UTMP,"r+")) == NULL)
    return -1;
  utmp_pos = get_tslot(ttyname) * sizeof(struct utmp);
  if(utmp_pos < 0)
    return -1;
  fseek(utmp,utmp_pos,0);
  fwrite((char *)u, sizeof(struct utmp),1,utmp);
  fclose(utmp);
  madeutent = 1;
  return(utmp_pos);
}

/****************************************************************************
 * Makes a utmp entry
 ***************************************************************************/
void makeutent(char *ttyname) 
{
  struct passwd *pwent;
  struct utmp u;
  
  struct timeval tp;
  struct timezone tzp;
  int l,l2;
  extern char *display_name;
  
  pwent=getpwuid(getuid());
  /* Prevent crashing when password file is readonly :) */
  if (pwent == NULL) {
	fprintf( stderr, "Could not read password file entry.\n" );
	clean_exit(1);     
  }
  memset((char *)&u, 0, sizeof(u));
  /* memset? never heard of it :-) thanks Brian */


#ifndef SVR4
  l = sizeof(u.ut_host);
  strncpy(u.ut_host,display_name,l);
#endif
  /* and now the line: */
  l = sizeof(u.ut_line);
  strncpy(u.ut_line,ttyname,l);
  if (!gettimeofday(&tp,&tzp))
    u.ut_time = tp.tv_sec;
  strncpy(u.ut_name,pwent->pw_name,sizeof(u.ut_name));
  (void) write_utmp(ttyname, &u);
}

/****************************************************************************
 * Removes a utmp entry
 ***************************************************************************/
/*
 * there is a nice function "endutent" defined in <utmp.h>;
 * like "setutent" it takes no arguments, so I think it gets all information
 * from library-calls.
 * That's why "setutent" has to be called by the child-process with
 * file-descriptors 0/1 set to the pty. But that child execs to the
 * application-program and therefore can't clean it's own utmp-entry!(?)
 * The master on the other hand doesn't have the correct process-id
 * and io-channels... I'll do it by hand:
 * (what's the position of the utmp-entry, the child wrote? :-)
 */
void cleanutent(void) 
{
  FILE *ut;
  struct utmp u;
  
  if((ut = fopen(UTMP,"r+")) == NULL)
    return;
  fseek(ut,utmp_pos,0);
  memset(&u,0,sizeof(u));
  fwrite((char *)&u,sizeof(struct utmp),1,ut);
  fclose(ut);
}


/***************************************************************************
 *
 * Here's where we start SYSV style utmp entry code 
 *
 **************************************************************************/
#else /* BSD */

/****************************************************************************
 * write a utmp entry to the utmp file 
 ***************************************************************************/
int write_utmp(struct utmp * u) 
{
  int pos;

  // regain root privileges
  seteuid(0);

  utmpname(UTMP);
  setutent();
  pututline(u);
  endutent();
  updwtmp(WTMP_FILE, u);
  pos = (int)NULL;
  madeutent = 1;

  // drop root privileges again
  seteuid(getuid());

  return(pos);
}

/****************************************************************************
 * Makes a utmp entry
 ***************************************************************************/
void makeutent(char *ttyname) 
{
  struct passwd *pwent;
  struct utmp u;

  pwent=getpwuid(getuid());
  /* Prevent crashing when password file is readonly :) */
  if (pwent == NULL) {
	fprintf( stderr, "Could not read password file entry.\n" );
	clean_exit(1);
  }
  memset((char *)&u, 0, sizeof(u));
  /* memset? never heard of it :-) thanks Brian */

#ifndef SVR4
  strcpy(u.ut_host,"\0"); /* Needs to be this way for Linux */
#endif
  /* and now the line: */
  strcpy(u.ut_line,ttyname); /* This works at the moment. I don't think that 
                                ttyname is NULL terminated, so something 
                                more elaborate may need to be done. */
  time(&u.ut_time);
  strncpy(u.ut_user,pwent->pw_name,sizeof(u.ut_user));
  u.ut_type = USER_PROCESS;
  u.ut_pid = getpid(); 
  strncpy(u.ut_id,(ttyname+3),2);

  // regain root privileges
  seteuid(0);
  
  (void) write_utmp(&u);

  // drop root privileges again
  seteuid(getuid());
}

/****************************************************************************
 * Removes a utmp entry
 ***************************************************************************/
/*
 * there is a nice function "endutent" defined in <utmp.h>;
 * like "setutent" it takes no arguments, so I think it gets all information
 * from library-calls.
 * That's why "setutent" has to be called by the child-process with
 * file-descriptors 0/1 set to the pty. But that child execs to the
 * application-program and therefore can't clean it's own utmp-entry!(?)
 * The master on the other hand doesn't have the correct process-id
 * and io-channels... I'll do it by hand:
 * (what's the position of the utmp-entry, the child wrote? :-)
 */
void cleanutent() 
{
  int pid;
  struct utmp *u;

  // regain root privileges
  seteuid(0);
  
  utmpname(UTMP);
  setutent();
  pid = getpid();
  /* The following 11 lines of code were copied from the 
   * poeigl-1.20 login/init package. Special thanks to 
   * poe for the code examples.
   */
  while((u = getutent()))
    {
      if(u->ut_pid == pid) 
	{
	  time(&u->ut_time);
#ifdef SVR4
 	  memset(&u->ut_user, 0, sizeof(u->ut_user));
#else
	  memset(&u->ut_user, 0, sizeof(u->ut_user));
	  memset(&u->ut_host, 0, sizeof(u->ut_host));
#endif
	  u->ut_type = DEAD_PROCESS;
	  u->ut_pid = 0;
#if !defined(SVR4) && !defined(AIXV3)
	  u->ut_addr = 0;
#endif
	  pututline(u); /* Was reversed with in the original */
	  endutent();
	  updwtmp(WTMP_FILE, u);
	}
    }

  // drop root privileges again
  seteuid(getuid());
}

#endif /* BSD */
#endif /* UTMP_SUPPORT */

