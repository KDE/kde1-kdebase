#ifndef _PROCESSOR_
#define _PROCESSOR_


#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qtabdlg.h>
#include <qpushbt.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <qfile.h>
#include <qevent.h>
#include <kmsgbox.h>
#include "kcontrol.h"


class KProcessorWidget : public KConfigWidget
{
  Q_OBJECT

public:

  KProcessorWidget(QWidget *parent, const char *name=0);

  void applySettings() {};
  void loadSettings() {};
  
private:
  QListBox *lBox;

};


#endif
