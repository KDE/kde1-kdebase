#include <sys/types.h>
#include <sys/sysctl.h>

void KMemoryWidget::update()
{
  int mib[2],memory,len;char blah[10];
  
  mib[0]=CTL_HW;mib[1]=HW_PHYSMEM;
  len=sizeof(memory);
  sysctl(mib,2,&memory,&len,NULL,0);
  
  snprintf(blah,10,"%d",memory);
  // Numerical values
  totalMem->setText(format(memory));
  /*	To: questions@freebsd.org
		Anyone have any ideas on how to calculate this */
  freeMem->setText("Unknown");
  sharedMem->setText(("Unknown"));
  bufferMem->setText(("Unknown"));
  /*	To count swap space, you'd need to -lkvm kcc, and make it suid root,
		let's not do that */
  swapMem->setText(("Unknown"));
  freeSwapMem->setText(("Unknown"));
}
