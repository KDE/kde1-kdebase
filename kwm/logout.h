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


class Klogout : public QDialog{
  Q_OBJECT
public:
  Klogout( QWidget *parent=0, const char *name=0, WFlags f=0);
  bool do_grabbing();
  bool eventFilter( QObject *, QEvent * );
  void prepareToShow(const QStrList* strlist1, 
		     const QStrList* strlist2,
		     const QStrList* strlist3);

public slots:
  void cleanup();

signals:
  void changeToClient(QString label);
  void doLogout();

protected:
  void    resizeEvent( QResizeEvent * );

private slots:
  void logout();
  void listbox1Select(int index);
  void listbox2Select(int index);
  void listbox3Select(int index);

private:

  QPushButton* button;
  QPushButton* button_cancel;
  QLabel* listboxlabel1;
  QListBox* listbox1;
  QLabel* listboxlabel2;
  QListBox* listbox2;
  QLabel* listboxlabel3;
  QListBox* listbox3;
  QFrame *frame;
  QLabel* label;
  void SetPointerGrab(QPoint);
  Client* reactive;
};
