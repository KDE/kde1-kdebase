// -*- C++ -*-

//
//  kmenuedit
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
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

#include <qapp.h>
#include <qdir.h>
#include <qfileinf.h>

#include <kapp.h>
#include <drag.h>
#include <kiconloader.h>
#include <kiconloaderdialog.h>
#include <kmsgbox.h>

#include "IconPathDialog.h"
#include "DesktopPathDialog.h"
#include "kmenuedit.h"

KIconLoader *global_pix_loader;
KIconLoaderDialog *global_pix_sel;
KIconLoaderDialog *global_pix_sel2;

int main( int argc, char **argv )
{
  //debug ( "[kmenuedit] started-------------------------" );

  KApplication a( argc, argv, "kmenuedit" );

  QString temp1, temp2;
  KConfig *config = a.getConfig();
  config->setGroup("KDE Desktop Entries");
  if( !config->hasKey("Path") || !config->hasKey("PersonalPath") )
    {
      if( config->hasKey("Path") )
	  temp1 = config->readEntry("Path");
      else
	  temp1 = KApplication::kde_appsdir().copy();
      if( config->hasKey("PersonalPath") )
	temp2 = config->readEntry("PersonalPath");
      else
	{
	  temp2 = QDir::homeDirPath();
	  temp2 += "/.kde/share/applnk";
	}
       DesktopPathDialog *desktop_dialog = new DesktopPathDialog( temp1, temp2 );
       desktop_dialog->exec();
       temp1 = desktop_dialog->getPath();
       config->writeEntry("Path", temp1);
       temp2 = desktop_dialog->getPPath();
       config->writeEntry("PersonalPath", temp2);
    }
  // check for existance of PersonalPath
  temp2 = config->readEntry("PersonalPath");
  QFileInfo fi(temp2);
  bool error = FALSE;
  if( !(fi.exists() && fi.isDir()) )
    {
      if( KMsgBox::yesNo( NULL, klocale->translate("KMenuedit"), 
			  klocale->translate("The directory for your personal menu does not exist.\n Do you want to create it now ?")) == 1 )
	{
	  QDir dir( fi.dirPath() );
	  if( !dir.mkdir(fi.fileName()) )
	    error = TRUE;
	}
      else
	{
	  error = TRUE;
	}
      if( error )
	{
	  KMsgBox::message( NULL, klocale->translate("KMenuedit"), 
			    klocale->translate("Unable to create directory for personal menu.\n Select Ok to exit KMenuedit."));
	  exit(1);
	}
    }
  global_pix_loader = KApplication::getKApplication()->getIconLoader();
  global_pix_sel = new KIconLoaderDialog;
  global_pix_sel2 = new KIconLoaderDialog;
  QStrList icon_sel_list;
  QStrList icon_sel_list2;
  icon_sel_list.append(KApplication::kde_icondir());
  icon_sel_list.append(QDir::homeDirPath()+"/.kde/share/icons");
  icon_sel_list2.append(KApplication::kde_icondir() + "/mini");
  icon_sel_list2.append(QDir::homeDirPath()+"/.kde/share/icons/mini");
  global_pix_sel->setDir(&icon_sel_list);
  global_pix_sel2->setDir(&icon_sel_list2);

  KMenuEdit edit;
  if( a.isRestored() )
    {
      if( KTopLevelWidget::canBeRestored(1) )
	{
	  edit.restore(1);
	}
    }
  a.setMainWidget( (QWidget *) &edit );
  a.setRootDropZone( new KDNDDropZone( (QWidget *) &edit, DndNotDnd ) );
  edit.show();
  edit.resize(edit.size());
  return a.exec();
}







