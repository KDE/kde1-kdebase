#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream.h>
#include <string.h>
#include <fcntl.h>
#include <kmisc.h>

extern "C" {
#include <mediatool.h>
}

#include "maudio.h"
#include "io_oss.h"
#include "sample.h"


#define USLEEP_DELAY	10000


uint32          *StatStatPtr=NULL;
MdCh_STAT       *StatChunk;
MdCh_FNAM       *FnamChunk;
MdCh_KEYS       *KeysChunk;
MediaCon        mcon;
char		IsSlave=0,SoftStop=0;

AudioSample	*ASample;
AudioDev	*ADev;


/* If you encounter looped output when pausing, it is a bug
 * in OSS. Call maudio with "-oss_bugs 1" then.
 */

int		secsLength, secsCurrent, bytes_per_s;

enum {PLAYING, PLAY_IT, STOPPED, PAUSE, PAUSING, START_MEDIA, STOP_MEDIA};

char PlayerStatus=STOPPED;

void ma_init(char argc, char **argv);
void ma_atexit(void);

int bugflags=255;

int main(char argc, char **argv)
{
  char	filename[LEN_FNAME+1];
  int	bytes_read, ret, ret2;


  char  textDevDSP[]="/dev/dsp";
  char  *PtrFname;
  int	ReleaseDelay=1;
  bool	retgrab;

  ma_init(argc, argv);

  ADev = new AudioDev(textDevDSP, O_WRONLY, 0 /* O_NONBLOCK */);
  ADev->setBugs(bugflags);	// OSS bug workaround flags :-(
  ASample = new AudioSample();

  while(1)
    {
      StatChunk->pos_max	= ASample->duration();
      StatChunk->pos_current	= ASample->playpos();

      if ( EventCounterRead(&(KeysChunk->exit), 0) )
	{ /* Terminate, when Exit pressed */
	  goto exit_pos;
	}
      if ( EventCounterRead(&(KeysChunk->stop), 0) )
	{ /* Terminate playing, when Stop pressed */
#ifdef DEBUG
	  cerr << "Got STOP";
#endif
	  PlayerStatus = STOP_MEDIA;
	  SoftStop = 0;
	}
      if ( EventCounterRead(&(KeysChunk->play), 0) )
	{ /* Start, when Start pressed */
	  if (PlayerStatus==STOPPED)
	    // If we were stopped, make a full (re-)start
	    PlayerStatus = START_MEDIA;
	  else
	    PlayerStatus = PLAYING;
	  SoftStop = 0;
	}
      if ( FileNameGet(FnamChunk, filename) )
	{
	  PlayerStatus=START_MEDIA;
	}

      // ----------------------------------------------------------------------------- //
      switch(PlayerStatus)
	{
	case STOPPED:

	  *StatStatPtr = MD_STAT_READY;
//	  StatChunk->sync_id = KeysChunk->sync_id; // !!! Argh!
	  if (! FileNameGet(FnamChunk, filename) )
	    {
	      if (ReleaseDelay!=0)
		{
		  // release audio device, if unused for some time
		  ReleaseDelay--;
		  if (ReleaseDelay==0)
		    {
#ifdef DEBUG
		      cerr << "maudio: Releasing audio device\n";
#endif
		      ADev->release();
		    }
		  else
		   { // Send some zero data, to keep (especially) OSS happy
		      ADev->emitSilence();
		   }
		}
		usleep(10*USLEEP_DELAY);
	    }
	  else
	    PlayerStatus=START_MEDIA;
	  break;
	  

	  // ------------------------------------------------------------------------- //
	case STOP_MEDIA:
#ifdef DEBUG
	  cerr << "Stopping Media ... 1";
#endif

	  if(SoftStop)
	    true;//ADev->sync();  // TODO: Sync nonblocking ... or flush *somehow* :-(
	  else
	    true;
	  //ADev->reset();

	  /* Reposition stream at start */
	  ASample->seek(0,0);

	  SoftStop=0;
	  ReleaseDelay=8; // Delay until releasing audio (about 0,8 sec)
#ifdef DEBUG
	  cerr << " 2";
#endif

	  *StatStatPtr = MD_STAT_READY;
	  PlayerStatus = STOPPED;
         // Give KAudio feedback on "finished". This crappy synchroization
          // thing REALLY must be replaced by some real protocol.
          StatChunk->sync_id = KeysChunk->sync_id;

#ifdef DEBUG
	  cerr << " OK\n";
#endif
	  break;

	  // ------------------------------------------------------------------------- //
	case PAUSE:
 	  ret2=ADev->pause();
#ifdef DEBUG
	  cerr << "Pausing, ioctl() says: " << ret2 << '\n';
#endif
	  PlayerStatus = PAUSING;
	  *StatStatPtr = MD_STAT_PAUSING;
	  break;


	  // ------------------------------------------------------------------------- //
	case PAUSING:
	  if ( KeysChunk->pause )
	    usleep(USLEEP_DELAY);
	  else
	    PlayerStatus = PLAYING;
	  break;
	  

	  // ------------------------------------------------------------------------- //
	case START_MEDIA:
#ifdef DEBUG
	  cerr << "Preparing filename" << filename <<  "1";
#endif
	  *StatStatPtr = MD_STAT_BUSY;
	  SoftStop=0;
	  PtrFname = strrchr(filename, '/');
	  if ( PtrFname == NULL )
	    PtrFname = filename;
	  else
	    // Skip '/' character
	    PtrFname++;
	  strncpy(StatChunk->songname, PtrFname, LEN_NAME);
	  //!!!ADev->reset();
#ifdef DEBUG
	  cerr <<" 2\n";
#endif

	  if ( ASample->setFilename(filename))
	    /* Probing said: Illegal media */
	    PlayerStatus = STOP_MEDIA;
	  else
	    {
	      ADev->setParams(ASample->bit_p_spl, \
			      ASample->stereo, \
			      ASample->frequency, \
			      ASample->channels);
	      // Must grab *after* probing media, so that frequency etc are set
	      retgrab = ADev->grab();
	      if (retgrab != true)
		{
		  PlayerStatus = STOP_MEDIA;
		  cerr << "Failed to grab\n";
		}
	      else
		PlayerStatus = PLAYING;
	    }
#ifdef DEBUG
	  cerr << "Preparing finished.\n";
#endif
	  break;


	  // ------------------------------------------------------------------------- //
	case PLAYING:

	  *StatStatPtr = MD_STAT_PLAYING;
	  /*
	   * Big Loop: Each time this loop is called, I read another
	   * chunk of audio data from the file.
	   */
	  bytes_read = ASample->readData();

	  if (bytes_read == 0)
	    {
	      SoftStop = 1;
	      PlayerStatus = STOP_MEDIA;
	    }
	  else
	    PlayerStatus = PLAY_IT;
	  break;


	  // ------------------------------------------------------------------------- //
	case PLAY_IT:
	  /*
	   * Small loop: This loop is called as often as necessary. Usually,
	   * this is called one time. But if the audio device is busy (EAGAIN),
	   * I have to retry.
	   */
	  if (KeysChunk->pause)
	    {
	      PlayerStatus = PAUSE;
	      goto break_pos;
	    }
	  ret = ADev->Write(ASample);

	  if (ret == -1 )
	    {
	      if (errno== EAGAIN)
		{
		  /* Retry after delay */
		  usleep(USLEEP_DELAY);
#ifdef DEBUG
		  cerr << "write() == EAGAIN\n";
#endif
		}
#ifdef DEBUG
	      else
		cerr << "write() == " << errno << '\n';
#endif
	    }
	  else
	    PlayerStatus = PLAYING;

	    break_pos:
	  break;


	  // ------------------------------------------------------------------------- //
	default:
	  cerr << "maudio: Undefined state, resetting";
	  PlayerStatus = STOP_MEDIA;
	  break;
	}
    }

exit_pos:
  ADev->reset();
  ADev->release();
  return(0);
}



void ma_init(char argc, char **argv)
{
  char          *identification=NULL;
  int           i;

  for (i=0; i<argc; i++)
    {
      /* Parse parameters */
      if ( (strcmp (argv[i],"-media") == 0) && (i<argc-1) )
	{
	  /* argv[i]="-media" and there exists at least one more
	   * parameter (i<argc-1).
	   */
	  IsSlave=1;
	  identification = argv[i+1];
	}
      else if ( (strcmp (argv[i],"-oss_bugs") == 0) && (i+1<argc) )
	{
	  bugflags = atoi(argv[i+1]);
	  cerr << "Playing with OSS Bug workaround! Flags= " << bugflags << '\n';
	}

    }

  if (!IsSlave)
    {
      cerr << "maudio: -media option missing.\n";
      exit(1);
    }


  MdConnect(atoi(identification), &mcon);
  if ( mcon.shm_adr == 0 )
    {
      cerr << "Could not find media master.\n";
      exit(1);
    }

  StatChunk = (MdCh_STAT*)FindChunkData(mcon.shm_adr, "STAT");
  if ( StatChunk == NULL )
    {
      cerr << "Could not find STAT chunk.\n";
      exit(1);
    }
  StatStatPtr = &(StatChunk->status);

#ifndef linux
  // Only Linux/OSS is supported today. Quit now for not wasting resources.
  // kaudioserver will get a SIGCHLD and exit, too, which is good.
  *StatStatPtr =  MD_STAT_EXITED;
  exit(1);
#endif

  KeysChunk = (MdCh_KEYS*)FindChunkData(mcon.shm_adr, "KEYS");
  if ( KeysChunk == NULL )
    {
      cerr << "Could not find KEYS chunk.\n";
      *StatStatPtr = MD_STAT_EXITED;
      /* Master will not lock up, because I can tell him, the player
       * exits right now.
       */
      exit(1);
    }

  FnamChunk = (MdCh_FNAM*)FindChunkData(mcon.shm_adr, "FNAM");
  if ( FnamChunk == NULL )
    {
      cerr << "Could not find FNAM chunk.\n";
      *StatStatPtr = MD_STAT_EXITED;
      /* Master will not lock up, because I can tell him, the player
       * exits right now.
       */
      exit(1);
    }


  *StatStatPtr = MD_STAT_INIT;
  /* Do now various stuff, as adding new chunks. */

  StatChunk->supp_keys = \
/*    | MD_KEY_FORWARD \
 *    | MD_KEY_BACKWARD \
 */
      MD_KEY_PLAY \
    | MD_KEY_STOP \
    | MD_KEY_PAUSE \
    | MD_KEY_EXIT;


  /* Setup cleanup function */
  atexit(ma_atexit);
  /* Do now various stuff, as adding new chunks. */

 
  /*
   * Set status to "Player ready". After this line, I may not add new chunks.
   */
  *StatStatPtr = MD_STAT_READY;
  return;
}


void ma_atexit(void)
{
  /* Notify mediatool of exiting slave */
  if (IsSlave)
    {
      if(StatStatPtr)
	*StatStatPtr =  MD_STAT_EXITED;
    }
  delete ASample;
}

