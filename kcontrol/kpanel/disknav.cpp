/*
 * disknav.cpp
 *
 * Copyright (C) 1998 Pietro Iglio (iglio@fub.it)
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
#include <qdir.h>

#include "disknav.h"
#include "disknav.moc"
#include <qlayout.h>
#include <kconfigbase.h>
#include <kprocess.h>

extern KConfigBase *config;

KDiskNavConfig::KDiskNavConfig( QWidget *parent, const char* name ) 
  : KConfigWidget (parent, name) //, int_validator(0, 20, 0)
{
    layout = new QVBoxLayout(this, 3);
    
    misc_group = new QGroupBox(i18n("Options"), this);

    edit_entries = new QLabel();
    edit_entries->setText("Edit entries: (drag a folder and create a link)");

    edit_global = new QPushButton(i18n("Edit &Global..."), misc_group);
    connect(edit_global, SIGNAL(clicked()), SLOT(edit_global_clicked()));

    edit_local = new QPushButton(i18n("Edit &Local..."), misc_group);
    connect(edit_local, SIGNAL(clicked()), SLOT(edit_local_clicked()));

    max_recent_folders_size = new QLineEdit(misc_group);
    max_recent_folders_size->setMaxLength(2);
    //max_recent_folders_size->setValidator(&int_validator);

    max_recent_files_size = new QLineEdit(misc_group);
    max_recent_files_size->setMaxLength(2);
    //max_recent_files_size->setValidator(&int_validator);

    max_navigable_folder_size = new QLineEdit(misc_group);
    max_navigable_folder_size->setMaxLength(4);
    //max_navigable_folder_size->setValidator(&int_validator);

    recent_folders_size = new QLabel(max_recent_folders_size, 
                                     "Max recent folder entries:",
                                     misc_group);
    recent_files_size = new QLabel(max_recent_files_size,
                                   "Max recent file entries:",
                                   misc_group);

    navigable_folder_size = new QLabel(max_navigable_folder_size,
                                   "Max files in a single folder:",
                                   misc_group);


    show_dot_files = new QCheckBox("Show dot files", misc_group);
    ignore_case = new QCheckBox("Ignore case when sorting", misc_group);

    show_global_section = new QCheckBox("Show Global section", misc_group);
    show_local_section = new QCheckBox("Show Local section", misc_group);
    show_recent_section = new QCheckBox("Show Recent section", misc_group);
    show_option_entry = new QCheckBox("Show Option entry", misc_group);

    terminal = new QLineEdit(misc_group);
    terminal->setMaxLength(80);

    terminal_label = new QLabel(terminal, 
                                "Terminal application:",
                                misc_group);


    edit_global->adjustSize();
    edit_local->adjustSize();
    misc_group->adjustSize();
    recent_files_size->adjustSize();
    recent_folders_size->adjustSize();
    navigable_folder_size->adjustSize();
    show_dot_files->adjustSize();
    ignore_case->adjustSize();

    show_global_section->adjustSize();
    show_local_section->adjustSize();
    show_recent_section->adjustSize();
    show_option_entry->adjustSize();

    terminal_label->adjustSize();


    layout->addWidget(misc_group, 2);

    misc_group->setMinimumSize(370, 270);

    layout->activate();
    
    loadSettings();
}

void KDiskNavConfig::edit_global_clicked() 
{
  KShellProcess proc;
  proc << "kfmclient folder /opt/kde/share/apps/kdisknav";
  proc.start(KShellProcess::DontCare);

}

void KDiskNavConfig::edit_local_clicked() 
{
  KShellProcess proc;
  proc << "kfmclient folder " + QDir::homeDirPath() + "/.kde/share/apps/kdisknav";
  proc.start(KShellProcess::DontCare);
}

KDiskNavConfig::~KDiskNavConfig( ) {
}

void KDiskNavConfig::resizeEvent(QResizeEvent *e) {
    KConfigWidget::resizeEvent(e);
    
    int x = 10;
    edit_local->move(x, 20);
    edit_global->move(edit_local->x() + edit_local->width() + 4, 20);

    int y = edit_local->y() + edit_local->height() + 8;

    recent_folders_size->move(x, y + 4);
    max_recent_folders_size->setGeometry(recent_folders_size->x() +
                                         navigable_folder_size->width() + 8,
                                         y, 30, 25);

    y = max_recent_folders_size->y() + max_recent_folders_size->height() + 8;
    recent_files_size->move(x, y + 4);

    max_recent_files_size->setGeometry(max_recent_folders_size->x(),
                                       y, 30, 25);

    y = max_recent_files_size->y() + max_recent_files_size->height() + 8;
    navigable_folder_size->move(x, y + 4);

    max_navigable_folder_size->setGeometry(max_recent_folders_size->x(), y,
                                    40, 25);

    y = max_navigable_folder_size->y() + max_navigable_folder_size->height() + 8;
    show_dot_files->move(x, y);
    ignore_case->move(show_dot_files->x() + show_dot_files->width() + 8,
                      y);

    y = show_dot_files->y() + show_dot_files->height() + 8;
    show_global_section->move(x, y);
    show_local_section->move(show_global_section->x() + show_global_section->width() + 8, y);

    y = show_global_section->y() + show_global_section->height() + 8;

    show_recent_section->move(x, y);
    show_option_entry->move(show_recent_section->x() + show_recent_section->width() + 8, y);

    y = show_recent_section->y() + show_recent_section->height() + 8;

    terminal_label->move(x, y);
    terminal->setGeometry(terminal_label->x() + terminal_label->width() +8, y,
                          150, 25);
}

void KDiskNavConfig::loadSettings() {

  config->setGroup("kdisknav");

  max_recent_folders_size->setText(config->readEntry("MaxRecentFoldersEntries", "4"));
  max_recent_files_size->setText(config->readEntry("MaxRecentFilesEntries", "4"));
  max_navigable_folder_size->setText(config->readEntry("MaxNavigableFolderEntries", "200"));
  show_dot_files->setChecked(config->readEntry("ShowDotFiles", "off") == "on");
  ignore_case->setChecked(config->readEntry("IgnoreCase", "off") == "on");

  show_global_section->setChecked(config->readEntry("ShowGlobalSection", "on") == "on");
  show_local_section->setChecked(config->readEntry("ShowLocalSection", "on") == "on");
  show_recent_section->setChecked(config->readEntry("ShowRecentSection", "on") == "on");
  show_option_entry->setChecked(config->readEntry("ShowOptionEntry", "on") == "on");

  terminal->setText(config->readEntry("Terminal", "kvt"));
}

void KDiskNavConfig::applySettings() {
    saveSettings();
    KWM::sendKWMCommand("kpanel:restart");
}

void KDiskNavConfig::saveSettings() {

    config->setGroup("kdisknav");

    config->writeEntry("MaxRecentFoldersEntries",
                       max_recent_folders_size->text());
    config->writeEntry("MaxRecentFilesEntries",
                       max_recent_files_size->text());
    config->writeEntry("MaxNavigableFolderEntries",
                       max_navigable_folder_size->text());

    if (config->hasKey("ShowDotFiles") || show_dot_files->isChecked())
      config->writeEntry("ShowDotFiles",
                         (show_dot_files->isChecked() ? "on" : "off"));

    if (config->hasKey("IgnoreCase") || show_dot_files->isChecked())
      config->writeEntry("IgnoreCase",
                         (ignore_case->isChecked() ? "on" : "off"));

    if (config->hasKey("ShowGlobalSection") || !show_global_section->isChecked())
      config->writeEntry("ShowGlobalSection",
                         (show_global_section->isChecked() ? "on" : "off"));

    if (config->hasKey("ShowLocalSection") || !show_local_section->isChecked())
      config->writeEntry("ShowLocalSection",
                         (show_local_section->isChecked() ? "on" : "off"));

    if (config->hasKey("ShowRecentSection") || !show_recent_section->isChecked())
      config->writeEntry("ShowRecentSection",
                         (show_recent_section->isChecked() ? "on" : "off"));

    if (config->hasKey("ShowOptionEntry") || !show_option_entry->isChecked())
      config->writeEntry("ShowOptionEntry",
                         (show_option_entry->isChecked() ? "on" : "off"));

    if (config->hasKey("Terminal") || strcmp(show_recent_section->text(), "kvt"))
      config->writeEntry("Terminal", terminal->text());

    config->sync();
}


