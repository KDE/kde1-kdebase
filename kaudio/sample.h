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

const         int NUM_BUF=8;
 
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
  /** Read data from file into one of the buffers. This function
      implements the multi-buffering technique and calls readDataI()
      for the actual reading */
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
  /// [RW]Buffer is a pointer to Buffers[n] , where n is in 0...NUM_BUF-1
  char		*RBuffer,*WBuffer;
  /// Buffers for the multi buffering technique
  char		Buffers[NUM_BUF][BUFFER_MAX]; // !!! Will never user bigger buffers
  /// num of validit (= written) buffers
  int		buffersValid;
  int		RBufId,WBufId;
  void nextWBuf();

private:
  /// Internal readData function. This reads into current Buffer
  int  readDataI();
  void setRBuf(int id);
  void setWBuf(int id);
  /// logically clears the multi buffers
  void clearBuffers();
  
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
