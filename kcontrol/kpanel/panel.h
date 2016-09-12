/*
 * panel.h
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

#ifndef __KCONTROL_KPANEL_H__
#define __KCONTROL_KPANEL_H__

#include <kcontrol.h>
#include <kwm.h>

#include <qbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qcombo.h>
#include <qchkbox.h>

class QButtonGroup;
class MyHelpFrame;

class KPanelConfig : public KConfigWidget
{
  Q_OBJECT

public:
    KPanelConfig( QWidget *parent=0, const char* name=0 );
    ~KPanelConfig( );
    virtual void loadSettings();
    virtual void saveSettings();
    virtual void applySettings();

protected:
    void  resizeEvent(QResizeEvent *e);

protected slots:
    void location_clicked(int);
    void taskbar_clicked(int);
    void style_clicked(int);

private:
    QGridLayout *layout;
   
    enum Location {LTop, Left, Button, Right} location;
    QRadioButton *loc_buttons[4];
    QButtonGroup *loc_group;

    enum Taskbar {Hidden, TTop, Bottom, Top_Left } taskbar;
    QRadioButton *task_buttons[4];
    QButtonGroup *task_group;

    enum Style { Tiny, Normal, Large } style;
    QRadioButton *style_buttons[3];
    QButtonGroup *style_group;
};

#endif

