/*
 *
 * Copyright (c) 1998 Matthias Ettrich <ettrich@kde.org>
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

#include <kapp.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "mouse.h"
#include "geom.h"
#include <qlayout.h>


KMouseConfig::~KMouseConfig ()
{

}

extern KConfig *config;

KMouseConfig::KMouseConfig (QWidget * parent, const char *name)
  : KConfigWidget (parent, name)
{
  QGridLayout *layout = new QGridLayout( this, 12, 4, 10, 1);
  layout->setColStretch( 2, 100 );
  layout->setColStretch( 3, 100 );

  QLabel* label;

  label = new QLabel(i18n("Active"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 0,2, AlignHCenter);

  label = new QLabel(i18n("Inactive"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 0,3, AlignHCenter);

  label = new QLabel(i18n("Titlebar and frame:"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addMultiCellWidget(label, 0,0,0,1);

  label = new QLabel(i18n("Inactive inner window:"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addMultiCellWidget(label, 4,4,0,3);

  label = new QLabel(i18n("Inner window, titlebar and frame:"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addMultiCellWidget(label, 8,8,0,3);

  label = new QLabel(i18n("Left Button"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 1,1);

  label = new QLabel(i18n("Middle Button"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 2,1);

  label = new QLabel(i18n("Right Button"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 3,1);

  label = new QLabel(i18n("Left Button"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 5,1);

  label = new QLabel(i18n("Middle Button"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 6,1);

  label = new QLabel(i18n("Right Button"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 7,1);

  label = new QLabel(i18n("ALT + Left Button"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 9,1);

  label = new QLabel(i18n("ALT + Middle Button"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 10,1);

  label = new QLabel(i18n("ALT + Right Button"), this);
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label, 11,1);

  QComboBox* combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Operations menu"));
  combo->insertItem(i18n("Toggle raise and lower"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addWidget(combo, 1,2);
  coTiAct1 = combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Operations menu"));
  combo->insertItem(i18n("Toggle raise and lower"));
  combo->insertItem(i18n("Nothing"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addWidget(combo, 2,2);
  coTiAct2 = combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Operations menu"));
  combo->insertItem(i18n("Toggle raise and lower"));
  combo->insertItem(i18n("Nothing"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addWidget(combo, 3,2);
  coTiAct3 =  combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate and raise"));
  combo->insertItem(i18n("Activate and lower"));
  combo->insertItem(i18n("Activate"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addWidget(combo, 1,3);
  coTiInAct1 = combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate and raise"));
  combo->insertItem(i18n("Activate and lower"));
  combo->insertItem(i18n("Activate"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addWidget(combo, 2,3);
  coTiInAct2 = combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate and raise"));
  combo->insertItem(i18n("Activate and lower"));
  combo->insertItem(i18n("Activate"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addWidget(combo, 3,3);
  coTiInAct3 = combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate, raise and pass click"));
  combo->insertItem(i18n("Activate and pass click"));
  combo->insertItem(i18n("Activate"));
  combo->insertItem(i18n("Activate and raise"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addMultiCellWidget(combo, 5,5, 2, 3);
  coWin1 = combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate, raise and pass click"));
  combo->insertItem(i18n("Activate and pass click"));
  combo->insertItem(i18n("Activate"));
  combo->insertItem(i18n("Activate and raise"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addMultiCellWidget(combo, 6,6, 2, 3);
  coWin2 = combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate, raise and pass click"));
  combo->insertItem(i18n("Activate and pass click"));
  combo->insertItem(i18n("Activate"));
  combo->insertItem(i18n("Activate and raise"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addMultiCellWidget(combo, 7,7, 2, 3);
  coWin3 = combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Move"));
  combo->insertItem(i18n("Toggle raise and lower"));
  combo->insertItem(i18n("Resize"));
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Nothing"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addMultiCellWidget(combo, 9,9, 2, 3);
  coAll1 = combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Move"));
  combo->insertItem(i18n("Toggle raise and lower"));
  combo->insertItem(i18n("Resize"));
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Nothing"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addMultiCellWidget(combo, 10,10, 2, 3);
  coAll2 = combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Move"));
  combo->insertItem(i18n("Toggle raise and lower"));
  combo->insertItem(i18n("Resize"));
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Nothing"));
  //combo->setMinimumSize(combo->sizeHint());
  layout->addMultiCellWidget(combo, 11,11, 2, 3);
  coAll3 =  combo;

  config->setGroup( "MouseBindings");
  setComboText(coTiAct1,config->readEntry("CommandActiveTitlebar1","Raise"));
  setComboText(coTiAct2,config->readEntry("CommandActiveTitlebar2","Lower"));
  setComboText(coTiAct3,config->readEntry("CommandActiveTitlebar3","Operations menu"));
  setComboText(coTiInAct1, config->readEntry("CommandInactiveTitlebar1","Activate and raise"));
  setComboText(coTiInAct2, config->readEntry("CommandInactiveTitlebar2","Activate and lower"));
  setComboText(coTiInAct3, config->readEntry("CommandInactiveTitlebar3","Activate"));
  setComboText(coWin1, config->readEntry("CommandWindow1","Activate, raise and pass click"));
  setComboText(coWin2, config->readEntry("CommandWindow2","Activate and pass click"));
  setComboText(coWin3, config->readEntry("CommandWindow3","Activate and pass click"));
  setComboText (coAll1, config->readEntry("CommandAll1","Move"));
  setComboText(coAll2, config->readEntry("CommandAll2","Toggle raise and lower"));
  setComboText(coAll3, config->readEntry("CommandAll3","Resize"));

  layout->activate();
}

void KMouseConfig::setComboText(QComboBox* combo, const char* text){
  int i;
  QString s = i18n(text); // no problem. These are already translated!
  for (i=0;i<combo->count();i++){
    if (s==combo->text(i)){
      combo->setCurrentItem(i);
      return;
    }
  }
}

const char*  KMouseConfig::functionTiAc(int i)
{
  switch (i){
  case 0: return "Raise"; break;
  case 1: return "Lower"; break;
  case 2: return "Operations menu"; break;
  case 3: return "Toggle raise and lower"; break;
  case 4: return "Nothing"; break;
  case 5: return ""; break;
  }
  return "";
}
const char*  KMouseConfig::functionTiInAc(int i)
{
  switch (i){
  case 0: return "Activate and raise"; break;
  case 1: return "Activate and lower"; break;
  case 2: return "Activate"; break;
  case 3: return ""; break;
  case 4: return ""; break;
  case 5: return ""; break;
  }
  return "";
}
const char*  KMouseConfig::functionWin(int i)
{
  switch (i){
  case 0: return "Activate, raise and pass click"; break;
  case 1: return "Activate and pass click"; break;
  case 2: return "Activate"; break;
  case 3: return "Activate and raise"; break;
  case 4: return ""; break;
  case 5: return ""; break;
  }
  return "";
}
const char*  KMouseConfig::functionAll(int i)
{
  switch (i){
  case 0: return "Move"; break;
  case 1: return "Toggle raise and lower"; break;
  case 2: return "Resize"; break;
  case 3: return "Raise"; break;
  case 4: return "Lower"; break;
  case 5: return "Nothing"; break;
  }
  return "";
}


void KMouseConfig::loadSettings()
{
}

void KMouseConfig::applySettings()
{
  config->setGroup("MouseBindings");
  config->writeEntry("CommandActiveTitlebar1", functionTiAc(coTiAct1->currentItem()));
  config->writeEntry("CommandActiveTitlebar2", functionTiAc(coTiAct2->currentItem()));
  config->writeEntry("CommandActiveTitlebar3", functionTiAc(coTiAct3->currentItem()));
  config->writeEntry("CommandInactiveTitlebar1", functionTiInAc(coTiInAct1->currentItem()));
  config->writeEntry("CommandInactiveTitlebar2", functionTiInAc(coTiInAct2->currentItem()));
  config->writeEntry("CommandInactiveTitlebar3", functionTiInAc(coTiInAct3->currentItem()));
  config->writeEntry("CommandWindow1", functionWin(coWin1->currentItem()));
  config->writeEntry("CommandWindow2", functionWin(coWin2->currentItem()));
  config->writeEntry("CommandWindow3", functionWin(coWin3->currentItem()));
  config->writeEntry("CommandAll1", functionAll(coAll1->currentItem()));
  config->writeEntry("CommandAll2", functionAll(coAll2->currentItem()));
  config->writeEntry("CommandAll3", functionAll(coAll3->currentItem()));

  config->sync();
}


#include "mouse.moc"
