/* $XConsortium: resource.c,v 1.47 94/04/17 20:03:43 gildea Exp $ */
/* $XFree86: xc/programs/xdm/resource.c,v 3.1 1994/12/29 10:22:27 dawes Exp $ */
/* $Id: resource.c,v 1.7 1999/01/28 14:10:02 kulow Exp $ */
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
 * resource.c
 */

# include <X11/Intrinsic.h>
# include "dm.h"
/*# include <X11/Xmu/CharSet.h>*/

char	*config;

char	*servers;
int	request_port;
int	debugLevel;
char	*errorLogFile;
int	daemonMode;
char	*pidFile;
int	lockPidFile;
char	*authDir;
int	autoRescan;
int	removeDomainname;
char	*keyFile;
char	*accessFile;
char	**exportList;
char	*randomFile;
char	*greeterLib;
int	choiceTimeout;	/* chooser choice timeout */

# define DM_STRING	0
# define DM_INT		1
# define DM_BOOL	2
# define DM_ARGV	3

/*
 * the following constants are supposed to be set in the makefile from
 * parameters set util/imake.includes/site.def (or *.macros in that directory
 * if it is server-specific).  DO NOT CHANGE THESE DEFINITIONS!
 *
 * On the other hand we don't use imake anymore so we have to define the
 * variables here.
 */
#ifndef __EMX__
#define Quote(s) #s
#define QUOTE(s) Quote(s)
#ifndef DEF_SERVER_LINE 
#define DEF_SERVER_LINE ":0 local " QUOTE(XBINDIR) "/X :0"
#endif
#ifndef XRDB_PROGRAM
/* use krdb instead? */
#define XRDB_PROGRAM QUOTE(XBINDIR) "/xrdb"
#endif
#ifndef DEF_SETUP
/* Should be run this as default? */
#define DEF_SETUP XDMDIR"/Xsetup"
#endif
#ifndef DEF_SESSION
#define DEF_SESSION XDMDIR"/Xsession" /* QUOTE(XBINDIR) "xterm -ls" */
#endif
#ifndef DEF_USER_PATH
#  ifdef __FreeBSD__
#    define DEF_USER_PATH "/bin:/usr/bin:" QUOTE(XBINDIR) ":/usr/local/bin"
#  elif __linux__
#    define DEF_USER_PATH "/bin:/usr/bin:" QUOTE(XBINDIR) ":/usr/local/bin"
#  else
#    define DEF_USER_PATH "/bin:/usr/bin:" QUOTE(XBINDIR) ":/usr/local/bin:/usr/ucb"
#  endif
#endif
#ifndef DEF_SYSTEM_PATH
#  ifdef __FreeBSD__
#    define DEF_SYSTEM_PATH "/sbin:/usr/sbin:/bin:/usr/bin:" QUOTE(XBINDIR) ":/usr/local/bin"
#  elif __linux__
#    define DEF_SYSTEM_PATH "/sbin:/usr/sbin:/bin:/usr/bin:" QUOTE(XBINDIR) ":/usr/local/bin"
#  else
#    define DEF_SYSTEM_PATH "/sbin:/usr/sbin:/bin:/usr/bin:" QUOTE(XBINDIR) ":/usr/local/bin:/etc:/usr/ucb"
#  endif
#endif
#ifndef DEF_SYSTEM_SHELL
#  ifdef _PATH_BSHELL
#    define DEF_SYSTEM_SHELL _PATH_BSHELL
#  else
#    define DEF_SYSTEM_SHELL "/bin/sh"
#  endif
#endif
#ifndef DEF_FAILSAFE_CLIENT
#define DEF_FAILSAFE_CLIENT QUOTE(XBINDIR) "/xterm"
#endif
#ifndef DEF_XDM_CONFIG
#define DEF_XDM_CONFIG XDMDIR "/xdm-config"
#endif
#ifndef DEF_CHOOSER
#define DEF_CHOOSER XDMDIR "/chooser"
#endif
#ifndef DEF_AUTH_NAME
#ifdef HASXDMAUTH
#define DEF_AUTH_NAME	"XDM-AUTHORIZATION-1 MIT-MAGIC-COOKIE-1"
#else
#define DEF_AUTH_NAME	"MIT-MAGIC-COOKIE-1"
#endif
#endif
#ifndef DEF_AUTH_DIR
/* This may be read only
 */
#define DEF_AUTH_DIR XDMDIR
#endif
#ifndef DEF_USER_AUTH_DIR
#define DEF_USER_AUTH_DIR	"/tmp"
#endif
#ifndef DEF_KEY_FILE
#define DEF_KEY_FILE	""
#endif
#ifndef DEF_ACCESS_FILE
#define DEF_ACCESS_FILE	""
#endif
#ifndef DEF_RANDOM_FILE
#define DEF_RANDOM_FILE _PATH_MEM
#endif
#ifndef DEF_GREETER_LIB
#define DEF_GREETER_LIB ""
#endif
#ifndef DEF_PID_FILE
#  ifdef __FreeBSD__
#    define DEF_PID_FILE _PATH_VARRUN"kdm.pid"
#  else
/* this may be readonly
 */
#    define DEF_PID_FILE XDMDIR "/kdm-pid"
#  endif
#endif
#else
/* unfortunately I have to declare all of them, because there is a limit
 * in argument size in OS/2
 * but everything needs to be fixed again
 */
#define DEF_SERVER_LINE ":0 local /XFree86/bin/X :0"
#ifndef XRDB_PROGRAM
#define XRDB_PROGRAM "/XFree86/bin/xrdb"
#endif
#ifndef DEF_SESSION
#define DEF_SESSION "/XFree86/bin/xterm -ls"
#endif
#ifndef DEF_USER_PATH
#define DEF_USER_PATH "c:\\os2;c:\\os2\apps;\\XFree86\\bin"
#endif
#ifndef DEF_SYSTEM_PATH
#define DEF_SYSTEM_PATH "c:\\os2;c:\\os2\apps;\\XFree86\\bin"
#endif
#ifndef DEF_SYSTEM_SHELL
#define DEF_SYSTEM_SHELL "sh"
#endif
#ifndef DEF_FAILSAFE_CLIENT
#define DEF_FAILSAFE_CLIENT "/XFree86/bin/xterm"
#endif
#ifndef DEF_XDM_CONFIG
#define DEF_XDM_CONFIG "/XFree86/lib/X11/xdm/xdm-config"
#endif
#ifndef DEF_CHOOSER
#define DEF_CHOOSER "/XFree86/lib/X11/xdm/chooser"
#endif
#ifndef DEF_AUTH_NAME
#ifdef HASXDMAUTH
#define DEF_AUTH_NAME	"XDM-AUTHORIZATION-1 MIT-MAGIC-COOKIE-1"
#else
#define DEF_AUTH_NAME	"MIT-MAGIC-COOKIE-1"
#endif
#endif
#ifndef DEF_AUTH_DIR
#define DEF_AUTH_DIR "/XFree86/lib/X11/xdm"
#endif
#ifndef DEF_USER_AUTH_DIR
#define DEF_USER_AUTH_DIR	"/tmp"
#endif
#ifndef DEF_KEY_FILE
#define DEF_KEY_FILE	""
#endif
#ifndef DEF_ACCESS_FILE
#define DEF_ACCESS_FILE	""
#endif
#ifndef DEF_RANDOM_FILE
#define DEF_RANDOM_FILE ""
#endif
#ifndef DEF_GREETER_LIB
#define DEF_GREETER_LIB "/XFree86/lib/X11/xdm/libXdmGreet.so"
#endif

#endif /* __EMX__ */

#define DEF_UDP_PORT	"177"	    /* registered XDMCP port, dont change */

struct dmResources {
	char	*name, *class;
	int	type;
	char	**dm_value;
	char	*default_value;
} DmResources[] = {
{"servers",	"Servers", 	DM_STRING,	&servers,
				DEF_SERVER_LINE},
{"requestPort",	"RequestPort",	DM_INT,       	(char **) &request_port,
				DEF_UDP_PORT},
{"debugLevel",	"DebugLevel",	DM_INT,		(char **) &debugLevel,
				"0"},
{"errorLogFile",	"ErrorLogFile",	DM_STRING,	&errorLogFile,
				""},
{"daemonMode",	"DaemonMode",	DM_BOOL,	(char **) &daemonMode,
				"true"},
{"pidFile",	"PidFile",	DM_STRING,	&pidFile,
				DEF_PID_FILE},
{"lockPidFile",	"LockPidFile",	DM_BOOL,	(char **) &lockPidFile,
				"true"},
{"authDir",	"authDir",	DM_STRING,	&authDir,
				DEF_AUTH_DIR},
{"autoRescan",	"AutoRescan",	DM_BOOL,	(char **) &autoRescan,
				"true"},
{"removeDomainname","RemoveDomainname",DM_BOOL,	(char **) &removeDomainname,
				"true"},
{"keyFile",	"KeyFile",	DM_STRING,	&keyFile,
				DEF_KEY_FILE},
{"accessFile",	"AccessFile",	DM_STRING,	&accessFile,
				DEF_ACCESS_FILE},
{"exportList",	"ExportList",	DM_ARGV,	(char **) &exportList,
				""},
{"randomFile",	"RandomFile",	DM_STRING,	&randomFile,
				DEF_RANDOM_FILE},
{"greeterLib",	"GreeterLib",	DM_STRING,	&greeterLib,
				DEF_GREETER_LIB},
{"choiceTimeout","ChoiceTimeout",DM_INT,	(char **) &choiceTimeout,
				"15"}
};

# define NUM_DM_RESOURCES	(sizeof DmResources / sizeof DmResources[0])

# define boffset(f)	XtOffsetOf(struct display, f)

struct displayResource {
	char	*name, *class;
	int	type;
	int	offset;
	char	*default_value;
};

/* resources for managing the server */

struct displayResource serverResources[] = {
{"serverAttempts","ServerAttempts",DM_INT,	boffset(serverAttempts),
				"1"},
{"openDelay",	"OpenDelay",	DM_INT,		boffset(openDelay),
				"15"},
{"openRepeat",	"OpenRepeat",	DM_INT,		boffset(openRepeat),
				"5"},
{"openTimeout",	"OpenTimeout",	DM_INT,		boffset(openTimeout),
				"120"},
{"startAttempts","StartAttempts",DM_INT,       	boffset(startAttempts),
				"4"},
{"pingInterval",	"PingInterval",	DM_INT,	boffset(pingInterval),
				"5"},
{"pingTimeout",	"PingTimeout",	DM_INT,		boffset(pingTimeout),
				"5"},
{"terminateServer","TerminateServer",DM_BOOL,	boffset(terminateServer),
				"false"},
{"grabServer",	"GrabServer",	DM_BOOL,	boffset(grabServer),
				"false"},
{"grabTimeout",	"GrabTimeout",	DM_INT,		boffset(grabTimeout),
				"3"},
{"resetSignal",	"Signal",	DM_INT,		boffset(resetSignal),
				"1"},	/* SIGHUP */
{"termSignal",	"Signal",	DM_INT,		boffset(termSignal),
				"15"},	/* SIGTERM */
{"resetForAuth",	"ResetForAuth",	DM_BOOL,	boffset(resetForAuth),
				"false"},
{"authorize",	"Authorize",	DM_BOOL,	boffset(authorize),
				"true"},
{"authComplain",	"AuthComplain",	DM_BOOL,	boffset(authComplain),
				"true"},
{"authName",	"AuthName",	DM_ARGV,	boffset(authNames),
				DEF_AUTH_NAME},
{"authFile",	"AuthFile",	DM_STRING,	boffset(clientAuthFile),
				""},
};

# define NUM_SERVER_RESOURCES	(sizeof serverResources/\
				 sizeof serverResources[0])

/* resources which control the session behaviour */

struct displayResource sessionResources[] = {
{"resources",	"Resources",	DM_STRING,	boffset(resources),
				""},
{"xrdb",	"Xrdb",		DM_STRING,	boffset(xrdb),
				XRDB_PROGRAM},
{"setup",	"Setup",	DM_STRING,	boffset(setup),
				DEF_SETUP},
{"startup",	"Startup",	DM_STRING,	boffset(startup),
				""},
{"reset",	"Reset",	DM_STRING,	boffset(reset),
				""},
{"session",	"Session",	DM_STRING,	boffset(session),
				DEF_SESSION},
{"userPath",	"Path",		DM_STRING,	boffset(userPath),
				DEF_USER_PATH},
{"systemPath",	"Path",		DM_STRING,	boffset(systemPath),
				DEF_SYSTEM_PATH},
{"systemShell",	"Shell",	DM_STRING,	boffset(systemShell),
				DEF_SYSTEM_SHELL},
{"failsafeClient","FailsafeClient",  DM_STRING,	boffset(failsafeClient),
				DEF_FAILSAFE_CLIENT},
{"userAuthDir",	"UserAuthDir",	DM_STRING,	boffset(userAuthDir),
				DEF_USER_AUTH_DIR},
{"chooser",	"Chooser",	DM_STRING,	boffset(chooser),
				DEF_CHOOSER},
};

# define NUM_SESSION_RESOURCES	(sizeof sessionResources/\
				 sizeof sessionResources[0])

XrmDatabase	DmResourceDB;

extern void freeArgs( char **argv );

void GetResource (name, class, valueType, valuep, default_value)
    char    *name, *class;
    int	    valueType;
    char    **valuep;
    char    *default_value;
{
    char	*type;
    XrmValue	value;
    char	*string, *new_string;
    char	str_buf[50];
    /*char        *str_it;*/
    int	len;
    extern char **parseArgs();

    if (DmResourceDB && XrmGetResource (DmResourceDB,
	name, class,
	&type, &value))
    {
	string = value.addr;
	len = value.size;
    }
    else
    {
	string = default_value;
	len = strlen (string);
    }

    Debug ("%s/%s value %*.*s\n", name, class, len, len, string);

    if (valueType == DM_STRING && *valuep)
    {
	if (strlen (*valuep) == len && !strncmp (*valuep, string, len))
	    return;
	else
	    free (*valuep);
    }

    switch (valueType) {
    case DM_STRING:
	new_string = malloc ((unsigned) (len+1));
	if (!new_string) {
		LogOutOfMem ("GetResource");
		return;
	}
	strncpy (new_string, string, len);
	new_string[len] = '\0';
	*(valuep) = new_string;
	break;
    case DM_INT:
	strncpy (str_buf, string, sizeof (str_buf));
	str_buf[sizeof (str_buf)-1] = '\0';
	*((int *) valuep) = atoi (str_buf);
	break;
    case DM_BOOL:
	strncpy (str_buf, string, sizeof (str_buf));
	str_buf[sizeof (str_buf)-1] = '\0';
	/*XmuCopyISOLatin1Lowered (str_buf, str_buf);*/
	/*str_it = str_buf;
	while( *str_it != '\0')
	     tolower( str_it++);*/
	if (!strcasecmp (str_buf, "true") ||
	    !strcasecmp (str_buf, "on") ||
	    !strcasecmp (str_buf, "yes"))
		*((int *) valuep) = 1;
	else if (!strcasecmp (str_buf, "false") ||
		 !strcasecmp (str_buf, "off") ||
		 !strcasecmp (str_buf, "no"))
		*((int *) valuep) = 0;
	break;
    case DM_ARGV:
	freeArgs (*(char ***) valuep);
	*((char ***) valuep) = parseArgs ((char **) 0, string);
	break;
    }
}

XrmOptionDescRec configTable [] = {
{"-server",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-udpPort",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-error",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-resources",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-session",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-debug",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-xrm",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-config",	".configFile",		XrmoptionSepArg,	(caddr_t) NULL }
};

XrmOptionDescRec optionTable [] = {
{"-server",	".servers",		XrmoptionSepArg,	(caddr_t) NULL },
{"-udpPort",	".requestPort",		XrmoptionSepArg,	(caddr_t) NULL },
{"-error",	".errorLogFile",	XrmoptionSepArg,	(caddr_t) NULL },
{"-resources",	"*resources",		XrmoptionSepArg,	(caddr_t) NULL },
{"-session",	"*session",		XrmoptionSepArg,	(caddr_t) NULL },
{"-debug",	"*debugLevel",		XrmoptionSepArg,	(caddr_t) NULL },
{"-xrm",	NULL,			XrmoptionResArg,	(caddr_t) NULL },
{"-daemon",	".daemonMode",		XrmoptionNoArg,		"true"         },
{"-nodaemon",	".daemonMode",		XrmoptionNoArg,		"false"        }
};

static int	originalArgc;
static char	**originalArgv;

void ReinitResources();

void InitResources (argc, argv)
int	argc;
char	**argv;
{
	XrmInitialize ();
	originalArgc = argc;
	originalArgv = argv;
	ReinitResources ();
}

extern void LogPanic();

void ReinitResources ()
{
    int	argc;
    char	**a;
    char	**argv;
    XrmDatabase newDB;

    argv = (char **) malloc ((originalArgc + 1) * sizeof (char *));
    if (!argv)
	LogPanic ("no space for argument realloc\n");
    for (argc = 0; argc < originalArgc; argc++)
	argv[argc] = originalArgv[argc];
    argv[argc] = 0;
    if (DmResourceDB)
	XrmDestroyDatabase (DmResourceDB);
    DmResourceDB = XrmGetStringDatabase ("");
    /* pre-parse the command line to get the -config option, if any */
    XrmParseCommand (&DmResourceDB, configTable,
		     sizeof (configTable) / sizeof (configTable[0]),
		     "DisplayManager", &argc, argv);
    GetResource ("DisplayManager.configFile", "DisplayManager.ConfigFile",
		 DM_STRING, &config, DEF_XDM_CONFIG);
    newDB = XrmGetFileDatabase ( config );
    if (newDB)
    {
	if (DmResourceDB)
	    XrmDestroyDatabase (DmResourceDB);
	DmResourceDB = newDB;
    }
    else if (argc != originalArgc)
	LogError ("Can't open configuration file %s\n", config );
    XrmParseCommand (&DmResourceDB, optionTable,
		     sizeof (optionTable) / sizeof (optionTable[0]),
		     "DisplayManager", &argc, argv);
    if (argc > 1)
    {
	LogError ("extra arguments on command line:");
	for (a = argv + 1; *a; a++)
		LogError (" \"%s\"", *a);
	LogError ("\n");
    }
    free (argv);
}

void LoadDMResources ()
{
	int	i;
	char	name[1024], class[1024];

	for (i = 0; i < NUM_DM_RESOURCES; i++) {
		sprintf (name, "DisplayManager.%s", DmResources[i].name);
		sprintf (class, "DisplayManager.%s", DmResources[i].class);
		GetResource (name, class, DmResources[i].type,
			      (char **) DmResources[i].dm_value,
			      DmResources[i].default_value);
	}
}

static
void CleanUpName (src, dst, len)
char	*src, *dst;
int	len;
{
    while (*src) {
	if (--len <= 0)
		break;
	switch (*src)
	{
	case ':':
	case '.':
	    *dst++ = '_';
	    break;
	default:
	    *dst++ = *src;
	}
	++src;
    }
    *dst = '\0';
}

void LoadDisplayResources (d, resources, numResources)
    struct display	    *d;
    struct displayResource  *resources;
    int			    numResources;
{
    int	i;
    char	name[1024], class[1024];
    char	dpyName[512], dpyClass[512];

    CleanUpName (d->name, dpyName, sizeof (dpyName));
    CleanUpName (d->class2 ? d->class2 : d->name, dpyClass, sizeof (dpyClass));
    for (i = 0; i < numResources; i++) {
	    sprintf (name, "DisplayManager.%s.%s", 
		    dpyName, resources[i].name);
	    sprintf (class, "DisplayManager.%s.%s",
		    dpyClass, resources[i].class);
	    GetResource (name, class, resources[i].type,
			  (char **) (((char *) d) + resources[i].offset),
			  resources[i].default_value);
    }
}

void LoadServerResources (d)
    struct display  *d;
{
    LoadDisplayResources (d, serverResources, NUM_SERVER_RESOURCES);
}

void LoadSessionResources (d)
    struct display  *d;
{
    LoadDisplayResources (d, sessionResources, NUM_SESSION_RESOURCES);
}
