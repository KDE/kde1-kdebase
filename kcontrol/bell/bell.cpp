/*
 * bell.cpp
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
#include <stdio.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
 
#include <qfileinf.h> 
#include <qstring.h>
#include <qmsgbox.h> 

#include <kmsgbox.h> 
#include "bell.h"

#include <X11/Xlib.h>

#include "geom.h"

static bool GUI;

KBellConfig::~KBellConfig ()
{
  if (GUI)
    {
      delete volume;
      delete pitch;
      delete duration;
      delete v;
      delete p;
      delete d;
      delete vlabel;
      delete plabel;
      delete dlabel;
      delete percent;
      delete hertz;
      delete ms;
      delete test;
    }
}

KBellConfig::KBellConfig (QWidget * parent, const char *name, bool init)
    : KConfigWidget (parent, name)
{
  if (init)
    GUI = FALSE;
  else
    GUI = TRUE;

  if (GUI)
    {
      volume = new KSlider(0,100,10,100, KSlider::Horizontal,    this, "vol");
      pitch = new KSlider(0,2000,200,2000, KSlider::Horizontal, this, "pitch");
      duration = new KSlider(0,500,50,500, KSlider::Horizontal, this, "dur");
      
      v = new QLCDNumber (3, this, "v");
      p = new QLCDNumber (4, this, "p");
      d = new QLCDNumber (3, this, "d");
      v->setFrameStyle( QFrame::NoFrame );
      p->setFrameStyle( QFrame::NoFrame );
      d->setFrameStyle( QFrame::NoFrame );

      vlabel = new QLabel(klocale->translate("Volume"), this);
      plabel = new QLabel(klocale->translate("Pitch"), this);
      dlabel = new QLabel(klocale->translate("Duration"), this);
      percent = new QLabel("%", this);
      hertz = new QLabel(klocale->translate("Hz"), this);
      ms = new QLabel(klocale->translate("ms"), this);

      test = new QPushButton((klocale->translate("Test"),this,"test");

      connect( volume,   SIGNAL(valueChanged(int)), v, SLOT(display(int)) );
      connect( pitch,    SIGNAL(valueChanged(int)), p, SLOT(display(int)) );
      connect( duration, SIGNAL(valueChanged(int)), d, SLOT(display(int)) );
      connect( test, SIGNAL(clicked()), SLOT(ringBell()));

      v->adjustSize();
      p->adjustSize();
      d->adjustSize();
      vlabel->adjustSize();
      plabel->adjustSize();
      dlabel->adjustSize();
      percent->adjustSize();
      hertz->adjustSize();
      ms->adjustSize();
      test->adjustSize();
    }

  config = kapp->getConfig();

  GetSettings();

  if (init)
    saveParams();
}

void KBellConfig::resizeEvent(QResizeEvent *)
{
  int h = SPACE_YO;
  int w = 0;

  w = max(w, vlabel->width());
  w = max(w, plabel->width());
  w = max(w, dlabel->width());

  vlabel->move(SPACE_XO, h);
  volume->setGeometry(w + 2*SPACE_XO, h, 200, vlabel->height());
  h += vlabel->height() + SPACE_YI;

  int center = volume->x() + ( volume->width() - v->width() )/2;

  int dh = ( v->height() - volume->height() )/2;

  v->move(center, h);
  percent->move(v->x() + v->width() + 3, h+dh);
  h += v->height() + SPACE_YO;

  plabel->move(SPACE_XO, h);
  pitch->setGeometry(w + 2*SPACE_XO, h, 200, vlabel->height());
  h += plabel->height() + SPACE_YI;

  p->move(center, h);
  hertz->move(p->x() + p->width() + 3, h+dh);
  h += p->height() + SPACE_YO;

  dlabel->move(SPACE_XO, h);
  duration->setGeometry(w + 2*SPACE_XO, h, 200, vlabel->height());
  h += dlabel->height() + SPACE_YI;

  d->move(center, h);
  ms->move(d->x() + d->width() + 3, h+dh);
  h += d->height() + SPACE_YO;

  test->move(SPACE_XO, h);
}

// set the slider and the LCD to 'val'
void KBellConfig::setBellVolume(int val)
{
  volume->setValue(val);
  v->display(val);
}

void KBellConfig::setBellPitch(int val)
{
  pitch->setValue(val);
  p->display(val);
}

void KBellConfig::setBellDuration(int val)
{
  duration->setValue(val);
  d->display(val);
}

// return the current LCD setting
int  KBellConfig::getBellVolume()
{
  return v->intValue();
}

int  KBellConfig::getBellPitch()
{
  return p->intValue();
}

int  KBellConfig::getBellDuration()
{
  return d->intValue();
}

void KBellConfig::GetSettings( void )
{
  XKeyboardState kbd;

  config->setGroup("Bell");
  bellVolume = config->readNumEntry("Volume",-1);
  bellPitch = config->readNumEntry("Pitch",-1);
  bellDuration = config->readNumEntry("Duration",-1);

  XGetKeyboardControl(kapp->getDisplay(), &kbd);

  // if the config file didn't have anything, use the X server settings
  if (bellVolume == -1)
    bellVolume = kbd.bell_percent;
  if (bellPitch == -1)
    bellPitch = kbd.bell_pitch;
  if (bellDuration == -1)
    bellDuration = kbd.bell_duration;
  
  // the GUI should reflect the real values
  if (GUI)
    {
      setBellVolume(kbd.bell_percent);
      setBellPitch(kbd.bell_pitch);
      setBellDuration(kbd.bell_duration);
    }
}

void KBellConfig::saveParams( void )
{
  XKeyboardControl kbd;

  if (GUI)
    {
      bellVolume = getBellVolume();
      bellPitch = getBellPitch();
      bellDuration = getBellDuration();
    }
  kbd.bell_percent = bellVolume;
  kbd.bell_pitch = bellPitch;
  kbd.bell_duration = bellDuration;
  XChangeKeyboardControl(kapp->getDisplay(), 
			 KBBellPercent | KBBellPitch | KBBellDuration,
			 &kbd);

  config->setGroup("Bell");
  config->writeEntry("Volume",bellVolume);
  config->writeEntry("Pitch",bellPitch);
  config->writeEntry("Duration",bellDuration);
  config->sync();
}

void KBellConfig::ringBell()
{
  // store the old state
  XKeyboardState old_state;
  XGetKeyboardControl(kapp->getDisplay(), &old_state);
  
  // switch to the test state
  XKeyboardControl kbd;
  kbd.bell_percent = getBellVolume();
  kbd.bell_pitch = getBellPitch();
  kbd.bell_duration = getBellDuration();
  XChangeKeyboardControl(kapp->getDisplay(), 
			 KBBellPercent | KBBellPitch | KBBellDuration,
			 &kbd);
  // ring bell
  XBell(kapp->getDisplay(),100);
  
  // restore old state
  kbd.bell_percent = old_state.bell_percent;
  kbd.bell_pitch = old_state.bell_pitch;
  kbd.bell_duration = old_state.bell_duration;
  XChangeKeyboardControl(kapp->getDisplay(), 
			 KBBellPercent | KBBellPitch | KBBellDuration,
			 &kbd);
}

void KBellConfig::loadSettings()
{
  GetSettings();
}

void KBellConfig::applySettings()
{
  saveParams();
}


#include "bell.moc"
