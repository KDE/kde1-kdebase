/*
  main.cpp - A sample KControl Application

  written 1997 by Matthias Hoelzer

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


//#include <kapp.h>
//#include <qobject.h>
#include <kslider.h>

#include <kcontrol.h>
#include <kimgio.h>
#include "display.h"
#include "colorscm.h"
#include "scrnsave.h"
#include "general.h"
#include "backgnd.h"

class KDisplayApplication : public KControlApplication
{
public:

  KDisplayApplication(int &argc, char **arg, const char *name);

  void init();
  void apply();
  void defaultValues();

private:

  KColorScheme *colors;
  KScreenSaver *screensaver;
  KFonts *fonts;
  KGeneral *general;
  KBackground *background;
};


KDisplayApplication::KDisplayApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  colors = 0; screensaver = 0; fonts = 0; general = 0; background = 0;

  if (runGUI())
    {
      if (!pages || pages->contains("background"))
        addPage(background = new KBackground(dialog, KDisplayModule::Setup),
		klocale->translate("&Background"), "kdisplay-3.html");
      if (!pages || pages->contains("screensaver"))
	addPage(screensaver = new KScreenSaver(dialog, KDisplayModule::Setup),
		klocale->translate("&Screensaver"), "kdisplay-4.html");

      if (!pages || pages->contains("colors"))
	addPage(colors = new KColorScheme(dialog, KDisplayModule::Setup),
		klocale->translate("&Colors"), "kdisplay-5.html");
      if (!pages || pages->contains("fonts"))
	addPage(fonts = new KFonts(dialog, KDisplayModule::Setup),
		klocale->translate("&Fonts"), "kdisplay-6.html");
      if (!pages || pages->contains("style"))
	addPage(general = new KGeneral(dialog, KDisplayModule::Setup),
		klocale->translate("&Style"), "kdisplay-7.html");

      if (background || screensaver || colors || fonts || general)
        dialog->show();
      else
        {
          fprintf(stderr, klocale->translate("usage: kcmdisplay [-init | {background,screensaver,colors,fonts,style}]\n"));
          justInit = TRUE;
        }

    }
}


void KDisplayApplication::init()
{
  KColorScheme *colors = new KColorScheme(0, KDisplayModule::Init);
  delete colors;
  KBackground *background =  new KBackground(0, KDisplayModule::Init);
  delete background;
  KScreenSaver *screensaver = new KScreenSaver(0, KDisplayModule::Init);
  delete screensaver;
  KFonts *fonts = new KFonts(0, KDisplayModule::Init);
  delete fonts;
  KGeneral *general = new KGeneral(0, KDisplayModule::Init);
  delete general;

  KConfigGroupSaver saver(kapp->getConfig(), "X11");
  if (kapp->getConfig()->readBoolEntry( "useResourceManager", false )){
      KProcess proc;
      proc.setExecutable("krdb1");
      proc.start( KProcess::Block );
  }


}


void KDisplayApplication::apply()
{
  if (colors)
    colors->applySettings();
  if (background)
    background->applySettings();
  if (screensaver)
    screensaver->applySettings();
  if (fonts)
    fonts->applySettings();
  if (general)
    general->applySettings();
}

void KDisplayApplication::defaultValues()
{
  if (colors)
    colors->defaultSettings();
  if (background)
    background->defaultSettings();
  if (screensaver)
    screensaver->defaultSettings();
  if (fonts)
    fonts->defaultSettings();
  if (general)
    general->defaultSettings();
}

int main(int argc, char **argv)
{
  //QApplication::setColorSpec( QApplication::ManyColor );
  // Please don't use this as it prevents users from choosing exactly the
  // colors they want - Mark Donohoe

  KDisplayApplication app(argc, argv, "kcmdisplay");
  app.setTitle(klocale->translate("Display settings"));

  kimgioRegister();

  if (app.runGUI())
    return app.exec();
  else
    {
      app.init();
      return 0;
    }
}




