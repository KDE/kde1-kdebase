/* 	info_generic.cpp
	1998 by Helge Deller (helge.deller@ruhr-uni-bochum.de)
	free source under GPL
	
	!!!!! this file will be included by info.cpp !!!!!
*/


// Default for unsupportet systems

// the following defines are not really ok here, but maybe we should show, what
// Information could be displayed here....

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
       

bool GetInfo_CPU( KTabListBox * )
{
	return FALSE;
}

bool GetInfo_IRQ( KTabListBox * )
{
	return FALSE;
}

bool GetInfo_DMA( KTabListBox * )
{
	return FALSE;
}

bool GetInfo_PCI( KTabListBox * )
{
	return FALSE;
}

bool GetInfo_IO_Ports( KTabListBox * )
{
	return FALSE;
}

bool GetInfo_Sound( KTabListBox * )
{
	return FALSE;
}

bool GetInfo_Devices( KTabListBox * )
{
	return FALSE;
}

bool GetInfo_SCSI( KTabListBox * )
{
	return FALSE;
}

bool GetInfo_Partitions( KTabListBox * )
{
	return FALSE;
}

bool GetInfo_XServer_and_Video( KTabListBox *lBox )
{
	return GetInfo_XServer_Generic( lBox );
}
