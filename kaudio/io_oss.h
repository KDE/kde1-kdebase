// -*-C++-*-
#ifndef IO_OSS_H
#define IO_OSS_H

#include "sample.h"

class AudioDev
{
public:
  enum {POST_BUG=1, CLOSELOOSE_BUG=2}; // next would be 4,8,16,...
  AudioDev(char *dev, int mode, int options);
  /* channels not supported by now */
  void setParams(int8 Bit_p_spl, bool Stereo, uint32 Freq, int8 Channels);
  bool grab();
  bool release();
  bool reset();
  bool sync();
  bool pause();
  void setBugs(int bugs);
  int  Write(AudioSample *a);

private:
  int   audiodev;	/* Should be private, cannot be for now :- */
  char	*devname;
  bool	opened;
  int	Mode;
  int	Options;
  int	bit_p_spl;
  bool	stereo;
  uint32 frequency;
  int8	channels;	/* Not supported by now */
  int	BugFlags;	/* Yeah, I like OSS and its thousand bugs :-( */
};


#endif /* IO_OSS_H */


