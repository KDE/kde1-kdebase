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


KLocaleConfig::KLocaleConfig(QWidget *parent, const char *name)
  : KConfigWidget (parent, name)
{
  gbox = new QGroupBox(klocale->translate("Language"), this);
  gbox->setGeometry(16,16,432,116);

  changedFlag = FALSE;
 
  QLabel *label = new QLabel(klocale->translate("First"), gbox);
  label->adjustSize(); label->move(14,22);
  combo1 = new KLanguageCombo(gbox);
  combo1->setGeometry(94,18,300,24);
  label->setBuddy(combo1);
  connect(combo1,SIGNAL(highlighted(int)),this,SLOT(changed(int)));

  label = new QLabel(klocale->translate("Second"), gbox);
  label->adjustSize(); label->move(14,52);
  combo2 = new KLanguageCombo(gbox);
  combo2->setGeometry(94,48,300,24);
  label->setBuddy(combo2);
  connect(combo2,SIGNAL(highlighted(int)),this,SLOT(changed(int)));

  label = new QLabel(klocale->translate("Third"), gbox);
  label->adjustSize(); label->move(14,82);
  combo3 = new KLanguageCombo(gbox);
  combo3->setGeometry(94,78,300,24);
  label->setBuddy(combo3);
  connect(combo3,SIGNAL(highlighted(int)),this,SLOT(changed(int)));

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