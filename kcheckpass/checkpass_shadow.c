/*
 * Wed Jan 27 14:25:45 MET 1999
 *
 * This is a modified version of checkpass_shadow.cpp
 *
 * Modifications made by Thorsten Kukuk <kukuk@suse.de>
 *                       Mathias kettner <kettner@suse.de>
 *
 * ------------------------------------------------------------
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
#include "kcheckpass.h"

/*******************************************************************
 * This is the authentication code for Shadow-Passwords
 *******************************************************************/

#ifdef HAVE_SHADOW
#include <string.h>
#include <pwd.h>

#ifndef __hpux
#include <shadow.h>
#endif

int authenticate(const char *login, const char *typed_in_password)
{
  char          *crpt_passwd;
  char          *password;
  int           result;

  struct passwd *pw= getpwnam(login);
  if ( pw == 0 )
    return 2;
  
  if (strcmp(pw->pw_passwd, "x") == 0) /* look up shadow */
    {
      struct spwd *spw = getspnam(login);
      if ( spw == 0 ) 
	return 2;
      else password = spw->sp_pwdp;
    }
  else
    password = pw->pw_passwd;
  
#if defined( __linux__ ) && defined( HAVE_PW_ENCRYPT )
  crpt_passwd = pw_encrypt(typed_in_password, password);  /* (1) */
#else  
  crpt_passwd = crypt(typed_in_password, password);
#endif
  result = strcmp(password, crpt_passwd );
  endspent();
  if (result == 0)
      return 1; /* success */
  else
      return 0;
}

/*
 (1) Deprecated - long passwords have known weaknesses.  Also,
     pw_encrypt is non-standard (requires libshadow.a) while
     everything else you need to support shadow passwords is in
     the standard (ELF) libc.
 */
#endif
