#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h> /* DF : for strcasecmp */
#include <string.h>
#include <ctype.h> /* CC: for isspace */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/cursorfont.h>
#include "rxvt.h"
#include "command.h"
#include "xsetup.h"
#include "screen.h"
#include "debug.h"

/* Stephan: a little bit hardcoded :) */
#if !defined PRINT_PIPE
#define PRINT_PIPE
#endif

extern Display		*display;
extern char		*xvt_name;	/* the name the program is run under */
extern char *fg_string_tmp, *bg_string_tmp;
extern char *print_pipe, *geom_string; 
extern char *reg_fonts[NUM_FONTS], *window_name, *icon_name;
extern unsigned char mask;
extern int login_shell,fat_sbar, MetaHandling;
extern WindowInfo MyWinInfo;
extern KeySym SecureKeysym;
extern KeySym BigFontKeysym;
extern KeySym SmallFontKeysym;
extern KeySym PageUpKeysym;
extern KeySym PageDownKeysym;
#ifdef MAPALERT
extern int map_alert;
#endif
#ifdef GREEK_KBD
extern KeySym GreekSwitchKeysym; 
#endif

struct res_list
{
  char **dataptr;
  char *keyword;
};


char *mask_string = NULL, *login_shell_string = NULL, *scrollbar = NULL;
char* SecureKeysym_string=NULL;
char *BigFontKeysym_string = NULL, *SmallFontKeysym_string = NULL;
char *PageUpKeysym_string = NULL, *PageDownKeysym_string = NULL;
#ifdef GREEK_KBD
char *elot_string = NULL;
#endif
#ifdef MAPALERT
char *Map_alert_string=NULL;
#endif
char *saveLines_string = NULL, *meta_string = NULL;

struct res_list  string_resource_list[] =
{
#ifdef PRINT_PIPE
  {&print_pipe,                 "print-pipe"},
#endif
  {&geom_string,                "geometry"},
  {&fg_string_tmp,                  "foreground"},
  {&bg_string_tmp,                  "background"},
  {&reg_fonts[DEFAULT_FONT],    "font"},
  {&window_name,                "title"},
  {&icon_name,                  "icon_name"},
  {&mask_string,                "bits"},
  {&login_shell_string,         "login_shell"},
  {&scrollbar,                  "scrollbar"},
  {&saveLines_string,           "saveLines"},
  {&meta_string,                "meta"},
  {&SecureKeysym_string,        "secure_key"},
  {&BigFontKeysym_string,       "bigger_font_key"},
  {&SmallFontKeysym_string,     "smaller_font_key"},
  {&PageUpKeysym_string,        "page_up_key"},
  {&PageDownKeysym_string,      "page_down_key"},
#ifdef MAPALERT
  {&Map_alert_string,		"mapalert"},
#endif
#ifdef GREEK_KBD
  {&elot_string,                 "greek_switch_keysym"},
#endif
  {0,                           NULL}
};

void GetThoseResourceStrings(void);

/* read the resources files */
void extract_resources(void)
{
  GetThoseResourceStrings();

  if(mask_string)
    {
      if(mask_string[0]=='7')
	mask = 0x7f;
      else
	mask = 0xff;
    }
#ifdef MAPALERT
  if(Map_alert_string)
    {
      if(strcasecmp(Map_alert_string,"true")==0)
	map_alert = 1;
      else
	map_alert = 0;
    }
#endif
  if(login_shell_string)
    {
      if(strcasecmp(login_shell_string,"true")==0)
	login_shell = 1;
      else
	login_shell = 0;
    }
  if(scrollbar)
    {
      if(strcasecmp(scrollbar,"fat")==0)
	fat_sbar = 1;
      else
	fat_sbar = 0;
    }

  if(saveLines_string)
    {
      sscanf(saveLines_string,"%d\n",&MyWinInfo.saved_lines);
      if(MyWinInfo.saved_lines < 0 )
	MyWinInfo.saved_lines = 0;
    }
  else
    MyWinInfo.saved_lines = DEF_SAVED_LINES;

  if(SecureKeysym_string)
    SecureKeysym = XStringToKeysym(SecureKeysym_string);
  if(BigFontKeysym_string)
    BigFontKeysym = XStringToKeysym(BigFontKeysym_string);
  if(SmallFontKeysym_string)
    SmallFontKeysym = XStringToKeysym(SmallFontKeysym_string);
  if(PageUpKeysym_string)
    PageUpKeysym = XStringToKeysym(PageUpKeysym_string);
  if(PageDownKeysym_string)
    PageDownKeysym = XStringToKeysym(PageDownKeysym_string);
#ifdef GREEK_KBD
  if(elot_string)
    GreekSwitchKeysym = XStringToKeysym(elot_string);
#endif

  if(meta_string)
    {
      if(strcasecmp(meta_string,"escape")==0)
	MetaHandling = ESCAPE;
      else if(strcasecmp(meta_string,"8thbit")==0)
	MetaHandling = BIT;
      else
	MetaHandling = 0;
    }

  return;
}

#ifdef REAL_RESOURCES
/***************************************************************************
 *
 * Get resources using the X library function "XGetDefault"
 *
 **************************************************************************/
void GetThoseResourceStrings(void)
{
  char *tmp;
  int i=0;
  
  while(string_resource_list[i].keyword != NULL)
    {
      if((tmp = XGetDefault(display,xvt_name,
			    string_resource_list[i].keyword)) != (char *)0)
	*string_resource_list[i].dataptr = tmp;
      i++;
    }
}
#else /* Real_resources */
#ifdef FAKE_RESOURCES
/***************************************************************************
 *
 * Get resources the hard way, but save lots of memory
 *
 **************************************************************************/
void GetThoseResourceStrings(void)
{
  char *home_dir = NULL;
  char *file;
  char *fname= ".Xdefaults";
  FILE *fd;
  char text[255];
  int xvt_len,i,n,m;

  home_dir = getenv("HOME");
  if(home_dir == NULL)
    home_dir = "./";

  file = safemalloc(strlen(home_dir)+strlen(fname)+2,"resources");
  strcpy(file,home_dir);
  strcat(file,"/");
  strcat(file,fname);
  
  fd = fopen(file,"r");
  if(fd == NULL)
    {
/*     fprintf(stderr,"%s: couldn't open Xdefaults file %s\n",xvt_name,file);*/
      free(file);
      return;
    }
  free(file);

  xvt_len = strlen(xvt_name);

  /* check for an exact match */
  while(fgets(text,254,fd))
    {
      if((strncmp(text,xvt_name,xvt_len)==0)&&
	 ((text[xvt_len] == '.')||(text[xvt_len] == '*')))
	{
	  i = 0;
	  while(string_resource_list[i].keyword != NULL)
	    {
	      if(strncmp(&text[xvt_len+1],string_resource_list[i].keyword,
			 strlen(string_resource_list[i].keyword))==0)
		{
		  /* skip the ":" and leading spaces */
		  n = xvt_len;
		  while((text[n] != 0)&&(text[n] != ':')&&(n<254))
		    n++;
		  n++;
		  while((text[n] != 0)&&(isspace(text[n]))&&(n<254))
		    n++;
		  if(strlen(&text[n]) > 0)
		    {
		      *string_resource_list[i].dataptr = 
			safemalloc(strlen(&text[n])+1,"resource2");
		      strcpy(*string_resource_list[i].dataptr,&text[n]);
		      n = strlen(*string_resource_list[i].dataptr)-1;
		      while((n>0)&&
			    (isspace((*(string_resource_list[i].dataptr))[n])))
			n--;
		      (*string_resource_list[i].dataptr)[n+1] = 0;
		    }
		}
	      i++;
	    }
	}
    }

  /* check for partial matches */
  rewind(fd);
  while(fgets(text,254,fd))
    {
      if(text[0] == '*')
	{
	  m = 1;
	  while(((text[m] == '*')||(text[m] =='.'))&&(text[m] > 0)&&(m<255))
	    m++;
	  i = 0;
	  while(string_resource_list[i].keyword != NULL)
	    {
	      if(strncmp(&text[m],string_resource_list[i].keyword,
			 strlen(string_resource_list[i].keyword))==0)
		{
		  /* skip the ":" and leading spaces */
		  n = m;
		  while((text[n] != 0)&&(text[n] != ':'))
		    n++;
		  n++;
		  while((text[n] != 0)&&(isspace(text[n])))
		    n++;
		  if(strlen(&text[n]) > 0)
		    {
		      *string_resource_list[i].dataptr = 
			safemalloc(strlen(&text[n])+1,"resource2");
		      strcpy(*string_resource_list[i].dataptr,&text[n]);
		      n = strlen(*string_resource_list[i].dataptr)-1;
		      while((n>0)&&
			    (isspace((*(string_resource_list[i].dataptr))[n])))
			n--;
		      (*string_resource_list[i].dataptr)[n+1] = 0;
		    }
		}
	      i++;
	    }
	}
    }
}

#else  /* fake_resources */

/***************************************************************************
 *
 * Blow off the resources completely
 *
 **************************************************************************/
void GetThoseResourceStrings(void)
{
}

#endif /* fake_resources */

#endif /* real_resources */
