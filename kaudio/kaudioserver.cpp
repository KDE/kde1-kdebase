/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)

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
#include <sys/types.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <config.h>
extern "C" {
#include <mediatool.h>
	   }

#define maxFnameLen 256
char		KMServerPidFile[maxFnameLen];


MdCh_IHDR	*IhdrChunk;
MdCh_STAT	*StatChunk;
MediaCon	m;
Display		*globalDisplay;
pid_t		maudioPID = 0;





/*
 * disconnectChild() removes the maudio process. This could
 * be done by using the MdDisconnect() command. But in case
 * of severe trouble (SHM segment garbled), I still want to
 * make sure, maudio does get killed.
 * So I send maudio a "QUIT" signal, which always should work.
 */
void disconnectChild()
{
  if (maudioPID != 0)
    kill(maudioPID, SIGQUIT);
}


// myExit(int retcode) ---------------------------------------------------
//  Cleans up anything that might be neccesary and then exits via
//  exit(). Thus this function never returns.
void myExit(int retcode)
{
  disconnectChild();
  unlink (KMServerPidFile);
  exit (retcode);
}

// signal handler for various signals. Calls myExit() -------------------
void mysigchild(int signum)
{
  myExit(1);  
}

// fatalexit() prints out a string and calls myExit() -------------------
void fatalexit(char *s)
{
  fprintf(stdout, s);
  myExit(2);
}

// myXerrorHandler() is my X11 fatal IO-Error-Handler -------------------
// It quits right away with a message.
int myXerrorHandler(Display *dsp)
{
  fatalexit("kaudioserver: Catching fatal X IO Error. Cleaning up.\n");
}


// Initialization of Mediatool library connection
void baseinitMediatool(void)
{
  MdConnectInit();
}



// Create a dummy X11 connection ----------------------------------------
void initXconnection()
{
  globalDisplay = XOpenDisplay(NULL);
  Display *dpy = globalDisplay;
  if ( globalDisplay != 0 ) {
    XSetIOErrorHandler(myXerrorHandler);
    Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0,0,1,1, \
	0,
        BlackPixelOfScreen(DefaultScreenOfDisplay(dpy)),
        BlackPixelOfScreen(DefaultScreenOfDisplay(dpy)) );
    //XMapWindow(dpy, w);
  }
}




// call this for every new connection
// Returns: 0= Failure to create connection
//	    1= Uses old connection
//          2= Created a new connection
int initMediatool(char *Cid)
{
  if (Cid[0]!=0) {
    // There is a string with length > 0
    int CommId = atoi(Cid);
    // Try to connect to given connection
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

  // We only get here, if there is no living Server ---------------------
  MdConnectNew(&m);
  if ( m.shm_adr == NULL )
    fatalexit("Could not create media connection.\n");

  // Great, a new connection could be created
  IhdrChunk   = (MdCh_IHDR*)FindChunkData(m.shm_adr, "IHDR");
  if (!IhdrChunk)
    // If there's no header chunk ("IHDR"), something has terribly got
    // messed up or someone is trying to play bad games with us.
    fatalexit("No IHDR chunk.\n");

  return(2);
}


int main ( int argc , char **argv )
{
  char		NotStarted[]="Player not started\n";
  char		*tmpadr;
  const char    kasFileName[]="/.kaudioserver";

  FILE		*KMServerPidHandle;
  char		ServerId[256];

  char* Opts[10];
  char MaudioText[]="maudio";
  char MediaText[]="-media";
  char DevnumText[]="-devnum";
  char PidRead[100];
  char DevnumArg[100];


  if ( argc > 1 )
    sprintf(DevnumArg, "%s", argv[1]);
  else
    sprintf(DevnumArg, "#" ); // Illegal Dev-Number => Default device

  // Create full path of ~/.kaudioserver, then delete the communication id
  // file
  tmpadr= getenv("HOME");
  int homePathLen = strlen(tmpadr);

  if ( (homePathLen+strlen(kasFileName)+1 ) >= maxFnameLen )
    fatalexit("HOME path too long");

  strcpy(KMServerPidFile,tmpadr);
  strcpy(KMServerPidFile+homePathLen,kasFileName);

  initXconnection();
  if ( globalDisplay == 0 ) {
    fprintf(stdout, "kaudioserver: Can't connect to the X Server.\n" \
	" Audio system might not terminate at end of session.\n");
  }


  baseinitMediatool();

  // Read old communication id
  KMServerPidHandle = fopen(KMServerPidFile,"r");
  if (KMServerPidHandle == NULL)
    PidRead[0]=0;
  else
    fgets(PidRead, 100, KMServerPidHandle);

  if(KMServerPidHandle)
    fclose(KMServerPidHandle);
  int ret =  initMediatool(PidRead);
  if (ret==0)
    fatalexit(NotStarted);

  if (ret ==1 ) {
    printf("Using old audio server with talk id %s\n", PidRead);
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
  if (DevnumArg[0] == '#')
    Opts[3]=(char*)NULL;  // Default Device
  else {
    // Custom device
    Opts[3]=(char*)DevnumText;
    Opts[4]=(char*)DevnumArg;
    Opts[5]=(char*)NULL;
  }

  int forkret=fork();
  if (forkret==0) {
    execvp( MaudioText, Opts);
    fprintf(stdout,"Failed starting audio server!\n");
  }
  else if (forkret > 0) {
    maudioPID = forkret;

    signal(SIGCHLD,mysigchild);
    signal(SIGHUP ,mysigchild);
    signal(SIGQUIT,mysigchild);
    signal(SIGBUS ,mysigchild);
    signal(SIGSEGV,mysigchild);
    signal(SIGTERM,mysigchild);
 
    /*
     * Stupid SysV IPC throws away SHM piece, when autodeletion is enabled
     * and when calling exec(). So I just idle around here, waiting for the
     * child to die :'-|
     */
    while(1) {
      XEvent *event_return;
      if (globalDisplay != 0) {
        //XCheckMaskEvent(globalDisplay, (long)(-1l) , event_return);
        //XFlush(globalDisplay);
        XNextEvent(globalDisplay, event_return);
      }
      sleep(1);
    }
  }

  fprintf(stdout,"Failed starting audio server!\n");
  myExit(1);
}

