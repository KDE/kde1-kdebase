/*
 * bell.h
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifndef __KBELLCONFIG_H__
#define __KBELLCONFIG_H__

#include <kapp.h>

#include <qlcdnum.h> 
#include <qlabel.h> 
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qradiobt.h> 

#include <kslider.h>
#include <kcontrol.h>


class KBellConfig : public KConfigWidget
{
  Q_OBJECT
public:
  KBellConfig( QWidget *parent=0, const char *name=0, bool init=FALSE );
  ~KBellConfig( );
  void resizeEvent( QResizeEvent * );
  void saveParams( void );

public slots:

  void loadSettings();
  void applySettings();
  
private slots:
  void ringBell();

private:
  void GetSettings( void );

  int getBellVolume();
  int getBellPitch();
  int getBellDuration();

  void setBellVolume(int);
  void setBellPitch(int);
  void setBellDuration(int);

  KSlider *volume, *pitch, *duration;
  QLabel *vlabel, *plabel, *dlabel;
  QLCDNumber *v, *p, *d;
  QLabel *percent, *hertz, *ms;

  QPushButton *test;

  KConfig *config;
  int bellVolume, bellPitch, bellDuration;
};

#endif

