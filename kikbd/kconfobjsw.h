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

#ifndef K_CONF_OBJSW
#define K_CONF_OBJSW

#include <qchkbox.h>
#include <qcombo.h>
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qradiobt.h>

#include <kcolordlg.h>
#include <kfontdialog.h>
#include <kcolorbtn.h>
#include <kapp.h>

#include "kconfobjs.h"

/**
  * Boolean Config Object Widget
  */
class KConfigBoolWidget: public KConfigObjectWidget {
 public:
  KConfigBoolWidget(KConfigBoolObject* obj, const char* label,
		    QWidget* parent=0L);
  virtual void dataChanged();
  virtual void changeData();
};

/**
  * QColor Config Object Widget
  */
class KConfigColorWidget: public KConfigObjectWidget {
  Q_OBJECT
 public:
  KConfigColorWidget(KConfigColorObject* obj, QWidget* parent=0L);
  virtual void dataChanged();
  virtual void changeData();
 public slots:
  void changeData(const QColor&) {changeData();}
};

/**
  * QFont Config Object Widget
  */
class KConfigFontWidget: public KConfigObjectWidget {
  Q_OBJECT
 public:
  KConfigFontWidget(KConfigFontObject* obj, const char* label,
		    QWidget* parent=0L);
  virtual void dataChanged();
  virtual void changeData();
 public slots:
  void activated();
};

/**
  * Combo Config Object Widget
  */
class KConfigComboWidget: public KConfigObjectWidget {
  Q_OBJECT
 public:
  KConfigComboWidget(KConfigComboObject* obj, const char* name,
		     QWidget* parent=0L);
 public slots:
  virtual void dataChanged();
  virtual void changeData();
  virtual void changeData(int) {changeData();}
};


#endif
