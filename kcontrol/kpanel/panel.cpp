/*
 * panel.cpp - General Settings for KPanel
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

#include "panel.h"
#include "panel.moc"
#include <qlayout.h>
#include <kconfigbase.h>

const char * locations[] = { "top", "left", "bottom", "right" , 0};
const char * taskbar_locations[] = { "hidden", "top", "bottom", "top_left", 0 };
const char * styles[] = { "tiny", "normal", "large", 0};

extern KConfigBase *config;

class MyHelpFrame : public QFrame {

    QWidget *widget;
protected:
    
    void resizeEvent(QResizeEvent *) {
	widget->resize(width(), widget->height());
	widget->move(0, (height() - widget->height())/ 2);
    }

public:
    MyHelpFrame(QWidget *parent=0, const char *name=0, WFlags f=0, bool allowLines=true) :
	QFrame (parent, name, f, allowLines) {};
    void setWidget(QWidget *_w) { widget = _w; }

};

KPanelConfig::KPanelConfig( QWidget *parent, const char* name ) 
    : KConfigWidget (parent, name), location(LTop), taskbar(TTop)
{
    layout = new QVBoxLayout(this, 10);
    
    loc_group = new QButtonGroup(klocale->translate("Location"), this);
    loc_buttons[0] = new QRadioButton( klocale->translate("Top"), loc_group );
    loc_buttons[1] = new QRadioButton( klocale->translate("Left"), loc_group );
    loc_buttons[2] = new QRadioButton( klocale->translate("Bottom"), loc_group);
    loc_buttons[location]->setChecked(true);
    connect(loc_group, SIGNAL(clicked(int)), SLOT(location_clicked(int)));
    layout->addWidget(loc_group);

    task_group = new QButtonGroup(klocale->translate("Taskbar"), this);
    task_buttons[0] = new QRadioButton( klocale->translate("Hidden"), task_group );
    task_buttons[1] = new QRadioButton( klocale->translate("Top"), task_group );
    task_buttons[2] = new QRadioButton( klocale->translate("Bottom"), task_group);
    task_buttons[3] = new QRadioButton( klocale->translate("Top/Left"), task_group);
    task_buttons[taskbar]->setChecked(true);
    connect(task_group, SIGNAL(clicked(int)), SLOT(taskbar_clicked(int)));
    layout->addWidget(task_group);

    down_frame = new QFrame(this);
    down_layout = new QHBoxLayout(down_frame, 10, 30);
    
    style_frame = new QFrame(down_frame);
    style_layout = new QHBoxLayout(style_frame, 5);

    style_label = new QLabel(style_frame);
    style_label->setText( klocale->translate("Style:"));
    style_label->setAlignment(AlignRight | AlignVCenter);
    
    style_layout->addWidget(style_label, 1, AlignTop);
    
    style_combo_frame = new MyHelpFrame(style_frame);
    style_combo = new QComboBox(style_combo_frame);
    style_combo_frame->setWidget(style_combo);

    style_combo->insertItem(klocale->translate("Tiny Style"), Tiny);
    style_combo->insertItem(klocale->translate("Normal Style"), Normal);
    style_combo->insertItem(klocale->translate("Large Style"), Large);
    style_layout->addWidget(style_combo_frame, 3, AlignTop );
    style_layout->activate();
  
    down_layout->addWidget(style_frame);

    option_frame = new QFrame(down_frame);
    option_layout = new QVBoxLayout(option_frame,0);
    options[0] = new QCheckBox( klocale->translate("Menu Tooltips"), option_frame);
    option_layout->addWidget(options[0]);
    options[1] = new QCheckBox( klocale->translate("Personal First"), option_frame);
    option_layout->addWidget(options[1]);
    options[2] = new QCheckBox( klocale->translate("Auto Hide Panel"), option_frame);
    option_layout->addWidget(options[2]);
    options[3] = new QCheckBox( klocale->translate("Auto Hide Taskbar"), option_frame);
    option_layout->addWidget(options[3]);

    option_layout->activate();
    
    down_layout->addWidget(option_frame);
    down_layout->activate();
    layout->addWidget(down_frame);

    layout->activate();
  
    loadSettings();
}

void KPanelConfig::location_clicked(int i) {
    location = static_cast<Location>(i);
}

void KPanelConfig::taskbar_clicked(int i) {
    taskbar = static_cast<Taskbar>(i);
}

KPanelConfig::~KPanelConfig( ) {
}

void KPanelConfig::resizeEvent(QResizeEvent *e) {
    KConfigWidget::resizeEvent(e);
    QRect rect;
    int i, h, w;

    rect = task_group->contentsRect();
    w = (rect.width() - 10) / 4;
    h = rect.top() + 
	(rect.height() - task_buttons[0]->sizeHint().height()) / 2;

    for (i = 0;  i < 4; i++)
	task_buttons[i]->move(w * i + 10, h);

    rect = loc_group->contentsRect();
    h = rect.top() + 
	(rect.height() - loc_buttons[0]->sizeHint().height()) / 2;
    for (i = 0; i < 3; i++)
	loc_buttons[i]->move(w * i + 10, h);

    // style_combo->resize(style_combo_frame->width(), style_combo->height());
    // style_combo->move(0, (style_combo_frame->height() - style_combo->height())/ 2);
}

void KPanelConfig::loadSettings() {
   
    config->setGroup("kpanel");
    int i = 0;
    loc_buttons[location]->setChecked(false);
    QString t = config->readEntry("Position", locations[0]);

    while (locations[i] && t != locations[i])  i++;
    if (!locations[i])
	i = 0;
    location = (Location)i;
    loc_buttons[location]->setChecked(true);
    
    task_buttons[taskbar]->setChecked(false);
    i = 0; t = config->readEntry("TaskbarPosition", taskbar_locations[0]);
    while (taskbar_locations[i] && t != taskbar_locations[i]) 
	i++;
    if (!taskbar_locations[i])
	i = 0;
    taskbar = (Taskbar)i;
    task_buttons[taskbar]->setChecked(true);

    options[0]->setChecked(config->readNumEntry("MenuToolTips")>=0);
    options[1]->setChecked(config->readEntry("PersonalFirst") == "on");
    options[2]->setChecked(config->readEntry("AutoHide") == "on");
    options[3]->setChecked(config->readEntry("AutoHideTaskbar") == "on");

    i = 0; t = config->readEntry("Style", styles[0]);
    while (styles[i] && t != styles[i]) 
	i++;
    if (!styles[i])
	style = Normal;
    else
	style = (Style)i;
    
    style_combo->setCurrentItem(style);
}

void KPanelConfig::applySettings() {
    saveSettings();
    KWM::sendKWMCommand("kpanel:restart");
}

void KPanelConfig::saveSettings() {

    config->setGroup("kpanel");
    config->writeEntry("Position", locations[location]);
    config->writeEntry("TaskbarPosition", taskbar_locations[taskbar]);

    // out of kpanel's prop.C
    config->writeEntry("MenuToolTips", options[0]->isChecked()?1000:-1);
    config->writeEntry("PersonalFirst", options[1]->isChecked()?"on":"off");
    config->writeEntry("AutoHide", options[2]->isChecked()?"on":"off");
    config->writeEntry("AutoHideTaskbar", options[3]->isChecked()?"on":"off");
   
    if (style_combo->currentItem() != style) {
	switch (style_combo->currentItem()){
	case 0: // tiny style
	    config->writeEntry("Style", "tiny");
	    config->writeEntry("BoxWidth",26);
	    config->writeEntry("BoxHeight",26);
	    config->writeEntry("Margin",0);
	    config->writeEntry("TaskbarButtonHorizontalSize",4);
	    config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
	    config->writeEntry("DesktopButtonRows",1);
	    config->writeEntry("DateFont","*-times-medium-i-normal--12-*");
	    break;
	case 1: // normal style
	    config->writeEntry("Style", "normal");
	    config->writeEntry("BoxWidth",45);
	    config->writeEntry("BoxHeight",45);
	    config->writeEntry("Margin",0);
	    config->writeEntry("TaskbarButtonHorizontalSize",4);
	    config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
	    config->writeEntry("DesktopButtonRows",2);
	    config->writeEntry("DateFont","*-times-medium-i-normal--12-*");
	    break;
	case 2: // large style
	    config->writeEntry("Style", "large");
	    config->writeEntry("BoxWidth",47);
	    config->writeEntry("BoxHeight",47);
	    config->writeEntry("Margin",4);
	    config->writeEntry("TaskbarButtonHorizontalSize",4);
	    config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--14-*");
	    config->writeEntry("DesktopButtonRows",2);
	    config->writeEntry("DateFont","*-times-bold-i-normal--12-*");
	    break;
	}
    }
    config->sync();
}
