// THIS IS DAMNED HACKED. PLEASE DO NOT LOOK AT THE CODE, PLEASE //

/*
   Copyright (c) 1997 Christian Esken (chris@magicon.prima.ruhr.de)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 	kaudioserver.cpp: Audio server for libmediatool/maudio
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

extern "C" {
#include <mediatool.h>
}

char		KMServerPidFile[256];


MdCh_IHDR	*IhdrChunk;
MediaCon	m;


// Signal handler for dying child. Remove communication id file and exit right now.
void mysigchild(int /*signum*/)
{
  unlink (KMServerPidFile);
  exit(1);  
}


char *fatalexit(char *s)
{
  fprintf(stderr, s);
  exit(-1);
}


// call this only *once*
void baseinitMediatool(void)
{
  MdConnectInit();
}


// call this for every new connection
bool initMediatool(void)
{
  MdConnectNew(&m);
  if ( m.shm_adr == NULL )
    fatalexit("Could not create media connection.\n");

  IhdrChunk   = (MdCh_IHDR*)FindChunkData(m.shm_adr, "IHDR");
  if (!IhdrChunk)
    fatalexit("No IHDR chunk.\n");

  return(true);
}


int main ( int, char** )
{
  char		NotStarted[]="Player not started\n";
  char		*tmpadr;
  
  FILE		*KMServerPidHandle;
  char		ServerId[256];

  char* Opts[10];
  char MaudioText[]="maudio";
  char MediaText[]="-media";
  //  int forkret;

  // Create full path of ~/.kaudioserver, then delete the communication id
  // file
  tmpadr= getenv("HOME");
  strcpy(KMServerPidFile,tmpadr);
  strcpy(KMServerPidFile+strlen(KMServerPidFile),"/.kaudioserver");

  baseinitMediatool();
  if (! initMediatool() )
    fatalexit(NotStarted);

  // Write communication id file
  KMServerPidHandle = fopen(KMServerPidFile,"w");
  if (KMServerPidHandle == NULL)
    {
      unlink (KMServerPidFile);
      fatalexit("PID could not get written.\n");
    }
  fprintf(KMServerPidHandle,"%i\n",IhdrChunk->ref);
  fclose(KMServerPidHandle);

  printf("Starting audio server with talk id %i.\n", IhdrChunk->ref);

  sprintf(ServerId,"%i", IhdrChunk->ref);

  /* The (char*) casts are for getting rid of const */
  Opts[0]=MaudioText;
  Opts[1]=(char*)MediaText;
  Opts[2]=(char*)ServerId;
  Opts[3]=(char*)NULL;

  int forkret=fork();
  if (forkret==0) {
    execvp( MaudioText, Opts);
    fprintf(stderr,"Failed starting audio server!\n");
  }
  else if (forkret > 0) {
    /* Stupid SysV IPC throws away SHM piece, when autodeletion is enabled and
     * when calling exec(). So I just idle around here, waiting for the child
     * to die :´-)
     */
    signal(SIGCHLD,mysigchild);
    while(1)
      sleep(6000);
  }

  fprintf(stderr,"Failed starting audio server!\n");
  unlink (KMServerPidFile);
  exit(1);
}
