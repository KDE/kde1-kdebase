/*
 * Copyright (c) 1998 Christian Esken <esken@kde.org> 
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
 *      Copyright (C) 1998, Christian Esken <esken@kde.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kcheckpass.h"

#ifdef HAVE_ETCPASSWD

/*******************************************************************
 * This is the authentication code for /etc/passwd passwords
 *******************************************************************/

#include <string.h>

int authenticate(const char *login, const char *passwd)
{
  struct passwd *pw;
  char *crpt_passwd;
  int result;

  /* Get the password entry for the user we want */
  pw = getpwnam(login);

  /* getpwnam should return a NULL pointer on error */
  if (pw == 0)
    return 0;

  /* Encrypt the password the user entered */
  crpt_passwd = crypt(passwd, pw->pw_passwd);

  /* Are they the same? */
  result = strcmp(pw->pw_passwd, crpt_passwd);

  /* Close the password file (or database) */
  endpwent();

  if (result == 0)
    return 1; /* success */
  else
    return 0; /* Oops something went wrong, perhaps no user was found? */
}

#endif
