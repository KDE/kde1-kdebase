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


#ifndef __KFONTLIST_H__
#define __KFONTLIST_H__

#include <qwidget.h>
#include <qpushbt.h>
#include <qlistbox.h>
#include <qlined.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qmsgbox.h>
#include <qmlined.h>

class KFontList : public QDialog
{
	Q_OBJECT
public:
	KFontList( QWidget *parent,const char* name );
//	~KFontList();

        void resizeEvent(QResizeEvent *e);
protected slots:

          void queryFonts();

protected:

        void readSettings();
	void writeSettings();

public slots:
	  void about();
	  
protected:


        QMultiLineEdit *eframe;
	QLabel *example_label;
	QStrList fontList;
	
	bool changed;
	
	Window root;
	int screen;
};

#endif

