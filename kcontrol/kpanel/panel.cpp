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
    layout = new QGridLayout(this, 2, 2, 10);
    int i, w = 0;

    loc_group = new QButtonGroup(i18n("Location"), this);
    loc_buttons[0] = new QRadioButton( i18n("&Top"), loc_group );
    loc_buttons[1] = new QRadioButton( i18n("&Left"), loc_group );
    loc_buttons[2] = new QRadioButton( i18n("&Bottom"), loc_group);
    loc_buttons[3] = new QRadioButton( i18n("&Right"), loc_group);

    for (i = 0; i < 4; i++) {
	loc_buttons[i]->adjustSize();
	if (w < loc_buttons[i]->width())
	    w = loc_buttons[i]->width();
    }

    loc_buttons[location]->setChecked(true);
    connect(loc_group, SIGNAL(clicked(int)), SLOT(location_clicked(int)));
    layout->addWidget(loc_group, 0, 0);

    task_group = new QButtonGroup(i18n("Taskbar"), this);
    task_buttons[0] = new QRadioButton( i18n("&Hidden"),
					task_group );
    task_buttons[1] = new QRadioButton( i18n("T&op"),
					task_group );
    task_buttons[2] = new QRadioButton( i18n("Botto&m"),
					task_group);
    task_buttons[3] = new QRadioButton( i18n("Top/Le&ft"),
					task_group);
    task_buttons[taskbar]->setChecked(true);
    for (i = 0; i < 4; i++) {
	task_buttons[i]->adjustSize();
	if (w < task_buttons[i]->width())
	    w = task_buttons[i]->width();
    }

    connect(task_group, SIGNAL(clicked(int)), SLOT(taskbar_clicked(int)));
    layout->addWidget(task_group, 0, 1);
   

    style_group = new QButtonGroup(i18n("Style"), this);

    style_buttons[0] = new QRadioButton( i18n("T&iny"),
					 style_group );
    style_buttons[1] = new QRadioButton( i18n("&Normal"),
					 style_group );
    style_buttons[2] = new QRadioButton( i18n("L&arge"),
					 style_group);
    connect(style_group, SIGNAL(clicked(int)), SLOT(style_clicked(int)));
    for (i = 0; i < 3; i++) {
	style_buttons[i]->adjustSize();
	if (w < style_buttons[i]->width())
	    w = style_buttons[i]->width();
    }

    layout->addWidget(style_group, 1, 0);

    w = w + 15;

    loc_group->setMinimumSize(w, loc_buttons[0]->sizeHint().height() * 4 + 30);
    task_group->setMinimumSize(w, task_buttons[0]->sizeHint().height() * 4 + 30);
    style_group->setMinimumSize(w, style_buttons[0]->sizeHint().height() * 3 + 30);
   
    layout->setRowStretch(0, 10);
    layout->setRowStretch(1, 10);
   
    layout->activate();
   

    loadSettings();
}

void KPanelConfig::location_clicked(int i) {
    // if I find out, that someone changed this in a C cast, just
    // because there are weired compilers out there, may just god
    // help him! :-)
    location = static_cast<Location>(i);
}

void KPanelConfig::taskbar_clicked(int i) {
    taskbar = static_cast<Taskbar>(i);
}

void KPanelConfig::style_clicked(int i) {
    style = static_cast<Style>(i);
}

KPanelConfig::~KPanelConfig( ) {
}

void KPanelConfig::resizeEvent(QResizeEvent *e) {
    KConfigWidget::resizeEvent(e);
    QRect rect;
    int i, h, o;

    rect = task_group->contentsRect();
    h = loc_buttons[0]->height();
    o = (rect.height() - 4*h) / 5;
    rect.setTop(rect.top() + 5);
    for (i = 0;  i < 4; i++)
	task_buttons[i]->move(10, 10 + (i+1)*o + i*h);

    rect = loc_group->contentsRect();
    h = loc_buttons[0]->height();
    o = (rect.height() - 4*h) / 5;
    rect.setTop(rect.top() + 5);
    for (i = 0;  i < 4; i++)
	loc_buttons[i]->move(10, 10 + (i+1)*o + i*h);
   
    rect = style_group->contentsRect();
    h = style_buttons[0]->height();
    o = (rect.height() - 3*h) / 4;
    rect.setTop(rect.top() + 5);
    for (i = 0;  i < 3; i++)
	style_buttons[i]->move(10, 10 + (i+1)*o + i*h);
   
/*   
    rect = loc_group->contentsRect();
    h = rect.top() +
	(rect.height() - loc_buttons[0]->sizeHint().height()) / 2;
    for (i = 0; i < 4; i++)
	loc_buttons[i]->move(w * i + 10, h);

    rect = style_group->contentsRect();
    h = rect.top() +
	(rect.height() - style_buttons[0]->sizeHint().height()) / 2;
    for (i = 0; i < 3; i++)
	style_buttons[i]->move(w * i + 10, h);
*/
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
    taskbar = static_cast<Taskbar>(i);
    task_buttons[taskbar]->setChecked(true);

    i = 0; t = config->readEntry("Style", styles[0]);
    while (styles[i] && t != styles[i])
	i++;
    if (!styles[i])
	style = Normal;
    else
	style = static_cast<Style>(i);

    style_buttons[style]->setChecked(true);
}

void KPanelConfig::applySettings() {
    saveSettings();
    KWM::sendKWMCommand("kpanel:restart");
}

void KPanelConfig::saveSettings() {

    config->setGroup("kpanel");
    config->writeEntry("Position", locations[location]);
    config->writeEntry("TaskbarPosition", taskbar_locations[taskbar]);

    // DF : open ~/.kderc and write icon style depending on panel style
    // so that icons become large or normal
    KConfig * cfg = new KConfig();
    cfg->setGroup( "KDE" );

    switch (style){
    case 0: // tiny style
	config->writeEntry("Style", "tiny");
	config->writeEntry("BoxWidth",26);
	config->writeEntry("BoxHeight",26);
	config->writeEntry("Margin",0);
	config->writeEntry("TaskbarButtonHorizontalSize",4);
	//config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
	config->writeEntry("DesktopButtonRows",1);
	//config->writeEntry("DateFont","*-times-medium-i-normal--12-*");
	break;
    case 1: // normal style
	config->writeEntry("Style", "normal");
	config->writeEntry("BoxWidth",45);
	config->writeEntry("BoxHeight",45);
	config->writeEntry("Margin",0);
	config->writeEntry("TaskbarButtonHorizontalSize",4);
	//config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
	config->writeEntry("DesktopButtonRows",2);
	//config->writeEntry("DateFont","*-times-medium-i-normal--12-*");

        // DF : write icon style
        cfg->writeEntry("kpanelIconStyle", "Normal", true, true /* global setting (.kderc) */);

	break;
    case 2: // large style
	config->writeEntry("Style", "large");
	config->writeEntry("BoxWidth",52);
	config->writeEntry("BoxHeight",52);
	config->writeEntry("Margin",2);
	config->writeEntry("TaskbarButtonHorizontalSize",4);
	//config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--14-*");
	config->writeEntry("DesktopButtonRows",2);
	//config->writeEntry("DateFont","*-times-bold-i-normal--12-*");

        // DF : write icon style
        cfg->writeEntry("kpanelIconStyle", "Large", true, true /* global setting (.kderc) */);

	break;
    }
    cfg->sync();
    delete cfg;
    config->sync();
}
