/* Copyright 1992 John Bovey, University of Kent at Canterbury.
 *
 *  You can do what you like with this source code as long as
 *  you don't try to make money out of it and you include an
 *  unaltered copy of this message (including the copyright).
 */
/*
 * This module has been very heavily modified by R. Nation
 * (nation@rocket.sanders.lockheed.com).
 * No additional restrictions are applied
 *
 * Additional modification by Garrett D'Amore (garrett@netcom.com) to
 * allow vt100 printing.  No additional restrictions are applied.
 *
 * atp	22/4/98	  bugfix to allow proper handling of a null character,
 * 		  which made kvt unusable with VMS. get_com_char needs
 * 		  to return an int, or else null will trigger end of buffer.
 * 
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 */

/* LICENSE changed: all authors including but not limited to John Bovey
 * aggreed to distribute their code under the terms of the GPL
 */

#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <utime.h>
#include <ctype.h>
#include "debug.h"

#if defined (_HPUX_SOURCE)
#define _TERMIOS_INCLUDED
#include <bsdtty.h>
#endif
/* sgis have /dev/ptmx [bmg] */
#ifdef __sgi__
#define SVR4
#endif

/* Solaris defines __SVR4 instead of SVR4 */
#ifdef __SVR4
#define SVR4
#endif

/* SCO OpenServer needs _SVID3 defined to pick up the definition of TCGETS */
#if defined (_SCO_DS)
#define _SVID3 1
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif   

#ifdef HAVE_SYS_STROPTS_H
#include <sys/stropts.h>
#define _NEW_TTY_CTRL
#endif

#ifdef HAVE_TERMIO_H
#ifndef HAVE_TERMIOS_H  /* Prefer new POSIX termios.h */
#include <termio.h>
#endif
#endif

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "rxvt.h"
#include "command.h"
#include "screen.h"
#include "xsetup.h"

#if defined(VDISCRD) && !defined(VDISCARD)
#define	VDISCARD	VDISCRD
#endif

#if defined(VWERSE) && !defined(VWERASE)
#define	VWERASE		VWERSE
#endif

/* Stephan: a little bit hardcoded :) */
#if !defined PRINT_PIPE
#define PRINT_PIPE
#endif

#define MAX_REFRESH_PERIOD 10

/* for greek keyboard mapping */
#ifdef GREEK_KBD	
#include "grkelot.h"

static greek_mode_ON = False;
#endif

int size_set = 0;      /* flag set once the window size has been set */

int mouse_block = 0;	/* block mouse while in popup */
int mouse_rep = 0;	/* allow mouse reporting */

#define KBUFSIZE	256	/* size of keyboard mapping buffer */
#define COM_BUF_SIZE   2048	/* size of buffer used to read from command */
#define STRING_MAX      512
#define ARGS_MAX         20     /* maximum # of args for esc sequences */

/*  Special character returned by get_com_char().
 */
#define GCC_NULL	0x100		/* Input buffer is empty */
#define ESC		033

/*  Flags used to control get_com_char();
 */
#define BUF_ONLY	1

/* Multiclick time
 */
#define MULTICLICK_TIME 500

/*  Global variables that are set up at the beginning and then not changed
 */
extern Display		*display;
extern Window		vt_win;
extern Window		main_win;
extern struct sbar_info sbar;

extern int login_shell;
extern int MetaHandling;
#ifdef MAPALERT
extern int map_alert;
KeySym AlertKeysym = XK_i;
#endif

extern void clean_exit( int r );
extern void cleanutent( void );
extern void makeutent( char *ttyname );
extern void scr_fore_color( int color );
extern void scr_back_color( int color );

int comm_fd = -1;/* file descriptor connected to the command */
static int comm_pid;	/* process id if child */
static int x_fd;	/* file descriptor of the X server connection */
static int fd_width;	/* width of file descriptors being used */
static int app_cur_keys = 0;/* flag to set cursor keys in application mode */
static int app_kp_keys = 0; /* flag to set application keypad keys */
#ifndef SVR4
static char ptynam[] = "/dev/ptyxx";
char ttynam[] = "/dev/ttyxx";
#else
char *ttynam, *ptsname();
#endif
static Atom wm_del_win;
extern WindowInfo MyWinInfo;

extern int fat_sbar;
extern int console;

KeySym SecureKeysym = XK_s;
KeySym BigFontKeysym = XK_greater;
KeySym SmallFontKeysym = XK_less;
KeySym PageUpKeysym = XK_Prior;
KeySym PageDownKeysym = XK_Next;
#ifdef GREEK_KBD
KeySym GreekSwitchKeysym = XK_Mode_switch;
int GreekMode = GREEK_ELOT928;
#endif
unsigned int BackspaceSendsControlH = 0;
unsigned int KeySetSend = 0;

int refresh_nl_count=0;
int refresh_nl_limit = 1;
extern int refresh_type;

/* cursor key codes - note that up/down/left/right are currently unused */

keyset keys[] = {
	/* home	    end		up	    down	left	    right
	   insert   delete	backspace		name		  */

	{"\e[H",    "\e[F",	"\e[A",	    "\e[B",	"\e[D",	    "\e[C",
	 "\e[2~",   "\177",	"\010",			"KVT/XTerm/ANSI"},

	{"\e[7~",   "\e[8~",	"\e[A",	    "\e[B",	"\e[D",	    "\e[C",
	 "\e[2~",   "\177",	"\010",			"Color XTerm"},

	{"\e[1~",   "\e[4~",	"\e[A",	    "\e[B",	"\e[D",	    "\e[C",
	 "\e[2~",   "\e[3~",	"\177",			"Console"},

	{"\eO\200", "\eOe",	"\e[A",	    "\e[B",	"\e[D",	    "\e[C",
	 "\e[2~",   "\177",	"\010",			"Original XTerm"},

	{"\eOH",    "\eOF",	"\eOA",	    "\eOB",	"\eOD",	    "\eOC",
	 "\e[2~",   "\e[3~",	"\177",			"Original XTerm II"},

	{"\eOq",    "\eOp",	"\eOA",	    "\eOB",	"\eOD",	    "\eOC",
	 "\eOr",    "\177",	"\010",			"VT 100"},

	{0,	    0,		0,	    0,		0,	    0,
	 0,	    0,		0,			0}
};

/*  Terminal mode structures.
*/
static struct termios ttmode;
#ifndef __FreeBSD__
#ifndef CINTR
#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE 0
#endif

#undef CTRL
#define CTRL(c) ((c) - '@')

#ifdef SVR4
#define CINTR 0177
#define CQUIT CTRL('U')
#define CERASE CTRL('H')
#else
#define CINTR CTRL('C')
#define CQUIT CTRL('\\')
#define CERASE 0177
#endif

#define CEOF CTRL('D')
#define CKILL CTRL('U')
#define CEOL _POSIX_VDISABLE
#define CEOL2 _POSIX_VDISABLE
#define CNSWTCH _POSIX_VDISABLE
#define CSTART CTRL('Q')
#define CSTOP CTRL('S')
#define CSUSP CTRL('Z');
#define CDSUSP _POSIX_VDISABLE
#define CRPRNT CTRL('R')
#define CFLUSH CTRL('O')
#define CWERASE CTRL('W')
#define CLNEXT CTRL('V')
#endif
#endif

#ifndef CEOL
#define CEOL _POSIX_VDISABLE
#endif

/*  Variables used for buffered command input.
 */
static unsigned char com_buf[COM_BUF_SIZE];
static unsigned char *com_buf_next, *com_buf_top;
unsigned int mask = 0x00ff;

static void catch_child(int);
static void catch_sig(int);
int run_command(unsigned char *,unsigned char **);
static unsigned char *lookup_key(XEvent *,int *, unsigned char);
static unsigned int get_com_char(int);
/* get to handle.   Matthias */ 
int handle_X_event(XEvent event, unsigned char);
void process_string(int);
#ifdef PRINT_PIPE
void process_print_pipe(void);
#endif
void process_escape_sequence(void);
void process_csi_sequence(void);
void process_xterm_sequence(void);
void process_terminal_mode(int,int,int,int *);
void process_sgr_mode(int,int,int,int *);
struct stat ttyfd_stat;
int ttyfd;

#ifndef HAVE_GETDTABLESIZE
#define _NEW_TTY_CTRL
#include <sys/resource.h>
int static getdtablesize() 
{
  struct rlimit   rlp;
  getrlimit(RLIMIT_NOFILE, &rlp);
  return(rlp.rlim_cur);
}
#endif


/*  Catch a SIGCHLD signal and exit if the direct child has died.
 */
static void catch_child(int nonsense)
{
  if (wait((int *)NULL) == comm_pid) 
    {
      clean_exit(0);
    }
  /* reinstall the SIGCHLD-handler since we have more childs now,
     for example other kvts (Matthias) */ 
  signal(SIGCHLD,catch_child);
}

/*  Catch a fatal signal and tidy up before quitting
 */
static void catch_sig(int sig)
{
  signal(sig,SIG_DFL);
  clean_exit(-1);
  setuid(getuid());
  kill(getpid(),sig);
}

/*  Run the command in a subprocess and return a file descriptor for the
 *  master end of the pseudo-teletype pair with the command talking to
 *  the slave.
 */

/* Changed the argv0 and execlp stuff similar to rxvt-2.18. 
 * Command is ignored as parameter. Matthias */ 
 int run_command(unsigned char *command,unsigned char **argv)
{
  int ptyfd;
  int uid, gid;
  unsigned char *s3, *s4;
  int i;
  int width, height;
#ifndef SVR4
  static char ptyc3[] = "pqrstuvwxyz";
  static char ptyc4[] = "0123456789abcdef";
 #endif

  /*  First find a master pty that we can open.  
   */

#ifdef SVR4
  ptyfd = open("/dev/ptmx",O_RDWR);
  if (ptyfd < 0) 
    {
      error("Can't open a pseudo teletype");
      return(-1);
    }
  grantpt(ptyfd);
  unlockpt(ptyfd);
  fcntl(ptyfd,F_SETFL,O_NDELAY);
  ttynam=ptsname(ptyfd);
#else
  ptyfd = -1;
  for (s3 = ptyc3; *s3 != 0; s3++) 
    {
      for (s4 = ptyc4; *s4 != 0; s4++) 
	{
	  ptynam[8] = ttynam[8] = *s3;
	  ptynam[9] = ttynam[9] = *s4;
	  if ((ptyfd = open(ptynam,O_RDWR)) >= 0) 
	    {
	      if (geteuid() == 0 || access(ttynam,R_OK|W_OK) == 0)
		break;
	      else 
		{
		  close(ptyfd);
		  ptyfd = -1;
		}
	    }
	}
      if (ptyfd >= 0)
	break;
    }
  if (ptyfd < 0) 
    {
      error("Can't open a pseudo teletype");
      return(-1);
    }
  fcntl(ptyfd,F_SETFL,O_NDELAY);
#endif  
  for (i = 1; i <= 15; i++)
    signal(i,catch_sig);
  signal(SIGCHLD,catch_child);
  lstat(ttynam,&ttyfd_stat);
  utime(ttynam,NULL);
    
  makeutent(&ttynam[5]);	/* stamp /etc/utmp */  

  comm_pid = fork();
  if (comm_pid < 0) 
    {
      error("Can't fork");
      return(-1);
    }
  if (comm_pid == 0) 
    {
      struct group *gr;
      
      if ((ttyfd = open(ttynam,O_RDWR)) < 0) 
	{
	  error("could not open slave tty %s",ttynam);
	  clean_exit(1);
	}
#ifdef SVR4
      ioctl(ttyfd,I_PUSH,"ptem");
      ioctl(ttyfd,I_PUSH,"ldterm");
#endif

      uid = getuid();
#ifndef SVR4
      if ((gr = getgrnam("tty")) != NULL)
	gid = gr->gr_gid;
      else
	gid = -1;
      fchown(ttyfd,uid,gid);
      fchmod(ttyfd,0600);
#endif
#ifdef TIOCCONS
     if (console) 
	{
	  int on = 1;
	  if (ioctl (ttyfd, TIOCCONS, (unsigned char *)&on) == -1)
	    fprintf(stderr, "kvt: cannot open console\n");
	}
#endif  /* TIOCCONS */
#if defined(_HPUX_SOURCE) || defined (__Lynx__)
      for (i = 0; i < sysconf(_SC_OPEN_MAX); i++)
	if (i != ttyfd)
	  close(i);
#else
      for (i = 0; i < getdtablesize(); i++)
	if (i != ttyfd)
	  close(i);
#endif

      dup(ttyfd);
      dup(ttyfd);
      dup(ttyfd);

      if (ttyfd > 2)
	close(ttyfd);
      
      if (setsid() < 0)
	perror("failed to set process group");
#if defined(TIOCSCTTY)
      ioctl(0, TIOCSCTTY, 0) ;
#endif
      {
	int pgrp = getpid();
	ioctl(0, TIOCSPGRP, (char *)&pgrp);
	setpgid(0,0);
	close(open(ttynam, O_WRONLY, 0));
	setpgid(0,0);
      }

      /* init of termios structure		*/
#if defined (__FreeBSD__) || (__NetBSD__) || defined(__bsdi__)
      ioctl(0,TIOCGETA,(char *)&ttmode);
#else
#   if defined (_HPUX_SOURCE) || defined(__Lynx__)
      tcgetattr(0, &ttmode);
#   else
      ioctl(0,TCGETS,(char *)&ttmode);
#   endif        
#endif
 
#if defined(_HPUX_SOURCE) || defined(__Lynx__)
      ttmode.c_iflag = BRKINT | IGNPAR | ICRNL| IXON;
      ttmode.c_lflag = ISIG|IEXTEN|ICANON|ECHO|ECHOE|ECHOK;
#else
      ttmode.c_iflag = BRKINT | IGNPAR | ICRNL| IXON | IMAXBEL;
      ttmode.c_lflag = ISIG|IEXTEN|ICANON|ECHO|ECHOE|ECHOK|ECHOCTL|ECHOKE;
#endif                                                       
      ttmode.c_oflag = OPOST | ONLCR ;
      ttmode.c_cflag = B9600 | CS8 | CREAD;

      ttmode.c_cc[VEOF] = CEOF;
#ifdef ALPHA
      (unsigned) ttmode.c_cc[VEOL] = CEOL;
#else
      ttmode.c_cc[VEOL] = CEOL; 
#endif
      ttmode.c_cc[VINTR] = CINTR;
      ttmode.c_cc[VQUIT] = CQUIT;
      ttmode.c_cc[VERASE] = CERASE;
      ttmode.c_cc[VKILL] = CKILL;
#if defined(_HPUX_SOURCE) || defined(__Lynx__)
      ttmode.c_cc[VSUSP] = CSWTCH;
#else
      ttmode.c_cc[VSUSP] = CSUSP;
#endif
#ifdef VDSUSP
      ttmode.c_cc[VDSUSP] = CDSUSP;
#endif
      ttmode.c_cc[VSTART] = CSTART;
      ttmode.c_cc[VSTOP] = CSTOP;
#ifdef VREPRINT
      ttmode.c_cc[VREPRINT] = CRPRNT;
#endif
#ifdef VDISCARD
      ttmode.c_cc[VDISCARD] = CFLUSH;
#endif
#ifdef VWERASE
      ttmode.c_cc[VWERASE] = CWERASE;
#endif
#ifdef VLNEXT
      ttmode.c_cc[VLNEXT] = CLNEXT;
#endif
#ifdef VSWTC
      ttmode.c_cc[VSWTC] = 0;
#endif                           

#ifdef VSWTCH
      ttmode.c_cc[VSWTCH] = 0;
#endif 
#if VMIN != VEOF
      ttmode.c_cc[VMIN] = 1;
#endif
#if VTIME != VEOL
      ttmode.c_cc[VTIME] = 0;
#endif

      if(mask == 0x7f)
	ttmode.c_cflag = B9600 | PARENB | CS7 | CREAD;
#if defined (__FreeBSD__) || (__NetBSD__) || defined(__bsdi__)
      ioctl(0,TIOCSETA,(char *)&ttmode);
#else
#   ifdef _HPUX_SOURCE
      tcsetattr(0, TCSANOW, &ttmode);
#   else
      ioctl(0,TCSETS,(char *)&ttmode);
#   endif        
#endif
      scr_get_size(&width,&height);
      tty_set_size(0,width,height);


 /* the stuff inspired from rxvt-2.18: */ 
	/* command interpreter path */
	if (argv != NULL)
	  {
	     setgid(getgid()); 
	     setuid(uid); 
	     execvp (argv [0], (char **) argv);
	     error ("can't execute \"%s\"", argv [0]);
	     clean_exit(1);
	  }
	else
	  {
	     const char *argv0, *shell;
	     char* base;

	     if ((shell = getenv ("SHELL")) == NULL || *shell == '\0')
	       shell = "/bin/sh";

	     base = strrchr (shell, '/');
	     argv0 = (base ? base + 1 : shell);
	     if (login_shell)
	       {
		  char * p = (char*) safemalloc((strlen (argv0)+2)*sizeof(char), argv0);
		  p [0] = '-';
		  strcpy (&p [1], argv0);
		  argv0 = p;
	       }
	     setgid(getgid()); 
	     setuid(uid);
	     execlp (shell, argv0, NULL);
	     error ("can't execute \"%s\"", shell);
	     clean_exit(1);
	  }

	/* originally rvxt-2.08:  (Matthias) */

/*       if(login_shell) */
/* 	{ */
/*           char *tmp; */
/* 	  strcpy(argv0,"-"); */
/*           if ((tmp = strrchr(argv[0], '/')) == NULL) */
/*             tmp = argv[0]; */
/*           else */
/*             tmp++; */
/* 	  strcat(argv0,tmp); */
/* 	  argv[0] = argv0; */
/* 	} */
/*       setgid(getgid()); */
/*       setuid(uid); */

/*       execvp(command,(char **)argv); */
/*       error("Couldn't execute %s",command); */
/*       clean_exit(1); */
    } 

  return(ptyfd);
}

/*  Tell the teletype handler what size the window is.  Called after a window
 *  size change.
 */
void tty_set_size(int fd,int width,int height)
{
  struct winsize wsize;
  
  if(fd < 0)
    return;

  wsize.ws_row = (unsigned short)height;
  wsize.ws_col = (unsigned short)width;
  ioctl(fd,TIOCSWINSZ,(char *)&wsize);
}

/*  Initialise the command connection.  This should be called after the X
 *  server connection is established.
 */
void init_command(unsigned char *command,unsigned char **argv)
{
#ifdef GREEK_KBD
  greek_init(GreekMode);
  /*atexit(greek_end);  not necessary */
#endif          
  /*  Enable the delete window protocol.
   */

   wm_del_win = XInternAtom(display,"WM_DELETE_WINDOW",False); 
   XSetWMProtocols(display,main_win,&wm_del_win,1); 
  
  if ((comm_fd = run_command(command,argv)) < 0) 
    {
      error("Quitting");
      clean_exit(1);
    }

  x_fd = XConnectionNumber(display);
#if defined(_HPUX_SOURCE) || defined(__Lynx__)
  fd_width = sysconf(_SC_OPEN_MAX);
#else
  fd_width = getdtablesize();
#endif

  com_buf_next = com_buf_top = com_buf;
}

/*  Convert the keypress event into a string.
    Matthias: trick: used the composed character from Qt if usefull
 */
static unsigned char *lookup_key(XEvent *ev,int *pcount, unsigned char qt_c)
{
  KeySym keysym;
  static XComposeStatus compose = {NULL,0};
  int count;
  static unsigned char kbuf[KBUFSIZE];
  unsigned char *s, *c;
  int meta;

  count = XLookupString(&ev->xkey,kbuf,KBUFSIZE-1,&keysym,&compose); 
 
  if (count <= 1 && qt_c != '\0'){
    kbuf[0] = qt_c;
    count = 1;
  }

  kbuf[count] = (unsigned char)0;

  meta = ev->xkey.state & Mod1Mask;
  s = NULL;
  /* no longer needed. (Matthias) */
  if((keysym == SecureKeysym)&&((ev->xkey.state &Mod1Mask) == Mod1Mask))
    {
/*       scr_secure(); */
      count = 0;
    }

  else if((keysym == BigFontKeysym)&&((ev->xkey.state &Mod1Mask) == Mod1Mask))
    {
/*       NewFont(BIGGER); */
      count = 0;
    }
  else if((keysym==SmallFontKeysym)&&((ev->xkey.state &Mod1Mask) == Mod1Mask))
    {
/*       NewFont(SMALLER); */
      count = 0;
    }
  else

  if((keysym == PageUpKeysym)&&((ev->xkey.state &Mod1Mask) == Mod1Mask))
    {
      scr_move_up_down(UP);
      count = 0;
    }
  else if((keysym == PageDownKeysym)&&((ev->xkey.state &Mod1Mask) == Mod1Mask))
    {
      scr_move_up_down(DOWN);
      count = 0;
    }
  /* added ShiftMask here to be more like an xterm. (Matthias) */ 
  else if((keysym == PageUpKeysym)&&((ev->xkey.state &ShiftMask) == ShiftMask))
    {
      scr_move_up_down(UP);
      count = 0;
    }
  else if((keysym == PageDownKeysym)&&((ev->xkey.state &ShiftMask) == ShiftMask))
    {
      scr_move_up_down(DOWN);
      count = 0;
    }
#ifdef MAPALERT
  else if((keysym == AlertKeysym)&&((ev->xkey.state &Mod1Mask) == Mod1Mask))
    {
      map_alert = !map_alert;
      count = 0;
    }
#endif
#ifdef GREEK_KBD
  else if(keysym == GreekSwitchKeysym)
    {
      greek_mode_ON = !greek_mode_ON;
      if (greek_mode_ON)
        greek_reset();
      count = 0;
    }
#endif
  else switch(keysym)
    {
      /* introduced BackSpace and Delete mapping (Matthias) */ 
    case XK_BackSpace :
      /* \010 = ^H (BS)  */
      /* \177 = ^? (Del) */
      strcpy(kbuf, BackspaceSendsControlH ? "\010" : "\177");
      count = 1;
      break;
    case XK_Delete :
#if XlibSpecificationRelease >= 6
    case XK_KP_Delete :
#endif
      /* strcpy(kbuf,"\033[3~"); */
      /* strcpy(kbuf, BackspaceSendsControlH ? "\177" : "\010"); */
      strcpy(kbuf, keys[KeySetSend].del);
      count = strlen(kbuf);
      break;

      
    case XK_Up :
#if XlibSpecificationRelease >= 6
    case XK_KP_Up :
#endif
      strcpy(kbuf,(app_cur_keys ? "\033OA" : "\033[A"));
      count = strlen(kbuf);
      break;
    case XK_Down :
#if XlibSpecificationRelease >= 6
    case XK_KP_Down :
#endif
      strcpy(kbuf,app_cur_keys ? "\033OB" : "\033[B");
      count = strlen(kbuf);
      break;
    case XK_Right :
      strcpy(kbuf,app_cur_keys ? "\033OC" : "\033[C");
      count = strlen(kbuf);
      break;
    case XK_Left :
      strcpy(kbuf,app_cur_keys ? "\033OD" : "\033[D");
      count = strlen(kbuf);
      break;
    case XK_KP_F1 :
      strcpy(kbuf,"\033OP");
      count = 3;
      break;
    case XK_KP_F2 :
      strcpy(kbuf,"\033OQ");
      count = 3;
      break;
    case XK_KP_F3 :
      strcpy(kbuf,"\033OR");
      count = 3;
      break;
    case XK_KP_F4 :
      strcpy(kbuf,"\033OS");
      count = 3;
      break;
    case XK_KP_0 :  
      strcpy(kbuf,app_kp_keys ? "\033Op" : "0");
      count = strlen(kbuf);
      break;
    case XK_KP_1 :
      strcpy(kbuf,app_kp_keys ? "\033Oq" : "1");
      count = strlen(kbuf);
      break;
    case XK_KP_2 :
      strcpy(kbuf,app_kp_keys ? "\033Or" : "2");
      count = strlen(kbuf);
      break;
    case XK_KP_3 :
      strcpy(kbuf,app_kp_keys ? "\033Os" : "3");
      count = strlen(kbuf);
      break;
    case XK_KP_4 :
      strcpy(kbuf,app_kp_keys ? "\033Ot" : "4");
      count = strlen(kbuf);
      break;
    case XK_KP_5 :
      strcpy(kbuf,app_kp_keys ? "\033Ou" : "5");
      count = strlen(kbuf);
      break;
    case XK_KP_6 :
      strcpy(kbuf,app_kp_keys ? "\033Ov" : "6");
      count = strlen(kbuf);
      break;
    case XK_KP_7 :
      strcpy(kbuf,app_kp_keys ? "\033Ow" : "7");
      count = strlen(kbuf);
      break;
    case XK_KP_8 :
      strcpy(kbuf,app_kp_keys ? "\033Ox" : "8");
      count = strlen(kbuf);
      break;
    case XK_KP_9 :
      strcpy(kbuf,app_kp_keys ? "\033Oy" : "9");
      count = strlen(kbuf);
      break;
    case XK_KP_Subtract :
      strcpy(kbuf,app_kp_keys ? "\033Om" : "-");
      count = strlen(kbuf);
      break;
    case XK_KP_Separator :
      strcpy(kbuf,app_kp_keys ? "\033Ol" : ",");
      count = strlen(kbuf);
      break;
    case XK_KP_Decimal :
      strcpy(kbuf,app_kp_keys ? "\033On" : ".");
      count = strlen(kbuf);
      break;
    case XK_KP_Enter :
      strcpy(kbuf,app_kp_keys ? "\033OM" : "\r");
      count = strlen(kbuf);
      break;
      /* changed home and end to more modern values (Matthias) */ 
    case XK_Home :
#if XlibSpecificationRelease >= 6
    case XK_KP_Home :
#endif
      /*strcpy(kbuf,"\033[H");*/
      strcpy(kbuf, keys[KeySetSend].home);
      count = strlen(kbuf);
      break;
    case XK_End :
#if XlibSpecificationRelease >= 6
    case XK_KP_End :
#endif
      /*strcpy(kbuf,"\033[F");*/
      strcpy(kbuf, keys[KeySetSend].end);
      count = strlen(kbuf);
      break;
    case XK_F1 :
      strcpy(kbuf,"\033[11~");
      count = 5;
      break;
    case XK_F2 :
      strcpy(kbuf,"\033[12~");
      count = 5;
      break;
    case XK_F3 :
      strcpy(kbuf,"\033[13~");
      count = 5;
      break;
    case XK_F4 :
      strcpy(kbuf,"\033[14~");
      count = 5;
      break;
    case XK_F5 :
      strcpy(kbuf,"\033[15~");
      count = 5;
      break;
    case XK_F6 :
      strcpy(kbuf,"\033[17~");
      count = 5;
      break;
    case XK_F7 :
      strcpy(kbuf,"\033[18~");
      count = 5;
      break;
    case XK_F8 :
      strcpy(kbuf,"\033[19~");
      count = 5;
      break;
    case XK_F9 :
      strcpy(kbuf,"\033[20~");
      count = 5;
      break;
    case XK_F10 :
      strcpy(kbuf,"\033[21~");
      count = 5;
      break;
    case XK_F11 :
      strcpy(kbuf,"\033[23~");
      count = 5;
      break;
    case XK_F12 :
      strcpy(kbuf,"\033[24~");
      count = 5;
      break;
    case XK_F13 :
      strcpy(kbuf,"\033[25~");
      count = 5;
      break;
    case XK_F14 :
      strcpy(kbuf,"\033[26~");
      count = 5;
      break;
    case XK_Help :
    case XK_F15 :
      strcpy(kbuf,"\033[28~");
      count = 5;
      break;
    case XK_Menu :
    case XK_F16 :
      strcpy(kbuf,"\033[29~");
      count = 5;
      break;
    case XK_F17 :
      strcpy(kbuf,"\033[31~");
      count = 5;
      break;
    case XK_F18 :
      strcpy(kbuf,"\033[32~");
      count = 5;
      break;
    case XK_F19 :
      strcpy(kbuf,"\033[33~");
      count = 5;
      break;
    case XK_F20 :
      strcpy(kbuf,"\033[34~");
      count = 5;
      break;
    case XK_Find :
      strcpy(kbuf,"\033[1~");
      count = 4;
      break;
    case XK_Insert :
#if XlibSpecificationRelease >= 6
    case XK_KP_Insert :
#endif
      /* strcpy(kbuf,"\033[2~"); */
      strcpy(kbuf, keys[KeySetSend].ins);
      count = strlen(kbuf);
      break;
    case XK_Execute :
      strcpy(kbuf,"\033[3~");
      count = 4;
      break;
    case XK_Select :
      strcpy(kbuf,"\033[4~");
      count = 4;
      break;
    case XK_Prior :
#if XlibSpecificationRelease >= 6
    case XK_KP_Prior :
#endif
      strcpy(kbuf,"\033[5~");
      count = 4;
      break;
    case XK_Next :
#if XlibSpecificationRelease >= 6
    case XK_KP_Next :
#endif
      strcpy(kbuf,"\033[6~");
      count = 4;
      break;
#ifdef GREEK_KBD
    default:
      if (greek_mode_ON)
        count = greek_xlat(kbuf, count);
#endif                
    }
  
  *pcount = count;

  /* if meta is pressed, precede message with ESC */
  /* used to set the 8th bit high, but this confuses things
   * for some non-english character sets */
  if((MetaHandling == ESCAPE) && meta &&(count > 0))
    {
      *pcount = count + 1;
      for(c = (kbuf + (count)); c > kbuf ; c--)
	{
	  *c = *(c-1);
	}
      *kbuf = 27;
      *(kbuf+count+1)=0;  
    }
  else if((MetaHandling == BIT) && meta &&(count > 0))
    {
      for(c = kbuf; c < kbuf+count ; c++)
	{
	  *c = *c | 0x80 ;
	}
      *(kbuf+count)=0;
    }
  else
    *(kbuf+count) = 0;
  return (kbuf);
}

/*  Return the next input character after first passing any keyboard input
 *  to the command.  If flags & BUF_ONLY is true then only buffered 
 *  characters are returned and once the buffer is empty the special value 
 *  GCC_NULL is returned.  GCC_NULL is defined as 0x100, so the return value
 *  has to be int. Otherwise NUL will falsely trigger the end of buffer 
 *  condition, and stop output. (VMS does this).
 */

/* made this global since we use it in handle_X_event, too. Matthias */ 
static int refreshed = 0;

static unsigned int get_com_char(int flags)
{
  fd_set in_fdset;
  int count,retval;
  unsigned char val;
  struct itimerval value;
/*   static int refreshed = 0; */
  int total;

  /* If there have been a lot of new lines, then update the screen */
  /* What the heck I'll cheat and only refresh less than every page-full.
   * the number of pages between refreshes is refresh_nl_limit, which
   * is incremented here because we must be doing flat-out scrolling. */
  /* refreshing should be correct for small scrolls, because of the 
   * time-out */
  if(refresh_nl_count > refresh_nl_limit*MyWinInfo.cheight)
    {
      if(refresh_nl_limit < MAX_REFRESH_PERIOD)
	refresh_nl_limit++;
      refresh_nl_count = 0;
      refreshed = 1;
      screen_refresh();
    }

  /* If we have characters already read in. return one */
  if (com_buf_next < com_buf_top)
    {
      refreshed = 0;
      return((unsigned int) (*com_buf_next++ & mask) );
    }
  /* Nothing to read, either return now or continue and wait */

  if (flags & BUF_ONLY) {
    refresh_nl_count = 0;
    refresh_nl_limit = 1; 
    if(!refreshed)
      screen_refresh();
    refreshed = 1;
    /* scrollbar movements are done in screen_refresh now (Matthias) */
    /*	  sbar_show(MyWinInfo.cheight+MyWinInfo.sline_top-1,MyWinInfo.offset,
	  MyWinInfo.offset + MyWinInfo.cheight -1); */
    XFlush(display);
    return ((unsigned int) GCC_NULL); 
  }

/*   for (;;)  */
/*     { */
       /* process any X events that are pending */ 
/*       while(XPending(display)) */
/* 	{ */
 /* 	  refreshed = 0;	   */
/* 	  get_X_event(); */
/* 	} */

       /* Nothing to do! */ 
       FD_ZERO(&in_fdset); 
       FD_SET(comm_fd,&in_fdset); 


       /* select handling slightly changed. Yet the "main" select (with blocking)
	  is inside the Qt event loop. Important. Matthias */

/*         value.it_value.tv_usec = 1000;   */
/*         value.it_value.tv_sec = 0;   */
/*         if(!refreshed)   */
/* 	  retval = select(fd_width,&in_fdset,NULL,NULL,&value.it_value);  */
/*         else   */
/* 	  retval = select(fd_width,&in_fdset,NULL,NULL,NULL);  */


/*        if (!refreshed){ */
/* 	 value.it_value.tv_usec = 1000;   */
/* 	 value.it_value.tv_sec = 0;   */
/*        } */
/*        else { */
/* 	 value.it_value.tv_usec = 0;   */
/* 	 value.it_value.tv_sec = 0;   */
/*        } */
       if (!refreshed){
	 value.it_value.tv_usec = 0;  
	 value.it_value.tv_sec = 1;  
       }
       else {
	 value.it_value.tv_usec = 0;  
	 value.it_value.tv_sec = 1;  
       }
       retval = select(fd_width,&in_fdset,NULL,NULL,&value.it_value); 


     /* See if we can read from the application */
       if(FD_ISSET(comm_fd,&in_fdset)) 
	{
	  count = 1;
	  com_buf_next = com_buf;
	  com_buf_top = com_buf;
	  total = COM_BUF_SIZE;
	  while((count > 0)&&(total>COM_BUF_SIZE/2))
	    {
	      count = read(comm_fd,com_buf_top,total);
	      if(count > 0)
		{
		  com_buf_top += count;
		  total -= count;
		}
	    }
	  if (com_buf_top > com_buf_next)
	    {
	      val = *com_buf_next++;
	      refreshed = 0;
	      return((unsigned int) (val & mask));	  
	    }
	}
      /* If the select statement timed out, we better update the screen */
       if(retval == 0 ) 
	 {
	  refresh_nl_count = 0;
	  refresh_nl_limit = 1; 
	  if(!refreshed)
	    screen_refresh();
	  refreshed = 1;
/* scrollbar movements are done in screen_refresh now (Matthias) */
/*	  sbar_show(MyWinInfo.cheight+MyWinInfo.sline_top-1,MyWinInfo.offset,
		    MyWinInfo.offset + MyWinInfo.cheight -1); */
	  XFlush(display);
	}

      return (unsigned int) GCC_NULL;
}

/****************************************************************************
 * Receive and process an X event
 **************************************************************************/

/* get to handle.   Matthias */ 
int handle_X_event(XEvent event, unsigned char qt_c)
{
  /* no longer needed. Matthias */ 
/*   XEvent event; */
  unsigned char *s;
  int count;
  Window root, child;
  int root_x, root_y, x, y;
  unsigned int mods;
  static Time buttonpress_time=0;
  static unsigned int clicks=1;

  /* this was set in get_com_char originally. Matthias */ 
  refreshed = 0;

  /* no longer needed. Matthias */ 
/*   XNextEvent(display,&event); */
  switch(event.type)
    {
    case KeyPress:
      s = lookup_key(&event,&count, qt_c);
      send_string(s,count);
      return 0;
    case ClientMessage:
      if (event.xclient.format == 32 && event.xclient.data.l[0] == wm_del_win)
	clean_exit(0);
      return 0;
    case MappingNotify:
      XRefreshKeyboardMapping(&event.xmapping);
      return 0;
    case GraphicsExpose:
    case Expose:
      if (!size_set) 
	{	      
	  /*  Force a window resize if an exposure event
	   *  arrives before the first resize event.
	   */
	  resize_window(0,0);
	  size_set = 1;
	}
      if(event.xany.window==vt_win)
	{
	  scr_refresh(event.xexpose.x,event.xexpose.y,
		      event.xexpose.width,event.xexpose.height);
	  return 0;
	}
      /* no longer needed */
/*       else  */
/* 	{ */
/* 	  while(XCheckTypedWindowEvent (display, event.xany.window, Expose, */
/* 					&dummy)); */
/* 	  while(XCheckTypedWindowEvent (display, event.xany.window,  */
/* 					GraphicsExpose, &dummy)); */

/* 	} */
      return 0;
    case VisibilityNotify:
      /* Here's my conclusiion:
       * If the window is completely onobscured, use bitblt's
       * to scroll. Even then, they're only used when doing partial
       * screen scrolling. When partially obscured, we have to fill
       * in the GraphicsExpose parts, which means that after each refresh,
       * we need to wait for the graphics expose or Noexpose events, 
       * which ought to make things real slow! */
      if(event.xvisibility.state == VisibilityUnobscured)
	{
  	  scr_refresh(0,0,MyWinInfo.cwidth,MyWinInfo.cheight); 
/* 	  refresh_type = FAST; */
	  /* use slow here. FAST seems to be buggy sometimes. (Matthias) */ 
	  refresh_type = SLOW;
	}
      else if(event.xvisibility.state == VisibilityPartiallyObscured)
	{
	  refresh_type = SLOW;
  	  scr_refresh(0,0,MyWinInfo.cwidth,MyWinInfo.cheight);  
	}
      else
	{
	  refresh_type = DONT_BOTHER;
	}
      break;

    case FocusIn:
      scr_focus(1);
      return 0;
    case FocusOut:
      scr_focus(0);
      return 0;
    case ConfigureNotify:
      resize_window(0,0);
      size_set = 1;
      return 0;
    case ButtonPress:
      if (event.xany.window == vt_win)
	{
	    if (mouse_rep && ((event.xbutton.state & ShiftMask) == 0)
		&& !mouse_block) {
		mouse_report (&(event.xbutton), 0);
		return 1;
	    } else {
	      switch (event.xbutton.button)
		{
		case Button1 :
		  /* recognize multiclick. bmg */
		    if (event.xbutton.time-buttonpress_time < MULTICLICK_TIME) {
		    clicks++;
		  } else {
		    clicks = 1;
		  }

		    /* don't react two times on the same event */
		    /* Matthias */
		    if (event.xbutton.time == buttonpress_time)
			clicks--;
		  buttonpress_time = event.xbutton.time;

		  scr_start_selection(clicks, event.xbutton.x,event.xbutton.y);
		  return 0;
		  /* deactivated since Button3 is for popupmenu. Matthias */ 
		  /* case Button3 : */
		  /* scr_extend_selection(event.xbutton.x,event.xbutton.y); */

		  /* Scroll up */
		case Button4:
		    MyWinInfo.offset += 3;
		    
		    /* check to make sure we aren't at the top already */
		    if (MyWinInfo.offset > MyWinInfo.sline_top)
			{
			    MyWinInfo.offset = MyWinInfo.sline_top;
			}
		    screen_refresh();
		    return 0;

		    /* Scroll down */
		case Button5:
		    MyWinInfo.offset -= 3;

		    /* check to make sure we aren't at the bottom already */
		    if (MyWinInfo.offset < 0)
			{
			    MyWinInfo.offset = 0;
			}
		    screen_refresh();
		    return 0;
		default:
		  return 0;
		}
	    }
	}
      return 0;
    case ButtonRelease:
      if (event.xany.window == vt_win)
	{
	    if (mouse_rep && ((event.xbutton.state & ShiftMask) == 0)
		&& !mouse_block) {
		mouse_report (&(event.xbutton), 1);
		return 1;
	    } else {
	      switch (event.xbutton.button)
		{
		case Button1:
		  /* no longer needed bmg */
		  /* case Button3: */

		  scr_make_selection(event.xbutton.time);
		  return 0;
		case Button2:
		    if ((event.xbutton.state & Mod1Mask) == 0)
			scr_paste_selection();
		  return 0;
		}
	    }
	}
      return 0;
    case MotionNotify:

      if (event.xany.window == vt_win && 
	  ((event.xbutton.state & Button1Mask) == Button1Mask)
	   && !(event.xbutton.state & Button2Mask))
	{
	  while(XCheckTypedWindowEvent(display, vt_win, MotionNotify, &event));
	  XQueryPointer(display,vt_win,&root,&child,
			&root_x,&root_y,&x,&y,&mods);
	  scr_extend_selection(x,y);
	  return 0;
	}
      return 0;
    default:
      return 0;
    }

    return 0;
}




/*  Send count characters directly to the command.
 */
void send_string(unsigned char *buf,int count)
{
  int i, write_bytes, wrote;
  
  if (count <= 0)
    return;
  
  i = 0;
  write_bytes = count;

  while(i < count)
    {
      wrote = write(comm_fd, buf, write_bytes);
      
      if(wrote >0)
	{
	  i += wrote;
	  write_bytes -= wrote;
	  buf += wrote;
	}
    }
}




/*  Send printf formatted output to the command.  Only used for small ammounts
 *  of data.
 */
void cprintf(unsigned char *fmt,...)
{
  va_list args;
  static unsigned char buf[1024];
  
  va_start(args,fmt);
  

  vsprintf(buf,fmt,args);
  va_end(args);
  send_string(buf,strlen(buf));
}

/***************************************************************************
 * Read a text string from the input buffer 
 **************************************************************************/
void read_string()
{
  unsigned char *ptr,c;
  int nl_count = 0;

  /* point to the start of the string */
  com_buf_next--;
  ptr = com_buf_next;

  while(com_buf_next < com_buf_top)
    {
      c = *com_buf_next;
      if(c>=' ' || c == '\n' || c == '\r' || c == '\t') 
	{
	  com_buf_next++;
	  if(c == '\n')
	    nl_count++;

	  /* should have MyWinInfo.cheight instead of 4 */
	  /*	  if(nl_count > 4)*/
	  if(refresh_nl_count + nl_count  > refresh_nl_limit*MyWinInfo.cheight)
	    {
	      refresh_nl_count += nl_count;
	      scr_add_lines(ptr,nl_count,com_buf_next - ptr);
	      return;
	    }
	}
      else
	{
	  refresh_nl_count += nl_count;
	  scr_add_lines(ptr,nl_count,com_buf_next - ptr);
	  return;
	}	
    }
  refresh_nl_count += nl_count;
  scr_add_lines(ptr,nl_count,com_buf_next - ptr);

  return;
}


/***************************************************************************
 * Read and process output from the application
 ***************************************************************************/
void get_token()
{
   unsigned int c;	/* magic token GCC_NULL is 0x100 so need an int*/ 


   /* somewhat changed. Matthias */ 
   /* Wait for something to do! */
   c = get_com_char(0);
   do {
     /* Got a character. Figure out what to do with it. */
      if(c>=' ' || c == '\n' || c == '\r' || c == '\t') 
	read_string();
      else
	{
	  switch(c)
	    {
	    case 0:         /* NUL does nothing */
	      break;
	    case EOF:
	      clean_exit(0);
	      break;
	    case 5:
	      cprintf("\033[?1;2c");/* I'm a VT100 w/ advanced video options */
	      break;
	    case 0xb:
	      scr_index(1);
	      break;
	    case 0xc:
	      scr_index(1);
	      break;
	    case ESC:
	      process_escape_sequence();
	      break;
	    case '\b' :
	      scr_backspace();
	      break;
	    case '\007' :	/* bell */
#ifdef MAPALERT
	      if (map_alert) XMapWindow(display,main_win);
#endif
	      XBell(display,0);
	      break;
	    case '\016':
	      scr_choose_charset(1);
	      break;
	    case '\017':
	      scr_choose_charset(0);
	      break;
	    }
	}
      c = get_com_char(1);
   }while(c != (unsigned int) GCC_NULL);
}


#ifdef PRINT_PIPE
void process_print_pipe()
{
  extern char *print_pipe;
  int c = 0;
  int pid, roura[2];
  char escape_seq [4] = "\033[4i";
  char rev_escape_seq [4] = "i4[\033";
  int index = 0;
  FILE *pipe_file=NULL;

  /* ****************** new part **************************** */
  if(pipe(roura)==-1){
    perror("kvt:pipe");
    return;
    }
  pid=fork();
  if(pid<0){
    perror("kvt:fork");
    return;
    }
  if(pid==0){
    if(close(0)==-1 || dup(roura[0])==-1 || close(roura[0])==-1 || close(roura[1])==-1){
      perror("kvt:child:dup");
      exit(1);
      }
    if(execlp(print_pipe,print_pipe,0)==-1){
      perror("kvt:exec");
      exit(1);
      }
    }
  else{
    if(close(roura[0])==-1)
      perror("kvt:close");
    if((pipe_file=fdopen(roura[1],"w"))==NULL)
      perror("kvt:fdopen");
    }        
  /* ********************* end ************************** */

  if (pipe_file == NULL)
    {
    fprintf (stderr, "rxvt: can't open printer pipe!\n");
    return ;
    }
  while (1)
    {
    c = get_com_char(0);
    
    if (c == escape_seq [index])
      index++;
    else 
      if (index)
        for (/*no init'ln*/; index > 0; index--)
          fputc (rev_escape_seq [index-1], pipe_file);
    
    if (index == 4) 
      {
      /* *********** some changes ************ */

      /*fflush (pipe_file);   --fclose flushes too */
      fclose (pipe_file);
      /* waitpid(pid,&stat,WUNTRACED); */
      return ;
      }

    if (index == 0) fputc (c, pipe_file);
    }
}

#endif
  
void process_escape_sequence()
{
  int c,a;

  c = get_com_char(0);
  switch(c)
    {
    case '[':
      process_csi_sequence();
      break;
    case ']':
      process_xterm_sequence();
      break;
    case '(':
      scr_set_charset(0,get_com_char(0));
      break;
    case ')':
      scr_set_charset(1,get_com_char(0));
      break;
    case '7':
      scr_save_cursor();
      break;
    case '8' :
      scr_restore_cursor();
      break;
    case '=' :
      app_kp_keys = 1;
      break;
    case '>' :
      app_kp_keys = 0;
      break;
    case 'D' :
      scr_index(1);
      break;
    case 'E':
      scr_add_lines("\n\r",1,2);
      break;
    case 'M' :
      scr_index(-1);
      break;
    case 'Z' :
      /* cprintf("\033[?6c"); */	/* I am a VT102 */
      cprintf("\033[?1;2c");	/* changed to vt100 (Matthias) */ 
      break;
    case 'c' :                  /* restore power-on values */
      scr_power_on();
      break;
    case '#':                /* set character size */
      a=get_com_char(0);
      if(a=='8')
	scr_E();
      break;
    case 'H' :               /* horizontal tab set */
      scr_set_tab(1);
      break;
    case 'G' :
      process_robs_sequence();
    default:
      return;
    }
} 


void process_csi_sequence()
{
  int c,n,i,nargs,private=0;
  int arg[ARGS_MAX];
  extern char *display_name;

  arg[0]=0;
  arg[1]=0;

  c = get_com_char(0);
    if (c >= '<' && c <= '?') 
    {
      private = c;
      c = get_com_char(0);
    }

  /*  read any numerical arguments
   */
  i = 0;
  do 
    {
      n = 0;
      while (c >= '0' && c <= '9') 
	{
	  n = n * 10 + c - '0';
	  c = get_com_char(0);
	}
      if (i < ARGS_MAX)
	arg[i++] = n;
      if (c == ESC)
	{
	  process_escape_sequence();
	  return;
	}
      if(c == '\b')
	scr_backspace();
      else if (c < ' ')
	{
	  scr_add_lines((unsigned char *)&c,0,1);
	  return;
	}
      if (c < '@')
	c = get_com_char(0);
    } while (c < '@' && c >= ' ');
  if (c == ESC)
    {
      process_escape_sequence();
      return;
    }
  if (c < ' ')
    return;
  nargs = i;
  switch (c) 
    {
#ifdef PRINT_PIPE
    case 'i':   /* start print_pipe */
      if (arg[0] == 5) process_print_pipe();
      break;
#endif
    case 'A':	/* cursor up */
      scr_move(0,((arg[0] == 0) ? -1 : -arg[0]),ROW_RELATIVE | COL_RELATIVE);
      break;
    case 'B' :	/* cursor down */
      scr_move(0,((arg[0] == 0) ? 1 : arg[0]),ROW_RELATIVE | COL_RELATIVE);
      break; 
   case 'C' :	/* cursor forward */
      scr_move(((arg[0] == 0) ? 1 : arg[0]),0,ROW_RELATIVE | COL_RELATIVE);
      break;
    case 'D' :	/* cursor back */
      scr_move(((arg[0] == 0) ? -1 : -arg[0]),0,ROW_RELATIVE | COL_RELATIVE);
      break;

      /* the next are from me (Matthias) */ 
    case 'G' :	/* move to col */
      scr_move(((arg[0] == 0) ? 1 : arg[0]-1),0,ROW_RELATIVE);
      break;
    case 'd' :	/* move to row */
      scr_move(0, ((arg[0] == 0) ? 1 : arg[0]-1),COL_RELATIVE);
      break;


    case 'f' :
    case 'H' :	/* position cursor */
      if(nargs==0)
	scr_move(0,0,0);
      else if (nargs == 1)
	scr_move(0,((arg[0]==0)? 0 : (arg[0]-1)),0);
      else 
	scr_move(arg[1] - 1,arg[0] - 1,0);
      break;
    case 'J' :
      scr_erase_screen(arg[0]);
      break;
    case 'K' :
      scr_erase_line(arg[0]);
      break;
    case 'L' :
      scr_insert_delete_lines( (arg[0]==0) ? 1 : arg[0],INSERT);
      break;
    case 'M' :
      scr_insert_delete_lines( (arg[0]==0) ? 1 : arg[0],DELETE);
      break;
    case 'P' :
      scr_insert_delete_characters( (arg[0]==0) ? 1 : arg[0],DELETE);
      break;
    case '@' :
      scr_insert_delete_characters( (arg[0]==0) ? 1 : arg[0],INSERT);
      break;
    case 'c' :
      /* cprintf("\033[?6c");*/	/* I am a VT102 */ 
      cprintf("\033[?1;2c");	/* changed to vt100 (Matthias) */
      break;
    case 's': /* Matthias */ 
    case 'h' :
    case 'l' :
      process_terminal_mode(c,private,nargs,arg);
      break;
    case 'm' :
      process_sgr_mode(c,private,nargs,arg);
      break;
    case 'n' :		/* request for information */
      switch (arg[0]) 
	{
	case 6 :
	  scr_report_position();
	  break;
        case 7 :
          cprintf(display_name);
          break;
	}
      break;
    case 'r' :		/* set top and bottom margins */
      /* what's this about? something to do with vi on ESIX systems */
      if (private == '?') break;
      if (nargs < 2 || arg[0] >= arg[1])
	scr_set_margins(0,10000);
      else
	scr_set_margins(arg[0] - 1,arg[1] - 1);
      break;
    case 'g' :                  /* tab clear */
      if((nargs == 1)&&(arg[0] == 3))
	/* clear all tabs stops */
	scr_set_tab(-1);
      else if ((nargs == 0)||((nargs == 1)&&(arg[0] == 0)))
	scr_set_tab(0);
      break;
    case 'X' :
      /* Matthias */
      scr_erase_char(arg[0]);
      break;
    default:
      fprintf(stderr, "kvt: unknown CSI-sequence '%c'\n", c);
    }
}



void process_xterm_sequence()
{
  int c,n,i,arg;
  unsigned char string[STRING_MAX+1];

  c = get_com_char(0);
  n = 0;
  while (c >= '0' && c <= '9') 
    {
      n = n * 10 + c - '0';
      c = get_com_char(0);
    }
  arg = n;

  c = get_com_char(0);
  i = 0;
  while ( c != 7 && c != GCC_NULL && i < STRING_MAX)
    {
      if (c >= ' ')
	string[i++] = c;
      c = get_com_char(0);
    }
  string[i] = 0;
  switch (arg) 
    {
    case 0 :
      change_window_name(string);
      change_icon_name(string);
      break;
    case 1 :
      change_icon_name(string);
      break;
    case 2 :
      change_window_name(string);
      break;

    }
}


void process_terminal_mode(int c,int private,int nargs,int *arg)
{
  int mode;
  int w = 1 ,h = 1;
  extern XSizeHints sizehints;
  mode = (c == 'h') ? HIGH : LOW;
  if (private == '?') 
    {
      switch (arg[0]) 
	{
	case 1 :
	  app_cur_keys = (mode == HIGH);
	  break;
	case 3:                 /* 132 columns mode is N/A */
	  if(mode == HIGH)
	    w = 132*MyWinInfo.fwidth + sizehints.base_width;
	  else
	    w = 80*MyWinInfo.fwidth + sizehints.base_width;
	  h = MyWinInfo.cheight*MyWinInfo.fheight + sizehints.base_height;
	  sizehints.width = w;
	  sizehints.height = h;
	  sizehints.flags = PMinSize | PResizeInc | PBaseSize | PWinGravity;
 	  XSetWMNormalHints(display,main_win,&sizehints); 
 	  XResizeWindow(display, main_win, w, h); 
	  XClearWindow(display,vt_win);
  	  XSync(display,0);
	  resize_window(w,h);
	  break;
	case 4:                 /* Smooth scrolling is N/A */
	                        /* Could switch to single line scroll after */
	                        /* receiving this, but why bother? */
	  break;
	case 5:
	  /* reverse video */
	  scr_rev_vid(mode);
	  break; /* this took me hours to find .... Matthias */ 
	case 6 :
	  scr_set_decom(mode);
	  break;
	case 7 :
	  scr_set_wrap(mode);
	  break;
	case 8:                 /* auto repeat is N/A */
	  break;
	 case 25:		/* visible/invisible cursor */
	   scr_cursor_visible(mode); /* Matthias */ 
	   break;
	case 47 :		/* switch to main screen */
	  scr_change_screen(mode);
	  break;
	case 1000:		/* X11 mouse reporting */
	  mouse_rep = mode;
	  /* adjust mouse cursor to use */
	  adjust_cursor(mode);
	  break;
	}
    }
  else if (private == 0) 
    {
      if(arg[0]==4)
	scr_set_insert(mode);
    }
}


void process_sgr_mode(int c,int private,int nargs,int *arg)
{
  int i;

  if (nargs == 0)
    scr_change_rendition(1,~RS_NONE);
  else
    for (i = 0; i < nargs; i++) 
      switch (arg[i]) 
	{
	case 0 :
	  scr_change_rendition(1,~RS_NONE);
	  break;
	case 1 :
	  scr_change_rendition(0,RS_BOLD);
	  break;
	case 4 :
	  scr_change_rendition(0,RS_ULINE);
	  break;
	case 5 :
/*	  scr_change_rendition(0,RS_BLINK);*/
	  break;
	case 7 :
	  scr_change_rendition(0,RS_RVID);
	  break;
	case 22 :
	  scr_change_rendition(1,RS_BOLD);
	  break;
	case 24 :
	  scr_change_rendition(1,RS_ULINE);
	  break;
	case 25 :
/*	  scr_change_rendition(1,RS_BLINK);*/
	  break;
	case 27 :
	  scr_change_rendition(1,RS_RVID);
	  break;
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	  scr_fore_color(arg[i]);
	  break;
	case 39:
	  scr_fore_color(28); /* Matthias */ 
	  break;
	case 40:
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	  scr_back_color(arg[i]);
	  break;
	case 49:
	  scr_back_color(39); /* Matthias */ 
	  break;
	default:
	  printf("unkown sgr sequence %d\n", arg[i]);
	}
}


void process_robs_sequence(void)
{
  unsigned char key,c,c1;
  int done,i,nargs,negative;
  long args[1000];
  unsigned char text[1000];

  (void)key;

  nargs = 0;
  done = 0;
  args[nargs] = 0;
  negative = 0;
  c1 = get_com_char(0);
  while((!done)&&(nargs < 999))
    {
      while(isdigit(c = get_com_char(0))||(c == '-'))
	{
	  if(c == '-')
	    negative = 1-negative;
	  args[nargs] = args[nargs]*10+(c-'0');
	}
      if(negative)args[nargs] = -args[nargs];
      if(c!= ';')
	done = 1;
      nargs++;
      args[nargs] = 0;
    }
  i=0;
  if((c1 == 'T')&&(nargs>=5))
    {
      while((i<args[nargs-1])&&(i<999))
	text[i++] = get_com_char(0);
    }
  text[i] = 0;

}



#ifdef NO_SETSID
setsid()
{
  if (setpgrp(0,0) < 0) return(-1);
  return(0);
}
#endif


