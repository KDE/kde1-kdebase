#include <qtabbar.h>
#include <kapp.h>
#include "memory.h"
#include "memory.moc"


KMemoryWidget::KMemoryWidget(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{
  totalMem = new QLabel(klocale->translate("Total memory"), this);
  totalMem->move(8,8);
  totalMem->setAutoResize(TRUE);
  totalMem = new QLabel("0k", this);
  totalMem->move(200,8);
  totalMem->setAutoResize(TRUE);
  totalMem->setFont(QFont("Courier"));

  freeMem = new QLabel(klocale->translate("Free memory"), this);
  freeMem->move(8,32);
  freeMem->setAutoResize(TRUE);
  freeMem = new QLabel("0k", this);
  freeMem->move(200,32);
  freeMem->setAutoResize(TRUE);
  freeMem->setFont(QFont("Courier"));

  sharedMem = new QLabel(klocale->translate("Shared memory"), this);
  sharedMem->move(8,56);
  sharedMem->setAutoResize(TRUE);
  sharedMem = new QLabel("0k", this);
  sharedMem->move(200,56);
  sharedMem->setAutoResize(TRUE);
  sharedMem->setFont(QFont("Courier"));

  bufferMem = new QLabel(klocale->translate("Buffer memory"), this);
  bufferMem->move(8,80);
  bufferMem->setAutoResize(TRUE);
  bufferMem = new QLabel("0k", this);
  bufferMem->move(200,80);
  bufferMem->setAutoResize(TRUE);
  bufferMem->setFont(QFont("Courier"));

  swapMem = new QLabel(klocale->translate("Swap memory"), this);
  swapMem->move(8,112);
  swapMem->setAutoResize(TRUE);
  swapMem = new QLabel("0k", this);
  swapMem->move(200,112);
  swapMem->setAutoResize(TRUE);
  swapMem->setFont(QFont("Courier"));

  freeSwapMem = new QLabel(klocale->translate("Free swap memory"), this);
  freeSwapMem->move(8,136);
  freeSwapMem->setAutoResize(TRUE);
  freeSwapMem = new QLabel("0k", this);
  freeSwapMem->move(200,136);
  freeSwapMem->setAutoResize(TRUE);
  freeSwapMem->setFont(QFont("Courier"));

  timer = new QTimer(this);
  timer->start(100);
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update()));

  update();
}


QString format(unsigned long value)
{
  QString  text;
  double   mb = value / 1048576.0;
  
  text.sprintf(klocale->translate("%10ld bytes  = %8.2f MB"), value, mb);
  return text;
}


// Include system-specific code

#ifdef linux
#include "memory_linux.cpp"
#elif sgi
#include "memory_sgi.cpp"
#elif __FreeBSD__
#include "memory_fbsd.cpp"
#else

// Default for unsupported systems

void KMemoryWidget::update()
{
  // Numerical values
  totalMem->setText(klocale->translate("not available"));
  freeMem->setText(klocale->translate("not available"));
  sharedMem->setText(klocale->translate("not available"));
  bufferMem->setText(klocale->translate("not available"));
  swapMem->setText(klocale->translate("not available"));
  freeSwapMem->setText(klocale->translate("not available"));
}

#endif

