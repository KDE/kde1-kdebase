/*  Copyright 1992 John Bovey, University of Kent at Canterbury.
*
*  You can do what you like with this source code as long as
*  you don't try to make money out of it and you include an
*  unaltered copy of this message (including the copyright).
*/
/*
* This module has been heavily modified by R. Nation
* (nation@rocket.sanders.lockheed.com).
* No additional restrictions are applied
*
* As usual, the author accepts no responsibility for anything, nor does
* he guarantee anything whatsoever.
*/
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "command.h"
#include "xsetup.h"
#include "screen.h"
#include "sbar.h"
#include "rxvt.h"
#include "debug.h"


/* I do need the next three from init_disply. Matthias */ 
extern char		*xvt_name;	/* the name the program is run under */
extern char		*window_name;	/* window name for titles etc. */
extern char		*icon_name;	/* name to display in the icon */


/*  The default command argv if we just run a shell.
 */
/* no longer needed */ 
/* static char *shell_argv[2] =  */
/*     { */
/*       "/bin/sh", */
/*       NULL, */
/*     }; */

 /* char *command = (char *)0; */

/*  Run the command in a subprocess and return a file descriptor for the
 *  master end of the pseudo-teletype pair with the command talking to
 *  the slave.
 */

/* renamed and changed similar to rxvt-2.18. Matthias */ 
/* added dynamic term entry (Matthias) */
void rxvt_main(int argc,char **argv)
{
  int i;
  char **com_argv = NULL;
  char* s;
  int len;
  char* t = "xterm-color";
  char* put;
  
  for (i = 0; i < argc; i++){
      if (strcmp(argv[i],"-tn") == 0){
	  i++;
	  if (i<argc)
	      t = argv[i];
      }
      else if (strcmp(argv[i],"-e") == 0)
	  break;
  }
  if (i < argc - 1) 
    {
      argv[i] = NULL;
      com_argv = argv + i + 1;
      argc = i;
    } 
  

  len = strlen(t);
  put= safemalloc(len+10,"display_string");
  sprintf(put,"TERM=%s",t);
  putenv(put);


  if ((s=strrchr(argv[0],'/'))!=NULL) 
    s++; 
  else 
     s=argv[0]; 
  xvt_name = window_name = icon_name = s; 
  
  if (com_argv){
    if ((s=strrchr(com_argv[0],'/'))!=NULL) 
      s++; 
    else 
      s=com_argv[0]; 
    window_name = icon_name = s; 
  }

  init_display(argc,argv);

  init_command(NULL ,(unsigned char **)com_argv);
  /* no longer needed. Matthias */ 
/*   get_token(); */
}

