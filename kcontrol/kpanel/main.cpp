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
#include "options.h"
#include "disknav.h"
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
    KOptionsConfig *options;
    KDiskNavConfig *disknav;
};

void sd(const QSize& r) {
    debug("size %d %d",r.width(), r.height());
}

KKPanelApplication::KKPanelApplication(int &argc, char **argv, 
				       const char *name)
    : KControlApplication(argc, argv, name)
{
    panel = 0; desktops = 0; options = 0, disknav = 0;

    if (runGUI())
	{
	    if (!pages || pages->contains("panel"))
		addPage(panel = new KPanelConfig(dialog, "panel"), 
			i18n("&Panel"), "kpanel-1.html");
	    if (!pages || pages->contains("options"))
		addPage(options = new KOptionsConfig(dialog, "options"),
			i18n("&Options"), "kpanel-2.html");
	    if (!pages || pages->contains("desktops"))
		addPage(desktops = new KDesktopsConfig(dialog, "desktops"),
			i18n("&Desktops"), "kpanel-3.html");
	    if (!pages || pages->contains("disknav"))
		addPage(disknav = new KDiskNavConfig(dialog, "disknav"),
			i18n("Disk &Navigator"), "kdisknav.html");
	    
	    if (panel || desktops || options || disknav) {
		QSize t;
		int w = 0, h = 0;
		
		if (panel) {
		    t = panel->minimumSize();
		    w = t.width();
		    h = t.height();
		}
		if (desktops) {
		    t = desktops->minimumSize();
		    if (w < t.width())
			w = t.width();
		    if (h < t.height())
			h = t.height();
		}
		if (options) {
		    t = options->minimumSize();
		    if (w < t.width())
			w = t.width();
		    if (h < t.height())
			h = t.height();
		}
		if (disknav) {
		    t = disknav->minimumSize();
		    if (w < t.width())
			w = t.width();
		    if (h < t.height())
			h = t.height();
		}
		dialog->resize(w,h);
		dialog->show();
	    }
	    else {
		fprintf(stderr, i18n("usage: kcmkpanel [-init | {panel,options,desktops,disknav}]\n"));
		justInit = true;
	    }
	    
	}
}


void KKPanelApplication::init() {}


void KKPanelApplication::apply()
{
    if (panel)
	panel->saveSettings();
    if (options)
	options->saveSettings();
    bool restarted = false;
    if (desktops) // desktop restarts kpanel by it's own
	restarted = desktops->justSave();
    if (disknav)
	disknav->saveSettings();
    if (!restarted)
	KWM::sendKWMCommand("kpanel:restart");
}


int main(int argc, char **argv)
{
    config = new KConfig(KApplication::kde_configdir() + "/kpanelrc", 
                         KApplication::localconfigdir() + "/kpanelrc");
    KKPanelApplication app(argc, argv, "kcmkpanel");
    
    app.setTitle(i18n("KPanel Configuration"));

    int ret;
    if (app.runGUI())
	ret = app.exec();
    else
	ret = 0;

    delete config;
    return ret;
}
