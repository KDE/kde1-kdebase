#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdlib.h>	// For atoi()

void KMemoryWidget::update()
{
  int mib[2],memory;size_t len;char blah[10];
  /* Stuff for swap display */
  int used, total, _free;
  FILE *pipe;
  char buf[80];
  
  mib[0]=CTL_HW;mib[1]=HW_PHYSMEM;
  len=sizeof(memory);
  sysctl(mib,2,&memory,&len,NULL,0);
  
  snprintf(blah,10,"%d",memory);
  // Numerical values
  totalMem->setText(format(memory));
  /*	To: questions@freebsd.org
		Anyone have any ideas on how to calculate this */
  freeMem->setText(i18n("Not available"));
  sharedMem->setText(i18n("Not available"));
  bufferMem->setText(i18n("Not available"));
  swapMem->setText(i18n("Not available"));
  freeSwapMem->setText(i18n("Not available"));
}
