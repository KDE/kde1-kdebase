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
       

bool GetInfo_CPU( KTabListBox *lBox )
{	lBox = lBox;	/* don't give compiler-warning */
	return FALSE;
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

