/*

    $Id: syssound.cpp,v 1.6 1998/04/17 22:08:08 kulow Exp $

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

    $Log: syssound.cpp,v $
    Revision 1.6  1998/04/17 22:08:08  kulow
    fixed typo

    Revision 1.5  1998/03/08 08:01:32  wuebben
    Bernd: implemented support for all sound events


*/  


#include <stdio.h>
#include <stdlib.h>
#include <qobject.h>
#include <qlabel.h>

#include <qlistbox.h>
#include <qpushbt.h>
#include <qlayout.h>
#include <qdir.h>
#include <qmsgbox.h>


#include <klocale.h>
#include <kapp.h>
#include <kwm.h>
#include <drag.h>
#include <kstring.h>

#include "syssound.h"
#include "syssound.moc"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#define EVENT_COUNT 28

#define SOUND_DEBUG 1

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


KSoundWidget::KSoundWidget(QWidget *parent, const char *name):
  KConfigWidget(parent, name), selected_event(0){


  QBoxLayout *col1, *col2, *col3, *columns, *top_layout;

  int delta;
  QString path;
  QDir dir;
  const QStrList *list;
  QLabel *eventlabel, *soundlabel, *statustext;

  //
  // CC: Set up the List of known System Events
  //

  eventlist = new QListBox(this);
  eventlist->insertItem(klocale->translate("(none)"));
  eventlist->insertItem(klocale->translate("Change to Desktop 1"));
  eventlist->insertItem(klocale->translate("Change to Desktop 2"));
  eventlist->insertItem(klocale->translate("Change to Desktop 3"));
  eventlist->insertItem(klocale->translate("Change to Desktop 4"));
  eventlist->insertItem(klocale->translate("Change to Desktop 5"));
  eventlist->insertItem(klocale->translate("Change to Desktop 6"));
  eventlist->insertItem(klocale->translate("Change to Desktop 7"));
  eventlist->insertItem(klocale->translate("Change to Desktop 8"));

  eventlist->insertItem(klocale->translate("Activate Window"));
  eventlist->insertItem(klocale->translate("Open new window"));
  eventlist->insertItem(klocale->translate("Close Window"));
  eventlist->insertItem(klocale->translate("Startup"));

  eventlist->insertItem(klocale->translate(    "Window Shade Up"));
  eventlist->insertItem(klocale->translate(    "Window Shade Down"));
  eventlist->insertItem(klocale->translate(    "Window Iconify"));
  eventlist->insertItem(klocale->translate(    "Window DeIconify"));
  eventlist->insertItem(klocale->translate(    "Window Maximize"));
  eventlist->insertItem(klocale->translate(    "Window UnMaximize"));
  eventlist->insertItem(klocale->translate(    "Window Sticky"));
  eventlist->insertItem(klocale->translate(    "Window UnSticky"));
  eventlist->insertItem(klocale->translate(    "Window Trans New"));
  eventlist->insertItem(klocale->translate(    "Window Trans Delete"));
  eventlist->insertItem(klocale->translate(    "Logout"));
  eventlist->insertItem(klocale->translate(    "Logout Message"));
  eventlist->insertItem(klocale->translate(    "Window Move Start"));
  eventlist->insertItem(klocale->translate(    "Window Move End"));
  eventlist->insertItem(klocale->translate(    "Window Resize Start"));
  eventlist->insertItem(klocale->translate(    "Window Resize End"));

  //
  // CC: Now set up the list of known WAV Files
  //

  soundlist = new QListBox(this);

  soundlist->insertItem(klocale->translate("(none)"));

  path = KApplication::kde_sounddir().copy();
  dir.setPath(path);
  dir.setNameFilter("*.wav");
  dir.setSorting(QDir::Name);
  dir.setFilter(QDir::Readable | QDir::Files);
  list = dir.entryList();

  soundlist->insertStrList(list);
  
  audiodrop = new KDNDDropZone(soundlist, DndURL);

  sounds_enabled = new QCheckBox(this);
  sounds_enabled->setText(klocale->translate("e&nable system sounds"));
  sounds_enabled->setMinimumSize(sounds_enabled->sizeHint());
  sounds_enabled->setMaximumSize(sounds_enabled->sizeHint());

  btn_test = new QPushButton(this);
  btn_test->setText(klocale->translate("&Test"));
  btn_test->setMinimumSize(btn_test->sizeHint());
  btn_test->setMaximumSize(btn_test->sizeHint());  

  eventlabel = new QLabel(eventlist, klocale->translate("&Events:"), this);
  eventlabel->setMinimumSize(eventlabel->sizeHint());
  eventlabel->setMaximumSize(eventlabel->sizeHint());

  soundlabel = new QLabel(soundlist, klocale->translate("&Sounds:"), this);
  soundlabel->setMinimumSize(soundlabel->sizeHint());
  soundlabel->setMaximumSize(soundlabel->sizeHint());

  statustext = new QLabel(klocale->translate(
	       "Additional WAV files can be dropped onto the sound list."
	       ),this);

  statustext->setMinimumSize(statustext->sizeHint());
  statustext->setMaximumSize(statustext->sizeHint());

  delta = eventlabel->height();

  top_layout = new QVBoxLayout(this, 10);

  columns = new QHBoxLayout(10);

  top_layout->addWidget(sounds_enabled,0,AlignLeft);
  top_layout->addLayout(columns,1);
  top_layout->addWidget(statustext,0,AlignLeft);

  col1 = new QVBoxLayout();
  col2 = new QVBoxLayout();
  col3 = new QVBoxLayout();

  columns->addLayout(col1, 3);
  columns->addLayout(col2, 4);
  columns->addLayout(col3, 0); 

  col1->addWidget(eventlabel,0,AlignLeft);
  col1->addWidget(eventlist,1);

  col2->addWidget(soundlabel,0,AlignLeft);
  col2->addWidget(soundlist,1);

  col3->addSpacing(delta);
  col3->addWidget(btn_test,0);
  // CC: add more buttons here
  col3->addStretch(1);

  top_layout->activate();
  setUpdatesEnabled(TRUE);

  readConfig();

  connect(eventlist, SIGNAL(highlighted(int)), this, SLOT(eventSelected(int)));
  connect(soundlist, SIGNAL(highlighted(const char*)), 
	  this, SLOT(soundSelected(const char*)));
  connect(btn_test, SIGNAL(clicked()), this, SLOT(playCurrentSound()));

  connect(audiodrop, SIGNAL(dropAction(KDNDDropZone*)), 
	  SLOT(soundDropped(KDNDDropZone*)));

};

KSoundWidget::~KSoundWidget(){

 // delete audiodrop;

}

void KSoundWidget::readConfig(){

  QString *str;
  QString hlp;
  KConfig *config;
  int lf;
  

  // CC: we need to read/write the config file of "kwmsound" and not 
  // our own (that would be called syssoundrc)

  config = new KConfig(kapp->localconfigdir()+"/kwmsoundrc");

  config->setGroup("SoundConfiguration");  

  for( lf = 0; lf < EVENT_COUNT; lf++) {

    str = new QString;
    *str = config->readEntry(eventNames[0][lf],"(none)");

    if (str->data()[0] == '/') {
      // CC: a file that is not in the default 
      // sound directory-> add it to the soundlist too

      addToSoundList(*str);

    }

    soundnames.append(str);

  }

  soundnames.setAutoDelete(TRUE);

  config->setGroup("GlobalConfiguration");

  hlp = config->readEntry("EnableSounds","No");

  if (!stricmp(hlp,"Yes")) 
    sounds_enabled->setChecked(True);
  else
    sounds_enabled->setChecked(False);

  delete config;

}


void KSoundWidget::eventSelected(int index){


  int i;
  uint listlen;
  char found;
  QString *sname, hlp;

  selected_event = index;

  if (0 == index)
    soundlist->setCurrentItem(0);
  else {
    // CC: at first, get the name of the sound file we want to select
    sname = soundnames.at(index-1);
    CHECK_PTR(sname); // CC: should never happen anyways...
      debug("event %d wants sound %s", index, sname->data());

    i = 1;
    listlen = soundlist->count();
    found = 0;
    while ( (!found) && (i < (int)listlen) ) {
      hlp = soundlist->text(i);
      if (!strcmp(hlp, *sname)) 
	found = 1;
      else
	i++;
    }
    
    if (found) 
      soundlist->setCurrentItem(i);
    else
      soundlist->setCurrentItem(0);
      // CC: By default, select "no sound"

  }

  soundlist->centerCurrentItem();

}

void KSoundWidget::soundSelected(const char *filename) 
{
  QString *snd;

  if (selected_event > 0) {
    snd = soundnames.at(selected_event-1);
    *snd= filename;
  }
}


void KSoundWidget::saveConfiguration(){

  KConfig *config;
  QString *sname, helper;
  int lf;
  KWM kwm;
  
  // config = kapp->getConfig();
  config = new KConfig(kapp->localconfigdir()+"/kwmsoundrc");

  config->setGroup("SoundConfiguration");  

  for( lf = 0; lf < EVENT_COUNT; lf++) {
    sname = soundnames.at(lf);

    if (!strcmp("", *sname)) 
      config->writeEntry(eventNames[0][lf],"(none)");
    else {
      // keep configuration files language--independent

      if (!strcmp(klocale->translate("(none)"), *sname))
	config->writeEntry(eventNames[0][lf], "(none)");
      else
	config->writeEntry(eventNames[0][lf], *sname);

    }

  }

  config->setGroup("GlobalConfiguration");

  if (sounds_enabled->isChecked())
    config->writeEntry("EnableSounds", "Yes");
  else
    config->writeEntry("EnableSounds", "No");

  config->sync();
  delete config;
  kwm.sendKWMCommand("syssnd_restart");

}


void KSoundWidget::playCurrentSound()
{
  QString hlp, sname;
  int soundno;

  audio.stop();
  soundno = soundlist->currentItem();
  if (soundno > 0) {
    sname = soundlist->text(soundno);
    if (sname[0] != '/') {
      hlp = KApplication::kde_sounddir().copy(); 
      hlp += "/";  // Don't forget this -- Bernd
      hlp += sname;
      audio.play((char*)hlp.data());
    } else {
      audio.play((char*)sname.data());
    }
  }
}


void KSoundWidget::soundDropped(KDNDDropZone *zone){

  QStrList &list = zone->getURLList();;
  QString msg;
  int i, len;

  //
  // CC: For now, we do only accept FILES ending with .wav...
  //

  len = list.count();

  for ( i = 0; i < len; i++) {

    QString url = list.at(i);

    if (strcmp("file:", url.left(5))) {

      // CC: for now, only file URLs are supported

      QMessageBox::warning(this, klocale->translate("Unsupported URL"),
        klocale->translate(
                 "Sorry, this type of URL is currently unsupported"\
		 "by the KDE System Sound Module"
		           )
			   );

    } else {

      // CC: Now check for the ending ".wav"

      if (stricmp(".WAV",url.right(4))) {
        ksprintf(&msg, i18n("Sorry, but \n%s\ndoes not seem "\
			    "to be a WAV--file."), url.data());

	QMessageBox::warning(this, klocale->translate("Improper File Extension"), msg);

      } else {

	// CC: Hurra! Finally we've got a WAV file to add to the list

	url = url.right(url.length()-5); // strip the leading "file:"

	if (!addToSoundList(url)) {

	  // CC: did not add file because it is already in the list
	  ksprintf(&msg, i18n("The file\n"
			      "%s\n"
			      "is already in the list"), url.data());

	  QMessageBox::warning(this, klocale->translate("File Already in List"), msg);

	}
      }
    }
  }
}


bool KSoundWidget::addToSoundList(QString sound){

  // Add "sound" to the sound list, but only if it is not already there

  char found = 0;
  int i, len;

  i = 0;
  len = soundnames.count();

  while ((!found) && (i < len)) {

    found = (sound == *soundnames.at(i));
    i++;
  }
 
 if (!found) {

   // CC: Fine, the sound is not already in the sound list!

   QString *tmp = new QString(sound); // CC: take a copy...
   //   soundnames.append(tmp); 
   soundlist->insertItem(*tmp);
   soundlist->setTopItem(soundlist->count()-1);

 }

 return !found;

}


void KSoundWidget::loadSettings(){

  readConfig();

}


void KSoundWidget::applySettings(){

  saveConfiguration();

}
