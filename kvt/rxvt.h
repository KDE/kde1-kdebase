/*  Copyright 1992 John Bovey, University of Kent at Canterbury.
 *
 *  You can do what you like with this source code as long as
 *  you don't try to make money out of it and you include an
 *  unaltered copy of this message (including the copyright).
 */

#define MARGIN 2	/* gap between the text and the window edges */

/*  Some wired in defaults so we can run without any external resources.
 */
#define DEF_SAVED_LINES 128	/* # of saved lines that scrolled of the top */

/* arguments to set and reset functions.
 */
#define LOW	0
#define HIGH	1

unsigned char *cstrcpy(unsigned char *source);
extern void clean_exit(int);
extern void cleanutent(void);
extern void makeutent(char *);
