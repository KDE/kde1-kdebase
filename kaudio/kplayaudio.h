// -*- C++ -*-

#ifndef DEMOCLIENT_H
#define DEMOCLIENT_H

#include "kapp.h"
#include "ktopwidget.h"

class SimpleAudioClient : public QObject
{
 Q_OBJECT

public:
  SimpleAudioClient();
  ~SimpleAudioClient();
  void play(const char *fname);
  KAudio   KAServer;

public slots:
  void quit();

};

#endif
