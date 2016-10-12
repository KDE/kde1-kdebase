/*
 * mouse.cpp
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
#include "mouse.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "geom.h"

static bool GUI;

MouseConfig::~MouseConfig ()
{
  if (GUI)
    {
      delete accel;
      delete thresh;
      delete leftHanded;
      delete rightHanded;
      delete handedBox;
    }
}

MouseConfig::MouseConfig (QWidget * parent, const char *name, bool init)
  : KConfigWidget (parent, name)
{
  if (init)
    GUI = FALSE;
  else
    GUI = TRUE;

  if (GUI)
    {
      accel = new KSliderControl(klocale->translate("Acceleration"), 
				 1,20,2,20, "x", 
				 this);
      thresh = new KSliderControl(klocale->translate("Threshold"), 
				  1,20,2,20, klocale->translate("pixels"), 
				  this);
      accel->setLabelSize(0.3);
      thresh->setLabelSize(0.3);
      accel->setLabelAlignment(AlignLeft);
      thresh->setLabelAlignment(AlignLeft);
      accel->setSteps(1,20);
      thresh->setSteps(1,20);

      handedBox = new QButtonGroup(klocale->translate("Button mapping"), 
				   this, "handed");
      rightHanded = new QRadioButton(klocale->translate("Right handed"), 
				     handedBox, "R");
      leftHanded = new QRadioButton(klocale->translate("Left handed"), 
				    handedBox, "L");

      accel->move(SPACE_XO, SPACE_YO);
      accel->resize(accel->minimumSize().width(), 
		    accel->minimumSize().height());
      thresh->move(SPACE_XO, accel->y() + accel->height() + SPACE_YO);
      thresh->resize(thresh->minimumSize().width(), 
		    thresh->minimumSize().height());

      rightHanded->adjustSize();
      leftHanded->adjustSize();

      QFontMetrics fm = handedBox->font();
      int titleH = fm.height();
      int titleW = fm.width(handedBox->title());
      int buttonH = rightHanded->height();
      int buttonW = max( rightHanded->width(), leftHanded->width() );
      int boxH = 2*buttonH + 3*SPACE_YI + titleH;
      int boxW = 0;
      if (boxW < titleW + 4*SPACE_XI)
	boxW =  titleW + 4*SPACE_XI;
      if (boxW < buttonW + 2*SPACE_XI)
	boxW = buttonW + 2*SPACE_XI;
      
      rightHanded->move(SPACE_XI, SPACE_YI + titleH);
      leftHanded->move(SPACE_XI, 2*SPACE_YI + buttonH + titleH);
      handedBox->move(SPACE_XO, thresh->y() + thresh->height() + SPACE_YO);
      handedBox->resize(boxW, boxH);

      // determine minimum size
      int w = max(accel->minimumSize().width(), thresh->minimumSize().width());
      int h = accel->minimumSize().height() + thresh->minimumSize().height() +
	boxH + 4*SPACE_YO;
      setMinimumSize(w,h);
    }

  handedEnabled = TRUE;
  config = kapp->getConfig();

  GetSettings();
}

void MouseConfig::resizeEvent(QResizeEvent *)
{
  accel->resize(width() - 2*SPACE_XO, accel->minimumSize().height());
  thresh->resize(width() - 2*SPACE_XO, thresh->minimumSize().height());
}

int MouseConfig::getAccel()
{
  return accel->intValue();
}

void MouseConfig::setAccel(int val)
{
  accel->setValue(val);
}

int MouseConfig::getThreshold()
{
  return thresh->intValue();
}

void MouseConfig::setThreshold(int val)
{
  thresh->setValue(val);
}


int MouseConfig::getHandedness()
{
  if (rightHanded->isChecked())
    return RIGHT_HANDED;
  else
    return LEFT_HANDED;
}

void MouseConfig::setHandedness(int val)
{
  rightHanded->setChecked(FALSE);
  leftHanded->setChecked(FALSE);
  if (val == RIGHT_HANDED)
    rightHanded->setChecked(TRUE);
  else
    leftHanded->setChecked(TRUE);
}

void MouseConfig::GetSettings( void )
{
  int accel_num, accel_den, threshold;
  XGetPointerControl( kapp->getDisplay(), 
		      &accel_num, &accel_den, &threshold );
  accel_num /= accel_den;   // integer acceleration only

  // get settings from X server
  int h = RIGHT_HANDED;
  unsigned char map[5];
  num_buttons = XGetPointerMapping(kapp->getDisplay(), map, 5);
      
  switch (num_buttons)
    {
    case 1:
      /* disable button remapping */
      if (GUI)
	{
	  rightHanded->setEnabled(FALSE);
	  leftHanded->setEnabled(FALSE);
	  handedEnabled = FALSE;
	}
      break;
    case 2:
      if ( (int)map[0] == 1 && (int)map[1] == 2 )
	h = RIGHT_HANDED;
      else if ( (int)map[0] == 2 && (int)map[1] == 1 )
	h = LEFT_HANDED;
      else
	{
	  /* custom button setup: disable button remapping */
	  if (GUI)
	    {
	      rightHanded->setEnabled(FALSE);
	      leftHanded->setEnabled(FALSE);
	    }
	}
      break;
    case 3:
      middle_button = (int)map[1];
      if ( (int)map[0] == 1 && (int)map[2] == 3 )
	h = RIGHT_HANDED;
      else if ( (int)map[0] == 3 && (int)map[2] == 1 )
	h = LEFT_HANDED;
      else
	{
	  /* custom button setup: disable button remapping */
	  if (GUI)
	    {
	      rightHanded->setEnabled(FALSE);
	      leftHanded->setEnabled(FALSE);
	      handedEnabled = FALSE;
	    }
	}
      break;
    default:
      /* custom setup with > 3 buttons: disable button remapping */
      if (GUI)
	{
	  rightHanded->setEnabled(FALSE);
	  leftHanded->setEnabled(FALSE);
	  handedEnabled = FALSE;
	}
      break;
    }

  config->setGroup("Mouse");
  int a = config->readNumEntry("Acceleration",-1);
  if (a == -1)
    accelRate = accel_num;
  else
    accelRate = a;

  int t = config->readNumEntry("Threshold",-1);
  if (t == -1)
    thresholdMove = threshold;
  else
    thresholdMove = t;

  QString key = config->readEntry("MouseButtonMapping");
  if (key == "RightHanded")
    handed = RIGHT_HANDED;
  else if (key == "LeftHanded")
    handed = LEFT_HANDED;
  else if (key == NULL)
    handed = h;

  // the GUI should always show the real values
  if (GUI)
    {
      setAccel(accel_num);
      setThreshold(threshold);
      setHandedness(h);
    }
}

void MouseConfig::saveParams( void )
{
  if (GUI)
    {
      accelRate = getAccel();
      thresholdMove = getThreshold();
      handed = getHandedness();
    }

  XChangePointerControl( kapp->getDisplay(),
			 TRUE, TRUE, accelRate, 1, thresholdMove);

  
  unsigned char map[5];
  int remap=1;
  if (handedEnabled)
    {
      switch (num_buttons)
	{
	case 1:
          {
 	    map[0] = (unsigned char) 1;
          }
	  break;
	case 2:
	  if (handed == RIGHT_HANDED)
	    {
	      map[0] = (unsigned char) 1;
	      map[1] = (unsigned char) 3;
	    }
	  else
	    {
	      map[0] = (unsigned char) 3;
	      map[1] = (unsigned char) 1;
	    }
	  break;
	case 3:
	  if (handed == RIGHT_HANDED)
	    {
	      map[0] = (unsigned char) 1;
	      map[1] = (unsigned char) middle_button;
	      map[2] = (unsigned char) 3;
	    }
	  else
	    {
	      map[0] = (unsigned char) 3;
	      map[1] = (unsigned char) middle_button;
	      map[2] = (unsigned char) 1;
	    }
	  break;
        case 5:  
         // Intellimouse case, where buttons 1-3 are left, middle, and
         // right, and 4-5 are up/down
         if (handed == RIGHT_HANDED)
           {
             map[0] = (unsigned char) 1;
             map[1] = (unsigned char) 2;
             map[2] = (unsigned char) 3;
             map[3] = (unsigned char) 4;
             map[4] = (unsigned char) 5;
           }
         else
           {
             map[0] = (unsigned char) 3;
             map[1] = (unsigned char) 2;
             map[2] = (unsigned char) 1;
             map[3] = (unsigned char) 4;
             map[4] = (unsigned char) 5;
           }
         break;
       default:
         {
           //catch-all for mice with four or more than five buttons
           //Without this, XSetPointerMapping is called with a undefined value
           //for map
           remap=0;  //don't remap
         }
         break;
	}
      int retval;
      if (remap)
       while ((retval=XSetPointerMapping(kapp->getDisplay(), map,
                                         num_buttons)) == MappingBusy)
         /* keep trying until the pointer is free */
         { };
    }

  config->setGroup("Mouse");
  config->writeEntry("Acceleration",accelRate);
  config->writeEntry("Threshold",thresholdMove);
  if (handed == RIGHT_HANDED)
    config->writeEntry("MouseButtonMapping","RightHanded");
  else
    config->writeEntry("MouseButtonMapping","LeftHanded");
}

void MouseConfig::loadSettings()
{
  GetSettings();
}

void MouseConfig::applySettings()
{
  saveParams();
}

void MouseConfig::defaultSettings()
{
  setThreshold(2);
  setAccel(2);
  setHandedness(RIGHT_HANDED);
}


#include "mouse.moc"
