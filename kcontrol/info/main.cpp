/*
  main.cpp for kcminfo = "control-panel information"

  based on:
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
   
  Last modified:
      - 3. Oktober 1998 by Helge Deller (helge.deller@ruhr-uni-bochum.de)
  */


#include <kcontrol.h>
#include "memory.h"

/* we have to include the info.cpp-file, to get the DEFINES about possible properties.
   example: we need the "define INFO_CPU_AVAILABLE" */
#include "info.cpp"
#include "info.h"


class KInfoApplication : public KControlApplication
{
public:

  KInfoApplication(int &argc, char **arg, const char *name);

private:

  KMemoryWidget *memory;
  KInfoListWidget *processor, *interrupts, *dma, *ioports, *pci, *sound,
  	*devices, *scsi, *partitions, *xserver; 
	
public slots:
  virtual void defaultValues();
};



#define CHECK_PAGE(pagename,widgetptr,getproc,title,helpfile) \
      if (!pages || pages->contains(pagename))		\
	addPage(widgetptr = new KInfoListWidget(dialog, pagename, title, getproc), \
		title, helpfile)


KInfoApplication::KInfoApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  memory = 0; 
  processor = 0;
  interrupts = 0;
  dma = 0;
  ioports = 0;
  pci= 0;
  sound = 0;
  devices = 0;
  scsi= 0;
  partitions = 0;
  xserver = 0;

  if (runGUI())
    {
      if (!pages || pages->contains("memory"))
	addPage(memory = new KMemoryWidget(dialog, "memory"), 
		i18n("&Memory"), "info-1.html");

  #ifdef INFO_CPU_AVAILABLE
      CHECK_PAGE("processor",  processor,  GetInfo_CPU,               i18n("&Processor"),   "info-2.html" );
  #endif      
  #ifdef INFO_IRQ_AVAILABLE
      CHECK_PAGE("interrupts", interrupts, GetInfo_IRQ,               i18n("&Interrupts"),  "info-3.html" );
  #endif      
  #ifdef INFO_DMA_AVAILABLE
      CHECK_PAGE("dma",        dma,        GetInfo_DMA,               i18n("DMA-C&hannels"),"info-4.html" );
  #endif      
  #ifdef INFO_IOPORTS_AVAILABLE
      CHECK_PAGE("ioports",    ioports,    GetInfo_IO_Ports,          i18n("&IO-Ports"),    "info-5.html" );
  #endif      
  #ifdef INFO_PCI_AVAILABLE
      CHECK_PAGE("pci",        pci,        GetInfo_PCI,               i18n("PCI-&Bus"),     "info-6.html" );
  #endif      
  #ifdef INFO_SOUND_AVAILABLE
      CHECK_PAGE("sound",      sound,      GetInfo_Sound,             i18n("S&ound"),       "info-7.html" );
  #endif      
  #ifdef INFO_DEVICES_AVAILABLE
      CHECK_PAGE("devices",    devices,    GetInfo_Devices,           i18n("&Devices"),     "info-8.html" ); 
  #endif      
  #ifdef INFO_SCSI_AVAILABLE
      CHECK_PAGE("scsi",       scsi,       GetInfo_SCSI,              i18n("S&CSI"),        "info-9.html" ); 
  #endif      
  #ifdef INFO_PARTITIONS_AVAILABLE
      CHECK_PAGE("partitions", partitions, GetInfo_Partitions,        i18n("P&artitions"),  "info-10.html"); 
  #endif      
  #ifdef INFO_XSERVER_AVAILABLE
      CHECK_PAGE("xserver",    xserver,    GetInfo_XServer_and_Video, i18n("&X-Server"),    "info-11.html"); 
  #endif
  
      dialog->setApplyButton(0);
      dialog->setCancelButton(0);

      if (memory || processor || interrupts || dma || ioports || pci || 
          sound || devices || scsi || partitions || xserver )
        dialog->show();
      else
        {
          fprintf(stderr, i18n("usage: kcminfo [-init | {memory,processor,interrupts,dma,ioports,pci,sound,devices,scsi,partitions,xserver}]\n"));
          justInit = TRUE;
        }

    }
}


#define LOAD_DEFAULTS(service) if (service) service->defaultSettings()

void KInfoApplication::defaultValues()
{
 LOAD_DEFAULTS(processor);
 LOAD_DEFAULTS(interrupts);
 LOAD_DEFAULTS(dma);
 LOAD_DEFAULTS(ioports);
 LOAD_DEFAULTS(pci);
 LOAD_DEFAULTS(sound);
 LOAD_DEFAULTS(devices);
 LOAD_DEFAULTS(scsi);
 LOAD_DEFAULTS(partitions);
 LOAD_DEFAULTS(xserver);
}


int main(int argc, char **argv)
{
  KInfoApplication app(argc, argv, "kcminfo");
  app.setTitle(i18n("System Information"));
  
  if (app.runGUI())
    return app.exec();
  else
    return 0;
}
