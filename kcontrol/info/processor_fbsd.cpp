#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysctl.h>

/* stdio.h has NULL, but also a lot of extra cruft */
#ifndef NULL
#define NULL 0L
#endif

/* Again avoid the cruft in stdlib.h since malloc() isn't gonna change 
   too often */
void    *malloc __P((size_t));

KProcessorWidget::KProcessorWidget(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{
  QString str;

  /* Stuff for sysctl */
  char *buf,*mhz,*cpustring;
  int mib[2],machspeed;
  size_t len;
  /* */

  cpustring=(char *)malloc(128);
  lBox = new QListBox(this);
  lBox->setGeometry(20,20,400,280);
  lBox->setFont(QFont("Courier"));

  mib[0] = CTL_HW;
  mib[1] = HW_MODEL;
  sysctl(mib,2,NULL,&len,NULL,0);
  buf=(char*)malloc(len);
  sysctl(mib,2,buf,&len,NULL,0);
  /*	Get the CPU speed, heh, heh, undocumented sysctls rule
		but I dunno if this works on 2.2 machines.	*/
  mib[0] = CTL_MACHDEP; mib[1] = 107;
  len=sizeof(machspeed);
  sysctl(mib,2,&machspeed,&len,NULL,0);
  /* Format the integer into correct xxx.xx MHz */
  mhz=(char *)malloc(20);
  snprintf(mhz,20,"%d.%02d",(machspeed+4999)/1000000,
		   ((machspeed+4999)/10000)%100);
  if (strcmp(mhz,"0.0")==0)
 	/* We dunno how fast it's running */
 	snprintf(cpustring,128,"%s, unknown speed",buf);
  else
	snprintf(cpustring,128,"%s running at %s MHz",buf,mhz);
  /* Put everything in the listbox */
  lBox->insertItem(buf);
  /* Clean up after ourselves */
  free(mhz); free(cpustring);
}

