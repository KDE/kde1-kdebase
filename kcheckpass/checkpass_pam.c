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
 *      Copyright (C) 1998, Caldera, Inc.
 */
#include "kcheckpass.h"
#ifdef HAVE_PAM

/*****************************************************************
 * This is the authentication code if you use PAM
 * Ugly, but proven to work.
 *****************************************************************/
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>

#ifdef KDE_PAM_SERVICE
#define KDE_PAM KDE_PAM_SERVICE
#else
#ifdef __FreeBSD__
#define KDE_PAM "login"  /* default PAM service used by kcheckpass */
#else
#define KDE_PAM "xdm"  /* default PAM service used by kcheckpass */
#endif
#endif

static const char *PAM_username;
static const char *PAM_password;

#ifdef PAM_MESSAGE_NONCONST
typedef struct pam_message pam_message_type;
#else
typedef const struct pam_message pam_message_type;
#endif

static int
PAM_conv (int num_msg, pam_message_type **msg,
	  struct pam_response **resp,
	  void *appdata_ptr)
{
  int             count = 0, replies = 0;
  struct pam_response *repl = NULL;
  int             size = sizeof(struct pam_response);

#define GET_MEM \
	if (!(repl = (realloc(repl, size)))) \
  		return PAM_CONV_ERR; \
	size += sizeof(struct pam_response)
#define COPY_STRING(s) (s) ? strdup(s) : NULL

  for (count = 0; count < num_msg; count++) {
    switch (msg[count]->msg_style) {
    case PAM_PROMPT_ECHO_ON:
      GET_MEM;
      repl[replies].resp_retcode = PAM_SUCCESS;
      repl[replies++].resp = COPY_STRING(PAM_username);
      /* PAM frees resp */
      break;
    case PAM_PROMPT_ECHO_OFF:
      GET_MEM;
      repl[replies].resp_retcode = PAM_SUCCESS;
      repl[replies++].resp = COPY_STRING(PAM_password);
      /* PAM frees resp */
      break;
    case PAM_TEXT_INFO:
      message("unexpected message from PAM: %s\n",
	      msg[count]->msg);
      break;
    case PAM_ERROR_MSG:
    default:
      /* Must be an error of some sort... */
      message("unexpected error from PAM: %s\n",
	     msg[count]->msg);
      free(repl);
      return PAM_CONV_ERR;
    }
  }
  if (repl)
    *resp = repl;
  return PAM_SUCCESS;
}

static struct pam_conv PAM_conversation = {
  &PAM_conv,
  NULL
};


int authenticate(const char *login, const char *passwd)
{
  pam_handle_t	*pamh;
  int		pam_error;

  PAM_username = login;
  PAM_password = passwd;

  pam_error = pam_start(KDE_PAM, login, &PAM_conversation, &pamh);
  if (pam_error != PAM_SUCCESS
      || (pam_error = pam_authenticate(pamh, 0)) != PAM_SUCCESS) {
    pam_end(pamh, pam_error);
    return 0;
  }

  pam_end(pamh, PAM_SUCCESS);
  return 1;
}

#endif
