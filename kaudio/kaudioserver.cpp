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
#include <string.h>
#include <signal.h>
#include <config.h>
extern "C" {
#include <mediatool.h>
	   }

char		KMServerPidFile[256];


MdCh_IHDR	*IhdrChunk;
MdCh_STAT	*StatChunk;
MediaCon	m;



void MYexit(int retcode)
{
  exit (retcode);
}

// Signal handler for dying child. Remove communication id file and exit right now.
void mysigchild(int /*signum*/)
{
  unlink (KMServerPidFile);
  MYexit(1);  
}


void fatalexit(char *s)
{
  fprintf(stderr, s);
  MYexit(-1);
}


// call this only *once*
void baseinitMediatool(void)
{
  MdConnectInit();
}


// call this for every new connection
// Returns: 0= Failure to create connection
//	    1= Uses old connection
//          2= Created a new connection
int initMediatool(char *Cid)
{
  if (Cid[0]!=0) {
    int CommId = atoi(Cid);
    MdConnect(CommId, &m);
    if (m.shm_adr != NULL) {
      // OK, there is an old connection
      StatChunk = (MdCh_STAT*)FindChunkData(m.shm_adr, "STAT");
      if (StatChunk)
	if ( ! ( StatChunk->status & MD_STAT_EXITED) )
	  // OK, everything is good. The Server still lives
	  return 1;
    }
  }

  MdConnectNew(&m);
  if ( m.shm_adr == NULL )
    fatalexit("Could not create media connection.\n");

  IhdrChunk   = (MdCh_IHDR*)FindChunkData(m.shm_adr, "IHDR");
  if (!IhdrChunk)
    fatalexit("No IHDR chunk.\n");

  return(2);
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
  char PidRead[100];


  // Create full path of ~/.kaudioserver, then delete the communication id
  // file
  tmpadr= getenv("HOME");
  strcpy(KMServerPidFile,tmpadr);
  strcpy(KMServerPidFile+strlen(KMServerPidFile),"/.kaudioserver");

  baseinitMediatool();

  // Read old communication id
  KMServerPidHandle = fopen(KMServerPidFile,"r");
  if (KMServerPidHandle == NULL)
    PidRead[0]=0;
  else
    fgets(PidRead, 100, KMServerPidHandle);

  fclose(KMServerPidHandle);
  int ret =  initMediatool(PidRead);
  if (ret==0)
    fatalexit(NotStarted);

  if (ret ==1 ) {
    printf("Using old audio server with talk id %s.\n", PidRead);
    exit (0);
  }

  // New communication wanted
  // Write communication id file
  KMServerPidHandle = fopen(KMServerPidFile,"w");
  if (KMServerPidHandle == NULL) {
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
    while(1) {
      sleep(100);
    }
  }

  fprintf(stderr,"Failed starting audio server!\n");
  unlink (KMServerPidFile);
  MYexit(1);
}
