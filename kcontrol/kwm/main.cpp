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


#include <kwm.h>
#include <kcontrol.h>
#include "windows.h"
#include "desktop.h"
#include "titlebar.h"
#include <ksimpleconfig.h>

KSimpleConfig *config;

class KKWMApplication : public KControlApplication
{
public:

  KKWMApplication(int &argc, char **arg, const char *name);

  void init();
  void apply();

private:

  KWindowConfig *windows;
  KTitlebarButtons *buttons;
  KTitlebarAppearance *appearance;
  KDesktopConfig *desktop;
};


KKWMApplication::KKWMApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  windows = 0; buttons = 0; appearance = 0; desktop = 0;

  if (runGUI())
    {
      if (!pages || pages->contains("windows"))
	addPage(windows = new KWindowConfig(dialog, "windows"), 
		klocale->translate("&Windows"), "kwm-3.html");
      if (!pages || pages->contains("buttons"))
	addPage(buttons = new KTitlebarButtons(dialog, "buttons"),
		klocale->translate("&Buttons"), "kwm-1.html");
      if (!pages || pages->contains("titlebar"))
	addPage(appearance = new KTitlebarAppearance(dialog, "titlebar"), 
		klocale->translate("&Titlebar"), "kwm-2.html");
      if (!pages || pages->contains("desktop"))
	addPage(desktop = new KDesktopConfig(dialog, "desktop"), 
		klocale->translate("&Desktop"), "kwm-3.html");

      if (windows || buttons || appearance || desktop)
        dialog->show();
      else
        {
          fprintf(stderr, klocale->translate("usage: kcmkwm [-init | {windows,buttons,titlebar,desktop}]\n"));
          justInit = TRUE;
        }

    }
}


void KKWMApplication::init()
{
  KWM::configureWm();
}


void KKWMApplication::apply()
{
  if (windows)
    windows->applySettings();
  if (desktop)
    desktop->applySettings();
  if (buttons)
    buttons->applySettings();
  if (appearance)
    appearance->applySettings();
}


int main(int argc, char **argv)
{
    config = new KSimpleConfig(KApplication::localconfigdir() + "/kwmrc");
    KKWMApplication app(argc, argv, "kcmkwm");
    app.setTitle(klocale->translate("Window manager style"));
    
    if (app.runGUI())
	return app.exec();
    else
	return 0;
    delete config;
}
