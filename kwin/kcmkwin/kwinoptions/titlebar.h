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
#include <qcombo.h> //CT 11feb98
#include <qchkbox.h> //CT 02Dec1998

#include <kslider.h>

#include <kcontrol.h>
#include <kiconloader.h>

#include <kwm.h>

#define NOFUNC     -1
#define MAXIMIZE    0
#define ICONIFY     1
#define CLOSE       2
#define STICKY      3
#define MENU        4

#define NUM_BUTTONS 6

//CT 06Nov1998
#define AT_LEFT                 0
#define AT_MIDDLE               1
#define AT_RIGHT                2

#define TITLEBAR_PLAIN                0
#define TITLEBAR_SHADED_VERT          1
#define TITLEBAR_SHADED_HORIZ         2
#define TITLEBAR_PIXMAP               3

//CT 11feb98; 23Oct1998
#define DCTB_MAXIMIZE      0
#define DCTB_SHADE         1
#define DCTB_ICONIFY       2
#define DCTB_STICKY        3
//#define DCTB_MOVE          4
//#define DCTB_RESIZE        5
//#define DCTB_RESTORE       6
//#define DCTB_OPERATIONS    7
#define DCTB_CLOSE         4

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
};



class KTitlebarAppearance : public KConfigWidget
{
  Q_OBJECT
public:
  KTitlebarAppearance( QWidget *parent=0, const char* name=0 );
  ~KTitlebarAppearance( );
  //CT  void  resizeEvent(QResizeEvent *e);
  void SaveSettings( void );

  void loadSettings();
  void applySettings();


private slots:

  void titlebarChanged();
  void activePressed();
  void inactivePressed();

private:
 void GetSettings( void );

 //CT 02Dec1998 macStyle
 bool getFramedTitle(void);
 void setFramedTitle(bool);
 bool getPixedText(void);
 void setPixedText(bool);

 //CT 06Nov1998
 int getAlign(void);
 void setAlign(int);
 //CT

 int getTitlebar( void );
 void setTitlebar(int);

 int getTitleAnim( void );
 void setTitleAnim(int);

 //CT 02Dec1998 macStyle
 QCheckBox *cbFrame, *cbPixedText;
 QGroupBox *appearBox;

 //CT 06Nov1998
 QButtonGroup *alignBox;
 QRadioButton *leftAlign, *midAlign, *rightAlign;
 //CT
 QButtonGroup *titlebarBox;
 QRadioButton *vShaded, *hShaded, *plain, *pixmap;

 //CT 11feb98
 int getDCTBAction();
 void setDCTBAction(int);

 QGroupBox *titlebarDblClickBox;
 QLabel * lDblClick;
 QComboBox * dblClickCombo;
 //CT ---

 QGroupBox *animBox; //CT 21Oct1998
 KSlider *titleAnim;
 //CT  QLabel *tlabel;
 QLCDNumber *t;
 QLabel *sec;

 QGroupBox *pixmapBox;
 QLabel *lPixmapActive, *lPixmapInactive;
 QPushButton *pbPixmapActive, *pbPixmapInactive;
 QPixmap pixmapActive, pixmapInactive, pixmapActiveOld, pixmapInactiveOld;

 QString sPixmapActive, sPixmapInactive;

 KIconLoader *iconLoader;
};


#endif

