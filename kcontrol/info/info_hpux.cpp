/* 	info_hpux.cpp
	
	!!!!! this file will be included by info.cpp !!!!!
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

#include <unistd.h>
#include <sys/param.h>
#include <sys/pstat.h>

bool GetInfo_CPU( KTabListBox *lBox )
{
  long page_size;
  struct pst_dynamic psd;
  struct pst_static  pst;
  QString str;
  char buf[256];
  
  if( pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1 )
    perror("pstat_getstatic");
  page_size =  pst.page_size;

  if( pstat_getdynamic(&psd, sizeof(pst), (size_t)1, 0) == -1 )
    perror("pstat_getstatic");
  
  switch( sysconf(_SC_CPU_VERSION) )
    {
    case CPU_PA_RISC1_0:
      str = "CPU_PA_RISC1_0";
      break;
    case CPU_PA_RISC1_1:
      str = "CPU_PA_RISC1_1";
      break;
    case CPU_PA_RISC2_0:
      str = "CPU_PA_RISC2_0";
      break;
    default:
      str = i18n("unknown CPU type");
      break;
    }

  lBox->insertItem(str);
  
  str = "HOSTNAME= ";
  if( gethostname(buf,256) != 0 )
    str += i18n("unavailable");
  else
    str += buf;
  lBox->insertItem(str);

  lBox->insertItem(i18n("Number of active processor(s)= ") + psd.psd_proc_cnt);

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
{	lBox = lBox;
	return FALSE;
}

bool GetInfo_IO_Ports( KTabListBox *lBox )
{	lBox = lBox;
	return FALSE;
}

bool GetInfo_Sound( KTabListBox *lBox )
{	lBox = lBox;
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

