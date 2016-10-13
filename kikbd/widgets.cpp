/*
   - 

  written 1998 by Alexander Budnik <budnik@linserv.jinr.ru>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */
#include <fstream>
#include <stdlib.h>

#include <qframe.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <qbttngrp.h>
#include <qlabel.h>
#include <qdir.h>
#include <qcombo.h>
#include <qtooltip.h>
#include <qbitmap.h>
#include <qaccel.h>
#include <qdialog.h>

#include <kiconloader.h>
#include <kmsgbox.h>
#include <kcolordlg.h>

#include "widgets.h"
#include "widgets.moc"

//====================================================
// helper functions
//====================================================
//
// Add translated tooltip to widget
//
void addToolTip(QWidget* w, const char* tip) {
  QToolTip::add(w, klocale->translate(tip));
}
QWidget* mkButton(QWidget* parent, QBoxLayout* box, const char* slot,
		      QObject* obj, const char* label, const char* tip)
{
  QPushButton* but = new QPushButton(i18n(label), parent);
  but->setMinimumSize(but->sizeHint());
  box->addWidget(but);
  addToolTip(but, tip);
  QObject::connect(but, SIGNAL(clicked()), obj, slot);
  return but;
}
//
// This function draw presentation of the given keyboard map
//
QPixmap mapLine(KiKbdMapConfig* map) {
  QPainter pp, pb;
  QString  text = map->getGoodLabel()+" "+map->getComment();

  //CT - 01Apr1999 - extremely stupid fix for the problem of the
  //        fontMetrics() use *before* pp activation, which leads to segfault
  //        with Qt above 1.42
  QPixmap tmp(16,16);
  pp.begin(&tmp);
  //CT

  int      width = pp.fontMetrics().width(text) + 25;
  /*CT*/  pp.end(); //CT of course, we have to reinitialize it later
  QPixmap  pm(width, 16);
  QBitmap  bm(width, 16);

  // pixmap
  pm.fill((kapp->mainWidget())->colorGroup().base());
  pp.begin(&pm);
  pp.setPen(map->getColor());
  pp.drawText(25, 1, width-25, 14, AlignLeft | AlignVCenter, text);
  pp.fillRect(0, 0, 23, 16, gray);
  pp.drawPixmap(1, 1, map->getIcon());
  pp.end();

  // mask
  bm.fill(color0);
  pb.begin(&bm);
  pb.setPen(color1);
  pb.fillRect(0, 0, 24, 16, color1);
  pb.fillRect(1, 1, width-1, 14, color1);
  pb.end();
  
  return pm.setMask(bm), pm;
}
//=====================================================
// widgets for configure list of maps
//=====================================================
KiKbdMapsWidget::KiKbdMapsWidget(QWidget* parent)
  :QWidget(parent), mapsStr(kikbdConfig->getMaps())
{
  QVBoxLayout *mbox  = new QVBoxLayout(this);
  QAccel      *accel = new QAccel(this);
  QBoxLayout  *hbox, *vbox;
  QWidget     *wid;
  
  //--- keyboards group
  hbox = new QHBoxLayout();
  mbox->addLayout(hbox, 20);

  addToolTip(mapsList=new QListBox(this), 
	     gettext("List of active keyboard maps"));
  hbox->addWidget(mapsList, 20);
  connect(mapsList, SIGNAL(highlighted(int)), SLOT(highlighted(int)));
  connect(mapsList, SIGNAL(selected(int)), SLOT(selected(int)));

  vbox = new QVBoxLayout(5);
  hbox->addSpacing(14);
  hbox->addLayout(vbox);

  // add
  wid = mkButton(this, vbox, SLOT(addMap()), this, gettext("&Add"),
		 gettext("Add new keyboard map"));
  accel->connectItem(accel->insertItem(Key_Insert), wid, SLOT(animateClick()));
  // delete
  wid = mkButton(this, vbox, SLOT(deleteMap()), this,
		 gettext("&Delete"), gettext("Remove selected keyboard map"));
  connect(this, SIGNAL(activateDelete(bool)), wid, SLOT(setEnabled(bool)));
  accel->connectItem(accel->insertItem(Key_Delete), wid, SLOT(animateClick()));
  // up
  wid = mkButton(this, vbox, SLOT(upMap()), this,
		 gettext("U&p"), gettext("Move up selected keyboard map"));
  connect(this, SIGNAL(activateUp(bool)), wid, SLOT(setEnabled(bool)));
  // down
  wid = mkButton(this, vbox, SLOT(downMap()), this,
		 gettext("D&own"), gettext("Move down selected keyboard map"));
  connect(this, SIGNAL(activateDown(bool)), wid, SLOT(setEnabled(bool)));
  // info
  wid = mkButton(this, vbox, SLOT(infoMap()), this,
		 gettext("&Info"), 
		 gettext("Display information for selected keyboard map"));
  connect(this, SIGNAL(infoClick()), wid, SLOT(animateClick())); 
  connect(this, SIGNAL(activateInfo(bool)), wid, SLOT(setEnabled(bool)));

  vbox->addStretch(5);
  mbox->addSpacing(7);
  mbox->
    addWidget(wid=kikbdConfig->
	      createWidget(&kikbdConfig->getHotList(), this,
			   gettext("Use &hot list"),
			   gettext("Use only default and last active "
				   "keyboard maps when switch key sequence is used")));
  connect(this, SIGNAL(activateHot(bool)), wid, SLOT(setEnabled(bool)));

  accel->connectItem(accel->insertItem(Key_Up), this, SLOT(selectionUp()));
  accel->connectItem(accel->insertItem(Key_Down), this, 
		     SLOT(selectionDown()));
  connect(kikbdConfig->find(&mapsStr), SIGNAL(dataChanged()),
	  SLOT(dataChanged()));
  stopFlag = 0;
  dataChanged();
}
void KiKbdMapsWidget::changeData()  // called when we change mapsStr
{
  chkActive();
  stopFlag++;
  kikbdConfig->find(&mapsStr)->markDataChanged();
  stopFlag--;
}
void KiKbdMapsWidget::dataChanged() // called when mapsStr changed
{
  if(stopFlag) return;
  QStrList list = mapsStr;
  mapsStr.clear();
  mapsList->clear();
  for(unsigned i=0; i<list.count(); addMap(list.at(i++)));
  if(mapsList->count()) mapsList->setCurrentItem(0);
  else chkActive();
}
void KiKbdMapsWidget::selectionUp()
{
  int current = mapsList->currentItem();
  if(current > 0) mapsList->setCurrentItem(current-1);
}
void KiKbdMapsWidget::selectionDown()
{
  int current = mapsList->currentItem();
  if(current < (int)mapsList->count()-1) mapsList->setCurrentItem(current+1);
}
void KiKbdMapsWidget::chkActive()
{
  int current = mapsList->currentItem(), count = mapsList->count();
  emit activateDelete(current >= 0);
  emit activateUp    (current > 0);
  emit activateDown  (current < count-1);
  emit activateInfo  (current >= 0);
  emit activateHot   (count > 2);
}
void KiKbdMapsWidget::addMap(const char* name)
{
  int current = mapsList->currentItem();
  mapsList->insertItem(mapLine(kikbdConfig->getMap(name)),
		       current==-1?-1:current+1);
  mapsStr.insert(current==-1?0:current+1, name);
  mapsList->setCurrentItem(current==-1?0:current+1);

  changeData();
}
void KiKbdMapsWidget::addMap()
{
  //--- create list of map to add
  QStrList list = kikbdConfig->availableMaps();
  QStrList mapsToAdd, sortList;
  unsigned i;for(i=0; i<list.count(); i++)
    if(mapsStr.find(list.at(i)) == -1) {
      sortList.inSort(kikbdConfig->getMap(list.at(i))->getGoodLabel());
      mapsToAdd.insert(sortList.at(), list.at(i));
    }
  if(mapsToAdd.count() == 0) {
    KMsgBox::message(0, i18n("Adding Keyboard"),
		     i18n("There are no keyboard maps left to add"));
    return;
  }
  KiKbdAddDialog addDialog(this);
  if(addDialog.exec(mapsToAdd, mapsStr))
    addMap(mapsToAdd.at(addDialog.selectedMap()));
}
void KiKbdMapsWidget::deleteMap()
{
  int current = mapsList->currentItem();
  mapsStr.remove(current);
  mapsList->removeItem(current);
  if(mapsList->count() > 0)
    mapsList->setCurrentItem(current==0?current:current-1);
  changeData();
}
void KiKbdMapsWidget::changeMap(int dif)
{
  int i2 = mapsList->currentItem();
  int i1 = i2 + dif;
  QString name  = mapsStr.at(i1);
  mapsStr.remove(i1);
  mapsStr.insert(i2, name);

  const QPixmap pixmap = *mapsList->pixmap(i1);
  mapsList->changeItem(*mapsList->pixmap(i2), i1);
  mapsList->changeItem(pixmap, i2);
  mapsList->setCurrentItem(i1);
  changeData();
}
void KiKbdMapsWidget::infoMap()
{
  QDialog dialog(this, "", TRUE);
  dialog.setCaption(i18n("Keyboard map information"));

  QBoxLayout *box;
  QBoxLayout *topLayout = new QVBoxLayout(&dialog, 5);
  QGroupBox  *group = new QGroupBox("", &dialog);
  topLayout->addWidget(group, 20);
  // label
  KiKbdMapInfoWidget *label = new KiKbdMapInfoWidget(group);
  label->changeMap(mapsStr.at(mapsList->currentItem()));
  box = new QVBoxLayout(group, 10);
  box->addWidget(label, 10);
  box->addStretch(1);
  box->activate();
  // ok button
  QPushButton *ok = new QPushButton(i18n("OK"), &dialog);
  ok->setFixedSize(ok->sizeHint());
  ok->setFocus();
  ok->setDefault(TRUE);
  connect(ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  topLayout->addLayout(box=new QHBoxLayout(5));
  box->addStretch(10);
  box->addWidget(ok);
  box->addStretch(10);

  topLayout->activate();
  dialog.resize(0, 0);
  dialog.exec();
}

//=====================================================
// custom dialogs
//=====================================================
KiKbdAddDialog::KiKbdAddDialog(QWidget* parent):QDialog(parent, "", TRUE)
{
  setCaption(i18n("Add Keyboard"));

  topLayout = new QVBoxLayout(this, 5);
  // info + combo
  QGroupBox  *group = new QGroupBox(i18n("Available keyboard maps"), 
				    this);
  topLayout->addWidget(group, 10);
  groupLayout = new QVBoxLayout(group, 20);
  maps        = new QComboBox(group);
  label       = new KiKbdMapInfoWidget(this);
  groupLayout->addWidget(maps);
  groupLayout->addWidget(label, 10);
  maps->setMinimumSize(maps->sizeHint());
  //--- buttons
  QBoxLayout  *buttons = new QHBoxLayout(10);
  QPushButton *ok      = new QPushButton(i18n("OK"), this);
  QPushButton *cancel  = new QPushButton(i18n("Cancel"), this);  
  topLayout->addLayout(buttons, 2);
  QSize size1 = ok->sizeHint();
  QSize size2 = cancel->sizeHint();
  if(size1.width () < size2.width ()) size1.setWidth (size2.width() );
  if(size1.height() < size2.height()) size1.setHeight(size2.height());
  ok    ->setFixedSize(size1);
  cancel->setFixedSize(size1);
  connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
  buttons->addStretch(10);
  buttons->addWidget(ok);
  buttons->addWidget(cancel);
  buttons->addStretch(10);
  ok->setFocus();
  ok->setDefault(TRUE);
  connect(maps, SIGNAL(activated(int)), SLOT(setInfo(int)));
}
void KiKbdAddDialog::setInfo(int i)
{
  label->changeMap(mapsToAdd.at(i));
  groupLayout->activate();
  topLayout->activate();
  resize(0, 0);
}
int KiKbdAddDialog::exec(const QStrList& toAdd, QStrList& mapStr) 
{ 
  mapsToAdd = toAdd;
  //--- load list of maps
  // also look up prefered language
  QStrList preferedLang;
  int *prefIndex = new int[4];
  unsigned j;for(j = 0; j < 4; prefIndex[j++]=-1);
  preferedLang = kapp->getLocale()->languageList();
  if(preferedLang.find("en") == -1) preferedLang.insert(0, "en");
  // add
  unsigned i;for(i = 0; i < mapsToAdd.count(); i++) {
    KiKbdMapConfig *map = kikbdConfig->getMap(mapsToAdd.at(i));
    maps->insertItem(mapLine(map));
    for(j = 0; j < 4; j++)
      if((prefIndex[j] == -1) && preferedLang.at(j) == map->getLocale()) {
	int k;for(k = mapStr.count(); k-- > 0;) {
	  KiKbdMapConfig *map1 = kikbdConfig->getMap(mapStr.at(k));
	  if(map->getLocale() == map1->getLocale()
	     && map->getCharset() == map1->getCharset()) break;
	}
	if(k < 0) prefIndex[j] = i;
      }
  }
  int prefIndex0;
  for(j=0; j<4 && (prefIndex0=prefIndex[j],prefIndex0 == -1);
      prefIndex0=0, j++);
  delete prefIndex;
  maps->setCurrentItem(prefIndex0);
  QListBox *box = maps->listBox();
  if(box) {
    box->setCurrentItem(prefIndex0);
    box->centerCurrentItem();
  }
  setInfo(prefIndex0);
  
  //--- execute dialog
  return QDialog::exec();
}
KiKbdMapInfoWidget::KiKbdMapInfoWidget(QWidget* parent):QLabel(parent)
{
  setAlignment(WordBreak);
}
void KiKbdMapInfoWidget::changeMap(const char* map)
{
  QString      text  = kikbdConfig->getMap(map)->getInfo();
  QStrList     tlist = KObjectConfig::separate(text, '\n');
  QFontMetrics font  = fontMetrics();
  int width = 0, height = tlist.count();
  for(unsigned i=0; i<tlist.count(); i++) {
    int w = font.width(tlist.at(i));
    if(w > width) width = w;
  }
  if(width > 400) {
    width = 400;
    height += 4;
  }
  height *= font.height();

  setMinimumSize(width, height);
  setText(text);
}

//=====================================================
// configurations widgets
//=====================================================
KiKbdGeneralWidget::KiKbdGeneralWidget(QWidget* parent)
  :QWidget(parent)
{
  QBoxLayout  *topLayout = new QVBoxLayout(this, 20);
  QGroupBox   *group = new QGroupBox(i18n("Keyboard maps"), this);
  QVBoxLayout *mbox  = new QVBoxLayout(group, 20);
  QBoxLayout  *hbox;
  QWidget     *wid;
  
  topLayout->addWidget(group, 10);

  //--- keyboards group
  mbox->addWidget(new KiKbdMapsWidget(group));

  //--- switches group
  group = new QGroupBox(i18n("Switch and Alt Switch"), this);
  hbox  = new QHBoxLayout(group, 20);

  hbox->addWidget(wid=kikbdConfig->
		  createWidget(&kikbdConfig->getSwitchRef(), group, 0L,
			       gettext("Key(s) for switching between "
				       "keyboard maps")));
  connect(wid, SIGNAL(activated(const char*)),  SLOT(newSwitch(const char*)));

  hbox->addWidget(wid=kikbdConfig->
		  createWidget(&kikbdConfig->getAltSwitchRef(), group, 0L,
			       gettext("Key for activating Alternate "
				       "symbols in current keyboard map")));
  connect(this, SIGNAL(activateAltSwitch(bool)), wid, SLOT(setEnabled(bool)));
  group->setMinimumHeight(2*wid->height());

  topLayout->addWidget(group);

  //--- options group
  group = new QGroupBox(i18n("Options"), this);
  hbox  = new QHBoxLayout(group, 20);

  hbox->addWidget(kikbdConfig->
		  createWidget(&kikbdConfig->getKeyboardBeep(), group,
			       gettext("&Beep"),
			       gettext("Beep when keyboard "
				       "map is changed")));
  wid = mkButton(group, hbox, SLOT(advanced()), this,
		 gettext("Ad&vanced"), gettext("Advanced options"));
  group->setMinimumHeight(2*wid->height());
  topLayout->addWidget(group);

  newSwitch();
  connect(kikbdConfig->find(&kikbdConfig->getMaps()), SIGNAL(dataChanged()),
	  SLOT(listChanged()));
}
void KiKbdGeneralWidget::newSwitch(const char*)
{
  emit activateAltSwitch((!kikbdConfig->oneKeySwitch())
			 && kikbdConfig->hasAltKeys());
}

void KiKbdGeneralWidget::advanced()
{
  QDialog dialog(this, "", TRUE);
  dialog.setCaption(i18n("Advanced"));

  QWidget    *wid, *wid2;
  QBoxLayout *topLayout = new QVBoxLayout(&dialog, 5);
  QGroupBox  *group     = new QGroupBox(&dialog);
  QBoxLayout *groupLayout = new QVBoxLayout(group, 20);
  QBoxLayout *hbox;

  topLayout->addWidget(group, 10);

  //--- kpanel menu
  hbox = new QHBoxLayout();
  groupLayout->addLayout(hbox);
  // emulate capslock
  hbox->addWidget(wid=kikbdConfig->
		  createWidget(&kikbdConfig->getEmuCapsLock(), group,
			       gettext("&Emulate Caps Lock"), 
			       gettext("Emulate XServer "
				       "Caps Lock. Needed for correct "
				       "keyboard operation in some languages")));
  // auto menu
  hbox->addWidget(kikbdConfig->
		  createWidget(&kikbdConfig->getAutoMenu(), group,
			       gettext("&World Menu"), 
			       gettext("Show menu in any window by "
				       "holding sown the selected "
				       "Switch key(s)")));
  // save classes
  hbox = new QHBoxLayout();
  groupLayout->addLayout(hbox);
  hbox->addWidget(wid2=kikbdConfig->
		  createWidget(&kikbdConfig->getSaveClasses(), group,
			       gettext("&Save Classes"), 
			       gettext("Save relations between window "
				       "classes and keyboard maps on exit")));
  // custom codes
  hbox->addWidget(kikbdConfig->
		  createWidget(&kikbdConfig->getCodes(), group,
			       0L, 
			       gettext("Define X codes")));
  // input
  QButtonGroup* butg = (QButtonGroup*)kikbdConfig->
    createWidget(&kikbdConfig->getInput(), group, gettext("Input"));
  hbox = new QHBoxLayout(butg, 15);
  // global
  hbox->addWidget(wid=butg->find(0));
  addToolTip(wid, gettext("Standard behavior. Keyboard map effective for "
			  "all windows"));
  // window
  hbox->addWidget(wid=butg->find(1));
  addToolTip(wid, gettext("Extended behavior. Keyboard map state is saved for "
			  "each window independently"));
  // class
  hbox->addWidget(wid=butg->find(2));
  addToolTip(wid, gettext("Special behavior. Keyboard state is saved for each "
			  "window class independently"));

  groupLayout->addWidget(butg);
  connect(wid, SIGNAL(toggled(bool)), wid2, SLOT(setEnabled(bool)));
  wid2->setEnabled(butg->find(2)->isOn());

  //--- Ok button
  QPushButton *ok = new QPushButton(i18n("OK"), &dialog);
  ok->setFixedSize(ok->sizeHint());
  ok->setFocus();
  ok->setDefault(TRUE);
  connect(ok, SIGNAL(clicked()), &dialog, SLOT(accept()));

  hbox = new QHBoxLayout(10);
  topLayout->addLayout(hbox, 2);
  hbox->addStretch(10);
  hbox->addWidget(ok);
  hbox->addStretch(10);

  groupLayout->activate();
  topLayout->activate();

  dialog.resize(0, 0);  
  dialog.exec();
}

/**
   Style widget
   colors for normal, background, caps, alternate button
   button font
*/
void makeColorButton (QVBoxLayout* layout, QWidget* but, QLabel*& label,
			  const char* text, const char* tip) {
  QHBoxLayout* hbox = new QHBoxLayout();
  label = new QLabel(i18n(text), but->parentWidget());

  layout->addLayout(hbox);
  but->setMinimumSize(100, 30);
  label->setMinimumSize(label->sizeHint());
  hbox->addWidget(but  , 0);
  hbox->addWidget(label, 0);
  hbox->addStretch(10);
  addToolTip(but, tip);
}
KiKbdStyleWidget::KiKbdStyleWidget(QWidget* parent):QWidget(parent)
{
  QBoxLayout *topLayout = new QVBoxLayout(this, 20);
  QGroupBox *group = new QGroupBox(i18n("Button Colors"),
				   this);
  QVBoxLayout *vbox = new QVBoxLayout(group, 25);
  QHBoxLayout *hbox;
  QLabel      *label;
  QWidget     *but;

  topLayout->addWidget(group, 10);
  // foreground button color
  but = kikbdConfig->createWidget(&kikbdConfig->getForColor(), group);
  makeColorButton(vbox, but, label, gettext("Foreground"), 
		  gettext("Text label foreground color"));

  // background button color
  but = kikbdConfig->createWidget(&kikbdConfig->getBakColor(), group);
  makeColorButton(vbox, but, label, gettext("Background"), 
		  gettext("Text label background color"));

  // caps  color
  but = kikbdConfig->createWidget(&kikbdConfig->getCapsColor(), group);
  makeColorButton(vbox, but, label, gettext("With Caps Lock"), 
		  gettext("Background when Emulated Caps Lock active"));
  connect(this, SIGNAL(enableCaps(bool)), but, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(enableCaps(bool)), label, SLOT(setEnabled(bool)));

  //  alternate color
  but = kikbdConfig->createWidget(&kikbdConfig->getAltColor(), group);
  makeColorButton(vbox, but, label, gettext("With Alternate"), 
		  gettext("Background when Alternate switch key is pressed"));
  connect(this, SIGNAL(enableAlternate(bool)), but, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(enableAlternate(bool)), label, SLOT(setEnabled(bool)));

  vbox->addStretch(10);

  /**
     font
  */
  group = new QGroupBox(i18n("Button Font"), this);
  topLayout->addWidget(group, 0);
  vbox = new QVBoxLayout(group, 20);
  hbox = new QHBoxLayout();
  vbox->addLayout(hbox);

  QWidget *but2 = kikbdConfig->
    createWidget(&kikbdConfig->getCustFont(), group, 
		 gettext("&Customize Font"),
		 gettext("Customize Font for text label or use global "
			 "settings"));
  hbox->addWidget(but2, 2);
  but = kikbdConfig->createWidget(&kikbdConfig->getFont(), group,
				  gettext("C&hange Font"));
  but->setMinimumSize(but->sizeHint());
  hbox->addWidget(but);
  hbox->addStretch(5);
  vbox->addStretch(5);
  group->setMinimumHeight(3*but->height());
  connect(but2, SIGNAL(toggled(bool)), but, SLOT(setEnabled(bool)));
  but->setEnabled(kikbdConfig->getCustFont());

  /** We whant to disable caps color and alt color depending
      on altswitch, switch and emucapslock
  */
  connect(kikbdConfig->find(&kikbdConfig->getSwitchRef()), 
	  SIGNAL(dataChanged()), SLOT(checkShow()));
  connect(kikbdConfig->find(&kikbdConfig->getAltSwitchRef()), 
	  SIGNAL(dataChanged()), SLOT(checkShow()));
  connect(kikbdConfig->find(&kikbdConfig->getEmuCapsLock()), 
	  SIGNAL(dataChanged()), SLOT(checkShow()));
}
void KiKbdStyleWidget::checkShow()
{
  emit enableCaps(kikbdConfig->getEmuCapsLock());
  emit enableAlternate(!kikbdConfig->oneKeySwitch() 
		       && kikbdConfig->hasAltKeys()
		       && strcmp(kikbdConfig->getAltSwitchRef(), "None"));
}

//=========================================================
//  startup widget
//=========================================================
KiKbdStartupWidget::KiKbdStartupWidget(QWidget* parent):QWidget(parent)
{
  QBoxLayout *topLayout = new QVBoxLayout(this, 20);
  QGroupBox  *group     = new QGroupBox("", this);
  QBoxLayout *vbox;
  QWidget    *widget;

  topLayout->addWidget(group, 5);
  vbox  = new QVBoxLayout(group, 20);
  /**
     Autostart?
  */
  vbox->
    addWidget(kikbdConfig->
	      createWidget(&kikbdConfig->getAutoStart(), group,
			   gettext("&Autostart"),
			   gettext("Start up automatically")), 0);
  /**
     Do docking?
  */
  vbox->addWidget(widget=kikbdConfig->
		  createWidget(&kikbdConfig->getDocking(), group, 
			       gettext("&Docked"),
			       gettext("Dock into special area in the panel")),
		  0);
  connect(widget, SIGNAL(toggled(bool)), SLOT(slotInvert(bool)));

  /**
     place to start
  */
  QBoxLayout *hbox = new QHBoxLayout();
  vbox->addLayout(hbox);
  hbox->addWidget(widget=new QLabel(i18n("Place"), group), 0);
  widget->setMinimumSize(widget->sizeHint());
  widget->setEnabled(!kikbdConfig->getDocking());
  connect(this, SIGNAL(signalInvert(bool)), widget, SLOT(setEnabled(bool)));

  hbox->addWidget(widget=kikbdConfig->
		  createWidget(&kikbdConfig->getAutoStartPlace(), group,
			       gettext("Place in selected corner")), 0);
  connect(this, SIGNAL(signalInvert(bool)), widget, SLOT(setEnabled(bool)));
  widget->setEnabled(!kikbdConfig->getDocking());
  hbox->addStretch(5);

  vbox->addSpacing(20);
  topLayout->addStretch(20);
}
