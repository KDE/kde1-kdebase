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

#include <iostream.h> 

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

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
#define KWM_AUTORAISE "AutoRaise"

// CT 19jan98
#define KWM_PLACEMENT "WindowsPlacement"

KWindowConfig::~KWindowConfig ()
{
  delete transparent;
  delete opaque;
  delete placementCombo;    
  delete iTLabel;                       
  delete interactiveTrigger;
  delete placementBox;      
  delete focusCombo;
  delete resizeAnimOn;
  delete resizeOpaqueOn;
  delete moveBox;
  delete resizeBox;
  delete focusBox;
}

extern KSimpleConfig *config;

KWindowConfig::KWindowConfig (QWidget * parent, const char *name)
  : KConfigWidget (parent, name)
{
  // move type
  moveBox = new QButtonGroup(klocale->translate("Window movement"), this);
  transparent = new QRadioButton(klocale->translate("Transparent"), moveBox);
  opaque = new QRadioButton(klocale->translate("Opaque"), moveBox);

  // resize animation - CT 27May98
  resizeBox = new QButtonGroup(klocale->translate("Window resize"), this);
  resizeAnimOn = new QCheckBox(klocale->translate("With animation"), 
				  resizeBox);
  resizeOpaqueOn = new QCheckBox(klocale->translate("Opaque"), resizeBox);

  // placement policy --- CT 19jan98, 13mar98 ---
  placementBox = new QButtonGroup(klocale->translate("Placement policy"), this);
  placementCombo = new QComboBox(FALSE, placementBox);
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
  //CT 13mar98 interactive trigger config

  connect(placementCombo, SIGNAL(activated(int)),this,
	  SLOT(ifPlacementIsInteractive()) );
  iTLabel = new QLabel(klocale->translate(i18n("Allowed Overlap:")), 
		       placementBox);
  interactiveTrigger = new KNumericSpinBox(placementBox);
  interactiveTrigger->setRange(0,500);
  
  
  // maximize behaviour
  maximizeBox = new QButtonGroup(klocale->translate("Maximize Style"), this);
  fullScreen =  new QRadioButton(klocale->translate("Maximize fully"), maximizeBox);
  vertOnly = new QRadioButton(klocale->translate("Maximize vertically"), maximizeBox);

  // focus policy
  focusBox = new QButtonGroup(klocale->translate("Focus Policy"), this);
  focusCombo =  new QComboBox(FALSE, focusBox);
  focusCombo->insertItem(klocale->translate("Click to focus"), 
			 CLICK_TO_FOCUS);
  focusCombo->insertItem(klocale->translate("Focus follows mouse"), 
			 FOCUS_FOLLOWS_MOUSE);
  focusCombo->insertItem(klocale->translate("Classic focus follows mouse"),
			 CLASSIC_FOCUS_FOLLOWS_MOUSE);
  focusCombo->insertItem(klocale->translate("Classic sloppy focus"), 
			 CLASSIC_SLOPPY_FOCUS);
  connect(focusCombo, SIGNAL(activated(int)),this,
	  SLOT(setAutoRaiseEnabled()) );


  // autoraise delay
  autoRaise = new KSlider(0,3000,100,0, KSlider::Horizontal, this);
  autoRaise->setSteps(100,100);
  alabel = new QLabel(klocale->translate("Auto raise delay\n(<100 disables)"), this);
  s = new QLCDNumber (5, this);
  s->setFrameStyle( QFrame::NoFrame );
  sec = new QLabel(klocale->translate("miliseconds"), this);
  connect( autoRaise, SIGNAL(valueChanged(int)), s, SLOT(display(int)) );
  s->adjustSize();
  alabel->adjustSize();
  sec->adjustSize();

  GetSettings();
}

void KWindowConfig::resizeEvent(QResizeEvent *)
{
  int h = SPACE_YO;
  int boxW = 0;

  QFontMetrics fm = moveBox->font();
  int titleW;
  int buttonW;
  int titleH = fm.height();

  // move type
  transparent->adjustSize();
  opaque->adjustSize();
  
  int buttonH = transparent->height();
  int boxH = 2*buttonH + 3*SPACE_YI + titleH;

  transparent->move(SPACE_XI, SPACE_YI + titleH);
  opaque->move(SPACE_XI, 2*SPACE_YI + buttonH + titleH);
  int h1 = h;

  titleW = fm.width(moveBox->title());
  buttonW = max( transparent->width(), opaque->width() );
  if (boxW < titleW + 4*SPACE_XI)
    boxW =  titleW + 4*SPACE_XI;
  if (boxW < buttonW + 2*SPACE_XI)
    boxW = buttonW + 2*SPACE_XI;

  // resize animation
  resizeAnimOn->adjustSize();
  resizeAnimOn->move(SPACE_XI, SPACE_YI + titleH);
  resizeOpaqueOn->adjustSize();
  resizeOpaqueOn->move(SPACE_XI, 2*SPACE_YI + buttonH + titleH);
  int h2 = h;
  h += boxH + SPACE_YO;

  titleW = fm.width(resizeBox->title());
  buttonW = max( resizeAnimOn->width(), resizeOpaqueOn->width() );
  if (boxW < titleW + 4*SPACE_XI)
    boxW =  titleW + 4*SPACE_XI;
  if (boxW < buttonW + 2*SPACE_XI)
    boxW = buttonW + 2*SPACE_XI;

  // placement policy --- CT 31jan98 ---
  placementCombo->adjustSize();
  iTLabel->adjustSize();
  if (placementCombo->width() < (boxW - 2*SPACE_XI)) 
    placementCombo->setGeometry(SPACE_XI,
				SPACE_YI + titleH,
				boxW - 2*SPACE_XI,
				placementCombo->height());
  else
    placementCombo->move(SPACE_XI, SPACE_YI + titleH);
  //CT 13mar98
  iTLabel->move(SPACE_XI, 2*SPACE_YI+placementCombo->height() + titleH/2);
  interactiveTrigger->setGeometry(3/2*SPACE_XI+iTLabel->width(),
				  2*SPACE_YI+placementCombo->height() 
				  + titleH/2,
				  40,20);
  int h3 = h;

  titleW = fm.width(placementBox->title());
  buttonW = placementCombo->width();
  if (boxW < titleW + 4*SPACE_XI)
    boxW =  titleW + 4*SPACE_XI;
  if (boxW < buttonW + 2*SPACE_XI)
    boxW = buttonW + 2*SPACE_XI;

  // maximize behaviour
  fullScreen->adjustSize();
  fullScreen->move(SPACE_XI, SPACE_YI + titleH);
  vertOnly->adjustSize();
  vertOnly->move(SPACE_XI, 2*SPACE_YI + buttonH + titleH);
  int h4 = h;
  h += boxH + SPACE_YO;

  titleW = fm.width(maximizeBox->title());
  buttonW = max( fullScreen->width(), vertOnly->width() );
  if (boxW < titleW + 4*SPACE_XI)
    boxW =  titleW + 4*SPACE_XI;
  if (boxW < buttonW + 2*SPACE_XI)
    boxW = buttonW + 2*SPACE_XI;

    // focus policy
  focusCombo->adjustSize();
  focusCombo->move(SPACE_XI, SPACE_YI + titleH);
  if (focusCombo->width() < (boxW - 2*SPACE_XI)) 
    focusCombo->setGeometry(SPACE_XI,
				SPACE_YI + titleH,
				boxW - 2*SPACE_XI,
				focusCombo->height());
  else
    focusCombo->move(SPACE_XI, SPACE_YI + titleH);

  int h5 = h;
  h += boxH + SPACE_YO;

  titleW = fm.width(focusBox->title());
  buttonW = focusCombo->width();
  if (boxW < titleW + 4*SPACE_XI)
    boxW =  titleW + 4*SPACE_XI;
  if (boxW < buttonW + 2*SPACE_XI)
    boxW = buttonW + 2*SPACE_XI;

  moveBox->setGeometry(SPACE_XO, h1, boxW, boxH);
  resizeBox->setGeometry(moveBox->x() + boxW + SPACE_XO, h2, boxW, boxH);
  placementBox->setGeometry(SPACE_XO, h3, boxW, boxH);
  maximizeBox->setGeometry(placementBox->x()+ boxW + SPACE_XO, h4, boxW, boxH);
  focusBox->setGeometry(SPACE_XO + boxW/2, h5, boxW, boxH);

  int w = alabel->width();
  alabel->move(SPACE_XO, h);
  autoRaise->setGeometry(w + 2*SPACE_XO, h, 200, alabel->height());
  h += autoRaise->height() ;
  int center = autoRaise->x() + ( autoRaise->width() - s->width() )/2;
  int dh = ( s->height() - sec->height() )/2;
  s->move(center, h);
  sec->move(s->x() + s->width() + 3, h+dh);
  h += s->height() + SPACE_YO;
}

int KWindowConfig::getMove()
{
  if (transparent->isChecked())
    return TRANSPARENT;
  else
    return OPAQUE;
}

void KWindowConfig::setMove(int trans)
{
  if (trans == TRANSPARENT)
  {
    transparent->setChecked(TRUE);
    opaque->setChecked(FALSE);
  }
  else
  {
    opaque->setChecked(TRUE);
    transparent->setChecked(FALSE);
  }
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
  if (resizeAnimOn->isChecked())
    return RESIZE_ANIM_ON;
  else
    return RESIZE_ANIM_OFF;
}

void KWindowConfig::setResizeAnim(int anim)
{
  if (anim == RESIZE_ANIM_ON)
    resizeAnimOn->setChecked(TRUE);
  else
    resizeAnimOn->setChecked(FALSE);
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
  {
    fullScreen->setChecked(TRUE);
    vertOnly->setChecked(FALSE);
  }
  else
  {
    vertOnly->setChecked(TRUE);
    fullScreen->setChecked(FALSE);
  }
}

int KWindowConfig::getMaximize()
{
  if (fullScreen->isChecked())
    return MAXIMIZE_FULL;
  else
    return MAXIMIZE_VERT;
}

void KWindowConfig::setAutoRaise(int tb)
{
  autoRaise->setValue(tb);
  s->display(tb);
}

int KWindowConfig::getAutoRaise()
{
  return s->intValue();
}

void KWindowConfig::setAutoRaiseEnabled( )
{
  // the auto raise related widgets are: autoRaise, alabel, s, sec
  if ( focusCombo->currentItem() != CLICK_TO_FOCUS )
    {
      autoRaise->show();
      alabel->show();
      s->show();
      sec->show();
    }
  else
    {
      autoRaise->hide();
      alabel->hide();
      s->hide();
      sec->hide();
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


void KWindowConfig::GetSettings( void )
{
  QString key;

  config->setGroup( "General" );

  key = config->readEntry(KWM_MOVE);
  if( key == "Transparent")
    setMove(TRANSPARENT);
  else if( key == "Opaque")
    setMove(OPAQUE);

  key = config->readEntry(KWM_RESIZE_ANIM);
  if( key == "on")
    setResizeAnim(RESIZE_ANIM_ON);
  else if( key == "off")
    setResizeAnim(RESIZE_ANIM_OFF);

  key = config->readEntry(KWM_RESIZE_OPAQUE);
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

  int k = config->readNumEntry(KWM_AUTORAISE,0);
  setAutoRaise(k);

  // this will disable/hide the auto raise delay widget if focus==click
  setAutoRaiseEnabled();
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
  
  v = getResizeAnim();
  if (v == RESIZE_ANIM_ON)
    config->writeEntry(KWM_RESIZE_ANIM, "on");
  else
    config->writeEntry(KWM_RESIZE_ANIM, "off");

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

  v = getAutoRaise();
  if (v <100) v = 0; //interval set less than 100 disables AutoRaise
  config->writeEntry(KWM_AUTORAISE,v);

  config->sync();

  // tell kwm to re-parse the config file
  //system("kwmclient configure");
  KWM::configureWm();
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
