/*
 * locale.h
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


#ifndef __KLOCALECONFIG_H__
#define __KLOCALECONFIG_H__


#include <kcontrol.h>
#include <qgrpbox.h>

#include "klangcombo.h"


class KLocaleConfig : public KConfigWidget
{
  Q_OBJECT

public:

  KLocaleConfig( QWidget *parent=0, const char *name=0);
  ~KLocaleConfig( );

  void loadLanguageList(KLanguageCombo *combo);

public slots:

  void loadSettings();
  void applySettings();
  void defaultSettings();
    
private:

  KLanguageCombo *combo1, *combo2, *combo3;
  QGroupBox *gbox;
  
  QStrList tags, languages;
  bool changedFlag;

private slots:

  void changed(int);
 
};


#endif

