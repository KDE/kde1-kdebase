#include <sys/systeminfo.h>


KProcessorWidget::KProcessorWidget(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{
  QString str;
  char buf[256];
  lBox = new QListBox(this);
  lBox->setGeometry(20,20,400,280);
  lBox->setFont(QFont("Courier"));

  sysinfo(SI_ARCHITECTURE, buf, sizeof(buf));
  str = buf;
  lBox->insertItem(str);
}

