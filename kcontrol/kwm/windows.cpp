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
#define KWM_RESIZE    "ResizeAnimation"
#define KWM_AUTORAISE "AutoRaise"

// CT 19jan98
#define KWM_PLACEMENT "WindowsPlacement"

KWindowConfig::~KWindowConfig ()
{
  delete transparent;
  delete opaque;
  delete clickTo;
  delete followMouse;
  delete animOn;
  delete animOff;
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

  // resize animation
  resizeBox = new QButtonGroup(klocale->translate("Resize Animation"), this);
  animOn = new QRadioButton(klocale->translate("On"), resizeBox);
  animOff = new QRadioButton(klocale->translate("Off"), resizeBox);

  // placement policy --- CT 19jan98 ---
  placementBox = new QButtonGroup(klocale->translate("Placement policy"), this);
  random = new QRadioButton(klocale->translate("Random placement"), placementBox);
  smart = new QRadioButton(klocale->translate("Smart placement"), placementBox);

  // maximize behaviour
  maximizeBox = new QButtonGroup(klocale->translate("Maximize Style"), this);
  fullScreen =  new QRadioButton(klocale->translate("Maximize fully"), maximizeBox);
  vertOnly = new QRadioButton(klocale->translate("Maximize vertically"), maximizeBox);

  // focus policy
  focusBox = new QButtonGroup(klocale->translate("Focus Policy"), this);
  clickTo =  new QRadioButton(klocale->translate("Click to focus"), focusBox);
  followMouse = new QRadioButton(klocale->translate("Focus follows mouse"), focusBox);

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
  connect( clickTo, SIGNAL(clicked()), this, SLOT(setAutoRaiseEnabled()));
  connect( followMouse, SIGNAL(clicked()), this, SLOT(setAutoRaiseEnabled()));

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
  animOn->adjustSize();
  animOn->move(SPACE_XI, SPACE_YI + titleH);
  animOff->adjustSize();
  animOff->move(SPACE_XI, 2*SPACE_YI + buttonH + titleH);
  int h2 = h;
  h += boxH + SPACE_YO;

  titleW = fm.width(resizeBox->title());
  buttonW = max( animOff->width(), animOn->width() );
  if (boxW < titleW + 4*SPACE_XI)
    boxW =  titleW + 4*SPACE_XI;
  if (boxW < buttonW + 2*SPACE_XI)
    boxW = buttonW + 2*SPACE_XI;

  // placement policy --- CT 19jan98 ---
  random->adjustSize();
  random->move(SPACE_XI, SPACE_YI + titleH);
  smart->adjustSize();
  smart->move(SPACE_XI, 2*SPACE_YI + buttonH + titleH);
  int h3 = h;

  titleW = fm.width(placementBox->title());
  buttonW = max( smart->width(), random->width() );
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
  clickTo->adjustSize();
  clickTo->move(SPACE_XI, SPACE_YI + titleH);
  followMouse->adjustSize();
  followMouse->move(SPACE_XI, 2*SPACE_YI + buttonH + titleH);
  int h5 = h;
  h += boxH + SPACE_YO;

  titleW = fm.width(focusBox->title());
  buttonW = max( clickTo->width(), followMouse->width() );
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

// placement policy --- CT 19jan98 ---
int KWindowConfig::getPlacement()
{
  if (random->isChecked())
    return RANDOM_PLACEMENT;
  else
    return SMART_PLACEMENT;
}

void KWindowConfig::setPlacement(int plac)
{ 
  if (plac == RANDOM_PLACEMENT)
    {
      random->setChecked(TRUE);
      smart->setChecked(FALSE);
    }
  else
    {
      smart->setChecked(TRUE);
      random->setChecked(FALSE);
    }
}



int KWindowConfig::getFocus()
{
  if (clickTo->isChecked())
    return CLICK_TO_FOCUS;
  else
    return FOCUS_FOLLOW_MOUSE;
}

void KWindowConfig::setFocus(int foc)
{
  if (foc == CLICK_TO_FOCUS)
  {
    clickTo->setChecked(TRUE);
    followMouse->setChecked(FALSE);
  }
  else
  {
    followMouse->setChecked(TRUE); 
    clickTo->setChecked(FALSE);
  }
}

int KWindowConfig::getAnim()
{
  if (animOn->isChecked())
    return RESIZE_ANIM_ON;
  else
    return RESIZE_ANIM_OFF;
}

void KWindowConfig::setAnim(int tb)
{
  if (tb == RESIZE_ANIM_ON)
  {
    animOn->setChecked(TRUE);
    animOff->setChecked(FALSE);
  }
  else
  {
    animOff->setChecked(TRUE);
    animOn->setChecked(FALSE);
  }
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
  if ( followMouse->isChecked() )
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

void KWindowConfig::GetSettings( void )
{
  QString key;

  config->setGroup( "General" );

  key = config->readEntry(KWM_MOVE);
  if( key == "Transparent")
    setMove(TRANSPARENT);
  else if( key == "Opaque")
    setMove(OPAQUE);

  key = config->readEntry(KWM_RESIZE);
  if( key == "on")
    setAnim(RESIZE_ANIM_ON);
  else if( key == "off")
    setAnim(RESIZE_ANIM_OFF);

  // placement policy --- CT 19jan98 ---
  key = config->readEntry(KWM_PLACEMENT);
  if( key == "random")
    setPlacement(RANDOM_PLACEMENT);
  else if( key == "smart")
    setPlacement(SMART_PLACEMENT);


  key = config->readEntry(KWM_FOCUS);
  if( key == "ClickToFocus")
    setFocus(CLICK_TO_FOCUS);
  else if( key == "FocusFollowMouse")
    setFocus(FOCUS_FOLLOW_MOUSE);

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


  // placement policy --- CT 19jan98 ---
  v =getPlacement();
  if (v == RANDOM_PLACEMENT)
    config->writeEntry(KWM_PLACEMENT, "random");
  else
    config->writeEntry(KWM_PLACEMENT, "smart");


  v = getFocus();
  if (v == CLICK_TO_FOCUS)
    config->writeEntry(KWM_FOCUS,"ClickToFocus");
  else
    config->writeEntry(KWM_FOCUS,"FocusFollowMouse");
     
  v = getAnim();
  if (v == RESIZE_ANIM_ON)
    config->writeEntry(KWM_RESIZE, "on");
  else
    config->writeEntry(KWM_RESIZE, "off");

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
