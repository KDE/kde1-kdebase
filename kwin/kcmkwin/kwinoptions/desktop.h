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

#ifndef __KKWMDESKTOPCONFIG_H__
#define __KKWMDESKTOPCONFIG_H__

#include <qlcdnum.h> 
#include <qlabel.h>
#include <qdialog.h>
#include <qmsgbox.h>
#include <qchkbox.h> 
#include <qbttngrp.h>
#include <qpushbt.h>
#include <kslider.h>

#include <kcontrol.h>

#include <kwm.h>


class KDesktopConfig : public KConfigWidget
{
  Q_OBJECT

public:

  KDesktopConfig( QWidget *parent=0, const char* name=0 );
  ~KDesktopConfig( );

  void  resizeEvent(QResizeEvent *e);
  void SaveSettings( void );

  void loadSettings();
  void applySettings();

public  slots:

  void setEBorders();

  
private:

  void GetSettings( void );

  int getElectricBorders( void );
  bool getElectricBordersMovePointer( void );

  void setElectricBorders( int );
  void setElectricBordersMovePointer( bool );

  QButtonGroup *ElectricBox;
  QCheckBox *enable, *movepointer;
  KSlider *delayslider;
  QLabel *delaylabel;
  QLCDNumber *delaylcd;
  QLabel *sec;

};

#endif

