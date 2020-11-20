    /*

    $Id: kdm-config.h,v 1.11.4.1 1999/03/09 16:49:34 steffen Exp $

    Copyright (C) 1997, 1998 Steffen Hansen
                             stefh@mip.ou.dk


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */
 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* xdm stuff, should always defined */
#define UNIXCONN          
#define TCPCONN           
#define GREET_USER_STATIC 

#ifdef XDMBINDIR
# define BINDIR XDMBINDIR
#endif

#define DEF_XDM_CONFIG XDMDIR "/xdm-config"
#define DEF_AUTH_DIR XDMDIR "/authdir"

/* If this isn't defined, we crash boxes with S3 cards.
 * See genauth.c 
 */
#define FRAGILE_DEV_MEM 1

/* Authorization stuff */
/* How do we check for HASXDMAUTH? Use Imake ?? */
#if defined(HAVE_KRB5_KRB5_H)
#endif
#if defined(HAVE_RPC_RPC_H) && defined(HAVE_RPC_KEY_PROT_H)
# define SECURE_RPC 1
#endif

/* Too many systems have trouble with secure rpc,
   so its disabled for now:
*/
#undef SECURE_RPC

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifdef HAVE_PAM
# define USE_PAM 1
#else
# ifdef HAVE_SHADOW
#  define USESHADOW 1
# endif
#endif

#ifdef HAVE_SYSLOG_H
 #define USE_SYSLOG
#endif

#ifndef _PATH_VARRUN
#define _PATH_VARRUN "/var/run/"
#endif

#ifndef _PATH_MEM
#define _PATH_MEM "/dev/mem"
#endif

#ifdef sun
#define SVR4 1
#endif

#ifdef __cplusplus
extern "C" {
#endif
//int XdmcpAllocARRAY8();
int Debug( char*, ...);
int LogError( char*, ...);
int LogOutOfMem( char*, ...);
#ifdef __cplusplus
}
#endif
