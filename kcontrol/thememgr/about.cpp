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
#include <qframe.h>
#include <qfont.h>

#include "about.h"
#include "themecreator.h"
#include "global.h"
#include "version.h"


//-----------------------------------------------------------------------------
About::About (QWidget * aParent, const char *aName, bool aInit)
  : AboutInherited(aParent, aName)
{
  QBoxLayout* box;
  QLabel* lbl;
  QFrame* frm;
  QString str;
  QFont fnt;

  if (aInit) return;

  connect(theme, SIGNAL(changed()), SLOT(slotThemeChanged()));

  box = new QVBoxLayout(this, 20, 6);

  lblTheme = new QLabel(" ", this);
  fnt = lblTheme->font();
  fnt.setPointSize(fnt.pointSize() * 1.2);
  lblTheme->setFont(fnt);
  lblTheme->setMinimumSize(lblTheme->sizeHint());
  lblTheme->setAutoResize(true);
  box->addWidget(lblTheme);

  lblVersion = new QLabel(" ", this);
  lblVersion->setMinimumSize(lblVersion->sizeHint());
  lblVersion->setAutoResize(true);
  box->addWidget(lblVersion);

  lblAuthor = new QLabel(" ", this);
  lblAuthor->setMinimumSize(lblAuthor->sizeHint());
  lblAuthor->setAutoResize(true);
  box->addWidget(lblAuthor);

  lblHomepage = new QLabel(" ", this);
  lblHomepage->setMinimumSize(lblHomepage->sizeHint());
  lblHomepage->setAutoResize(true);
  box->addWidget(lblHomepage);

  frm = new QFrame(this);
  frm->setFrameStyle(QFrame::HLine|QFrame::Raised);
  box->addSpacing(5);
  box->addWidget(frm);
  box->addSpacing(5);

  lbl = new QLabel(i18n("KDE Theme Manager"), this);
  lbl->setFont(fnt);
  lbl->setMinimumSize(lbl->sizeHint());
  box->addWidget(lbl);

  str.sprintf(i18n("Version %s\n\n"
		   "Copyright (C) 1998 by\n%s\n\n"
		   "Gnu Public License (GPL)"),
	           KTHEME_VERSION, 
                   "Stefan Taferner <taferner@kde.org>\n"
                   "Waldo Bastian <bastian@kde.org>");
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
void About::slotThemeChanged()
{
  QString str, value;

  // Theme name
  value = theme->name();
  if (value.isEmpty())
  {
    value = theme->name();
    if (value.isEmpty()) value = i18n("Unknown");
  }
  str.sprintf(i18n("%s Theme"), (const char*)value);
  lblTheme->setText(str);

  // Version
  value = theme->version();
  if (value.isEmpty()) str = "";
  else str.sprintf(i18n("Version %s"), (const char*)value);
  lblVersion->setText(str);

  // Author and email address
  value = theme->author();
  if (value.isEmpty()) value = i18n("Unknown");
  str.sprintf(i18n("by %s"), (const char*)value);

  value = theme->email();
  if (!value.isEmpty())
  {
    if (value.find('<') >= 0 && value.find('>') >= 0)
      str += " (" + value + ')';
    else str += " <" + value + '>';
  }
  lblAuthor->setText(str);

  // Homepage
  value = theme->homepage();
  lblHomepage->setText(value);
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
