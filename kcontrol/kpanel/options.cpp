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
    
    tooltips_group = new QGroupBox(klocale->translate("Menu Tooltips"), this);
    tips = new QCheckBox(i18n("Show Menu Tooltips"), tooltips_group);
    connect(tips, SIGNAL(toggled(bool)), SLOT(tooltips_clicked(bool)));

    slider = new KSlider(0, 2000, 10, 0,
			 KSlider::Horizontal, tooltips_group);
    tips_label = new QLabel(slider, i18n("&Delay:"), tooltips_group);
    slider->setSteps ( 125, 125 );
    slider->setTickmarks ( static_cast<QSlider::TickSetting>(QSlider::Below) );
    slider->setTickInterval ( 250 );
    slider->setTracking( true );
    
    slider_value = new QLCDNumber(4, tooltips_group);
    slider_value->setFrameStyle( QFrame::NoFrame );        
    connect(slider, SIGNAL(valueChanged(int)), slider_value,
	    SLOT(display(int)));
    slider_value->display(slider->value());
    
    slider_label = new QLabel(i18n("ms"), tooltips_group);
    slider->adjustSize();
    slider_value->adjustSize();
    tips->adjustSize();
    tooltips_group->adjustSize();
    tooltips_group->setMinimumSize(100, slider->sizeHint().height() * 2+ 
				   tips->sizeHint().height() + 
				   slider_value->sizeHint().height() +
				   38);

    layout->addWidget(tooltips_group, 3);
    
    others_group = new QGroupBox(i18n("Others"), this);
    personal_first = new QCheckBox( i18n("&Personal First"), 
				    others_group);
    hide_panel = new QCheckBox( i18n("Auto &Hide Panel"), 
				others_group);
    hide_taskbar = new QCheckBox( i18n("Auto Hide &Taskbar"), 
				  others_group);
    personal_first->adjustSize();
    layout->addWidget(others_group, 2);
    
    others_group->setMinimumSize(100, 
				 personal_first->sizeHint().height() * 3 + 30);
    
    clock_group = new QButtonGroup(i18n("Clock"), this);
    clock_group->setExclusive(true);
    clock[0] = new QRadioButton(i18n("&24h"), 
				clock_group);
    clock[1] = new QRadioButton(i18n("&12h AM/PM"), 
				clock_group);
    clock[0]->adjustSize();
    clock_group->setMinimumSize(100, clock[0]->sizeHint().height() * 2 + 30);
    
    layout->addWidget(clock_group, 2);

    layout->activate();
    
    loadSettings();
}

void KOptionsConfig::tooltips_clicked(bool click) 
{
    slider->setEnabled(click);
}

KOptionsConfig::~KOptionsConfig( ) {
}

void KOptionsConfig::resizeEvent(QResizeEvent *e) {
    KConfigWidget::resizeEvent(e);

    QRect rect = others_group->contentsRect();
    int h = personal_first->height();
    int o = (rect.height() - 10 - 3*h) / 6;

    rect.setTop(rect.top() + 10);

    personal_first->move(10,rect.top() + o);
    hide_panel->adjustSize();
    hide_panel->move(10,rect.top() + 3*o+h);
    hide_taskbar->adjustSize();
    hide_taskbar->move(10,rect.top() + 5*o+2*h);

    rect = tooltips_group->contentsRect();
    h = (rect.height() - 20 - tips->height()) / 3;
    tips_label->adjustSize();
    o = tips_label->width();
    tips_label->setGeometry(10, h + 10 + tips->height(), o , 2 * h);
    o += 20;
    
    slider->setGeometry(o + 10, 2 * h + 10 + tips->height(), 
			rect.width() - 20 - o, h);

    int r = (rect.width() - o - slider_value->width() - 
	     slider_label->width() -20) / 2;
    slider_value->setGeometry(r + o + 10, h + 10 + tips->height() ,
			      slider_value->width(), h);
    slider_label->setGeometry(r + o + 20 + slider_value->width() , 
			      h + 10 + tips->height(), 
			      slider_label->width(), h);
    tips->setGeometry(10, 18, tips->width(), tips->height());
    
    rect = clock_group->contentsRect();
    h = clock[0]->height();
    o = (rect.height() - 5 - 2*h) / 4;
    rect.setTop(rect.top() + 5);

    clock[0]->adjustSize();
    clock[1]->adjustSize();
    clock[0]->move(10, rect.top() + o);
    clock[1]->move(10, rect.top() + h + 3*o);
}

void KOptionsConfig::loadSettings() {
   
    config->setGroup("kpanel");

    personal_first->setChecked(config->readEntry("PersonalFirst") == "on");
    hide_panel->setChecked(config->readEntry("AutoHide") == "on");
    hide_taskbar->setChecked(config->readEntry("AutoHideTaskbar") == "on");
    bool ampm = config->readEntry("ClockAmPm", "off") == "off";
    clock[0]->setChecked(ampm);
    clock[1]->setChecked(!ampm);
    int del = config->readNumEntry("MenuToolTips", 1000);
    slider->setValue(del);
    tips->setChecked(del >= 0);
    tooltips_clicked(del >= 0);
}

void KOptionsConfig::applySettings() {
    saveSettings();
    KWM::sendKWMCommand("kpanel:restart");
}

void KOptionsConfig::saveSettings() {

    config->setGroup("kpanel");

    // out of kpanel's prop.C
    config->writeEntry("PersonalFirst", personal_first->isChecked()?"on":"off");
    config->writeEntry("AutoHide", hide_panel->isChecked()?"on":"off");
    config->writeEntry("AutoHideTaskbar", hide_taskbar->isChecked()?"on":"off");
    config->writeEntry("ClockAmPm", clock[1]->isChecked()?"on":"off");
    config->writeEntry("MenuToolTips", tips->isChecked()?slider->value():-1);
    config->sync();
}
