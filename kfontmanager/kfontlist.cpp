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
 

#include <qtabdlg.h>
#include <qstrlist.h> 

#include <kapp.h>

#include "kfontlist.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <klocale.h>
#define i18n(X) klocale->translate(X)

extern char DOCS_PATH[256];
extern char PICS_PATH[256];

KFontList::KFontList (QWidget * parent, const char *name)
    : QDialog (parent, name)
{

  eframe = new QMultiLineEdit(this,"edit");
  //  eframe->setGeometry(10,10, 400, 400);
  eframe->setFont(QFont("fixed",10));
  setMinimumSize (100, 100);

  readSettings();
  queryFonts();

}



void KFontList::about(){

  QMessageBox::message (i18n("About kfontmanager"), i18n("kfontmanager Version 0.1\n"\
			"Copyright 1997\nBernd Johannes Wuebben\n"\
			"wuebben@math.cornell.edu\n"),i18n("Ok"));

}


void KFontList::resizeEvent(QResizeEvent *e){

  (void) e;
  eframe->setGeometry(10,10, width() - 20, height() - 20);


}


/*void KFontList::helpselected(){
  
  if ( fork() == 0 )
    {
      QString path = DOCS_PATH;
      path += "/kedit+.html";
      execlp( "kdehelp", "kdehelp", path.data(), 0 );
      ::exit( 1 );      
      
    }	 

}
*/
/*
void KFontList::apply(bool){

}
*/

void KFontList::queryFonts(){

  int numFonts;
  Display *kde_display;
  char** fontNames;
  char** fontNames_copy;
  QString qfontname;

  QStrList fontlist(TRUE);


  kde_display = XOpenDisplay( NULL );

  fontNames = XListFonts(kde_display, "*", 32767, &numFonts);
  fontNames_copy = fontNames;

  eframe->setAutoUpdate(FALSE);
  

  for(int i = 0 ; i< numFonts - 1; i++){

    if (**fontNames != '-'){ // Font Name doesn't start with a dash -- an alias
      fontNames ++;
      continue;
    };
      
    qfontname = "";
    qfontname = *fontNames;

    //    if(fontlist.find(qfontname) == -1)
            fontlist.inSort(qfontname);


    fontNames ++;

  }

  for(fontlist.first(); fontlist.current(); fontlist.next())
    eframe->insertLine(fontlist.current());

  eframe->setAutoUpdate(TRUE);

  eframe->update();

  XFreeFontNames(fontNames_copy);

  XCloseDisplay(kde_display);




}


void KFontList::readSettings(){



  /*	QString str;
	
	config = a->getConfig();

	config->setGroup( "Text Font" );
	*/

}

void KFontList::writeSettings(){
		
  /*
	config = a->getConfig();
	
	config->setGroup( "Text Font" );


	config->sync();

	*/
}

#include "kfontlist.moc"

