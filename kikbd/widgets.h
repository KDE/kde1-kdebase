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

#include <drag.h>
#include <kcontrol.h>
#include <ktablistbox.h>
#include "kikbdconf.h"

class KiKbdGeneralWidget: public KConfigWidget {
  Q_OBJECT
 protected:
  KTabListBox  *mapsList;
  QComboBox *altSwitchBox, *switchBox;
  QStrList &mapsStr, mapsToAdd;
  void   addMap(const char*);
  void   chkActivate();
  friend class KiKbdAdvancedDialog;
  friend class KiKbdAddMapDialog;
  virtual void resizeEvent(QResizeEvent*);
 public:
  KiKbdGeneralWidget(QWidget*, const char*);
  void loadSettings();
  void applySettings(){}
 public slots:
  void addMap();
  void upMap();
  void downMap();
  void infoMap();
  void deleteMap();
  void highlighted(int, int){chkActivate();}
  void selected(int, int){emit infoClick();}
  void advanced();
  void setLongComment(int);
  void newSwitch(const char*);
  QString mapInfo(const char*) const;
 signals:
  void activateDelete(bool);
  void activateUp(bool);
  void activateDown(bool);
  void activateInfo(bool);
  void activateHot(bool);
  void setLongComment(const char*);
  void infoClick();
};

class KiKbdStyleWidget: public KConfigWidget {
  Q_OBJECT
 protected:
 public:
  KiKbdStyleWidget(QWidget*, const char*);
  void loadSettings(){}
  void applySettings(){}
 public slots:
  void aboutToShow(const char*);
 signals:
  void enableAlternate(bool);
  void enableCaps(bool);
};

class KiKbdStartupWidget: public KConfigWidget {
  Q_OBJECT
 public:
  KiKbdStartupWidget(QWidget*, const char*);
  void loadSettings(){}
  void applySettings(){}
 public slots:
  void slotInvert(bool);
 signals:
  void signalInvert(bool);
};

extern KiKbdConfig *kikbdConfig;

#endif
