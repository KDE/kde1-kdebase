/*
 * main.cpp
 *
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <kimgio.h>
#include <kslider.h>
#include <kmsgbox.h>

#define private public
#include <kcontrol.h>
#undef private

#include "installer.h"
#include "options.h"
#include "about.h"
#include "theme.h"

static msg_handler oldMsgHandler = 0;


//-----------------------------------------------------------------------------
class KThemesApplication : public KControlApplication
{
public:

  KThemesApplication(int &argc, char **arg, const char *name);

  virtual void init();
  virtual void apply();
  virtual void defaultValues();

private:
  Installer* mInstaller;
  Options* mOptions;
  About* mAbout;
};



//-----------------------------------------------------------------------------
KThemesApplication::KThemesApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  initMetaObject();

  if (!(Theme::mkdirhier(Theme::workDir()))) exit(1);
  if (!(Theme::mkdirhier(Theme::themesDir()))) exit(1);

  mInstaller = 0;

  if (runGUI())
  {
    addPage(mInstaller = new Installer(dialog), i18n("Installer"), "kthememgr-1.html" );
    addPage(mOptions = new Options(dialog), i18n("Contents"), "kthememgr-2.html" );
    addPage(mAbout = new About(dialog), i18n("About"), "kthememgr-3.html" );

    connect(mInstaller, SIGNAL(changed(Theme *)), 
            mOptions, SLOT(slotThemeChanged(Theme *)));
    connect(mInstaller, SIGNAL(changed(Theme *)), 
            mAbout, SLOT(slotThemeChanged(Theme *)));

    dialog->show();
    mInstaller->readThemesList();
  }
}

//-----------------------------------------------------------------------------
void KThemesApplication::init()
{
  //debug(i18n("No init necessary"));
}


//-----------------------------------------------------------------------------
void KThemesApplication::defaultValues()
{
    mInstaller->defaultSettings();
}


//-----------------------------------------------------------------------------
void KThemesApplication::apply()
{
  mAbout->applySettings();
  mOptions->applySettings();
  mInstaller->applySettings();
}


//=============================================================================
// Message handler
static void msgHandler(QtMsgType aType, const char* aMsg)
{
  QString appName = kapp->appName();
  QString msg = aMsg;
  msg.detach();

  switch (aType)
  {
  case QtDebugMsg:
    kdebug(KDEBUG_INFO, 0, msg);
    break;

  case QtWarningMsg:
    fprintf(stderr, "%s: %s\n", (const char*)appName, msg.data());
    if (strncmp(aMsg,"KCharset:",9) != 0 &&
	strncmp(aMsg,"QGManager:",10) != 0 &&
	strncmp(aMsg,"QPainter:",9) != 0 &&
	strncmp(aMsg,"QPixmap:",8) != 0 &&
	strncmp(aMsg,"QFile:", 6) != 0)
    {
      KMsgBox::message(NULL, appName+" "+i18n("warning"), msg.data(),
		       KMsgBox::EXCLAMATION);
    }
    else kdebug(KDEBUG_INFO, 0, msg);
    break;

  case QtFatalMsg:
    fprintf(stderr, appName+" "+i18n("fatal error")+": %s\n", msg.data());
    KMsgBox::message(NULL, appName+" "+i18n("fatal error"),
		     aMsg, KMsgBox::STOP);
    abort();
  }
}


//-----------------------------------------------------------------------------
void init(void)
{
  oldMsgHandler = qInstallMsgHandler(msgHandler);

}


//-----------------------------------------------------------------------------
void cleanup(void)
{
  qInstallMsgHandler(oldMsgHandler);
}


//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  char *ch = malloc(100);
  kimgioRegister();

  KThemesApplication app(argc, argv, "kthememgr");
  app.setTitle(i18n("Kde Theme Manager"));
  init();

  if (app.runGUI()) app.exec();
  else app.init();

  cleanup();
  return 0;
}
