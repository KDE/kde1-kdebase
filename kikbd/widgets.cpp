#include <fstream.h>
#include <stdlib.h>

#include <qframe.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <qbttngrp.h>
#include <qlabel.h>
#include <qdir.h>
#include <qcombo.h>
#include <qtooltip.h>

#include <kiconloader.h>
#include <kmsgbox.h>
#include <kcolordlg.h>
#include "widgets.h"

//=====================================================
//  constant data
//=====================================================
const char* switchLabels[] = {
  "(None)",
  "Right Alt",
  "Right Control",
  "Rights (Alt+Shift)" ,
  "Rights (Ctrl+Alt)"  ,
  "Rights (Ctrl+Shift)",
  "Lefts  (Alt+Shift)",
  "Lefts  (Ctrl+Alt)",
  "Lefts  (Ctrl+Shift)",
  "Both Shift's (Shift+Shift)"
};
const char* altSwitchLabels[] = {
  "(None)",
  "Right Alt",
  "Right Control",
  "Left  Alt",
  "Left  Control",
};
const char* autoStartPlaceLabels[] = {
  "Top Left",
  "Top Right",
  "Bottom Left",
  "Bottom Right"
};
const char* inputLabels[] = {
  "Global", "Window", "Class"
};

//=====================================================
// configurations widgets
//=====================================================
KiKbdGeneralWidget::KiKbdGeneralWidget(QWidget* parent, const char* name)
  :KConfigWidget(parent, name), mapsStr(kikbdConfig->getMaps())
{
  QWidget *tipw;
  QBoxLayout *topLayout = new QVBoxLayout(this, 20);
  
  //--- keyboards group
  QGroupBox *group = new QGroupBox(klocale->translate("Keyboards"),
				   this);
  topLayout->addWidget(group, 10);
  QVBoxLayout *mbox = new QVBoxLayout(group, 20);
  QHBoxLayout *hbox = new QHBoxLayout();
  mbox->addLayout(hbox, 20);
  mapsList = new KTabListBox(group, 0, 3);
  mapsList->clearTableFlags(Tbl_autoHScrollBar);
  mapsList->setTableFlags(Tbl_autoVScrollBar);
  mapsList->setSeparator('\t');
  QFontMetrics fi(mapsList->font());
  const char *lb = klocale->translate("Label");
  mapsList->setColumn(0, lb, fi.width(lb)+mapsList->tabWidth());
  lb = klocale->translate("Locale");
  mapsList->setColumn(1, lb, fi.width(lb)+mapsList->tabWidth());
  mapsList->setColumn(2, klocale->translate("Description"), 0);
  hbox->addWidget(mapsList, 20);
  QToolTip::add(mapsList, klocale->translate("List of active keyboard maps"));

  QVBoxLayout *vbox = new QVBoxLayout(5);
  hbox->addLayout(vbox);
  QPushButton *butAdd = new QPushButton(klocale->translate("Add"), group);
  QPushButton *butDelete = new QPushButton(klocale->translate("Delete"), group);
  QPushButton *butUp   = new QPushButton(klocale->translate("Up"), group);
  QPushButton *butDown = new QPushButton(klocale->translate("Down"), group);
  butAdd->setMinimumSize(butAdd->sizeHint());
  butDelete->setMinimumSize(butDelete->sizeHint());
  butDelete->setEnabled(FALSE);
  butUp->setMinimumSize(butUp->sizeHint());
  butDown->setMinimumSize(butDown->sizeHint());
  vbox->addWidget(butAdd);
  vbox->addWidget(butDelete);
  vbox->addWidget(butUp);
  vbox->addWidget(butDown);
  vbox->addStretch(5);
  QWidget *hot = kikbdConfig->hotListWidget(klocale->translate("Use \"hotlist\""), group);
  mbox->addWidget(hot);
  // tips
  QToolTip::add(butAdd, klocale->translate("Adding new keyboard map"));
  QToolTip::add(butDelete, klocale->translate("Remove selected keyboard map"));
  QToolTip::add(butUp, klocale->translate("Up selected keyboard map"));
  QToolTip::add(butDown, klocale->translate("Down selected keyboard map"));
  QToolTip::add(hot, klocale->translate("Use only default and last active keyboard maps to switching from keyboard"));

  //--- switches group
  group = new QGroupBox(klocale->translate("Switch and Alt Switch"), this);
  hbox = new QHBoxLayout(group, 20);
  switchBox = (QComboBox*)kikbdConfig->switchWidget(switchLabels, group);
  hbox->addWidget(switchBox);
  QToolTip::add(switchBox, klocale->translate("Key(s) to switch beetwing keyboard maps"));
  altSwitchBox = (QComboBox*)kikbdConfig->altSwitchWidget(altSwitchLabels, group);
  hbox->addWidget(altSwitchBox);
  group->setMinimumHeight(2*altSwitchBox->height());
  topLayout->addWidget(group);
  QToolTip::add(altSwitchBox, klocale->translate("Key to activate Alternate symbols in current keyboard map"));

  //--- options group
  group = new QGroupBox(klocale->translate("Options"), this);
  hbox = new QHBoxLayout(group, 20);

  hbox->addWidget(tipw = kikbdConfig->
		  keyboardBeepWidget(klocale->translate("Beep"),
				     group));
  QToolTip::add(tipw, klocale->translate("Beep when ever keyboard mapping changed"));
  QPushButton *butAdv = new QPushButton(klocale->translate("Advanced"), group);
  butAdv->setMinimumSize(butAdv->sizeHint());
  hbox->addWidget(butAdv);
  group->setMinimumHeight(2*butAdv->height());
  topLayout->addWidget(group);
  QToolTip::add(butAdv, klocale->translate("Advanced options"));

  //--- connect add button
  connect(butAdd, SIGNAL(clicked()), SLOT(addMap()));
  //--- connect delete button
  connect(mapsList, SIGNAL(highlighted(int, int)), 
	  SLOT(highlighted(int, int)));
  connect(this, SIGNAL(activateDelete(bool)), butDelete,
	  SLOT(setEnabled(bool)));
  connect(this, SIGNAL(activateUp(bool)), butUp, SLOT(setEnabled(bool)));
  connect(butUp, SIGNAL(clicked()), SLOT(upMap()));
  connect(butDown, SIGNAL(clicked()), SLOT(downMap()));
  connect(this, SIGNAL(activateDown(bool)), butDown, SLOT(setEnabled(bool)));
  connect(butDelete, SIGNAL(clicked()), SLOT(deleteMap()));
  connect(butAdv, SIGNAL(clicked()), SLOT(advanced()));
  connect(switchBox, SIGNAL(activated(const char*)), 
	  SLOT(newSwitch(const char*)));
  connect(this, SIGNAL(activateHot(bool)), hot, SLOT(setEnabled(bool)));
}
void KiKbdGeneralWidget::resizeEvent(QResizeEvent* ev)
{
  KConfigWidget::resizeEvent(ev);
  mapsList->setColumnWidth(2, mapsList->width() - mapsList->columnWidth(0)
			   - mapsList->columnWidth(1));
}
void KiKbdGeneralWidget::newSwitch(const char*)
{
  altSwitchBox->setEnabled((!kikbdConfig->oneKeySwitch())
			   && kikbdConfig->hasAltKeys());
}
void KiKbdGeneralWidget::loadSettings()
{
  //--- maps
  QStrList list = mapsStr;
  mapsStr.clear();
  unsigned i;for(i=0; i<list.count(); addMap(list.at(i++)));
}
void KiKbdGeneralWidget::chkActivate()
{
  int current = mapsList->currentItem();
  emit activateDelete(current >= 0);
  emit activateUp(current>0);
  emit activateDown((unsigned)current < mapsList->count()-1);
  emit activateHot(mapsList->count() > 2);
}
void KiKbdGeneralWidget::addMap(const char* name)
{
  mapsStr.append(name);
  QString label = kikbdConfig->getMap(name)->getLabel();
  QString locale = kikbdConfig->getMap(name)->getLocale();
  QString comment = kikbdConfig->getMap(name)->getComment();
  mapsList->insertItem(label+"\t"+locale+"\t"+comment);
  newSwitch(switchBox->currentText());
  chkActivate();
}
void KiKbdGeneralWidget::deleteMap()
{
  int current = mapsList->currentItem();
  mapsStr.remove(current);
  newSwitch(switchBox->currentText());
  mapsList->removeItem(current);
  chkActivate();
}
void KiKbdGeneralWidget::upMap()
{
  int current = mapsList->currentItem()-1;
  QString name  = mapsStr.at(current);
  mapsStr.remove(current);
  mapsStr.insert(current+1, name);

  QString label = mapsList->text(current);
  mapsList->removeItem(current);
  mapsList->insertItem(label, current+1);
  chkActivate();
}
void KiKbdGeneralWidget::downMap()
{
  int current = mapsList->currentItem()+1;
  QString name  = mapsStr.at(current);
  mapsStr.remove(current);
  mapsStr.insert(current-1, name);
  
  QString label = mapsList->text(current);
  mapsList->removeItem(current);
  mapsList->insertItem(label, current-1);
  chkActivate();
}
void KiKbdGeneralWidget::setLongComment(int i)
{
  QString com = kikbdConfig->getMap(mapsToAdd.at(i))->getLongComment();
  emit setLongComment((const char*)com);
}
void KiKbdGeneralWidget::addMap()
{
  cout << "adding" << endl;
  QDialog dialog(this, "", TRUE);
  dialog.setCaption(klocale->translate("Add Keyboard"));

  //--- create list of map to add
  QStrList list = kikbdConfig->availableMaps();
  mapsToAdd.clear();
  unsigned i;for(i=0; i<list.count(); i++)
    if(mapsStr.find(list.at(i)) == -1) {
      mapsToAdd.inSort(list.at(i));
    }
  if(mapsToAdd.count() == 0) {
    KMsgBox::message(0, klocale->translate("Adding Keyboard"),
		     klocale->translate("There is no more keyboard maps"));
    return;
  }

  QBoxLayout *topLayout = new QVBoxLayout(&dialog, 5);
  QGroupBox *group = new QGroupBox(klocale->translate("Available keyboard maps"), 
				   &dialog);
  topLayout->addWidget(group, 10);

  QBoxLayout *choise = new QVBoxLayout(group, 20);
  QComboBox *maps = new QComboBox(group);
  QLabel *label = new QLabel(&dialog);
  label->setAlignment(WordBreak);
  choise->addWidget(maps);
  choise->addWidget(label, 10);
  maps->setMinimumSize(maps->sizeHint());
  
  //--- buttons
  QBoxLayout *buttons = new QHBoxLayout(10);
  topLayout->addLayout(buttons, 2);
  QPushButton *ok = new QPushButton(klocale->translate("OK"), &dialog);
  QPushButton *cancel = new QPushButton(klocale->translate("Cancel"), &dialog);  
  QSize size1 = ok->sizeHint();
  QSize size2 = cancel->sizeHint();
  if(size1.width() < size2.width()) size1.setWidth(size2.width());
  if(size1.height() < size2.height()) size1.setHeight(size2.height());
  ok->setFixedSize(size1);
  cancel->setFixedSize(size1);
  connect(ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
  buttons->addStretch(10);
  buttons->addWidget(ok);
  buttons->addWidget(cancel);
  buttons->addStretch(10);

  dialog.resize(440, 240);

  //--- load list of maps
  for(i=0; i<mapsToAdd.count(); i++)
    maps->insertItem(kikbdConfig->getMap(mapsToAdd.at(i))->getGoodLabel());
  ok->setFocus();
  connect(maps, SIGNAL(activated(int)), SLOT(setLongComment(int)));
  connect(this, SIGNAL(setLongComment(const char*)), label,
	  SLOT(setText(const char*)));
  maps->setCurrentItem(0);
  setLongComment(0);

  //--- execute dialog
  if(dialog.exec()) addMap(mapsToAdd.at(maps->currentItem()));
}
void KiKbdGeneralWidget::advanced()
{
  QDialog dialog(this, "", TRUE);
  dialog.setCaption(klocale->translate("Advanced"));

  QWidget* tipw;
  QBoxLayout *topLayout = new QVBoxLayout(&dialog, 5);
  QGroupBox  *group = new QGroupBox(&dialog);
  QBoxLayout *groupLayout = new QVBoxLayout(group, 20);
  topLayout->addWidget(group, 10);

  QBoxLayout *hbox = new QHBoxLayout();
  groupLayout->addLayout(hbox);

  //--- kpanel menu
  hbox->addWidget(tipw = kikbdConfig->
		  emuCapsLockWidget(klocale->translate("Emulate CapsLock"),
				    group));
  QToolTip::add(tipw, klocale->translate("Emulate XServer CapsLock. Needed for some languages to be correct"));
  hbox->addWidget(tipw = kikbdConfig->
		  autoMenuWidget(klocale->translate("World Menu"),
				 group));
  QToolTip::add(tipw, klocale->translate("Show menu in any window by holding Switch keys"));
  hbox = new QHBoxLayout();
  groupLayout->addLayout(hbox);
  QCheckBox *cbox = (QCheckBox*)
    kikbdConfig->saveClassesWidget(klocale->translate("Save Classes"),
				   group);
  hbox->addWidget(cbox);
  QToolTip::add(cbox, klocale->translate("Save relations between window classes and keyboard maps on exit"));
  QButtonGroup* butg = (QButtonGroup*)
    kikbdConfig->inputWidget(inputLabels, klocale->translate("Input"),
			     group);
  hbox = new QHBoxLayout(butg, 15);
  hbox->addWidget(tipw = butg->find(0));
  QToolTip::add(tipw, klocale->translate("Standard behavior. Keyboard map active for all windows"));
  hbox->addWidget(tipw = butg->find(1));
  QToolTip::add(tipw, klocale->translate("Extended behavior. Each windows remember it's own keyboard map"));
  hbox->addWidget(tipw = butg->find(2));
  QToolTip::add(tipw, klocale->translate("Special behavior. Each windows class remember it's own keyboard map"));
  groupLayout->addWidget(butg);
  connect(butg->find(2), SIGNAL(toggled(bool)), cbox, SLOT(setEnabled(bool)));
  cbox->setEnabled(butg->find(2)->isOn());

  //--- Ok button
  QBoxLayout *buttons = new QHBoxLayout(10);
  topLayout->addLayout(buttons, 2);
  QPushButton *ok = new QPushButton(klocale->translate("OK"), &dialog);
  ok->setFixedSize(ok->sizeHint());
  buttons->addStretch(10);
  buttons->addWidget(ok);
  buttons->addStretch(10);
  connect(ok, SIGNAL(clicked()), &dialog, SLOT(accept()));

  dialog.resize(380, 260);
  ok->setFocus();
  
  dialog.exec();
}

/**
   Style widget
   colors for normal, caps, alternate button
   button font
*/
KiKbdStyleWidget::KiKbdStyleWidget(QWidget* parent, const char* name)
  :KConfigWidget(parent, name)
{
  QBoxLayout *topLayout = new QVBoxLayout(this, 20);
  
  /**
     color group
  */
  QGroupBox *group = new QGroupBox(klocale->translate("Button Colors"),
				   this);
  topLayout->addWidget(group, 10);
  QVBoxLayout *vbox = new QVBoxLayout(group, 25);
  QWidget *but;
  QLabel  *label;
  QHBoxLayout *hbox;
  int width = 100, height = 30;

  /**
     forecground button color
  */
  hbox = new QHBoxLayout();
  vbox->addLayout(hbox);
  but = kikbdConfig->forColorWidget(group);
  but->setMinimumSize(width, height);
  hbox->addWidget(but, 0);
  label = new QLabel(klocale->translate("Foreground"), group);
  label->setMinimumSize(label->sizeHint());
  hbox->addWidget(label, 0);
  hbox->addStretch(10);
  QToolTip::add(but, klocale->translate("Color of the Text Label"));

  /**
     caps  color
  */
  hbox = new QHBoxLayout();
  vbox->addLayout(hbox);
  but = kikbdConfig->capsColorWidget(group);
  but->setMinimumSize(width, height);
  hbox->addWidget(but, 0);
  label = new QLabel(klocale->translate("With CapsLock"), group);
  label->setMinimumSize(label->sizeHint());
  hbox->addWidget(label, 0);
  hbox->addStretch(10);
  QToolTip::add(but, klocale->translate("Background when Emulated CapsLock active"));
  connect(this, SIGNAL(enableCaps(bool)), but, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(enableCaps(bool)), label, SLOT(setEnabled(bool)));

   /**
     alternate color
  */
  hbox = new QHBoxLayout();
  vbox->addLayout(hbox);
  but = kikbdConfig->altColorWidget(group);
  but->setMinimumSize(width, height);
  hbox->addWidget(but, 0);
  label = new QLabel(klocale->translate("With Alternate"), group);
  label->setMinimumSize(label->sizeHint());
  hbox->addWidget(label, 0);
  hbox->addStretch(10);
  QToolTip::add(but, klocale->translate("Background when Alternate switch is pressed"));

  but = new QPushButton(group);
  vbox->addWidget(but);
  vbox->addStretch(10);
  connect(this, SIGNAL(enableAlternate(bool)), but, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(enableAlternate(bool)), label, SLOT(setEnabled(bool)));

  /**
     font
  */
  group = new QGroupBox(klocale->translate("Button Font"),
			this);
  topLayout->addWidget(group, 0);
  vbox = new QVBoxLayout(group, 20);
  hbox = new QHBoxLayout();
  vbox->addLayout(hbox);

  QWidget *b;
  b = kikbdConfig->custFontWidget(klocale->translate("Customize Font"),
				  group);
  hbox->addWidget(b, 2);
  QToolTip::add(b, klocale->translate("Customize Font for text label or use global settings"));

  but = kikbdConfig->fontWidget(group);
  ((QPushButton*)but)->setText(klocale->translate("Change Font"));
  but->setMinimumSize(but->sizeHint());
  hbox->addWidget(but);
  hbox->addStretch(5);
  vbox->addStretch(5);
  group->setMinimumHeight(3*but->height());
  connect(b, SIGNAL(toggled(bool)), but, SLOT(setEnabled(bool)));
  but->setEnabled(kikbdConfig->getCustFont());
}
void KiKbdStyleWidget::aboutToShow(const char* page)
{
  if(QString(page) == klocale->translate("&Style")) {
    emit enableCaps(kikbdConfig->getEmuCapsLock());
    emit enableAlternate(!kikbdConfig->oneKeySwitch() 
			 && kikbdConfig->hasAltKeys());
  }
}

//=========================================================
//  startup widget
//=========================================================
KiKbdStartupWidget::KiKbdStartupWidget(QWidget* parent, const char* name)
  :KConfigWidget(parent, name)
{
  QBoxLayout *topLayout = new QVBoxLayout(this, 20);
  QBoxLayout *vbox, *hbox;
  QGroupBox  *group;
  QCheckBox  *autoStart, *docked;
  QComboBox  *startPlace;
  QLabel     *label;

  group = new QGroupBox("", this);
  topLayout->addWidget(group, 5);

  /**
     Autostart?
  */
  vbox  = new QVBoxLayout(group, 20);
  vbox->addWidget(autoStart = (QCheckBox*)
		  kikbdConfig->autoStartWidget(klocale->translate("Autostart"), group), 0);
  QToolTip::add(autoStart, klocale->translate("Start up automaticaly"));
  
  /**
     Do docking?
  */
  hbox = new QHBoxLayout();
  vbox->addLayout(hbox);
  //hbox->addSpacing(20);
  hbox->addWidget(docked = (QCheckBox*)
		  kikbdConfig->dockingWidget("Docked", group), 0);
  QToolTip::add(docked, klocale->translate("Dock into special area in kpanel"));

  /**
     place to start
  */
  hbox = new QHBoxLayout();
  vbox->addLayout(hbox);
  //hbox->addSpacing(20);
  label = new QLabel(klocale->translate("Place"), group), 0;
  label->setMinimumSize(label->sizeHint());
  hbox->addWidget(label, 0);
  hbox->addWidget(startPlace = (QComboBox*)
  	  kikbdConfig->autoStartPlaceWidget(autoStartPlaceLabels,
  						    group), 0);
  hbox->addStretch(5);
  vbox->addSpacing(20);
  topLayout->addStretch(5);
  QToolTip::add(startPlace, klocale->translate("Place in selected corner"));

  //connect(autoStart, SIGNAL(toggled(bool)), docked, SLOT(setEnabled(bool)));
  //connect(autoStart, SIGNAL(toggled(bool)), 
  //  startPlace, SLOT(setEnabled(bool)));
  //connect(autoStart, SIGNAL(toggled(bool)), label, SLOT(setEnabled(bool)));

  connect(docked, SIGNAL(toggled(bool)), SLOT(slotInvert(bool)));
  connect(this, SIGNAL(signalInvert(bool)),
	  startPlace, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(signalInvert(bool)), label, SLOT(setEnabled(bool)));

  //docked->setEnabled(autoStart->isChecked());
  startPlace->setEnabled(!docked->isChecked());
  label->setEnabled(!docked->isChecked());
}
void KiKbdStartupWidget::slotInvert(bool f)
{
  emit signalInvert(!f);
}

