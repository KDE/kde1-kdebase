/*
 * titlebar.cpp
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

#include <iostream.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <kmsgbox.h> 
#include <kapp.h>
#include <kiconloaderdialog.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>

#include "titlebar.h"

#include "geom.h"

extern KSimpleConfig *config;

// config file keywords used by kwm
#define KWM_TITLEBARLOOK   "TitlebarLook"
#define KWM_TITLEANIMATION "TitleAnimation"

//  buttons 1 2 3 are on left, 4 5 6 on right
#define KWM_B1 "ButtonA"
#define KWM_B2 "ButtonB"
#define KWM_B3 "ButtonC"
#define KWM_B4 "ButtonF"
#define KWM_B5 "ButtonE"
#define KWM_B6 "ButtonD"

// this function grabbed from kwm/client.C
static QPixmap* loadIcon(const char* name)
{
  QPixmap *result = new QPixmap;
  QString fn = "/share/apps/kwm/pics/";
  fn.append(name);
  QString s = KApplication::findFile(fn);
  if (!s.isEmpty())
    result->load(s.data());
  return result;
}

KTitlebarButtons::~KTitlebarButtons ()
{
  delete minB;
  delete maxB;
  delete stickyB;
  delete closeB;
  delete menuB;

  delete minP;
  delete maxP;
  delete stickyP;
  delete closeP;
  delete menuP;

  delete left;
  delete right;
  delete off;

  for (int i=0; i<3; i++)
    {
      delete minRB[i];
      delete maxRB[i];
      delete stickyRB[i];
      delete closeRB[i];
      delete menuRB[i];
    }

  delete minBox;
  delete maxBox;
  delete stickyBox;
  delete closeBox;
  delete menuBox;

  delete blankTitlebar;
  delete titlebarFrame;

}

KTitlebarButtons::KTitlebarButtons (QWidget * parent, const char *name)
  : KConfigWidget (parent, name)
{
  int label_w = 0, pixmap_w = 16, selection_w = 0, i;

  titlebarFrame = new QFrame(this, "testframe");
  titlebarFrame ->setFrameStyle(QFrame::WinPanel | QFrame::Raised );
  blankTitlebar = new TitlebarPreview(titlebarFrame, "blanktbar");

  // button name labels
  minB = new QLabel(klocale->translate("Minimize"), this);
  minB->adjustSize();
  label_w = max(label_w, minB->width());

  maxB = new QLabel(klocale->translate("Maximize"), this);
  maxB->adjustSize();
  label_w = max(label_w, maxB->width());

  stickyB = new QLabel(klocale->translate("Sticky"), this);
  stickyB->adjustSize();
  label_w = max(label_w, stickyB->width());

  closeB = new QLabel(klocale->translate("Close"), this);
  closeB->adjustSize();
  label_w = max(label_w, closeB->width());

  menuB = new QLabel(klocale->translate("Menu"), this);
  menuB->adjustSize();
  label_w = max(label_w, menuB->width());

  // pixmap labels to show which button is which
  QPixmap *pm;
  pm = loadIcon("iconify.xpm");
  minP = new QLabel("", this);
  minP->setPixmap( *pm );
  minP->adjustSize();
  delete pm;

  pm = loadIcon("maximize.xpm");
  maxP = new QLabel("", this);
  maxP->setPixmap( *pm );
  maxP->adjustSize();
  delete pm;

  pm = loadIcon("pinup.xpm");
  stickyP = new QLabel("", this);
  stickyP->setPixmap( *pm );
  stickyP->adjustSize();
  delete pm;

  pm = loadIcon("close.xpm");
  closeP = new QLabel("", this);
  closeP->setPixmap( *pm );
  closeP->adjustSize();
  delete pm;

  pm = loadIcon("menu.xpm");
  menuP = new QLabel("", this);
  menuP->setPixmap( *pm );
  menuP->adjustSize();
  delete pm;

  // left/right/off column labels
  left = new QLabel(klocale->translate("Left"), this);
  left->adjustSize();
  selection_w = max(selection_w, left->width());

  right = new QLabel(klocale->translate("Right"), this);
  right->adjustSize();
  selection_w = max(selection_w, right->width());

  off = new QLabel(klocale->translate("Off"), this);
  off->adjustSize();
  selection_w = max(selection_w, off->width());

  // left/right/off radio buttons and groups
  minBox = new QButtonGroup("", this, NULL);
  minBox->setFrameStyle( QFrame::NoFrame );
  for (i=0; i<3; i++)
    {
      minRB[i] = new QRadioButton("", minBox, NULL);
      minRB[i]->adjustSize();
      connect(minRB[i], SIGNAL(clicked()), this, SLOT(updatePreview()));
    }

  maxBox = new QButtonGroup("", this, NULL);
  maxBox->setFrameStyle( QFrame::NoFrame );
  for (i=0; i<3; i++)
    {
      maxRB[i] = new QRadioButton("", maxBox, NULL);
      maxRB[i]->adjustSize();
      connect(maxRB[i], SIGNAL(clicked()), this, SLOT(updatePreview()));
    }

  stickyBox = new QButtonGroup("", this, NULL);
  stickyBox->setFrameStyle( QFrame::NoFrame );
  for (i=0; i<3; i++)
    {
      stickyRB[i] = new QRadioButton("", stickyBox, NULL);
      stickyRB[i]->adjustSize();
      connect(stickyRB[i], SIGNAL(clicked()), this, SLOT(updatePreview()));
    }

  closeBox = new QButtonGroup("", this, NULL);
  closeBox->setFrameStyle( QFrame::NoFrame );
  for (i=0; i<3; i++)
    {
      closeRB[i] = new QRadioButton("", closeBox, NULL);
      closeRB[i]->adjustSize();
      connect(closeRB[i], SIGNAL(clicked()), this, SLOT(updatePreview()));
    }

  menuBox = new QButtonGroup("", this, NULL);
  menuBox->setFrameStyle( QFrame::NoFrame );
  for (i=0; i<3; i++)
    {
      menuRB[i] = new QRadioButton("", menuBox, NULL);
      menuRB[i]->adjustSize();
      connect(menuRB[i], SIGNAL(clicked()), this, SLOT(updatePreview()));
    }

  label_width = label_w;
  pixmap_width = pixmap_w;
  selection_width = 3*selection_w + 2*SPACE_XO;

  GetSettings();
}

void KTitlebarButtons::resizeEvent(QResizeEvent *)
{
  int h = SPACE_YO;
  int column_h = h + 2*SPACE_YO;   // vertical position of column headings

  h = column_h + left->height() + SPACE_YO;

  // use the previews own setGeometry to do what we mean
  titlebarFrame->setGeometry( SPACE_XO, SPACE_YO, 
			      width() - 2*SPACE_XO, minP->height() + 8 );
  blankTitlebar->setGeometry( 4, 4, 
			      width() - 2*SPACE_XO - 8, minP->height() );
  blankTitlebar->setPixmapSize( minP->width(), minP->height() );

  minB->move(SPACE_XO, h);
  minP->move(label_width + 2*SPACE_XO, h);

  int i, c, xs[3];
  int boxW = selection_width;
  int boxH = minP->height();
  int boxX = minP->x() + minP->width() + SPACE_XO;
  float frac;

  for (i=0; i<3; i++)
    {
      frac = (i/3.0 + 1.0/6.0);
      xs[i] = (int) ((float) frac*selection_width);
    }
  c = minRB[0]->width()/3;   // why 3?
  left->move(  boxX + c + xs[0] - left->width()/2,  column_h);
  right->move( boxX + c + xs[1] - right->width()/2, column_h);
  off->move(   boxX + c + xs[2] - off->width()/2,   column_h);

  // now we are ready to position everything
  minBox->resize(boxW, boxH);
  minBox->move(minP->x() + minP->width() + SPACE_XO, h);
  for (i=0; i<3; i++)
    minRB[i]->move( xs[i], 0);

  h += SPACE_YO + minB->height();

  maxB->move(SPACE_XO, h);
  maxP->move(label_width + 2*SPACE_XO, h);
  maxBox->resize(boxW, boxH);
  maxBox->move(maxP->x() + maxP->width() + SPACE_XO, h);
  for (i=0; i<3; i++)
    maxRB[i]->move( xs[i], 0);
  h += SPACE_YO + maxB->height();

  stickyB->move(SPACE_XO, h);
  stickyP->move(label_width + 2*SPACE_XO, h);
  stickyBox->resize(boxW, boxH);
  stickyBox->move(stickyP->x() + stickyP->width() + SPACE_XO, h);
  for (i=0; i<3; i++)
    stickyRB[i]->move( xs[i], 0);
  h += SPACE_YO + stickyB->height();

  closeB->move(SPACE_XO, h);
  closeP->move(label_width + 2*SPACE_XO, h);
  closeBox->resize(boxW, boxH);
  closeBox->move(closeP->x() + closeP->width() + SPACE_XO, h);
  for (i=0; i<3; i++)
    closeRB[i]->move( xs[i], 0);
  h += SPACE_YO + closeB->height();

  menuB->move(SPACE_XO, h);
  menuP->move(label_width + 2*SPACE_XO, h);
  menuBox->resize(boxW, boxH);
  menuBox->move(menuP->x() + menuP->width() + SPACE_XO, h);
  for (i=0; i<3; i++)
    menuRB[i]->move( xs[i], 0);
  h += SPACE_YO + menuB->height();

  drawPreview(TRUE);
}

void KTitlebarButtons::updatePreview()
{
  drawPreview(TRUE);
}

void KTitlebarButtons::drawPreview(bool draw)
{
  int left = 0, right = 0;
  QPixmap *p = closeP->pixmap();
  
  for (int i=0; i<NUM_BUTTONS; i++)
    selectedFunc[i] = NOFUNC;

  blankTitlebar->removeAll();

  // place the highest priority pixmaps first

  // menu can only go at the edge: A or F
  if (menuRB[0]->isChecked())
    {
      if (draw)
	{
	  p = menuP->pixmap();
	  blankTitlebar->setA( p );
	}
      selectedFunc[0] = MENU;
      left++;
    }
  else if (menuRB[1]->isChecked())
    {
      if (draw)
	{
	  p = menuP->pixmap();
	  blankTitlebar->setF( p );
	}
      selectedFunc[5] = MENU;
      right++;
    }
  else 
    {
      menuRB[2]->setChecked(TRUE);
    }

  // close can go in A, B, E, or F
  if (closeRB[0]->isChecked())
    {
      if (draw) p = closeP->pixmap();
      if (left == 0)
	{
	  if (draw) blankTitlebar->setA( p );
	  selectedFunc[0] = CLOSE;
	}
      else
	{
	  if (draw) blankTitlebar->setB( p );
	  selectedFunc[1] = CLOSE;
	}
      left++;
    }
  else if (closeRB[1]->isChecked())
    {
      if (draw) p = closeP->pixmap();
      if (right == 0)
	{
	  if (draw) blankTitlebar->setF( p );
	  selectedFunc[5] = CLOSE;
	}
      else
	{
	  if (draw) blankTitlebar->setE( p );
	  selectedFunc[4] = CLOSE;
	}
      right++;
    }
  else 
    {
      // make sure it is OFF
      closeRB[2]->setChecked(TRUE);
    }


  // sticky can go anywhere but always fits
  if (stickyRB[0]->isChecked())
    {
      if (draw) p = stickyP->pixmap();
      if (left == 0)
	{
	  if (draw) blankTitlebar->setA( p );
	  selectedFunc[0] = STICKY;
	}
      else if (left == 1)
	{
	  if (draw) blankTitlebar->setB( p );
	  selectedFunc[1] = STICKY;
	}
      else
	{
	  if (draw) blankTitlebar->setC( p );
	  selectedFunc[2] = STICKY;
	}
      left++;
    }
  else if (stickyRB[1]->isChecked())
    {
      if (draw) p = stickyP->pixmap();
      if (right == 0)
	{
	  if (draw) blankTitlebar->setF( p );
	  selectedFunc[5] = STICKY;
	}
      else if (right == 1)
	{
	  if (draw) blankTitlebar->setE( p );
	  selectedFunc[4] = STICKY;
	}
      else
	{
	  if (draw) blankTitlebar->setD( p );
	  selectedFunc[3] = STICKY;
	}
      right++;
    }
  else 
    {
      // make sure this func is OFF
      stickyRB[2]->setChecked(TRUE);
    }

  // max may not fit is the selected side is full already
  if (maxRB[0]->isChecked())
    {
      if (draw) p = maxP->pixmap();
      if (left == 0)
	{
	  if (draw) blankTitlebar->setA( p );
	  selectedFunc[0] = MAXIMIZE;
	}
      else if (left == 1)
	{
	  if (draw) blankTitlebar->setB( p );
	  selectedFunc[1] = MAXIMIZE;
	}
      else if (left == 2)
	{
	  if (draw) blankTitlebar->setC( p );
	  selectedFunc[2] = MAXIMIZE;
	}
      else
	{

	  // can't place max on left
	  KMsgBox::message(this,klocale->translate("Warning"),
			   klocale->translate("The left side of the titlebar "
					      "is full... disabling the 'maximise' "
					      "button\n"), 
			   KMsgBox::EXCLAMATION,
			   klocale->translate("Close") );
	  maxRB[0]->setChecked(FALSE);
	  maxRB[2]->setChecked(TRUE);
	  left--;
	}
      left++;
    }
  else if (maxRB[1]->isChecked())
    {
      if (draw) p = maxP->pixmap();
      if (right == 0)
	{
	 if (draw)  blankTitlebar->setF( p );
	  selectedFunc[5] = MAXIMIZE;
	}
      else if (right == 1)
	{
	  if (draw) blankTitlebar->setE( p );
	  selectedFunc[4] = MAXIMIZE;
	}
      else if (right == 2)
	{
	  if (draw) blankTitlebar->setD( p );
	  selectedFunc[3] = MAXIMIZE;
	}
      else
	{
	  // can't place max on right
	  KMsgBox::message(this,klocale->translate("Warning"),
			   klocale->translate("The right side of the titlebar "
					      "is full... disabling the 'maximise' "
					      "button\n"), 
			   KMsgBox::EXCLAMATION,
			   klocale->translate("Close") );
	  maxRB[1]->setChecked(FALSE);
	  maxRB[2]->setChecked(TRUE);
	  right--;
	}
      right++;
    }
  else 
    {
      // make sure this func is OFF
      maxRB[2]->setChecked(TRUE);
    }

  // min may not fit is the selected side is full already
  if (minRB[0]->isChecked())
    {
      if (draw) p = minP->pixmap();
      if (left == 0)
	{
	  if (draw) blankTitlebar->setA( p );
	  selectedFunc[0] = ICONIFY;
	}
      else if (left == 1)
	{
	  if (draw) blankTitlebar->setB( p );
	  selectedFunc[1] = ICONIFY;
	}
      else if (left == 2)
	{
	  if (draw) blankTitlebar->setC( p );
	  selectedFunc[2] = ICONIFY;
	}
      else
	{
	  // left side is full
	  KMsgBox::message(this,klocale->translate("Warning"),
			   klocale->translate("The left side of the titlebar "
					      "is full... disabling the 'minimise' "
					      "button\n"), 
			   KMsgBox::EXCLAMATION,
			   klocale->translate("Close") );
	  minRB[0]->setChecked(FALSE);
	  minRB[2]->setChecked(TRUE);
	  left--;
	}
      left++;
    }
  else if (minRB[1]->isChecked())
    {
      if (draw) p = minP->pixmap();
      if (right == 0)
	{
	  if (draw) blankTitlebar->setF( p );
	  selectedFunc[5] = ICONIFY;
	}
      else if (right == 1)
	{
	  if (draw) blankTitlebar->setE( p );
	  selectedFunc[4] = ICONIFY;
	}
      else if (right == 2)
	{
	  if (draw) blankTitlebar->setD( p );
	  selectedFunc[3] = ICONIFY;
	}
      else
	{
	  // can't place min on right
	  KMsgBox::message(this,klocale->translate("Warning"),
			   klocale->translate("The right side of the titlebar "
					      "is full... disabling the 'minimise' "
					      "button\n"), 
			   KMsgBox::EXCLAMATION,
			   klocale->translate("Close") );
	  minRB[1]->setChecked(FALSE);
	  minRB[2]->setChecked(TRUE);
	  right--;
	}
      right++;
    }
  else 
    {
      // make sure it is OFF
      minRB[2]->setChecked(TRUE);
    }
}

int KTitlebarButtons::getFunc(int button)
{
  return selectedFunc[button];
}

void KTitlebarButtons::setButton(int button, int func)
{
  // if button < 3, the func button goes on the left side
  // otherwise, the func button goes on the right side

  switch (func)
    {
    case ICONIFY:
      if (button < 3)
      {
	minRB[0]->setChecked(TRUE);  
        minRB[1]->setChecked(FALSE);
      }
      else
      {
	minRB[1]->setChecked(TRUE);  
        minRB[0]->setChecked(FALSE);
      }
      break;
    case MAXIMIZE:
      if (button < 3)
      {
	maxRB[0]->setChecked(TRUE);  
        maxRB[1]->setChecked(FALSE);
      }
      else
      {
	maxRB[1]->setChecked(TRUE);  
        maxRB[0]->setChecked(FALSE);
      }
      break;
    case STICKY:
      if (button < 3)
      {
	stickyRB[0]->setChecked(TRUE);
        stickyRB[1]->setChecked(FALSE);
      }
      else
      {
	stickyRB[1]->setChecked(TRUE);
        stickyRB[0]->setChecked(FALSE);
      }
      break;
    case CLOSE:
      if (button < 3)
      {
	closeRB[0]->setChecked(TRUE); 
        closeRB[1]->setChecked(FALSE);
      }
      else
      {
	closeRB[1]->setChecked(TRUE); 
        closeRB[0]->setChecked(FALSE);
      }
      break;
    case MENU:
      if (button < 3)
      {
	menuRB[0]->setChecked(TRUE);  
        menuRB[1]->setChecked(FALSE);
      }
      else
      {
	menuRB[1]->setChecked(TRUE);  
        menuRB[0]->setChecked(FALSE);
      }
      break;
    }
}

void KTitlebarButtons::setState()
{
  drawPreview(FALSE);
}

void KTitlebarButtons::getStringValue(int b, QString *str)
{
  switch (b)
    {
    case MENU:
      *str = "Menu";
      break;
    case STICKY:
      *str = "Sticky";
      break;
    case CLOSE:
      *str = "Close";
      break;
    case MAXIMIZE:
      *str = "Maximize";
      break;
    case ICONIFY:
      *str = "Iconify";
      break;
    case NOFUNC:
      *str = "Off";
      break;
    }
}

void KTitlebarButtons::SaveSettings( void )
{
  config->setGroup( "Buttons");

  QString str;
  int b;

  b = getFunc(0);
  getStringValue(b, &str);
  config->writeEntry(KWM_B1, str);

  b = getFunc(1);
  getStringValue(b, &str);
  config->writeEntry(KWM_B2, str);

  b = getFunc(2);
  getStringValue(b, &str);
  config->writeEntry(KWM_B3, str);

  b = getFunc(3);
  getStringValue(b, &str);
  config->writeEntry(KWM_B4, str);

  b = getFunc(4);
  getStringValue(b, &str);
  config->writeEntry(KWM_B5, str);

  b = getFunc(5);
  getStringValue(b, &str);
  config->writeEntry(KWM_B6, str);

  config->sync();

  // tell kwm to re-parse the config file
  //system("kwmclient configure");
  KWM::configureWm();
}

int KTitlebarButtons::buttonFunc(QString *key)
{
  int ret = NOFUNC;

  if( *key == "Off" )
    ret = NOFUNC;
  else if( *key == "Maximize" )
    ret = MAXIMIZE;
  else if( *key == "Iconify" )
    ret = ICONIFY;
  else if( *key == "Close" )
    ret = CLOSE;
  else if( *key == "Sticky" )
    ret = STICKY;
  else if (*key == "Menu" )
    ret = MENU;
  
  return ret;
}

void KTitlebarButtons::GetSettings( void )
{
  QString key;

  config->setGroup( "Buttons");
  int ABUTTON=0, BBUTTON=0, CBUTTON=0, DBUTTON=0, EBUTTON=0, FBUTTON=0;
  
  key = config->readEntry(KWM_B1);
  ABUTTON = buttonFunc(&key);

  key = config->readEntry(KWM_B2);
  BBUTTON = buttonFunc(&key);

  key = config->readEntry(KWM_B3);
  CBUTTON = buttonFunc(&key);

  key = config->readEntry(KWM_B4);
  DBUTTON = buttonFunc(&key);

  key = config->readEntry(KWM_B5);
  EBUTTON = buttonFunc(&key);

  key = config->readEntry(KWM_B6);
  FBUTTON = buttonFunc(&key);

  // clear all buttons (for reloading!)
  minRB[0]->setChecked(FALSE);  
  minRB[1]->setChecked(FALSE);
  maxRB[0]->setChecked(FALSE);  
  maxRB[1]->setChecked(FALSE);
  stickyRB[0]->setChecked(FALSE);
  stickyRB[1]->setChecked(FALSE);
  closeRB[0]->setChecked(FALSE); 
  closeRB[1]->setChecked(FALSE);
  menuRB[0]->setChecked(FALSE);  
  menuRB[1]->setChecked(FALSE);

  setButton(0, ABUTTON);
  setButton(1, BBUTTON);
  setButton(2, CBUTTON);
  setButton(3, DBUTTON);
  setButton(4, EBUTTON);
  setButton(5, FBUTTON);
  setState();
}

void KTitlebarButtons::loadSettings()
{
  GetSettings();
  drawPreview(TRUE);
}

void KTitlebarButtons::applySettings()
{
  SaveSettings();
}


// titlebar preview code
TitlebarPreview::~TitlebarPreview( )
{
  delete a;
  delete b;
  delete c;
  delete d;
  delete e;
  delete f;
}

TitlebarPreview::TitlebarPreview( QWidget *parent, const char *name )
        : QFrame( parent, name )
{
  a = new QLabel("", this, "a", 0);
  a->hide();
  b = new QLabel("", this, "b", 0);
  b->hide();
  c = new QLabel("", this, "c", 0);
  c->hide();
  d = new QLabel("", this, "d", 0);
  d->hide();
  e = new QLabel("", this, "e", 0);
  e->hide();
  f = new QLabel("", this, "f", 0);
  f->hide();
  setBackgroundColor( QColor( 0, 10, 160 ) );
}

void TitlebarPreview::setPixmapSize(int w, int /*unused*/ )
{
  xa = 0;
  xb = w;
  xc = 2*w;
  xd = width() - 3*w;
  xe = width() - 2*w;
  xf = width() - w;
}

void TitlebarPreview::setA( QPixmap *pm )
{
  a->setPixmap( *pm );
  a->adjustSize();
  a->show();
}
void TitlebarPreview::setB( QPixmap *pm )
{
  b->setPixmap( *pm );
  b->adjustSize();
  b->show();
}
void TitlebarPreview::setC( QPixmap *pm )
{
  c->setPixmap( *pm );
  c->adjustSize();
  c->show();
}
void TitlebarPreview::setD( QPixmap *pm )
{
  d->setPixmap( *pm );
  d->adjustSize();
  d->show();
}
void TitlebarPreview::setE( QPixmap *pm )
{
  e->setPixmap( *pm );
  e->adjustSize();
  e->show();
}
void TitlebarPreview::setF( QPixmap *pm )
{
  f->setPixmap( *pm );
  f->adjustSize();
  f->show();
}
void TitlebarPreview::removeAll( void )
{
  a->hide();
  b->hide();
  c->hide();
  d->hide();
  e->hide();
  f->hide();
}

void TitlebarPreview::paintEvent( QPaintEvent * )
{
  a->move(xa, 0);
  b->move(xb, 0);
  c->move(xc, 0);
  d->move(xd, 0);
  e->move(xe, 0);
  f->move(xf, 0);
}

// appearance dialog
KTitlebarAppearance::~KTitlebarAppearance ()
{
  delete shaded;
  delete plain;
  delete titlebarBox;

  delete titleAnim;
  delete tlabel;
  delete t;
  delete sec;
}

KTitlebarAppearance::KTitlebarAppearance (QWidget * parent, const char *name)
  : KConfigWidget (parent, name)
{
  // titlebar shading style
  titlebarBox = new QButtonGroup(klocale->translate("Titlebar appearance"), 
				 this);
  shaded = new QRadioButton(klocale->translate("Shaded"), 
			    titlebarBox);
  connect(shaded, SIGNAL(clicked()), this, SLOT(titlebarChanged()));
  plain = new QRadioButton(klocale->translate("Plain"), 
			   titlebarBox);
  connect(plain, SIGNAL(clicked()), this, SLOT(titlebarChanged()));
  pixmap = new QRadioButton(klocale->translate("Pixmap"),
	                   titlebarBox);
  connect(pixmap, SIGNAL(clicked()), this, SLOT(titlebarChanged()));

  // titlebar animation
  titleAnim = new KSlider(0,100,10,100, KSlider::Horizontal, this);
  titleAnim->setSteps(10,10);
  tlabel = new QLabel(klocale->translate("Title Animation"), this);
  t = new QLCDNumber (2, this);
  t->setFrameStyle( QFrame::NoFrame );
  sec = new QLabel(klocale->translate("ms"), this);
  connect( titleAnim,   SIGNAL(valueChanged(int)), t, SLOT(display(int)) );

  shaded->adjustSize();
  plain->adjustSize();
  pixmap->adjustSize();
  t->adjustSize();
  tlabel->adjustSize();
  sec->adjustSize();

  pixmapBox    = new QGroupBox(klocale->translate("Pixmap"), this); 

  pixmapActive = pixmapInactive = 0;

  pbPixmapActive = new QPushButton(pixmapBox);
  pbPixmapInactive = new QPushButton(pixmapBox);
  pbPixmapActive->resize(32,32);
  pbPixmapInactive->resize(32,32);
  connect(pbPixmapActive, SIGNAL(clicked()), this, SLOT(activePressed()));
  connect(pbPixmapInactive, SIGNAL(clicked()), this, SLOT(inactivePressed()));

  lPixmapActive = new QLabel(pbPixmapActive, klocale->translate("&Active pixmap"), pixmapBox);
  lPixmapActive->adjustSize(); 
  lPixmapInactive = new QLabel(pbPixmapInactive, klocale->translate("&Inactive pixmap"), pixmapBox);
  lPixmapInactive->adjustSize();

  GetSettings();
}

void KTitlebarAppearance::resizeEvent(QResizeEvent *)
{
  int h = SPACE_YO;

  QFontMetrics fm = titlebarBox->font();
  int titleH = fm.height();
  int titleW = fm.width( titlebarBox->title() );
  int buttonH = shaded->height();
  int buttonW = max( shaded->width(), plain->width() );
  buttonW = max( buttonW, pixmap->width() );
  int boxH = 0;
  int boxW = 0;
  boxH = 3*buttonH + 4*SPACE_YI + titleH;
  if (boxW < titleW + 4*SPACE_XI)
    boxW =  titleW + 4*SPACE_XI;
  if (boxW < buttonW + 2*SPACE_XI)
    boxW = buttonW + 2*SPACE_XI;

  shaded->move(SPACE_XI, SPACE_YI + titleH);
  plain->move(SPACE_XI, 2*SPACE_YI + buttonH + titleH);
  pixmap->move(SPACE_XI, 3*SPACE_YI + 2 * buttonH + titleH);
  titlebarBox->setGeometry(SPACE_XO, h, boxW, boxH);
  pixmapBox->setGeometry(2*SPACE_XO+boxW, h, boxW, boxH);

  h += boxH + SPACE_YO;

  int dw = SPACE_XO;

  int w = tlabel->width();
  tlabel->move(dw, h);
  titleAnim->setGeometry(dw + w + 2*SPACE_XO, h, 200, tlabel->height()+SPACE_YO/2);
  h += titleAnim->height() ;
  int center = titleAnim->x() + ( titleAnim->width() - t->width() )/2;
  int dh = ( t->height() - sec->height() )/2;
  t->move(center, h);
  sec->move(t->x() + t->width() + 3, h+dh);
  h += t->height() + SPACE_YO;    

  w = max(lPixmapActive->width(), lPixmapInactive->width());
  lPixmapActive->move(SPACE_XI, SPACE_YI + titleH);
  lPixmapInactive->move(SPACE_XI, 2*SPACE_YI + titleH + pbPixmapActive->height());
  pbPixmapActive->move(SPACE_XI+w+SPACE_XO, + titleH);
  pbPixmapInactive->move(SPACE_XI+w+SPACE_XO, SPACE_YI + titleH + pbPixmapActive->height());
  pixmapBox->resize(2*SPACE_XI+w+SPACE_XO+pbPixmapActive->width(), pixmapBox->height());

  h += 2*lPixmapActive->height() + 2*SPACE_YI;
}

int KTitlebarAppearance::getTitlebar()
{
  if (shaded->isChecked())
    return TITLEBAR_SHADED;
  else
    if (pixmap->isChecked())
      return TITLEBAR_PIXMAP;
    else
      return TITLEBAR_PLAIN;
}

void KTitlebarAppearance::setTitlebar(int tb)
{
  if (tb == TITLEBAR_PIXMAP)
  {
    plain->setChecked(FALSE);
    shaded->setChecked(FALSE);
    pixmap->setChecked(TRUE);
    pixmapBox->show();
    return;
  }
  if (tb == TITLEBAR_SHADED)
  {
    shaded->setChecked(TRUE);
    plain->setChecked(FALSE);
    pixmap->setChecked(FALSE);
    pixmapBox->hide();
    return;
  }
  plain->setChecked(TRUE);
  shaded->setChecked(FALSE);
  pixmap->setChecked(FALSE);
  pixmapBox->hide();
}
 
int KTitlebarAppearance::getTitleAnim()
{
  return t->intValue();
}
void KTitlebarAppearance::setTitleAnim(int tb)
{
  titleAnim->setValue(tb);
  t->display(tb);
}
 
void KTitlebarAppearance::SaveSettings( void )
{
  config->setGroup( "General" );
  int t = getTitlebar();
  if (t == TITLEBAR_SHADED)
    config->writeEntry(KWM_TITLEBARLOOK, "shaded");
  else
    if (t == TITLEBAR_PIXMAP)
      config->writeEntry(KWM_TITLEBARLOOK, "pixmap");
    else
      config->writeEntry(KWM_TITLEBARLOOK, "plain");

  config->writeEntry("TitlebarPixmapActive", sPixmapActive);
  config->writeEntry("TitlebarPixmapInactive", sPixmapInactive);

  int a = getTitleAnim();
  config->writeEntry(KWM_TITLEANIMATION, a);
   
  config->sync();

  // tell kwm to re-parse the config file
  KWM::configureWm();
}

void KTitlebarAppearance::GetSettings( void )
{
  KIconLoader iconLoader;
  QString key;

  config->setGroup( "General" );

  key = config->readEntry("TitlebarLook");
  if( key == "shaded")
    setTitlebar(TITLEBAR_SHADED);
  else if( key == "pixmap")
    setTitlebar(TITLEBAR_PIXMAP);
  else
    setTitlebar(TITLEBAR_PLAIN);

  sPixmapActive = config->readEntry("TitlebarPixmapActive");
  sPixmapInactive = config->readEntry("TitlebarPixmapInactive");
  if (!sPixmapActive.isEmpty())
    pbPixmapActive->setPixmap(iconLoader.loadIcon(sPixmapActive));
  if (!sPixmapInactive.isEmpty())
    pbPixmapInactive->setPixmap(iconLoader.loadIcon(sPixmapInactive));


  int k = config->readNumEntry(KWM_TITLEANIMATION,0);
  setTitleAnim(k);
}

void KTitlebarAppearance::loadSettings()
{
  GetSettings();
}

void KTitlebarAppearance::applySettings()
{
  SaveSettings();
}


void KTitlebarAppearance::titlebarChanged()
{
  setTitlebar(getTitlebar());
}


void KTitlebarAppearance::activePressed()
{
  KIconLoaderDialog dlg(this);
  QString name = sPixmapActive;
  QPixmap map;

  map = dlg.selectIcon(name, "*");
  if (!name.isEmpty())
    {
      sPixmapActive = name;
      pbPixmapActive->setPixmap(map);
    }
}


void KTitlebarAppearance::inactivePressed()
{
  KIconLoaderDialog dlg(this);
  QString name = sPixmapInactive;
  QPixmap map;

  map = dlg.selectIcon(name, "*");
  if (!name.isEmpty())
    {
      sPixmapInactive = name;
      pbPixmapInactive->setPixmap(map);
    }
}


#include "titlebar.moc"

