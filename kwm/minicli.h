// minicli
// Copyright (C) 1997 Matthias Ettrich

#include <qlined.h>
#include <qlabel.h>


void execute(const char* cmd);

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
};

