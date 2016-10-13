/*
   - 

  written 1998 by Alexander Budnik <budnik@linserv.jinr.ru>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */
#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <kiconloader.h>

#include "kcontrol.h"
#include "kwm.h"
#include "kikbdconf.h"
#include "widgets.h"

using namespace std;

class KiKbdApplication : public KControlApplication
{
protected:
  bool startKikbd;
public:
  KiKbdApplication(int &argc, char **arg);
  void init();
  void apply();
  void help();
  void defaultValues();
  KiKbdGeneralWidget *general;
  KiKbdStartupWidget *startup;
  KiKbdStyleWidget   *style;
};
KiKbdConfig *kikbdConfig;

KiKbdApplication::KiKbdApplication(int &argc, char **argv)
  : KControlApplication(argc, argv, "kikbd")
{
  if (runGUI())
    {
      //--- read configuration
      kikbdConfig = new KiKbdConfig(FALSE);
      kikbdConfig->loadConfig();

      startKikbd = (QString("-startkikbd")==argv[1])?TRUE:FALSE;
      mainWidget()->setIcon(getIconLoader()->loadMiniIcon("kikbd.xpm"));
      
      /**
	 General
      */
	general = new KiKbdGeneralWidget(dialog);
	addPage(general, i18n("&General"), "");

      /**
	 Style
      */
	style = new KiKbdStyleWidget(dialog);
	addPage(style, i18n("&Style"), "");

      /**
	 StartUp
      */
	startup = new KiKbdStartupWidget(dialog);
	addPage(startup, i18n("Start&Up"), "");

        // Needs to be done before showing the dialog, so that it shows with the
        // correct title (KControl relies on that to swallow it)
        ((KControlApplication *)kapp)->setTitle(i18n("International Keyboard"));

	dialog->show();
    }
  else init();
}
void KiKbdApplication::init()
{
  if(KiKbdConfig::readAutoStart()) system("kikbd &");
}
void KiKbdApplication::apply()
{
  kikbdConfig->saveConfig();
  if(startKikbd) {
    system("kikbd&");
    startKikbd = FALSE;
  }
  else
    system("kikbd -reconfig");
}
void KiKbdApplication::defaultValues()
{
  kikbdConfig->setDefaults();
}
void KiKbdApplication::help()
{
  kapp->invokeHTMLHelp("kikbd/kikbd-4.html","");
}

void msgHandler(QtMsgType type, const char* msg)
{
  switch(type) {
  case QtWarningMsg:
  case QtDebugMsg:
    break;
  case QtFatalMsg:
    cerr << msg << endl;
    exit(0);
  }
}

int main(int argc, char **argv)
{
  qInstallMsgHandler(msgHandler);
  KiKbdApplication app(argc, argv);
  
  if (app.runGUI())
    return app.exec();
  else
    return 0;
}
