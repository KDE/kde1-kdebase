/*
 * options.cpp - Extended Settings for KPanel
 *
 * Copyright (C) 1997 Stephan Kulow (coolo@kde.org)
 *               1997 Matthias Ettrich (ettrich@kde.org)
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

#include "options.h"
#include "options.moc"
#include <qlayout.h>
#include <kconfigbase.h>
#include <kslider.h>

extern KConfigBase *config;

KOptionsConfig::KOptionsConfig( QWidget *parent, const char* name )
    : KConfigWidget (parent, name)
{
    layout = new QVBoxLayout(this, 5);

    tooltips_group = new QGroupBox(i18n("Menu Tooltips"), this);

    //CT 19Dec1998 using layouts
    QGridLayout *t_lay = new QGridLayout(tooltips_group, 4,8,1,1);
    t_lay->addRowSpacing(0,15);
    t_lay->addRowSpacing(3,1);
    t_lay->addColSpacing(0,5);
    t_lay->addColSpacing(2,5);
    t_lay->addColSpacing(7,5);

    t_lay->setRowStretch(0,0);
    t_lay->setRowStretch(1,0);
    t_lay->setRowStretch(2,0);
    t_lay->setRowStretch(3,0);
    t_lay->setColStretch(0,0);
    t_lay->setColStretch(1,0);
    t_lay->setColStretch(2,0);
    t_lay->setColStretch(3,0);
    t_lay->setColStretch(4,0);
    t_lay->setColStretch(5,1);
    t_lay->setColStretch(6,0);
    t_lay->setColStretch(7,0);
    //CT

    tips = new QCheckBox(i18n("Show &Menu Tooltips"), tooltips_group);
    connect(tips, SIGNAL(toggled(bool)), SLOT(tooltips_clicked(bool)));

    //CT 19Dec1998 using layouts
    tips->adjustSize();
    tips->setMinimumSize(tips->size());
    t_lay->addWidget(tips,1,1);
    //CT

    slider = new KSlider(0, 2000, 10, 0,
			 KSlider::Horizontal, tooltips_group);
    slider->setSteps ( 125, 125 );
    slider->setTickmarks ( static_cast<QSlider::TickSetting>(QSlider::Below) );
    slider->setTickInterval ( 250 );
    slider->setTracking( true );
    //CT 19Dec1998 using layouts
    slider->adjustSize();
    slider->setMinimumSize(slider->size());
    slider->setMinimumHeight(slider->height()-20);
    t_lay->addMultiCellWidget(slider,1,1,4,6);
    //CT

    tips_label = new QLabel(slider, i18n("De&lay:"), tooltips_group);
    //CT 19Dec1998 using layouts
    tips_label->adjustSize();
    tips_label->setMinimumSize(tips_label->size());
    t_lay->addWidget(tips_label,1,3);
    //CT

    slider_small_label = new QLabel(i18n("Small"), tooltips_group);
    //CT 19Dec1998 using layouts
    slider_small_label->adjustSize();
    slider_small_label->setMinimumSize(slider_small_label->size());
    t_lay->addWidget(slider_small_label,2,4);
    //CT

    slider_large_label = new QLabel(i18n("Large"), tooltips_group);
    //CT 19Dec1998 using layouts
    slider_large_label->adjustSize();
    slider_large_label->setMinimumSize(slider_large_label->size());
    t_lay->addWidget(slider_large_label,2,6);
    //CT

    t_lay->activate();

    layout->addWidget(tooltips_group);

    //CT 17Oct1998
    vis_group = new QGroupBox(i18n("Visuals"), this);

    QVBoxLayout *v_lay = new QVBoxLayout(vis_group,5);
    v_lay->addSpacing(5);

    //CT 19Dec1998 using layouts
    t_lay = new QGridLayout(5,11,1);
    v_lay->addLayout(t_lay);

    t_lay->addRowSpacing(0,5);
    t_lay->addRowSpacing(4,5);
    t_lay->addColSpacing(0,5);
    t_lay->addColSpacing(2,5);
    t_lay->addColSpacing(7,5);
    t_lay->addColSpacing(10,5);

    t_lay->setRowStretch(0,0);
    t_lay->setRowStretch(1,0);
    t_lay->setRowStretch(2,0);
    t_lay->setRowStretch(3,0);
    t_lay->setRowStretch(4,0);
    t_lay->setColStretch(0,0);
    t_lay->setColStretch(1,0);
    t_lay->setColStretch(2,0);
    t_lay->setColStretch(3,0);
    t_lay->setColStretch(4,1);
    t_lay->setColStretch(5,0);
    t_lay->setColStretch(6,0);
    t_lay->setColStretch(7,0);
    t_lay->setColStretch(8,1);
    t_lay->setColStretch(9,0);
    t_lay->setColStretch(10,0);
    //CT

    hide_panel = new QCheckBox( i18n("Auto &Hide Panel"),
				vis_group);
    connect(hide_panel,SIGNAL(toggled(bool)), SLOT(hide_panel_clicked(bool)));
    //CT 19Dec1998 using layouts
    hide_panel->adjustSize();
    hide_panel->setMinimumSize(hide_panel->size());
    t_lay->addWidget(hide_panel,2,1);
    //CT

    p_dl_label = new QLabel( i18n("Delay:"), vis_group);
    //CT 19Dec1998 using layouts
    p_dl_label->adjustSize();
    p_dl_label->setMinimumSize(p_dl_label->size());
    t_lay->addMultiCellWidget(p_dl_label,1,1,3,5);
    //CT

    p_small_label = new QLabel ( i18n("Small"), vis_group);
    //CT 19Dec1998 using layouts
    p_small_label->adjustSize();
    p_small_label->setMinimumSize(p_small_label->size());
    t_lay->addWidget(p_small_label,3,3);
    //CT

    p_large_label = new QLabel ( i18n("Large"), vis_group);
    //CT 19Dec1998 using layouts
    p_large_label->adjustSize();
    p_large_label->setMinimumSize(p_large_label->size());
    t_lay->addWidget(p_large_label,3,5);
    //CT

    panel_delay_slider = new KSlider(0, 10000, 1000, 6000,
			 KSlider::Horizontal, vis_group);
    panel_delay_slider->setSteps(1000,1000);
    //CT 19Dec1998 using layouts
    panel_delay_slider->adjustSize();
    panel_delay_slider->setMinimumSize(panel_delay_slider->size());
    panel_delay_slider->setMinimumHeight(panel_delay_slider->height()-20);
    t_lay->addMultiCellWidget(panel_delay_slider,2,2,3,5);
    //CT

    p_spd_label = new QLabel ( i18n ("Speed:"), vis_group);
    //CT 19Dec1998 using layouts
    p_spd_label->adjustSize();
    p_spd_label->setMinimumSize(p_spd_label->size());
    t_lay->addMultiCellWidget(p_spd_label,1,1,7,9);
    //CT

    p_slow_label = new QLabel ( i18n("Slow"), vis_group);
    //CT 19Dec1998 using layouts
    p_slow_label->adjustSize();
    p_slow_label->setMinimumSize(p_slow_label->size());
    t_lay->addWidget(p_slow_label,3,7);
    //CT

    p_fast_label = new QLabel ( i18n("Fast"), vis_group);
    //CT 19Dec1998 using layouts
    p_fast_label->adjustSize();
    p_fast_label->setMinimumSize(p_fast_label->size());
    t_lay->addWidget(p_fast_label,3,9);
    //CT

    panel_speed_slider = new KSlider(1, 20, 1, 4,
			 KSlider::Horizontal, vis_group);
    panel_speed_slider->setSteps(1,1);
    //CT 19Dec1998 using layouts
    panel_speed_slider->adjustSize();
    panel_speed_slider->setMinimumSize(panel_speed_slider->size());
    panel_speed_slider->setMinimumHeight(panel_speed_slider->height()-20);
    t_lay->addMultiCellWidget(panel_speed_slider,2,2,7,9);
    //CT

 
    //CT 19Dec1998 using layouts
    t_lay = new QGridLayout(5,11,1);
    v_lay->addLayout(t_lay);

    t_lay->addRowSpacing(0,5);
    t_lay->addRowSpacing(4,5);
    t_lay->addColSpacing(0,5);
    t_lay->addColSpacing(2,5);
    t_lay->addColSpacing(7,5);
    t_lay->addColSpacing(10,5);

    t_lay->setRowStretch(0,0);
    t_lay->setRowStretch(1,0);
    t_lay->setRowStretch(2,0);
    t_lay->setRowStretch(3,0);
    t_lay->setRowStretch(4,0);
    t_lay->setColStretch(0,0);
    t_lay->setColStretch(1,0);
    t_lay->setColStretch(2,0);
    t_lay->setColStretch(3,0);
    t_lay->setColStretch(4,1);
    t_lay->setColStretch(5,0);
    t_lay->setColStretch(6,0);
    t_lay->setColStretch(7,0);
    t_lay->setColStretch(8,1);
    t_lay->setColStretch(9,0);
    t_lay->setColStretch(10,0);
    //CT


    hide_taskbar = new QCheckBox( i18n("Auto Hide &Taskbar"),
				  vis_group);
    connect(hide_taskbar,SIGNAL(toggled(bool)),SLOT(hide_taskbar_clicked(bool)));
    //CT 19Dec1998 using layouts
    hide_taskbar->adjustSize();
    hide_taskbar->setMinimumSize(hide_taskbar->size());
    t_lay->addWidget(hide_taskbar,2,1);
    //CT

    t_dl_label = new QLabel( i18n("Delay:"), vis_group);
    //CT 19Dec1998 using layouts
    t_dl_label->adjustSize();
    t_dl_label->setMinimumSize(t_dl_label->size());
    t_lay->addMultiCellWidget(t_dl_label,1,1,3,5);
    //CT

    t_small_label = new QLabel ( i18n("Small"), vis_group);
    //CT 19Dec1998 using layouts
    t_small_label->adjustSize();
    t_small_label->setMinimumSize(t_small_label->size());
    t_lay->addWidget(t_small_label,3,3);
    //CT

    t_large_label = new QLabel ( i18n("Large"), vis_group);
    //CT 19Dec1998 using layouts
    t_large_label->adjustSize();
    t_large_label->setMinimumSize(t_large_label->size());
    t_lay->addWidget(t_large_label,3,5);
    //CT

    tbar_delay_slider = new KSlider(0, 10000, 1000, 6000,
			 KSlider::Horizontal, vis_group);
    tbar_delay_slider->setSteps(1000,1000);
    //CT 19Dec1998 using layouts
    tbar_delay_slider->adjustSize();
    tbar_delay_slider->setMinimumSize(tbar_delay_slider->size());
    tbar_delay_slider->setMinimumHeight(tbar_delay_slider->height()-20);
    t_lay->addMultiCellWidget(tbar_delay_slider,2,2,3,5);
    //CT

    t_spd_label = new QLabel ( i18n ("Speed:"), vis_group);
    //CT 19Dec1998 using layouts
    t_spd_label->adjustSize();
    t_spd_label->setMinimumSize(t_spd_label->size());
    t_lay->addMultiCellWidget(t_spd_label,1,1,7,9);
    //CT

    t_slow_label = new QLabel ( i18n("Slow"), vis_group);
    //CT 19Dec1998 using layouts
    t_slow_label->adjustSize();
    t_slow_label->setMinimumSize(t_slow_label->size());
    t_lay->addWidget(t_slow_label,3,7);
    //CT

    t_fast_label = new QLabel ( i18n("Fast"), vis_group);
    //CT 19Dec1998 using layouts
    t_fast_label->adjustSize();
    t_fast_label->setMinimumSize(t_fast_label->size());
    t_lay->addWidget(t_fast_label,3,9);
    //CT

    tbar_speed_slider = new KSlider(1, 20, 1, 4,
			 KSlider::Horizontal, vis_group);
    tbar_speed_slider->setSteps(1,1);
    //CT 19Dec1998 using layouts
    tbar_speed_slider->adjustSize();
    tbar_speed_slider->setMinimumSize(tbar_speed_slider->size());
    tbar_speed_slider->setMinimumHeight(tbar_speed_slider->height()-20);
    t_lay->addMultiCellWidget(tbar_speed_slider,2,2,7,9);
    //CT

    //CT 19Dec1998 using layouts
    t_lay = new QGridLayout(5,7,1);
    v_lay->addLayout(t_lay);

    t_lay->addRowSpacing(0,5);
    t_lay->addRowSpacing(4,5);
    t_lay->addColSpacing(0,5);
    t_lay->addColSpacing(2,5);
    t_lay->addColSpacing(6,5);

    t_lay->setRowStretch(0,0);
    t_lay->setRowStretch(1,0);
    t_lay->setRowStretch(2,0);
    t_lay->setRowStretch(3,0);
    t_lay->setRowStretch(4,0);
    t_lay->setColStretch(0,0);
    t_lay->setColStretch(1,0);
    t_lay->setColStretch(2,0);
    t_lay->setColStretch(3,0);
    t_lay->setColStretch(4,1);
    t_lay->setColStretch(5,0);
    t_lay->setColStretch(6,0);
    //CT

    show_hide = new QCheckBox( i18n("Animate Show/Hide"), vis_group);
    connect(show_hide,SIGNAL(toggled(bool)),SLOT(show_hide_clicked(bool)));
    //CT 19Dec1998 using layouts
    show_hide->adjustSize();
    show_hide->setMinimumSize(show_hide->size());
    t_lay->addWidget(show_hide,2,1);
    //CT

    show_hide_label = new QLabel( i18n("Speed:"), vis_group);
    //CT 19Dec1998 using layouts
    show_hide_label->adjustSize();
    show_hide_label->setMinimumSize(show_hide_label->size());
    t_lay->addMultiCellWidget(show_hide_label,1,1,3,5);
    //CT

    show_slow_label = new QLabel ( i18n("Slow"), vis_group);
    //CT 19Dec1998 using layouts
    show_slow_label->adjustSize();
    show_slow_label->setMinimumSize(show_slow_label->size());
    t_lay->addWidget(show_slow_label,3,3);
    //CT

    show_fast_label = new QLabel ( i18n("Fast"), vis_group);
    //CT 19Dec1998 using layouts
    show_fast_label->adjustSize();
    show_fast_label->setMinimumSize(show_fast_label->size());
    t_lay->addWidget(show_fast_label,3,5);
    //CT

    show_hide_slider = new KSlider(10,200,19,50,
				   KSlider::Horizontal, vis_group);
    show_hide_slider->setSteps(10,10);
    //CT 19Dec1998 using layouts
    show_hide_slider->adjustSize();
    show_hide_slider->setMinimumSize(show_hide_slider->size());
    show_hide_slider->setMinimumHeight(show_hide_slider->height()-20);
    t_lay->addMultiCellWidget(show_hide_slider,2,2,3,5);
    //CT

    //CT try to align the checkboxes somehow
    int max  = hide_panel->width();
    int temp = hide_taskbar->width();
    max      = temp>max?temp:max;
    temp     = show_hide->width();
    max      = temp>max?temp:max;
    hide_panel->setMinimumWidth(max);
    hide_taskbar->setMinimumWidth(max);
    show_hide->setMinimumWidth(max);
    //CT

    v_lay->activate();

    layout->addWidget(vis_group);

    others_group = new QGroupBox( i18n("Others"), this);

    //CT 19Dec1998 using layouts
    t_lay = new QGridLayout(others_group,9,3,1,1);
    t_lay->addRowSpacing(0,15);
    t_lay->addRowSpacing(8,5);
    t_lay->addColSpacing(0,5);
    t_lay->addColSpacing(2,5);
    t_lay->setColStretch(0,0);
    t_lay->setColStretch(1,1);
    t_lay->setColStretch(2,0);
    //CT

    personal_first = new QCheckBox( i18n("Personal Menu Entries &First"),
				    others_group);
    //CT 19Dec1998 using layouts
    personal_first->adjustSize();
    personal_first->setMinimumSize(personal_first->size());
    t_lay->addWidget(personal_first,1,1);
    //CT

    folders_first = new QCheckBox ( i18n("Menu F&olders First"),
				    others_group);
    //CT 19Dec1998 using layouts
    folders_first->adjustSize();
    folders_first->setMinimumSize(folders_first->size());
    t_lay->addWidget(folders_first,3,1);
    //CT

    clock = new QCheckBox(i18n("&Clock shows time in AM/PM format"),
				others_group);
    //CT 19Dec1998 using layouts
    clock->adjustSize();
    clock->setMinimumSize(clock->size());
    t_lay->addWidget(clock,5,1);
    //CT

    clockBeats = new QCheckBox( i18n("Clock shows time in &Internet beats"), others_group );
    //CT 19Dec1998 using layouts
    clockBeats->adjustSize();
    clockBeats->setMinimumSize(clockBeats->size());
    t_lay->addWidget(clockBeats,7,1);

    t_lay->activate();
    //CT

    layout->addWidget(others_group);

    loadSettings();
    layout->activate();

}

void KOptionsConfig::tooltips_clicked(bool click)
{
  tips_label->setEnabled(click);
  slider_small_label->setEnabled(click);
  slider_large_label->setEnabled(click);
  slider->setEnabled(click);
}

//CT 17Oct1998
void KOptionsConfig::hide_panel_clicked(bool click)
{
  p_dl_label->setEnabled(click);
  p_small_label->setEnabled(click);
  p_large_label->setEnabled(click);
  panel_delay_slider->setEnabled(click);
  p_spd_label->setEnabled(click);
  p_slow_label->setEnabled(click);
  p_fast_label->setEnabled(click);
  panel_speed_slider->setEnabled(click);
}

void KOptionsConfig::hide_taskbar_clicked(bool click)
{
  t_dl_label->setEnabled(click);
  t_small_label->setEnabled(click);
  t_large_label->setEnabled(click);
  tbar_delay_slider->setEnabled(click);
  t_spd_label->setEnabled(click);
  t_slow_label->setEnabled(click);
  t_fast_label->setEnabled(click);
  tbar_speed_slider->setEnabled(click);
}

void KOptionsConfig::show_hide_clicked(bool click)
{
  show_hide_label->setEnabled(click);
  show_slow_label->setEnabled(click);
  show_fast_label->setEnabled(click);
  show_hide_slider->setEnabled(click);
}
//CT

KOptionsConfig::~KOptionsConfig( ) {
}

//CT 17Oct1998 -- rewritten
//CT 19Dec1998 I HATE applying layout management
//CT this is out! void KOptionsConfig::resizeEvent(QResizeEvent *e) {

void KOptionsConfig::loadSettings() {

  int val;
  config->setGroup("kpanel");

  personal_first->setChecked(config->readEntry("PersonalFirst") == "on");
  //CT 17Oct1998
  folders_first->setChecked(config->readEntry("FoldersFirst") == "on");
  //CT
  hide_panel->setChecked(config->readEntry("AutoHide") == "on");
  //CT 17Oct1998
  if (hide_panel->isChecked()) {
    val = config->readNumEntry("AutoHideDelay",6000);
    panel_delay_slider->setValue(val);
    val = config->readNumEntry("AutoHideSpeed",4);
    panel_speed_slider->setValue(val);
  }
  hide_panel_clicked(hide_panel->isChecked());
  //CT

  hide_taskbar->setChecked(config->readEntry("AutoHideTaskbar") == "on");
  //CT 17Oct1998
  if (hide_taskbar->isChecked()) {
    val = config->readNumEntry("AutoHideTaskbarDelay",6000);
    tbar_delay_slider->setValue(val);
    val = config->readNumEntry("AutoHideTaskbarSpeed",4);
    tbar_speed_slider->setValue(val);
  }
  hide_taskbar_clicked(hide_taskbar->isChecked());

  val = config->readNumEntry("HideShowAnimation", 50);
  if ( val < 0 )   val = 0;
  show_hide->setChecked( val > 0 );
  show_hide_slider->setValue(val);
  show_hide_clicked(val > 0);
  //CT

  clock->setChecked (config->readEntry("ClockAmPm", "off") == "on");
  clockBeats->setChecked( config->readEntry("ClockBeats", "off") == "on");
  val = config->readNumEntry("MenuToolTips", 1000);
  slider->setValue(val);
  tips->setChecked(val >= 0);
  tooltips_clicked(val >= 0);
}

void KOptionsConfig::applySettings() {
    saveSettings();
    KWM::sendKWMCommand("kpanel:restart");
}

void KOptionsConfig::saveSettings() {

    config->setGroup("kpanel");

    // out of kpanel's prop.C
    config->writeEntry("PersonalFirst", personal_first->isChecked()?"on":"off");
    //CT 17Oct1998
    config->writeEntry("FoldersFirst", folders_first->isChecked()?"on":"off");
    if (hide_panel->isChecked()) {
      config->writeEntry("AutoHide", "on");
      config->writeEntry("AutoHideDelay",
			 panel_delay_slider->value()<10?10:
			 panel_delay_slider->value());
      config->writeEntry("AutoHideSpeed",panel_speed_slider->value());
    }
    else config->writeEntry("AutoHide", "off");

    if (hide_taskbar->isChecked()) {
      config->writeEntry("AutoHideTaskbar", "on");
      config->writeEntry("AutoHideTaskbarDelay",
			 tbar_delay_slider->value()<10?10:
			 tbar_delay_slider->value());
      config->writeEntry("AutoHideTaskbarSpeed",tbar_speed_slider->value());
    }
    else config->writeEntry("AutoHideTaskbar", "off");

    config->writeEntry("HideShowAnimation",
		       show_hide->isChecked()?show_hide_slider->value():0);

    config->writeEntry("ClockAmPm", clock->isChecked()?"on":"off");
    config->writeEntry("ClockBeats", clockBeats->isChecked()?"on":"off");
    //CT

    config->writeEntry("MenuToolTips", tips->isChecked()?slider->value():-1);
    config->sync();
}
