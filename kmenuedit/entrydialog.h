// -*- C++ -*-

//
//  kmenuedit
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@mail.bonn.netsurf.de
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef entrydialog_included
#define entrydialog_included

#include <qdialog.h>
#include <qlined.h>

#include <ktabctl.h>

class QPushButton;
class QFrame;
class QCheckBox;
class QComboBox;
class QLabel;
class QListBox;

class MenuButton;

class PLineEdit : public QLineEdit
{
  Q_OBJECT;
public:
  PLineEdit(QWidget* parent=0, const char* name=0)
    : QLineEdit(parent, name) { initMetaObject(); }
  ~PLineEdit() {}
protected:
  virtual void focusOutEvent (QFocusEvent* e) 
  { emit returnPressed(); QLineEdit::focusOutEvent(e); }
};

class EntryDialog : public QDialog
{
  friend MenuButton;
  Q_OBJECT;
public:
  EntryDialog(QWidget* parent = NULL, const char* name = NULL );

public slots:
  void invoke() { exec(); }

protected slots:
  void pixnameChanged ();
  void bigPixnameChanged ();
  void umountPixnameChanged ();
  void pixButPressed ();
  void bigPixButPressed ();
  void umountPixButPressed ();
  void typeActivated( int t );
  void insertFileType();
  void removeFileType();

protected:
  QFrame             *f_app_1;
  QFrame             *f_app_2;
  QFrame             *f_app_3;
  QFrame             *f_app_4;
  QFrame             *f_app_5;

  QComboBox          *c_type;
  QLineEdit          *i_name;
  QLineEdit          *i_fname;
  PLineEdit          *i_big_pixmap;
  PLineEdit          *i_pixmap;
  QLineEdit          *i_comment;
  QPushButton        *b_big_pixmap;
  QPushButton        *b_pixmap;
  QPushButton        *b_ok;
  QPushButton        *b_cancel;
  QFrame             *f_sub_diag;

  KTabCtl            *tb_app;
  QLineEdit          *i_command;
  QLineEdit          *i_dir;
  QLineEdit          *i_term_opt;
  QCheckBox          *ch_use_term;

  QCheckBox          *cb_file;
  QCheckBox          *cb_ftp;
  QCheckBox          *cb_http;
  QCheckBox          *cb_tar;
  QCheckBox          *cb_info;
  QCheckBox          *cb_man;
  QListBox           *l_inside;
  QListBox           *l_outside;
  QPushButton        *b_ins;
  QPushButton        *b_rem;
  QLineEdit          *i_pattern;

  QLineEdit          *i_fvwm;

  QLineEdit          *i_url;

  QLineEdit          *i_device;
  QLineEdit          *i_mount;
  QLineEdit          *i_fstype;
  PLineEdit          *i_mount_pix;
  QPushButton        *b_mount_pix;
  QCheckBox          *cb_read_only;

  //QLineEdit          *i_ftype_pattern;
  //QLineEdit          *i_default_app;
};

#endif /* entrydialog_included */





