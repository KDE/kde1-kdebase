/*
 * advanced.cpp
 *
 * Copyright (c) 1998 Cristian Tibirna ctibirna@gch.ulaval.ca
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 * Licensing issues to solve, fast (will be GPL or - more likely - Artistic)
 *
 */


#include <qlayout.h>
#include <kfiledialog.h>
#include <kapp.h>


#include <ksimpleconfig.h>

#include "advanced.h"



extern KConfig *config;

KAdvancedConfig::~KAdvancedConfig () {
}


KAdvancedConfig::KAdvancedConfig(QWidget * parent, const char *name)
  : KConfigWidget (parent, name) {

  QBoxLayout *lay = new QVBoxLayout(this,5);

  keyBox = new QGroupBox(klocale->translate("Keyboard and Mouse"), this);

  QGridLayout *chkLay = new QGridLayout(keyBox,5,2,5);
  chkLay->addRowSpacing(0,10);
  chkLay->setRowStretch(0,0);
  chkLay->setRowStretch(1,1);
  chkLay->setRowStretch(2,1);
  chkLay->setRowStretch(3,0);
  chkLay->setRowStretch(4,1);

  ctrltab = new QCheckBox(klocale->translate("Ctrl-Tab walks through desktops"),keyBox);
  ctrltab->adjustSize();
  ctrltab->setMinimumSize(ctrltab->size());
  chkLay->addMultiCellWidget(ctrltab,1,1,0,1);

  trall = new QCheckBox(klocale->translate("Alt-Tab is limited to current desktop"),keyBox);
  trall->adjustSize();
  trall->setMinimumSize(trall->size());
  chkLay->addMultiCellWidget(trall,2,2,0,1);

  atLabel = new QLabel(klocale->translate("Alt-Tab mode:"),keyBox);
  atLabel->adjustSize();
  atLabel->setMinimumSize(atLabel->size());
  atLabel->setAlignment(AlignHCenter | AlignVCenter);
  chkLay->addWidget(atLabel,3,0);

  alttab = new QComboBox(FALSE, keyBox, "AltTab");
  alttab->insertItem(klocale->translate("KDE"),ATM_KDE);
  alttab->insertItem(klocale->translate("CDE"),ATM_CDE);
  alttab->adjustSize();
  alttab->setMinimumSize(alttab->size());
  chkLay->addWidget(alttab,3,1);

  b3grab = new QCheckBox(klocale->translate("Grab the Right Mouse Button"),keyBox);
  b3grab->adjustSize();
  b3grab->setMinimumSize(atLabel->size());
  chkLay->addMultiCellWidget(b3grab,4,4,0,1);

  chkLay->activate();

  lay->addWidget(keyBox);

  filterBox = new QGroupBox(klocale->translate("Filters"),this);

  chkLay = new QGridLayout(filterBox,5,5,5);
  chkLay->addRowSpacing(0,10);
  chkLay->addColSpacing(0,5);
  chkLay->addColSpacing(2,10);
  chkLay->addColSpacing(4,5);

  chkLay->setRowStretch(0,0);
  chkLay->setRowStretch(1,0);
  chkLay->setRowStretch(2,0);
  chkLay->setRowStretch(3,0);
  chkLay->setRowStretch(4,1);

  chkLay->setColStretch(0,0);
  chkLay->setColStretch(1,1);
  chkLay->setColStretch(2,0);
  chkLay->setColStretch(3,1);
  chkLay->setColStretch(4,0);

  wLabel = new QLabel(klocale->translate("Windows will:"),filterBox);
  wLabel->adjustSize();
  wLabel->setMinimumSize(wLabel->size());
  chkLay->addMultiCellWidget(wLabel,1,1,1,4);

  opCombo = new QComboBox(FALSE,filterBox, "Filters");
  opCombo->insertItem(klocale->translate("have tiny decorations"),TDEC);
  opCombo->insertItem(klocale->translate("have no decorations"),NDEC);
  opCombo->insertItem(klocale->translate("never gain focus"),NFOC);
  opCombo->insertItem(klocale->translate("start as Sticky"),STIC);
  opCombo->insertItem(klocale->translate("be excluded from session management"),SESS);
  opCombo->setCurrentItem(TDEC);
  opCombo->adjustSize();
  opCombo->setMinimumSize(opCombo->size());
  chkLay->addMultiCellWidget(opCombo,2,2,1,4);

  connect(opCombo, SIGNAL(activated(int)),this,SLOT(filterSelected(int)));

  ifLabel = new QLabel(klocale->translate("if they match the following:"),filterBox);
  ifLabel->adjustSize();
  ifLabel->setMinimumSize(ifLabel->size());
  chkLay->addMultiCellWidget(ifLabel,3,3,1,4);

  tList = new myListBrowser(klocale->translate("Titles"),filterBox,"tList");
  tList->adjustSize();
  tList->setMinimumSize(tList->size());
  chkLay->addWidget(tList,4,1);

  cList = new myListBrowser(klocale->translate("Classes"),filterBox,"cList");
  cList->adjustSize();
  cList->setMinimumSize(cList->size());
  chkLay->addWidget(cList,4,3);

  chkLay->activate();

  lay->addWidget(filterBox);

  lay->activate();

  loadSettings();

}

void KAdvancedConfig::filterSelected(int item) {

  if (item < 4) {
    cList->setEnabled(TRUE);
    tList->feedList(&lists[item*2]);
    cList->feedList(&lists[item*2+1]);
  }
  else {
    cList->setEnabled(FALSE);
    tList->feedList(&lists[item*2]);
  }
}

bool KAdvancedConfig::getCtrlTab() {
 return (ctrltab->isChecked());
}

void KAdvancedConfig::setCtrlTab(bool a) {
  ctrltab->setChecked(a);
}

bool KAdvancedConfig::getTrAll() {
 return (!trall->isChecked());
}

void KAdvancedConfig::setTrAll(bool a) {
  trall->setChecked(!a);
}

bool KAdvancedConfig::getB3Grab() {
 return (b3grab->isChecked());
}

void KAdvancedConfig::setB3Grab(bool a) {
  b3grab->setChecked(a);
}

int KAdvancedConfig::getATMode() {
  return alttab->currentItem();
}

void KAdvancedConfig::setATMode (int a) {
  alttab->setCurrentItem(a);
}

void KAdvancedConfig::loadSettings() {

  QString key;

  config->setGroup( "General" );

  key = config->readEntry(CTRLTAB, "on");
  setCtrlTab( key == "on" );

  key = config->readEntry(TRALL, "on");
  setTrAll( key == "on" );

  key = config->readEntry(B3GRAB, "on");
  setB3Grab( key == "on" );

  key = config->readEntry(AT_MODE, "KDE");

  if (key == "CDE") setATMode( ATM_CDE );
  else setATMode( ATM_KDE );

  config->setGroup( "Decoration" );

  config->readListEntry(TDECORTTL,lists[L_TDECTTL]);
  config->readListEntry(TDECORCLS,lists[L_TDECCLS]);
  config->readListEntry(NDECORTTL,lists[L_NDECTTL]);
  config->readListEntry(NDECORCLS,lists[L_NDECCLS]);

  config->setGroup( "Focus" );

  config->readListEntry(NFOCUSTTL,lists[L_NFOCTTL]);
  config->readListEntry(NFOCUSCLS,lists[L_NFOCCLS]);

  config->setGroup( "Sticky" );

  config->readListEntry(STICKYTTL,lists[L_STICTTL]);
  config->readListEntry(STICKYCLS,lists[L_STICCLS]);

  config->setGroup( "Session" );

  config->readListEntry(SESSIGNORE,lists[L_SESSTTL]);

  filterSelected(TDEC);

}

void KAdvancedConfig::saveSettings() {

  config->setGroup( "General" );

  config->writeEntry(CTRLTAB,getCtrlTab()?"on":"off");
  config->writeEntry(TRALL,getTrAll()?"on":"off");
  config->writeEntry(B3GRAB,getB3Grab()?"on":"off");
  config->writeEntry(AT_MODE,(getATMode() == ATM_KDE)?"KDE":"CDE");

  //CT save lists
  filterSelected(opCombo->currentItem());
  //CT

  config->setGroup( "Decoration" );

  config->writeEntry(TDECORTTL,lists[L_TDECTTL]);
  config->writeEntry(TDECORCLS,lists[L_TDECCLS]);
  config->writeEntry(NDECORTTL,lists[L_NDECTTL]);
  config->writeEntry(NDECORCLS,lists[L_NDECCLS]);

  config->setGroup( "Focus" );

  config->writeEntry(NFOCUSTTL,lists[L_NFOCTTL]);
  config->writeEntry(NFOCUSCLS,lists[L_NFOCCLS]);

  config->setGroup( "Sticky" );

  config->writeEntry(STICKYTTL,lists[L_STICTTL]);
  config->writeEntry(STICKYCLS,lists[L_STICCLS]);

  config->setGroup( "Session" );

  config->writeEntry(SESSIGNORE,lists[L_SESSTTL]);

  config->sync();
}

void KAdvancedConfig::applySettings() {

  saveSettings();
}

myListBrowser::~myListBrowser() {
}

myListBrowser::myListBrowser(const char *title, QWidget *parent, const char *name)
 : QWidget(parent,name) {

  victimList = new QStrList; //CT this is the pointer to the edited list

  QBoxLayout *genLay = new QVBoxLayout(this,5);

  browserBox = new QGroupBox(title, this);

  QGridLayout *lay = new QGridLayout(browserBox,4,3,3);
  lay->addRowSpacing(0,10);
  lay->addRowSpacing(2, 5);

  lay->setRowStretch(0,0);
  lay->setRowStretch(1,0);
  lay->setRowStretch(2,0);
  lay->setRowStretch(3,1);

  lay->setColStretch(0,1);
  lay->setColStretch(1,0);
  lay->setColStretch(2,0);

  bEdit = new QLineEdit(browserBox, "tEdit");//?
  bEdit->adjustSize();
  bEdit->setMinimumSize(bEdit->size());
  lay->addWidget(bEdit,1,0);

  connect(bEdit, SIGNAL(textChanged(const char *)),
	  this, SLOT(bEditChanged(const char *)));

  bAdd = new QPushButton("+",browserBox);
  bAdd->adjustSize();
  bAdd->setFixedWidth(20);
  bAdd->setMinimumSize(bAdd->size());
  lay->addWidget(bAdd,1,1);

  bAdd->setEnabled(FALSE);
  connect(bAdd, SIGNAL(clicked()), this, SLOT(addIt()));

  bDel = new QPushButton("-",browserBox);
  bDel->adjustSize();
  bDel->setFixedWidth(20);
  bDel->setMinimumSize(bDel->size());
  lay->addWidget(bDel,1,2);

  bDel->setEnabled(FALSE);
  connect(bDel, SIGNAL(clicked()), this, SLOT(deleteIt()));

  bList = new QListBox(browserBox);//?
  //  tList->clearList();//?
  bList->adjustSize();
  bList->setMinimumSize(bList->size());
  lay->addMultiCellWidget(bList,3,3,0,2);

  bList->setMultiSelection(FALSE);
  bList->clearSelection();
  connect(bList, SIGNAL(selected(int)), this, SLOT(itemSelected()));
  connect(bList, SIGNAL(highlighted(int)), this, SLOT (itemHilited()));

  lay->activate();

  genLay->addWidget(browserBox);

  genLay->activate();

}

void myListBrowser::setEnabled(bool a) {
  /* bAdd->setEnabled(a);
     bDel->setEnabled(a);
     dEdit->setEnabled(a);
     bList->setEnabled(a);*/
  browserBox->setEnabled(a);
  //CT 13Apr1999 - why a groupbox doesn't disable/enable its children
  bEdit->setEnabled(a);
  bAdd ->setEnabled(a);
  bDel ->setEnabled(a);
  bList->setEnabled(a);
  if(a) {
     bAdd->setEnabled(FALSE);
     bDel->setEnabled(FALSE);
     bEdit->setText("");
     bList->clearSelection();
  }
}

void myListBrowser::feedList(QStrList *thisList) {

  victimList->clear();
  for(unsigned int i = 0; i < bList->count(); i++) 
    victimList->insert(i, bList->text(i));
    
  bList->clear();

  bList->insertStrList(thisList);

  victimList = thisList;
}

void myListBrowser::bEditChanged(const char *a) {
  bool not_empty = strcmp(a, "");
  bAdd->setEnabled(not_empty);
  bDel->setEnabled(not_empty);
}

void myListBrowser::addIt() {
  bAdd->setEnabled(FALSE);
  bDel->setEnabled(FALSE);
  bList->insertItem((const char *)(bEdit->text()));
  bEdit->setText("");
  bList->clearSelection();
}

void myListBrowser::deleteIt() {
  bAdd->setEnabled(FALSE);
  bDel->setEnabled(FALSE);
  bList->removeItem(bList->currentItem());
  bList->clearSelection();
}

void myListBrowser::itemSelected() {
  bAdd->setEnabled(TRUE);
  bDel->setEnabled(TRUE);
  bEdit->setText((const char *)(bList->text(bList->currentItem())));
}

void myListBrowser::itemHilited() {
  bAdd->setEnabled(FALSE);
  bDel->setEnabled(FALSE);
  bEdit->setText("");
}

#include "advanced.moc"
