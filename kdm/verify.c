/* $XConsortium: verify.c,v 1.32 94/04/17 20:03:55 gildea Exp $ */
/* $XFree86: xc/programs/xdm/greeter/verify.c,v 3.0 1994/06/26 13:12:06 dawes Exp $ */
/* $Id$ */
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
 * verify.c
 *
 * typical unix verification routine.
 */

# include	"dm.h"
# include	<pwd.h>
# ifdef NGROUPS_MAX
# include	<grp.h>
# endif
#ifdef USE_PAM
#include <security/pam_appl.h>
#ifdef KDE_PAM_SERVICE
#define KDE_PAM KDE_PAM_SERVICE
#else  
#define KDE_PAM "xdm"  /* default PAM service called by kdm */
#endif 
#else /* ! USE_PAM */
#ifdef USESHADOW
# include	<shadow.h>
#endif
#endif /* USE_PAM */

# include	"greet.h"

#ifdef X_NOT_STDC_ENV
char *getenv();
#endif

static char *envvars[] = {
#if defined(sony) && !defined(SYSTYPE_SYSV) && !defined(_SYSTYPE_SYSV)
    "bootdev",
    "boothowto",
    "cputype",
    "ioptype",
    "machine",
    "model",
    "CONSDEVTYPE",
    "SYS_LANGUAGE",
    "SYS_CODE",
    "TZ",
#endif
#if (defined(SVR4) || defined(SYSV)) && defined(i386) && !defined(sun)
    "TZ",
    "XLOCAL",
#endif
    NULL
};

static char **
userEnv (d, useSystemPath, user, home, shell)
struct display	*d;
int	useSystemPath;
char	*user, *home, *shell;
{
    char	**env;
    char	**envvar;
    char	*str;
    extern char **defaultEnv (), **setEnv ();
    
    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    env = setEnv (env, "HOME", home);
    env = setEnv (env, "USER", user);
#if defined(SYSV) || defined(SVR4)
    env = setEnv (env, "LOGNAME", user);
#endif
    env = setEnv (env, "PATH", useSystemPath ? d->systemPath : d->userPath);
    env = setEnv (env, "SHELL", shell);
    for (envvar = envvars; *envvar; envvar++)
    {
	if ( (str = getenv(*envvar)))
	    env = setEnv (env, *envvar, str);
    }
    return env;
}

#ifdef NGROUPS_MAX
static int
groupMember (name, members)
    char *name;
    char **members;
{
	while (*members) {
		if (!strcmp (name, *members))
			return 1;
		++members;
	}
	return 0;
}

static void
getGroups (name, verify, gid)
    char		*name;
    struct verify_info	*verify;
    int			gid;
{
	int		ngroups;
	struct group	*g;
	int		i;

	ngroups = 0;
	verify->groups[ngroups++] = gid;
	setgrent ();
	/* SUPPRESS 560 */
	while ( (g = getgrent())) {
		/*
		 * make the list unique
		 */
		for (i = 0; i < ngroups; i++)
			if (verify->groups[i] == g->gr_gid)
				break;
		if (i != ngroups)
			continue;
		if (groupMember (name, g->gr_mem)) {
			if (ngroups >= NGROUPS_MAX)
				LogError ("%s belongs to more than %d groups, %s ignored\n",
					name, NGROUPS_MAX, g->gr_name);
			else
				verify->groups[ngroups++] = g->gr_gid;
		}
	}
	verify->ngroups = ngroups;
	endgrent ();
}
#endif /* NGROUPS_MAX */


#ifdef USE_PAM
static char *PAM_password;

static int PAM_conv (int num_msg,
		     const struct pam_message **msg,
		     struct pam_response **resp,
		     void *appdata_ptr) {
	int count = 0, replies = 0;
	struct pam_response *reply = NULL;
	int size = sizeof(struct pam_response);

	#define GET_MEM if (reply) realloc(reply, size); else reply = malloc(size); \
	if (!reply) return PAM_CONV_ERR; \
	size += sizeof(struct pam_response)
	#define COPY_STRING(s) (s) ? strdup(s) : NULL

	for (count = 0; count < num_msg; count++) {
		switch (msg[count]->msg_style) {
		case PAM_PROMPT_ECHO_ON:
			/* user name given to PAM already */
			return PAM_CONV_ERR;
		case PAM_PROMPT_ECHO_OFF:
			/* wants password */
			GET_MEM;
			reply[replies].resp_retcode = PAM_SUCCESS;
			reply[replies++].resp = COPY_STRING(PAM_password);
			/* PAM frees resp */
			break;
		case PAM_TEXT_INFO:
			break;
		default:
			/* unknown or PAM_ERROR_MSG */
			if (reply) free (reply);
			return PAM_CONV_ERR;
		}
	}
	if (reply) *resp = reply;
	return PAM_SUCCESS;
}

static struct pam_conv PAM_conversation = {
	&PAM_conv,
	NULL
};

/* for pam session (see session.c)*/
pam_handle_t *pamh;
#endif /* USE_PAM */

extern void printEnv( char **e );

int Verify (d, greet, verify)
struct display		*d;
struct greet_info	*greet;
struct verify_info	*verify;
{
	struct passwd	*p;
#ifdef USESHADOW
	struct spwd	*sp;
#endif
#ifdef USE_PAM
	int pam_error;
#endif USE_PAM
#if !defined(SVR4) || !defined(GREET_LIB) /* shared lib decls handle this */
	char		*crypt ();
	char		**systemEnv (), **parseArgs ();
#endif
	char		*shell, *home;
	char		**argv;

	Debug ("Verify %s ...\n", greet->name);
	p = getpwnam (greet->name);
	if (!p || strlen (greet->name) == 0) {
		Debug ("getpwnam() failed.\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
#ifndef USE_PAM
#ifdef USESHADOW
	sp = getspnam(greet->name);
	if (sp != NULL) {
	  /*Debug ("getspnam() failed.  Are you root?\n");
	    bzero(greet->password, strlen(greet->password));
	    return 0;
	  */
	  char* tmp;
	  tmp = p->pw_passwd;
	  p->pw_passwd = sp->sp_pwdp;
	  sp->sp_pwdp = tmp;
	}
	endspent();
#endif /* USESHADOW */
	if (strcmp (crypt (greet->password, p->pw_passwd), p->pw_passwd))
	{
		Debug ("password verify failed\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
#else /* USE_PAM */
       #define PAM_BAIL \
	  if (pam_error != PAM_SUCCESS) { \
          pam_end(pamh, 0); \
	  pamh = NULL; \
	  return 0; \
        }
       pamh = NULL;
       PAM_password = greet->password;
       pam_error = pam_start(KDE_PAM, p->pw_name, &PAM_conversation, &pamh);
       PAM_BAIL;
       pam_error = pam_set_item(pamh, PAM_TTY, d->name);
       PAM_BAIL;
       pam_error = pam_authenticate(pamh, 0);
       PAM_BAIL;
       pam_error = pam_acct_mgmt(pamh, 0);
       /* really should do password changing, but it doesn't fit well */
       PAM_BAIL;
       pam_error = pam_setcred(pamh, 0);
       PAM_BAIL;
       /* setup sessions later.  Since Verify succeded, we don't
          have to worry about closing the pam handle?  It will
          be closed when the session is closed.
       */
#endif /* USE_PAM */

	Debug ("verify succeeded\n");
/*	bzero(greet->password, strlen(greet->password)); */
	verify->uid = p->pw_uid;
#ifdef NGROUPS_MAX
	getGroups (greet->name, verify, p->pw_gid);
#else
	verify->gid = p->pw_gid;
#endif
	home = p->pw_dir;
	shell = p->pw_shell;
	argv = 0;
	if (d->session)
		argv = parseArgs (argv, d->session);
	if (greet->string)
		argv = parseArgs (argv, greet->string);
	if (!argv)
		argv = parseArgs (argv, "xsession");
	verify->argv = argv;
	verify->userEnviron = userEnv (d, p->pw_uid == 0,
				       greet->name, home, shell);
	Debug ("user environment:\n");
	printEnv (verify->userEnviron);
	verify->systemEnviron = systemEnv (d, greet->name, home);
	Debug ("system environment:\n");
	printEnv (verify->systemEnviron);
	Debug ("end of environments\n");
	return 1;
}



