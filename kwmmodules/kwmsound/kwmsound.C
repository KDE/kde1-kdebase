/* This file is part of the KDE system sound package
    Copyright (C) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)

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
*/  
#include <stdlib.h>
#include <kapp.h>
#include "kwmsound.h"

#include <stdio.h>

#include "kwmsound.moc"

char *eventNames[EVENT_COUNT] = 
                               { "Desktop1", "Desktop2", "Desktop3", "Desktop4", "Desktop5",
                                 "Desktop6", "Desktop7", "Desktop8", "WindowActivate", 
                                 "WindowOpen", "WindowClose", "Startup" };


KWmSound::KWmSound(KWMModuleApplication *modapp)
{
  audio = new KAudio();
  sounds.setAutoDelete(TRUE);
  last_event = NoEvent;
  last_priority = 0;
  loadSetup();

  connect(modapp, SIGNAL(init()), this, SLOT(init()));
  connect(modapp, SIGNAL(desktopChange(int)), this, SLOT(desktopChange(int)));
  connect(modapp, SIGNAL(windowAdd(Window)), this, SLOT(windowAdd(Window)));
  connect(modapp, SIGNAL(windowRemove(Window)), this, SLOT(windowRemove(Window)));
  connect(modapp, SIGNAL(windowActivate(Window)), this, SLOT(windowActivate(Window)));
  connect(modapp, SIGNAL(commandReceived(QString)), this, SLOT(processKWMCommand(QString)));

  doPlaying((int)Startup);
  audio->sync();
}

KWmSound::~KWmSound()
{
  delete audio;
}

void KWmSound::loadSetup()
{
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
   
   helper = config->readEntry(eventNames[i], "(none)"); 
   if (!strcmp(helper, "(none)")) {
     helper = "";
   } else if ('/' != helper[0]) {
     tmp = KApplication::kde_sounddir().copy(); 
     tmp += helper;
     helper = tmp;
   }  
   snd->name = helper;
   snd->priority = 1; //CC: for now, priority has no effect..

   sounds.append(snd);
 }

 config->setGroup("GlobalConfiguration");
 helper = config->readEntry("EnableSounds","No");
 sounds_enabled = stricmp(helper, "No");
}

void KWmSound::init()
{
  loadSetup();
  audio->stop();
}


void KWmSound::desktopChange(int deskno)
{
  doPlaying((int)Desktop1+(deskno-1));
}


void KWmSound::windowAdd(Window)
{
  doPlaying((int)WindowOpen);
}


void KWmSound::windowRemove(Window)
{
  doPlaying((int)WindowClose);
}



void KWmSound::windowActivate(Window)
{
  SoundInfo *info;

  if ( (last_event > Desktop8) &&
       (last_event != WindowOpen))
    doPlaying(WindowActivate);
  else {
    info = sounds.at((int)WindowActivate);
    last_event = WindowActivate;
    last_priority = info->priority;
  }
}


void KWmSound::processKWMCommand(QString cmd)
{
  if (!strcmp(cmd,"syssnd_restart")) {
	loadSetup();
  }
}	

void KWmSound::doPlaying(int index)
{
  SoundInfo *info;
  QString *fname;
  
  if (!sounds_enabled)
    return;

  info = sounds.at(index);
  if (NULL == info) return;

  fname =&(info->name);

  if (info->priority >= last_priority)
    audio->stop();
  audio->play((char*)fname->data());

  last_event = (EventType)index;
  last_priority = info->priority;
}

int main(int argc, char *argv[])
{
  KWMModuleApplication *a = new KWMModuleApplication(argc, argv);
  
  KWmSound *sound;
 
  sound = new KWmSound(a);
  a->connectToKWM();

  a->exec();
  delete sound;
  delete a;
}
