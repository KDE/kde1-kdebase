#ifndef _MEMORY_
#define _MEMORY_


#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qtabdlg.h>
#include <qpushbt.h>
#include <qtimer.h>


#include "kcontrol.h"


class KMemoryWidget : public KConfigWidget
{
  Q_OBJECT

public:

  KMemoryWidget(QWidget *parent, const char *name=0);

  void applySettings() {};
  void loadSettings() {};

private:

  QLabel *freeMem, *totalMem, *sharedMem, *bufferMem, *swapMem, *freeSwapMem;
  QTimer *timer;
  
public slots:

  void update();
};


#endif
