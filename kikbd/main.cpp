#include <stream.h>
#include <string.h>
#include <stdlib.h>

#include <kiconloader.h>

#include "kcontrol.h"
#include "kwm.h"
#include "kikbdconf.h"
#include "widgets.h"

class KiKbdApplication : public KControlApplication
{
public:
  KiKbdApplication(int &argc, char **arg, char *name);
  void init();
  void apply();
  void help();
  KiKbdGeneralWidget *general;
  KiKbdStartupWidget *startup;
  KiKbdStyleWidget  *style;
};
KiKbdConfig *kikbdConfig;

KiKbdApplication::KiKbdApplication(int &argc, char **argv,
				   char *name)
  : KControlApplication(argc, argv, name)
{
  if (runGUI())
    {
      mainWidget()->setIcon(getIconLoader()->loadMiniIcon("kikbd.xpm"));
      //--- read configuration
      kikbdConfig = new KiKbdConfig();
      kikbdConfig->loadConfig();
      
      /**
	 General
      */
      //if (!pages || pages->contains("general")) {
	general = new KiKbdGeneralWidget(dialog, "general");
	general->loadSettings();
	addPage(general, klocale->translate("&General"), "");
	//}

      /**
	 Style
      */
	//if (!pages || pages->contains("style")) {
	style = new KiKbdStyleWidget(dialog, "style");
	addPage(style, klocale->translate("&Style"), "");
	connect(dialog, SIGNAL(selected(const char*)), style, 
		SLOT(aboutToShow(const char*)));
	//}

      /**
	 StartUp
      */
	//if (!pages || pages->contains("startup")) {
	startup = new KiKbdStartupWidget(dialog, "startup");
	addPage(startup, klocale->translate("Start&Up"), "");
	//}

	//if(general || style || startup)
	dialog->show();
    }
  else init();
}
void KiKbdApplication::init()
{
  kikbdConfig = new KiKbdConfig();
  kikbdConfig->loadConfig();
  if(kikbdConfig->getAutoStart()) {
    system("kikbd &");
  }
}
void KiKbdApplication::apply()
{
  general->applySettings();
  kikbdConfig->saveConfig();
  system("kikbd -reconfig");
}
void KiKbdApplication::help()
{
  kapp->invokeHTMLHelp("kikbd/kikbd.html","");
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
  KiKbdApplication app(argc, argv, "kikbd");
  app.setTitle(klocale->translate("International Keyboard"));
  
  if (app.runGUI())
    return app.exec();
  else
    return 0;
}
