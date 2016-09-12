/*
 * $Id: info_fbsd.cpp,v 1.10 1998/11/09 09:14:30 garbanzo Exp $
 *
 * info_fbsd.cpp is part of the KDE program kcminfo.  This displays
 * various information about the system (hopefully a FreeBSD system)
 * it's running on.
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


/*
 * all following functions should return TRUE, when the Information 
 * was filled into the lBox-Widget. Returning FALSE indicates that
 * information was not available.
 */

#include <sys/types.h>
#include <sys/sysctl.h>

#include <fstab.h>
#include <stdlib.h>

#include <qfile.h>
#include <qfontmetrics.h>

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
  /* systat lists the interrupts assigned to devices as well as how many were
     generated.  Parsing its output however is about as fun as a sandpaper
     enema.  The best idea would probably be to rip out the guts of systat.
     Too bad it's not very well commented */
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
  QTextStream *t; QString s;
 
  if (!sndstat->exists()) {
    delete sndstat;
    return false;
  }
  
  if (!sndstat->open(IO_ReadOnly)) {
    delete sndstat;
    return false;
  }
  
  lbox->clear();
  
  t = new QTextStream(sndstat);

  while ((s=t->readLine())!="")
    lbox->insertItem(s);

  delete t;
  sndstat->close();
  delete sndstat;
  return true;
}

bool GetInfo_Devices (KTabListBox *lbox)
{
	QFile *dmesg = new QFile("/var/run/dmesg.boot");

	if (!dmesg->exists()) {
		delete dmesg;
		return false;
	}

	if (!dmesg->open(IO_ReadOnly)) {
		delete dmesg;
		return false;
	}

	lbox->clear();

	QTextStream *t = new QTextStream(dmesg);
	QString s;

	while ((s=t->readLine())!="")
		lbox->insertItem(s);

	delete t;
	dmesg->close();
	delete dmesg;
	return true;
}

bool GetInfo_SCSI (KTabListBox *lbox)
{
  /*
   * This code relies on the system at large having "the" CAM (see the FreeBSD
   * 3.0 Release notes for more info) SCSI layer, and not the older one.
   * If someone who has a system with the older SCSI layer and would like to
   * tell me (garbanzo@hooked.net) how to extract that info, I'd be much
   * obliged.
   */
  FILE *pipe;
  QFile *camcontrol = new QFile("/sbin/camcontrol");
  QTextStream *t;
  QString s;
  
  if (!camcontrol->exists()) {
    delete camcontrol;
    return false;
  }
  
  /* This prints out a list of all the scsi devies, perhaps eventually we could
     parse it as opposed to schlepping it into a listbox */
  if ((pipe = popen("/sbin/camcontrol devlist", "r")) == NULL) {
    delete camcontrol;
    return false;
  }

  t = new QTextStream(pipe, IO_ReadOnly);

  while ((s=t->readLine())!="")
    lbox->insertItem(s);
  
  delete t; delete camcontrol; pclose(pipe);
  
  if (!lbox->count())
    return false;
  
  return true;
}

bool GetInfo_Partitions (KTabListBox *lbox)
{
	int maxwidth[4]={0,0,0,0};

	struct fstab *fstab_ent;
	QFontMetrics fm(lbox->tableFont());
	QString s;

	if (setfsent() != 1) /* Try to open fstab */ {
		kdebug(KDEBUG_ERROR, 0, i18n("Ahh couldn't open fstab!"));
		return false;
	}

	lbox->setNumCols(4);
	lbox->setSeparator(';');

	maxwidth[0]=fm.width(i18n("Device"));
	lbox->setColumn(0, i18n("Device"), maxwidth[0]+2);

	maxwidth[1]=fm.width(i18n("Mount Point"));
	lbox->setColumn(1, i18n("Mount Point"), maxwidth[1]+2);

	maxwidth[2]=fm.width(i18n("FS Type"));
	lbox->setColumn(2, i18n("FS Type"), maxwidth[2]+2);

	maxwidth[3]=fm.width(i18n("Mount Options"));
	lbox->setColumn(3, i18n("Mount Options"), maxwidth[3]+2);


	while ((fstab_ent=getfsent())!=NULL) {
		if (fm.width(fstab_ent->fs_spec) > maxwidth[0]) {
			maxwidth[0]=fm.width(fstab_ent->fs_spec);
			lbox->setColumnWidth(0, maxwidth[0]+10);
		}

		if (fm.width(fstab_ent->fs_file) > maxwidth[1]) {
			maxwidth[1]=fm.width(fstab_ent->fs_file);
			lbox->setColumnWidth(1, maxwidth[1]+10);
		}

		if (fm.width(fstab_ent->fs_vfstype) > maxwidth[2]) {
			maxwidth[2]=fm.width(fstab_ent->fs_vfstype);
			lbox->setColumnWidth(2, maxwidth[2]+10);
		}

		if (fm.width(fstab_ent->fs_mntops) > maxwidth[3]) {
			maxwidth[3]=fm.width(fstab_ent->fs_mntops);
			lbox->setColumnWidth(3, maxwidth[3]+10);
		}

		s.sprintf("%s;%s;%s;%s", fstab_ent->fs_spec, fstab_ent->fs_file, fstab_ent->fs_vfstype, fstab_ent->fs_mntops);
		lbox->insertItem(s);
	}

	endfsent(); /* Close fstab */
	return true;
}

bool GetInfo_XServer_and_Video (KTabListBox *lBox)
{
	return GetInfo_XServer_Generic( lBox );
}
