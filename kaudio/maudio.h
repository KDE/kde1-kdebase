// -*-C++-*-

#ifndef _MAUDIO_H
#define _MAUDIO_H

#include <stdio.h>
#include <mediatool.h>


/* uint32 is supposed to be 32 Bit. This is fixed here. Original
 * wavplay1.0pl2 code used "u_long", which is 64 Bit on Alpha, and
 * thus wrong. !!! Should be corrected in wavplay
 */
typedef  struct
{	char    	RiffID [4] ;
	uint32    	RiffSize ;
	char    	WaveID [4] ;
	char    	FmtID  [4] ;
	uint32    	FmtSize ;
	uint16   	wFormatTag ;
	uint16   	nChannels ;
	uint32		nSamplesPerSec ;
	uint32		nAvgBytesPerSec ;
	uint16		nBlockAlign ;
	uint16		wBitsPerSample ;
	char		DataID [4] ;
	uint32		nDataBytes ;
} WAVE_HEADER ;


#endif /* _MAUDIO_H */
