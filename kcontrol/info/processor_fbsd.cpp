#include <sys/types.h>
#include <sys/sysctl.h>

/* stdio.h has NULL, but also a lot of extra cruft */
#ifndef NULL
#define NULL 0L
#endif

KProcessorWidget::KProcessorWidget(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{
  QString str;

  /* Stuff for sysctl */
  char *buf;
  int mib[2];
  size_t len;
  /* */

  lBox = new QListBox(this);
  lBox->setGeometry(20,20,400,280);
  lBox->setFont(QFont("Courier"));

  mib[0] = CTL_HW;
  mib[1] = HW_MODEL;
  sysctl(mib,2,NULL,&len,NULL,0);
  buf=malloc(len);
  sysctl(mib,2,buf,&len,NULL,0);
  lBox->insertItem(buf);
}

