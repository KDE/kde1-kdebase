#include "kintlist.h"

int KIntList::compareItems( GCI item1, GCI item2 )
{
  int* item1x = (int*)item1;
  int* item2x = (int*)item2;
  if( *item1x == *item2x )
    return 0;
  else if( *item1x < *item2x )
         return(-1);  
       else
         return(1);
}

