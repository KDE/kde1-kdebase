/*
 * disknav.h
 *
 * Copyright (c) 1997 Pietro Iglio
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

#ifndef __KCONTROL_DISKNAV_H__
#define __KCONTROL_DISKNAV_H__

#include <qbutton.h>
#include <qlabel.h>
#include <qbttngrp.h>
#include <qcombo.h> 
#include <qchkbox.h> 
#include <qlcdnum.h> 
#include <qlined.h> 
#include <qvalidator.h> 

#include <kcontrol.h>
#include <kwm.h>

class QButtonGroup;

class KDiskNavConfig : public KConfigWidget
{
  Q_OBJECT

public:
    KDiskNavConfig( QWidget *parent=0, const char* name=0 );
    ~KDiskNavConfig( );
    virtual void loadSettings();
    virtual void saveSettings();
    virtual void applySettings();

    //CT protected:
    //CT    void  resizeEvent(QResizeEvent *e);

protected slots:
    void edit_global_clicked();
    void edit_local_clicked();

private:
    QLabel *edit_entries;
    QPushButton *edit_global;
    QPushButton *edit_local;
    QGroupBox *hist_group, *misc_group;

    QLabel *recent_folders_size;
    QLabel *recent_files_size;
    QLabel *navigable_folder_size;
    QLabel *terminal_label;

    QLineEdit *max_recent_folders_size;
    QLineEdit *max_recent_files_size;
    QLineEdit *max_navigable_folder_size;

    QCheckBox *show_dot_files;
    QCheckBox *ignore_case;

    QCheckBox *show_global_section;
    QCheckBox *show_local_section;
    QCheckBox *show_recent_section;
    QCheckBox *show_option_entry;
    QLineEdit *terminal;

    //QIntValidator int_validator;
};

#endif

