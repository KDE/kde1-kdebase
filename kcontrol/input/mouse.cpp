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

#include <iostream.h> 

#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
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
      delete alabel;
      delete a;
      delete accel;
      delete tlabel;
      delete t;
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
      accel = new KSlider(1,20,2,20, KSlider::Horizontal, this, "acc");
      a = new QLCDNumber (2, this, "a");
      a->setFrameStyle( QFrame::NoFrame );
      alabel = new QLabel(klocale->translate("Acceleration"), this);
      connect( accel, SIGNAL(valueChanged(int)), a, SLOT(display(int)) );

      thresh = new KSlider(0,20,2,20, KSlider::Horizontal, this, "thr");
      t = new QLCDNumber (2, this, "t");
      t->setFrameStyle( QFrame::NoFrame );
      tlabel = new QLabel(klocale->translate("Threshold"), this);
      connect( thresh, SIGNAL(valueChanged(int)), t, SLOT(display(int)) );

      handedBox = new QButtonGroup(klocale->translate("Button mapping"), 
				   this, "handed");
      rightHanded = new QRadioButton(klocale->translate("Right handed"), 
				     handedBox, "R");
      leftHanded = new QRadioButton(klocale->translate("Left handed"), 
				    handedBox, "L");
 
      a->adjustSize();
      t->adjustSize();
      alabel->adjustSize();
      tlabel->adjustSize();
      rightHanded->adjustSize();
      leftHanded->adjustSize();
    }

  handedEnabled = TRUE;
  config = kapp->getConfig();

  GetSettings();
}

void MouseConfig::resizeEvent(QResizeEvent *)
{
  int h = SPACE_YO;
  int w = 0;
  int center;
  
  w = max( alabel->width(), tlabel->width() );

  alabel->move(SPACE_XO, h);
  QSize qsha = accel->sizeHint();
  accel->setGeometry(w + 2*SPACE_XO, h, 200, qsha.height());
  h += (QMAX( qsha.height(),alabel->height()) + SPACE_YI);

  center = accel->x() + ( accel->width() - a->width() )/2;

  a->move(center, h);
  h += a->height() + SPACE_YO;

  tlabel->move(SPACE_XO, h);
  QSize qsht = thresh->sizeHint();
  thresh->setGeometry(w + 2*SPACE_XO, h, 200, qsht.height());
  h += (QMAX( qsht.height(),tlabel->height()) + SPACE_YI);

  t->move(center, h);
  h += t->height() + SPACE_YO;

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
  int h_box = h;
  h += boxH + SPACE_YO; 

  rightHanded->move(SPACE_XI, SPACE_YI + titleH);
  leftHanded->move(SPACE_XI, 2*SPACE_YI + buttonH + titleH);
  handedBox->setGeometry(SPACE_XO, h_box, boxW, boxH);
}

int MouseConfig::getAccel()
{
  return a->intValue();
}

void MouseConfig::setAccel(int val)
{
  accel->setValue(val);
  a->display(val);
}

int MouseConfig::getThreshold()
{
  return t->intValue();
}

void MouseConfig::setThreshold(int val)
{
  thresh->setValue(val);
  t->display(val);
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

  
  unsigned char map[3];
  if (handedEnabled)
    {
      switch (num_buttons)
	{
	case 1:
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
	}
      int retval;
      while ((retval=XSetPointerMapping(kapp->getDisplay(), map, num_buttons))
	     == MappingBusy)
	/* keep trying until the pointer is free */ 
	;
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

#include "mouse.moc"
