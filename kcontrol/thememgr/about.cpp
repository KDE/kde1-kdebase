/*
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org>
 */
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <qchkbox.h>
#include <qlistbox.h>
#include <qcombo.h>
#include <kapp.h>
#include <kconfig.h>

#include "about.h"
#include "theme.h"
#include "global.h"
#include "version.h"


//-----------------------------------------------------------------------------
About::About (QWidget * aParent, const char *aName, bool aInit)
  : AboutInherited(aParent, aName)
{
  QBoxLayout* box;
  QLabel* lbl;
  QString str;

  if (aInit) return;

  box = new QVBoxLayout(this, 20, 6);

  str.sprintf(i18n("Kde Theme Manager\nVersion %s\n\nCopyright (C) 1998 by\n%s"),
	      KTHEME_VERSION, "Stefan Taferner <taferner@kde.org>");
  lbl = new QLabel(str, this);
  lbl->setMinimumSize(lbl->sizeHint());
  box->addWidget(lbl);

  box->addStretch(1000);
  box->activate();
}


//-----------------------------------------------------------------------------
About::~About()
{
}


//-----------------------------------------------------------------------------
void About::loadSettings()
{
}


//-----------------------------------------------------------------------------
void About::applySettings()
{
}


//-----------------------------------------------------------------------------
#include "about.moc"
