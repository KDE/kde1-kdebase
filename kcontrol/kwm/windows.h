/*
 * windows.h
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

#ifndef __KWINDOWCONFIG_H__
#define __KWINDOWCONFIG_H__

#include <qlcdnum.h> 
#include <qlabel.h>
#include <qdialog.h>
#include <qmsgbox.h>
#include <qradiobt.h> 
#include <qbttngrp.h>
#include <qpushbt.h>
#include <kslider.h>

#include <kcontrol.h>

#include <kwm.h>

#define TRANSPARENT 0
#define OPAQUE      1

#define CLICK_TO_FOCUS     0
#define FOCUS_FOLLOW_MOUSE 1

#define TITLEBAR_PLAIN  0
#define TITLEBAR_SHADED 1

#define RESIZE_ANIM_OFF 0
#define RESIZE_ANIM_ON  1

#define MAXIMIZE_FULL 0
#define MAXIMIZE_VERT 1

// CT 19jan98
#define RANDOM_PLACEMENT 0
#define SMART_PLACEMENT  1

class KWindowConfig : public KConfigWidget
{
  Q_OBJECT
public:
  KWindowConfig( QWidget *parent=0, const char* name=0 );
  ~KWindowConfig( );
  void  resizeEvent(QResizeEvent *e);
  void SaveSettings( void );

  void loadSettings();
  void applySettings();
  
private slots:
  void setAutoRaiseEnabled();

private:

  void GetSettings( void );

  int getMove( void );
  int getAnim( void );
  int getPlacement( void ); //CT
  int getFocus( void );
  int getMaximize( void );
  int getAutoRaise( void );

  void setMove(int);
  void setAnim(int);
  void setPlacement(int); //CT
  void setFocus(int);
  void setMaximize(int);
  void setAutoRaise(int);

  QButtonGroup *moveBox;
  QRadioButton *transparent, *opaque; 

  // CT 19jan98
  QButtonGroup *placementBox;
  QRadioButton *random, *smart;


  QButtonGroup *focusBox;
  QRadioButton *clickTo, *followMouse;

  QButtonGroup *resizeBox;
  QRadioButton *animOn, *animOff;

  QButtonGroup *maximizeBox;
  QRadioButton *fullScreen, *vertOnly;

  KSlider *autoRaise;
  QLabel *alabel;
  QLCDNumber *s;
  QLabel *sec;

};

#endif

