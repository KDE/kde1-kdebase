/*
  main.cpp - KPanel configuration module

  written 1997 by Stephan Kulow
  
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
#include "panel.h"
#include "desktops.h"
#include <ksimpleconfig.h>

KConfigBase *config;

class KKPanelApplication : public KControlApplication
{
public:

    KKPanelApplication(int &argc, char **arg, const char *name);
    
    virtual void init();
    virtual void apply();
    
private:

    KPanelConfig *panel;
    KDesktopsConfig *desktops;
};


KKPanelApplication::KKPanelApplication(int &argc, char **argv, 
				       const char *name)
    : KControlApplication(argc, argv, name)
{
  panel = 0; desktops = 0;

  if (runGUI())
    {
      if (!pages || pages->contains("panel"))
	addPage(panel = new KPanelConfig(dialog, "panel"), 
		klocale->translate("&Panel"), "panel.html");
      if (!pages || pages->contains("desktops"))
	addPage(desktops = new KDesktopsConfig(dialog, "desktops"),
		klocale->translate("&Desktops"), "desktops.html");
      dialog->show();
    }
}


void KKPanelApplication::init() {}


void KKPanelApplication::apply()
{
    if (panel)
	panel->saveSettings();
    if (desktops)
	desktops->justSave();
    config->sync();
    KWM::sendKWMCommand("kpanel:restart");
}


int main(int argc, char **argv)
{
    config = new KConfig(KApplication::localconfigdir() + "/kpanelrc");
    KKPanelApplication app(argc, argv, "kcmkpanel");
    
    app.setTitle(klocale->translate("KPanel Configuration"));

    int ret;
    if (app.runGUI())
	ret = app.exec();
    else
	ret = 0;

    delete config;
    return ret;
}
