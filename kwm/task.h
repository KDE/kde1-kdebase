// ktask 
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


class Ktask : public QDialog{
  Q_OBJECT
public:
  Ktask( QWidget *parent=0, const char *name=0, WFlags f=0);
  bool do_grabbing();
  bool eventFilter( QObject *, QEvent * );
  void prepareToShow(const QStrList* strlist, int active);

public slots:
  void cleanup();

signals:
  void changeToClient(QString label);

protected:
  void    resizeEvent( QResizeEvent * );

private slots:
  void logout();
  void listboxSelect(int index);
  void buttonSelect();
private:
  QPushButton* button;
  QPushButton* button_logout;
  QPushButton* button_cancel;
  QListBox* listbox;
  QFrame *frame;
  QLabel* label;
  void SetPointerGrab(QPoint);
  Client* reactive;
};
