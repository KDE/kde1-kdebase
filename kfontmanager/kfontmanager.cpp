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
#include <qfile.h>
#include <qtstream.h> 
#include <qtabdlg.h>


#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <kapp.h>
#include "kfontmanager.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


extern char DOCS_PATH[256];
extern char PICS_PATH[256];

#define HEIGHT 290
#define WIDTH 400

#include <klocale.h>
#define i18n(X) klocale->translate(X)

KFontManager::KFontManager (QWidget * parent, const char *name)
    : QDialog (parent, name)
{


  availableLabel = new QLabel(i18n("Available X11 Fonts"), this,"availlabel");

  availableFontsList = new QListBox(this,"avalableFonts");
  
  connect(availableFontsList,SIGNAL(highlighted(int)),
	  this,SLOT(display_available_example(int)));
  
  selectedLabel = new QLabel(i18n("Fonts made available to KDE"), this,"selectlabel");
  selectedFontsList = new QListBox(this,"avalableFonts");

  connect(selectedFontsList,SIGNAL(highlighted(int)),
	  this,SLOT(display_selected_example(int)));

  add = new QPushButton(i18n("Add"), this);
  connect(add,SIGNAL(clicked()),this,SLOT(add_slot()));

  remove = new QPushButton(i18n("Remove"), this);
  connect(remove,SIGNAL(clicked()),this,SLOT(remove_slot()));

  help = new QPushButton(i18n("Help"), this);
  connect(help,SIGNAL(clicked()),SLOT(helpselected()));

  example_label = new QLabel(this,"examples");
  example_label->setAlignment(AlignCenter);
  example_label->setBackgroundColor(white);
  example_label->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  example_label->setText(i18n("The KDE Font Manager Example String"));

  connect( parent, SIGNAL( applyButtonPressed() ),
	   this, SLOT( writeKDEInstalledFonts() ) );  	  

  resize(parent->width(),parent->height());

  readSettings();
  queryFonts();

  setMinimumSize (100, 100);

}



void KFontManager::about(){

  QMessageBox::message (i18n("About kfontmanager"), i18n("kfontmanager Version 0.2\n"\
			"Copyright 1997\nBernd Johannes Wuebben\n"\
			"wuebben@math.cornell.edu\n"),i18n("Ok"));

}

void KFontManager::remove_slot(){

  if(selectedFontsList->currentItem() != -1){
    selectedFontsList->removeItem(selectedFontsList->currentItem());
  }
  else{
    QApplication::beep();
  }

}

void KFontManager::add_slot(){
  
  if(availableFontsList->currentItem() != -1){

    QString new_item = availableFontsList->text(availableFontsList->currentItem());
    QString string;

    for (uint i  = 0; i < selectedFontsList->count();i++){
    
      string = "";
      string = selectedFontsList->text(i);
      if ( string == new_item){
	// already in KDE font list
         QApplication::beep();
	 return;
      }
    }
    selectedFontsList->inSort(new_item);


  }
  else{
    QApplication::beep();
  }
}



void KFontManager::resizeEvent(QResizeEvent *e){

  (void) e;

  availableLabel->setGeometry (15,10,180,25);
  selectedLabel->setGeometry (width()/2 + 5,10,180,25);
  availableFontsList->setGeometry(10,35,(width()-20)/2 -10 ,height()-150);
  selectedFontsList->setGeometry((width()-20)/2 +5,35,(width()-20)/2+5 ,height()-150);
  example_label->setGeometry(20,height() - 100 ,width() - 40, 40);

  add->setGeometry(( width()-20) - 190 , height() - 50, 90, 30);
  remove->setGeometry((width()-20) -90, height() - 50, 90, 30);
  help->setGeometry(20, height() - 50, 90, 30);

}

void KFontManager::helpselected(){

  kapp->invokeHTMLHelp( "kfontmanager/index.html", "" );

}

void KFontManager::apply(bool){

}


void KFontManager::display_available_example(int i){

  QString string;

  string = availableFontsList->text(i);
  example_label->setFont(QFont(string,14));
  
}


void KFontManager::display_selected_example(int i){

  QString string;

  string = selectedFontsList->text(i);
  example_label->setFont(QFont(string,14));
  
  
}

bool KFontManager::loadKDEInstalledFonts(){

  QString fontfilename;

  //TODO replace by QDir::homePath();

  fontfilename =  getenv("HOME");
  if(fontfilename.isEmpty()){
    QMessageBox::message(i18n("Sorry"),i18n("The environment variable HOME\n"\
			 "is not set\n"),i18n("Ok"));
    QApplication::exit(1);

  }
    
  fontfilename = fontfilename + "/.kde/share/config/kdefonts";

  QString home;
  home = getenv("HOME");
  home = home + "/.kde";
  struct stat buf;

  if( stat(home.data(),&buf) == -1 ){
    mkdir(home.data(),S_IRUSR | S_IWUSR | S_IXUSR |S_IRGRP | S_IWGRP | S_IXGRP | 
	  S_IROTH | S_IWOTH |S_IXOTH);
  }

  home = home + "/share";
  
  if( stat(home.data(),&buf) == -1 ){
    mkdir(home.data(),S_IRUSR | S_IWUSR | S_IXUSR |S_IRGRP | S_IWGRP | S_IXGRP | 
	  S_IROTH | S_IWOTH |S_IXOTH);
  }

  home = home + "/config";
  
  if( stat(home.data(),&buf) == -1 ){
    mkdir(home.data(),S_IRUSR | S_IWUSR | S_IXUSR |S_IRGRP | S_IWGRP | S_IXGRP | 
	  S_IROTH | S_IWOTH |S_IXOTH);
  }


  QFile fontfile(fontfilename);

  if (!fontfile.exists())
    return false;

  if(!fontfile.open(IO_ReadOnly)){
    return false;
  }

  if (!fontfile.isReadable())
    return false;
  
  selectedFontsList->setAutoUpdate(FALSE);
  
  QTextStream t(&fontfile);


  while ( !t.eof() ) {
    QString s = t.readLine();
    s = s.stripWhiteSpace();
    if (!s.isEmpty())
      selectedFontsList->insertItem( s );
  }

  fontfile.close();

  selectedFontsList->setAutoUpdate(TRUE);
   selectedFontsList->update();
  
  return true;

}

bool KFontManager::writeKDEInstalledFonts(){
  

  QString fontfilename;

  fontfilename =  getenv("HOME");
  fontfilename = fontfilename + "/.kde/share/config/kdefonts";

  QFile fontfile(fontfilename);

  if (!fontfile.open(IO_WriteOnly | IO_Truncate)){
    QMessageBox::message(i18n("Sorry"),i18n("Can not create:\n ~/.kde/share/config/kdefonts\n"),i18n("Ok"));
    return false;
  }

  if (!fontfile.isWritable()){
    QMessageBox::message(i18n("Sorry"),i18n("~/.kde/share/config/kdefonts exists but\n"\
			 "is not writeable\n"\
			 "Can't save KDE Fontlist."),i18n("Ok"));
    return false;
  }

  QTextStream t(&fontfile);
  
  int number = selectedFontsList->count();

  if( number >  0){
    QString fontname;
    for(int i = 0; i < number; i++){
      fontname = selectedFontsList->text(i);
      fontname = fontname.stripWhiteSpace();
      if (!fontname.isEmpty())
	t << fontname.data() << '\n';
    }
  
  }

  fontfile.close();
  
  return true;

}

void KFontManager::queryFonts(){

  int numFonts;
  Display *kde_display;
  char** fontNames;
  char** fontNames_copy;
  QString qfontname;

  QStrList fontlist(TRUE);
  QStrList installedfontlist(TRUE);
  
  kde_display = XOpenDisplay( NULL );

  bool have_installed = loadKDEInstalledFonts();

  fontNames = XListFonts(kde_display, "*", 32767, &numFonts);
  fontNames_copy = fontNames;

  availableFontsList->setAutoUpdate(FALSE);
  selectedFontsList->setAutoUpdate(FALSE);

  for( int k = 0; k < numFonts; k++){
    
    if (**fontNames != '-'){ // font name doesn't start with a dash -- an alias
      
      /*

      qfontname = "";
      qfontname = *fontNames;
      if(fontlist.find(qfontname) == -1)
          fontlist.inSort(qfontname);

      */

      fontNames ++;
      continue;
    };
      
    qfontname = "";
    qfontname = *fontNames;
    int dash = qfontname.find ('-', 1, TRUE); // find next dash

    if (dash == -1) { // No such next dash -- this shouldn't happen.
                      // let's skip it.
      fontNames ++;
      continue;
    }

    // the font family name is between the second and third dash therefore
    // let's find the third dash:

    int dash_two = qfontname.find ('-', dash + 1 , TRUE); 

    if (dash == -1) { // No such next dash -- this shouldn't happen.
                      // let's skip it.
      fontNames ++;
      continue;
    }

    // fish the font family name out of the font info string

    qfontname = qfontname.mid(dash +1, dash_two - dash -1);

    if(fontlist.find(qfontname) == -1)
      fontlist.inSort(qfontname);

    if(!have_installed){

      // we don't have a kdefontlist file yet -- prepare a default list 
      // of installed fonts
      
      if( !qfontname.contains("open look", TRUE)){
	if(qfontname != "nil"){
	  if(installedfontlist.find(qfontname) == -1)
	    installedfontlist.inSort(qfontname);
	}
      }
    }

    fontNames ++;

  }

  for(fontlist.first(); fontlist.current(); fontlist.next())
   availableFontsList->insertItem(fontlist.current());

  if(!have_installed){
    
    for(installedfontlist.first(); installedfontlist.current(); installedfontlist.next())
      selectedFontsList->insertItem(installedfontlist.current());
  }

  availableFontsList->setAutoUpdate(TRUE);
  availableFontsList->update();
  selectedFontsList->setAutoUpdate(TRUE);
  selectedFontsList->update();

  XFreeFontNames(fontNames_copy);
  XCloseDisplay(kde_display);


}


void KFontManager::readSettings(){



  /*	QString str;
	
	config = a->getConfig();

	config->setGroup( "Text Font" );
	*/

}

void KFontManager::writeSettings(){
		
  /*
	config = a->getConfig();
	
	config->setGroup( "Text Font" );


	config->sync();

	*/
}

#include "kfontmanager.moc"

