// klogout 
// Copyright (C) 1997 Matthias Ettrich

#include <qmsgbox.h>
#include <qlined.h>
#include <qlabel.h>
#include <qframe.h>
#include <qdialog.h>
#include <qpushbt.h>
#include <qlistbox.h>
#include <qstring.h>

#include <qapp.h>
#include <qwindefs.h>
#include <X11/Xlib.h>
#include <stdlib.h>

#include "client.h"


class KWarning : public QDialog{
  Q_OBJECT
public:
  KWarning( QWidget *parent=0, const char *name=0, WFlags f=0);
  bool do_grabbing();
  void setText(const char* text, bool with_button);
  bool eventFilter( QObject *, QEvent * );
public slots:
  void ok();
  void release();
private:
  QFrame *frame;
  QPushButton* button;
  QLabel* label;
  void SetPointerGrab(QPoint);
  Client* reactive;
};
