/*
 * deskops.cpp - General Settings for KPanel
 *
 * Copyright (C) 1997 Stephan Kulow (coolo@kde.org)
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

#include "desktops.h"
#include "desktops.moc"
#include <ksimpleconfig.h>

extern KConfigBase *config;

KDesktopsConfig::KDesktopsConfig( QWidget *parent, const char* name ) 
    : KConfigWidget(parent, name)
{
    layout = new QVBoxLayout(this, 5);
    layout->addStretch(2);
    fields_layout = new QGridLayout(4, 4);
    layout->addLayout(fields_layout);
    
    layout->addStretch(2);

    slider_layout = new QGridLayout(2, 5, 30);
    layout->addLayout(slider_layout, 3);

    layout->addStretch(2);

    int number_of_desktops = KWM::numberOfDesktops();

    QString tmp;
    for (int i = 0; i < 8; i++) {
	fields[i] = new QLineEdit(this);
	fields[i]->setText(KWM::getDesktopName(i+1));
	fields[i]->setEnabled(i<number_of_desktops);
	fields[i]->setMaximumSize(300, fields[i]->sizeHint().height());
	fields[i]->setMinimumSize(60, fields[i]->sizeHint().height());
	tmp.setNum(i+1);
	tmp.prepend("&");
	labels[i] = new QLabel(fields[i], tmp, this);
	labels[i]->setAlignment( ShowPrefix | AlignCenter);
	fields_layout->addWidget(fields[i], i % 4, i / 4 * 2 + 1);
	fields_layout->addWidget(labels[i], i % 4, i / 4 * 2 , AlignCenter );
    }
    fields_layout->setColStretch(0, 1);
    fields_layout->setColStretch(2, 1);
    fields_layout->setColStretch(1, 6);
    fields_layout->setColStretch(3, 6);
    
    fields_layout->setRowStretch(0, 1);
    fields_layout->setRowStretch(1, 1);
    fields_layout->setRowStretch(2, 1);
    fields_layout->setRowStretch(3, 1);

    visible = new QSlider(1, 4, 1, number_of_desktops/2, QSlider::Horizontal, this);
    visible->setMaximumSize(1000, visible->sizeHint().height());
    visible_label = new QLabel(visible, i18n("&Visible"), this);
    visible_label->setAlignment(ShowPrefix | AlignRight | AlignVCenter);
    visible_label->adjustSize();
    visible_label->setMinimumSize(visible_label->sizeHint());
    slider_layout->addMultiCellWidget(visible, 0, 0, 2, 3);
    slider_layout->addWidget(visible_label, 0, 1);
    connect(visible, SIGNAL(valueChanged(int)), this, SLOT(visible_changed(int)));

    width = new QSlider(1, 6, 1, width_value, QSlider::Horizontal, this);
    width_label = new QLabel(width, i18n("&Width"), this);
    width_label->setAlignment(ShowPrefix | AlignRight | AlignVCenter);
    width_label->adjustSize();
    width_label->setMinimumSize(width_label->sizeHint());
    width->adjustSize();
    width->setMaximumSize(1000, width->sizeHint().height());
    slider_layout->addMultiCellWidget(width, 1, 1, 2, 3);
    slider_layout->addWidget(width_label, 1, 1);

    loadSettings();
    layout->activate();
}

KDesktopsConfig::~KDesktopsConfig( ) {
}

void KDesktopsConfig::visible_changed(int value) {
    for (int i = 0; i < 8; i++)
	fields[i]->setEnabled(i<value*2);
}

void KDesktopsConfig::loadSettings() {
    int number_of_desktops = KWM::numberOfDesktops();
    
    for (int i = 0; i < 8; i++) {
	fields[i]->setText(KWM::getDesktopName(i+1));
	fields[i]->setEnabled(i<number_of_desktops);
    }
    visible->setValue(number_of_desktops/2);
    width_value =  config->readNumEntry("DesktopButtonHorizontalSize");
    width->setValue(width_value);
}

bool KDesktopsConfig::justSave() {
    width_value = width->value();
    for (int i = 0; i < 8; i++)
	KWM::setDesktopName(i+1, fields[i]->text());
    config->writeEntry("DesktopButtonHorizontalSize", width_value);
    config->sync();
    if (KWM::numberOfDesktops() != visible->value() * 2) {
	KWM::setNumberOfDesktops(visible->value() * 2);
	return true;
    } else
	return false; 
}

void KDesktopsConfig::saveSettings() {
    debug("save");
    justSave();
    config->sync();
}

void KDesktopsConfig::applySettings() {
    debug("apply");
    saveSettings();
}



