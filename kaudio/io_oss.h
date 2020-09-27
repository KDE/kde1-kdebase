// -*-C++-*-
#ifndef IO_OSS_H
#define IO_OSS_H

#include "sample.h"

#ifdef ENABLE_PULSE
struct pa_simple;
#endif

class AudioDev
{
public:
  enum {POST_BUG=1, CLOSELOOSE_BUG=2}; // next would be 4,8,16,...
  AudioDev(int devnum);
  /* channels not supported by now */
  void setParams(int8 Bit_p_spl, bool Stereo, uint32 Freq, int8 Channels);
  bool grab(bool probeOnly=false);
  bool release();
  bool reset();
  bool sync();
  bool pause();
  void setBugs(int bugs);
  int  Write(char *data, uint32 num);
  int emitSilence();
private:
  int   audiodev;	/* Should be private, cannot be for now :- */
  char	*devname;
  bool	opened;
  int	bit_p_spl;
  bool	stereo;
  uint32 frequency;
  int8	channels;	/* Not supported by now */
  int	BugFlags;	/* Yeah, I like OSS and its thousand bugs :-( */
  char  *silence8;      /* Memory buffer where "silence" is stored */
  char  *silence16;     /* Memory buffer where "silence" is stored */
  bool  ParamsChanged;

#ifdef ENABLE_PULSE
  pa_simple *pa_conn;
#endif
};


#endif /* IO_OSS_H */


