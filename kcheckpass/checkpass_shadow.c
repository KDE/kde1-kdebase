/*
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
#ifdef HAVE_SHADOW


/*******************************************************************
 * This is the authentication code for Shadow-Passwords
 *******************************************************************/

#include <string.h>

#ifndef __hpux
#include <shadow.h>
#endif

int authenticate(const char *login, const char *passwd)
{
  struct spwd *pw;
  char* crpt_passwd;
  int result;

  pw = getspnam(login);
  if ( pw == 0 ) {
    endspent();
    return 0;
  }
#if defined( __linux__ ) && defined( HAVE_PW_ENCRYPT )
  crpt_passwd = pw_encrypt(passwd, pw->sp_pwdp);  // (1)
#else  
  crpt_passwd = crypt(passwd, pw->sp_pwdp);
#endif
  result = strcmp(pw->sp_pwdp, crpt_passwd );
  endspent();
  if (result == 0)
    return 1; // success
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
