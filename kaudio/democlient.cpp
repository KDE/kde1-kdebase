/*
 * Demonstration on using the KAudio class. Playing digital audio made easy. :-)
 */

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <mediatool.h>
}
#include "kaudio.h"

//int main ( int argc, char *argv[] )
int main ( int, char** )
{
  bool          replayPossible=false;
  char		WAVname[256];

  /************* initialization ************************************/
  KAudio *KAS = new KAudio();
  if (KAS->serverStatus())
    {
      cerr << "Failed contacting audio server\n";
      exit (1);
    }

  /************* interactive loop **********************************/
  while(1)
    {
      /* Make string empty */
      WAVname[0]=0;
      /* Read string (filename) */
      cout << "Enter full path of WAV\nq for quit, s for stopping, r for replaying current Wav):\n";
      scanf("%s",WAVname);
      if (!strcmp(WAVname,"q"))
	{
	  KAS->stop(); // <-- Stop Wav, when we quit
	  break;
	}
      else
	if (!strcmp(WAVname,"s"))
	  KAS->stop();   // <-- Stop Wav
	else
	  if (!strcmp(WAVname,"r"))
	    {
	      if (replayPossible)
		KAS->play(); // <-- Restart last Wav
	    }
	  else
	    {
	      KAS->play(WAVname);
	      replayPossible = true;
	    }
    }

  return 0;
}
