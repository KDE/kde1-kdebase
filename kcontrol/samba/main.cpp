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


#include <kcontrol.h>
#include "ksmbstatus.h"


class KSambaApplication : public KControlApplication
{
public:

  KSambaApplication(int &argc, char **arg, char *name, char *title);

private:

  NetMon *status;
};


KSambaApplication::KSambaApplication(int &argc, char **argv, char *name, char *title)
  : KControlApplication(argc, argv, name, title)
{
  status = 0;

  if (runGUI())
    {
      if (!pages || pages->contains("status"))
	addPage(status = new NetMon(dialog, "bell"), "&Status", "sambastatus.html");
      
      dialog->setApplyButton(0);
      dialog->setCancelButton(0);
      dialog->show();
    }
}



int main(int argc, char **argv)
{
  KSambaApplication app(argc, argv, "samba", "Samba");
  
  if (app.runGUI())
    return app.exec();
  else
    return 0;
}
