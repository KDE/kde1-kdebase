#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdlib.h>	// For atoi()

void KMemoryWidget::update()
{
  int mib[2],memory,len;char blah[10];
  /* Stuff for swap display */
  int used, total, free;
  FILE *pipe;
  char buf[80], *used_str, *total_str;
  
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
  /*	Q&D hack for swap display. Borrowed from xsysinfo-1.4  */
  if ((pipe = popen("/usr/sbin/pstat -ks", "r")) == NULL) {
     used = total = 1;
     return;
  }
  fgets(buf, sizeof(buf), pipe);
  fgets(buf, sizeof(buf), pipe);
  fgets(buf, sizeof(buf), pipe);
  fgets(buf, sizeof(buf), pipe);
  strtok(buf, " ");
  total_str = strtok(NULL, " ");
  used_str = strtok(NULL, " ");
  pclose(pipe);
  used = atoi(used_str);
  total = atoi(total_str); 
  free=total-used;
  swapMem->setText(format(1024*used));
  freeSwapMem->setText(format(1024*free));
}
