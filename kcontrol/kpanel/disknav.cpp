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
    QBoxLayout *lay = new QVBoxLayout(this,5);
    
    hist_group = new QGroupBox(i18n("History"), this);

    //CT 25Oct1998
    QBoxLayout *hLay = new QVBoxLayout(hist_group,10,5);
    hLay->addSpacing(10);

    QBoxLayout *btnLay = new QHBoxLayout();
    hLay->addLayout(btnLay);
    btnLay->addSpacing(20);

    QGridLayout *inLay = new QGridLayout(3,5);
    hLay->addLayout(inLay);

    inLay->setColStretch(0,0);
    inLay->setColStretch(1,1);
    inLay->setColStretch(2,0);
    inLay->setColStretch(3,1);
    inLay->setColStretch(4,0);
    inLay->addColSpacing(0,20);
    inLay->addColSpacing(2,20);
    inLay->addColSpacing(4,20);
    //CT

    edit_entries = new QLabel();
    edit_entries->setText(i18n("Edit entries: (drag a folder and create a link)"));

    edit_personal = new QPushButton(i18n("Edit &Personal..."), hist_group);
    connect(edit_personal, SIGNAL(clicked()), SLOT(edit_personal_clicked()));
    //CT 25Oct1998
    edit_personal->adjustSize();
    edit_personal->setMinimumWidth(edit_personal->width());
    edit_personal->setFixedHeight(edit_personal->height());
    btnLay->addWidget(edit_personal);
    btnLay->addSpacing(20);
    //CT

    edit_shared = new QPushButton(i18n("Edit &Shared..."), hist_group);
    connect(edit_shared, SIGNAL(clicked()), SLOT(edit_shared_clicked()));
    //CT 25Oct1998
    edit_shared->adjustSize();
    edit_shared->setMinimumWidth(edit_shared->width());
    edit_shared->setFixedHeight(edit_shared->height());
    btnLay->addWidget(edit_shared);
    btnLay->addSpacing(20);
    //CT

    max_recent_folders_size = new QLineEdit(hist_group);
    max_recent_folders_size->setMaxLength(2);
    //max_recent_folders_size->setValidator(&int_validator);
    //CT 25Oct1998
    max_recent_folders_size->adjustSize();
    max_recent_folders_size->setMinimumSize(max_recent_folders_size->size());
    inLay->addWidget(max_recent_folders_size,0,3);
    //CT

    max_recent_files_size = new QLineEdit(hist_group);
    max_recent_files_size->setMaxLength(2);
    //max_recent_files_size->setValidator(&int_validator);
    //CT 25Oct1998
    max_recent_files_size->adjustSize();
    max_recent_files_size->setMinimumSize(max_recent_files_size->size());
    inLay->addWidget(max_recent_files_size,1,3);
    //CT

    max_navigable_folder_size = new QLineEdit(hist_group);
    max_navigable_folder_size->setMaxLength(4);
    //max_navigable_folder_size->setValidator(&int_validator);
    //CT 25Oct1998
    max_navigable_folder_size->adjustSize();
    max_navigable_folder_size->setMinimumSize(max_navigable_folder_size->size());
    inLay->addWidget(max_navigable_folder_size,2,3);
    //CT

    recent_folders_size = new QLabel(max_recent_folders_size, 
                                     i18n("Max recent folder entries:"),
                                     hist_group);
    //CT 25Oct1998
    recent_folders_size->adjustSize();
    recent_folders_size->setMinimumSize(recent_folders_size->size());
    inLay->addWidget(recent_folders_size,0,1);
    //CT

    recent_files_size = new QLabel(max_recent_files_size,
                                   i18n("Max recent file entries:"),
                                   hist_group);
    //CT 25Oct1998
    recent_files_size->adjustSize();
    recent_files_size->setMinimumSize(recent_files_size->size());
    inLay->addWidget(recent_files_size,1,1);
    //CT

    navigable_folder_size = new QLabel(max_navigable_folder_size,
                                   i18n("Max files in a single folder:"),
                                   hist_group);
    //CT 25Oct1998
    navigable_folder_size->adjustSize();
    navigable_folder_size->setMinimumSize(navigable_folder_size->size());
    inLay->addWidget(navigable_folder_size,2,1);
    //CT
    
    hLay->activate();

    lay->addWidget(hist_group);

    //CT 25Oct1998
    misc_group = new QGroupBox(i18n("Options"), this);

    inLay = new QGridLayout(misc_group,5,5,10,5);

    inLay->setColStretch(0,0);
    inLay->setColStretch(1,1);
    inLay->setColStretch(2,0);
    inLay->setColStretch(3,1);
    inLay->setColStretch(4,0);
    inLay->addColSpacing(0,20);
    inLay->addColSpacing(2,20);
    inLay->addColSpacing(4,10);

    inLay->addRowSpacing(0,10);
    //CT

    show_dot_files = new QCheckBox(i18n("Show dot files"), misc_group);
    //CT 25Oct1998
    show_dot_files->adjustSize();
    show_dot_files->setMinimumSize(show_dot_files->size());
    inLay->addWidget(show_dot_files,1,1);
    //CT

    ignore_case = new QCheckBox(i18n("Ignore case when sorting"), misc_group);
    //CT 25Oct1998
    ignore_case->adjustSize();
    ignore_case->setMinimumSize(ignore_case->size());
    inLay->addWidget(ignore_case,1,3);
    //CT

    show_shared_section = new QCheckBox(i18n("Show Shared section"), misc_group);
    //CT 25Oct1998
    show_shared_section->adjustSize();
    show_shared_section->setMinimumSize(show_shared_section->size());
    inLay->addWidget(show_shared_section,2,1);
    //CT

    show_personal_section = new QCheckBox(i18n("Show Personal section"), misc_group);
    //CT 25Oct1998
    show_personal_section->adjustSize();
    show_personal_section->setMinimumSize(show_personal_section->size());
    inLay->addWidget(show_personal_section,2,3);
    //CT

    show_recent_section = new QCheckBox(i18n("Show Recent section"), misc_group);
    //CT 25Oct1998
    show_recent_section->adjustSize();
    show_recent_section->setMinimumSize(show_recent_section->size());
    inLay->addWidget(show_recent_section,3,1);
    //CT

    show_option_entry = new QCheckBox(i18n("Show Option entry"), misc_group);
    //CT 25Oct1998
    show_option_entry->adjustSize();
    show_option_entry->setMinimumSize(show_option_entry->size());
    inLay->addWidget(show_option_entry,3,3);
    //CT

    terminal = new QLineEdit(misc_group);
    terminal->setMaxLength(80);
    //CT 25Oct1998
    terminal->adjustSize();
    terminal->setMinimumSize(terminal->size());
    inLay->addWidget(terminal,4,3);
    //CT

    terminal_label = new QLabel(terminal, 
                                i18n("Terminal application:"),
                                misc_group);
    //CT 25Oct1998
    terminal_label->adjustSize();
    terminal_label->setMinimumSize(terminal_label->size());
    inLay->addWidget(terminal_label,4,1);
    //CT

    /*CT 25Oct1998
    edit_shared->adjustSize();
    edit_personal->adjustSize();
    misc_group->adjustSize();
    recent_files_size->adjustSize();
    recent_folders_size->adjustSize();
    navigable_folder_size->adjustSize();
    show_dot_files->adjustSize();
    ignore_case->adjustSize();

    show_shared_section->adjustSize();
    show_personal_section->adjustSize();
    show_recent_section->adjustSize();
    show_option_entry->adjustSize();

    terminal_label->adjustSize();
    */

    inLay->activate();

    lay->addWidget(misc_group);

    /*CT    misc_group->setMinimumSize(370, 270);*/

    lay->activate();
    
    loadSettings();
}

void KDiskNavConfig::edit_shared_clicked() 
{
  KShellProcess proc;
  proc << "kfmclient folder " + KApplication::kde_datadir() + "/kdisknav";
  proc.start(KShellProcess::DontCare);
}

void KDiskNavConfig::edit_personal_clicked() 
{
  KShellProcess proc;
  proc << "kfmclient folder " + KApplication::localkdedir() + "/share/apps/kdisknav";
  proc.start(KShellProcess::DontCare);
}

KDiskNavConfig::~KDiskNavConfig( ) {
}

/*CT 25Oct1998 - useless (harmful!) when using layouts
void KDiskNavConfig::resizeEvent(QResizeEvent *e) {
    KConfigWidget::resizeEvent(e);
    
    int x = 10;
    edit_personal->move(x, 20);
    edit_shared->move(edit_personal->x() + edit_personal->width() + 4, 20);

    int y = edit_personal->y() + edit_personal->height() + 8;

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
    show_shared_section->move(x, y);
    show_personal_section->move(show_shared_section->x() + show_shared_section->width() + 8, y);

    y = show_shared_section->y() + show_shared_section->height() + 8;

    show_recent_section->move(x, y);
    show_option_entry->move(show_recent_section->x() + show_recent_section->width() + 8, y);

    y = show_recent_section->y() + show_recent_section->height() + 8;

    terminal_label->move(x, y);
    terminal->setGeometry(terminal_label->x() + terminal_label->width() +8, y,
                          150, 25);
}
*/
void KDiskNavConfig::loadSettings() {

  config->setGroup("kdisknav");

  max_recent_folders_size->setText(config->readEntry("MaxRecentFoldersEntries", "4"));
  max_recent_files_size->setText(config->readEntry("MaxRecentFilesEntries", "4"));
  max_navigable_folder_size->setText(config->readEntry("MaxNavigableFolderEntries", "200"));
  show_dot_files->setChecked(config->readEntry("ShowDotFiles", "off") == "on");
  ignore_case->setChecked(config->readEntry("IgnoreCase", "off") == "on");

  show_shared_section->setChecked(config->readEntry("ShowGlobalSection", "on") == "on");
  show_personal_section->setChecked(config->readEntry("ShowLocalSection", "on") == "on");
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

    if (config->hasKey("ShowGlobalSection") || !show_shared_section->isChecked())
      config->writeEntry("ShowGlobalSection",
                         (show_shared_section->isChecked() ? "on" : "off"));

    if (config->hasKey("ShowLocalSection") || !show_personal_section->isChecked())
      config->writeEntry("ShowLocalSection",
                         (show_personal_section->isChecked() ? "on" : "off"));

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


