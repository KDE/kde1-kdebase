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
#ifndef __KWMSOUND_H__
#define __KWMSOUND_H__

#include <mediatool.h>
#include <kaudio.h>

#include <kwmmapp.h>

#include <qlist.h>
#include <qstring.h>

#define EVENT_COUNT 12

typedef struct {
  QString name;
  int priority;
} SoundInfo;


enum EventType {Desktop1=0, Desktop2=1, Desktop3, Desktop4, Desktop5, Desktop6, Desktop7, Desktop8,
	WindowActivate, WindowOpen, WindowClose, Startup, NoEvent};


class KWmSound : public QObject
{
  Q_OBJECT;

public:
  KWmSound(KWMModuleApplication *modap);
  virtual ~KWmSound();

  void loadSetup();

public slots:
    void init();
    void desktopChange(int deskno);
    void windowAdd(Window);
    void windowRemove(Window);
    void windowActivate(Window);
    void processKWMCommand(QString cmd);
private:

  void doPlaying(int index);

  KAudio *audio;
  EventType last_event;
  int last_priority;  

  QList<SoundInfo> sounds;
  char sounds_enabled;

};


#endif
