
#if !defined( lint ) && !defined( SABER )
static const char sccsid[] = "@(#)passwd.c	4.02 97/04/01 xlockmore";

#endif

/*-
 * passwd.cpp, part of the KDE kscreensaver
 *
 * changes: 06-Jun-98: updated for PAM support.
 * Code is taken essentially verbatim from xlockmore 4.09. 
 * PAM support in xlockmore is attributed as:
 * 24-Jan-98: Updated PAM support and made it configure-able.
 *            Marc Ewing <marc@redhat.com>  Original PAM support from
 *            25-Jul-96 Michael K. Johnson <johnsonm@redhat.com>
 *---------------------------------------------------------
 * passwd.c - passwd stuff.
 *
 * Copyright (c) 1988-91 by Patrick J. Naughton.
 *
 * Revision History:
 *
 * Changes of David Bagley <bagleyd@bigfoot.com>
 * 25-May-96: When xlock is compiled with shadow passwords it will still
 *            work on non shadowed systems.  Marek Michalkiewicz
 *            <marekm@i17linuxb.ists.pwr.wroc.pl>
 * 25-Feb-96: Lewis Muhlenkamp
 *            Added in ability for any of the root accounts to unlock
 *            screen.  Message now gets sent to syslog if root does the
 *            unlocking.
 * 23-Dec-95: Ron Hitchens <ron@idiom.com> reorganized.
 * 10-Dec-95: More context handling stuff for DCE thanks to
 *            Terje Marthinussen <terjem@cc.uit.no>.
 * 01-Sep-95: DCE code added thanks to Heath A. Kehoe
 *            <hakehoe@icaen.uiowa.edu>.
 * 24-Jun-95: Extracted from xlock.c, encrypted passwords are now fetched
 *            on start-up to ensure correct operation (except Ultrix).
 */

#include <kmsgbox.h>
#include "main.h" // for MODE_PREVIEW
#include "xlockmore.h"

#ifdef HAVE_PAM
#define USE_PAM 1
extern "C" {
#include <security/pam_appl.h>
}
#ifdef KDE_PAM_SERVICE
#define KDE_PAM KDE_PAM_SERVICE
#else  
#define KDE_PAM "xdm"  /* default PAM service called by kscreensaver */
#endif 
#else /* !USE_PAM */
#ifdef HAVE_SHADOW
#define USE_SHADOW 1
#endif
#endif


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif


extern char *ProgramName;
extern Bool allowroot;
extern Bool inroot;
extern Bool inwindow;
extern Bool grabmouse;

#define ROOT "root"

#if !defined( SUNOS_ADJUNCT_PASSWD )
#include <pwd.h>
#endif

#ifdef __bsdi__
#include <sys/param.h>
#if _BSDI_VERSION >= 199608
#define       BSD_AUTH
#endif
#endif

#ifdef        BSD_AUTH
#include <login_cap.h>
static login_cap_t *lc = NULL;
static login_cap_t *rlc = NULL;

#endif

extern int mode;

#if ( HAVE_FCNTL_H && defined( USE_MULTIPLE_ROOT ))
#include <fcntl.h>
#include <errno.h>
#if ( HAVE_SYSLOG_H && defined( USE_SYSLOG ))
#include <syslog.h>
#endif
extern int  errno;

void        get_multiple(struct passwd *);
void        set_multiple();

#define BUFMAX 1024		/* Maximum size of pipe buffer */

/* Linked list to keep track of everyone that's authorized * to unlock the
   screen. */
struct pwln {
	char       *pw_name;
#ifdef        BSD_AUTH
	login_cap_t *pw_lc;
#else
	char       *pw_passwd;
#endif
	struct pwln *next;
};
typedef struct pwln pwlnode;
typedef struct pwln *pwlptr;

pwlptr      pwll, pwllh = (pwlptr) NULL;
extern pwlptr pwllh;

/* Function that creates and initializes a new node that * will be added to
   the linked list. */
pwlptr
new_pwlnode(void)
{
	pwlptr      pwl;

	if ((pwl = (pwlptr) malloc(sizeof (pwlnode))) == 0)
		return ((pwlptr) ENOMEM);

	pwl->pw_name = (char *) NULL;
#ifdef BSD_AUTH
	pwl->pw_lc = NULL;
#else
	pwl->pw_passwd = (char *) NULL;
#endif
	pwl->next = (pwlptr) NULL;

	return (pwl);
}
#endif

#ifdef ultrix
#include <auth.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include<stdarg.h>
#ifdef OSF1_ENH_SEC
#include <sys/security.h>
#include <prot.h>
#endif


#if defined( __linux__ ) && defined( USE_SHADOW ) && defined( HAVE_PW_ENCRYPT )
/* Deprecated - long passwords have known weaknesses.  Also, pw_encrypt is
   non-standard (requires libshadow.a) while everything else you need to
   support shadow passwords is in the standard (ELF) libc.  */
#define crypt pw_encrypt
#endif

#ifdef USE_SHADOW
#ifndef __hpux
#include <shadow.h>
#endif
#endif

#ifdef SUNOS_ADJUNCT_PASSWD
#include <sys/label.h>
#include <sys/audit.h>
#include <pwdadj.h>
#define passwd passwd_adjunct
#define pw_passwd pwa_passwd
#define getpwnam(_s) getpwanam(_s)
#define pw_name pwa_name
#define getpwuid(_s) (((_s)==0)?getpwanam(ROOT):getpwanam(cuserid(NULL)))
#endif /* SUNOS_ADJUNCT_PASSWD */

/* System V Release 4 redefinitions of BSD functions and structures */
#if !defined( SHADOW ) && (defined( SYSV ) || defined( SVR4 ))

#ifdef LESS_THAN_AIX3_2
struct passwd {
	char       *pw_name;
	char       *pw_passwd;
	uid_t       pw_uid;
	gid_t       pw_gid;
	char       *pw_gecos;
	char       *pw_dir;
	char       *pw_shell;
};

#endif /* LESS_THAN_AIX3_2 */

#ifdef HPUX_SECURE_PASSWD
#define passwd s_passwd
#define getpwnam(_s) getspwnam(_s)
#define getpwuid(_u) getspwuid(_u)
#endif /* HPUX_SECURE_PASSWD */

#endif /* defined( SYSV ) || defined( SVR4 ) */

#ifdef HP_PASSWDETC		/* HAVE_SYS_WAIT_H */
#include <sys/wait.h>
#endif /* HP_PASSWDETC */

#ifdef AFS
#include <afs/kauth.h>
#include <afs/kautils.h>
#endif /* AFS */

char        user[PASSLENGTH];

#ifdef USE_PAM
/* used to pass the password to the conversation function */
static char *PAM_password;

/*-
 * PAM conversation function
 * Here we assume (for now, at least) that echo on means login name, and
 * echo off means password.
 */


#ifdef PAM_MESSAGE_NONCONST
typedef struct pam_message pam_message_type;
#else
typedef const struct pam_message pam_message_type;
#endif

static int
PAM_conv(int num_msg,
	 pam_message_type **msg,
	 struct pam_response **resp,
	 void *appdata_ptr)
{
	int         replies = 0;
	struct pam_response *reply = NULL;


#define COPY_STRING(s) (s) ? strdup(s) : NULL


	reply = (struct pam_response *) malloc(sizeof (struct pam_response) *
					       num_msg);

	if (!reply)
		return PAM_CONV_ERR;


	for (replies = 0; replies < num_msg; replies++) {
		switch (msg[replies]->msg_style) {
			case PAM_PROMPT_ECHO_ON:
				reply[replies].resp_retcode = PAM_SUCCESS;
				reply[replies].resp = COPY_STRING(user);
				/* PAM frees resp */
				break;
			case PAM_PROMPT_ECHO_OFF:
				reply[replies].resp_retcode = PAM_SUCCESS;
				reply[replies].resp = COPY_STRING(PAM_password);
				/* PAM frees resp */
				break;
			case PAM_TEXT_INFO:
				/* ignore it... */
				reply[replies].resp_retcode = PAM_SUCCESS;
				reply[replies].resp = NULL;
				break;
			case PAM_ERROR_MSG:
				/* ignore it... */
				reply[replies].resp_retcode = PAM_SUCCESS;
				reply[replies].resp = NULL;
				break;
			default:
				/* Must be an error of some sort... */
				(void) free((void *) reply);
				return PAM_CONV_ERR;
		}
	}
	*resp = reply;
	return PAM_SUCCESS;
}
static struct pam_conv PAM_conversation =
{
	&PAM_conv,
	NULL
};

#endif /* USE_PAM */

#if !defined( ultrix ) && !defined( USE_PAM )
static char userpass[PASSLENGTH];
static char rootpass[PASSLENGTH];

#endif /* !ultrix && !USE_PAM */

#ifdef DCE_PASSWD
static int  usernet, rootnet;
static int  check_dce_net_passwd(char *, char *);

#endif

#if defined( HAVE_KRB4 ) || defined( HAVE_KRB5 )
#ifdef HAVE_KRB4
#include <krb.h>
#else /* HAVE_KRB5 */
#include <krb5.h>
#endif
#include <sys/param.h>
static int  krb_check_password(struct passwd *, char *);

#endif

#if ((!defined( OSF1_ENH_SEC ) &&  !defined( HP_PASSWDETC )))
static struct passwd *
my_passwd_entry(void)
{
	uid_t         uid;
	struct passwd *pw;

#ifdef USE_SHADOW
	struct spwd *spw;

#endif

	uid = getuid();
#ifndef SUNOS_ADJUNCT_PASSWD
	{
		char       *user = NULL;

		pw = 0;
		user = getenv("LOGNAME");
		if (!user)
			user = getenv("USER");
		if (user) {
/* PURIFY 3.2 on Solaris2 reports an uninitialized memory read and PURIFY 4.0
   on SunOS4 reports a file descriptor allocated on the next line */
			pw = getpwnam(user);
/* PURIFY 3.2 on Solaris2 reports an uninitialized memory read on the next
   line PURIFY 4.0 on SunOS4 does not report this error */
			if (pw && (pw->pw_uid != uid))
				pw = 0;
		}
	}
	if (!pw)
#endif
		pw = getpwuid(uid);
#ifdef USE_SHADOW
	if ((spw = getspnam(pw->pw_name)) != NULL) {
		char       *tmp;	/* swap */

		tmp = pw->pw_passwd;
		pw->pw_passwd = spw->sp_pwdp;
		spw->sp_pwdp = tmp;
	}
	endspent();
#endif
	return (pw);
}
#endif

void
error(char *s1,...)
{
        char       *s2;
        va_list     vl;
 
        va_start(vl, s1);
        s2 = va_arg(vl, char *);
 
        va_end(vl);
        (void) fprintf(stderr, s1, ProgramName, s2);
        exit(1);
}

static void
getUserName(void)
{

#ifdef HP_PASSWDETC

/* 
 * The PasswdEtc libraries have replacement passwd functions that make
 * queries to DomainOS registries.  Unfortunately, these functions do
 * wierd things to the process (at minimum, signal handlers get changed,
 * there are probably other things as well) that cause xlock to become
 * unstable.
 *
 * As a (really, really sick) workaround, we'll fork() and do the getpw*()
 * calls in the child, and pass the information back through a pipe.
 */
	struct passwd *pw;
	int         pipefd[2], n, total = 0, stat_loc;
	pid_t       pid;

	pipe(pipefd);

	if ((pid = fork()) == 0) {
		close(pipefd[0]);
		pw = getpwuid(getuid());
		write(pipefd[1], pw->pw_name, strlen(pw->pw_name));
		close(pipefd[1]);
		_exit(0);
	}
	if (pid < 0)
		error("%s: could not get user password (fork failed)\n");
	close(pipefd[1]);

	while ((n = read(pipefd[0], &(user[total]), 50)) > 0)
		total += n;

	wait(&stat_loc);

	if (n < 0)
		error("%s: could not get user name (read failed)\n");
	user[total] = 0;

	if (total < 1)
		error("%s: could not get user name (lookups failed)\n");

#else /* !HP_PASSWDETC */
#ifdef OSF1_ENH_SEC
	struct pr_passwd *pw;

	/*if ((pw = getprpwuid(getuid())) == NULL) */
	if ((pw = getprpwuid(starting_ruid())) == NULL)
		error("%s: could not get user name.\n");
	(void) strcpy(user, pw->ufld.fd_name);

#else /* !OSF1_ENH_SEC */

	struct passwd *pw;

	if (!(pw = my_passwd_entry()))
		/*if ((pw = (struct passwd *) getpwuid(getuid())) == NULL) */
		error("%s: could not get user name.\n");
/* PURIFY 3.2 on Solaris2 reports an uninitialized memory read on the next
   line. PURIFY 4.0 on SunOS4 does not report this error */
	(void) strcpy(user, pw->pw_name);

#if ( HAVE_FCNTL_H && defined( USE_MULTIPLE_ROOT ))
	get_multiple(pw);
#endif

#endif /* !OSF1_ENH_SEC */
#endif /* !HP_PASSWDETC */
}

#if !defined( USE_PAM )
#if defined( USE_SHADOW )
static int
passwd_invalid(char *passwd)
{
	int         i = strlen(passwd);

	return (i == 1 || i == 2);
}
#endif

#if !defined( ultrix ) && !defined( DCE_PASSWD ) && !defined( BSD_AUTH )
/* This routine is not needed if HAVE_FCNTL_H and USE_MULTIPLE_ROOT */
static void
getCryptedUserPasswd(void)
{

#ifdef HP_PASSWDETC

/* 
 * still very sick, see above
 */
	struct passwd *pw;
	int         pipefd[2], n, total = 0, stat_loc;
	pid_t       pid;

	pipe(pipefd);

	if ((pid = fork()) == 0) {
		close(pipefd[0]);
		pw = getpwuid(getuid());
		write(pipefd[1], pw->pw_passwd, strlen(pw->pw_passwd));
		close(pipefd[1]);
		_exit(0);
	}
	if (pid < 0)
		error("%s: could not get user password (fork failed)\n");
	close(pipefd[1]);

	while ((n = read(pipefd[0], &(userpass[total]), 50)) > 0)
		total += n;

	wait(&stat_loc);

	if (n < 0)
		error("%s: could not get user password (read failed)\n");
	user[total] = 0;

	if (total < 1)
		error("%s: could not get user password (lookups failed)\n");

#else /* !HP_PASSWDETC */
#ifdef OSF1_ENH_SEC
	struct pr_passwd *pw;

	/*if ((pw = getprpwuid(getuid())) == NULL) */
	if ((pw = getprpwuid(starting_ruid())) == NULL)
		error("%s: could not get encrypted user password.\n");
	(void) strcpy(userpass, pw->ufld.fd_encrypt);

#else /* !OSF1_ENH_SEC */

	struct passwd *pw;

	if (!(pw = my_passwd_entry()))
		error("%s: could not get encrypted user password.\n");

	// checking of "canGetPasswd" was here once. But it is now
	// done in main.cpp (only for Shadow passwords).
	(void) strcpy(userpass, pw->pw_passwd);

#endif /* !OSF1_ENH_SEC */
#endif /* !HP_PASSWDETC */
}

static void
getCryptedRootPasswd(void)
{

#ifdef HP_PASSWDETC

/* 
 * Still really, really sick.  See above.
 */
	struct passwd *pw;
	int         pipefd[2], n, total = 0, stat_loc;
	pid_t       pid;

	pipe(pipefd);

	if ((pid = fork()) == 0) {
		close(pipefd[0]);
		pw = getpwnam(ROOT);
		write(pipefd[1], pw->pw_passwd, strlen(pw->pw_passwd));
		close(pipefd[1]);
		_exit(0);
	}
	if (pid < 0)
		error("%s: could not get root password (fork failed)\n");
	close(pipefd[1]);

	while ((n = read(pipefd[0], &(rootpass[total]), 50)) > 0)
		total += n;

	wait(&stat_loc);

	if (n < 0)
		error("%s: could not get root password (read failed)\n");
	rootpass[total] = 0;

	if (total < 1)
		error("%s: could not get root password (lookups failed)\n");

#else /* !HP_PASSWDETC */
#ifdef OSF1_ENH_SEC
	struct pr_passwd *pw;

	if ((pw = getprpwnam(ROOT)) == NULL)
		error("%s: could not get encrypted root password.\n");
	(void) strcpy(rootpass, pw->ufld.fd_encrypt);

#else /* !OSF1_ENH_SEC */
	struct passwd *pw;

#ifdef USE_SHADOW
	struct spwd *spw;

#endif

/* PURIFY 3.2 on Solaris2 reports an uninitialized memory read on the next
   line. PURIFY 4.0 on SunOS4 does not report this error */
	if (!(pw = getpwnam(ROOT)))
		if (!(pw = getpwuid(0)))
			/*if ((pw = (struct passwd *) getpwuid(0)) == NULL) */
			error("%s: could not get encrypted root password.\n");
#ifdef USE_SHADOW
	if ((spw = getspnam(pw->pw_name)) != NULL) {
		char       *tmp;	/* swap */

		tmp = pw->pw_passwd;
		pw->pw_passwd = spw->sp_pwdp;
		spw->sp_pwdp = tmp;
	}
	endspent();
#endif
/* PURIFY 3.2 on Solaris2 reports an uninitialized memory read on the next
   line. PURIFY 4.0 on SunOS4 does not report this error */
	(void) strcpy(rootpass, pw->pw_passwd);

#if ( HAVE_FCNTL_H && defined( USE_MULTIPLE_ROOT ))
	set_multiple();
#endif /* HAVE_FCNTL_H && MULTIPLE_ROOT */

#endif /* !OSF1_ENH_SEC */
#endif /* !HP_PASSWDETC */
}

#endif /* !ultrix && !DCE_PASSWD && !BSD_AUTH */
#endif /* !USE_PAM */

/* 
 * we don't allow for root to have no password, but we handle the case
 * where the user has no password correctly; they have to hit return
 * only
 */
int
checkPasswd(char *buffer)
{
	int         done;

#ifdef DCE_PASSWD
	if (usernet)
		done = check_dce_net_passwd(user, buffer);
	else
		done = !strcmp(userpass, crypt(buffer, userpass));

	if (done)
		return 1;
	if (!allowroot)
		return 0;

	if (rootnet)
		done = check_dce_net_passwd(ROOT, buffer);
	else
		done = !strcmp(rootpass, crypt(buffer, rootpass));
#else /* !DCE_PASSWD */

#ifdef USE_PAM

/*-
 * Use PAM to do authentication.  No session logging, only authentication.
 * Bail out if there are any errors.
 * For now, don't try to display strings explaining errors.
 * Later, we could integrate PAM more by posting errors to the
 * user.
 * Query: should we be using PAM_SILENT to shut PAM up?
 */
	pam_handle_t *pamh;
	int         pam_error;

#define PAM_BAIL if (pam_error != PAM_SUCCESS) { \
    pam_end(pamh, 0); return 0; \
}
	PAM_password = buffer;
	pam_error = pam_start(KDE_PAM, user, &PAM_conversation, &pamh);
	PAM_BAIL;
	pam_error = pam_authenticate(pamh, 0);
	if (pam_error != PAM_SUCCESS) {
		/* Try as root; bail if no success there either */
		pam_error = pam_set_item(pamh, PAM_USER, ROOT);
		PAM_BAIL;
		pam_error = pam_authenticate(pamh, 0);
		PAM_BAIL;
	}
	/* Don't do account management or credentials; credentials
	 * aren't needed and account management would just lock up
	 * a computer and require root to come and unlock it.  Blech.
	 */
	pam_end(pamh, PAM_SUCCESS);
	/* If this point is reached, the user has been authenticated. */
	done = True;
#else /* !USE_PAM */


#ifdef ultrix
	done = ((authenticate_user((struct passwd *) getpwnam(user),
				   buffer, NULL) >= 0) || (allowroot &&
			 (authenticate_user((struct passwd *) getpwnam(ROOT),
					    buffer, NULL) >= 0)));
#else /* !ultrix */

#ifdef BSD_AUTH
	char       *pass;
	char       *style;
	char       *name;

	done = 0;
#if ( HAVE_FCNTL_H && defined( USE_MULTIPLE_ROOT ))
	/* Scan through the linked list until you match a password.  Print
	 * message to log if password match doesn't equal the user.
	 *
	 * This should be changed to allow the user name to be typed in also
	 * to make this more secure.
	 */
	for (pwll = pwllh; done == 0 && pwll->next; pwll = pwll->next) {
		name = pwll->pw_name;
		lc = pwll->pw_lc;
#else
	name = user;
#endif
	if ((pass = strchr(buffer, ':')) != NULL) {
		*pass++ = '\0';
		style = login_getstyle(lc, buffer, "auth-xlock");
		if (auth_response(name, lc->lc_class, style,
				  "response", NULL, "", pass) > 0)
			done = 1;
		else if (rlc != NULL) {
			style = login_getstyle(rlc, buffer, "auth-xlock");
			if (auth_response(ROOT, rlc->lc_class, style,
					  "response", NULL, "", pass) > 0)
				done = 1;
		}
		pass[-1] = ':';
	}
	if (done == 0) {
		style = login_getstyle(lc, NULL, "auth-xlock");
		if (auth_response(name, lc->lc_class, style,
				  "response", NULL, "", buffer) > 0)
			done = 1;
		else if (rlc != NULL) {
			style = login_getstyle(rlc, NULL, "auth-xlock");
			if (auth_response(ROOT, rlc->lc_class, style,
					  "response", NULL, "", buffer) > 0)
				done = 1;
		}
	}
#if ( HAVE_FCNTL_H && defined( USE_MULTIPLE_ROOT ))
}
#endif

#else /* !BSD_AUTH */

#ifdef AFS
	char       *reason;

	/* check afs passwd first, then local, then root */
	done = !ka_UserAuthenticate(user, "", 0, buffer, 0, &reason);
	if (!done)
#endif /* !AFS */
#if defined(HAVE_KRB4) || defined(HAVE_KRB5)
		if (!strcmp(userpass, "*"))
			done = (krb_check_password((struct passwd *) getpwuid(getuid()), buffer) ||
				(allowroot && !strcmp((char *) crypt(buffer, rootpass), rootpass)));
		else
#endif /* !HAVE_KRB4 && !HAVE_KRB5 */
#if ( HAVE_FCNTL_H && defined( USE_MULTIPLE_ROOT ))
			/* Scan through the linked list until you match a password.  Print
			 * message to log if password match doesn't equal the user.
			 *
			 * This should be changed to allow the user name to be typed in also
			 * to make this more secure.
			 */
			done = 0;
	for (pwll = pwllh; pwll->next; pwll = pwll->next)
		if (!strcmp((char *) crypt(buffer, pwll->pw_passwd), pwll->pw_passwd)) {
#if ( HAVE_SYSLOG_H && defined( USE_SYSLOG ))
			if (strcmp(user, pwll->pw_name) != 0)
				syslog(LOG_NOTICE, "xlock: %s unlocked screen", pwll->pw_name);
#endif
			done = 1;
			break;
		}
#else
			done = ((!strcmp((char *) crypt(buffer, userpass), userpass)) ||
				(allowroot && !strcmp((char *) crypt(buffer, rootpass), rootpass)));
#endif
	/* userpass is used */
	if (!*userpass && *buffer)
		/*
		 * the user has no password, but something was typed anyway.
		 * sounds fishy: don't let him in...
		 */
		done = False;

#endif /* !BSD_AUTH */
#endif /* !ultrix */
#endif /* !USE_PAM */
#endif /* !DCE_PASSWD */

	return done;
}

/*-
 * Functions for DCE authentication
 *
 * Revision History:
 * 21-Aug-95: Added fallback to static password file [HAK]
 * 06-Jul-95: Mods by Heath A. Kehoe <hakehoe@icaen.uiowa.edu> for
 *            inclusion into xlockmore
 * May-95: Created by Phil O'Connell <philo@icaen.uiowa.edu>
 */

#ifdef DCE_PASSWD
#include <pthread.h>
#include <dce/sec_login.h>

static void
initDCE(void)
{
	sec_login_handle_t login_context;
	error_status_t error_status;
	boolean32   valid;
	struct passwd *pwd;

	/* test to see if this user exists on the network registry */
	valid = sec_login_setup_identity((unsigned_char_p_t) user,
			  sec_login_no_flags, &login_context, &error_status);
	if (!valid) {
		switch (error_status) {
			case sec_rgy_object_not_found:
				break;
			case sec_rgy_server_unavailable:
				(void) fprintf(stderr, "%s: the network registry is not available.\n",
					       ProgramName);
				break;
			case sec_login_s_no_memory:
				error("%s: out of memory\n");
				/*NOTREACHED */
			default:
				(void) fprintf(stderr,
					       "%s: sec_login_setup_identity() returned status %d\n",
					    ProgramName, (int) error_status);
				break;
		}

		pwd = getpwnam(user);
		if (!pwd || strlen(pwd->pw_passwd) < 10) {
			error("%s: could not get user password\n");
			/*NOTREACHED */
		}
		usernet = 0;
		(void) strcpy(userpass, pwd->pw_passwd);
	} else
		usernet = 1;

	if (allowroot) {
		valid = sec_login_setup_identity((unsigned_char_p_t) ROOT,
			  sec_login_no_flags, &login_context, &error_status);
		if (!valid) {
			switch (error_status) {
				case sec_rgy_object_not_found:
					break;
				case sec_rgy_server_unavailable:
					(void) fprintf(stderr, "%s: the network registry is not available.\n",
						       ProgramName);
					break;
				case sec_login_s_no_memory:
					error("%s: out of memory\n");
					/*NOTREACHED */
				default:
					(void) fprintf(stderr,
						       "%s: sec_login_setup_identity() returned status %d\n",
					    ProgramName, (int) error_status);
					break;
			}

			pwd = getpwuid(0);
			if (!pwd || strlen(pwd->pw_passwd) < 10) {
				(void) fprintf(stderr,
					       "%s: could not get root password\n", ProgramName);
				allowroot = 0;
			}
			rootnet = 0;
			(void) strcpy(rootpass, pwd->pw_passwd);
		} else
			rootnet = 1;
	}
	pthread_lock_global_np();
}

static char *
error_string(error_status_t error_status)
{
	static char buf[60];

	switch (error_status) {
		case error_status_ok:
			return "no error";
		case sec_rgy_object_not_found:
			return "The principal does not exist";
		case sec_rgy_server_unavailable:
			return "The network registry is not available";
		case sec_login_s_no_memory:
			return "Not enough memory is available to complete the operation";
		case sec_login_s_already_valid:
			return "The login context has already been validated";
		case sec_login_s_default_use:
			return "Can't validate the default context";
		case sec_login_s_acct_invalid:
			return "The account is invalid or has expired";
		case sec_login_s_unsupp_passwd_type:
			return "The password type is not supported";
		case sec_login_s_context_invalid:
			return "The login context itself is not valid";
		default:
			(void) sprintf(buf, "error status #%d", (int) error_status);
			return buf;
	}
}


/* 
 *----------------------------------------------------------------------
 *     Function Created 5/95 to be used with xlock to validate DCE
 *     passwords.  Routine simply returns a (1) if the the variable
 *     PASS is the USER's PASSWORD, else it returns a (0).
 *     Functions used:
 *
 *               sec_login_setup_identity
 *               sec_login_validate_identity
 *               sec_login_certify_identity
 *
 *     where setup_identity obtains the login context for the USER.
 *     This identity is then validated with validate_identity.  Finally,
 *     cerfify_identity is called to make sure that the Security
 *       Server used to set up and validate a login context is legitimate.
 *
 *       Created by Phil O'Connell
 *     philo@icaen.uiowa.edu
 *     Student Programmer
 *
 *-----------------------------------------------------------------------
 */

static int
check_dce_net_passwd(char *usr, char *pass)
{
	sec_login_handle_t login_context;
	error_status_t error_status;
	sec_passwd_rec_t password;
	boolean32   reset_password;
	sec_login_auth_src_t auth_src;
	unsigned_char_p_t principal_name;
	boolean32   valid = 0;
	char       *passcpy;
	boolean32   current_context;


	pthread_unlock_global_np();
	/* -------------------- SETUP IDENTITY--------------------------------- */

	principal_name = (unsigned_char_p_t) usr;

	/*
	 * We would rather like to refresh and existing login context instead of
	 * making a new one.
	 */

	sec_login_get_current_context(&login_context, &error_status);
	if (error_status != error_status_ok) {
		current_context = 0;
		(void) fprintf(stderr,
		       "get_current_context failed! Setting up a new one\n");

		valid = sec_login_setup_identity(principal_name, sec_login_no_flags,
					      &login_context, &error_status);

		if (!valid) {
			(void) fprintf(stderr, "sec_login_setup_identity() failed: %s\n",
				       error_string(error_status));
			pthread_lock_global_np();
			return 0;
		}
	} else
		current_context = 1;

/*--------------- VALIDATE IDENTITY ---------------------------------*/

	/* make a copy of pass, because sec_login_validate_identity() will
	   clobber the plaintext password passed to it */
	passcpy = strdup(pass);

	password.key.key_type = sec_passwd_plain;
	password.key.tagged_union.plain = (unsigned char *) passcpy;
	password.pepper = NULL;
	password.version_number = sec_passwd_c_version_none;

	valid = sec_login_validate_identity(login_context, &password, &reset_password, &auth_src, &error_status);

	/* report unusual error conditions */
	if (error_status != error_status_ok &&
	    error_status != sec_rgy_passwd_invalid &&
	    error_status != sec_login_s_already_valid &&
	    error_status != sec_login_s_null_password) {
		(void) fprintf(stderr, "sec_login_validate_identity failed: %s\n",
			       error_string(error_status));
	}
	/* done with the copy of the password */
	(void) free(passcpy);

	/* Refresh the context if we already have one */
	if (current_context) {
		if (!sec_login_refresh_identity(login_context, &error_status)) {
			(void) fprintf(stderr, "sec_login_refresh_identity failed: %s\n",
				       error_string(error_status));
		} else {
			passcpy = strdup(pass);

			password.key.key_type = sec_passwd_plain;
			password.key.tagged_union.plain = (unsigned char *) passcpy;
			password.pepper = NULL;
			password.version_number = sec_passwd_c_version_none;

			/* Have to validate the refreshed context */
			valid = sec_login_validate_identity(login_context,
			&password, &reset_password, &auth_src, &error_status);

			/* report unusual error conditions */
			if (error_status != error_status_ok &&
			    error_status != sec_rgy_passwd_invalid &&
			    error_status != sec_login_s_null_password) {
				(void) fprintf(stderr, "sec_login_validate_identity failed: %s\n",
					       error_string(error_status));
			}
		}
		/* done with the copy of the password */
		(void) free(passcpy);
	}
	/* make sure that the authentication service is not an imposter */
	if (valid) {
		if (!sec_login_certify_identity(login_context, &error_status)) {
			(void) fprintf(stderr, "Authentication service is an imposter!\n");
			/* logoutUser(); */
			valid = 0;
		}
	}
	pthread_lock_global_np();
	return valid;
}

#endif /* DCE_PASSWD */

#ifdef HAVE_KRB4
int
krb_check_password(struct passwd *pwd, char *pass)
{
	char        realm[REALM_SZ];
	char        tkfile[MAXPATHLEN];

	/* find local realm */
	if (krb_get_lrealm(realm, 1) != KSUCCESS)
		/*(void) strncpy(realm, KRB_REALM, sizeof (realm)); */
		(void) strncpy(realm, krb_get_default_realm(), sizeof (realm));

	/* Construct a ticket file */
	(void) sprintf(tkfile, _PATH_TMP"/xlock_tkt_%d", getpid());

	/* Now, let's make the ticket file named above the _active_ tkt file */
	krb_set_tkt_string(tkfile);

	/* ask the kerberos server for a ticket! */
	if (krb_get_pw_in_tkt(pwd->pw_name, "", realm,
			      "krbtgt", realm,
			      DEFAULT_TKT_LIFE,
			      pass) == INTK_OK) {
		unlink(tkfile);
		return 1;
	}
	return 0;
}
#endif /* HAVE_KERB4 */

#ifdef HAVE_KRB5
/*-
 * Pretty much all of this was snatched out of the kinit from the Kerberos5b6
 * distribution.  The reason why I felt it was necessary to use kinit was
 * that if someone is locked for a long time, their credentials could expire,
 * so xlock must be able to get a new ticket.  -- dah <rodmur@ecst.csuchico.edu>
 */
#define KRB5_DEFAULT_OPTIONS 0
#define KRB5_DEFAULT_LIFE 60*60*10	/* 10 hours */
static int
krb_check_password(struct passwd *pwd, char *pass)
{
	krb5_context kcontext;
	krb5_timestamp now;
	krb5_ccache ccache = NULL;
	krb5_principal me;
	krb5_principal server;
	int         options = KRB5_DEFAULT_OPTIONS;
	krb5_deltat lifetime = KRB5_DEFAULT_LIFE;
	krb5_error_code code;
	krb5_creds  my_creds;
	char       *client_name;
	krb5_address **addrs = (krb5_address **) 0;
	krb5_preauthtype *preauth = NULL;

	krb5_data   tgtname =
	{
		0,
		KRB5_TGS_NAME_SIZE,
		KRB5_TGS_NAME
	};

	krb5_init_context(&kcontext);
	krb5_init_ets(kcontext);

	if ((code = krb5_timeofday(kcontext, &now))) {
		com_err(ProgramName, code, "while getting time of day");
		return 0;	/* seems better to deny access, than just exit, which
				   was what happened in kinit  */
	}
	if ((code = krb5_cc_default(kcontext, &ccache))) {
		com_err(ProgramName, code, "while getting default ccache");
		return 0;
	}
	code = krb5_cc_get_principal(kcontext, ccache, &me);
	if (code) {
		if ((code = krb5_parse_name(kcontext, pwd->pw_name, &me))) {
			com_err(ProgramName, code, "when parsing name %s", pwd->pw_name);
			return 0;
		}
	}
	if ((code = krb5_unparse_name(kcontext, me, &client_name))) {
		com_err(ProgramName, code, "when unparsing name");
		return 0;
	}
	(void) memset((char *) &my_creds, 0, sizeof (my_creds));

	my_creds.client = me;

	if ((code = krb5_build_principal_ext(kcontext, &server,
				      krb5_princ_realm(kcontext, me)->length,
					krb5_princ_realm(kcontext, me)->data,
					     tgtname.length, tgtname.data,
				      krb5_princ_realm(kcontext, me)->length,
					krb5_princ_realm(kcontext, me)->data,
					     0))) {
		com_err(ProgramName, code, "while building server name");
		return 0;
	}
	my_creds.server = server;

	my_creds.times.starttime = 0;
	my_creds.times.endtime = now + lifetime;
	my_creds.times.renew_till = 0;

	if (strlen(pass) == 0)
		strcpy(pass, "*");
	/* if pass is NULL, krb5_get_in_tkt_with_password will prompt with
	   krb5_default_pwd_prompt1 for password, you don't want that in
	   this application, most likely the user won't have '*' for a
	   password -- dah <rodmur@ecst.csuchico.edu> */

	code = krb5_get_in_tkt_with_password(kcontext, options, addrs,
					     NULL, preauth, pass, 0,
					     &my_creds, 0);

	(void) memset(pass, 0, sizeof (pass));

	if (code) {
		if (code == KRB5KRB_AP_ERR_BAD_INTEGRITY)
			return 0;	/* bad password entered */
		else {
			com_err(ProgramName, code, "while getting initial credentials");
			return 0;
		}
	}
	code = krb5_cc_initialize(kcontext, ccache, me);
	if (code != 0) {
		com_err(ProgramName, code, "when initializing cache");
		return 0;
	}
	code = krb5_cc_store_cred(kcontext, ccache, &my_creds);
	if (code) {
		com_err(ProgramName, code, "while storing credentials");
		return 0;
	}
	krb5_free_principal(kcontext, server);

	krb5_free_context(kcontext);

	return 1;		/* success */
}
#endif /* HAVE_KRB5 */

#undef passwd
#undef pw_name
#undef pw_passwd
#ifndef SUNOS_ADJUNCT_PASSWD
#include <pwd.h>
#endif

#if ( HAVE_FCNTL_H && defined( USE_MULTIPLE_ROOT ))

void
get_multiple(struct passwd *pw)
{
	/* This should be the first element on the linked list.
	 * If not, then there could be problems.
	 * Also all memory allocations tend to force an exit of
	 * the program.  This should probably be changed somehow.
	 */
	if (pwllh == (pwlptr) NULL) {
		if ((pwll = new_pwlnode()) == (pwlptr) ENOMEM) {
			perror("new");
			exit(1);
		}
		pwllh = pwll;
	}
	if ((pwll->pw_name = strdup(pw->pw_name)) == NULL) {
		perror("new");
		exit(1);
	}
#ifdef        BSD_AUTH
	pwll->pw_lc = login_getclass(pw->pw_class);
#else
	if ((pwll->pw_passwd = strdup(pw->pw_passwd)) == NULL) {
		perror("new");
		exit(1);
	}
#endif
	if ((pwll->next = new_pwlnode()) == (pwlptr) ENOMEM) {
		perror("new");
		exit(1);
	}
}

void
set_multiple(void)
{
#ifdef BSD_AUTH
	struct passwd *pw;
	pwlptr      pwll;

	if (pwllh == (pwlptr) NULL) {
		if ((pwll = new_pwlnode()) == (pwlptr) ENOMEM) {
			perror("new");
			exit(1);
		}
		pwllh = pwll;
	}
	for (pwll = pwllh; pwll->next; pwll = pwll->next);

	while ((pw = getpwent()) != (struct passwd *) NULL) {
		if (pw->pw_uid)
			continue;
		if ((pwll->pw_name = strdup(pw->pw_name)) == NULL) {
			perror("new");
			exit(1);
		}
		pwll->pw_lc = login_getclass(pw->pw_class);

		if ((pwll->next = new_pwlnode()) == (pwlptr) ENOMEM) {
			perror("new");
			exit(1);
		}
	}

	if (pwll->next = new_pwlnode())
		pwll = pwll->next;
#else /* !BSD_AUTH */
	/* If you thought the above was sick, then you'll think this is
	 * downright horrific.  This is set up so that a child process
	 * is created to read in the password entries using getpwent(3C).
	 * In the man pages on the HPs, getpwent(3C) has in it the fact
	 * that once getpwent(3C) has opened the password file, it keeps
	 * it open until the process is finished.  Thus, the child
	 * process exits immediately after reading the entire password
	 * file.  Otherwise, the password file remains open the entire
	 * time this program is running.
	 *
	 * I went with getpwent(3C) because it will actually read in
	 * the password entries from the NIS maps as well.
	 */
	struct passwd *pw;
	int         pipefd[2];
	char        buf[BUFMAX], xlen;
	pid_t       cid;

#ifdef USE_SHADOW
	struct spwd *spw;

#endif

	if (pipe(pipefd) < 0) {
		perror("Pipe Generation");
		exit(1);
	}
	if ((cid = fork()) < 0) {
		perror("fork");
		exit(1);
	} else if (cid == 0) {
		/* child process.  Used to read in password file.  Also checks to
		 * see if the uid is zero.  If so, then writes that to the pipe.
		 */
		register int sbuf = 0;
		char       *cbuf, *pbuf;

		close(pipefd[0]);
		while ((pw = getpwent()) != (struct passwd *) NULL) {
			if (pw->pw_uid != 0)
				continue;
#ifdef USE_SHADOW
			if ((spw = getspnam(pw->pw_name)) != NULL) {
				char       *tmp;	/* swap */

				tmp = pw->pw_passwd;
				pw->pw_passwd = spw->sp_pwdp;
				spw->sp_pwdp = tmp;
			}
#endif
			if (pw->pw_passwd[0] != '*') {
				xlen = strlen(pw->pw_name);
				if ((sbuf + xlen) >= BUFMAX) {
					if (write(pipefd[1], buf, sbuf) != sbuf)
						perror("write");
					sbuf = 0;
				}
				cbuf = &buf[sbuf];
				*cbuf++ = xlen;
				for (pbuf = pw->pw_name; *pbuf;)
					*cbuf++ = *pbuf++;
				sbuf += xlen + 1;

				xlen = strlen(pw->pw_passwd);
				if ((sbuf + xlen) >= BUFMAX) {
					if (write(pipefd[1], buf, sbuf) != sbuf)
						perror("write");
					sbuf = 0;
				}
				cbuf = &buf[sbuf];
				*cbuf++ = xlen;
				for (pbuf = pw->pw_passwd; *pbuf;)
					*cbuf++ = *pbuf++;
				sbuf += xlen + 1;
			}
		}
#ifdef USE_SHADOW
		endspent();
#endif
		cbuf = &buf[sbuf];
		*cbuf = -1;
		sbuf++;
		if (write(pipefd[1], buf, sbuf) != sbuf)
			perror("write");

		close(pipefd[1]);
		exit(0);
	} else {
		/* parent process.  Does the actual creation of the linked list.
		 * It assumes that everything coming through the pipe are password
		 * entries that are authorized to unlock the screen.
		 */
		register int bufsize = BUFMAX, done = 0, sbuf = BUFMAX,
		            i;
		char       *cbuf, *pbuf;
		pwlptr      pwll;

		close(pipefd[1]);

		if (pwllh == (pwlptr) NULL) {
			if ((pwll = new_pwlnode()) == (pwlptr) ENOMEM) {
				perror("new");
				exit(1);
			}
			pwllh = pwll;
		}
		for (pwll = pwllh; pwll->next; pwll = pwll->next);
		while (!done) {
			if (sbuf >= bufsize) {
				if ((bufsize = read(pipefd[0], buf, BUFMAX)) <= 0)
					perror("read");
				sbuf = 0;
			}
			cbuf = &buf[sbuf];
			xlen = *cbuf++;
			if (xlen < 0) {
				done = 1;
				break;
			}
			sbuf++;
			if (sbuf >= bufsize) {
				if ((bufsize = read(pipefd[0], buf, BUFMAX)) <= 0)
					perror("read");
				sbuf = 0;
			}
			if ((pwll->pw_name = (char *) malloc(xlen + 1)) == NULL)
				break;
			pbuf = pwll->pw_name;
			cbuf = &buf[sbuf];
			for (i = 0; i < xlen; i++) {
				*pbuf++ = *cbuf++;
				sbuf++;
				if (sbuf >= bufsize) {
					if ((bufsize = read(pipefd[0], buf, BUFMAX)) <= 0)
						perror("read");
					sbuf = 0;
					cbuf = buf;
				}
			}
			*pbuf = (char) NULL;

			cbuf = &buf[sbuf];
			xlen = *cbuf++;
			sbuf++;
			if (sbuf >= bufsize) {
				if ((bufsize = read(pipefd[0], buf, BUFMAX)) <= 0)
					perror("read");
				sbuf = 0;
			}
			if ((pwll->pw_passwd = (char *) malloc(xlen + 1)) == NULL)
				break;
			pbuf = pwll->pw_passwd;
			cbuf = &buf[sbuf];
			for (i = 0; i < xlen; i++) {
				*pbuf++ = *cbuf++;
				sbuf++;
				if (sbuf >= bufsize) {
					if ((bufsize = read(pipefd[0], buf, BUFMAX)) <= 0)
						perror("read");
					sbuf = 0;
					cbuf = buf;
				}
			}
			*pbuf = (char) NULL;

			if ((pwll->next = new_pwlnode()) == (pwlptr) ENOMEM)
				break;
			pwll = pwll->next;
		}
		close(pipefd[0]);
	}
#endif /* !BSD_AUTH */
}
#endif

void
initPasswd(void)
{
        getUserName();
#if !defined( ultrix ) && !defined( DCE_PASSWD ) && !defined( USE_PAM)
	if ( mode != MODE_PREVIEW ) {
#ifdef        BSD_AUTH
                struct passwd *pwd = getpwnam(user);
 
                lc = login_getclass(pwd->pw_class);
                if (allowroot && (pwd = getpwnam(ROOT)) != NULL)
                        rlc = login_getclass(pwd->pw_class);
#else /* !BSD_AUTH */
                getCryptedUserPasswd();
                if (allowroot)
		    getCryptedRootPasswd();
#endif /* !BSD_AUTH */
        }
#endif /* !ultrix && !DCE_PASSWD && !USE_PAM*/
#ifdef DCE_PASSWD
        initDCE();
#endif

}


