// minicli
// Copyright (C) 1997 Matthias Ettrich
//
// Torben added command completion
// 09.11.97
//
// Dawit added short URL support
// 02.26.99

#include <qlined.h>
#include <qlabel.h>
#include <kURLcompletion.h>

void execute ( const char* );
bool isExecutable ( const char* );
bool isValidShortURL ( const char * );

class Minicli : public QFrame{
  Q_OBJECT
public:
  Minicli( QWidget *parent=0, const char *name=0, WFlags f=0);
  bool do_grabbing();
  bool eventFilter( QObject *, QEvent * );
  void cleanup();

protected:
  void    resizeEvent( QResizeEvent * );

private slots:
  void return_pressed();

private:
  QLineEdit* lineedit;
  QLabel* label;
  Client* reactive;
  void commandCompletion();
  unsigned int max_hist; //CT 15Jan1999 limit the in-memory history too

  KURLCompletion kurlcompletion;
};

