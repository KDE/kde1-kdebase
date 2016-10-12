/*
 * keyboard.cpp
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
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <qfileinf.h> 
#include <qstring.h>
#include <qmsgbox.h> 

#include <kmsgbox.h> 
#include "keyboard.h"

#include <X11/Xlib.h>

#include "geom.h"

static bool GUI;

KeyboardConfig::~KeyboardConfig ()
{
  if (GUI)
    {
      delete click;
    }
}

KeyboardConfig::KeyboardConfig (QWidget * parent, const char *name, bool init)
    : KConfigWidget (parent, name)
{
  if (init)
    GUI = FALSE;
  else
    GUI = TRUE;

  if (GUI)
    {
      repeatBox = new QButtonGroup(klocale->translate("Keyboard repeat"), 
				   this);
      repeatOn = new QRadioButton(klocale->translate("On"), repeatBox);
      repeatOff = new QRadioButton(klocale->translate("Off"), repeatBox);
      
      click = new KSliderControl(klocale->translate("Key click volume"),
				 0, 100, 10, 100, "%", this);
      click->setLabelSize(0.3);
      click->setLabelAlignment(AlignLeft);
      click->setSteps(5,25);

      repeatOn->adjustSize();
      repeatOff->adjustSize();

      QFontMetrics fm = repeatBox->font();
      int titleH = fm.height();
      int titleW = fm.width(repeatBox->title()); 
      int buttonH = repeatOn->height();
      int buttonW = max( repeatOn->width(), repeatOff->width() );
      int boxH = 2*buttonH + 3*SPACE_YI + titleH;
      int boxW = 0;
      if (boxW < titleW + 4*SPACE_XI)
	boxW =  titleW + 4*SPACE_XI;
      if (boxW < buttonW + 2*SPACE_XI)
	boxW = buttonW + 2*SPACE_XI;
      
      repeatOn->move(SPACE_XI, SPACE_YI + titleH);
      repeatOff->move(SPACE_XI, 2*SPACE_YI + buttonH + titleH);
      repeatBox->move(SPACE_XO, SPACE_YO);
      repeatBox->resize(boxW, boxH);

      click->move(SPACE_XO, repeatBox->y() + repeatBox->height() + SPACE_YO);
      setMinimumSize( click->minimumSize().width() + 2*SPACE_XO,
		      click->minimumSize().height() + repeatBox->height() +
		      3*SPACE_YO );
    }

  config = kapp->getConfig();

  GetSettings();
}

void KeyboardConfig::resizeEvent(QResizeEvent *)
{
  // only the slider control can change width
  click->resize(width() - 2*SPACE_XO, click->minimumSize().height() );
}

// return the current LCD setting
int  KeyboardConfig::getRepeat()
{
  if (repeatOn->isChecked())
    return AutoRepeatModeOn;
  else
    return AutoRepeatModeOff;
}

int  KeyboardConfig::getClick()
{
  return click->intValue();
}

// set the slider and LCD values
void KeyboardConfig::setRepeat(int r)
{
  repeatOn->setChecked(FALSE);
  repeatOff->setChecked(FALSE);
  if (r == AutoRepeatModeOn)
    repeatOn->setChecked(TRUE);
  else
    repeatOff->setChecked(TRUE);
}

void KeyboardConfig::setClick(int v)
{
  click->setValue(v);
}

void KeyboardConfig::GetSettings( void )
{
  XKeyboardState kbd;

  XGetKeyboardControl(kapp->getDisplay(), &kbd);

  config->setGroup("Keyboard");
  QString key = config->readEntry("KeyboardRepeat");
  if (key == NULL)
      keyboardRepeat = kbd.global_auto_repeat;
  else if (key == "on")
    keyboardRepeat = AutoRepeatModeOn;
  else
    keyboardRepeat = AutoRepeatModeOff;

  clickVolume = config->readNumEntry("ClickVolume",-1);
  if (clickVolume == -1)
    clickVolume = kbd.key_click_percent;
  
  // the GUI should reflect the real values
  if (GUI)
    {
      setClick(kbd.key_click_percent);
      setRepeat(kbd.global_auto_repeat);
    }
}

void KeyboardConfig::saveParams( void )
{
  XKeyboardControl kbd;

  if (GUI)
    {
      clickVolume = getClick();
      keyboardRepeat = getRepeat();
    }

  kbd.key_click_percent = clickVolume;
  kbd.auto_repeat_mode = keyboardRepeat;
  XChangeKeyboardControl(kapp->getDisplay(), 
			 KBKeyClickPercent | KBAutoRepeatMode,
			 &kbd);

  config->setGroup("Keyboard");
  config->writeEntry("ClickVolume",clickVolume);
  if (keyboardRepeat == AutoRepeatModeOn)
    config->writeEntry("KeyboardRepeat","on");
  else
    config->writeEntry("KeyboardRepeat","off");
  config->sync();
}

void KeyboardConfig::loadSettings()
{
  GetSettings();
}

void KeyboardConfig::applySettings()
{
  saveParams();
}

void KeyboardConfig::defaultSettings()
{
  setClick(50);
  setRepeat(TRUE);
}


#include "keyboard.moc"
