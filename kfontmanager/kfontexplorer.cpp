/*
    $Id$

    Requires the Qt widget libraries, available at no cost at 
    http://www.troll.no
       
    Copyright (C) 1996 Bernd Johannes Wuebben   
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
  
    $Log$
    sure they are still ok.

    Revision 1.7  1997/11/09 04:04:59  wuebben
    Bernd: necessary iso charset changes

    Revision 1.6  1997/11/07 18:45:15  kulow
    some more porting issues. Mainly default arguments and variable binding
    in for loops

    Revision 1.5  1997/08/31 19:45:52  kdecvs
    Kalle:
    adapted to changes in KConfig and KLocale

    Revision 1.4  1997/08/31 15:04:25  kdecvs
    Lars: i18n

    Revision 1.3  1997/05/22 13:30:52  kulow
    Coolo: updated kedit to 0.5
    updated kfontmanager to 0.2.2

    Revision 1.1  1997/05/21 04:47:09  wuebben
    Initial revision

    Revision 1.4  1997/05/21 04:44:47  wuebben
    preparing release 0.2.2

    Revision 1.3  1997/04/29 05:38:06  wuebben
    *** empty log message ***



#include <qstrlist.h> 

#include "stdio.h"
#include <stdio.h>

#include "qfile.h"
#include "kfontmanager.h"
#include <qstrlist.h> 
#include <qfile.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


#define YOFFSET 5
#define XOFFSET 5
#include <X11/Xlib.h>

#include <klocale.h>
#define klocale KLocale::klocale()
#include <kapp.h>
#include "kfontexplorer.h"

#define YOFFSET  5
#define XOFFSET  5
#define LABLE_LENGTH  40
#define LABLE_HEIGHT 20
#include <klocale.h>
#define i18n(X) klocale->translate(X)
					     QFont::ISO_8859_2,
KFontExplorer::KFontExplorer( QWidget *parent, const char *name,  bool modal)
    : QDialog( parent, name, modal ){
					     QFont::ISO_8859_6,
					     QFont::ISO_8859_7,
                   
  box1->setGeometry(15,15 ,SIZE_X -  XOFFSET +10
		   ,140);
  box1->setTitle(i18n("Requested Font"));
{

  box1->setGeometry(15,165,SIZE_X -  XOFFSET + 10 
  setCaption( klocale->translate("Select Font") );
  box1->setTitle(i18n("Actual Font"));
  box1->setGeometry(XOFFSET,YOFFSET,SIZE_X -  XOFFSET
		   ,150);
  box1->setTitle( klocale->translate("Requested Font") );
  family_label->setText(i18n("Family:"));
  family_label->setGeometry(5*XOFFSET,10*YOFFSET,LABLE_LENGTH,LABLE_HEIGHT);
  box1->setGeometry(XOFFSET,160,SIZE_X -  XOFFSET
		   ,130);
  actual_family_label->setText(i18n("Family:"));
  actual_family_label->setGeometry(5*XOFFSET,190,40,LABLE_HEIGHT);

  family_label = new QLabel(this,"family");
  actual_family_label_data->setGeometry(5*XOFFSET +50 ,190,110,LABLE_HEIGHT);
			     LABLE_HEIGHT);

  size_label->setText(i18n("Size:"));
  size_label->setGeometry(7*XOFFSET + LABLE_LENGTH + 12*XOFFSET +2* FONTLABLE_LENGTH,
			  10*YOFFSET,LABLE_LENGTH,LABLE_HEIGHT);

  actual_charset_label_data = new QLabel(this,"acharsetd");
  actual_size_label->setText(i18n("Size:"));
  actual_size_label->setGeometry(5*XOFFSET,190 +LABLE_HEIGHT ,
				 LABLE_LENGTH,LABLE_HEIGHT);
  size_label->setText(klocale->translate("Size:"));
  size_label->setGeometry(6*XOFFSET + LABLE_LENGTH + 12*XOFFSET +2* FONTLABLE_LENGTH,
  actual_size_label_data->setGeometry(5*XOFFSET +50 ,190 + LABLE_HEIGHT

  actual_size_label = new QLabel(this,"asize");
  actual_size_label->setText(klocale->translate("Size:"));
  weight_label->setText(i18n("Weight:"));
  weight_label->setGeometry(5*XOFFSET,18*YOFFSET + LABLE_HEIGHT 

  actual_size_label_data = new QLabel(this,"asized");
  actual_size_label_data->setGeometry(3*XOFFSET +60 ,200 + LABLE_HEIGHT
  actual_weight_label->setText(i18n("Weight:"));
  actual_weight_label->setGeometry(5*XOFFSET,190 + 2*LABLE_HEIGHT ,
				 LABLE_LENGTH,LABLE_HEIGHT);
  weight_label->setText(klocale->translate("Weight:"));
  weight_label->setGeometry(3*XOFFSET,15*YOFFSET + LABLE_HEIGHT -20 
  actual_weight_label_data->setGeometry(5*XOFFSET +50 ,190 + 2*LABLE_HEIGHT

  actual_weight_label = new QLabel(this,"aweight");
  actual_weight_label->setText(klocale->translate("Weight:"));
  style_label->setText(i18n("Style:"));
  style_label->setGeometry(7*XOFFSET + LABLE_LENGTH + 12*XOFFSET + 

			   17*YOFFSET + LABLE_HEIGHT 
  actual_weight_label_data->setGeometry(3*XOFFSET +60 ,200 + 2*LABLE_HEIGHT
				      ,110,LABLE_HEIGHT);

  style_label = new QLabel(this,"style");
  actual_style_label->setText(i18n("Style:"));
  actual_style_label->setGeometry(5*XOFFSET,190 + 3*LABLE_HEIGHT ,
				 LABLE_LENGTH,LABLE_HEIGHT);
			   15*YOFFSET + LABLE_HEIGHT  -20
			 ,LABLE_LENGTH,
  actual_style_label_data->setGeometry(5*XOFFSET +50 ,190 + 3*LABLE_HEIGHT

  actual_style_label = new QLabel(this,"astyle");
  actual_style_label->setGeometry(3*XOFFSET,200 + 3*LABLE_HEIGHT ,

  //  family_combo->setInsertionPolicy(QComboBox::NoInsertion);

   fill_family_combo();
  actual_style_label_data = new QLabel(this,"astyled");
  family_combo->setGeometry(8*XOFFSET + LABLE_LENGTH
			    ,10*YOFFSET - COMBO_ADJUST ,4* LABLE_LENGTH,COMBO_BOX_HEIGHT);


  //  QToolTip::add( family_combo, "Select Font Family" );
  family_combo->setInsertionPolicy(QComboBox::NoInsertion);
  family_combo->setGeometry(6*XOFFSET + LABLE_LENGTH
  size_combo = new QComboBox( true, this, "Size" );
      charset_combo->insertItem( charsetsStr[i] );

  charset_combo->setInsertionPolicy(QComboBox::NoInsertion);
  connect( charset_combo, SIGNAL(activated(int)),
	   SLOT(charset_chosen_slot(int)) );
  // QToolTip::add( charset_combo, "Select Font Weight" );

  size_combo = new QComboBox( true, this, klocale->translate("Size") );
  size_combo->insertItem( "4" );
  size_combo->insertItem( "5" );
  size_combo->insertItem( "6" );
  size_combo->insertItem( "7" );
  size_combo->insertItem( "8" );
  size_combo->insertItem( "9" );
  size_combo->insertItem( "10" );
  size_combo->insertItem( "11" );
  size_combo->insertItem( "12" );
  size_combo->insertItem( "13" );
  size_combo->insertItem( "14" );
  size_combo->insertItem( "15" );
  size_combo->insertItem( "16" );
  size_combo->insertItem( "17" );
  size_combo->insertItem( "18" );
  size_combo->insertItem( "19" );
  size_combo->setInsertionPolicy(QComboBox::NoInsertion);
  size_combo->setGeometry(12*XOFFSET + 6*LABLE_LENGTH
			    ,10*YOFFSET - COMBO_ADJUST 

  // we may want to allow the user to choose another size, since I
  // can really not presume to have listed all useful sizes.

  //  size_combo->setInsertionPolicy(QComboBox::NoInsertion);

  weight_combo = new QComboBox( TRUE, this, "Weight" );
  weight_combo->insertItem( "normal" );
  weight_combo->insertItem( "bold" );
  weight_combo->setGeometry(8*XOFFSET + LABLE_LENGTH
			    ,21*YOFFSET - COMBO_ADJUST
  //  QToolTip::add( size_combo, "Select Font Size in Points" );


  weight_combo = new QComboBox( TRUE, this, klocale->translate("Weight") );
  //  QToolTip::add( weight_combo, "Select Font Weight" );
  weight_combo->insertItem( klocale->translate("bold") );
  style_combo = new QComboBox( TRUE, this, "Style" );
  style_combo->insertItem( "roman" );
  style_combo->insertItem( "italic" );
  style_combo->setGeometry(12*XOFFSET + 6*LABLE_LENGTH
			    ,21*YOFFSET- COMBO_ADJUST
	   SLOT(weight_chosen_slot(const char *)) );
  // QToolTip::add( weight_combo, "Select Font Weight" );

  style_combo = new QComboBox( TRUE, this, klocale->translate("Style") );

  // QToolTip::add( style_combo, "Select Font Style" );
  style_combo->insertItem( klocale->translate("italic") );
  style_combo->setGeometry(10*XOFFSET + 6*LABLE_LENGTH
  cancel_button = new QPushButton(i18n("Cancel"),this);
			   ,2*LABLE_LENGTH + 20,COMBO_BOX_HEIGHT);
  cancel_button->setGeometry( 3*XOFFSET +100, OKBUTTONY, 80, BUTTONHEIGHT );
  connect( style_combo, SIGNAL(activated(const char *)),
  */
  /*  ok_button = new QPushButton( i18n("Ok"), this );
  ok_button->setGeometry( 3*XOFFSET, OKBUTTONY, 80, BUTTONHEIGHT );
  /*
  cancel_button = new QPushButton( klocale->translate("Cancel"),this);
  cancel_button->setGeometry( 3*XOFFSET +100, OKBUTTONY +40, 80, BUTTONHEIGHT );

  example_label = new QLabel(this,"examples");
  example_label->setFont(selFont);
  ok_button = new QPushButton( klocale->translate("Ok"), this );
  ok_button->setGeometry( 3*XOFFSET, OKBUTTONY +40,80, BUTTONHEIGHT );
  connect( ok_button, SIGNAL( clicked() ), SLOT( accept() ) );	
  */
  example_label = new QLabel(this,"examples");
  example_label->setText("Qui fit Maecenas, ut nemo");

  example_label->setGeometry(200,190,190, 80);
  example_label->setAlignment(AlignCenter);
  example_label->setBackgroundColor(white);
  setFont(QFont("Times",14,QFont::Normal));
  this->setMinimumSize(405,330);
  this->setMinimumSize(430,290);
  */

}


	KFontExplorer dlg( NULL, "Font Selector", TRUE );

  selFont.setCharSet(charsetsIds[index]);
  emit fontSelected(selFont);
}

int KFontExplorer::getFont( QFont &theFont )
{
	KFontExplorer dlg( 0L, "Font Selector", TRUE );
	dlg.setFont( theFont );
	int result = dlg.exec();

	if ( result == Accepted )
		theFont = dlg.font();

	return result;
}


void KFontExplorer::setFont( const QFont& aFont){

  selFont = aFont;
  setCombos();
  display_example(selFont);
}  


void KFontExplorer::family_chosen_slot(const char* family){

  selFont.setFamily(family);
  //display_example();
  emit fontSelected(selFont);
}

void KFontExplorer::size_chosen_slot(const char* size){
  
  QString size_string = size;

  selFont.setPointSize(size_string.toInt());
  if ( weight_string == QString("normal"))
  emit fontSelected(selFont);
  if ( weight_string == QString("bold"))

void KFontExplorer::weight_chosen_slot(const char* weight){

  QString weight_string = weight;

  if ( weight_string == QString(klocale->translate("normal")))
    selFont.setBold(false);
  if ( weight_string == QString(klocale->translate("bold")))
       selFont.setBold(true);
  // display_example();
  if ( style_string == QString("roman"))
}
  if ( style_string == QString("italic"))
void KFontExplorer::style_chosen_slot(const char* style){


  QString style_string = style;

  if ( style_string == QString(klocale->translate("roman")))
    selFont.setItalic(false);
  if ( style_string == QString(klocale->translate("italic")))
    selFont.setItalic(true);
  emit fontSelected(selFont);
}
       

void KFontExplorer::display_example(const QFont& font){

  QString string;
  int i;

  example_label->setFont(font);
    actual_weight_label_data->setText("Bold");
  QFontInfo info = example_label->fontInfo();
    actual_weight_label_data->setText("Normal");
  
  string.setNum(info.pointSize());
    actual_style_label_data->setText("italic");

    actual_style_label_data->setText("roman");
    actual_weight_label_data->setText(klocale->translate("Bold"));

    actual_style_label_data->setText(klocale->translate("roman"));
  
 QFont::CharSet charset=info.charSet();
  for(i = 0;i<CHARSETS_COUNT;i++)
    if (charset==charsetsIds[i]){
      break;
 int number_of_entries; 
  
 int i;
}

 for (int i = 0;i < number_of_entries ; i++){

 QString string;
 QComboBox* combo;
 int number_of_entries, i=0; 
 bool found;

 number_of_entries =  family_combo->count(); 
 string = selFont.family();
 combo = family_combo; 
 found = false;

 for (i = 0;i < number_of_entries ; i++){
   //   printf("%s with %s\n",string.data(), ((QString) combo->text(i)).data());
   if ( string.lower() == ((QString) combo->text(i)).lower()){
     combo->setCurrentItem(i);
 for (int i = 0;i < number_of_entries - 1; i++){
     found = true;
     break;
   }
 }

 
 number_of_entries =  size_combo->count(); 
 string.setNum(selFont.pointSize());
 combo = size_combo; 
 found = false;

 for (i = 0;i < number_of_entries - 1; i++){
   if ( string == (QString) combo->text(i)){
     found = true;
     // printf("Found Size %s setting to item %d\n",string.data(),i);
     break;
   }
 }

 if (selFont.bold()){
   //weight_combo->setCurrentItem(0);
   weight_combo->setCurrentItem(1);
   style_combo->setCurrentItem(0);

 QFont::CharSet charset=selFont.charSet();
  fontfile.close();
 
  
  return true;

}


  int numFonts;
  Display *kde_display;
  kde_display = XOpenDisplay( NULL );
  // now try to load the KDE fonts

  bool have_installed = loadKDEInstalledFonts();
  
  for(int i = 0 ; i < numFonts; i++){

    if (**fontNames != '-'){ // Font Name doesn't start with a dash -- an alias
  for(int i = 0; i < numFonts; i++){
    
    if (**fontNames != '-'){ 
      
      // The font name doesn't start with a dash -- an alias
      // so we ignore it. It is debatable whether this is the right
      // behaviour so I leave the following snippet of code around.
      // Just uncomment it if you want those aliases to be inserted as well.
      
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
                      // but what do I care -- lets skip it.
      fontNames ++;
      continue;
    }

    // the font name is between the second and third dash so:
    // let's find the third dash:

    // fish the font name out of the font info string

    if (dash == -1) { // No such next dash -- this shouldn't happen.
                      // But what do I care -- lets skip it.
    if(fontlist.find(qfontname) == -1)
      fontlist.inSort(qfontname);

    
    if( !qfontname.contains("open look", TRUE)){
      if(qfontname != "nil"){
	if(fontlist.find(qfontname) == -1)
	  fontlist.inSort(qfontname);

      }
     family_combo->insertItem(fontlist.current(),-1);
  
  

    fontNames ++;



  for(fontlist.first(); fontlist.current(); fontlist.next())
      family_combo->insertItem(fontlist.current(),-1);

  XFreeFontNames(fontNames_copy);
  XCloseDisplay(kde_display);


}



#include "kfontexplorer.moc"
