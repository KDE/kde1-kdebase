/*
 * $Id$
 *
 * Copyright (c) 1997 Bernd Johannes Wuebben wuebben@math.cornell.edu
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

#include <iostream.h> 

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <kapp.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "desktop.h"
#include "geom.h"
#include <ksimpleconfig.h>

// kwm config keywords
#define KWM_ELECTRIC_BORDER                  "ElectricBorder"
#define KWM_ELECTRIC_BORDER_MOVE_POINTER     "ElectricBorderMovePointer"

KDesktopConfig::~KDesktopConfig ()
{

  /*  delete ElectricBox;
  delete enable;
  delete movepointer;
  delete delayslider;
  delete delaylcd;
  delete delaylabel;
  delete sec;
  */
}

extern KSimpleConfig *config;

KDesktopConfig::KDesktopConfig (QWidget * parent, const char *name)
  : KConfigWidget (parent, name)
{


  ElectricBox = new QButtonGroup(klocale->translate("Active desktop borders"), this);
  enable= new 
    QCheckBox(klocale->translate("Enable active desktop borders"), ElectricBox);
  movepointer = new 
    QCheckBox(klocale->translate("Move pointer towards center after switch"),ElectricBox);

  delayslider = new KSlider(0,3000,100,0, KSlider::Horizontal, ElectricBox);
  delayslider->setSteps(100,100);

  delaylabel = 
    new QLabel(klocale->translate("Dekstop switch delay in ms:"), ElectricBox);
  delaylcd = new QLCDNumber (5, ElectricBox);
  delaylcd->setFrameStyle( QFrame::NoFrame );

  sec = new QLabel(klocale->translate("ms"),ElectricBox );
  connect( delayslider, SIGNAL(valueChanged(int)), delaylcd, SLOT(display(int)) );
  delaylcd->adjustSize();
  delaylabel->adjustSize();
  sec->adjustSize();

  connect( enable, SIGNAL(clicked()), this, SLOT(setEBorders()));

  GetSettings();
}



void KDesktopConfig::setEBorders(){

  if(enable->isChecked()){
    movepointer->setEnabled(true);
    delayslider->setEnabled(true);
    delaylabel->setEnabled(true);
  }
  else{
    delayslider->setEnabled(false);
    movepointer->setEnabled(false);
    delaylabel->setEnabled(false);
  }

}

int KDesktopConfig::getElectricBorders()
{
  if (enable->isChecked())
    return delayslider->value();
  else
    return -1;
}

bool KDesktopConfig::getElectricBordersMovePointer()
{
  return movepointer->isChecked();
}

void KDesktopConfig::setElectricBordersMovePointer(bool move){

  if(move){
    movepointer->setEnabled(true);
    movepointer->setChecked(true);
  }
  else{
    movepointer->setEnabled(false);
    movepointer->setChecked(false);
  }

  movepointer->setEnabled(enable->isChecked());

}

void KDesktopConfig::setElectricBorders(int delay)
{
  if (delay == -1)
  {
    enable->setChecked(false);
    movepointer->setEnabled(false);
    delayslider->setValue(0);
    delaylcd->display(0);
    
  }
  else
  {
    enable->setChecked(TRUE);
    movepointer->setEnabled(true);
    movepointer->setChecked(true);
    delayslider->setValue(delay);
    delaylcd->display(delay);
  }

  setEBorders();

}


void KDesktopConfig::GetSettings( void )
{
  QString key;

  config->setGroup( "General" );

  int value = config->readNumEntry(KWM_ELECTRIC_BORDER);
  setElectricBorders(value);

  bool  bvalue = (bool) config->readNumEntry(KWM_ELECTRIC_BORDER_MOVE_POINTER,1);
  setElectricBordersMovePointer(bvalue);

}

void KDesktopConfig::SaveSettings( void )
{
  int v;
  bool bv;
  config->setGroup( "General" );

  v = getElectricBorders();
  config->writeEntry(KWM_ELECTRIC_BORDER,v);

  bv = getElectricBordersMovePointer();
  config->writeEntry(KWM_ELECTRIC_BORDER_MOVE_POINTER,(int) bv);

  config->sync();

  // tell kwm to re-parse the config file
  //system("kwmclient configure");
  KWM::configureWm();
}

void KDesktopConfig::loadSettings()
{
  GetSettings();
}

void KDesktopConfig::applySettings()
{
  SaveSettings();
}

void KDesktopConfig::resizeEvent(QResizeEvent *)
{
  
  ElectricBox->setGeometry(SPACE_XO, 20,this->width() - 40, 200);
  enable->adjustSize();
  enable->setGeometry(20,20,enable->width(),30);

  movepointer->adjustSize();
  movepointer->setGeometry(20,20 + enable->height() ,movepointer->width(),30);

  delaylabel->adjustSize();
  delaylabel->setGeometry(20,
			  30 + enable->height() + movepointer->height()
			  ,delaylabel->width(),30);

  delayslider->setGeometry(20,
			   60 +enable->height() + movepointer->height(),
			   200,30);

  delaylcd->setGeometry(20 + delayslider->width()/2,
			90 +enable->height() + movepointer->height(),
			delaylcd->width(),delaylcd->height());

  sec->setGeometry( 20 + delayslider->width()/2 + delaylcd->width(),
		   100 +enable->height() + movepointer->height(),
			delaylcd->width(),delaylcd->height());



}

#include "desktop.moc"
