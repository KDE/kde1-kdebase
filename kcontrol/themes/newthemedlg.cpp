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

#include "newthemedlg.h"
#include "themecreator.h"
#include "global.h"


//-----------------------------------------------------------------------------
NewThemeDlg::NewThemeDlg(): 
  NewThemeDlgInherited(0, i18n("Create New Theme"), true)
{
  QPushButton* btn;
  QHBoxLayout* bbox = new QHBoxLayout;

  mGrid = new QGridLayout(this, 8, 3, 20, 4);
  mGridRow = 0;

  mEdtFilename = newLine(i18n("File name"));
  mEdtName = newLine(i18n("Detailed name"));
  mEdtAuthor = newLine(i18n("Author"));
  mEdtEmail = newLine(i18n("Email"));
  mEdtHomepage = newLine(i18n("Webpage"));

  mGrid->setRowStretch(mGridRow++, 10);

  mGrid->addLayout(bbox, mGridRow++, 2);
  btn = new QPushButton(i18n("OK"), this);
  btn->setFixedSize(100, btn->sizeHint().height());
  bbox->addWidget(btn);
  connect(btn, SIGNAL(clicked()), SLOT(accept()));

  btn = new QPushButton(i18n("Cancel"), this);
  btn->setFixedSize(100, btn->sizeHint().height());
  bbox->addWidget(btn);
  connect(btn, SIGNAL(clicked()), SLOT(reject()));

  setValues();

  mGrid->setColStretch(0, 1);
  mGrid->setColStretch(1, 1);
  mGrid->setColStretch(2, 100);
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

  mEdtFilename->setText(i18n("NewTheme"));
  mEdtName->setText(i18n("A New Theme"));
  mEdtAuthor->setText(cfg->readEntry("author"));
  mEdtEmail->setText(cfg->readEntry("email"));
  mEdtHomepage->setText(cfg->readEntry("homepage"));
}


//-----------------------------------------------------------------------------
QLineEdit* NewThemeDlg::newLine(const char* aLabelText)
{
  QLabel* lbl;
  QLineEdit* edt;

  edt = new QLineEdit(this);
  edt->setMinimumSize(edt->sizeHint());
  edt->setMaximumSize(32767, edt->sizeHint().height());
  mGrid->addMultiCellWidget(edt, mGridRow, mGridRow, 1, 2);

  lbl = new QLabel(aLabelText, this);
  lbl->setMinimumSize(lbl->sizeHint());
  lbl->setMaximumSize(512, edt->sizeHint().height());
  //lbl->setBuddy(edt);
  mGrid->addWidget(lbl, mGridRow, 0);

  mGridRow++;
  return edt;
}
