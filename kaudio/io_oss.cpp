#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <iostream>
#include <errno.h>

#ifdef ENABLE_PULSE
#include <pulse/simple.h>

#else// ENABLE_PULSE

// Linux/OSS includes
#ifdef linux
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/soundcard.h>  // Here UNIX_SOUND_SYSTEM gets defined
#define OSS_AUDIO
#endif

// FreeBSD includes
#ifdef __FreeBSD__
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <machine/soundcard.h>  // Here UNIX_SOUND_SYSTEM gets defined
#define OSS_AUDIO
#endif

// NetBSD includes
#ifdef __NetBSD__
#include <fcntl.h>
#include "sys/ioctl.h"
#include <sys/types.h>
#include <soundcard.h>
#define OSS_AUDIO
#endif

// UnixWare includes
#ifdef _UNIXWARE
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/soundcard.h>
#define OSS_AUDIO
#endif

#endif// ENABLE_PULSE

#include "maudio.h"
#include "sample.h"
#include "io_oss.h"

extern int          BUFFSIZE;   // Crap! Must be out into AudioSample class



AudioDev::AudioDev(int devnum)
{
  opened=false;
  char textDevDSP[]="/dev/dsp";
  int  lenDevDSP = strlen(textDevDSP);

  bit_p_spl = 0; // This forces setting parameter new in "grab"

  if ( devnum > 10 || devnum<=0 ) {
    // Illegal or default device number: Use default DSP
    devname = new char[lenDevDSP+1];
    strcpy(devname,textDevDSP);  // Yields "/dev/dsp"
  }
  else {
    // Yields "/dev/dspX", with X in [0..9].
    devname = new char[lenDevDSP+2];
    strcpy(devname,textDevDSP);
    devname[lenDevDSP] = char('0'+(unsigned char)devnum -1);
    devname[lenDevDSP+1] = 0; // 0-termination of string
  }

 
  // Adding a quick hack. Trying out what happens if I feed OSS with
  // Zero-Data when idle. Is OSS perhaps happy then?!?
  silence8 = new char[BUFFER_MAX];
  silence16= new char[BUFFER_MAX];
  for (int i=0; i<BUFFER_MAX; i+=2) {
    silence8[i] = 0x80; silence8[i+1] = 0x80;   // 0x80
    silence16[i]= 0x00; silence16[i+1]= 0x00;   // 0x00
  }

#ifdef ENABLE_PULSE
  pa_conn = NULL;
#endif
}

void AudioDev::setBugs(int bugs)
{
  BugFlags = bugs;
}


bool AudioDev::grab(bool probeOnly)
{
  if (ParamsChanged) {
    // When playback parameters are changed with setParams(), we have to
    // re-open the device.
    release();
  }
    
  if (!opened) {
      /* Open audio device non-blocking! */
#ifdef OSS_AUDIO
      audiodev=open(devname, O_WRONLY, 0 /* O_NONBLOCK */);
#elif defined(ENABLE_PULSE)
      if (probeOnly) {
          return true; // so sue me
      }

      pa_sample_spec ss;
      ss.rate = frequency;
      ss.channels = stereo ? 2 : 1;
      switch(bit_p_spl) {
      case 32:
          ss.format = PA_SAMPLE_S32LE;
          break;
      case 24:
          ss.format = PA_SAMPLE_S24_32LE;
          break;
      case 16:
          ss.format = PA_SAMPLE_S16LE;
          break;
      case 8:
          ss.format = PA_SAMPLE_U8;
          break;
      default:
          std::cerr << "unhandled bpp " << bit_p_spl << std::endl;
          ss.format = PA_SAMPLE_U8;
          break;
      }

      pa_conn = pa_simple_new(
                  NULL,               // Use the default server.
                  "MAudio",           // Our application's name.
                  PA_STREAM_PLAYBACK,
                  NULL,               // Use the default device.
                  "Notications",      // Description of our stream.
                  &ss,                // Our sample format.
                  NULL,               // Use default channel map
                  NULL,               // Use default buffering attributes.
                  NULL                // Ignore error code.
                  );
       ParamsChanged=false;
       opened = true;

#else
      audiodev=-1; // fail!
#endif 
     if (audiodev == -1 ) {
       if ( errno != EBUSY ) {
	 // Tell the user something has gone wrong.
	 // But don't tell him the device is busy, he already
	 // should know this.
           std::cerr << "maudio: Cannot open audio device.\n";
         return false;
       }
       else {
	 // errno == EBUSY
	 if ( probeOnly )
	   return true; // OK, the device *could* be opened
	 else
	   return false; // No, the device can not be opened NOW
       }
     }

     // We only get here, if the device If the device could NOT be opened

     if (probeOnly) {
       // If we only wanted to probe for the device, close it and return here
       close(audiodev);
       return true;
     }
     else {
       /*
	* Now set sample format, then channels, then speed. It is important to follow this
	* scheme. See OSS documentation for more info.
	*/
#ifdef OSS_AUDIO
       
       // Set fragments
       int arg;
       if (bit_p_spl==8)
	 arg = 0x00100000; // 0xMMMMSSSS;
       else
	 arg = 0x00200000; //

       // now calc count=ld(BUFFSIZE);
       int tmp   = BUFFSIZE;
       int count = 0;
       while (tmp>1) {
	 tmp /=2;
	 count++;
       }
       arg |= count;

       if (ioctl(audiodev, SNDCTL_DSP_SETFRAGMENT, &arg)) {
	 release();
	 return false;
       }

       int Param;  /* For calling ioctl() */
       Param= bit_p_spl ; ioctl(audiodev, SNDCTL_DSP_SAMPLESIZE , &Param);
       Param= stereo    ; ioctl(audiodev, SNDCTL_DSP_STEREO     , &Param);
       Param= frequency ; ioctl(audiodev, SNDCTL_DSP_SPEED      , &Param);
#endif
       ParamsChanged=false;
       opened = true;
     }
  }
  return opened;
}

bool AudioDev::release()
{
#ifdef ENABLE_PULSE
  if (pa_conn) pa_simple_free(pa_conn);
  pa_conn = NULL;
#else
  if (opened) {
//  if ( BugFlags & CLOSELOOSE_BUG)
//  Loose data on close? Then sync before close

    sync(); // Always sync() on close. I am fed up with OSS-Bug compatibility mode :-(
    close(audiodev);
  }
#endif
  opened=false;

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
      sync();
      return( ioctl(audiodev, SNDCTL_DSP_RESET, 0) );
#elif defined(ENABLE_PULSE)
    return pa_simple_flush(pa_conn, NULL) == 0;
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
#elif defined(ENABLE_PULSE)
      return pa_simple_drain(pa_conn, NULL) == 0;
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
  // See if any params were changed since the last call. If yes, then we have
  // to setup the sound device in AudioDev::grab()
  ParamsChanged = (    (bit_p_spl != Bit_p_spl)
		    || (stereo    != Stereo   )
		    || (frequency != Freq     )
		    || (channels  != Channels ));

  bit_p_spl = Bit_p_spl;
  stereo    = Stereo;
  frequency = Freq;
  channels  = Channels;
}

int AudioDev::Write(char *data, uint32 num)
{
#ifdef OSS_AUDIO
  return ( write(audiodev, data, num) );
#elif defined(ENABLE_PULSE)
  return ( pa_simple_write(pa_conn, data, num, &errno) );
#else
  char *dummy = data;  // remove warnings on unsupported systems
  data = dummy;        // remove warnings on unsupported systems
  return 1;
#endif
}

int AudioDev::emitSilence()
{
  if (!opened) {
    return 0;
  }

#ifdef OSS_AUDIO
  audio_buf_info ABI;
  ioctl(audiodev,SNDCTL_DSP_GETOSPACE, &ABI);
  int bytesLeft = ABI.bytes;
  int emittedBytes = 0;
  while (bytesLeft >= BUFFSIZE)
    {
      bytesLeft -= BUFFSIZE;
      // Emit silence, if there may be sound underrun
#ifdef DEBUG
      cerr  << "FreeFrags =" << ABI.fragments << '\n';
      cerr  << "Bytes left=" << bytesLeft << '\n';
#endif
      if (bit_p_spl == 8)
	emittedBytes += write(audiodev, silence8, BUFFSIZE );
      else
	emittedBytes += write(audiodev, silence16, BUFFSIZE );
    }
  return emittedBytes;
#else
  return 0;
#endif
}
