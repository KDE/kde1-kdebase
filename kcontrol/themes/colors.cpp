/*
 * colors.cpp
 *
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org> and
 *                    Roberto Alsina <ralsina@unl.edu.ar>
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
#include <qdir.h>
#include <qstrlist.h>
#include <qfiledlg.h>
#include <qbttngrp.h>
#include <qframe.h>
#include <qgrpbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qchkbox.h>
#include <qlistbox.h>
#include <qcombo.h>
#include <kcolorbtn.h>

#include "colors.h"
#include "themecreator.h"
#include "widgetcanvas.h"


//-----------------------------------------------------------------------------
Colors::Colors (QWidget * aParent, const char *aName, bool aInit)
  : ColorsInherited(aParent, aName)
{
  mGui = !aInit;
  if (!mGui)
  {
    return;
  }
  mColorPushColor = -1;

  connect(theme, SIGNAL(changed()), SLOT(slotThemeChanged()));
  connect(theme, SIGNAL(apply()), SLOT(slotThemeApply()));

  mGrid = new QGridLayout(this, 7, 2, 20, 6);

  mCanvas = new WidgetCanvas(this);
  mCanvas->setFixedSize(400,145);
  mCanvas->drawSampleWidgets();
  mGrid->addWidget(mCanvas, 0, 0);
  connect(mCanvas, SIGNAL(widgetSelected(int)), SLOT(slotWidgetColor(int)));

  mBtnColor = new KColorButton(this);
  mBtnColor->setFixedSize(100, 30);
  mGrid->addWidget(mBtnColor, 1, 0);
  connect(mBtnColor, SIGNAL(changed(const QColor&)), 
	  SLOT(slotSelectColor(const QColor &)));

  mCbxColorNames = new QComboBox(this);
  mCbxColorNames->setFixedSize(160, 25);
  mCbxColorNames->insertItem(i18n("Inactive title bar"));
  mCbxColorNames->insertItem(i18n("Inactive title text"));
  mCbxColorNames->insertItem(i18n("Active title bar"));
  mCbxColorNames->insertItem(i18n("Active title text"));
  mCbxColorNames->insertItem(i18n("Background"));
  mCbxColorNames->insertItem(i18n("Text"));
  mCbxColorNames->insertItem(i18n("Select background"));
  mCbxColorNames->insertItem(i18n("Select text"));
  mCbxColorNames->insertItem(i18n("Window background"));
  mCbxColorNames->insertItem(i18n("Window text"));
  mGrid->addWidget(mCbxColorNames, 2, 0);
  connect(mCbxColorNames, SIGNAL(activated(int)), SLOT(slotWidgetColor(int)));

  mGrid->setColStretch(0, 3);
  mGrid->setColStretch(1, 1);
  mGrid->activate();
}


//-----------------------------------------------------------------------------
Colors::~Colors()
{
}


//-----------------------------------------------------------------------------
void Colors::loadSettings()
{
  debug("Colors::loadSettings() called");
}


//-----------------------------------------------------------------------------
void Colors::applySettings()
{
  debug("Colors::applySettings() called");

#ifdef BROKEN
  theme->inactiveTitleColor = mCanvas->inactiveTitleColor;
  theme->inactiveTextColor = mCanvas->inactiveTextColor;
  theme->activeTitleColor = mCanvas->activeTitleColor;
  theme->activeTextColor = mCanvas->activeTextColor;
  theme->backgroundColor = mCanvas->backgroundColor;
  theme->textColor = mCanvas->textColor;
  theme->selectColor = mCanvas->selectColor;
  theme->selectTextColor = mCanvas->selectTextColor;
  theme->windowColor = mCanvas->windowColor;
  theme->windowTextColor = mCanvas->windowTextColor;
  theme->contrast = mCanvas->contrast;
#endif
}


//-----------------------------------------------------------------------------
void Colors::slotThemeApply()
{
  applySettings();
}


//-----------------------------------------------------------------------------
void Colors::slotThemeChanged()
{
  debug("Colors::slotThemeChanged() called");

  mCanvas->inactiveTitleColor = theme->inactiveBackgroundColor;
  mCanvas->inactiveTextColor = theme->inactiveForegroundColor;
  mCanvas->activeTitleColor = theme->activeBackgroundColor;
  mCanvas->activeTextColor = theme->activeForegroundColor;
  mCanvas->backgroundColor = theme->backgroundColor;
  mCanvas->textColor = theme->foregroundColor;
  mCanvas->selectColor = theme->selectBackgroundColor;
  mCanvas->selectTextColor = theme->selectForegroundColor;
  mCanvas->windowColor = theme->backgroundColor;
  mCanvas->windowTextColor = theme->foregroundColor;
  mCanvas->contrast = theme->contrast;

  mCanvas->drawSampleWidgets();
}


//-----------------------------------------------------------------------------
void Colors::slotSelectColor(const QColor & aColor)
{
  switch(mCbxColorNames->currentItem()+1)
  {
  case 1:	mCanvas->inactiveTitleColor=aColor;
    break;
  case 2:	mCanvas->inactiveTextColor=aColor;
    break;
  case 3:	mCanvas->activeTitleColor=aColor;
    break;
  case 4:	mCanvas->activeTextColor=aColor;
    break;
  case 5:	mCanvas->backgroundColor=aColor;
    break;
  case 6:	mCanvas->textColor=aColor;
    break;
  case 7:	mCanvas->selectColor=aColor;
    break;
  case 8:	mCanvas->selectTextColor=aColor;
    break;
  case 9:	mCanvas->windowColor=aColor;
    break;
  case 10:	mCanvas->windowTextColor=aColor;
    break;
  }
	
  mCanvas->drawSampleWidgets();
}


//-----------------------------------------------------------------------------
void Colors::slotWidgetColor(int idx)
{
  QColor col;

  debug("slotWidgetColor(%d)", idx);

  if (mCbxColorNames->currentItem() != idx)
    mCbxColorNames->setCurrentItem(idx);

  switch(idx+1)
  {
  case 1:	col=mCanvas->inactiveTitleColor;
    break;
  case 2:	col=mCanvas->inactiveTextColor;
    break;
  case 3:	col=mCanvas->activeTitleColor;
    break;
  case 4:	col=mCanvas->activeTextColor;
    break;
  case 5:	col=mCanvas->backgroundColor;
    break;
  case 6:	col=mCanvas->textColor;
    break;
  case 7:	col=mCanvas->selectColor;
    break;
  case 8:	col=mCanvas->selectTextColor;
    break;
  case 9:	col=mCanvas->windowColor;
    break;
  case 10:	col=mCanvas->windowTextColor;
    break;
  }
  mBtnColor->setColor(col);
  mColorPushColor=idx;	
}


//-----------------------------------------------------------------------------
#include "colors.moc"
