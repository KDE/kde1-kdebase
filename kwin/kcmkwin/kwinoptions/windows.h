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
#include <qchkbox.h>
#include <qbttngrp.h>
#include <qpushbt.h>

#include <kslider.h>
#include <qcombo.h>   //CT 31jan98
#include <kspinbox.h> //CT 13mar98
#include <kcontrol.h>

#include <kwm.h>

#define TRANSPARENT 0
#define OPAQUE      1

#define CLICK_TO_FOCUS     0
#define FOCUS_FOLLOW_MOUSE 1

#define TITLEBAR_PLAIN  0
#define TITLEBAR_SHADED 1

#define RESIZE_TRANSPARENT  0
#define RESIZE_OPAQUE       1

#define MAXIMIZE_FULL 0
#define MAXIMIZE_VERT 1

// CT 19jan98
#define SMART_PLACEMENT       0
#define CASCADE_PLACEMENT     1 //CT 31jan98
#define INTERACTIVE_PLACEMENT 2 //CT 13mar98
#define RANDOM_PLACEMENT      3
#define MANUAL_PLACEMENT      4 //CT 13mar98

#define  CLICK_TO_FOCUS                0
#define  FOCUS_FOLLOWS_MOUSE           1
#define  CLASSIC_FOCUS_FOLLOWS_MOUSE   2
#define  CLASSIC_SLOPPY_FOCUS          3


class KWindowConfig : public KConfigWidget
{
  Q_OBJECT
public:
  KWindowConfig( QWidget *parent=0, const char* name=0 );
  ~KWindowConfig( );
  //  void  resizeEvent(QResizeEvent *e);
  void SaveSettings( void );

  void loadSettings();
  void applySettings();

private slots:
  void setAutoRaiseEnabled();
  void ifPlacementIsInteractive();
  void autoRaiseOnTog(bool);//CT 23Oct1998
  void clickRaiseOnTog(bool);

private:

  void GetSettings( void );

  int getMove( void );
  int getResizeAnim( void );
  int getResizeOpaque ( void );
  int getPlacement( void ); //CT
  int getFocus( void );
  int getMaximize( void );
  int getAutoRaiseInterval( void );

  void setMove(int);
  void setResizeAnim(int);
  void setResizeOpaque(int);
  void setPlacement(int); //CT
  void setFocus(int);
  void setMaximize(int);
  void setAutoRaiseInterval(int);
  void setAutoRaise(bool);
  void setClickRaise(bool);

  QButtonGroup *windowsBox;
  QCheckBox *opaque, *vertOnly;

  QCheckBox *resizeOpaqueOn;
  KSlider *resizeAnimSlider;
  QLabel *resizeAnimTitleLabel, *resizeAnimNoneLabel, *resizeAnimFastLabel;

  //CT 19jan98; 21Oct1998
  QButtonGroup *plcBox;
  QComboBox *placementCombo;
  KNumericSpinBox *interactiveTrigger;
  QLabel *iTLabel;

  QButtonGroup *fcsBox;
  QComboBox *focusCombo;
  QCheckBox *autoRaiseOn;
  QCheckBox *clickRaiseOn;
  KSlider *autoRaise;
  QLabel *alabel;
  QLCDNumber *s;
  //CT  QLabel *sec;

};

#endif

