/* 	
    info_svr4.cpp

    UNIX System V Release 4 specific Information about the Hardware.
    Appropriate for SCO OpenServer and UnixWare.
    Written 20-Feb-99 by Ronald Joe Record (rr@sco.com)
    Initially based on info_sgi.cpp
*/

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

#define INFO_DEV_SNDSTAT "/dev/sndstat"

#include <sys/systeminfo.h>

/*  all following functions should return TRUE, when the Information 
    was filled into the lBox-Widget.
    returning FALSE indicates, that information was not available.
*/

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
      QString str;
      char buf[256];

      sysinfo(SI_ARCHITECTURE, buf, sizeof(buf));
      str = buf;
      lBox->insertItem(str);
      return TRUE;
}


bool GetInfo_IRQ( KTabListBox *lBox )
{	lBox = lBox;
	return FALSE;
}

bool GetInfo_DMA( KTabListBox *lBox )
{	lBox = lBox;
	return FALSE;
}

bool GetInfo_PCI( KTabListBox *lBox )
{
      QString str;
      char buf[256];

      sysinfo(SI_BUSTYPES, buf, sizeof(buf));
      str = buf;
      lBox->insertItem(str);
      return TRUE;
}

bool GetInfo_IO_Ports( KTabListBox *lBox )
{	lBox = lBox;
	return FALSE;
}

bool GetInfo_Sound( KTabListBox *lBox )
{
  if ( GetInfo_ReadfromFile( lBox, INFO_DEV_SNDSTAT, 0 ))
    return TRUE;
  else
    return FALSE;
}

bool GetInfo_Devices( KTabListBox *lBox )
{	lBox = lBox;
	return FALSE;
}

bool GetInfo_SCSI( KTabListBox *lBox )
{	lBox = lBox;
	return FALSE;
}

bool GetInfo_Partitions( KTabListBox *lBox )
{	lBox = lBox;
	return FALSE;
}

bool GetInfo_XServer_and_Video( KTabListBox *lBox )
{	lBox = lBox;
	return GetInfo_XServer_Generic( lBox );
}

