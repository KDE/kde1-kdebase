/*
 * mouse.h
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


#ifndef __MOUSECONFIG_H__
#define __MOUSECONFIG_H__

#include <qdialog.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qlcdnum.h>
#include <qradiobt.h>
#include <qbttngrp.h>

#include <kapp.h>
#include <kslider.h>

#include <kcontrol.h>

#define RIGHT_HANDED 0
#define LEFT_HANDED  1

class MouseConfig : public KConfigWidget
{
  Q_OBJECT
public:
  MouseConfig( QWidget *parent=0, const char* name=0, bool init=FALSE);
  ~MouseConfig();
  void  resizeEvent(QResizeEvent *e);
  void saveParams( void );

  void loadSettings();
  void applySettings();
  
private:
  void GetSettings( void );

  int getAccel();
  int getThreshold();
  int getHandedness();

  void setAccel(int);
  void setThreshold(int);
  void setHandedness(int);

  KSlider *accel;
  QLabel *alabel;
  QLCDNumber *a;

  KSlider *thresh;
  QLabel *tlabel;
  QLCDNumber *t;

  QButtonGroup *handedBox;
  QRadioButton *leftHanded, *rightHanded;
  int num_buttons;
  int middle_button;
  bool handedEnabled;

  KConfig *config;
  int accelRate, thresholdMove, handed;
};

#endif

