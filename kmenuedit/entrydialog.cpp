//
//  kmenuedit
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@home.imv.de or chris@kde.org
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

#include <qcombo.h>
#include <qchkbox.h>
#include <qframe.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qstrlist.h>
#include <qlayout.h>

#include <kiconloader.h>
#include <kiconloaderdialog.h>

//#include "pmenu.h"
#include "entrydialog.h"
#include "entrydialog.moc"

extern KIconLoader *global_pix_loader;
extern KIconLoaderDialog *global_pix_sel;
extern KIconLoaderDialog *global_pix_sel2;
extern QStrList *global_file_types;

EntryDialog::EntryDialog (QWidget* parent, const char* name)
  :QDialog( 0, name, FALSE, WStyle_Customize | WStyle_NormalBorder )
{
  initMetaObject();

  int widg_height;

  QVBoxLayout *main_box = new QVBoxLayout(this, 6);
  QGridLayout *top_box = new QGridLayout(6, 3);
  QHBoxLayout *button_box = new QHBoxLayout();

  main_box->addLayout(top_box, 0);

  // create main dialog
  c_type = new QComboBox( this, "ComboBox_1" );
  c_type->insertItem( klocale->translate("Separator") );
  c_type->insertItem( klocale->translate("Submenu") );
  c_type->insertItem( klocale->translate("Application") );
  c_type->insertItem( klocale->translate("Swallow") );
  c_type->insertItem( klocale->translate("Link") );
  c_type->insertItem( klocale->translate("Device") );
  c_type->setFixedSize(c_type->sizeHint().width()+4, c_type->sizeHint().height());

  widg_height = c_type->sizeHint().height();

  QLabel* tmpQLabel;
  tmpQLabel = new QLabel( this, "Label_1" );
  tmpQLabel->setText( klocale->translate("Type:") );
  tmpQLabel->setFixedWidth(tmpQLabel->sizeHint().width());
  
  top_box->addWidget(tmpQLabel, 0, 0, AlignRight);
  top_box->addWidget(c_type, 0, 1, AlignLeft);

  tmpQLabel = new QLabel( this );
  tmpQLabel->setText( klocale->translate("File Name:") );
  tmpQLabel->setFixedWidth(tmpQLabel->sizeHint().width());
  
  i_fname = new QLineEdit( this );
  i_fname->setGeometry( 72, 40, 208, 24 );
  i_fname->setText( "" );
  i_fname->setMaxLength( 32767 );
  i_fname->setEchoMode( QLineEdit::Normal );
  i_fname->setFrame( TRUE );
  i_fname->setFixedSize( 208, widg_height);

  top_box->addWidget(tmpQLabel, 1, 0, AlignRight);
  top_box->addWidget(i_fname, 1, 1, AlignLeft);

  tmpQLabel = new QLabel( this, "Label_2" );
  tmpQLabel->setText( klocale->translate("Name:") );
  tmpQLabel->setFixedWidth(tmpQLabel->sizeHint().width());
  
  i_name = new QLineEdit( this, "LineEdit_1" );
  i_name->setGeometry( 72, 72, 208, 24 );
  i_name->setText( "" );
  i_name->setMaxLength( 32767 );
  i_name->setEchoMode( QLineEdit::Normal );
  i_name->setFrame( TRUE );
  i_name->setFixedSize( 208, widg_height);

  top_box->addWidget(tmpQLabel, 2, 0, AlignRight);
  top_box->addWidget(i_name, 2, 1, AlignLeft);
  
  tmpQLabel = new QLabel( this, "Label_3" );
  tmpQLabel->setText( klocale->translate("Icon:") );
  tmpQLabel->setFixedWidth(tmpQLabel->sizeHint().width());

  i_big_pixmap = new PLineEdit( this, "LineEdit_3" );
  i_big_pixmap->setGeometry( 72, 104, 208, 24 );
  i_big_pixmap->setText( "" );
  i_big_pixmap->setMaxLength( 32767 );
  i_big_pixmap->setEchoMode( QLineEdit::Normal );
  i_big_pixmap->setFrame( TRUE );
  i_big_pixmap->setFixedSize( 208, widg_height);
  
  b_big_pixmap = new QPushButton( this, "PushButton_4" );
  b_big_pixmap->setFixedSize( 54, 54 );
  b_big_pixmap->setText( "" );

  top_box->addWidget(tmpQLabel, 3, 0, AlignRight);
  top_box->addWidget(i_big_pixmap, 3, 1, AlignLeft);
  top_box->addMultiCellWidget(b_big_pixmap, 2, 3, 2, 2, AlignVCenter|AlignLeft);
  
  i_pixmap = new PLineEdit( this, "LineEdit_2" );
  i_pixmap->setGeometry( 72, 136, 208, 24 );
  i_pixmap->setText( "" );
  i_pixmap->setMaxLength( 32767 );
  i_pixmap->setEchoMode( QLineEdit::Normal );
  i_pixmap->setFrame( TRUE );
  i_pixmap->setFixedSize( 208, widg_height);
  
  b_pixmap = new QPushButton( this, "PushButton_3" );
  b_pixmap->setFixedSize(24, 24);
  b_pixmap->setText( "" );
  
  tmpQLabel = new QLabel( this, "Label_4" );
  tmpQLabel->setText( klocale->translate("Mini Icon:") );
  tmpQLabel->setFixedWidth(tmpQLabel->sizeHint().width());

  top_box->addWidget(tmpQLabel, 4, 0, AlignRight);
  top_box->addWidget(i_pixmap, 4, 1, AlignLeft);
  top_box->addWidget(b_pixmap, 4, 2, AlignLeft);
  
  tmpQLabel = new QLabel( this, "Label_5" );
  tmpQLabel->setText( klocale->translate("Comment:") );
  tmpQLabel->setFixedWidth(tmpQLabel->sizeHint().width());
  
  i_comment = new QLineEdit( this, "LineEdit_4" );
  i_comment->setGeometry( 72, 168, 208, 24 );
  i_comment->setText( "" );
  i_comment->setMaxLength( 32767 );
  i_comment->setEchoMode( QLineEdit::Normal );
  i_comment->setFrame( TRUE );
  i_comment->setFixedSize( 208, widg_height);

  top_box->addWidget(tmpQLabel, 5, 0, AlignRight);
  top_box->addWidget(i_comment, 5, 1, AlignLeft);

  f_sub_diag = new QFrame( this, "Frame_1" );
  f_sub_diag->setMinimumSize(336, 216);

  b_ok = new QPushButton( this, "PushButton_1" );
  b_ok->setText( klocale->translate("OK") );
  b_cancel = new QPushButton( this, "PushButton_2" );
  b_cancel->setText( klocale->translate("Cancel") );

  QSize ok_size = b_ok->sizeHint();
  QSize min_size = ok_size.expandedTo(b_cancel->sizeHint());
  b_ok->setMinimumSize(min_size);
  b_cancel->setMinimumSize(min_size);

  main_box->addWidget(f_sub_diag, 1);
  main_box->addLayout(button_box, 0);

  button_box->addSpacing(6);
  button_box->addStretch(1);
  button_box->addWidget(b_ok, 0);
  button_box->addStretch(2);
  button_box->addWidget(b_cancel, 0);
  button_box->addStretch(1);
  button_box->addSpacing(6);

  // end main dialog
  // create application dialog

  f_sub_layout = new QVBoxLayout(f_sub_diag);
  tb_app = new KTabCtl(f_sub_diag);
  f_sub_layout->addWidget(tb_app);
  f_sub_layout->activate();

  f_app_1 = new QFrame(f_sub_diag);
  tb_app->addTab(f_app_1, klocale->translate("Execute"));
  f_app_2 = new QFrame(f_sub_diag);
  tb_app->addTab(f_app_2, klocale->translate("Application"));
  f_app_3 = new QFrame(f_sub_diag);
  tb_app->addTab(f_app_3, klocale->translate("Swallow"));
  f_app_4 = new QFrame(f_sub_diag);
  tb_app->addTab(f_app_4, klocale->translate("Link"));
  f_app_5 = new QFrame(f_sub_diag);
  tb_app->addTab(f_app_5, klocale->translate("Device"));

  f_app_1->setMinimumSize(336, 216);
  f_app_2->setMinimumSize(336, 216);
  f_app_3->setMinimumSize(336, 216);
  f_app_4->setMinimumSize(336, 216);
  f_app_5->setMinimumSize(336, 216);

  // create application 1st dialog from 'entry2.dlg'
  i_command = new QLineEdit( f_app_1, "LineEdit_5" );
  i_command->setGeometry( 24, 32, 264, 24 );
  i_command->setText( "" );
  i_command->setMaxLength( 32767 );
  i_command->setEchoMode( QLineEdit::Normal );
  i_command->setFrame( TRUE );
  
  tmpQLabel = new QLabel( f_app_1, "Label_6" );
  tmpQLabel->setGeometry( 16, 8, i_command->width(), 24 );
  tmpQLabel->setText( klocale->translate("Execute:") );
  tmpQLabel->setAlignment( 289 );
  tmpQLabel->setMargin( -1 );
  
  tmpQLabel = new QLabel( f_app_1, "Label_7" );
  tmpQLabel->setGeometry( 16, 56, i_command->width(), 24 );
  tmpQLabel->setText( klocale->translate("Working Directory:") );
  tmpQLabel->setAlignment( 289 );
  tmpQLabel->setMargin( -1 );
  
  i_dir = new QLineEdit( f_app_1, "LineEdit_6" );
  i_dir->setGeometry( 24, 80, 264, 24 );
  i_dir->setText( "" );
  i_dir->setMaxLength( 32767 );
  i_dir->setEchoMode( QLineEdit::Normal );
  i_dir->setFrame( TRUE );
  
  ch_use_term = new QCheckBox( f_app_1, "CheckBox_1" );
  ch_use_term->setGeometry( 16, 112, i_command->width()-8, 16 );
  ch_use_term->setText( klocale->translate("Run in terminal") );
  
  tmpQLabel = new QLabel( f_app_1, "Label_8" );
  tmpQLabel->setGeometry( 24, 136, i_command->width(), 16 );
  tmpQLabel->setText( klocale->translate("Terminal Options:") );
  tmpQLabel->setAlignment( 289 );
  tmpQLabel->setMargin( -1 );
  
  i_term_opt = new QLineEdit( f_app_1, "LineEdit_7" );
  i_term_opt->setGeometry( 32, 152, 256, 24 );
  i_term_opt->setText( "" );
  i_term_opt->setMaxLength( 32767 );
  i_term_opt->setEchoMode( QLineEdit::Normal );
  i_term_opt->setFrame( TRUE );
  // end 'entry2.dlg'
  // ---
  // create application 2nd dialog from 'entry3.dlg'
  cb_file = new QCheckBox( f_app_2, "CheckBox_2" );
  cb_file->setGeometry( 16, 8, 56, 24 );
  cb_file->setText( "FILE" );
  
  cb_ftp = new QCheckBox( f_app_2, "CheckBox_3" );
  cb_ftp->setGeometry( 72, 8, 56, 24 );
  cb_ftp->setText( "FTP" );
  
  cb_http = new QCheckBox( f_app_2, "CheckBox_4" );
  cb_http->setGeometry( 128, 8, 56, 24 );
  cb_http->setText( "HTTP" );
  
  cb_tar = new QCheckBox( f_app_2, "CheckBox_5" );
  cb_tar->setGeometry( 16, 32, 56, 24 );
  cb_tar->setText( "TAR" );
  
  cb_info = new QCheckBox( f_app_2, "CheckBox_6" );
  cb_info->setGeometry( 72, 32, 56, 24 );
  cb_info->setText( "INFO" );
  
  cb_man = new QCheckBox( f_app_2, "CheckBox_7" );
  cb_man->setGeometry( 128, 32, 56, 24 );
  cb_man->setText( "MAN" );
  
  l_inside = new QListBox( f_app_2, "ListBox_1" );
  l_inside->setGeometry( 8, 56, 128, 120 );
  l_inside->setFrameStyle( 51 );
  l_inside->setLineWidth( 2 );
  
  b_ins = new QPushButton( f_app_2, "PushButton_1" );
  b_ins->setGeometry( 144, 88, 48, 32 );
  b_ins->setText( "<-" );
  
  b_rem = new QPushButton( f_app_2, "PushButton_2" );
  b_rem->setGeometry( 144, 120, 48, 32 );
  b_rem->setText( "->" );
  
  l_outside = new QListBox( f_app_2, "ListBox_2" );
  l_outside->setGeometry( 200, 56, 128, 120 );
  l_outside->setFrameStyle( 51 );
  l_outside->setLineWidth( 2 );
  l_outside->insertStrList( global_file_types );
  
  i_pattern = new QLineEdit( f_app_2, "LineEdit_4" );
  i_pattern->setGeometry( 192, 24, 136, 24 );
  i_pattern->setText( "" );
  i_pattern->setMaxLength( 32767 );
  i_pattern->setEchoMode( QLineEdit::Normal );
  i_pattern->setFrame( TRUE );
  
  tmpQLabel = new QLabel( f_app_2, "Label_4" );
  tmpQLabel->setGeometry( 192, 8, i_pattern->width(), 16 );
  tmpQLabel->setText( klocale->translate("Binary Pattern:") );
  tmpQLabel->setAlignment( 289 );
  tmpQLabel->setMargin( -1 );
  // end 'entry3.dlg'
  // ---
  // create application 3rd dialog from 'entry6.dlg'; modified for swallow entry
  i_swallow_exec = new QLineEdit( f_app_3, "LineEdit_8" );
  i_swallow_exec->setGeometry( 24, 36, 296, 24 );
  i_swallow_exec->setText( "" );
  i_swallow_exec->setMaxLength( 32767 );
  i_swallow_exec->setEchoMode( QLineEdit::Normal );
  i_swallow_exec->setFrame( TRUE );

  tmpQLabel = new QLabel( f_app_3, "Label_8" );
  tmpQLabel->setGeometry( 16, 12, i_swallow_exec->width(), 24 );
  tmpQLabel->setText( klocale->translate("Application to swallow:") );
  tmpQLabel->setAlignment( 289 );
  tmpQLabel->setMargin( -1 );

  tmpQLabel = new QLabel( f_app_3, "" );
  tmpQLabel->setGeometry( 16, 68, i_swallow_exec->width(), 24 );
  tmpQLabel->setText( klocale->translate("Title of application:") );
  tmpQLabel->setAlignment( 289 );
  tmpQLabel->setMargin( -1 );

  tmpQLabel = new QLabel( f_app_3, "" );
  tmpQLabel->setGeometry( 16, 124, i_swallow_exec->width(), 24 );
  tmpQLabel->setText( klocale->translate("Application to execute on button press:") );
  tmpQLabel->setAlignment( 289 );
  tmpQLabel->setMargin( -1 );
  
  i_swallow_title = new QLineEdit( f_app_3, "" );
  i_swallow_title->setGeometry( 24, 92, 296, 24 );
  i_swallow_title->setText( "" );
  i_swallow_title->setMaxLength( 32767 );
  i_swallow_title->setEchoMode( QLineEdit::Normal );
  i_swallow_title->setFrame( TRUE );

  i_sexec = new QLineEdit( f_app_3, "" );
  i_sexec->setGeometry( 24, 148, 296, 24 );
  i_sexec->setText( "" );
  i_sexec->setMaxLength( 32767 );
  i_sexec->setEchoMode( QLineEdit::Normal );
  i_sexec->setFrame( TRUE );

  // end 'entry6.dlg'
  // ---
  // create application 4th dialog from 'entry7.dlg'
  i_url = new QLineEdit( f_app_4, "LineEdit_9" );
  i_url->setGeometry( 24, 40, 296, 24 );
  i_url->setText( "" );
  i_url->setMaxLength( 32767 );
  i_url->setEchoMode( QLineEdit::Normal );
  i_url->setFrame( TRUE );

  tmpQLabel = new QLabel( f_app_4, "Label_10" );
  tmpQLabel->setGeometry( 16, 16, i_url->width(), 24 );
  tmpQLabel->setText( klocale->translate("URL:") );
  tmpQLabel->setAlignment( 289 );
  tmpQLabel->setMargin( -1 );
  
  // end 'entry7.dlg'
  // ---
  // create application 5th dialog from 'entry4.dlg'
  QGridLayout *f5_layout = new QGridLayout(f_app_5, 7, 3, 6);
  f5_layout->addRowSpacing(0, 8);

  i_device = new QLineEdit( f_app_5, "LineEdit_5" );
  i_device->setFixedSize(176, widg_height);
  i_device->setText( "" );
  i_device->setMaxLength( 32767 );
  i_device->setEchoMode( QLineEdit::Normal );
  i_device->setFrame( TRUE );
  f5_layout->addWidget(i_device, 1, 1, AlignLeft);
  
  i_mount = new QLineEdit( f_app_5, "LineEdit_6" );
  i_mount->setFixedSize(176, widg_height);
  i_mount->setText( "" );
  i_mount->setMaxLength( 32767 );
  i_mount->setEchoMode( QLineEdit::Normal );
  i_mount->setFrame( TRUE );
  f5_layout->addWidget(i_mount, 2, 1, AlignLeft);
  
  i_fstype = new QLineEdit( f_app_5, "LineEdit_7" );
  i_fstype->setFixedSize(176, widg_height);
  i_fstype->setText( "" );
  i_fstype->setMaxLength( 32767 );
  i_fstype->setEchoMode( QLineEdit::Normal );
  i_fstype->setFrame( TRUE );
  f5_layout->addWidget(i_fstype, 3, 1, AlignLeft);
  
  i_mount_pix = new PLineEdit( f_app_5, "LineEdit_8" );
  i_mount_pix->setFixedSize(176, widg_height);
  i_mount_pix->setText( "" );
  i_mount_pix->setMaxLength( 32767 );
  i_mount_pix->setEchoMode( QLineEdit::Normal );
  i_mount_pix->setFrame( TRUE );
  f5_layout->addWidget(i_mount_pix, 4, 1, AlignLeft);
  
  b_mount_pix = new QPushButton( f_app_5, "PushButton_3" );
  b_mount_pix->setFixedSize( 54, 54 );
  b_mount_pix->setText( "" );
  f5_layout->addMultiCellWidget(b_mount_pix, 4, 5, 2, 2, AlignVCenter|AlignLeft);

  cb_read_only = new QCheckBox( f_app_5, "CheckBox_8" );
  cb_read_only->setText( klocale->translate("Read Only") );
  cb_read_only->setFixedSize(cb_read_only->sizeHint());
  f5_layout->addWidget(cb_read_only, 5, 1, AlignLeft);
  
  tmpQLabel = new QLabel( f_app_5, "Label_5" );
  tmpQLabel->setText( klocale->translate("Device:") );
  tmpQLabel->setFixedWidth(tmpQLabel->sizeHint().width());
  f5_layout->addWidget(tmpQLabel, 1, 0, AlignRight);
  
  tmpQLabel = new QLabel( f_app_5, "Label_6" );
  tmpQLabel->setText( klocale->translate("Mount Point:") );
  tmpQLabel->setFixedWidth(tmpQLabel->sizeHint().width());
  f5_layout->addWidget(tmpQLabel, 2, 0, AlignRight);
  
  tmpQLabel = new QLabel( f_app_5, "Label_7" );
  tmpQLabel->setText( klocale->translate("FS Type:") );
  tmpQLabel->setFixedWidth(tmpQLabel->sizeHint().width());
  f5_layout->addWidget(tmpQLabel, 3, 0, AlignRight);
  
  tmpQLabel = new QLabel( f_app_5, "Label_8" );
  tmpQLabel->setText( klocale->translate("Unmount Icon:") );
  tmpQLabel->setFixedWidth(tmpQLabel->sizeHint().width());
  f5_layout->addWidget(tmpQLabel, 4, 0, AlignRight);

  f5_layout->addRowSpacing(6, 8);
  f5_layout->setRowStretch(6, 1);
  f5_layout->activate();
  // end 'entry4.dlg'

  main_box->activate();

  resize( 380, 472 );
  setMinimumSize( 380, 472 );

  c_type->setFocus();
  connect( (QObject *) b_pixmap, SIGNAL(clicked()), this, SLOT(pixButPressed()) );
  connect( (QObject *) b_big_pixmap, SIGNAL(clicked()), this, SLOT(bigPixButPressed()) );
  connect( (QObject *) b_mount_pix, SIGNAL(clicked()), this, SLOT(umountPixButPressed()) );
  connect( (QObject *) b_ok, SIGNAL(clicked()), parent, SLOT(change_accept()) );
  connect( (QObject *) b_cancel, SIGNAL(clicked()), parent, SLOT(change_reject()) );
  connect( (QObject *) i_pixmap, SIGNAL(returnPressed()),\
	   this, SLOT(pixnameChanged()) );
  connect( (QObject *) i_big_pixmap, SIGNAL(returnPressed()),\
	   this, SLOT(bigPixnameChanged()) );
  connect( (QObject *) i_mount_pix, SIGNAL(returnPressed()),\
	   this, SLOT(umountPixnameChanged()) );
  connect( c_type, SIGNAL(activated(int)), this, SLOT(typeActivated(int)) );
  connect( b_ins, SIGNAL(clicked()), this, SLOT(insertFileType()) );
  connect( b_rem, SIGNAL(clicked()), this, SLOT(removeFileType()) );
}


void EntryDialog::pixnameChanged()
{
  QString new_name = i_pixmap->text();
  if( new_name.isEmpty() )
    b_pixmap->setPixmap( global_pix_loader->loadApplicationMiniIcon(i_big_pixmap->text(),
								    16, 16 ));
  else
    b_pixmap->setPixmap( global_pix_loader->loadApplicationMiniIcon(new_name, 16, 16 ));
}

void EntryDialog::bigPixnameChanged()
{
  QString new_name = i_big_pixmap->text();
  b_big_pixmap->setPixmap( global_pix_loader->loadApplicationIcon(new_name, 70, 70 ));
  if( ((QString) i_pixmap->text()).isEmpty() )
    b_pixmap->setPixmap( global_pix_loader->loadApplicationMiniIcon(new_name, 16, 16 ));
}

void EntryDialog::umountPixnameChanged()
{
  QString new_name = i_mount_pix->text();
  b_mount_pix->setPixmap( global_pix_loader->loadApplicationIcon(new_name, 70, 70) );
}

void EntryDialog::pixButPressed()
{
  QString name;
  QPixmap temp;
  temp = global_pix_sel2->selectIcon(name, "*" );
  if( name.isNull() )
    return;
  i_pixmap->setText(name);
  pixnameChanged();
}

void EntryDialog::bigPixButPressed()
{
  QString name;
  QPixmap temp;
  temp = global_pix_sel->selectIcon(name, "*" );
  if( name.isNull() )
    return;
  i_big_pixmap->setText(name);
  bigPixnameChanged();
}

void EntryDialog::umountPixButPressed()
{
  QString name;
  QPixmap temp;
  temp = global_pix_sel->selectIcon(name, "*" );
  if( name.isNull() )
    return;
  i_mount_pix->setText(name);
  umountPixnameChanged();
}

void EntryDialog::typeActivated( int t )
{
  delete tb_app;
  delete f_sub_layout;
  f_sub_layout = new QVBoxLayout(f_sub_diag);
  tb_app = new KTabCtl(f_sub_diag);
  f_sub_layout->addWidget(tb_app);
  f_sub_layout->activate();
  switch(t) {
  case 0:  // separator
    break;
  case 1:  // submenu
    break;
  case 2:  // unix_com
    tb_app->addTab(f_app_1, klocale->translate("Execute"));
    tb_app->addTab(f_app_2, klocale->translate("Application"));
    i_command->setText(i_sexec->text());
    break;
  case 3:  // swallow_com
    tb_app->addTab(f_app_3, klocale->translate("Swallow"));
    tb_app->addTab(f_app_2, klocale->translate("Application"));
    i_sexec->setText(i_command->text());
    break;
  case 4:  // url
    tb_app->addTab(f_app_4, klocale->translate("Link"));
    f_app_4->raise();
    break;
  case 5:  // device
    tb_app->addTab(f_app_5, klocale->translate("Device"));
    f_app_5->raise();
    break;
  };
  tb_app->show();
}

void EntryDialog::insertFileType()
{
  if( l_outside->currentItem() < 0 )
    return;
  l_inside->inSort( l_outside->text(l_outside->currentItem()) );
  l_outside->removeItem( l_outside->currentItem() );
}

void EntryDialog::removeFileType()
{
  if( l_inside->currentItem() < 0 )
    return;
  l_outside->inSort( l_inside->text(l_inside->currentItem()) );
  l_inside->removeItem( l_inside->currentItem() );
}
