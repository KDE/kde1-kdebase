// -*-C++-*-

#ifndef SAMPLE_H
#define SAMPLE_H

#include <stdio.h>
#include <mediatool.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define MAUDIO_MAX_FRAGS 32
#define BUFFER_MAX       4096

class AudioSample
{
public:
  enum {PCM_WAV, ADPCM_WAV, SUNAU, IFF};

  AudioSample();
  ~AudioSample();
  uint32 playpos();
  uint32 duration();
  void seek(uint32 secs, uint32 msecs);
  int  setFilename(char* fname);
  int  readData();

  char		*Data[MAUDIO_MAX_FRAGS];

  /* Format of audio file */
  char		filetype;	// AU, WAV, IFF, ...

  bool		stereo;
  int8		bit_p_spl;	// bits per sample, 8, 12, 16
  uint32	frequency;
  int8		channels;
  uint32	bytes;

  uint32	BuferValidLength;
  char		Buffer[BUFFER_MAX];    // !!! Will never user bigger buffers

private:

  FILE		*audiofile;
  int8		num_frags;
  int8		cur_write_frag;
  int8		cur_read_frag;
  uint32	headerLen;

  uint32	bytes_per_s;
  unsigned long	MediaLength;	// in bytes
  uint32	Duration;	// in secs
  char		*Filename;
  bool		opened;
};


#endif // SAMPLE_H
