#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <iostream.h>

// Linux/OSS includes
#ifdef linux
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/soundcard.h>  // Here UNIX_SOUND_SYSTEM gets defined
#define OSS_AUDIO
#endif


#include "maudio.h"
#include "sample.h"
#include "io_oss.h"


AudioDev::AudioDev(char *dev, int mode, int options)
{
  opened=false;
  Mode=mode;
  Options=options;
  devname =strdup(dev);		// !!! Should use mystrdup()
}

void AudioDev::setBugs(int bugs)
{
  BugFlags = bugs;
}


bool AudioDev::grab()
{
  int	Param[2]={0,0};		/* For calling ioctl() */

  if (!opened)
    {
      /* Open audio device non-blocking! */
#ifdef OSS_AUDIO
      audiodev=open(devname, Mode, Options);
#else
      audiodev=-1; // fail!
#endif 
     if (audiodev == -1 )
	{
	  cerr << "maudio: Cannot open audio device.\n";
	  return false;
	}
      else
	opened = true;
    }
  /*
   * Now set sample format, then channels, then speed. It is important to follow this
   * scheme. See OSS documentation for more info.
   */
#ifdef OSS_AUDIO

  // Set fragments
//  int arg = 0x00080010; // 0xMMMMSSSS;
//  if (ioctl(audiodev, SNDCTL_DSP_SETFRAGMENT, &arg)) {
//    release();
//    return false;
//  }

  Param[0]= bit_p_spl ; ioctl(audiodev, SNDCTL_DSP_SAMPLESIZE , Param);
  Param[0]= stereo    ; ioctl(audiodev, SNDCTL_DSP_STEREO     , Param);
  Param[0]= frequency ; ioctl(audiodev, SNDCTL_DSP_SPEED      , Param);
#endif

  return true;
}

bool AudioDev::release()
{
  if (opened)
    {
//      if ( BugFlags & CLOSELOOSE_BUG)
	/* Loose data on close? Then sync before close */

      sync(); // Always sync() on close
      close(audiodev);
      opened=false;
    }

  return true;
}

bool AudioDev::reset()
{
  if (opened)
    {
#ifdef DEBUG
      cerr << "RESET!\n";
#endif

#ifdef OSS_AUDIO
//      sync(); // OSS is so buggy, I must sync before reset :-(
      return( ioctl(audiodev, SNDCTL_DSP_RESET, 0) );
#else
      return(true);
#endif
    }
  else
    return true;
}

bool AudioDev::sync()
{
  if (opened)
    {
#ifdef DEBUG
      cerr << "SYNC!\n";
#endif
#ifdef OSS_AUDIO
      return( ioctl(audiodev, SNDCTL_DSP_SYNC, 0) );
#else
      return(true);
#endif
    }
  else
    return true;
}

bool AudioDev::pause()
{
  if (opened) {

#ifdef OSS_AUDIO
    if( (BugFlags&POST_BUG) == true )
      // Do not use POST if POST is buggy
      return true;
    else
      return( ioctl(audiodev, SNDCTL_DSP_POST, 0) );
#else
    return true;
#endif
  }
  else
    return true;
}

void AudioDev::setParams(int8 Bit_p_spl, bool Stereo, uint32 Freq, int8 Channels)
{

  bit_p_spl = Bit_p_spl;
  stereo    = Stereo;
  frequency = Freq;
  channels  = Channels;
}

int AudioDev::Write(AudioSample *a)
{
#ifdef OSS_AUDIO
  return ( write(audiodev, a->Buffer, a->BuferValidLength) );
#else
  return true;
#endif
}
