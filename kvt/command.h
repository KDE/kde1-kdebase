/*  Copyright 1992 John Bovey, University of Kent at Canterbury.
 *
 *  You can do what you like with this source code as long as
 *  you don't try to make money out of it and you include an
 *  unaltered copy of this message (including the copyright).
 *
 * This module has been heavily modifiedby R. Nation
 * (nation@rocket.sanders.lockheed.com).
 * No additional restrictions are applied
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 */

/* LICENSE changed: all authors including but not limited to John Bovey
 * aggreed to distribute their code under the terms of the GPL
 */

/* @(#)command.h	1.1 14/7/92 (UKC) */

#define ESCAPE 1
#define BIT 2

void tty_set_size(int,int,int);
void init_command(unsigned char *,unsigned char **);
void send_string(unsigned char *,int);
void cprintf(unsigned char *,...);
/* get to handle.   Matthias */ 
void handle_token(unsigned char);
void process_robs_sequence(void);
