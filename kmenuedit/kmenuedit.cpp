// -*- C++ -*-

//
//  kmenuedit
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@bonn.netsurf.de
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

#include <qscrbar.h>
#include <qpushbt.h>
#include <qframe.h>
#include <qkeycode.h>
#include <qmsgbox.h>
#include <qdir.h>
#include <qstrlist.h>

#include <kapp.h>
#include <kmsgbox.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kiconloaderdialog.h>

#include "kmenuedit.h"
#include "kmenuedit.moc"
#include "pmenu.h"

extern KIconLoader *global_pix_loader;
KStatusBar *global_status_bar;
QStrList *global_file_types;
bool changes_to_save;

KMenuEdit::KMenuEdit( const char *name = NULL )
  : KTopLevelWidget( name )
{
  initMetaObject();
  KConfig *config = KApplication::getKApplication()->getConfig();
  config->setGroup("kmenuedit");
  f_main = new QFrame(this);
  f_main->setFrameStyle( 0 );

  menubar = new KMenuBar( this, "menubar" );
  QPopupMenu *file = new QPopupMenu;
  file->insertItem("Reload", this, SLOT(reload()) );
  file->insertItem("Save", this, SLOT(save()) );
  file->insertSeparator();
  file->insertItem("Quit", qApp, SLOT(quit()), CTRL+Key_Q);
  //QPopupMenu *edit_menu = new QPopupMenu;
  QPopupMenu *options = new QPopupMenu;
  options->insertItem("Reload Filetypes", this, SLOT(reloadFileTypes()) );
  QPopupMenu *help = new QPopupMenu;
  //help->insertItem("Help", this, SLOT(startHelp()), CTRL+Key_H );
  help->insertItem("About", this, SLOT(about()) );

  menubar->insertItem( "&File", file );
  //menubar->insertItem( "&Edit", edit_menu);
  menubar->insertItem( "&Options", options);
  menubar->insertSeparator();
  menubar->insertItem( "&Help", KApplication::getKApplication()->getHelpMenu(TRUE, "Kmenuedit 0.2.0\n(C) by Christoph Neerfeld\nChristoph.Neerfeld@mail.bonn.netsurf.de") );
  //menubar->insertItem( "&Help", help, ALT+Key_H );

  // create toolbar
  toolbar = new KToolBar(this);
  QPixmap temp_pix;
  temp_pix = global_pix_loader->loadIcon("reload.xpm");
  toolbar->insertButton(temp_pix, 0, SIGNAL(clicked()), this,
                      SLOT(reload()), TRUE, "Reload");
  temp_pix = global_pix_loader->loadIcon("filefloppy.xpm");
  toolbar->insertButton(temp_pix, 1, SIGNAL(clicked()), this,
                      SLOT(save()), TRUE, "Save");
  toolbar->insertSeparator();
  //temp_pix = global_pix_loader->loadIcon("exit.xpm");
  //toolbar->insertButton(temp_pix, 2, SIGNAL(clicked()), qApp,
  //                    SLOT(quit()), TRUE, "Exit");
  toolbar->setBarPos( (KToolBar::BarPosition) config->readNumEntry("ToolBarPos") );
  setMenu(menubar);
  addToolBar(toolbar);
  statusbar = new KStatusBar(this);
  global_status_bar = statusbar;
  statusbar->insertItem("--------------------", 0);
  statusbar->insertItem("", 1);
  statusbar->changeItem("", 0);
  setView(f_main, TRUE);

  setStatusBar(statusbar);
  enableToolBar(KToolBar::Show);

  f_mask = new QFrame(f_main);
  f_mask->setFrameStyle( 0 );
  f_move = new QFrame(f_mask, "f_move");
  f_move->setGeometry( 0, 0, QApplication::desktop()->width(), QApplication::desktop()->height() );
  scrollx = new QScrollBar(f_main);
  scrollx->setMinimumSize( 16, 16 );
  scrollx->setOrientation(QScrollBar::Horizontal);
  scrollx->setRange(0, QApplication::desktop()->width() - f_mask->width() );
  scrollx->setSteps( 8, f_mask->width() );
  scrolly = new QScrollBar(f_main);
  scrolly->setMinimumSize( 16, 16 );
  scrolly->setRange(0, QApplication::desktop()->height() - f_mask->height() );
  scrolly->setSteps( 8, f_mask->height() );

  top2bottom = new QGridLayout( f_main, 2, 2, 2 );
  top2bottom->addWidget( f_mask, 0, 0, AlignCenter );
  top2bottom->setRowStretch( 0, 1 );
  top2bottom->setColStretch( 0, 1 );
  top2bottom->addWidget( scrolly, 0, 1, AlignCenter );
  top2bottom->addWidget( scrollx, 1, 0, AlignCenter );
  top2bottom->activate();

  connect( scrollx, SIGNAL(valueChanged(int)), this, SLOT(move_h(int)) );
  connect( scrolly, SIGNAL(valueChanged(int)), this, SLOT(move_v(int)) );

  // setup menu data
  pers_menu_data = NULL;
  glob_menu_data = NULL;
  loadMenus();
  
  // load file types
  global_file_types = new QStrList;
  global_file_types->setAutoDelete(TRUE);
  reloadFileTypes();

  config->setGroup("kmenuedit");  
  int x, y;
  x = config->readNumEntry( "PersPosX", 5 );
  y = config->readNumEntry( "PersPosY", 5 );
  QPoint p( x, y );
  pers_menu_data->popupConfig( p, f_move, FALSE );
  x = config->readNumEntry( "GlobPosX", QApplication::desktop()->width() / 2 );
  y = config->readNumEntry( "GlobPosY", 5 );
  p.setX( x );
  p.setY( y );
  glob_menu_data->popupConfig( p, f_move, FALSE );

  setMinimumSize(400, 200);
  int width, height;
  width  = config->readNumEntry("Width");
  height = config->readNumEntry("Height");
  if( width < minimumSize().width() )
    width = minimumSize().width();
  if( height < minimumSize().height() )
    height = minimumSize().height();
  resize(width, height);
  changes_to_save = FALSE;
}

KMenuEdit::~KMenuEdit()
{
  if( changes_to_save )
    {
      if( QMessageBox::information ( this, "Changes not saved !", 
				     "Do you want to save your changes", "Yes", "No" )  == 0 )
	{
	  saveMenus();
	}
    }
  KConfig *config = KApplication::getKApplication()->getConfig();
  config->setGroup("kmenuedit");
  config->writeEntry("PersPosX", pers_menu_data->configPos().x());
  config->writeEntry("PersPosY", pers_menu_data->configPos().y());
  config->writeEntry("GlobPosX", glob_menu_data->configPos().x());
  config->writeEntry("GlobPosY", glob_menu_data->configPos().y());
  config->writeEntry("Width", width());
  config->writeEntry("Height", height());
  config->writeEntry("ToolBarPos", (int) toolbar->barPos() );
  config->sync();
}

void KMenuEdit::resizeEvent( QResizeEvent *e )
{
  KTopLevelWidget::resizeEvent( e );
  scrollx->setRange(0, QApplication::desktop()->width() - f_mask->width() );
  scrollx->setSteps( 8, f_mask->width() );
  scrolly->setRange(0, QApplication::desktop()->height() - f_mask->height() );
  scrolly->setSteps( 8, f_mask->height() );
}

void KMenuEdit::move_h( int x )
{
  f_move->move( x*(-1), f_move->y() );
}

void KMenuEdit::move_v( int y )
{
  f_move->move( f_move->x(), y*(-1) );
}

void KMenuEdit::startHelp()
{
  KApplication::getKApplication()->invokeHTMLHelp( "", "");
}

void KMenuEdit::about()
{
  QMessageBox::message( "About", \
                        "Kmenuedit 0.2.1\n\r(c) by Christoph Neerfeld\n\rChristoph.Neerfeld@bonn.netsurf.de", "Ok" );
}

void KMenuEdit::loadMenus()
{
  QCursor cursor(waitCursor);
  QApplication::setOverrideCursor(cursor);
  QString dir_name;
  KConfig *config = KApplication::getKApplication()->getConfig();
  config->setGroup("KDE Desktop Entries");
  dir_name = config->readEntry("PersonalPath");
  dir_name = dir_name.stripWhiteSpace();
  QDir dir(dir_name);
  if( pers_menu_data )
    delete pers_menu_data;
  pers_menu_data = new PMenu;
  pers_menu_data->parse(dir);
  if( !pers_menu_data->count() )
    pers_menu_data->add(new PMenuItem(unix_com, "EMPTY"));
  dir_name = config->readEntry("Path");
  dir_name = dir_name.stripWhiteSpace();
  dir = dir_name;
  if( glob_menu_data )
    delete glob_menu_data;
  glob_menu_data = new PMenu;
  glob_menu_data->parse(dir);
  //if( !glob_menu_data->count() )
  //  glob_menu_data->add(new PMenuItem(unix_com, "EMPTY"));
  QApplication::restoreOverrideCursor();
}

void KMenuEdit::saveMenus()
{
  QCursor cursor(waitCursor);
  QApplication::setOverrideCursor(cursor);
  QString dir_name;
  KConfig *config = KApplication::getKApplication()->getConfig();
  config->setGroup("KDE Desktop Entries");
  dir_name = config->readEntry("PersonalPath");
  dir_name = dir_name.stripWhiteSpace();
  QDir dir(dir_name);
  pers_menu_data->copyLnkFiles(dir);
  pers_menu_data->renameLnkFiles(dir);
  pers_menu_data->writeConfig(dir);
  dir_name = config->readEntry("Path");
  dir_name = dir_name.stripWhiteSpace();
  dir.setPath(dir_name);
  glob_menu_data->copyLnkFiles(dir);
  glob_menu_data->renameLnkFiles(dir);
  glob_menu_data->writeConfig(dir);
  QApplication::restoreOverrideCursor();
  changes_to_save = FALSE;
}

void KMenuEdit::reload()
{
  int code = 1;
  if( changes_to_save )
    {
      code = KMsgBox::yesNo(this, "Reload Menus", "Reloading the menus will discard all changes.\n
Are you sure you want to reload ?" );
    }
  if( code == 1 )
    {
      loadMenus();
      QPoint p( 5, 5 );
      pers_menu_data->popupConfig( p, f_move, FALSE );
      p.setX( QApplication::desktop()->width() / 2 );
      glob_menu_data->popupConfig( p, f_move, FALSE );
    }
}

void KMenuEdit::save()
{
  saveMenus();
}

void KMenuEdit::reloadFileTypes()
{
  // kfm II method of mimetypes
  QString dir_name = KApplication::kdedir();
  dir_name += "/share/mimelnk";
  QDir dir(dir_name);
  if( !dir.exists() )
    return;
  global_file_types->clear();
  dir.setFilter(QDir::Dirs);

  const QFileInfoList *dir_list = dir.entryInfoList();
  QFileInfoListIterator d_it( *dir_list );
  QFileInfo *fi;

  QDir sub_dir(dir);
  sub_dir.setFilter(QDir::All);
  sub_dir.setNameFilter("*.kdelnk");
  const QFileInfoList *subdir_list = sub_dir.entryInfoList();
  QFileInfoListIterator subd_it( *subdir_list );

  while( (fi = d_it.current()) ) 
    {
      if( fi->fileName()[0] == '.' )
	{ ++d_it; continue; }
      sub_dir.cd(fi->fileName());
      subdir_list = sub_dir.entryInfoList();
      subd_it.toFirst();
      while( (fi = subd_it.current()) )
	{
	  QFile config(fi->absFilePath());
	  if( !config.open(IO_ReadOnly) ) 
	    { ++subd_it; continue; }
	  config.close(); // kalle
	  // kalle	  QTextStream st( (QIODevice *) &config);
	  KConfig kconfig(fi->absFilePath());
	  kconfig.setGroup("KDE Desktop Entry");
	  kconfig.readEntry("WmCommand");
	  //debug("type = %s", (const char *) kconfig.readEntry("MimeType") );
	  global_file_types->inSort( kconfig.readEntry("MimeType") );
	  config.close();
	  ++subd_it;
	}
      sub_dir.cdUp();
      ++d_it;
    }
}



