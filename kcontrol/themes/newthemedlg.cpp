/*
 * newthemedlg.cpp
 *
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org>
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

#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <kapp.h>
#include <kbuttonbox.h>

#include "newthemedlg.h"
#include "themecreator.h"
#include "global.h"


//-----------------------------------------------------------------------------
NewThemeDlg::NewThemeDlg(): 
  NewThemeDlgInherited(0, i18n("Create New Theme"), true)
{
  QPushButton* btn;

  mGrid = new QGridLayout(this, 8, 2, 20, 10);
  mGridRow = 0;

  mEdtFilename = newLine(i18n("&Name"));
  mEdtAuthor = newLine(i18n("&Author"));
  mEdtEmail = newLine(i18n("&Email"));
  mEdtHomepage = newLine(i18n("&Webpage"));
  mEdtDesc = newMultiLine(i18n("&Description"));

  
  mGrid->addRowSpacing(mGridRow++, 10);

  KButtonBox *bbox = new KButtonBox( this);
  bbox->addStretch( 10 );

  btn = bbox->addButton("&OK");
  btn->setDefault(true);
  connect(btn, SIGNAL(clicked()), SLOT(accept()));

  bbox->addStretch( 10 );

  btn = bbox->addButton("&Cancel");
  connect(btn, SIGNAL(clicked()), SLOT(reject()));

  bbox->addStretch( 10 );

  bbox->layout();

  mGrid->addMultiCellWidget(bbox, mGridRow, mGridRow, 0, 1);

  setValues();

  mGrid->setColStretch(0, 0);
  mGrid->setColStretch(1, 1);
  mGrid->activate();

  resize(sizeHint());
}


//-----------------------------------------------------------------------------
NewThemeDlg::~NewThemeDlg()
{
}


//-----------------------------------------------------------------------------
void NewThemeDlg::setValues(void)
{
  KConfig* cfg = kapp->getConfig();
  cfg->setGroup("General");

  mEdtFilename->setText(i18n("MyNewTheme"));
  mEdtAuthor->setText(cfg->readEntry("author"));
  mEdtEmail->setText(cfg->readEntry("email"));
  mEdtHomepage->setText(cfg->readEntry("homepage", "http://kde.themes.org"));
  mEdtDesc->setText(i18n("Give a short description of the theme
here..."));
}


//-----------------------------------------------------------------------------
QLineEdit* NewThemeDlg::newLine(const char* aLabelText)
{
  QLabel* lbl;
  QLineEdit* edt;

  edt = new QLineEdit(this);
  edt->setMinimumSize(edt->sizeHint());
  edt->setMaximumSize(32767, edt->sizeHint().height());
  mGrid->addWidget(edt, mGridRow, 1);

  lbl = new QLabel(edt, aLabelText, this);
  lbl->setMinimumSize(lbl->sizeHint());
  lbl->setMaximumSize(512, edt->sizeHint().height());
  //lbl->setBuddy(edt);
  mGrid->addWidget(lbl, mGridRow, 0);

  mGridRow++;
  return edt;
}

//-----------------------------------------------------------------------------
QMultiLineEdit* NewThemeDlg::newMultiLine(const char* aLabelText)
{
  QLabel* lbl;
  QMultiLineEdit* edt;

  edt = new QMultiLineEdit(this);
  edt->setMinimumSize( 350, 100);
  mGrid->addWidget(edt, mGridRow, 1);

  lbl = new QLabel(edt, aLabelText, this);
  lbl->setMinimumSize(lbl->sizeHint());
  //lbl->setMaximumSize(512, edt->sizeHint().height());
  //lbl->setBuddy(edt);
  mGrid->addWidget(lbl, mGridRow, 0);
  mGrid->setRowStretch(mGridRow, 1);

  mGridRow++;
  return edt;
}
