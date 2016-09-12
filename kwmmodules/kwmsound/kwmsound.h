/* 
   This file is part of the KDE system sound package

   $Id: kwmsound.h,v 1.3 1998/03/08 08:06:51 wuebben Exp $

   Copyright (C) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)
                 1998 Bernd Johannes Wuebben <wuebben@kde.org>
   
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

 
  $Log: kwmsound.h,v $
  Revision 1.3  1998/03/08 08:06:51  wuebben
  *** empty log message ***

  
*/

  
#ifndef __KWMSOUND_H__
#define __KWMSOUND_H__

#include <mediatool.h>
#include <kaudio.h>

#include <kwmmapp.h>

#include <qlist.h>
#include <qstring.h>



typedef struct {

  QString name;
  int priority;

} SoundInfo;


class KWmSound : public QObject{

  Q_OBJECT;

public:

  KWmSound(KWMModuleApplication *modap);
  virtual ~KWmSound();

  void loadSetup();

public slots:

    void init();
    void desktopChange(int deskno);
    void processKWMCommand(QString cmd);
    void playSound(QString sound);
    int  lookup(char* event);

private:

  void doPlaying(int index);

  KAudio *audio;
  QString last_event;
  int last_priority;  

  QList<SoundInfo> sounds;
  char sounds_enabled;

};


#endif
