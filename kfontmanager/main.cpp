    /*

    $Id$

    Requires the Qt widget libraries, available at no cost at 
    http://www.troll.no
       
    Copyright (C) 1997 Bernd Johannes Wuebben   
                       wuebben@math.cornell.edu


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */
 


#include <qstrlist.h> 
#include <qtabdlg.h>

#include <kapp.h>

#include "kfontmanager.h"
#include "kfontlist.h"
#include "kfontexplorer.h"
#include <kfontdialog.h>
#include <klocale.h>
#define i18n(X) klocale->translate(X)

char DOCS_PATH[256];
char PICS_PATH[256];

void setup(){


	QTabDialog *mainWindow = new QTabDialog( 0, 0, TRUE );

	mainWindow->setCaption( i18n("KDE Font Manager") );
	mainWindow->setCancelButton(i18n("Cancel"));
	mainWindow->setApplyButton(i18n("Apply"));

	KFontManager manager(mainWindow,"manager");
	KFontExplorer explorer(mainWindow,"explorer");
	
	KFontList list(mainWindow,"list");

	mainWindow->addTab( &manager, i18n("KDE Fonts") );
	mainWindow->addTab( &explorer,i18n("Font Explorer"));
	mainWindow->addTab( &list, i18n("Raw X11 Font List") );
	
       	mainWindow->resize( 430, 500 );
	mainWindow->exec();
}

int main( int argc, char *argv[] ){


	KApplication a( argc, argv, "kfontmanager" );
	a.setFont(QFont("Helvetica",12,QFont::Normal),TRUE);

	char *kdedir = a.kdedir().data();

	sprintf( PICS_PATH, "%s/share/apps/kfontmanager/pics", kdedir );
	sprintf( DOCS_PATH, "%s/share/doc/HTML/kfontmanager", kdedir );
	
	setup();
	return 1;
}



