/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#include <qframe.h>
#include <qlayout.h>
#include <qdragobject.h>
#include <kpixmap.h>
#include "utils.h"
#include "kdropsite.h"
#include "kdm-bgnd.moc"
#include <kfiledialog.h>

void KBGMonitor::setAllowDrop(bool a)
{
  if(a == allowdrop)
    return;
  allowdrop = a;

  QPainter p;
  p.begin( this );
  p.setRasterOp (NotROP);
  p.drawRect(0, 0, width(), height() );
  p.end();
}

// Destructor
KDMBackgroundWidget::~KDMBackgroundWidget()
{
  if(gui)
  {
  }
}


KDMBackgroundWidget::KDMBackgroundWidget(QWidget *parent, const char *name, bool init)
  : KConfigWidget(parent, name)
{
      gui = !init;
      loadSettings();
      if(gui)
        setupPage(parent);
}

void KDMBackgroundWidget::setupPage(QWidget *)
{
      QLabel *label;
      QGroupBox *tGroup, *lGroup, *rGroup;
      QRadioButton *rb;

      QString s = kapp->kde_datadir();
      s += "/kdmconfig/pics/monitor.xpm";
      QPixmap p(s.data()); // = iconloader->loadIcon("monitor.xpm");

      tGroup = new QGroupBox( klocale->translate("Preview"), this );

      label = new QLabel( tGroup );
      label->setAlignment( AlignCenter );
      label->setPixmap( p );
      label->setFixedSize(label->sizeHint());

      monitor = new KBGMonitor( label );
      monitor->setGeometry( 20, 10, 157, 111 );

      QBoxLayout *tLayout = new QVBoxLayout(tGroup, 10, 10, "tLayout");
      tLayout->addSpacing(tGroup->fontMetrics().height()/2);
      tLayout->addWidget(label, 1, AlignCenter);
      tLayout->activate();

      KDropSite *dropsite = new KDropSite( monitor );
      connect( dropsite, SIGNAL( dropAction( QDropEvent*) ), 
        this, SLOT( slotQDrop( QDropEvent*) ) );

      connect( dropsite, SIGNAL( dragLeave( QDragLeaveEvent*) ), 
        this, SLOT( slotQDragLeave( QDragLeaveEvent*) ) );

      connect( dropsite, SIGNAL( dragEnter( QDragEnterEvent*) ), 
        this, SLOT( slotQDragEnter( QDragEnterEvent*) ) );

      lGroup = new QGroupBox( klocale->translate("Color"), this );
      cGroup = new QButtonGroup( lGroup );
      cGroup->setFrameStyle(QFrame::NoFrame);
      cGroup->setExclusive( TRUE );
      QBoxLayout *cl = new QVBoxLayout(cGroup, 10, 10, "cl");

      QRadioButton *crb1 = new QRadioButton( klocale->translate("Solid Color"), cGroup );
      crb1->setFixedSize( crb1->sizeHint());
      cGroup->insert( crb1, Plain );
      cl->addWidget(crb1, 0, AlignLeft);

      QRadioButton *crb2 = new QRadioButton( klocale->translate("Horizontal Blend"), cGroup );
      crb2->setFixedSize( crb2->sizeHint());
      cGroup->insert( crb2, Horizontal );
      cl->addWidget(crb2, 0, AlignLeft);

      QRadioButton *crb3 = new QRadioButton( klocale->translate("Vertical Blend"), cGroup );
      crb3->setFixedSize( crb3->sizeHint());
      cGroup->insert( crb3, Vertical );
      cl->addWidget(crb3, 0, AlignLeft);

      connect( cGroup, SIGNAL( clicked( int ) ), SLOT( slotColorMode( int ) ) );

      cl->activate();

      colButton1 = new KColorButton( color1, lGroup );
      colButton1->setFixedSize( colButton1->sizeHint());
      connect( colButton1, SIGNAL( changed( const QColor & ) ),
		SLOT( slotSelectColor1( const QColor & ) ) );

      colButton2 = new KColorButton( color1, lGroup );
      colButton2->setFixedSize( colButton2->sizeHint());
      connect( colButton2, SIGNAL( changed( const QColor & ) ),
		SLOT( slotSelectColor2( const QColor & ) ) );

      rGroup = new QGroupBox( klocale->translate("Wallpaper"), this );

      QString path = kapp->kde_wallpaperdir().copy();
      QDir d( path, "*", QDir::Name, QDir::Readable | QDir::Files );
      QStrList list = *d.entryList();
      if(!wallpaper.isEmpty())
        list.append( wallpaper.data() );

      wpCombo = new QComboBox( false, rGroup );

      for ( uint i = 0; i < list.count(); i++ )
      {
	wpCombo->insertItem( list.at(i) );
	if ( wallpaper == list.at(i) )
		wpCombo->setCurrentItem( i );
      }

      if ( wallpaper.length() > 0 && wpCombo->currentItem() == 0 )
      {
	wpCombo->insertItem( wallpaper );
	wpCombo->setCurrentItem( wpCombo->count()-1 );
      }
      connect( wpCombo, SIGNAL( activated( const char * ) ),
		SLOT( slotWallpaper( const char * )  )  );
      wpCombo->setFixedHeight(wpCombo->sizeHint().height());

      button = new QPushButton( klocale->translate("Browse..."), rGroup );
      button->setFixedSize( button->sizeHint() );
      connect( button, SIGNAL( clicked() ), SLOT( slotBrowse() ) );

      wpGroup = new QButtonGroup( rGroup );
      wpGroup->setFrameStyle(QFrame::NoFrame);
      wpGroup->setExclusive( TRUE );
      QBoxLayout *wpl = new QHBoxLayout(wpGroup, 10, 10, "wpl");
      QBoxLayout *wpl1 = new QVBoxLayout(-1, "wpl1");
      QBoxLayout *wpl2 = new QVBoxLayout(-1, "wpl2");
      QBoxLayout *wpl3 = new QVBoxLayout(-1, "wpl3");
      wpl->addLayout(wpl1);
      wpl->addLayout(wpl2);
      wpl->addLayout(wpl3);

      rb = new QRadioButton( klocale->translate("None"), wpGroup );
      rb->setFixedSize( rb->sizeHint());
      wpGroup->insert( rb, NoPic );
      wpl1->addWidget(rb, 0, AlignLeft);

      rb = new QRadioButton( klocale->translate("Tile"), wpGroup );
      rb->setFixedSize( rb->sizeHint());
      wpGroup->insert( rb, Tile );
      wpl1->addWidget(rb, 0, AlignLeft);

      rb = new QRadioButton( klocale->translate("Center"), wpGroup );
      rb->setFixedSize( rb->sizeHint() );
      wpGroup->insert( rb, Center );
      wpl1->addWidget(rb, 0, AlignLeft);

      rb = new QRadioButton( klocale->translate("Scale"), wpGroup );
      rb->setFixedSize( rb->sizeHint() );
      wpGroup->insert( rb, Scale );
      wpl2->addWidget(rb, 0, AlignLeft);

      rb = new QRadioButton( klocale->translate("TopLeft"), wpGroup );
      rb->setFixedSize( rb->sizeHint());
      wpGroup->insert( rb, TopLeft );
      wpl2->addWidget(rb, 0, AlignLeft);

      rb = new QRadioButton( klocale->translate("TopRight"), wpGroup );
      rb->setFixedSize( rb->sizeHint());
      wpGroup->insert( rb, TopRight );
      wpl2->addWidget(rb, 0, AlignLeft);

      rb = new QRadioButton( klocale->translate("BottomLeft"), wpGroup );
      rb->setFixedSize( rb->sizeHint());
      wpGroup->insert( rb, BottomLeft );
      wpl3->addWidget(rb, 0, AlignLeft);

      rb = new QRadioButton( klocale->translate("BottomRight"), wpGroup );
      rb->setFixedSize( rb->sizeHint());
      wpGroup->insert( rb, BottomRight );
      wpl3->addWidget(rb, 0, AlignLeft);

      rb = new QRadioButton( klocale->translate("Fancy"), wpGroup );
      rb->setFixedSize( rb->sizeHint());
      wpGroup->insert( rb, Fancy );
      wpl3->addWidget(rb, 0, AlignLeft);

      wpl->activate();

      connect( wpGroup, SIGNAL( clicked( int ) ), SLOT( slotWallpaperMode( int ) ) );

      // Layouts

      // Bottom left (Color group)
      QBoxLayout *lLayout = new QVBoxLayout(lGroup, 10);
      lLayout->addSpacing(lGroup->fontMetrics().height()/2);
      lLayout->addWidget(cGroup, 0, AlignLeft);
      lLayout->addWidget(colButton1);
      lLayout->addWidget(colButton2);
      lLayout->addStretch(1);
      lLayout->activate();

      // Bottom right (Wallpaper group)
      QBoxLayout *rLayout = new QVBoxLayout(rGroup, 10);
      rLayout->addSpacing(rGroup->fontMetrics().height()/2);

      QBoxLayout *r1Layout = new QHBoxLayout(-1, "r1Layout");
      rLayout->addLayout(r1Layout);
      r1Layout->addWidget(wpCombo);
      r1Layout->addWidget(button);

      rLayout->addWidget(wpGroup,2);
      rLayout->addStretch(1);

      rLayout->activate();

      QBoxLayout *ml = new QVBoxLayout(this, 10, 10, "il");
      QBoxLayout *tl = new QVBoxLayout(-1, "tl");
      QBoxLayout *bl = new QHBoxLayout(-1, "bl");

      ml->addLayout(tl);
      tl->addWidget(tGroup);

      ml->addLayout(bl);
      bl->addWidget(lGroup);
      bl->addWidget(rGroup);

      ml->activate();

      showSettings();
}

void KDMBackgroundWidget::slotQDrop( QDropEvent *e )
{
  QStrList list;
  QString s;

#if QT_VERSION > 140
  if(QUrlDrag::decode( e, list ) )
  {
    monitor->setAllowDrop(false);
    s = list.first(); // we only want the first
    //debug("slotQDropEvent - %s", s.data());
    s = QUrlDrag::urlToLocalFile(s.data()); // a hack. should be improved
    if(!s.isEmpty())
      loadWallpaper(s.data());
  } 
#endif   
}

void KDMBackgroundWidget::slotQDragLeave( QDragLeaveEvent* )
{
  //debug("Got QDragLeaveEvent!");
  monitor->setAllowDrop(false);
}

void KDMBackgroundWidget::slotQDragEnter( QDragEnterEvent *e )
{
  //debug("Got QDragEnterEvent!");
#if QT_VERSION > 140
  if( QUrlDrag::canDecode( e ) )
  {
    monitor->setAllowDrop(true);
    e->accept();
  }
  else
#endif
    e->ignore();
}

void KDMBackgroundWidget::slotSelectColor1(const QColor &col)
{
  color1 = col;
  slotWallpaperMode(wpMode);
}

void KDMBackgroundWidget::slotSelectColor2(const QColor &col)
{
  color2 = col;
  slotWallpaperMode(wpMode);
}

void KDMBackgroundWidget::slotBrowse()
{
	QString path;

	path = kapp->kde_wallpaperdir().copy();

	QDir dir( path );
	if ( !dir.exists() )
		path = NULL;

	QString filename = KFileDialog::getOpenFileName( path );
	slotWallpaper( filename );
	if ( !filename.isEmpty() && !strcmp( filename, wallpaper) )
	{
		wpCombo->insertItem( wallpaper );
		wpCombo->setCurrentItem( wpCombo->count() - 1 );
	}
}

void KDMBackgroundWidget::setMonitor()
{
  QApplication::setOverrideCursor( waitCursor );

  if ( !wallpaper.isEmpty() )
  {
    //debug("slotSelectColor - setting wallpaper");
    float sx = (float)monitor->width() / QApplication::desktop()->width();
    float sy = (float)monitor->height() / QApplication::desktop()->height();

    QWMatrix matrix;
    matrix.scale( sx, sy );
    monitor->setBackgroundPixmap( wpPixmap.xForm( matrix ) );
  }

  QApplication::restoreOverrideCursor();
}

void KDMBackgroundWidget::slotWallpaper( const char *filename )
{
  if ( filename )
  {
    if ( loadWallpaper( filename ) == TRUE )
	setMonitor();
  }
}

void KDMBackgroundWidget::slotWallpaperMode( int m )
{
  wpMode = m;

  if(wpMode == NoPic)
  {
    wpCombo->setEnabled(false);
    button->setEnabled(false);
  }
  else
  {
    wpCombo->setEnabled(true);
    button->setEnabled(true);
  }

  if ( loadWallpaper( wallpaper ) == TRUE )
    setMonitor();
}

void KDMBackgroundWidget::slotColorMode( int m )
{
  colorMode = m;

  if(colorMode == Plain)
    colButton2->setEnabled(false);
  else
    colButton2->setEnabled(true);

  if ( loadWallpaper( wallpaper ) == TRUE )
    setMonitor();
}

// Attempts to load the specified wallpaper and creates a centred/scaled
// version if necessary.
// Note that centred pixmaps are placed on a full screen image of background
// color1, so if you want to save memory use a small tiled pixmap.
//
int KDMBackgroundWidget::loadWallpaper( const char *name, bool useContext )
{
  static int context = 0;
  QString filename;
  int rv = FALSE;
  KPixmap tmp;

  if ( useContext )
  {
	if ( context )
		QColor::destroyAllocContext( context );
		context = QColor::enterAllocContext();
  }

  if ( name[0] != '/' )
  {
    filename = kapp->kde_wallpaperdir().copy();
    filename += "/";
    filename += name;
  }
  else
    filename = name;

  if ( wpMode == NoPic || tmp.load( filename.data() ) == TRUE )
  {
    wallpaper = filename;
    int w = QApplication::desktop()->width();
    int h = QApplication::desktop()->height();

    wpPixmap.resize( w, h );

    if(wpMode != Scale && wpMode != Tile)
    {
      switch(colorMode)
      {
        default:
        case Plain:
          wpPixmap.fill( color1 );
          break;
        case Horizontal:
          wpPixmap.gradientFill(color1, color2, false);
          break;
        case Vertical:
          wpPixmap.gradientFill(color1, color2, true);
          break;
      } // switch..
    } // if..

    switch ( wpMode )
    {
	case Tile:
	  wpPixmap = tmp;
	  break;
	case Center:
	  bitBlt( &wpPixmap, (w - tmp.width())/2,
			(h - tmp.height())/2, &tmp, 0, 0, tmp.width(), tmp.height() );
	  break;

	case Scale:
	{
	  float sx = (float)w / tmp.width();
	  float sy = (float)h / tmp.height();
	  QWMatrix matrix;
	  matrix.scale( sx, sy );
          QPixmap qtmp = tmp.xForm(matrix);
	  wpPixmap = (const KPixmap&)qtmp;
	}
	break;

	case TopLeft:
	  bitBlt( &wpPixmap, 0, 0, &tmp, 0, 0, tmp.width(), tmp.height() );
	  break;

	case TopRight:
	  bitBlt( &wpPixmap, w-tmp.width(), 0,
		&tmp, 0, 0, tmp.width(), tmp.height() );
	  break;

	case BottomLeft:
	  bitBlt( &wpPixmap, 0, h-tmp.height(),
		&tmp, 0, 0, tmp.width(), tmp.height() );
	  break;

	case BottomRight:
	  bitBlt( &wpPixmap, w-tmp.width(), h-tmp.height(),
		&tmp, 0, 0, tmp.width(), tmp.height() );
	  break;

	case NoPic:
	case Fancy:
	  break;
	default:
        {
	  wpPixmap = tmp;
        }
        break;
    }
    rv = TRUE;
  }
  else
  {
    debug("KDMBackgroundWidget::loadWallpaper(): failed loading %s", filename.data());
    wallpaper = "";
  }

  if ( useContext )
    QColor::leaveAllocContext();

  return rv;
}

void KDMBackgroundWidget::showSettings()
{ 
  wpGroup->setButton(wpMode);
  cGroup->setButton(colorMode);

  colButton1->setColor( color1 );
  colButton2->setColor( color2 );

  if(colorMode == Plain)
    colButton2->setEnabled(false);
  else
    colButton2->setEnabled(true);

  wpCombo->setCurrentItem( 0 );
  for ( int i = 1; i < wpCombo->count(); i++ )
  {
    if ( wallpaper == wpCombo->text( i ) )
    {
      wpCombo->setCurrentItem( i );
      break;
    }
  }

  if ( wpMode == NoPic || !wallpaper.isEmpty() ) // && wpCombo->currentItem() == 0 )
  {
    loadWallpaper(wallpaper.data());
    wpCombo->insertItem( wallpaper );
    wpCombo->setCurrentItem( wpCombo->count()-1 );
  }

  if(wpMode == NoPic)
  {
    wpCombo->setEnabled(false);
    button->setEnabled(false);
  }
  else
  {
    wpCombo->setEnabled(true);
    button->setEnabled(true);
  }
/*
  ((QRadioButton *)wpGroup->find( NoPic ))->setChecked( wpMode == NoPic );
  ((QRadioButton *)wpGroup->find( Tile ))->setChecked( wpMode == Tile );
  ((QRadioButton *)wpGroup->find( Center ))->setChecked( wpMode == Center );
  ((QRadioButton *)wpGroup->find( Scale ))->setChecked( wpMode == Scale );
  ((QRadioButton *)wpGroup->find( TopLeft ))->setChecked( wpMode == TopLeft );
  ((QRadioButton *)wpGroup->find( TopRight ))->setChecked( wpMode == TopRight );
  ((QRadioButton *)wpGroup->find( BottomLeft ))->setChecked( wpMode == BottomLeft );
  ((QRadioButton *)wpGroup->find( BottomRight ))->setChecked( wpMode == BottomRight );
  ((QRadioButton *)wpGroup->find( Fancy ))->setChecked( wpMode == Fancy );

  ((QRadioButton *)cGroup->find( Plain ))->setChecked( colorMode == Plain );
  ((QRadioButton *)cGroup->find( Horizontal ))->setChecked( colorMode == Horizontal );
  ((QRadioButton *)cGroup->find( Vertical ))->setChecked( colorMode == Vertical );
*/
  setMonitor();
}

void KDMBackgroundWidget::applySettings()
{
  //debug("KDMBackgroundWidget::applySettings()"); 
  QString fn(CONFIGFILE);
  KSimpleConfig *c = new KSimpleConfig(fn);

  c->setGroup("KDMDESKTOP");

  // write color
  c->writeEntry( "BackGroundColor1", color1 );
  c->writeEntry( "BackGroundColor2", color2 );

  switch( colorMode )
  {
    case Vertical:
      c->writeEntry( "BackGroundColorMode", "Vertical" );
      break;
    case Horizontal:
      c->writeEntry( "BackGroundColorMode", "Horizontal" );
      break;
    case Plain:
    default:
      c->writeEntry( "BackGroundColorMode", "Plain" );
      break;
  }

  // write wallpaper

  if(!wallpaper.isEmpty())
  {
    QFileInfo fi(wallpaper.data());
    if(fi.exists())
      c->writeEntry( "BackGroundPicture", wallpaper );
    else
      c->deleteEntry( "BackGroundPicture", false);
  }
  else
    c->deleteEntry( "BackGroundPicture", false );

  switch ( wpMode )
  {
    case NoPic:
      c->writeEntry( "BackGroundPictureMode", "None" );
      break;
    case Tile:
      c->writeEntry( "BackGroundPictureMode", "Tile" );
      break;
    case Center:
      c->writeEntry( "BackGroundPictureMode", "Center" );
      break;
    case Scale:
      c->writeEntry( "BackGroundPictureMode", "Scale" );
      break;
    case TopLeft:
      c->writeEntry( "BackGroundPictureMode", "TopLeft" );
      break;
    case TopRight:
      c->writeEntry( "BackGroundPictureMode", "TopRight" );
      break;
    case BottomLeft:
      c->writeEntry( "BackGroundPictureMode", "BottomLeft" );
      break;
    case BottomRight:
      c->writeEntry( "BackGroundPictureMode", "BottomRight" );
      break;
    case Fancy:
      c->writeEntry( "BackGroundPictureMode", "Fancy" );
    break;
  } // switch

  delete c;
}

void KDMBackgroundWidget::loadSettings()
{
  iconloader = kapp->getIconLoader();
  QString fn(CONFIGFILE), str;
  
  // Get config object
  KSimpleConfig *c = new KSimpleConfig(fn);

  c->setGroup("KDMDESKTOP");
  color1  = c->readColorEntry( "BackGroundColor1", &darkCyan);
  color2  = c->readColorEntry( "BackGroundColor2", &darkBlue);

  str = c->readEntry( "BackGroundPicture" );
  if ( !str.isEmpty() )
  {
    QFileInfo fi(str.data());
    if(fi.exists())
    {
      iconloader->insertDirectory(0, fi.dirPath(true));
      wallpaper = str;
    }
    else wallpaper = "";
  }
  else wallpaper = "";

  QString strmode = c->readEntry( "BackGroundColorMode", "Plain");
  if(strmode == "Plain")
    colorMode = Plain;
  else if(strmode == "Horizontal")
    colorMode = Horizontal;
  else if(strmode == "Vertical")
    colorMode = Vertical;
  else
    colorMode = Plain;


  strmode = c->readEntry( "BackGroundPictureMode", "Scale");
  if(strmode == "None")
    wpMode = NoPic;
  else if(strmode == "Tile")
    wpMode = Tile;
  else if(strmode == "Center")
    wpMode = Center;
  else if(strmode == "Scale")
    wpMode = Scale;
  else if(strmode == "TopLeft")
    wpMode = TopLeft;
  else if(strmode == "TopRight")
    wpMode = TopRight;
  else if(strmode == "BottomLeft")
    wpMode = BottomLeft;
  else if(strmode == "BottomRight")
    wpMode = BottomRight;
  else if(strmode == "Fancy")
    wpMode = Fancy;
  else
    wpMode = Tile;

  delete c;
}


