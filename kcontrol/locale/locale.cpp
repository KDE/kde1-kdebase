/*
 * locale.cpp
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
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
#include <kapp.h>
#include <klocale.h>
#include <qlabel.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kmsgbox.h>

#include "locale.h"
#include "locale.moc"
#include <qlayout.h>

KLocaleConfig::KLocaleConfig(QWidget *parent, const char *name)
  : KConfigWidget (parent, name)
{
  QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
  QGridLayout *tl1 = new QGridLayout(5, 4, 5);
  tl->addLayout(tl1);
  tl->addStretch(1);

  gbox = new QGroupBox(klocale->translate("Language"), this);
  tl1->addMultiCellWidget(gbox, 0, 4, 0, 3);
  tl1->addRowSpacing(0, 10);
  tl1->addRowSpacing(4, 15);
  tl1->addColSpacing(0, 10);
  tl1->addColSpacing(3, 10);
  tl1->setColStretch(2, 1);

  changedFlag = FALSE;
 
  QLabel *label = new QLabel(klocale->translate("First"), gbox);
  label->setMinimumSize(label->sizeHint());
  combo1 = new KLanguageCombo(gbox);
  combo1->setMinimumWidth(combo1->sizeHint().width());
  combo1->setFixedHeight(combo1->sizeHint().height());
  label->setBuddy(combo1);
  connect(combo1,SIGNAL(highlighted(int)),this,SLOT(changed(int)));
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(combo1, 1, 2);

  label = new QLabel(klocale->translate("Second"), gbox);
  label->setMinimumSize(label->sizeHint());
  combo2 = new KLanguageCombo(gbox);
  combo2->setMinimumWidth(combo2->sizeHint().width());
  combo2->setFixedHeight(combo2->sizeHint().height());
  label->setBuddy(combo2);
  connect(combo2,SIGNAL(highlighted(int)),this,SLOT(changed(int)));
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(combo2, 2, 2);

  label = new QLabel(klocale->translate("Third"), gbox);
  label->setMinimumSize(label->sizeHint());
  combo3 = new KLanguageCombo(gbox);
  combo3->setMinimumWidth(combo3->sizeHint().width());
  combo3->setFixedHeight(combo3->sizeHint().height());
  label->setBuddy(combo3);
  connect(combo3,SIGNAL(highlighted(int)),this,SLOT(changed(int)));
  tl1->addWidget(label, 3, 1);
  tl1->addWidget(combo3, 3, 2);

  tl->activate();

  loadSettings();
}


KLocaleConfig::~KLocaleConfig ()
{
}


void KLocaleConfig::loadLanguageList(KLanguageCombo *combo)
{
  KConfig *config = kapp->getConfig();
  QString name;

  combo->clear();
  languages.clear();  
  tags.clear();

  config->setGroup("KCM Locale");
  config->readListEntry("Languages", tags);

  for (const char *lang = tags.first(); lang; lang = tags.next())
    {
      config->setGroup(lang);
      name = config->readEntry("Name");
      if (!name.isEmpty())
        languages.append(name);
      else
        languages.append(klocale->translate("without name!"));
             
     combo->insertLanguage(QString(lang)+";"+name);

     if (strcmp(lang,"C")==0)
       combo->setCurrentItem(combo->count()-1);
    }
}


void KLocaleConfig::loadSettings()
{
  KSimpleConfig config(QDir::homeDirPath()+"/.kderc");

  loadLanguageList(combo1);
  loadLanguageList(combo2);
  loadLanguageList(combo3);


  // This code is adopted from klocale.cpp

  QString languages, lang;
  int i=0, pos;

  config.setGroup("Locale");
  languages = config.readEntry("Language");

  while (1) {
    lang = languages.left(languages.find(':'));
    languages = languages.right(languages.length() - lang.length() - 1);
    if (lang.isEmpty() || lang == "C")
        break;
    i++;
    switch (i) {
      case 1: pos = tags.find(lang); 
              if (pos >= 0)
                combo1->setCurrentItem(pos);
              break;
      case 2: pos = tags.find(lang); 
              if (pos >= 0)
                combo2->setCurrentItem(pos);
              break;
      case 3: pos = tags.find(lang); 
              if (pos >= 0)
                combo3->setCurrentItem(pos);
              break;        
      default: return;
    }
  } 
}


void KLocaleConfig::applySettings()
{
  KSimpleConfig config(QDir::homeDirPath()+"/.kderc");
  QString value;

  value.sprintf("%s:%s:%s", tags.at(combo1->currentItem()),
                            tags.at(combo2->currentItem()),
                            tags.at(combo3->currentItem()));

  config.setGroup("Locale");
  config.writeEntry("Language", value);  
  config.sync();

  if (changedFlag)
    KMsgBox::message(this,klocale->translate("Applying language settings"),
      klocale->translate("Changed language settings apply only to newly started "
                         "applications.\nTo change the language of all "
                         "programs, you will have to logout first."));

  changedFlag = FALSE;
}


void KLocaleConfig::changed(int)
{
  changedFlag = TRUE;
}
