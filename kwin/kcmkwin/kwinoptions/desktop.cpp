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

//CT 15mar 98 - magics
#define KWM_BRDR_SNAP_ZONE                   "BorderSnapZone"
#define KWM_WNDW_SNAP_ZONE                   "WindowSnapZone"

#define MAX_BRDR_SNAP                          50
#define MAX_WNDW_SNAP                          50
#define MAX_EDGE_RES                         1000

KDesktopConfig::~KDesktopConfig ()
{

  //delete ElectricBox;
  delete enable;
  delete movepointer;
  delete delayslider;
  delete delaylcd;
  delete delaylabel;
  delete sec;
  
  //CT 15mar98 - safer? Maybe
  delete ElectricBox;
  
  delete BrdrSnapLabel; delete BrdrSnapSlider; delete BrdrSnapLCD;
  delete WndwSnapLabel; delete WndwSnapSlider; delete WndwSnapLCD;
  delete MagicBox;
  //CT ---
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
    new QLabel(klocale->translate("Dekstop switch delay:"), ElectricBox);
  delaylcd = new QLCDNumber (5, ElectricBox);
  delaylcd->setFrameStyle( QFrame::NoFrame );

  sec = new QLabel(klocale->translate("ms"),ElectricBox );
  connect( delayslider, SIGNAL(valueChanged(int)), delaylcd, SLOT(display(int)) );
  delaylcd->adjustSize();
  delaylabel->adjustSize();
  sec->adjustSize();

  connect( enable, SIGNAL(clicked()), this, SLOT(setEBorders()));

  //CT 15mar98 - add EdgeResistance, BorderAttractor, WindowsAttractor config
  MagicBox = new QButtonGroup(klocale->translate("Magic Borders"), this);

  BrdrSnapLabel = new QLabel(klocale->translate("Border Snap Zone:\n       (pixels)"), MagicBox);

  BrdrSnapSlider = new KSlider(0,50,1,0, KSlider::Horizontal, MagicBox);
  BrdrSnapSlider->setSteps(1,1);

  BrdrSnapLCD = new QLCDNumber (5, MagicBox);
  BrdrSnapLCD->setFrameStyle( QFrame::NoFrame );

  connect( BrdrSnapSlider, SIGNAL(valueChanged(int)), BrdrSnapLCD, SLOT(display(int)) );
  BrdrSnapLabel->adjustSize();
  BrdrSnapSlider->adjustSize();
  BrdrSnapLCD->adjustSize();


  WndwSnapLabel = new QLabel(klocale->translate("Window Snap Zone:\n       (pixels)"), MagicBox);
  WndwSnapSlider = new KSlider(0,50,1,0, KSlider::Horizontal, MagicBox);
  WndwSnapSlider->setSteps(1,1);

  WndwSnapLCD = new QLCDNumber (5, MagicBox);
  WndwSnapLCD->setFrameStyle( QFrame::NoFrame );
  
  connect( WndwSnapSlider, SIGNAL(valueChanged(int)), WndwSnapLCD, SLOT(display(int)) );
  WndwSnapLabel->adjustSize();
  WndwSnapSlider->adjustSize();
  WndwSnapLCD->adjustSize();
  

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


//CT 15mar98 - magics

int KDesktopConfig::getBorderSnapZone() {
  return BrdrSnapSlider->value();
}

void KDesktopConfig::setBorderSnapZone(int pxls) {
  BrdrSnapSlider->setValue(pxls);
  BrdrSnapLCD->display(pxls);
}

int KDesktopConfig::getWindowSnapZone() {
  return WndwSnapSlider->value();
}

void KDesktopConfig::setWindowSnapZone(int pxls) {
  WndwSnapSlider->setValue(pxls);
  WndwSnapLCD->display(pxls);
}
//CT ---

void KDesktopConfig::GetSettings( void )
{
  int v;
  QString key;

  config->setGroup( "General" );

  v = config->readNumEntry(KWM_ELECTRIC_BORDER);
  setElectricBorders(v);

  //CT 17mar98 re-allign this reading with the one in kwm  ("on"/"off")
  key = config->readEntry(KWM_ELECTRIC_BORDER_MOVE_POINTER);
  if (key == "on") setElectricBordersMovePointer(TRUE);
  else if (key == "off") setElectricBordersMovePointer(FALSE);
  else {
    setElectricBordersMovePointer(FALSE);
    config->writeEntry(KWM_ELECTRIC_BORDER_MOVE_POINTER,"off");
  }

  //CT 15mar98 - magics
  v = config->readNumEntry(KWM_BRDR_SNAP_ZONE);
  if (v > MAX_BRDR_SNAP) setBorderSnapZone(MAX_BRDR_SNAP);
  else if (v < 0) setBorderSnapZone (0);
  else setBorderSnapZone(v);

  v = config->readNumEntry(KWM_WNDW_SNAP_ZONE);
  if (v > MAX_WNDW_SNAP) setWindowSnapZone(MAX_WNDW_SNAP);
  else if (v < 0) setWindowSnapZone (0);
  else setWindowSnapZone(v);
  //CT ---
}

void KDesktopConfig::SaveSettings( void )
{
  int v;
  bool bv;
  config->setGroup( "General" );

  v = getElectricBorders();
  config->writeEntry(KWM_ELECTRIC_BORDER,v);

  bv = getElectricBordersMovePointer();
  config->writeEntry(KWM_ELECTRIC_BORDER_MOVE_POINTER,bv?"on":"off");

  config->sync();

  //CT 15mar98 - magics
  v = getBorderSnapZone();
  config->writeEntry(KWM_BRDR_SNAP_ZONE,v);

  v = getWindowSnapZone();
  config->writeEntry(KWM_WNDW_SNAP_ZONE,v);

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
  int xpos = SPACE_XO, ypos = SPACE_YO;
  
  ElectricBox->setGeometry(SPACE_XO, SPACE_YO,this->width() - 40, 200);
  enable->adjustSize();
  enable->setGeometry(SPACE_XO,ypos,enable->width(),30);

  movepointer->adjustSize();
  movepointer->setGeometry(SPACE_XO,ypos = ypos + enable->height() ,movepointer->width(),30);

  delaylabel->adjustSize();
  delaylabel->setGeometry(xpos,
			  ypos + movepointer->height() + 
			  (delaylcd->height()-delaylabel->height())/2,
			  delaylabel->width(),30);

  delaylcd->setGeometry(xpos = xpos + delaylabel->width() + SPACE_XI/2,
			ypos = ypos + movepointer->height() + SPACE_YI,
			delaylcd->width(),delaylcd->height());

  sec->setGeometry( xpos + delaylcd->width() + SPACE_XI/2,
		    ypos + (delaylcd->height()-sec->height())/2,
		    delaylcd->width(),delaylcd->height());

  delayslider->setGeometry(xpos - delaylabel->width()/2,
			   ypos = ypos + delaylcd->height() + SPACE_YI,
			   200,30);
  //rearrange
  ElectricBox->setGeometry(SPACE_XO, SPACE_YO,this->width() - 40, ypos + 30 + SPACE_YI);
  

  MagicBox->setGeometry (SPACE_XO, SPACE_YO + SPACE_YI + ElectricBox->height(), 
			 this->width() - 40, 200);

  // the other box's internal arrangement
  xpos = BrdrSnapLabel->width() + BrdrSnapLCD->width();
  if ((WndwSnapLabel->width() + WndwSnapLCD->width()) > xpos)
    xpos = WndwSnapLabel->width() + WndwSnapLCD->width();
  
  xpos = xpos + SPACE_XO;
  ypos = SPACE_YO;

  BrdrSnapLabel->setGeometry(SPACE_XO,
			     ypos + (BrdrSnapLCD->height() - BrdrSnapLabel->height())/2,
			     BrdrSnapLabel->width(), BrdrSnapLabel->height());
  BrdrSnapLCD->setGeometry(xpos - BrdrSnapLCD->width(),
			   ypos,
			   BrdrSnapLCD->width(), BrdrSnapLCD->height());
  BrdrSnapSlider->setGeometry(xpos,
			      ypos = ypos + (BrdrSnapLCD->height() -20)/2,
			      180, 30);

  ypos = ypos + BrdrSnapLCD->height() + SPACE_YI;
  WndwSnapLabel->setGeometry(SPACE_XO,
			     ypos + (WndwSnapLCD->height() - WndwSnapLabel->height())/2,
			     WndwSnapLabel->width(), WndwSnapLabel->height());
  WndwSnapLCD->setGeometry(xpos - WndwSnapLCD->width() ,
			   ypos,
			   WndwSnapLCD->width(), WndwSnapLCD->height());
  WndwSnapSlider->setGeometry(xpos,
			      ypos + (WndwSnapLCD->height()-20)/2,
			      180, 30);

  MagicBox->setGeometry (SPACE_XO, SPACE_YO + SPACE_YI + ElectricBox->height(), 
			 this->width() - 40, ypos + WndwSnapLCD->height() + SPACE_YI);

}

#include "desktop.moc"
