/*
 * This file is under GPL 
 *
 * Demonstration on using the KAudio class. Playing digital audio
 * made easy. :-)
 *
 * Questions, comments? Mail to esken@kde.org
 */

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <mediatool.h>
}

#include <kapp.h>

#include "kaudio.h"
#include "kplayaudio.h"
#include "kplayaudio.moc"

using namespace std;

char **global_argv;
KApplication *globalKapp;

void usage();
void usage()
{
  cout << i18n( "Usage" ) << ": " << global_argv[0] << i18n("filename") << "\n" ;
}


int main ( int argc, char** argv)
{
  global_argv = argv;
  globalKapp = new KApplication(argc, argv);

  /************* initialization ************************************/
  SimpleAudioClient *myClient = new SimpleAudioClient();
  if ( argc > 1 ) {
    myClient->play(argv[1]);
  }
  else {
    usage();
    return 0;
  }

  if (myClient->KAServer.serverStatus()) {
    cerr << i18n("Failed contacting audio server\n");
    return 1;
  }
  return globalKapp->exec();
}





SimpleAudioClient::SimpleAudioClient() : QObject()
{
  KAServer.setSignals(true);
  QObject::connect(&KAServer, SIGNAL(playFinished()), this, SLOT(quit()));
}

SimpleAudioClient::~SimpleAudioClient() {}


void SimpleAudioClient::quit()
{
  KAServer.stop();
  exit(0);
}

void SimpleAudioClient::play(const char *fname)
{
  KAServer.play(fname);
}
