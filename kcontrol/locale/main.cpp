/*
  main.cpp - A KControl Application

  written 1998 by Matthias Hoelzer
  
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


#include <kcontrol.h>
#include "locale.h"


class KLocaleApplication : public KControlApplication
{
public:

  KLocaleApplication(int &argc, char **arg, const char *name);

  void init();
  void apply();

private:

  KLocaleConfig *locale;
};


KLocaleApplication::KLocaleApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  locale = 0;

  if (runGUI())
    {
      if (!pages || pages->contains("language"))
        addPage(locale = new KLocaleConfig(dialog, "locale"), 
		klocale->translate("&Locale"), "locale.html");

      if (locale)
        dialog->show();
      else
        {
          fprintf(stderr, klocale->translate("usage: kcmlocale [-init | language])\n"));
	  justInit = TRUE;
        }
    }
}


void KLocaleApplication::init()
{
}


void KLocaleApplication::apply()
{
  if (locale)
    locale->applySettings();
}


int main(int argc, char **argv)
{
  KLocaleApplication app(argc, argv, "kcmlocale");
  app.setTitle(klocale->translate("Locale settings"));
  
  if (app.runGUI())
    return app.exec();
  else
    {
      app.init();
      return 0;
    }
}
