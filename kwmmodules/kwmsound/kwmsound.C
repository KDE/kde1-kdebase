/* 

   $Id: kwmsound.C,v 1.6 1999/01/28 14:10:16 kulow Exp $

   This file is part of the KDE system sound package
   Copyright (C) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)
                 1998 Bernd Wuebben <wuebben@kde.org>
   
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   $Log: kwmsound.C,v $
   Revision 1.6  1999/01/28 14:10:16  kulow
   some fixes for a cleaner compilation with non-GNU compilers

   Revision 1.5  1998/06/22 20:39:29  esken
   Fixing kwmsound: Waits for server process and exits cleanly after n retries.

   Revision 1.4  1998/05/01 22:34:51  esken
   Fixed a bug. Only assigned events are played now. Others
   were played using an empty filename.

   Revision 1.3  1998/03/08 08:05:44  wuebben
   Bernd: support for all sound events


*/  


#include <stdlib.h>
#include <kapp.h>
#include "kwmsound.h"

#include <stdio.h>
#include <unistd.h>

#include "kwmsound.moc"

#define EVENT_COUNT 28

// Define this for some useful debugging output -- Bernd
//#define SOUND_DEBUG 1

char *eventNames[2][29] = {

  {
    "Desktop1", 
    "Desktop2", 
    "Desktop3", 
    "Desktop4", 
    "Desktop5",
    "Desktop6", 
    "Desktop7", 
    "Desktop8", 
    "WindowActivate", 
    "WindowOpen", 
    "WindowClose", 
    "Startup", 
    "WindowShadeUp",                  
    "WindowShadeDown",
    "WindowIconify",
    "WindowDeIconify",
    "WindowMaximize",
    "WindowUnMaximize",
    "WindowSticky",
    "WindowUnSticky",
    "WindowTransNew",
    "WindowTransDelete",
    "Logout",
    "LogoutMessage",
    "WindowMoveStart",
    "WindowMoveEnd",
    "WindowResizeStart",
    "WindowResizeEnd",
  },
  {
    "Desktop1", 
    "Desktop2", 
    "Desktop3", 
    "Desktop4", 
    "Desktop5",
    "Desktop6", 
    "Desktop7", 
    "Desktop8", 
    "Window Activate", 
    "Window New", 
    "Window Delete", 
    "Startup", 
    "Window Shade Up",
    "Window Shade Down",
    "Window Iconify",
    "Window DeIconify",
    "Window Maximize",
    "Window UnMaximize",
    "Window Sticky",
    "Window UnSticky",
    "Window Trans New",
    "Window Trans Delete",
    "Logout",
    "Logout Message",
    "Window Move Start",
    "Window Move End",
    "Window Resize Start",
    "Window Resize End",
  }
};
                  

KWmSound::KWmSound(KWMModuleApplication *modapp){

  sleep(1);  // Workaround: wait for maudio to fire up
  int tries;
  for (tries=0; tries< 10; tries++) {
    audio = new KAudio();
    if ( audio && audio->serverStatus() == 0 )
       break;
    else {
      if (audio)
        delete audio;
      sleep(1);
    }
  }
  if (tries==10) {
    warning("kwmsound: Failed connecting the audio server.\n"
           "Please check manually if you can start kaudioserver.");
    exit(1);
  }

  sounds.setAutoDelete(TRUE);

  last_event = "";
  last_priority = 0;
  loadSetup();

  connect(modapp, SIGNAL(init()), this, 
	  SLOT(init()));

  connect(modapp, SIGNAL(desktopChange(int)), this, 
	  SLOT(desktopChange(int)));

  connect(modapp, SIGNAL(commandReceived(QString)), this, 
	  SLOT(processKWMCommand(QString)));

  connect(modapp, SIGNAL(playSound(QString)), this, 
	  SLOT(playSound(QString)));


  doPlaying(lookup("Startup"));
  audio->sync();

}


KWmSound::~KWmSound(){

  delete audio;

}


int  KWmSound::lookup(char* event){

#ifdef SOUND_DEBUG
  printf("Looking up:%s\n", event);
#endif

  for(int i = 0; i < EVENT_COUNT; i++){

    if(!strcmp(event,eventNames[1][i])){

#ifdef SOUND_DEBUG
        printf("Found:%s\n",eventNames[1][i] );
#endif

	return i;

    }
  }

  return -1;

}

void KWmSound::loadSetup(){

 KConfig *config;
 SoundInfo *snd;
 QString helper, tmp;
 int i;

 sounds.clear();

 config = KApplication::getKApplication()->getConfig();
 config->reparseConfiguration();
 config->setGroup("SoundConfiguration");

 for(i=0; i < EVENT_COUNT; i++) {

   snd = new SoundInfo();
   helper = config->readEntry(eventNames[0][i], "(none)"); 

#ifdef SOUND_DEBUG
   printf("Loading :%s\t\t%s\n",eventNames[0][i],helper.data());
#endif

   if (!strcmp(helper, "(none)")) {
   
     helper = "";
   
   } else if ('/' != helper[0]) {
     
     tmp = KApplication::kde_sounddir().copy();
     tmp += "/";
     tmp += helper;
     helper = tmp;

   }  

   snd->name = helper;
   snd->priority = 1; //CC: for now, priority has no effect..
   sounds.append(snd);

 }

 config->setGroup("GlobalConfiguration");

 helper         = config->readEntry("EnableSounds","No");
 sounds_enabled = stricmp(helper, "No");

}


void KWmSound::init(){

  loadSetup();
  audio->stop();

}



void KWmSound::desktopChange(int deskno){
  
  QString str;
  str = str.sprintf("Desktop%d",deskno);

  doPlaying(lookup(str.data()));

}


void KWmSound::playSound(QString soundevent){

#ifdef SOUND_DEBUG
  printf("kwmsound: received %s\n",soundevent.data());
#endif 

  doPlaying(lookup(soundevent.data()));

}


void KWmSound::processKWMCommand(QString cmd){

  if (!strcmp(cmd,"syssnd_restart")) {
	loadSetup();
  }

}	

void KWmSound::doPlaying(int index){

  SoundInfo *info;
  QString *fname;
  
  if (!sounds_enabled)
    return;

  if( index == -1 ){
    fprintf(stderr,"kwmsound: Unknown Soundevent.\n");
    return;
  }

  if(index >=(int) sounds.count() || index < 0){
    fprintf(stderr,"kwmsound: Invalid sound index. %d\n",index);
    return;
  }

  info = sounds.at(index);

  if (NULL == info) 
    return;

  fname =&(info->name);

  if (info->priority >= last_priority)
    audio->stop();

  // Check for unassigned sound. Do not try to play them
  if (fname->length() == 0)
    return;

  audio->play((char*)fname->data());

#ifdef SOUND_DEBUG
  printf("Trying to play:%s\n",fname->data());
#endif

  last_event = eventNames[1][index]   ;
  last_priority = info->priority;


}


int main(int argc, char **argv){

  KWMModuleApplication *a = new KWMModuleApplication(argc, argv);
  KWmSound *sound;
 
  sound = new KWmSound(a);
  a->connectToKWM();

  return a->exec();

}
