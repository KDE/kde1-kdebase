/*  info_linux.cpp
    Linux-specific Information about the Hardware.

    written 1998 by Helge Deller (helge.deller@ruhr-uni-bochum.de)

    To do (maybe?):
    - include Information about XFree86 and/or Accelerated X 
	(needs to change configure-skript, to see, if Header-files are available !)
    - maybe also include information about the video-framebuffer devices (newer Kernels >= 2.1.100)
    - rewrite detection-routines (maybe not to use the /proc-fs)
    - more & better sound-information
*/


#include <syscall.h>
#include <stdio.h>
#include <linux/kernel.h>

#include <kapp.h>
#include <ktablistbox.h>

#define INFO_CPU_AVAILABLE
#define INFO_IRQ_AVAILABLE
#define INFO_DMA_AVAILABLE
#define INFO_PCI_AVAILABLE
#define INFO_IOPORTS_AVAILABLE
#define INFO_SOUND_AVAILABLE
#define INFO_DEVICES_AVAILABLE
#define INFO_SCSI_AVAILABLE
#define INFO_PARTITIONS_AVAILABLE
#define INFO_XSERVER_AVAILABLE


#define MAXCOLUMNWIDTH 600

bool GetInfo_ReadfromFile( KTabListBox *lBox, char *Name, char splitchar  )
{
  QString str;
  char buf[512];

  QFile *file = new QFile(Name);

  if(!file->open(IO_ReadOnly)) {
    delete file; 
    return FALSE;
  }
  
  while (file->readLine(buf,sizeof(buf)-1) > 0) {
      if (strlen(buf))
      {  char *p=buf;
         if (splitchar!=0)    /* remove leading spaces between ':' and the following text */
	     while (*p)
	     {	if (*p==splitchar)
	        { *p++ = ' ';
		  while (*p==' ') ++p;
		  *(--p) = splitchar;
		  ++p;
		}
		else ++p;
	     }
         lBox->setSeparator(splitchar);
         lBox->insertItem(buf);
      }
  }
  file->close();
  delete file;
  return TRUE;
}




bool GetInfo_CPU( KTabListBox *lBox )
{
  lBox->setNumCols(2);
  lBox->setColumn(0,i18n("Information"),150 );
  lBox->setColumn(1,i18n("Value") );
  return GetInfo_ReadfromFile( lBox, "/proc/cpuinfo", ':' );
}


bool GetInfo_IRQ( KTabListBox *lBox )
{
//  lBox->setAutoUpdate(FALSE);
//  lBox->setDefaultColumnWidth(MAXCOLUMNWIDTH );
  return GetInfo_ReadfromFile( lBox, "/proc/interrupts", 0 );
}

bool GetInfo_DMA( KTabListBox *lBox )
{
  lBox->setNumCols(2);
  lBox->setColumn(0,i18n("DMA-Channel"),100 );
  lBox->setColumn(1,i18n("used by") );
  return GetInfo_ReadfromFile( lBox, "/proc/dma", ':' );
}

bool GetInfo_PCI( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, "/proc/pci", 0 );
}

bool GetInfo_IO_Ports( KTabListBox *lBox )
{
  lBox->setNumCols(2);
  lBox->setColumn(0,i18n("I/O-Range"),100 );
  lBox->setColumn(1,i18n("used by") );
  return GetInfo_ReadfromFile( lBox, "/proc/ioports", ':' );
}

bool GetInfo_Sound( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, "/proc/sound", 0 );
}

bool GetInfo_Devices( KTabListBox *lBox )
{  
  GetInfo_ReadfromFile( lBox, "/proc/devices", 0 );
  lBox->insertItem(QString(""));
  lBox->insertItem(QString("Misc devices:"));
  GetInfo_ReadfromFile( lBox, "/proc/misc", 0 );
  return TRUE;
}

bool GetInfo_SCSI( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, "/proc/scsi/scsi", 0 );
}

bool GetInfo_Partitions( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, "/proc/partitions", 0 );
}



bool GetInfo_XServer_and_Video( KTabListBox *lBox )
{
  return GetInfo_XServer_Generic( lBox );
}

