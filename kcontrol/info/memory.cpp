#include <qtabbar.h>
#include <kapp.h>
#include <kcharsets.h>
#include "memory.h"
#include "memory.moc"

#define STARTX  20
#define STARTX2 200
#define STARTY  20
#define DY      24

KMemoryWidget::KMemoryWidget(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{
  totalMem = new QLabel(i18n("Total memory"), this);
  totalMem->move(STARTX,STARTY+0*DY);
  totalMem->setAutoResize(TRUE);
  totalMem = new QLabel("0k", this);
  totalMem->move(STARTX2,STARTY+0*DY);
  totalMem->setAutoResize(TRUE);
  QFont courierFont(kapp->fixedFont);
  KApplication::getKApplication()->getCharsets()->setQFont(courierFont);
  totalMem->setFont(courierFont);

  freeMem = new QLabel(i18n("Free memory"), this);
  freeMem->move(STARTX,STARTY+1*DY);
  freeMem->setAutoResize(TRUE);
  freeMem = new QLabel("0k", this);
  freeMem->move(STARTX2,STARTY+1*DY);
  freeMem->setAutoResize(TRUE);
  freeMem->setFont(courierFont);

  sharedMem = new QLabel(i18n("Shared memory"), this);
  sharedMem->move(STARTX,STARTY+2*DY);
  sharedMem->setAutoResize(TRUE);
  sharedMem = new QLabel("0k", this);
  sharedMem->move(STARTX2,STARTY+2*DY);
  sharedMem->setAutoResize(TRUE);
  sharedMem->setFont(courierFont);

  bufferMem = new QLabel(i18n("Buffer memory"), this);
  bufferMem->move(STARTX,STARTY+3*DY);
  bufferMem->setAutoResize(TRUE);
  bufferMem = new QLabel("0k", this);
  bufferMem->move(STARTX2,STARTY+3*DY);
  bufferMem->setAutoResize(TRUE);
  bufferMem->setFont(courierFont);

  swapMem = new QLabel(i18n("Swap memory"), this);
  swapMem->move(STARTX,STARTY+5*DY);
  swapMem->setAutoResize(TRUE);
  swapMem = new QLabel("0k", this);
  swapMem->move(STARTX2,STARTY+5*DY);
  swapMem->setAutoResize(TRUE);
  swapMem->setFont(courierFont);

  freeSwapMem = new QLabel(i18n("Free swap memory"), this);
  freeSwapMem->move(STARTX,STARTY+6*DY);
  freeSwapMem->setAutoResize(TRUE);
  freeSwapMem = new QLabel("0k", this);
  freeSwapMem->move(STARTX2,STARTY+6*DY);
  freeSwapMem->setAutoResize(TRUE);
  freeSwapMem->setFont(courierFont);

  timer = new QTimer(this);
  timer->start(100);
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update()));

  update();
}


QString format(unsigned long value)
{
  QString  text;
  double   mb = value / 1048576.0;
  
  text.sprintf(i18n("%10ld bytes  = %8.2f MB"), value, mb);
  return text;
}


// Include system-specific code

#ifdef linux
#include "memory_linux.cpp"
#elif sgi
#include "memory_sgi.cpp"
#elif __FreeBSD__
#include "memory_fbsd.cpp"
#elif hpux
#include "memory_hpux.cpp"
#else

// Default for unsupported systems

void KMemoryWidget::update()
{
  // Numerical values
  totalMem->setText(i18n("Not available"));
  freeMem->setText(i18n("Not available"));
  sharedMem->setText(i18n("Not available"));
  bufferMem->setText(i18n("Not available"));
  swapMem->setText(i18n("Not available"));
  freeSwapMem->setText(i18n("Not available"));
}

#endif

