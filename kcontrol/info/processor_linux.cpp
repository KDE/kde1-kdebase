#include <syscall.h>
#include <linux/kernel.h>
#include <kapp.h>

KProcessorWidget::KProcessorWidget(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{
  QString str;
  char buf[256];
  lBox = new QListBox(this);
  lBox->setGeometry(20,20,400,280);
  lBox->setFont(QFont("Courier"));

  QFile *file = new QFile("/proc/cpuinfo");

  if(!file->open(IO_ReadOnly)) {
    KMsgBox::message(0,klocale->translate("File Error!"),
		     klocale->translate("Cannot read /proc/cpuinfo"));
    delete lBox;
    delete file; 
    return;
  }
  while(file->readLine(buf,100) > 0) {
      str.sprintf("%-16s", strtok(buf, ":"));
      str = str + QString(":") + QString(strtok(NULL, "\0"));
      lBox->insertItem(str);
  }
  file->close();
  delete file;
}
