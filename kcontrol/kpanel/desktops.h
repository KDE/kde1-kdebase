/*
 * desktops.h
 *
 * Copyright (c) 1997 Stephan Kulow
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

#ifndef __KCONTROL_DESKTOPS_H__
#define __KCONTROL_DESKTOPS_H__

#include <kcontrol.h>
#include <kwm.h>

#include <qbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qcombo.h> 
#include <qchkbox.h> 
#include <kslider.h>
#include <qlined.h>

class QButtonGroup;

class KDesktopsConfig : public KConfigWidget
{
  Q_OBJECT

public:
    KDesktopsConfig( QWidget *parent=0, const char* name=0 );
    ~KDesktopsConfig( );
    virtual void loadSettings();
    virtual void saveSettings();
    virtual void applySettings();

protected slots:
    void visible_changed(int value);

private:
    QVBoxLayout *layout;

    QGridLayout *fields_layout;
    QLineEdit *fields[8];
    QFrame *field_frame;
    QLabel *labels[8];
    
    QGridLayout *slider_layout;
    QSlider *visible;
    QSlider *width;
    QLabel  *visible_label;
    QLabel  *width_label;
    int width_value;
    
};

#endif

