/*
 * mouse.h
 *
 * Copyright (c) 1998 Matthias Ettrich <ettrich@kde.org>
 *
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

#ifndef __KKWMMOUSECONFIG_H__
#define __KKWMMOUSECONFIG_H__

#include <qlcdnum.h> 
#include <qlabel.h>
#include <qdialog.h>
#include <qmsgbox.h>
#include <qchkbox.h> 
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qcombo.h>
#include <kslider.h>

#include <kcontrol.h>

#include <kwm.h>


class KMouseConfig : public KConfigWidget
{
  Q_OBJECT

public:

  KMouseConfig( QWidget *parent=0, const char* name=0 );
  ~KMouseConfig( );

  void loadSettings();
  void applySettings();

public  slots:

private:

QComboBox* coTiAct1; 
QComboBox* coTiAct2; 
QComboBox* coTiAct3; 
QComboBox* coTiInAct1; 
QComboBox* coTiInAct2; 
QComboBox* coTiInAct3; 

QComboBox* coWin1; 
QComboBox* coWin2; 
QComboBox* coWin3; 

QComboBox* coAll1; 
QComboBox* coAll2; 
QComboBox* coAll3; 


  const char* functionTiAc(int); 
  const char* functionTiInAc(int); 
  const char* functionWin(int); 
  const char* functionAll(int); 

  void setComboText(QComboBox* combo, const char* text);
  
};

#endif

