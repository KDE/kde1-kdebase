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
#include "memory.h"
#include "processor.h"


class KInfoApplication : public KControlApplication
{
public:

  KInfoApplication(int &argc, char **arg, const char *name);

private:

  KMemoryWidget *memory;
  KProcessorWidget *processor; 
};


KInfoApplication::KInfoApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  memory = 0; processor = 0;

  if (runGUI())
    {
      if (!pages || pages->contains("memory"))
	addPage(memory = new KMemoryWidget(dialog, "memory"), 
		klocale->translate("&Memory"), "memory.html");
      if (!pages || pages->contains("processor"))
	addPage(processor = new KProcessorWidget(dialog, "processor"), 
		klocale->translate("&Processor"), "processor.html");
      
      dialog->setApplyButton(0);
      dialog->setCancelButton(0);
      dialog->show();
    }
}



int main(int argc, char **argv)
{
  KInfoApplication app(argc, argv, "kcminfo");
  app.setTitle(klocale->translate("System Information"));
  
  if (app.runGUI())
    return app.exec();
  else
    return 0;
}
