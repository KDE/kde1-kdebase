/*
 * options.h
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

#ifndef __KCONTROL_OPTIONS_H__
#define __KCONTROL_OPTIONS_H__

#include <qslider.h>
#include <qbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qcombo.h> 
#include <qchkbox.h> 
#include <qlcdnum.h> 

#include <kcontrol.h>
#include <kwm.h>

class QButtonGroup;

class KOptionsConfig : public KConfigWidget
{
  Q_OBJECT

public:
    KOptionsConfig( QWidget *parent=0, const char* name=0 );
    ~KOptionsConfig( );
    virtual void loadSettings();
    virtual void saveSettings();
    virtual void applySettings();

protected:
    void  resizeEvent(QResizeEvent *e);

protected slots:
    void tooltips_clicked(bool click);

private:
    QVBoxLayout *layout;
    QLabel *tips_label;
    QGroupBox *tooltips_group;
    QCheckBox *tips;
    QLCDNumber *slider_value;
    QLabel *slider_label;
    QSlider *slider;

    QCheckBox *personal_first;
    QCheckBox *hide_panel;
    QCheckBox *hide_taskbar;
    QGroupBox *others_group;

    QRadioButton *clock[2];
    QButtonGroup *clock_group;

};

#endif

