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

//#include <qslider.h>
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
#include <kslider.h> //CT

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

/*CT 19Dec1998 - using layouts instead
protected:
    void  resizeEvent(QResizeEvent *e);
CT*/
protected slots:
    void tooltips_clicked(bool click);
    void hide_panel_clicked(bool click);
    void hide_taskbar_clicked(bool click);
    void show_hide_clicked(bool click);

private:
    QVBoxLayout *layout;
    QLabel *tips_label;
    QGroupBox *tooltips_group;
    QCheckBox *tips;
    QLabel *slider_small_label;
    QLabel *slider_large_label;
    KSlider *slider;

    QGroupBox *vis_group;
    QCheckBox *hide_panel;
    QLabel *p_dl_label;
    QLabel *p_small_label;
    QLabel *p_large_label;
    KSlider *panel_delay_slider;
    QLabel *p_spd_label;
    QLabel *p_slow_label;
    QLabel *p_fast_label;
    KSlider *panel_speed_slider;

    QCheckBox *hide_taskbar;
    QLabel *t_dl_label;
    QLabel *t_small_label;
    QLabel *t_large_label;
    KSlider *tbar_delay_slider;
    QLabel *t_spd_label;
    QLabel *t_slow_label;
    QLabel *t_fast_label;
    KSlider *tbar_speed_slider;

    QCheckBox *show_hide;
    QLabel *show_hide_label;
    QLabel *show_slow_label;
    QLabel *show_fast_label;
    KSlider *show_hide_slider;

    QGroupBox *others_group;
    QCheckBox *personal_first;
    QCheckBox *folders_first;
    QCheckBox *clock;
    QCheckBox *clockBeats;


};

#endif

