
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <config.h>

extern "C" {
#include <mediatool.h>
}

#include "maudio.h"
#include "io_oss.h"
#include "sample.h"
#include "version.h"


#define USLEEP_DELAY	1000


uint32		*StatStatPtr=NULL;
MdCh_STAT	*StatChunk;
MdCh_FNAM	*FnamChunk;
MdCh_KEYS	*KeysChunk;
MediaCon		mcon;
char		IsSlave=0,SoftStop=0;

AudioSample	*ASample;
AudioDev	*ADev;

int		BUFFSIZE;
int		MaudioDevnum=0;

int		secsLength, secsCurrent, bytes_per_s;

enum {PLAYING, PLAY_IT, STOPPED, PAUSE, PAUSING, START_MEDIA, STOP_MEDIA};
char PlayerStatus=STOPPED;

void ma_init(char argc, char **argv);
void ma_atexit(void);
void preWrite(void);


void MYexit(int retcode)
{
  exit (retcode);
}


int main(int argc, char **argv)
{
  char	filename[LEN_FNAME+1];
  int	bytes_read, ret;
  int   preWriting=0,preReading=0;

  static char* WBold=NULL;


  char	*PtrFname;
  int	ReleaseDelay=1;
  bool	retgrab;

  ma_init(argc, argv);

  // create a new Audio device for device number "MaudioDevnum"
  // This device number must be mapped in the class to a
  // physcial audio device, e.g. /dev/audio , /dev/dsp1 or
  // any other appropiate representation.
  ADev = new AudioDev(MaudioDevnum);
  ASample = new AudioSample();


  // Probe, if device can be grabbed (test)
  bool openSuccess = ADev->grab(true);
  ADev->release();
  if (!openSuccess)
    goto exit_pos;


  // Following is the communication via mediatool.
  // I will write an encapsulation class for this soon,
  // so that it won't look so ugly anymore
  while(1) {
    StatChunk->pos_max	= ASample->duration();
    StatChunk->pos_current	= ASample->playpos();

    KeysChunk->is_alive = 1; // Give a sign, maudio is still alive

    if ( EventCounterRead(&(KeysChunk->exit), 0) ) {
      // Terminate, when Exit pressed
      goto exit_pos;
    }
    if ( EventCounterRead(&(KeysChunk->posnew), 0) ) {
      //Reposition audio stream
      ASample->seek(KeysChunk->pos_new,0);
    }
    if ( EventCounterRead(&(KeysChunk->forward), 0) ) {
      //Reposition audio stream
      ASample->seek(ASample->playpos()+1,0);
    }
    if ( EventCounterRead(&(KeysChunk->backward), 0) ) {
      //Reposition audio stream
      uint32 cur=ASample->playpos(); if (cur!=0) cur--;
      ASample->seek(cur,0);
    }
    if ( EventCounterRead(&(KeysChunk->stop), 0) ) {
      // Terminate playing, when Stop pressed
      PlayerStatus = STOP_MEDIA;
      SoftStop = 0;
    }
    if ( EventCounterRead(&(KeysChunk->play), 0) ) {
      // Start, when Start pressed
      if (PlayerStatus==STOPPED)
	// If we were stopped, make a full (re-)start
	PlayerStatus = START_MEDIA;
      else
	PlayerStatus = PLAYING;
      SoftStop = 0;
    }
    if ( FileNameGet(FnamChunk, filename) ) {
      PlayerStatus=START_MEDIA;
    }

    if ( (PlayerStatus!=PLAYING) &&  (PlayerStatus!=PLAY_IT) ) {
      // Send some zero data, to (e.g.) fill up the kernel buffer
      ADev->emitSilence();
    }


    // ----------------------------------------------------------------------
    switch(PlayerStatus) {
    case STOPPED:
      *StatStatPtr = MD_STAT_READY;
      if (! FileNameGet(FnamChunk, filename) ) {
	if (ReleaseDelay!=0) {
	  // release audio device, if unused for some time
	  ReleaseDelay--;
	  if (ReleaseDelay==0) {
#ifdef DEBUG
	    std::cerr << "maudio: Releasing audio device\n";
#endif
	    ADev->release();
	  }
	}
	usleep(USLEEP_DELAY/10);
      }
      else
	PlayerStatus=START_MEDIA;
      break;
	  

      // -------------------------------------------------------------------
    case STOP_MEDIA:
      /* ---------------------------------------------------------------------
	 This is the right way! Never sync or reset while the device is
	 open. Flushing is now solved by emitting "Zero Data". This is the
	 way to go for maudio II anyhow.
	 ------------------------------------------------------------------ */

      // Reposition stream at start
      ASample->seek(0,0);

      SoftStop=0;
      ReleaseDelay=18; // Delay until releasing audio (about 2 sec)

      *StatStatPtr = MD_STAT_READY;
      PlayerStatus = STOPPED;
      // Give KAudio feedback on "finished". This crappy synchronization
      // thing REALLY must be replaced by some real protocol.
      StatChunk->sync_id = KeysChunk->sync_id;
      break;

      // ---------------------------------------------------------------------
    case PAUSE:
      ADev->pause();
      PlayerStatus = PAUSING;
      *StatStatPtr = MD_STAT_PAUSING;
      break;


      // ---------------------------------------------------------------------
    case PAUSING:
      if ( KeysChunk->pause )
	usleep(USLEEP_DELAY/10);
      else
	PlayerStatus = PLAYING;
      break;
	  

      // ---------------------------------------------------------------------
    case START_MEDIA:
      *StatStatPtr = MD_STAT_BUSY;
      SoftStop=0;
      WBold = 0;
      PtrFname = strrchr(filename, '/');
      if ( PtrFname == NULL )
	PtrFname = filename;
      else
	// Skip '/' character
	PtrFname++;
      strncpy(StatChunk->songname, PtrFname, LEN_NAME);
      if ( ASample->setFilename(filename))
	/* Probing said: Illegal media */
	PlayerStatus = STOP_MEDIA;
      else {
	ADev->setParams(ASample->bit_p_spl, \
			ASample->stereo,    \
			ASample->frequency, \
			ASample->channels);
	// Must grab *after* probing media, so that frequency etc are set
	retgrab = ADev->grab();
	if (retgrab != true) {
	  PlayerStatus = STOP_MEDIA;
#ifdef DEBUG
	  std::cerr << "maudio: Failed to grab sound device\n";
#endif
	}
	else {
	  PlayerStatus = PLAYING;
	  // remember pre-reading and pre-writing some data
	  preWriting = preReading = NUM_BUF-2;
	}
      }
      break;


      // ---------------------------------------------------------------------
    case PLAYING:
      *StatStatPtr = MD_STAT_PLAYING;
      /*
       * Big Loop: Each time this loop is called, I read another
       * chunk of audio data from the file.
       */
      bytes_read = ASample->readData();
      if (bytes_read!=0 && preReading!=0) {
	preReading--;
      }
      else
	PlayerStatus = PLAY_IT;
      break;


      // ---------------------------------------------------------------------
    case PLAY_IT:
      /*
       * Small loop: This loop is called as often as necessary. Usually,
       * this is called one time. But if the audio device is busy (EAGAIN),
       * I have to retry.
       */
      if ( (ASample->buffersValid == 0) /*&& (bytes_read == 0) */ ) {
        // Nothing to read and to write. OK,then finish
        SoftStop = 1;
        PlayerStatus = STOP_MEDIA;
        break;
      }


      if (WBold == ASample->WBuffer)
	std::cerr << "maudio: warning (please ignore)\n";

      ret = ADev->Write(ASample->WBuffer,BUFFSIZE);
      WBold = ASample->WBuffer;

      if (ret == -1 ) {
	if (errno== EAGAIN) {
	  /* Retry after delay */
	  usleep(USLEEP_DELAY);
	}
	else {
	  std::cerr << "maudio OSS Error: " << errno << "\n";
	}
      } // EAGAIN
      else {
	ASample->nextWBuf();

	if ( preWriting!=0) {
	  // pre-write data into device, so that it is always a little ahead
	  // and slow computers dont experience sound hickups
	  // Pre-writing is done every time after the sound device got opened.
	  preWriting--;
	}
	else {
	  // Allow Pause not in pre-writing phase
	  if (KeysChunk->pause)
	    PlayerStatus = PAUSE;
	  else
	    PlayerStatus = PLAYING;
	}
      }
      break;


      // ---------------------------------------------------------------------
    default:
      std::cerr << "maudio: Undefined state, resetting";
      PlayerStatus = STOP_MEDIA;
      break;
    }
  }

 exit_pos:
  ADev->reset();
  ADev->release();
  return(0);
}



// Write data, retrying until it gets accepted
void preWrite(void)
{
  int ret;

  while (1)
  {
    ret = ADev->Write(ASample->WBuffer,BUFFSIZE);
    if (ret == -1 && errno == EAGAIN)
      usleep(USLEEP_DELAY); /* Retry after delay */
    else
      break;
  }

  if (ret!=-1)
    ASample->nextWBuf(); // If buffer was written
}





void ma_init(char argc, char **argv)
{
  char          *identification=NULL;
  int           i;

  for (i=0; i<argc; i++) {
    /* Parse parameters */
    if ( (strcmp (argv[i],"-media") == 0) && (i<argc-1) ) {
      /* argv[i]="-media" and there exists at least one more
       * parameter (i<argc-1).
       */
      IsSlave=1;
      identification = argv[i+1];
    }
    else if ( (strcmp (argv[i],"-devnum") == 0) && (i+1<argc) ) {
      MaudioDevnum = atoi(argv[i+1]);
    }
    else  if (strcmp (argv[i],"-version") == 0) {
      char vers[50];
      sprintf (vers,"%.2f", APP_VERSION);
      std::cout << argv[0] << " V" << vers << ".\n(C)1997-1998 by Christian Esken (esken@kde.org).\n";
      std::cout << "This program can be distributed under the terms of GPL\n";
      exit(0);
    }
  }

  if (!IsSlave) {
    std::cerr << "maudio: -media option missing.\n";
    MYexit(1);
  }


  MdConnect(atoi(identification), &mcon);
  if ( mcon.shm_adr == 0 ) {
    std::cerr << "Could not find media master.\n";
    MYexit(1);
  }

  StatChunk = (MdCh_STAT*)FindChunkData(mcon.shm_adr, "STAT");
  if ( StatChunk == NULL ) {
    std::cerr << "Could not find STAT chunk.\n";
    MYexit(1);
  }
  StatStatPtr = &(StatChunk->status);

#if ! ( defined (linux) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (_UNIXWARE) )
  // Linux, FreeBSD, NetBSD and UnixWare are supported today. Quit now for
  // not wasting resources.
  // kaudioserver will get a SIGCHLD and exit, too, which is good.
  *StatStatPtr =  MD_STAT_EXITED;
  MYexit(1);
#endif

  KeysChunk = (MdCh_KEYS*)FindChunkData(mcon.shm_adr, "KEYS");
  if ( KeysChunk == NULL ) {
    std::cerr << "Could not find KEYS chunk.\n";
    *StatStatPtr = MD_STAT_EXITED;
    /* Master will not lock up, because I can tell him, the player
     * exits right now.
     */
    MYexit(1);
  }

  FnamChunk = (MdCh_FNAM*)FindChunkData(mcon.shm_adr, "FNAM");
  if ( FnamChunk == NULL ) {
    std::cerr << "Could not find FNAM chunk.\n";
    *StatStatPtr = MD_STAT_EXITED;
    /* Master will not lock up, because I can tell him, the player
     * exits right now.
     */
    MYexit(1);
  }


  *StatStatPtr = MD_STAT_INIT;
  // Do now various stuff, as adding new chunks.

  StatChunk->supp_keys = \
    MD_KEY_FORWARD \
    | MD_KEY_BACKWARD \
    | MD_KEY_PLAY \
    | MD_KEY_STOP \
    | MD_KEY_PAUSE \
    | MD_KEY_EXIT;


  /* Setup cleanup function */
  atexit(ma_atexit);
  /* Do now various stuff, as adding new chunks. */

 
  // Set status to "Player ready". After this line, I may not add new chunks.
  *StatStatPtr = MD_STAT_READY;
  return;
}


void ma_atexit(void)
{
  /* Notify mediatool of exiting slave */
  if (IsSlave) {
    if (KeysChunk)
      KeysChunk->is_alive = 0;
    if(StatStatPtr)
      *StatStatPtr =  MD_STAT_EXITED;
  }
  delete ASample;
}

