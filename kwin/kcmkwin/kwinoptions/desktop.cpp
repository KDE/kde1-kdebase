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

#include <qlayout.h> //CT
#include <kapp.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "desktop.h"
#include "geom.h"

// kwm config keywords
#define KWM_ELECTRIC_BORDER                  "ElectricBorder"
#define KWM_ELECTRIC_BORDER_DELAY            "ElectricBorderNumberOfPushes"
#define KWM_ELECTRIC_BORDER_MOVE_POINTER     "ElectricBorderPointerWarp"

//CT 15mar 98 - magics
#define KWM_BRDR_SNAP_ZONE                   "BorderSnapZone"
#define KWM_WNDW_SNAP_ZONE                   "WindowSnapZone"

#define MAX_BRDR_SNAP                          50
#define MAX_WNDW_SNAP                          50
#define MAX_EDGE_RES                         1000

//CT 21Oct1998 - emptied
KDesktopConfig::~KDesktopConfig ()
{
}

extern KConfig *config;

//CT 21Oct1998 - rewritten for using layouts
KDesktopConfig::KDesktopConfig (QWidget * parent, const char *name)
  : KConfigWidget (parent, name)
{

  QBoxLayout *lay = new QVBoxLayout(this, 5);

  ElectricBox = new QButtonGroup(klocale->translate("Active desktop borders"),
				 this);

  QGridLayout *eLay = new QGridLayout(ElectricBox,5,3,10,5);
  eLay->addRowSpacing(0,10);
  eLay->setColStretch(0,0);
  eLay->setColStretch(1,1);

  enable= new
    QCheckBox(klocale->translate("Enable active desktop borders"),
	      ElectricBox);
  enable->adjustSize();
  enable->setMinimumSize(enable->size());
  eLay->addMultiCellWidget(enable,1,1,0,1);

  movepointer = new
    QCheckBox(klocale->translate("Move pointer towards center after switch"),
	      ElectricBox);
  movepointer->adjustSize();
  movepointer->setMinimumSize(movepointer->size());
  eLay->addMultiCellWidget(movepointer,2,2,0,1);

  delaylabel = new QLabel(klocale->translate("Desktop switch delay:"),
			  ElectricBox);
  delaylabel->adjustSize();
  delaylabel->setMinimumSize(delaylabel->size());
  delaylabel->setAlignment(AlignVCenter|AlignLeft);
  eLay->addWidget(delaylabel,3,0);

  delaylcd = new QLCDNumber (2, ElectricBox);
  delaylcd->setFrameStyle( QFrame::NoFrame );
  delaylcd->setFixedHeight(30);
  delaylcd->adjustSize();
  delaylcd->setMinimumSize(delaylcd->size());
  eLay->addWidget(delaylcd,3,1);

  delayslider = new KSlider(0,MAX_EDGE_RES/10,10,0,
			    KSlider::Horizontal, ElectricBox);
  delayslider->setSteps(10,10);
  delayslider->adjustSize();
  delayslider->setMinimumSize(delaylabel->width(), delayslider->height());
  eLay->addMultiCellWidget(delayslider,4,4,1,2);

  connect( delayslider, SIGNAL(valueChanged(int)), delaylcd, SLOT(display(int)) );

  connect( enable, SIGNAL(clicked()), this, SLOT(setEBorders()));

  eLay->activate();

  lay->addWidget(ElectricBox,5);

  //CT 15mar98 - add EdgeResistance, BorderAttractor, WindowsAttractor config
  MagicBox = new QButtonGroup(klocale->translate("Magic Borders"), this);

  eLay = new QGridLayout(MagicBox,4,3,10,5);
  eLay->addRowSpacing(0,10);
  eLay->addRowSpacing(2,10);
  eLay->setColStretch(0,0);
  eLay->setColStretch(1,0);
  eLay->setColStretch(2,1);

  BrdrSnapLabel = new QLabel(klocale->translate("Border Snap Zone:\n       (pixels)"), MagicBox);
  BrdrSnapLabel->adjustSize();
  BrdrSnapLabel->setMinimumSize(BrdrSnapLabel->size());
  BrdrSnapLabel->setAlignment(AlignTop);
  eLay->addWidget(BrdrSnapLabel,1,0);

  BrdrSnapLCD = new QLCDNumber (2, MagicBox);
  BrdrSnapLCD->setFrameStyle( QFrame::NoFrame );
  BrdrSnapLCD->setFixedHeight(30);
  BrdrSnapLCD->adjustSize();
  BrdrSnapLCD->setMinimumSize(BrdrSnapLCD->size());
  eLay->addWidget(BrdrSnapLCD,1,1);

  BrdrSnapSlider = new KSlider(0,MAX_BRDR_SNAP,1,0,
			       KSlider::Horizontal, MagicBox);
  BrdrSnapSlider->setSteps(1,1);
  BrdrSnapSlider->adjustSize();
  BrdrSnapSlider->setMinimumSize( BrdrSnapLabel->width()+
				  BrdrSnapLCD->width(),
				  BrdrSnapSlider->height());
  eLay->addWidget(BrdrSnapSlider,1,2);
  eLay->addRowSpacing(0,5);

  connect( BrdrSnapSlider, SIGNAL(valueChanged(int)), BrdrSnapLCD, SLOT(display(int)) );

  WndwSnapLabel = new QLabel(klocale->translate("Window Snap Zone:\n       (pixels)"), MagicBox);
  WndwSnapLabel->adjustSize();
  WndwSnapLabel->setMinimumSize(WndwSnapLabel->size());
  WndwSnapLabel->setAlignment(AlignTop);
  eLay->addWidget(WndwSnapLabel,3,0);

  WndwSnapLCD = new QLCDNumber (2, MagicBox);
  WndwSnapLCD->setFrameStyle( QFrame::NoFrame );
  WndwSnapLCD->setFixedHeight(30);
  WndwSnapLCD->adjustSize();
  WndwSnapLCD->setMinimumSize(WndwSnapLCD->size());
  eLay->addWidget(WndwSnapLCD,3,1);

  WndwSnapSlider = new KSlider(0,MAX_WNDW_SNAP,1,0,
			       KSlider::Horizontal, MagicBox);
  WndwSnapSlider->setSteps(1,1);
  WndwSnapSlider->adjustSize();
  WndwSnapSlider->setMinimumSize( WndwSnapLabel->width()+
				  WndwSnapLCD->width(),
				  WndwSnapSlider->height());
  eLay->addWidget(WndwSnapSlider,3,2);

  connect( WndwSnapSlider, SIGNAL(valueChanged(int)), WndwSnapLCD, SLOT(display(int)) );

  eLay->activate();

  lay->addWidget(MagicBox,5);

  lay->activate();

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

bool KDesktopConfig::getElectricBorders()
{
  return  enable->isChecked();
}

int KDesktopConfig::getElectricBordersDelay(){
    return delayslider->value();
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

void KDesktopConfig::setElectricBorders(bool b){
    enable->setChecked(b);
    setEBorders();
}

void KDesktopConfig::setElectricBordersDelay(int delay)
{
    delayslider->setValue(delay);
    delaylcd->display(delay);

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
  setElectricBorders(v != -1);

  v = config->readNumEntry(KWM_ELECTRIC_BORDER_DELAY);
  setElectricBordersDelay(v);

  //CT 17mar98 re-allign this reading with the one in kwm  ("on"/"off")
  // matthias: this is obsolete now. Should be fixed in 1.1 with NoWarp, MiddleWarp, FullWarp
  key = config->readEntry(KWM_ELECTRIC_BORDER_MOVE_POINTER);
  if (key == "MiddleWarp")
    setElectricBordersMovePointer(TRUE);

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

  v = getElectricBordersDelay()>10?80*getElectricBordersDelay():800;
  if (getElectricBorders())
    config->writeEntry(KWM_ELECTRIC_BORDER,v);
  else
    config->writeEntry(KWM_ELECTRIC_BORDER,-1);


  config->writeEntry(KWM_ELECTRIC_BORDER_DELAY,getElectricBordersDelay());

  bv = getElectricBordersMovePointer();
  config->writeEntry(KWM_ELECTRIC_BORDER_MOVE_POINTER,bv?"MiddleWarp":"NoWarp");

  config->sync();

  //CT 15mar98 - magics
  v = getBorderSnapZone();
  config->writeEntry(KWM_BRDR_SNAP_ZONE,v);

  v = getWindowSnapZone();
  config->writeEntry(KWM_WNDW_SNAP_ZONE,v);

}

void KDesktopConfig::loadSettings()
{
  GetSettings();
}

void KDesktopConfig::applySettings()
{
  SaveSettings();
}



#include "desktop.moc"
