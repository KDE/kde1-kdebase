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
    layout = new QVBoxLayout(this, 3);
    
    tooltips_group = new QGroupBox(i18n("Menu Tooltips"), this);
    tips = new QCheckBox(i18n("Show &Menu Tooltips"), tooltips_group);
    connect(tips, SIGNAL(toggled(bool)), SLOT(tooltips_clicked(bool)));

    slider = new KSlider(0, 2000, 10, 0,
			 KSlider::Horizontal, tooltips_group);
    tips_label = new QLabel(slider, i18n("De&lay:"), tooltips_group);
    slider->setSteps ( 125, 125 );
    slider->setTickmarks ( static_cast<QSlider::TickSetting>(QSlider::Below) );
    slider->setTickInterval ( 250 );
    slider->setTracking( true );
    
    slider_small_label = new QLabel(i18n("Small"), tooltips_group);
    slider_large_label = new QLabel(i18n("Large"), tooltips_group);
    slider->adjustSize();
    tips->adjustSize();
    slider_small_label->adjustSize();
    tooltips_group->adjustSize();
    tooltips_group->setMinimumSize(100, slider->sizeHint().height()+
				   slider_small_label->sizeHint().height()+38);

    layout->addWidget(tooltips_group, 3);

    //CT 17Oct1998
    vis_group = new QGroupBox(i18n("Visuals"), this);

    hide_panel = new QCheckBox( i18n("Auto &Hide Panel"), 
				vis_group);
    connect(hide_panel,SIGNAL(toggled(bool)), SLOT(hide_panel_clicked(bool)));
    p_dl_label = new QLabel( i18n("Delay:"), vis_group);
    p_small_label = new QLabel ( i18n("Small"), vis_group);
    p_large_label = new QLabel ( i18n("Large"), vis_group);
    panel_delay_slider = new KSlider(1000, 10000, 1000, 6000,
			 KSlider::Horizontal, vis_group);
    panel_delay_slider->setSteps(1000,1000);

    p_spd_label = new QLabel ( i18n ("Speed:"), vis_group);
    p_slow_label = new QLabel ( i18n("Slow"), vis_group);
    p_fast_label = new QLabel ( i18n("Fast"), vis_group);
    panel_speed_slider = new KSlider(1, 20, 1, 4,
			 KSlider::Horizontal, vis_group);
    panel_speed_slider->setSteps(1,1);

    
    hide_taskbar = new QCheckBox( i18n("Auto Hide &Taskbar"), 
				  vis_group);
    connect(hide_taskbar,SIGNAL(toggled(bool)),SLOT(hide_taskbar_clicked(bool)));
    t_dl_label = new QLabel( i18n("Delay:"), vis_group);
    t_small_label = new QLabel ( i18n("Small"), vis_group);
    t_large_label = new QLabel ( i18n("Large"), vis_group);

    tbar_delay_slider = new KSlider(1000, 10000, 1000, 6000,
			 KSlider::Horizontal, vis_group);
    tbar_delay_slider->setSteps(1000,1000);

    t_spd_label = new QLabel ( i18n ("Speed:"), vis_group);
    t_slow_label = new QLabel ( i18n("Slow"), vis_group);
    t_fast_label = new QLabel ( i18n("Fast"), vis_group);
    tbar_speed_slider = new KSlider(1, 20, 1, 4,
			 KSlider::Horizontal, vis_group);
    tbar_speed_slider->setSteps(1,1);

    show_hide = new QCheckBox( i18n("Animate Show/Hide"), vis_group);
    connect(show_hide,SIGNAL(toggled(bool)),SLOT(show_hide_clicked(bool)));
    show_hide_label = new QLabel( i18n("Speed"), vis_group);
    show_slow_label = new QLabel ( i18n("Slow"), vis_group);
    show_fast_label = new QLabel ( i18n("Fast"), vis_group);
    show_hide_slider = new KSlider(10,200,19,50,
				   KSlider::Horizontal, vis_group);
    show_hide_slider->setSteps(10,10);

    panel_delay_slider->adjustSize();
    p_small_label->adjustSize();
    p_dl_label->adjustSize();
    layout->addWidget(vis_group, 2);
    vis_group->setMinimumSize(100,(panel_delay_slider->sizeHint().height() 
				   +p_small_label->sizeHint().height()
				   +p_dl_label->sizeHint().height())*3 + 60);

    others_group = new QGroupBox( i18n("Others"), this);
    personal_first = new QCheckBox( i18n("Personal &First"), 
				    others_group);
    folders_first = new QCheckBox ( i18n("F&olders First"),
				    others_group);
    clock = new QCheckBox(i18n("&Clock shows time in AM/PM format"), 
				others_group);
    personal_first->adjustSize();
    folders_first->adjustSize();
    clock->adjustSize();

    layout->addWidget(others_group, 2);
    others_group->setMinimumSize(100, 
				 personal_first->sizeHint().height() * 3 + 30);
    
    //CT
    
    layout->activate();
    
    loadSettings();
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
void KOptionsConfig::resizeEvent(QResizeEvent *e) {
    KConfigWidget::resizeEvent(e);

    QRect rect = vis_group->contentsRect();
    int h = panel_delay_slider->height();
    h += p_small_label->height();
    h += p_dl_label->height();
    int o = (rect.height() - 10 - 3*h) / 6;

    rect.setTop(rect.top() + 10);

    hide_panel->adjustSize();
    hide_panel->move(10,rect.top() + o + (h - hide_panel->height())/2);
    hide_taskbar->adjustSize();
    hide_taskbar->move(10,rect.top() + 3*o+h+(h - hide_taskbar->height())/2);
    show_hide->adjustSize();
    show_hide->move(10,rect.top() + 5*o+2*h+(h - show_hide->height())/2);

    int maxw = hide_panel->width();
    if (maxw < hide_taskbar->width()) maxw =hide_taskbar->width();
    if (maxw < show_hide->width()) maxw = show_hide->width();
    
    int xpos = maxw + 20; 
    int ypos = 10+2*o;
    int dispw = (rect.width()-xpos)/2;
    
    p_dl_label->move(xpos + (dispw-20-p_dl_label->width())/2,ypos);
    ypos += p_dl_label->height()+2;
    panel_delay_slider->setGeometry(xpos, ypos, 
				    dispw-10, panel_delay_slider->height());

    ypos += panel_delay_slider->height() -3;
    p_small_label->move(xpos,ypos);

    p_large_label->adjustSize();
    xpos += dispw -10- p_large_label->width();
    p_large_label->move(xpos,ypos);

    ypos += p_small_label->height()+o;

    xpos = maxw + 20;
    t_dl_label->adjustSize();
    t_dl_label->move(xpos+(dispw-10-t_dl_label->width())/2, ypos);
 
    ypos += t_dl_label->height()+2;
    tbar_delay_slider->adjustSize();
    tbar_delay_slider->setGeometry(xpos,ypos,
				   dispw-10, tbar_delay_slider->height());
    
    t_small_label->adjustSize();
    ypos += tbar_delay_slider->height()-3;
    t_small_label->move(xpos,ypos);

    t_large_label->adjustSize();
    t_large_label->move(xpos+dispw-10-t_large_label->width(),ypos);

    ypos += t_small_label->height()+o;

    show_hide_label->adjustSize();
    show_hide_label->move(xpos + (dispw-10-show_hide_label->width()/2),ypos);

    show_hide_slider->adjustSize();
    ypos += show_hide_label->height();
    show_hide_slider->setGeometry(xpos,ypos,dispw*2 - 10, 
				  show_hide_slider->height());

    show_slow_label->adjustSize();
    ypos += show_hide_slider->height()-3;
    show_slow_label->move(xpos,ypos);

    show_fast_label->adjustSize();
    xpos = rect.width()-10-show_fast_label->width();
    show_fast_label->move(xpos,ypos);
    
    //CT another column
    xpos = maxw + dispw + 40;
    ypos = p_dl_label->y();

    p_spd_label->adjustSize();
    p_spd_label->move(xpos+(dispw-10-p_spd_label->width())/2,ypos);

    ypos = panel_delay_slider->y();

    panel_speed_slider->adjustSize();
    panel_speed_slider->setGeometry(xpos,ypos,dispw-20,
				    panel_speed_slider->height());

    p_slow_label->adjustSize();
    ypos = p_small_label->y();
    p_slow_label->move(xpos,ypos);

    p_fast_label->adjustSize();
    xpos = rect.width()-20 - p_fast_label->width();
    p_fast_label->move(xpos,ypos);

    ypos = t_dl_label->y();
    xpos = maxw + dispw + 40;
    t_spd_label->adjustSize();
    t_spd_label->move(xpos+(dispw-10-t_spd_label->width())/2,ypos);

    ypos = tbar_delay_slider->y();

    tbar_speed_slider->adjustSize();
    tbar_speed_slider->setGeometry(xpos,ypos,dispw-20,
				   tbar_speed_slider->height());

    t_slow_label->adjustSize();
    ypos = t_small_label->y();
    t_slow_label->move(xpos,ypos);
    
    t_fast_label->adjustSize();
    xpos=rect.width()-10-t_fast_label->width();
    t_fast_label->move(xpos,ypos);
    

    rect = tooltips_group->contentsRect();
    rect.setTop(rect.top() + 5);

    ypos = 10  +(slider->height()+
		slider_small_label->height()-tips->height())/2;
    tips->setGeometry(10, ypos, tips->width(), tips->height());
    xpos = tips->width() + 20;

    tips_label->adjustSize();
    tips_label->move(xpos, 10 + (slider->height()
				     +slider_small_label->height()
				     -tips_label->height())/2 );

    xpos += tips_label->width() +10;
    slider->setGeometry(xpos, ypos, rect.width() - xpos -10, slider->height());

    ypos = 15 + slider->height();
    slider_small_label->move(xpos,ypos);

    slider_large_label->adjustSize();
    xpos = rect.width() - 10 - slider_large_label->width();
    slider_large_label->move(xpos,ypos);
    
    rect = others_group->contentsRect();
    h = clock->height();
    o = (rect.height()  - 3*h) / 4;
    rect.setTop(rect.top() + 5);

    personal_first->move(10,10+o);
    folders_first->adjustSize();
    folders_first->move(10,10+2*o+h);
    clock->adjustSize();
    clock->move(10, 10+3*o + 2*h);
}

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
  
  val = config->readNumEntry("HideShowAnimate",50);
  show_hide->setChecked( val >= 0 );
  if (val >= 0 ) show_hide_slider->setValue(val);
  show_hide_clicked(val >= 0);
  //CT

  clock->setChecked (config->readEntry("ClockAmPm", "off") == "on");
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
      config->writeEntry("AutoHideDelay",panel_delay_slider->value());
      config->writeEntry("AutoHideSpeed",panel_speed_slider->value());
    }
    else config->writeEntry("AutoHide", "off");

    if (hide_taskbar->isChecked()) {
      config->writeEntry("AutoHideTaskbar", "on");
      config->writeEntry("AutoHideTaskbarDelay",tbar_delay_slider->value());
      config->writeEntry("AutoHideTaskbarSpeed",tbar_speed_slider->value());
    }
    else config->writeEntry("AutoHideTaskbar", "off");

    config->writeEntry("HideShowAnimation", 
		       show_hide->isChecked()?show_hide_slider->value():0);

    config->writeEntry("ClockAmPm", clock->isChecked()?"off":"on");
    //CT

    config->writeEntry("MenuToolTips", tips->isChecked()?slider->value():-1);
    config->sync();
}
