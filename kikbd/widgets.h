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
#ifndef _WIDGETS_H
#define _WIDGETS_H

#include <qwidget.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qlistbox.h>
#include <qstrlist.h>
#include <qdialog.h>
#include <qgrpbox.h>
#include <qlined.h>
#include <qlayout.h>
#include <qlabel.h>

#include <drag.h>
#include <kcontrol.h>
#include <ktablistbox.h>
#include "kikbdconf.h"

class KiKbdMapInfoWidget: public QLabel {
  Q_OBJECT
 public:
  KiKbdMapInfoWidget(QWidget*);
 public slots:
  void changeMap(const char*);
};

class KiKbdMapsWidget: public QWidget {
  Q_OBJECT
 protected:
  QListBox  *mapsList;
  QStrList  &mapsStr;  
  int        stopFlag;
 protected:
  void   addMap(const char*);
  void   changeMap(int);
  void   chkActive ();
  void   changeData();
 public:
  KiKbdMapsWidget(QWidget* parent=0L);
 public slots:
  void addMap   ();
  void upMap    (){changeMap(-1);}
  void downMap  (){changeMap(+1);}
  void infoMap  ();
  void deleteMap();
  void selected   (int){emit infoClick();}
  void highlighted(int){chkActive();}
  void selectionUp  ();
  void selectionDown();
  void dataChanged  ();
 signals:
  void activateDelete(bool);
  void activateUp    (bool);
  void activateDown  (bool);
  void activateInfo  (bool);
  void activateHot   (bool);
  void infoClick();
};

class KiKbdAddDialog: public QDialog {
  Q_OBJECT
 protected:
  QStrList    mapsToAdd;
  QComboBox  *maps;
  QBoxLayout *topLayout, *groupLayout;
  KiKbdMapInfoWidget *label;
 public:
  KiKbdAddDialog(QWidget* parent);
  int exec(const QStrList&, QStrList&);
  int selectedMap() const {return maps->currentItem();}
 public slots:
  void setInfo(int);
};

class KiKbdGeneralWidget: public QWidget {
  Q_OBJECT
 protected:
  friend class KiKbdAdvancedDialog;
 public:
  KiKbdGeneralWidget(QWidget*);
 public slots:
  void advanced();
  void newSwitch(const char*p=0L);
  void listChanged() {newSwitch();}
 signals:
  void activateAltSwitch(bool);
};

class KiKbdStyleWidget: public QWidget {
  Q_OBJECT
 public:
  KiKbdStyleWidget(QWidget*);
 public slots:
  void checkShow();
 signals:
  void enableAlternate(bool);
  void enableCaps(bool);
};

class KiKbdStartupWidget: public QWidget {
  Q_OBJECT
 public:
  KiKbdStartupWidget(QWidget*);
 public slots:
  void slotInvert(bool f){emit signalInvert(!f);}
 signals:
  void signalInvert(bool f);
};

extern KiKbdConfig *kikbdConfig;

#endif
