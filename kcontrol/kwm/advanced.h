/*
 * advanced.h
 *
 * Copyright (c) 1998 Cristian Tibirna ctibirna@gch.ulaval.ca
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 */

#ifndef __KADVCONFIG_H__
#define __KADVCONFIG_H__

#include <qlabel.h>
#include <qframe.h>
#include <qpainter.h>
#include <qgrpbox.h>
#include <qchkbox.h>
#include <qpushbt.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>

#include <kiconloader.h>
#include <kcontrol.h>

// kwm configs and values

#define CTRLTAB            "ControlTab"
#define CTAB_ON            1
#define CTAB_OFF           0

#define TRALL              "TraverseAll"
#define TRALL_ON           1
#define TRALL_OFF          0

#define B3GRAB             "Button3Grab"
#define B3_ON              1
#define B3_OFF             0

#define AT_MODE            "AltTabMode"
#define ATM_KDE            0
#define ATM_CDE            1

#define TDECORTTL          "tinyDecorationTitles"
#define TDECORCLS          "tinyDecorationClasses"
#define NDECORTTL          "noDecorationTitles"
#define NDECORCLS          "noDecorationClasses"
#define NFOCUSTTL          "noFocusTitles"
#define NFOCUSCLS          "noFocusClasses"
#define STICKYTTL          "stickyTitles"
#define STICKYCLS          "stickyClasses"
#define SESSIGNORE         "proxyignore"

enum filterLists {
  L_TDECTTL,
  L_TDECCLS,
  L_NDECTTL,
  L_NDECCLS,
  L_NFOCTTL,
  L_NFOCCLS,
  L_STICTTL,
  L_STICCLS,
  L_SESSTTL
};

enum bigComboItems {
  TDEC,
  NDEC,
  NFOC,
  STIC,
  SESS
};


class myListBrowser : public QWidget {

  Q_OBJECT

 public:

  myListBrowser(const char *, QWidget *, const char *);
  ~myListBrowser();

  void feedList(const QStrList *);

  virtual void setEnabled(bool);

 protected:

 protected slots:
  void bEditChanged(const char *);
  void addIt();
  void deleteIt();
  void itemSelected();
  void itemHilited();

 private:

  QGroupBox *browserBox;
  QPushButton *bAdd, *bDel;
  QLineEdit *bEdit;
  QListBox *bList;

};  

class KAdvancedConfig : public KConfigWidget {

 Q_OBJECT

 public:

  KAdvancedConfig( QWidget *parent=0, const char* name=0);
  ~KAdvancedConfig( );

  void saveSettings( void );

  void loadSettings();
  void applySettings();

 protected:


 protected slots:

  void filterSelected(int);

 private:

  QStrList lists[L_SESSTTL+1];

  QGroupBox *keyBox;
  QCheckBox *ctrltab, *trall, *b3grab;
  QLabel *atLabel;
  QComboBox *alttab;

  QGroupBox *filterBox;
  QLabel *wLabel, *ifLabel;
  QComboBox *opCombo;

  myListBrowser *tList, *cList;

  bool getCtrlTab();
  void setCtrlTab(bool);
  bool getTrAll();
  void setTrAll(bool);
  bool getB3Grab();
  void setB3Grab(bool);
  int getATMode();
  void setATMode(int);

};


#endif
