/*
 * titlebar.h
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __KTITLEBARCONFIG_H__
#define __KTITLEBARCONFIG_H__

#include <qframe.h>
#include <qpushbt.h>
#include <qmsgbox.h>
#include <qlabel.h>
#include <qbttngrp.h>
#include <qradiobt.h> 
#include <qpixmap.h>
#include <qgrpbox.h> 
#include <qpainter.h>
#include <qbttngrp.h>
#include <qlcdnum.h>

#include <kslider.h>

#include <kcontrol.h>

#include <kwm.h>

#define NOFUNC     -1
#define MAXIMIZE    0
#define ICONIFY     1
#define CLOSE       2
#define STICKY      3
#define MENU        4

#define NUM_BUTTONS 6

#define TITLEBAR_PLAIN  0
#define TITLEBAR_SHADED 1
#define TITLEBAR_PIXMAP 2

class TitlebarPreview : public QFrame
{
  Q_OBJECT
public:
  TitlebarPreview( QWidget *parent=0, const char *name=0 );
  ~TitlebarPreview( );
  void setA( QPixmap *pm );
  void setB( QPixmap *pm );
  void setC( QPixmap *pm );
  void setD( QPixmap *pm );
  void setE( QPixmap *pm );
  void setF( QPixmap *pm );
  void removeAll(void);
  void setPixmapSize(int x, int y);
protected:
  void paintEvent( QPaintEvent * );
private:
  QLabel *a, *b, *c, *d, *e, *f;
  int xa, xb, xc, xd, xe, xf;
};


class KTitlebarButtons : public KConfigWidget
{
  Q_OBJECT
public:
  KTitlebarButtons( QWidget *parent=0, const char* name=0 );
  ~KTitlebarButtons( );
  void  resizeEvent(QResizeEvent *e);
  void SaveSettings( void );

  void loadSettings();
  void applySettings();
  
protected slots:
    void updatePreview();
    	  
private:
 void GetSettings( void );
 void getStringValue(int, QString *);
 int buttonFunc(QString *);

 int  getFunc(int button);
 void setButton(int button, int func);
 void setState();

 void drawPreview(bool draw);

 QFrame *titlebarFrame;

 TitlebarPreview *blankTitlebar;
 
 QLabel *right, *left, *off;
 QLabel *minB, *maxB, *stickyB, *closeB, *menuB;
 QLabel *minP, *maxP, *stickyP, *closeP, *menuP;
 QButtonGroup *minBox, *maxBox, *stickyBox, *closeBox, *menuBox;
 QRadioButton *minRB[3], *maxRB[3], *stickyRB[3], *closeRB[3], *menuRB[3];

 int label_width, pixmap_width, selection_width;

 // current button->func mapping
 int selectedFunc[NUM_BUTTONS];

 KConfig       *config;
};



class KTitlebarAppearance : public KConfigWidget
{
  Q_OBJECT
public:
  KTitlebarAppearance( QWidget *parent=0, const char* name=0 );
  ~KTitlebarAppearance( );
  void  resizeEvent(QResizeEvent *e);
  void SaveSettings( void );

  void loadSettings();
  void applySettings();
  
  
private slots:

  void titlebarChanged();
  void activePressed();
  void inactivePressed();
  
private:
 void GetSettings( void );

 int getTitlebar( void );
 void setTitlebar(int);

 int getTitleAnim( void );
 void setTitleAnim(int);

 QButtonGroup *titlebarBox;
 QRadioButton *shaded, *plain, *pixmap;
 
 KSlider *titleAnim;
 QLabel *tlabel;
 QLCDNumber *t;
 QLabel *sec;

 QGroupBox *pixmapBox;
 QLabel *lPixmapActive, *lPixmapInactive;
 QPushButton *pbPixmapActive, *pbPixmapInactive;
 QPixmap *pixmapActive, *pixmapInactive;

 QString sPixmapActive, sPixmapInactive; 
 KConfig       *config;
};


#endif

