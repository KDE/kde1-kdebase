/***************************************************************************
 *
 * This module is all original work by Robert Nation 
 * (nation@rocket.sanders.lockheed.com) 
 *
 * Copyright 1994, Robert Nation, no rights reserved.
 * The author gaurantees absolutely nothing about anything, anywhere, anytime
 * Use this code at your own risk for anything at all.
 *
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "debug.h"

/* Number of pad characters to use when checking for out-of-bounds */
#define DEBUG_PAD 4
#define MAX_MALLOCS 100

#ifdef DEBUG_MALLOC
static void *all_ptrs[MAX_MALLOCS];
static char *all_names[MAX_MALLOCS];
static int first_try = 1;
#endif
/****************************************************************************
 *
 * Malloc that checks for NULL return, and addes out-of-bounds checking if
 * DEBUG_MALLOC is set 
 *
 ***************************************************************************/
void *safemalloc(int size, const char *identifier)
{
  void *s;
#ifdef DEBUG_MALLOC  
  char *vptr;
  int *l,i;

  if(first_try)
    {
      for(i=0;i<MAX_MALLOCS;i++)
	all_ptrs[i] = 0;
      first_try = 0;
    }


  if ((s = malloc(size+2*DEBUG_PAD*sizeof(int))) == NULL)
    abort();
  vptr = s;
  for(i=0;i<(DEBUG_PAD-1)*sizeof(int);i++)
    vptr[i] = 0x3e;
  for(i=size+(DEBUG_PAD*sizeof(int));i<(size+(2*DEBUG_PAD)*sizeof(int));i++)
    vptr[i] = 0x3f;
  l = (int *)s + DEBUG_PAD-1;
  *l = size;

  fprintf(stderr,"Allocated %d bytes at %x, name %s\n",size,s, identifier);

  i=0;
  while((i<MAX_MALLOCS)&&(all_ptrs[i] != 0))
    i++;
  if(i>=MAX_MALLOCS)
    {
      fprintf(stderr,"MAX_MALLOCS exceeded. Please increase\n");
    }
  else
    {
      all_ptrs[i] = s;
      all_names[i] = identifier;
      check_all_mem("Malloc",identifier);
    }

  return((void *)((int *)s+DEBUG_PAD));
#else
  if ((s = malloc(size)) == NULL)
    abort();
/*  printf("Malloced %d bytes at %x. Id:%s\n",size,s,identifier);*/

  return((void *)s);
#endif
}


/****************************************************************************
 *
 * Free command good for use with above malloc, checks for out-of-bounds
 * before freeing.
 *
 ***************************************************************************/
void safefree(void *ptr, char *id1, char *id2)
{
#ifdef DEBUG_MALLOC
  int i;
  char *base;

  base = (char *)((int *)(ptr) - DEBUG_PAD);
  
  i=0;
  fprintf(stderr,"Freeing memory at %x",base);
  while((i<MAX_MALLOCS)&&(all_ptrs[i] != (void *)(base)))
    i++;
  if(i<MAX_MALLOCS)
    {
      fprintf(stderr," name %s\n",all_names[i]);
      check_all_mem(id1,id2);
      all_ptrs[i] = 0;
    }
  
  free((void *)(base));
#else
  free(ptr);
/*  printf("Freeed %x\n",ptr);*/
#endif
}


/****************************************************************************
 *
 * Checks all allocated memory for out of bounds memory usage.
 *
 ***************************************************************************/
void check_all_mem(char *which1, char *which2)
{
#ifdef DEBUG_MALLOC
  int l,i,j,fail = 0;
  int *lptr;
  char *base; 
  static char *previous_check1 = NULL;
  static char *previous_check2 = NULL;

  if(first_try)
    return;

  for(i=0;i<MAX_MALLOCS;i++)
    if(all_ptrs[i] != 0)
      {
	/* Check each memoryy region */
	base = all_ptrs[i];

	for(j=0;j<((DEBUG_PAD-1)*sizeof(int));j++)
	  if(base[j] != 0x3e)
	    {
	      fprintf(stderr,"Ouch! ptr = %x j = %d %s %s:%s\n",
		      base,j,all_names[i],which1,which2);
	      if(previous_check1 != NULL)
		fprintf(stderr,"Last successful check %s %s\n",previous_check1,
			previous_check2);
	      fail = 1;
	    }

	lptr = (int*) (&base[(DEBUG_PAD-1)*sizeof(int)]);
	l = *lptr;
	for(j=l+DEBUG_PAD*sizeof(int); j<l+(2*DEBUG_PAD)*sizeof(int);j++)
	  if(base[j] != 0x3f)
	    {
	      fprintf(stderr,"Ouch2! ptr = %x k = %d name %s %s:%s\n",
		      base,j,all_names[i],which1,which2);
	      if(previous_check1 != NULL)
		fprintf(stderr,"Last successful check %s %s\n",previous_check1,
			previous_check2);
	      fail = 1;
	    }
      }
  if(!fail)
    {
      previous_check1 = which1;
      previous_check2 = which2;
    }
#endif
}

