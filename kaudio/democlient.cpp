/*
 * This file is under GPL 
 *
 * Demonstration on using the KAudio class. Playing digital audio
 * made easy. :-)
 *
 * Questions, comments? Mail to esken@kde.org
 */

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <mediatool.h>
}

#include <qchkbox.h>
#include <qtooltip.h>
#include <qkeycode.h>
#include <qfiledlg.h>
#include <kmenubar.h>
#include <kfiledialog.h>

#include "kaudio.h"
#include "democlient.h"
#include "democlient.moc"



KApplication *globalKapp;

int main ( int argc, char** argv)
{
  globalKapp = new KApplication( argc, argv, "Demo-Audioclient" );

  /************* initialization ************************************/
  DemoClient *myClient = new DemoClient();
  if (myClient->KAServer.serverStatus()) {
    cerr << "Failed contacting audio server\n";
    exit (1);
  }

  return globalKapp->exec();
}

void DemoClient::signalsOnOff()
{
  // This part switches Qt signals on and off
  QtSignals = signalsCB->isChecked();
  KAServer.setSignals(QtSignals);
  if (QtSignals)
    statbar->changeItem("Qt Signals: On" ,1);
  else
    statbar->changeItem("Qt Signals: Off",1);

}

void DemoClient::loadClicked()
{
  QString fname = KFileDialog::getOpenFileName( 0, "*.wav", this );
  if ( !fname.isEmpty() ) {
    // the user selected a valid existing file
    WAVname = fname.copy();
    fname += " selected";
    statbar->changeItem(fname ,2);
    replayPossible=false;
  }
  else {
    // the user cancelled the dialog
  }
}
void DemoClient::stopClicked()
{
  KAServer.stop();   // <-- Stop Wav
  statbar->changeItem("Stop pressed" ,2);
}

void DemoClient::quit()
{
  KAServer.stop();
  exit(0);
}

void DemoClient::replayClicked()
{
  if (replayPossible) {
    KAServer.play(); // <-- Restart last Wav
  }
  else {
    char *MyString;
    MyString = WAVname.data();
    KAServer.play(MyString);
    replayPossible = true;
  }
}


DemoClient::DemoClient() : KTopLevelWidget()
{
  replayPossible=false;
  QtSignals=false;
  KAServer.setSignals(QtSignals);
  number = 0;
  createWidgets();

  QObject::connect(&KAServer, SIGNAL(playFinished()), this, SLOT(playOK()));
}

void DemoClient::playOK()
{
  number++;
  char myString[100];
  sprintf(myString,"%u", number);
  QString s = "Finished. ";
  s += myString;
  s += " items signalled";
  statbar->changeItem(s ,2);
}

void DemoClient::createWidgets()
{
  int WIDTH=80; int HEIGHT=40;
  createMenu();
  // Setup/Create the status bar
  statbar = new KStatusBar(this);
  statbar->insertItem( "Qt Signals: Off", 1 );
  statbar->insertItem( "", 2 );
  setStatusBar(statbar);

  // Create a big container containing every widget of this toplevel
  Container  = new QWidget(this);
  int ix=0,iy=0;

  QPushButton *loadPB =
    createButton( ix, iy, WIDTH, HEIGHT, "Load", "Load medium" );
  connect( loadPB, SIGNAL(clicked()), SLOT(loadClicked()) );
  ix += WIDTH;

  QPushButton *replayPB =
    createButton( ix, iy, WIDTH, HEIGHT, "Replay", "Replay medium" );
  connect( replayPB, SIGNAL(clicked()), SLOT(replayClicked()) );
  ix += WIDTH;

  QPushButton *stopPB =
    createButton( ix, iy, WIDTH, HEIGHT, "Stop", "Stop medium" );
  connect( stopPB, SIGNAL(clicked()), SLOT(stopClicked()) );
  ix += WIDTH;

  signalsCB =
    new QCheckBox("Signals:",Container,"PBsignalsOnOff");
  connect(signalsCB, SIGNAL(clicked()), SLOT(signalsOnOff()) );
  signalsCB->setGeometry( ix, iy, WIDTH, HEIGHT );
  ix += WIDTH;

 Container->setFixedSize(ix+100,HEIGHT);
 setView(Container);
 show();
}

/******************************************************************************
 *
 * Function:    createButton()
 *
 * Task:        Create a push button and place it. Set its name and an
 *              optional tooltip text
 *
 * in:          x,y,w,h         Position and size
 *              name            Nam of button
 *              TT              Tool-Tip text. If NULL, no tooltip is used.
 *
 * out:         QPushButton     Created push button
 *
 *****************************************************************************/
QPushButton* DemoClient::createButton( int x, int y, int w, int h, const char *name, const char *TT )
{
  QPushButton *pb = new QPushButton( name,Container,name );
  pb->setGeometry( x, y, w, h );
  //pb->setLineWidth( 1 );
  if (TT)
    QToolTip::add( pb, TT );
  return pb;
}

void DemoClient::createMenu()
{
  QPopupMenu *Mfile = new QPopupMenu;
  CHECK_PTR( Mfile );
  Mfile->insertItem( "Quit",  this, SLOT(quit())   , CTRL+Key_Q  );
  KMenuBar *mainmenu = new KMenuBar( this , "mainmenu" );
  CHECK_PTR( mainmenu );
  mainmenu->insertItem( "&File", Mfile );
  mainmenu->setMenuBarPos(KMenuBar::Top);
  setMenu(mainmenu);
}
