// -*- C++ -*-

#ifndef DEMOCLIENT_H
#define DEMOCLIENT_H

#include <qpushbt.h>
#include "kapp.h"
#include "ktopwidget.h"

class DemoClient : public KTopLevelWidget
{
 Q_OBJECT

public:
  DemoClient();
  KAudio   KAServer;

public slots:
  void playOK();
  void stopClicked();
  void replayClicked();
  void signalsOnOff();
  void quit();
  void loadClicked();

private:
  void		createWidgets();
  void		createMenu();
  QPushButton*	createButton( int x, int y, int w, int h, const char *name, const char *TT );
  unsigned int	number;
  bool		replayPossible;
  QString	WAVname;
  bool		QtSignals;
  KStatusBar	*statbar;
  QWidget	*Container;
  QCheckBox	*signalsCB;
};

#endif
