/*  info_linux.cpp
    Linux-specific Information about the Hardware.

    written 1998 by Helge Deller (helge.deller@ruhr-uni-bochum.de)

    To do (maybe?):
    - include Information about XFree86 and/or Accelerated X 
	(needs to change configure-skript, to see, if Header-files are available !)
    - maybe also include information about the video-framebuffer devices (newer Kernels >= 2.1.100)
    - rewrite detection-routines (maybe not to use the /proc-fs)
    - more & better sound-information
 
    /dev/sndstat support added: 1998-12-08 Duncan Haldane (f.d.m.haldane@cwix.com)
*/


#include <syscall.h>
#include <stdio.h>
#include <linux/kernel.h>

#include <ctype.h>

#include <kapp.h>
#include <ktablistbox.h>

#define INFO_CPU_AVAILABLE
#define INFO_CPU "/proc/cpuinfo"

#define INFO_IRQ_AVAILABLE
#define INFO_IRQ "/proc/interrupts"

#define INFO_DMA_AVAILABLE
#define INFO_DMA "/proc/dma"

#define INFO_PCI_AVAILABLE
#define INFO_PCI "/proc/pci"

#define INFO_IOPORTS_AVAILABLE
#define INFO_IOPORTS "/proc/ioports"

#define INFO_SOUND_AVAILABLE
#define INFO_DEV_SNDSTAT "/dev/sndstat"
#define INFO_SOUND "/proc/sound" 

#define INFO_DEVICES_AVAILABLE
#define INFO_DEVICES "/proc/devices"
#define INFO_MISC "/proc/misc"

#define INFO_SCSI_AVAILABLE
#define INFO_SCSI "/proc/scsi/scsi"

#define INFO_PARTITIONS_AVAILABLE
#define INFO_PARTITIONS "/proc/partitions"

#define INFO_XSERVER_AVAILABLE


#define MAXCOLUMNWIDTH 600

bool GetInfo_ReadfromFile( KTabListBox *lBox, char *Name, char splitchar  )
{
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
	     {
		if (!isgraph(*p))
			*p = ' ';
		if (*p==splitchar)
	        { *p++ = ' ';
		  while (*p==' ') ++p;
		  *(--p) = splitchar;
		  ++p;
		}
		else ++p;
	     }
	 else
	 {
		while (*p)
		{
		   if (!isgraph(*p))
			*p = ' ';
		   ++p;
		}
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
  return GetInfo_ReadfromFile( lBox, INFO_CPU, ':' );
}


bool GetInfo_IRQ( KTabListBox *lBox )
{
//  lBox->setAutoUpdate(FALSE);
//  lBox->setDefaultColumnWidth(MAXCOLUMNWIDTH );
  return GetInfo_ReadfromFile( lBox, INFO_IRQ, 0 );
}

bool GetInfo_DMA( KTabListBox *lBox )
{
  lBox->setNumCols(2);
  lBox->setColumn(0,i18n("DMA-Channel"),100 );
  lBox->setColumn(1,i18n("used by") );
  return GetInfo_ReadfromFile( lBox, INFO_DMA, ':' );
}

bool GetInfo_PCI( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, INFO_PCI, 0 );
}

bool GetInfo_IO_Ports( KTabListBox *lBox )
{
  lBox->setNumCols(2);
  lBox->setColumn(0,i18n("I/O-Range"),100 );
  lBox->setColumn(1,i18n("used by") );
  return GetInfo_ReadfromFile( lBox, INFO_IOPORTS, ':' );
}

bool GetInfo_Sound( KTabListBox *lBox )
{
  if ( GetInfo_ReadfromFile( lBox, INFO_DEV_SNDSTAT, 0 )) 
    return TRUE;
  else 
    return GetInfo_ReadfromFile( lBox, INFO_SOUND, 0 );
}

bool GetInfo_Devices( KTabListBox *lBox )
{  
  GetInfo_ReadfromFile( lBox, INFO_DEVICES, 0 );
  lBox->insertItem(QString(""));
  lBox->insertItem(QString("Misc devices:"));
  GetInfo_ReadfromFile( lBox, INFO_MISC, 0 );
  return TRUE;
}

bool GetInfo_SCSI( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, INFO_SCSI, 0 );
}

bool GetInfo_Partitions( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, INFO_PARTITIONS, 0 );
}



bool GetInfo_XServer_and_Video( KTabListBox *lBox )
{
  return GetInfo_XServer_Generic( lBox );
}
