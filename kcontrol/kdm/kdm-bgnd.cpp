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

#include "utils.h"
#include "kdm-bgnd.moc"



// Destructor
KDMBackgroundWidget::~KDMBackgroundWidget()
{
  if(gui)
  {
    delete monitor;
    delete wpGroup;
    delete colButton;
    delete wpCombo;
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
      changed = FALSE;

      QLabel *label;
      QPushButton *button;
      QGroupBox *group;
      QRadioButton *rb;
      kapp->processEvents();
      QPixmap p = iconloader->loadIcon("monitor.xpm");

      label = new QLabel( this );
      label->setPixmap( p );
      label->setMinimumSize( label->sizeHint() );
      label->move( 200, 15 );

      monitor = new KBGMonitor( label );
      monitor->setGeometry( 20, 10, 157, 111 );
      monitor->setBackgroundColor( color );

      group = new QGroupBox( klocale->translate("Color"), this );
      group->setGeometry( 15, 190, 210, 130 );

      colButton = new KColorButton( color, group );
      colButton->setGeometry( 15, 25, 75, 30 );
      connect( colButton, SIGNAL( changed( const QColor & ) ),
		SLOT( slotSelectColor( const QColor & ) ) );

      group = new QGroupBox( klocale->translate("Wallpaper"), this );
      group->setGeometry( 240, 190, 215, 130 );

      QString path = kapp->kdedir().copy();
      path += "/share/wallpapers";
      QDir d( path, "*", QDir::Name, QDir::Readable | QDir::Files );
      QStrList list = *d.entryList();
      if(!wallpaper.isEmpty())
        list.append( wallpaper.data() );

      wpCombo = new QComboBox( group );
      wpCombo->setGeometry( 15, 20, 190, 25 );
      wpCombo->insertItem( NO_WALLPAPER, 0 );
      wpCombo->setCurrentItem( 0 );

      //QStrListIterator it( *list );
      for ( uint i = 0; i < list.count(); i++ )
      {
	wpCombo->insertItem( list.at(i) );
	if ( wallpaper == list.at(i) )
		wpCombo->setCurrentItem( i );
      }

      if ( wallpaper != NO_WALLPAPER && wpCombo->currentItem() == 0 )
      {
	wpCombo->insertItem( wallpaper );
	wpCombo->setCurrentItem( wpCombo->count()-1 );
      }
      connect( wpCombo, SIGNAL( activated( const char * ) ),
		SLOT( slotWallpaper( const char * )  )  );

      wpGroup = new QButtonGroup( this );
      wpGroup->hide();
      wpGroup->setExclusive( TRUE );

      rb = new QRadioButton( klocale->translate("Tiled"), group );
      rb->setGeometry( 20, 50, 85, 25 );
      wpGroup->insert( rb, Tiled );

      rb = new QRadioButton( klocale->translate("Centred"), group );
      rb->setGeometry( 20, 75, 85, 25 );
      wpGroup->insert( rb, Centred );

      rb = new QRadioButton( klocale->translate("Scaled"), group );
      rb->setGeometry( 20, 100, 85, 25 );
      wpGroup->insert( rb, Scaled );

      connect( wpGroup, SIGNAL( clicked( int ) ), SLOT( slotWallpaperMode( int ) ) );

      button = new QPushButton( klocale->translate("Browse..."), group );
      button->setGeometry( 125, 55, 80, 25 );
      connect( button, SIGNAL( clicked() ), SLOT( slotBrowse() ) );

      showSettings();
}


void KDMBackgroundWidget::slotSelectColor(const QColor &col)
{
  color = col;
  if ( wpMode == Centred || wallpaper == NO_WALLPAPER )
    setMonitor();
}

void KDMBackgroundWidget::slotBrowse()
{
	QString path;

	path = kapp->kdedir().copy();
	path += "/share/wallpapers";

	QDir dir( path );
	if ( !dir.exists() )
		path = NULL;

	QString filename = QFileDialog::getOpenFileName( path );
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

  if ( !wallpaper.isEmpty() && wallpaper != NO_WALLPAPER )
  {
    //debug("slotSelectColor - setting wallpaper");
    float sx = (float)monitor->width() / QApplication::desktop()->width();
    float sy = (float)monitor->height() / QApplication::desktop()->height();

    QWMatrix matrix;
    matrix.scale( sx, sy );
    monitor->setBackgroundPixmap( wpPixmap.xForm( matrix ) );
  }
  else
  {
    monitor->setBackgroundColor( color );
    //debug("slotSelectColor - setting backgroundcolor");
  }

  QApplication::restoreOverrideCursor();
}

void KDMBackgroundWidget::slotWallpaper( const char *filename )
{
  if ( filename )
  {
    if ( !strcmp( filename, NO_WALLPAPER ) )
    {
	wallpaper = "";
	setMonitor();
    }
    else if ( loadWallpaper( filename ) == TRUE )
    {
	//wallpaper = filename;
	setMonitor();
    }
    changed = TRUE;
  }
}

void KDMBackgroundWidget::slotWallpaperMode( int m )
{
	wpMode = m;
	if ( loadWallpaper( wallpaper ) == TRUE )
	{
		setMonitor();
		changed = TRUE;
	}
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
  QPixmap tmp;

  if ( useContext )
  {
	if ( context )
		QColor::destroyAllocContext( context );
		context = QColor::enterAllocContext();
  }

  if ( name[0] != '/' )
  {
	filename = kapp->kdedir().copy();
	filename += "/share/wallpapers/";
	filename += name;
  }
  else
	filename = name;
  //debug("loadWallPaper: %s", filename.data());
  if ( tmp.load( filename.data() ) == TRUE )
  //tmp = iconloader->loadIcon(filename);  // Why doesn't this work?
  //if(!tmp.isNull())
  {
    wallpaper = filename;
    int w = QApplication::desktop()->width();
    int h = QApplication::desktop()->height();

    switch ( wpMode )
    {
	case Centred:
	{
 	  wpPixmap.resize( w, h );
	  wpPixmap.fill( color );
	  bitBlt( &wpPixmap, (w - tmp.width())/2,
			(h - tmp.height())/2, &tmp, 0, 0, tmp.width(), tmp.height() );
	}
	break;

	case Scaled:
	{
	  float sx = (float)w / tmp.width();
	  float sy = (float)h / tmp.height();
	  QWMatrix matrix;
	  matrix.scale( sx, sy );
	  wpPixmap = tmp.xForm( matrix );
	}
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
  colButton->setColor( color );

  wpCombo->setCurrentItem( 0 );
  for ( int i = 1; i < wpCombo->count(); i++ )
  {
    if ( wallpaper == wpCombo->text( i ) )
    {
      wpCombo->setCurrentItem( i );
      break;
    }
  }

  if ( wallpaper != NO_WALLPAPER ) // && wpCombo->currentItem() == 0 )
  {
    loadWallpaper(wallpaper.data());
    wpCombo->insertItem( wallpaper );
    wpCombo->setCurrentItem( wpCombo->count()-1 );
  }

  ((QRadioButton *)wpGroup->find( Tiled ))->setChecked( wpMode == Tiled );
  ((QRadioButton *)wpGroup->find( Centred ))->setChecked( wpMode == Centred );
  ((QRadioButton *)wpGroup->find( Scaled ))->setChecked( wpMode == Scaled );

  setMonitor();
}

void KDMBackgroundWidget::applySettings()
{
  //debug("KDMBackgroundWidget::applySettings()"); 
  QString fn(CONFIGFILE);
  KSimpleConfig *c = new KSimpleConfig(fn);

  //c->deleteGroup("KDMDESKTOP");
  c->setGroup("KDMDESKTOP");

  // write color
  QString colName(10);
  colName.sprintf("#%02x%02x%02x", color.red(), color.green(), color.blue());
  c->writeEntry( "BackGroundColor", colName );

  // write wallpaper
  c->writeEntry("BackGroundPicture", "");
  if(wallpaper != NO_WALLPAPER)
  {
    QFileInfo fi(wallpaper.data());
    if(fi.exists())
    {
      c->writeEntry( "BackGroundPicture", wallpaper );
      switch ( wpMode )
      {
        case Tiled:
  	  c->writeEntry( "BackGroundPictureTile", 1 );
	  c->deleteEntry( "BackGroundPictureCenter", false );
          break;
        case Centred:
  	  c->deleteEntry( "BackGroundPictureTile", false );
	  c->writeEntry( "BackGroundPictureCenter", 1 );
	  break;
        default:
  	  c->deleteEntry( "BackGroundPictureTile", false );
	  c->deleteEntry( "BackGroundPictureCenter", false );
	break;
      } // switch
    } // if(fi...
  } // if(wall...
  else
  {
    c->deleteEntry( "BackGroundPicture", false );
    c->deleteEntry( "BackGroundPictureTile", false );
    c->deleteEntry( "BackGroundPictureCenter", false );
  }

  c->writeEntry( "FancyBackGround", fancy );
  delete c;
}

void KDMBackgroundWidget::loadSettings()
{
  iconloader = kapp->getIconLoader();
  QString fn(CONFIGFILE), str;
  
  // Get config object
  KSimpleConfig *c = new KSimpleConfig(fn);
  c->setGroup("KDMDESKTOP");

  c->setGroup("KDMDESKTOP");
  str = "DarkBlue";
  str = c->readEntry("BackGroundColor", str.data());
  color.setNamedColor(str);
  wpMode = Scaled;
  if(c->readNumEntry("BackGroundPictureTile", 1))
    wpMode = Tiled;
  else
  if(c->readNumEntry("BackGroundPictureCenter", 1))
    wpMode = Centred;
  wallpaper = NO_WALLPAPER;
  str = c->readEntry( "BackGroundPicture" );
  if ( !str.isEmpty() )
  {
    QFileInfo fi(str.data());
    if(fi.exists())
    {
      iconloader->insertDirectory(0, fi.dirPath(true));
      wallpaper = str;
    }
    //loadWallpaper( str );
  }
  fancy = c->readNumEntry("FancyBackGround", 0);

  delete c;
}


