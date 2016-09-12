/*
 *
 * $Id: checkpass_osfc2passwd.c,v 1.1.2.1 1999/07/08 03:02:19 garbanzo Exp $
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kcheckpass.h"

#ifdef HAVE_OSF_C2_PASSWD

/*******************************************************************
 * This is the authentication code for OSF C2 security passwords
 *******************************************************************/

#include <string.h>

int authenticate(const char *login, const char *passwd)
{
  struct passwd *pw;
  char c2passwd[256];
  int result;

  /* Get the password entry for the user we want */
  pw = getpwnam(login);

  /* getpwnam should return a NULL pointer on error */
  if (pw == 0)
    return 0;

  /* Are they the same? */
  osf1c2_getprpwent(c2passwd, login, sizeof(c2passwd));
  if (strcmp(c2passwd, osf1c2crypt(passwd, c2passwd)) != 0) {
    return 0;
  }

  return 1; /* success */
}


/*
The following code was lifted from the file osfc2.c from the ssh 1.2.26
distribution.  Parts of the code that were not needed by kcheckpass
(notably the osf1c2_check_account_and_terminal() function and the code
to set the external variable days_before_password_expires have been
removed).  The original copyright from the osfc2.c file is included
below.
*/

/*

osfc2.c

Author: Christophe Wolfhugel

Copyright (c) 1995 Christophe Wolfhugel

Free use of this file is permitted for any purpose as long as
this copyright is preserved in the header.

This program implements the use of the OSF/1 C2 security extensions
within ssh. See the file COPYING for full licensing informations.

*/

#include <sys/security.h>
#include <prot.h>
#include <sia.h>

static int     c2security = -1;
static int     crypt_algo;

void
initialize_osf_security(int ac, char **av)
{
  FILE *f;
  char buf[256];
  char siad[] = "siad_ses_init=";
  int i;

  if (access(SIAIGOODFILE, F_OK) == -1)
    {
      /* Broken OSF/1 system, better don't run on it. */
      fprintf(stderr, "%s does not exist. Your OSF/1 system is probably broken\n",
             SIAIGOODFILE);
      exit(1);
    }
  if ((f = fopen(MATRIX_CONF, "r")) == NULL)
    {
      /* Another way OSF/1 is probably broken. */
      fprintf(stderr, "%s unreadable. Your OSF/1 system is probably broken.\n"

             MATRIX_CONF);
      exit(1);
    }

  /* Read matrix.conf to check if we run C2 or not */
  while (fgets(buf, sizeof(buf), f) != NULL)
    {
      if (strncmp(buf, siad, sizeof(siad) - 1) == 0)
       {
         if (strstr(buf, "OSFC2") != NULL)
           c2security = 1;
         else if (strstr(buf, "BSD") != NULL)
           c2security = 0;
         break;
       }
    }
  fclose(f);
  if (c2security == -1)
    {
      fprintf(stderr, "C2 security initialization failed : could not determine security level.\n");
      exit(1);
    }
  if (c2security == 1)
    set_auth_parameters(ac, av);
}


int
osf1c2_getprpwent(char *p, char *n, int len)
{
  time_t pschg, tnow;

  if (c2security == 1)
    {
      struct es_passwd *es;
      struct pr_passwd *pr = getprpwnam(n);
      if (pr)
       {
         strncpy(p, pr->ufld.fd_encrypt, len);
         crypt_algo = pr->ufld.fd_oldcrypt;

         tnow = time(NULL);
         if (pr->uflg.fg_schange == 1)
           pschg = pr->ufld.fd_schange;
         else
           pschg = 0;
         if (pr->uflg.fg_template == 0)
           {
             /** default template, system values **/
             if (pr->sflg.fg_lifetime == 1)
               if (pr->sfld.fd_lifetime > 0 &&
                   pschg + pr->sfld.fd_lifetime < tnow)
                 return 1;
           }
         else                      /** user template, specific values **/
           {
             es = getespwnam(pr->ufld.fd_template);
             if (es)
               {
                 if (es->uflg->fg_expire == 1)
                   if (es->ufld->fd_expire > 0 &&
                       pschg + es->ufld->fd_expire < tnow)
                     return 1;
               }
           }
       }
    }
  else
    {
      struct passwd *pw = getpwnam(n);
      if (pw)
       strncpy(p, pw->pw_passwd, len);
    }
  return 0;
}

char *
osf1c2crypt(const char *pw, char *salt)
{
   if (c2security == 1) {
     return(dispcrypt(pw, salt, crypt_algo));
   } else
     return(crypt(pw, salt));
}

#endif
