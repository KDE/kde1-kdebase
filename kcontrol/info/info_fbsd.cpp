/*
 * $Id$
 * info_fbsd.cpp is part of the KDE program kcminfo.  This displays
 * various information about the system it's running on.
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


/*  all following functions should return TRUE, when the Information 
    was filled into the lBox-Widget.
    returning FALSE indicates, that information was not available.
*/
       

#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include <qfile.h>

/* stdio.h has NULL, but also a lot of extra cruft */
#ifndef NULL
#define NULL 0L
#endif

/* Again avoid the cruft in stdlib.h since malloc() isn't gonna change 
   too often, unless someone decides to de-KNR FreeBSD */
void    *malloc __P((size_t));

bool GetInfo_CPU (KTabListBox *lBox)
{ 
  QString str;

  /* Stuff for sysctl */
  char *buf, *mhz, *cpustring;
  int mib[2], machspeed;
  size_t len;

  cpustring=(char *)malloc(128);

  mib[0] = CTL_HW;
  mib[1] = HW_MODEL;
  sysctl(mib,2,NULL,&len,NULL,0);
  buf=(char*)malloc(len);
  sysctl(mib,2,buf,&len,NULL,0);
  /*	Get the CPU speed, only on Genuine P5s
		heh, heh, undocumented sysctls rule but I dunno
		if this works on 2.2.x machines. */
  mib[0] = CTL_MACHDEP; mib[1] = 107;
  len=sizeof(machspeed);
  sysctl(mib,2,&machspeed,&len,NULL,0);
  /* Format the integer into correct xxx.xx MHz */
  mhz=(char *)malloc(20);
  snprintf(mhz,20,"%d.%02d",(machspeed+4999)/1000000,
		   ((machspeed+4999)/10000)%100);
  if (strcmp(mhz,"0.00")==0)
 	/* We dunno how fast it's running */
 	snprintf(cpustring,128,i18n("%s, unknown speed"),buf);
  else
	snprintf(cpustring,128,i18n("%s running at %s MHz"),buf,mhz);

  /* Put everything in the listbox */
  lBox->insertItem(cpustring);
  /* Clean up after ourselves, this time I mean it ;-) */
  free(mhz); free(cpustring); free(buf);

  return TRUE;
}

bool GetInfo_IRQ (KTabListBox *)
{
	return FALSE;
}

bool GetInfo_DMA (KTabListBox *)
{
	return FALSE;
}

bool GetInfo_PCI (KTabListBox *)
{
	return FALSE;
}

bool GetInfo_IO_Ports (KTabListBox *)
{
	return FALSE;
}

bool GetInfo_Sound (KTabListBox *lbox)
{
	QFile *sndstat = new QFile("/dev/sndstat");

	if (!sndstat->exists()) {
		delete sndstat;
		return false;
	}

	if (!sndstat->open(IO_ReadOnly)) {
		delete sndstat;
		return false;
	}

	lbox->clear();

	QTextStream *t = new QTextStream(sndstat);
	QString s;

	while ((s=t->readLine())!="")
		lbox->insertItem(s);

	delete t;
	sndstat->close();
	delete sndstat;
	return true;
}

bool GetInfo_Devices (KTabListBox *)
{
	return FALSE;
}

bool GetInfo_SCSI (KTabListBox *)
{
	return FALSE;
}

bool GetInfo_Partitions (KTabListBox *)
{
	return FALSE;
}

bool GetInfo_XServer_and_Video (KTabListBox *lBox)
{
	return GetInfo_XServer_Generic( lBox );
}

