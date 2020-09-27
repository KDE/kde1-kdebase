/*
 * windows.cpp
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

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <qlayout.h> //CT 21Oct1998
#include <kapp.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "windows.h"
#include "geom.h"
#include <ksimpleconfig.h>

// kwm config keywords
#define KWM_MOVE      "WindowMoveType"
#define KWM_FOCUS     "FocusPolicy"
#define KWM_MAXIMIZE  "MaximizeOnlyVertically"
#define KWM_RESIZE_ANIM    "ResizeAnimation"
#define KWM_RESIZE_OPAQUE    "WindowResizeType"
#define KWM_AUTORAISE_INTERVAL "AutoRaiseInterval"
#define KWM_AUTORAISE "AutoRaise"
#define KWM_CLICKRAISE "ClickRaise"

// CT 19jan98
#define KWM_PLACEMENT "WindowsPlacement"

KWindowConfig::~KWindowConfig ()
{
}

extern KConfig *config;

//CT 19Oct1998 - rewritten
KWindowConfig::KWindowConfig (QWidget * parent, const char *name)
  : KConfigWidget (parent, name)
{

  QBoxLayout *lay = new QVBoxLayout (this, 5);

  windowsBox = new QButtonGroup(klocale->translate("Windows"), this);

  QBoxLayout *wLay = new QVBoxLayout (windowsBox,10,5);
  wLay->addSpacing(10);

  QBoxLayout *bLay = new QVBoxLayout(5);
  wLay->addLayout(bLay);

  QGridLayout *rLay = new QGridLayout(2,3,5);
  wLay->addLayout(rLay);
  rLay->setColStretch(0,0);
  rLay->setColStretch(1,1);

  //CT checkboxes: maximize, move, resize behaviour
  vertOnly = new QCheckBox(klocale->translate("Vertical maximization only by default"), windowsBox);
  vertOnly->adjustSize();
  vertOnly->setMinimumSize(vertOnly->size());
  bLay->addWidget(vertOnly);

  opaque = new QCheckBox(klocale->translate("Display content in moving windows"), windowsBox);
  opaque->adjustSize();
  opaque->setMinimumSize(opaque->size());
  bLay->addWidget(opaque);

  resizeOpaqueOn = new QCheckBox(klocale->translate("Display content in resizing windows"), windowsBox);
  resizeOpaqueOn->adjustSize();
  resizeOpaqueOn->setMinimumSize(resizeOpaqueOn->size());
  bLay->addWidget(resizeOpaqueOn);

  // resize animation - CT 27May98; 19Oct1998
  resizeAnimTitleLabel = new QLabel(klocale->translate("Resize animation:"),
				    windowsBox);
  resizeAnimTitleLabel->adjustSize();
  resizeAnimTitleLabel->setMinimumSize(resizeAnimTitleLabel->size());
  rLay->addWidget(resizeAnimTitleLabel,0,0);

  resizeAnimSlider = new KSlider(0,10,10,0,KSlider::Horizontal, windowsBox);
  resizeAnimSlider->setSteps(10,1);
  //  resizeAnimSlider->setFixedHeight(20);
  resizeAnimSlider->adjustSize();
  resizeAnimSlider->setMinimumSize(resizeAnimSlider->size());
  rLay->addMultiCellWidget(resizeAnimSlider,0,0,1,2);

  resizeAnimNoneLabel= new QLabel(klocale->translate("None"),windowsBox);
  resizeAnimNoneLabel->adjustSize();
  resizeAnimNoneLabel->setMinimumSize(resizeAnimNoneLabel->size());
  resizeAnimNoneLabel->setAlignment(AlignTop|AlignLeft);
  rLay->addWidget(resizeAnimNoneLabel,1,1);

  resizeAnimFastLabel= new QLabel(klocale->translate("Fast"),windowsBox);
  resizeAnimFastLabel->adjustSize();
  resizeAnimFastLabel->setMinimumSize(resizeAnimFastLabel->size());
  resizeAnimFastLabel->setAlignment(AlignTop|AlignRight);
  rLay->addWidget(resizeAnimFastLabel,1,2);

  wLay->activate();

  lay->addWidget(windowsBox);

  // placement policy --- CT 19jan98, 13mar98 ---
  plcBox = new QButtonGroup(klocale->translate("Placement policy"),this);

  QGridLayout *pLay = new QGridLayout(plcBox,3,3,10,5);
  pLay->addRowSpacing(0,10);

  placementCombo = new QComboBox(FALSE, plcBox);
  placementCombo->insertItem(klocale->translate(i18n("Smart")),
			     SMART_PLACEMENT);
  placementCombo->insertItem(klocale->translate(i18n("Cascade")),
			     CASCADE_PLACEMENT);
  placementCombo->insertItem(klocale->translate(i18n("Interactive")),
			     INTERACTIVE_PLACEMENT);
  placementCombo->insertItem(klocale->translate(i18n("Random")),
			     RANDOM_PLACEMENT);
  placementCombo->insertItem(klocale->translate(i18n("Manual")),
			     MANUAL_PLACEMENT);
  placementCombo->setCurrentItem(SMART_PLACEMENT);
  placementCombo->adjustSize();
  placementCombo->setMinimumSize(placementCombo->size());
  placementCombo->setMaximumHeight(placementCombo->height());
  pLay->addWidget(placementCombo,1,0);

  //CT 13mar98 interactive trigger config
  connect(placementCombo, SIGNAL(activated(int)),this,
	  SLOT(ifPlacementIsInteractive()) );

  iTLabel = new QLabel(klocale->translate(i18n("  Allowed overlap:\n"
					       "(% of desktop space)")),
		       plcBox);
  iTLabel->adjustSize();
  iTLabel->setMinimumSize(iTLabel->size());
  iTLabel->setAlignment(AlignTop|AlignHCenter);
  pLay->addWidget(iTLabel,1,1);

  interactiveTrigger = new KNumericSpinBox(plcBox);
  interactiveTrigger->setRange(0,500);
  interactiveTrigger->adjustSize();
  interactiveTrigger->setMinimumSize(interactiveTrigger->size());
  pLay->addWidget(interactiveTrigger,1,2);

  pLay->addRowSpacing(2,10);

  pLay->activate();

  lay->addWidget(plcBox);

  // focus policy
  fcsBox = new QButtonGroup(klocale->translate("Focus policy"),this);

  QGridLayout *fLay = new QGridLayout(fcsBox,5,3,10,5);
  fLay->addRowSpacing(0,10);
  fLay->setColStretch(0,0);
  fLay->setColStretch(1,1);
  fLay->setColStretch(2,1);


  focusCombo =  new QComboBox(FALSE, fcsBox);
  focusCombo->insertItem(klocale->translate("Click to focus"),
			 CLICK_TO_FOCUS);
  focusCombo->insertItem(klocale->translate("Focus follows mouse"),
			 FOCUS_FOLLOWS_MOUSE);
  focusCombo->insertItem(klocale->translate("Classic focus follows mouse"),
			 CLASSIC_FOCUS_FOLLOWS_MOUSE);
  focusCombo->insertItem(klocale->translate("Classic sloppy focus"),
			 CLASSIC_SLOPPY_FOCUS);
  focusCombo->adjustSize();
  focusCombo->setMinimumSize(focusCombo->size());
  focusCombo->setMaximumHeight(focusCombo->height());
  fLay->addMultiCellWidget(focusCombo,1,1,0,1);

  connect(focusCombo, SIGNAL(activated(int)),this,
	  SLOT(setAutoRaiseEnabled()) );

  // autoraise delay

  autoRaiseOn = new QCheckBox(klocale->translate("Auto Raise"), fcsBox);
  autoRaiseOn->adjustSize();
  autoRaiseOn->setMinimumSize(autoRaiseOn->size());
  fLay->addWidget(autoRaiseOn,2,0);
  connect(autoRaiseOn,SIGNAL(toggled(bool)), this, SLOT(autoRaiseOnTog(bool)));

  clickRaiseOn = new QCheckBox(klocale->translate("Click Raise"), fcsBox);
  clickRaiseOn->adjustSize();
  clickRaiseOn->setMinimumSize(clickRaiseOn->size());
  fLay->addWidget(clickRaiseOn,3,0);

  connect(clickRaiseOn,SIGNAL(toggled(bool)), this, SLOT(clickRaiseOnTog(bool)));

  alabel = new QLabel(klocale->translate("Delay (ms)"), fcsBox);
  alabel->adjustSize();
  alabel->setMinimumSize(alabel->size());
  alabel->setAlignment(AlignVCenter|AlignHCenter);
  fLay->addWidget(alabel,2,1);

  s = new QLCDNumber (4, fcsBox);
  s->setFrameStyle( QFrame::NoFrame );
  s->setFixedHeight(30);
  s->adjustSize();
  s->setMinimumSize(s->size());
  fLay->addWidget(s,2,2);

  autoRaise = new KSlider(0,3000,100,500, KSlider::Horizontal, fcsBox);
  autoRaise->setSteps(100,100);
  autoRaise->setFixedHeight(20);
  autoRaise->adjustSize();
  autoRaise->setMinimumSize(alabel->width()+s->width(), autoRaise->height());
  fLay->addMultiCellWidget(autoRaise,3,3,1,2);

  connect( autoRaise, SIGNAL(valueChanged(int)), s, SLOT(display(int)) );

  fLay->activate();

  lay->addWidget(fcsBox);

  lay->activate();

  GetSettings();
}

int KWindowConfig::getMove()
{
  if (opaque->isChecked())
    return OPAQUE;
  else
    return TRANSPARENT;
}

void KWindowConfig::setMove(int trans)
{
  if (trans == TRANSPARENT)
    opaque->setChecked(FALSE);
  else
    opaque->setChecked(TRUE);
}

// placement policy --- CT 31jan98 ---
int KWindowConfig::getPlacement()
{
  return placementCombo->currentItem();
}

void KWindowConfig::setPlacement(int plac)
{
  placementCombo->setCurrentItem(plac);
}



int KWindowConfig::getFocus()
{
    return focusCombo->currentItem();
}

void KWindowConfig::setFocus(int foc)
{
  focusCombo->setCurrentItem(foc);
}

int KWindowConfig::getResizeAnim()
{
  return resizeAnimSlider->value();
}

void KWindowConfig::setResizeAnim(int anim)
{
  resizeAnimSlider->setValue(anim);
}

int KWindowConfig::getResizeOpaque()
{
  if (resizeOpaqueOn->isChecked())
    return RESIZE_OPAQUE;
  else
    return RESIZE_TRANSPARENT;
}

void KWindowConfig::setResizeOpaque(int opaque)
{
  if (opaque == RESIZE_OPAQUE)
    resizeOpaqueOn->setChecked(TRUE);
  else
    resizeOpaqueOn->setChecked(FALSE);
}

void KWindowConfig::setMaximize(int tb)
{
  if (tb == MAXIMIZE_FULL)
    vertOnly->setChecked(FALSE);
  else
    vertOnly->setChecked(TRUE);
}

int KWindowConfig::getMaximize()
{
  if (vertOnly->isChecked())
    return MAXIMIZE_VERT;
  else
    return MAXIMIZE_FULL;
}

void KWindowConfig::setAutoRaiseInterval(int tb)
{
    autoRaise->setValue(tb);
    s->display(tb);
}

int KWindowConfig::getAutoRaiseInterval()
{
    return s->intValue();
}


void KWindowConfig::setAutoRaise(bool on)
{
    autoRaiseOn->setChecked(on);
}

void KWindowConfig::setClickRaise(bool on)
{
    clickRaiseOn->setChecked(on);
}

void KWindowConfig::setAutoRaiseEnabled()
{
  // the auto raise related widgets are: autoRaise, alabel, s, sec
  if ( focusCombo->currentItem() != CLICK_TO_FOCUS )
    {
      autoRaiseOn->setEnabled(TRUE);
      autoRaiseOnTog(autoRaiseOn->isChecked());
      clickRaiseOn->setEnabled(TRUE);
      clickRaiseOnTog(clickRaiseOn->isChecked());
    }
  else
    {
      autoRaiseOn->setEnabled(FALSE);
      autoRaiseOnTog(FALSE);
      clickRaiseOn->setEnabled(FALSE);
      clickRaiseOnTog(FALSE);
    }
}

// CT 13mar98 interactiveTrigger configured by this slot
void KWindowConfig::ifPlacementIsInteractive( )
{
  if( placementCombo->currentItem() == INTERACTIVE_PLACEMENT) {
    iTLabel->setEnabled(TRUE);
    interactiveTrigger->show();
  }
  else {
    iTLabel->setEnabled(FALSE);
    interactiveTrigger->hide();
  }
}
//CT

//CT 23Oct1998 make AutoRaise toggling much clear
void KWindowConfig::autoRaiseOnTog(bool a) {
  autoRaise->setEnabled(a);
  s->setEnabled(a);
  alabel->setEnabled(a);
}
//CT

void KWindowConfig::clickRaiseOnTog(bool a) {
}

void KWindowConfig::GetSettings( void )
{
  QString key;

  config->setGroup( "General" );

  key = config->readEntry(KWM_MOVE, "Opaque");
  if( key == "Transparent")
    setMove(TRANSPARENT);
  else if( key == "Opaque")
    setMove(OPAQUE);

  //CT 17Jun1998 - variable animation speed from 0 (none!!) to 10 (max)
  int anim = 1;
  if (config->hasKey(KWM_RESIZE_ANIM)) {
    anim = config->readNumEntry(KWM_RESIZE_ANIM);
    if( anim < 1 ) anim = 0;
    if( anim > 10 ) anim = 10;
    setResizeAnim(anim);
  }
  else{
    setResizeAnim(1);
    config->writeEntry(KWM_RESIZE_ANIM, 1);
  }

  key = config->readEntry(KWM_RESIZE_OPAQUE, "Opaque");
  if( key == "Opaque")
    setResizeOpaque(RESIZE_OPAQUE);
  else if ( key == "Transparent")
    setResizeOpaque(RESIZE_TRANSPARENT);

  // placement policy --- CT 19jan98 ---
  key = config->readEntry(KWM_PLACEMENT);
  //CT 13mar98 interactive placement
  if( key.left(11) == "interactive") {
    setPlacement(INTERACTIVE_PLACEMENT);
    int comma_pos = key.find(',');
    if (comma_pos < 0)
      interactiveTrigger->setValue(0);
    else
      interactiveTrigger->setValue (key.right(key.length()
					      - comma_pos).toUInt(0));
    iTLabel->setEnabled(TRUE);
    interactiveTrigger->show();
  }
  else {
    interactiveTrigger->setValue(0);
    iTLabel->setEnabled(FALSE);
    interactiveTrigger->hide();
    if( key == "random")
      setPlacement(RANDOM_PLACEMENT);
    else if( key == "cascade")
      setPlacement(CASCADE_PLACEMENT); //CT 31jan98
    //CT 31mar98 manual placement
    else if( key == "manual")
      setPlacement(MANUAL_PLACEMENT);

    else
      setPlacement(SMART_PLACEMENT);
  }

  key = config->readEntry(KWM_FOCUS);
  if( key == "ClickToFocus")
    setFocus(CLICK_TO_FOCUS);
  else if( key == "FocusFollowMouse")
    setFocus(FOCUS_FOLLOWS_MOUSE);
  else if(key == "ClassicFocusFollowMouse")
    setFocus(CLASSIC_FOCUS_FOLLOWS_MOUSE);
  else if(key == "ClassicSloppyFocus")
    setFocus(CLASSIC_SLOPPY_FOCUS);

  key = config->readEntry(KWM_MAXIMIZE);
  if( key == "on")
    setMaximize(MAXIMIZE_VERT);
  else if( key == "off")
    setMaximize(MAXIMIZE_FULL);

  int k = config->readNumEntry(KWM_AUTORAISE_INTERVAL,0);
  setAutoRaiseInterval(k);

  key = config->readEntry(KWM_AUTORAISE);
  setAutoRaise(key == "on");
  key = config->readEntry(KWM_CLICKRAISE);
  setClickRaise(key != "off");
  setAutoRaiseEnabled();      // this will disable/hide the auto raise delay widget if focus==click
}

void KWindowConfig::SaveSettings( void )
{
  int v;
  config->setGroup( "General" );

  v = getMove();
  if (v == TRANSPARENT)
    config->writeEntry(KWM_MOVE,"Transparent");
  else
    config->writeEntry(KWM_MOVE,"Opaque");


  // placement policy --- CT 31jan98 ---
  v =getPlacement();
  if (v == RANDOM_PLACEMENT)
    config->writeEntry(KWM_PLACEMENT, "random");
  else if (v == CASCADE_PLACEMENT)
    config->writeEntry(KWM_PLACEMENT, "cascade");
  //CT 13mar98 manual and interactive placement
  else if (v == MANUAL_PLACEMENT)
    config->writeEntry(KWM_PLACEMENT, "manual");
  else if (v == INTERACTIVE_PLACEMENT) {
    char tmpstr[20];
    sprintf(tmpstr,"interactive,%d",interactiveTrigger->getValue());
    config->writeEntry(KWM_PLACEMENT, tmpstr);
  }
  else
    config->writeEntry(KWM_PLACEMENT, "smart");


  v = getFocus();
  if (v == CLICK_TO_FOCUS)
    config->writeEntry(KWM_FOCUS,"ClickToFocus");
  else if (v == CLASSIC_SLOPPY_FOCUS)
    config->writeEntry(KWM_FOCUS,"ClassicSloppyFocus");
  else if (v == CLASSIC_FOCUS_FOLLOWS_MOUSE)
    config->writeEntry(KWM_FOCUS,"ClassicFocusFollowMouse");
  else
    config->writeEntry(KWM_FOCUS,"FocusFollowMouse");

  //CT - 17Jun1998
  config->writeEntry(KWM_RESIZE_ANIM, getResizeAnim());


  v = getResizeOpaque();
  if (v == RESIZE_OPAQUE)
    config->writeEntry(KWM_RESIZE_OPAQUE, "Opaque");
  else
    config->writeEntry(KWM_RESIZE_OPAQUE, "Transparent");

  v = getMaximize();
  if (v == MAXIMIZE_VERT)
    config->writeEntry(KWM_MAXIMIZE, "on");
  else
    config->writeEntry(KWM_MAXIMIZE, "off");

  v = getAutoRaiseInterval();
  if (v <0) v = 0;
  config->writeEntry(KWM_AUTORAISE_INTERVAL,v);

  if (autoRaiseOn->isChecked())
    config->writeEntry(KWM_AUTORAISE, "on");
  else
    config->writeEntry(KWM_AUTORAISE, "off");

  if (clickRaiseOn->isChecked())
    config->writeEntry(KWM_CLICKRAISE, "on");
  else
    config->writeEntry(KWM_CLICKRAISE, "off");

  config->sync();

}

void KWindowConfig::loadSettings()
{
  GetSettings();
}

void KWindowConfig::applySettings()
{
  SaveSettings();
}

#include "windows.moc"
