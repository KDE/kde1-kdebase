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

#ifndef SBAR_H
#define SBAR_H

struct sbar_info 
{
  Window sb_win;
  Window sb_up_win;
  Window sb_down_win;
  GC sbgc;
  GC sbupgc;
  GC sbdowngc;
  int width;
  int height;       /* height excluding the arrows */
};


void sbar_init(void);
void sbar_show(int,int,int);

#endif /* SBAR_H */
