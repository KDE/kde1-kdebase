#include <qtabbar.h>

#include <kapp.h>
#include "processor.h"
#include "processor.moc"


// Include system-specific code

#ifdef linux
#include "processor_linux.cpp"
#elif sgi
#include "processor_sgi.cpp"
#elif __FreeBSD__
#include "processor_fbsd.cpp"
#else

// Default for unsupportet systems

KProcessorWidget::KProcessorWidget(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{
  lBox = new QListBox(this);
  lBox->setGeometry(20,20,400,280);
  lBox->setFont(QFont("Courier"));

  lBox->insertItem(klocale->translate("This system is not yet supported :-("));
}


#endif
