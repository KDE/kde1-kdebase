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
#include <klocale.h>
 
#include "kcontrol.h"
#include "syssound.h"


class KSyssoundApplication : public KControlApplication
{
public:

  KSyssoundApplication(int &argc, char **arg, char *name);

  void init();
  void apply();

private:

  KSoundWidget *sound;
};


KSyssoundApplication::KSyssoundApplication(int &argc, char **argv, char *name)
  : KControlApplication(argc, argv, name)
{
  sound = 0;

  if (runGUI())
    {
      if (!pages || pages->contains("syssound"))
        addPage(sound = new KSoundWidget(dialog, "sound"), klocale->translate("&Sound"), "syssound.html");

      dialog->show();
    }
}





void KSyssoundApplication::init()
{
  KWM::sendKWMCommand("syssnd_restart");
}


void KSyssoundApplication::apply()
{
  if (sound)
    sound->applySettings();
}


int main(int argc, char **argv)
{
  KSyssoundApplication app(argc, argv, "kcmsyssound");

  app.setTitle(klocale->translate("System Sounds"));

  if (app.runGUI())
    return app.exec();
  else
    return 0;
}






