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
    Revision 1.3  1997/11/09 04:05:01  wuebben
    Bernd: necessary iso charset changes

    Revision 1.2  1997/05/22 13:30:52  kulow
    Coolo: updated kedit to 0.5
    updated kfontmanager to 0.2.2

    Revision 1.1  1997/05/21 04:47:09  wuebben
    Initial revision

    Revision 1.1  1997/04/26 20:33:39  wuebben
    Initial revision

    Revision 1.3  1997/04/20 14:59:45  wuebben
    fixed a minor bug which caused the last font in the font list to not
    be displayed

    Revision 1.1  1997/04/20 00:18:15  wuebben
    Initial revision

    Revision 1.2  1997/03/02 22:40:59  wuebben
    *** empty log message ***

    Revision 1.1  1997/01/04 17:36:44  wuebben
    Initial revision


*/


#ifndef _K_FONT_EXPLORER_H_
#define _K_FONT_EXPLORER_H_

#include <qmsgbox.h>
#include <qpixmap.h>
#include <qapp.h>
#include <qframe.h> 
#include <qbttngrp.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qframe.h>
#include <qgrpbox.h>
#include <qlabel.h>
#include <qlined.h>
#include <qlistbox.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qscrbar.h>
#include <qtooltip.h>

#include <qstring.h>
#include <qfont.h>


class KFontExplorer : public QDialog {

    Q_OBJECT

      // Usage of the KFontDialog Class:
      //
      // Case 1) The pointer fontlist is null ( Recommended Usage !!)
      // 
      // In this case KFontDialog will first search
      // for ~/.kde/config/kdefonts. If kdefonts if available
      // it will insert the fonts listed there into the family combo.
      // 
      // Note: ~/.kde/config/kdefonts is managed by kfontmanager. 
      // ~/.kde/config/kdefonts is a newline separated list of font names.
      // Such as: time\nhelvetica\nfixed\n etc.You should however not 
      // manipulate that list -- that is the job of kfontmanager.
      // 
      // If ~/.kde/config/kdefonts doesn't exist, KFontDialog will query
      // the X server and insert all availabe fonts.
      //
      // Case 2) The pointer fontlist is non null. In this cae KFontDialog 
      // will insert the strings of that QStrList into the family combo.
      // 
      // Note: Due to a bug in Qt 1.2 you must 
      // supply at this point at least two fonts in the QStrList that
      // fontlist points to. The bug has been reported and will hopefully
      // be fixed in Qt.1.3. 


public:
    KFontExplorer( QWidget *parent = 0L, const char *name = 0L,
			bool modal = FALSE, const QStrList* fontlist = 0L );

    void setFont( const QFont &font );
    QFont font()	{  return selFont; }

    /*
     * This is probably the function you are looking for.
     * Just call this to pop up a dialog to get the selected font.
     * returns result().
     */

    static int getFont( QFont &theFont );

signals:
	/*
	 * connect to this to monitor the font as it as selected if you are
	 * not running modal.
	 */
	void fontSelected( const QFont &font );

private slots:

      void 	family_chosen_slot(const char* );
      void      size_chosen_slot(const char* );
      void      weight_chosen_slot(const char*);
      void      style_chosen_slot(const char*);
      void      display_example(const QFont &font);
      void      charset_chosen_slot(int index);
      void 	setColors();

private:

    bool loadKDEInstalledFonts();
    void fill_family_combo();
    void setCombos();
   
    QGroupBox	 *box1;
    QGroupBox	 *box2;
    
    // pointer to an optinally supplied list of fonts to 
    // inserted into the fontdialog font-family combo-box
    QStrList     *fontlist; 

    QLabel	 *family_label;
    QLabel	 *size_label;
    QLabel       *weight_label;
    QLabel       *style_label;
    QLabel	 *charset_label;

    QLabel	 *actual_family_label;
    QLabel	 *actual_size_label;
    QLabel       *actual_weight_label;
    QLabel       *actual_style_label;
    QLabel	 *actual_charset_label;


    QLabel	 *actual_family_label_data;
    QLabel	 *actual_size_label_data;
    QLabel       *actual_weight_label_data;
    QLabel       *actual_style_label_data;
    QLabel	 *actual_charset_label_data;
    QComboBox    *family_combo;
    QComboBox    *size_combo;
    QComboBox    *weight_combo;
    QComboBox    *style_combo;
    QComboBox	 *charset_combo;    
    QPushButton	 *ok_button;
    QPushButton	 *cancel_button;

    QLabel       *example_label;
    QFont         selFont;

};


#endif

