/*
 * bg.cpp.  Part of the KDE Project.
 *
 * Copyright (C) 1997 Martin Jones
 *               1998 Matej Koss
 *
 *
 */

//----------------------------------------------------------------------------

#include <stdlib.h>

#include <qimage.h>
#include <qfile.h>
#include <qpainter.h>
#include <qstring.h>
#include <qpmcache.h>
#include <qdir.h>

#include <kapp.h>
#include <kpixmap.h>
#include <kstring.h>
#include <kwm.h>
#include <kurl.h>
#include <kprocess.h>

#include "bg.h"
#include "bg.moc"
#include "config-kbgndwm.h"

//----------------------------------------------------------------------------


KBackground::KBackground()
{
  wpMode = Tiled;
  gfMode = Flat;
  orMode = Portrait;

  bgPixmap = 0;
  applied = false;

  hasPm = false;
  bUseWallpaper = false;

  timerRandom = 0;
  randomDesk = 0;
  desk = 0;

  useDir = false;
}


KBackground::~KBackground()
{
}



void KBackground::setImmediately( const char *_wallpaper, int mode )
{
  hasPm = false;
  color1 = QColor(black);
  gfMode=Flat;
  orMode=Portrait;
  wpMode = mode;
  bUseWallpaper = true;

  KURL url = _wallpaper;
  wallpaper = url.path();

  hasPm = true;
  apply();

  QString command;
  command << "kbgwm_change_" << desk;
  KWM::sendKWMCommand( command.data() );
}


/*
 * Read the settings from desktoXprc
 */
void KBackground::readSettings( int num, bool one, int onedesk )
{
  hasPm = false;

  desk = num;
  oneDesktopMode = one;
  oneDesk = onedesk;

  QString tmpf;
  ksprintf( &tmpf, "/desktop%drc", num);

  bool first_time = false;

  QFileInfo fi( KApplication::localconfigdir() + tmpf );
  if ( ! fi.exists() ){
    tmpf = "/kcmdisplayrc";
    first_time = true;
  }

  KConfig config(KApplication::kde_configdir() + tmpf,
		  KApplication::localconfigdir() + tmpf);

  if ( !first_time ) {
    config.setGroup( "Common" );
    randomMode = config.readBoolEntry( "RandomMode", DEFAULT_ENABLE_RANDOM_MODE );

    if ( randomMode ) {
      if ( !timerRandom ) {
	timerRandom = new QTimer( this );
	connect(timerRandom, SIGNAL(timeout()), SLOT(randomize()));
      }
      timerRandom->stop();
      timerRandom->start( config.readNumEntry( "Timer",
                                              DEFAULT_RANDOM_TIMER ) * 1000 );
      int count = config.readNumEntry( "Count", DEFAULT_RANDOM_COUNT );
      bool inorder = config.readBoolEntry( "InOrder", DEFAULT_RANDOM_IN_ORDER);
      useDir = config.readBoolEntry( "UseDir", DEFAULT_RANDOM_USE_DIR );

      if ( useDir ) {
	QString tmpd = config.readEntry( "Directory", KApplication::kde_wallpaperdir());
	QDir d( tmpd, "*", QDir::Name, QDir::Readable | QDir::Files );

	QStrList *list = (QStrList *)d.entryList();

	count = list->count();
	if ( inorder ) {
	  randomDesk = config.readNumEntry( "Item", DEFAULT_DESKTOP );
	  randomDesk++;
	  if ( randomDesk >= count ) randomDesk = DEFAULT_DESKTOP;
	} else if ( count > 0 )
	  randomDesk = rand() % count;

	config.writeEntry( "Item", randomDesk );
	config.sync();

	color1 = QColor(DEFAULT_COLOR_1);
	gfMode = DEFAULT_COLOR_MODE;
	orMode = DEFAULT_ORIENTATION_MODE;
	wpMode = DEFAULT_WALLPAPER_MODE;
	bUseWallpaper = true;

	wallpaper = d.absPath() + "/" + list->at( randomDesk );
	name.sprintf( "%s_%d_%d_%d#%02x%02x%02x#%02x%02x%02x#", wallpaper.data(),
		      wpMode, gfMode, orMode, color1.red(), color1.green(),
		      color1.blue(), color1.red(), color2.green(), color2.blue());

	hasPm = true;

	// this is mainly for kpager, so that we can at anytime find out how desktop
	//          really looks
	config.writeEntry( "Item", randomDesk );
	config.sync();

	return;
      }
      else if ( inorder ) {
	randomDesk = config.readNumEntry( "Item", DEFAULT_DESKTOP );
	randomDesk++;
	if ( randomDesk >= count ) randomDesk = DEFAULT_DESKTOP;
      }
      else if ( count > 0 )
	randomDesk = rand() % count;

    }
    else
      randomDesk = DEFAULT_DESKTOP;

    // this is mainly for kpager, so that we can at anytime find out how desktop
    //          really looks
    config.writeEntry( "Item", randomDesk );
    config.sync();
  }
  else
    randomDesk = num + 1;


  ksprintf( &tmpf, "Desktop%d", randomDesk);
  config.setGroup( tmpf );

  QString str;

  str = config.readEntry( "Color1", DEFAULT_COLOR_1 );
  color1.setNamedColor( str );

  str = config.readEntry( "Color2", DEFAULT_COLOR_2 );
  color2.setNamedColor( str );

  gfMode = DEFAULT_COLOR_MODE;
  str = config.readEntry( "ColorMode", "unset" );
  if ( str == "Gradient" ) {
      gfMode = Gradient;
      hasPm = true;
  }
  else if (str == "Pattern") {
    gfMode = Pattern;
    QStrList strl;
    config.readListEntry("Pattern", strl);
    uint size = strl.count();
    if (size > 8) size = 8;
    uint i = 0;
    for (i = 0; i < 8; i++)
      pattern[i] = (i < size) ? QString(strl.at(i)).toUInt() : 255;
  }

  orMode = DEFAULT_ORIENTATION_MODE;
  str = config.readEntry( "OrientationMode", "unset" );
  if ( str == "Landscape" )
    orMode = Landscape;

  wpMode = DEFAULT_WALLPAPER_MODE;
  str = config.readEntry( "WallpaperMode", "unset" );
  if ( str == "Mirrored" )
    wpMode = Mirrored;
  else if ( str == "CenterTiled" )
    wpMode = CenterTiled;
  else if ( str == "Centred" )
    wpMode = Centred;
  else if ( str == "CentredBrick" )
    wpMode = CentredBrick;
  else if ( str == "CentredWarp" )
    wpMode = CentredWarp;
  else if ( str == "CentredMaxpect" )
    wpMode = CentredMaxpect;
  else if ( str == "SymmetricalTiled" )
    wpMode = SymmetricalTiled;
  else if ( str == "SymmetricalMirrored" )
    wpMode = SymmetricalMirrored;
  else if ( str == "Scaled" )
    wpMode = Scaled;

  wallpaper = DEFAULT_WALLPAPER;
  bUseWallpaper = config.readBoolEntry( "UseWallpaper", DEFAULT_WALLPAPER_MODE );
  if ( bUseWallpaper )
    wallpaper = config.readEntry( "Wallpaper", DEFAULT_WALLPAPER );

  name.detach();
  name.sprintf( "%s_%d_%d_%d#%02x%02x%02x#%02x%02x%02x#", wallpaper.data(),
		wpMode, gfMode, orMode, color1.red(), color1.green(),
		color1.blue(), color1.red(), color2.green(), color2.blue());

  QString tmp;
  for (int i = 0; i < 8; i++) {
    tmp.sprintf("%02x", pattern[i]);
    name += tmp;
  }

  hasPm = true;

}


void KBackground::randomize()
{
    doRandomize( TRUE );
}


void KBackground::doRandomize(bool fromTimer)
{
  if ( randomMode ) {
    readSettings( desk, oneDesktopMode, oneDesk );

    QString command;
    if ( oneDesktopMode )
      command = "kbgwm_change";
    else
      command << "kbgwm_change_" << desk;
    
    KWM::sendKWMCommand( command.data() );

    if ( ( oneDesktopMode && desk == oneDesk ) ||
	 ( !oneDesktopMode && desk == ( KWM::currentDesktop() - 1 ) ) )
      apply();

    return;
  }
  if (!fromTimer) {
      KShellProcess proc;
      proc << "kcmdisplay";
      proc.start(KShellProcess::DontCare);
  }
}

QPixmap *KBackground::loadWallpaper()
{

  if( !bUseWallpaper ) return 0;

  QString filename;

  if ( wallpaper[0] != '/' )
    {
      filename = KApplication::kde_wallpaperdir().copy() + "/";
      filename += wallpaper;
    }
  else
    filename = wallpaper;

  KPixmap *wpPixmap = new KPixmap;

  if ( wpPixmap->load( filename, 0, KPixmap::LowColor ) == FALSE )
    {
      delete wpPixmap;
      wpPixmap = 0;
    }

  return wpPixmap;
}


void KBackground::apply()
{
  applied = false;

  // the background pixmap is cached?
  bgPixmap = QPixmapCache::find( name );
  if ( bgPixmap )
    {
      debug( "Desktop background found in cache" );
      qApp->desktop()->setBackgroundPixmap( *bgPixmap );
      bgPixmap = 0;
      applied = true;
      return;
    }

  QPixmap *wpPixmap = loadWallpaper();

  uint w=0, h=0;

  if (wpPixmap) {
    w = QApplication::desktop()->width();
    h = QApplication::desktop()->height();

    bgPixmap = new QPixmap;
  }

  if ( !wpPixmap || (wpMode == Centred) || (wpMode == CentredBrick) ||
       (wpMode == CentredWarp) || ( wpMode == CentredMaxpect) ) {
    if (bgPixmap)
      bgPixmap->resize(w, h);
	
    switch (gfMode) {

    case Gradient:
      {
	int numColors = 4;
	if ( QColor::numBitPlanes() > 8 )
	  numColors = 16;
		
	KPixmap pmDesktop;
		
	if ( orMode == Portrait ) {

	  pmDesktop.resize( 20, QApplication::desktop()->height() );
	  pmDesktop.gradientFill( color2, color1, true, numColors );

	} else {

	  pmDesktop.resize( QApplication::desktop()->width(), 20 );
	  pmDesktop.gradientFill( color2, color1, false, numColors );
		
	}

	delete bgPixmap;
	bgPixmap = new QPixmap();
		
	if (! wpPixmap ) {

	  qApp->desktop()->setBackgroundPixmap(pmDesktop);
	  *bgPixmap = pmDesktop;
		
	} else {
	  bgPixmap->resize(w, h);
		
	  if ( orMode == Portrait ) {
	    for (uint pw = 0; pw <= w; pw += pmDesktop.width())
	      bitBlt( bgPixmap, pw, 0, &pmDesktop, 0, 0,
		      pmDesktop.width(), h);
	  } else {
	    for (uint ph = 0; ph <= h; ph += pmDesktop.height()) {
	      debug("land %d",ph);
	      bitBlt( bgPixmap, 0, ph, &pmDesktop, 0, 0,
		      w, pmDesktop.height());
	    }
	  }
	}

	// background switch is deferred in case the user switches
	// again while the background is loading
	startTimer( 0 );
      }
      break;
	
    case Flat:
      if (wpPixmap ) {
	delete bgPixmap;
	bgPixmap = new QPixmap(w, h);
	bgPixmap->fill( color1 );
      } else {
	qApp->desktop()->setBackgroundColor( color1 );
	applied = true;
      }
      break;
	
    case Pattern:
      {
	QPixmap tile(8, 8);
	tile.fill(color2);
	QPainter pt;
	pt.begin(&tile);
	pt.setBackgroundColor( color2 );
	pt.setPen( color1 );
		
	for (int y = 0; y < 8; y++) {
	  uint v = pattern[y];
	  for (int x = 0; x < 8; x++) {
	    if ( v & 1 )
	      pt.drawPoint(7 - x, y);
	    v /= 2;
	  }
	}
	pt.end();

	delete bgPixmap;
	bgPixmap = new QPixmap();

	if (! wpPixmap ) {
	  qApp->desktop()->setBackgroundPixmap(tile);
	  *bgPixmap = tile;
	  applied = true;
	} else {
	  bgPixmap->resize(w, h);
	  uint sx, sy = 0;
	  while (sy < h) {
	    sx = 0;
	    while (sx < w) {
	      bitBlt( bgPixmap, sx, sy, &tile, 0, 0, 8, 8);
	      sx += 8;
	    }
	    sy += 8;
	  }
	}
	break;
      }
    }
	
  }

  if ( wpPixmap )
    {
	
      if ( ( wpPixmap->width() > (int)w || wpPixmap->height() > (int)h ||
	   wpMode == CentredMaxpect ) &&
	   wpMode != Scaled ) {
	// shrink if image is bigger than desktop or CentredMaxpect
	float sc;
	float S = (float)h / (float)w ;
	float I = (float)wpPixmap->height() / (float)wpPixmap->width() ;
	
	if (S < I)
	  sc= (float)h / (float)wpPixmap->height();
	else
	  sc= (float)w / (float)wpPixmap->width();
	
	QWMatrix scaleMat;
	scaleMat.scale(sc,sc);

	QPixmap tmp2 = wpPixmap->xForm( scaleMat );
	wpPixmap->resize( tmp2.width(), tmp2.height() );
	bitBlt( wpPixmap, 0, 0, &tmp2 );
      }

      switch ( wpMode )
	{

	case Tiled:
	  {
	    bgPixmap->resize( wpPixmap->width(), wpPixmap->height() );
	    bgPixmap->fill( color1 );
	    bitBlt( bgPixmap, 0, 0, wpPixmap );
	  }
	  break;

	case Mirrored:
	  {
	    int w = wpPixmap->width();
	    int h = wpPixmap->height();

	    bgPixmap->resize( w * 2, h * 2);

	    /* quadrant 2 */
 	    bitBlt( bgPixmap, 0, 0, wpPixmap );
	
	    /* quadrant 1 */
	    QWMatrix S(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
	    QPixmap newp = wpPixmap->xForm( S );
	    bitBlt( wpPixmap, 0, 0, &newp );
	    bitBlt( bgPixmap, w, 0, wpPixmap );

	    /* quadrant 4 */
	    S.setMatrix(1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
	    newp = wpPixmap->xForm( S );
	    bitBlt( wpPixmap, 0, 0, &newp );
	    bitBlt( bgPixmap, w, h, wpPixmap );

	    /* quadrant 3 */
	    S.setMatrix(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
	    newp = wpPixmap->xForm( S );
	    bitBlt( wpPixmap, 0, 0, &newp );
	    bitBlt( bgPixmap, 0, h, wpPixmap );

	  }
	  break;

	case SymmetricalTiled:
	case SymmetricalMirrored:
	  {
	    int fliph = 0;
	    int flipv = 0;
	    uint w0 = wpPixmap->width();
	    uint h0 = wpPixmap->height();

	    bgPixmap->resize(w, h);

	    if (w == w0) {
	      /* horizontal center line */
	      int y, ay;

	      y = h0 - ((h/2)%h0); /* Starting point in picture to copy */
	      ay = 0;    /* Vertical anchor point */
	      while (ay < (int)h) {
		bitBlt( bgPixmap, 0, ay, wpPixmap, 0, y );
		ay += h0 - y;
		y = 0;
		if ( wpMode == SymmetricalMirrored ) {
		  QWMatrix S(1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
		  QPixmap newp = wpPixmap->xForm( S );
		  bitBlt( wpPixmap, 0, 0, &newp );
		  flipv = !flipv;
		}
	      }
	    }
	    else if (h == h0) {
	      /* vertical centerline */
	      int x, ax;

	      x = w0 - ((w/2)%w0); /* Starting point in picture to copy */
	      ax = 0;    /* Horizontal anchor point */
	      while (ax < (int)w) {
		bitBlt( bgPixmap, ax, 0, wpPixmap, x, 0 );
		ax += w0 - x;
		x = 0;
		if ( wpMode == SymmetricalMirrored ) {
		  QWMatrix S(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
		  QPixmap newp = wpPixmap->xForm( S );
		  bitBlt( wpPixmap, 0, 0, &newp );
		  fliph = !fliph;
		}
	      }
	    }
	    else {
	      /* vertical and horizontal centerlines */
	      int x,y, ax,ay;

	      y = h0 - ((h/2)%h0); /* Starting point in picture to copy */
	      ay = 0;    /* Vertical anchor point */

	      while (ay < (int)h) {
		x = w0 - ((w/2)%w0);/* Starting point in picture to cpy */
		ax = 0;    /* Horizontal anchor point */
		while (ax < (int)w) {
		  bitBlt( bgPixmap, ax, ay, wpPixmap, x, y );
		  if ( wpMode == SymmetricalMirrored ) {
		    QWMatrix S(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
		    QPixmap newp = wpPixmap->xForm( S );
		    bitBlt( wpPixmap, 0, 0, &newp );
		    fliph = !fliph;
		  }
		  ax += w0 - x;
		  x = 0;
		}
		if ( wpMode == SymmetricalMirrored ) {
		  QWMatrix S(1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
		  QPixmap newp = wpPixmap->xForm( S );
		  bitBlt( wpPixmap, 0, 0, &newp );
		  flipv = !flipv;
		  if (fliph) {   /* leftmost image is always non-hflipped */
		    S.setMatrix(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
		    newp = wpPixmap->xForm( S );
		    bitBlt( wpPixmap, 0, 0, &newp );
		    fliph = !fliph;
		  }
		}
		ay += h0 - y;
		y = 0;
	      }
	    }
	  }
	  break;
	
	case CenterTiled:
	  {
	    int i, j, x, y, w0, h0, ax, ay, w1, h1, offx, offy;

	    bgPixmap->resize(w, h);

	    w0 = wpPixmap->width();  h0 = wpPixmap->height();

	    /* compute anchor pt (top-left coords of top-left-most pic) */
	    ax = (w-w0)/2;  ay = (h-h0)/2;
	    while (ax>0) ax = ax - w0;
	    while (ay>0) ay = ay - h0;

	    for (i=ay; i < (int)h; i+=h0) {
	      for (j=ax; j < (int)w; j+=w0) {
		/* if image goes off tmpPix, only draw subimage */
	
		x = j;  y = i;  w1 = w0;  h1 = h0;  offx = offy = 0;
		if (x<0)           { offx = -x;  w1 -= offx;  x = 0; }
		if (x+w1>w0) { w1 = (w0-x); }

		if (y<0)           { offy = -y;  h1 -= offy;  y = 0; }
		if (y+h1>h0)    { h1 = (h0-y); }
	
		bitBlt( bgPixmap, x, y, wpPixmap, offx, offy );
	      }
	    }

	  }
	  break;
		
	case Centred:
	case CentredMaxpect:
	  {
	    bitBlt( bgPixmap, ( w - wpPixmap->width() ) / 2,
		    ( h - wpPixmap->height() ) / 2, wpPixmap, 0, 0,
		    wpPixmap->width(), wpPixmap->height() );
	  }
	  break;
		
	case Scaled:
	  {
	    float sx = (float)w / wpPixmap->width();
	    float sy = (float)h / wpPixmap->height();
			
	    bgPixmap->resize( w, h );
	    bgPixmap->fill( color1 );
			
	    QWMatrix matrix;
	    matrix.scale( sx, sy );
	    QPixmap newp = wpPixmap->xForm( matrix );
	    bitBlt( bgPixmap, 0, 0, &newp );
	  }
	  break;

	case CentredBrick:
	  {
	    int i, j, k;

	    QPainter paint( bgPixmap );
	    paint.setPen( white );
	    for ( i=k=0; i < (int)w; i+=20,k++ ) {
	      paint.drawLine( 0, i, w, i );
	      for (j=(k&1) * 20 + 10; j< (int)w; j+=40)
		paint.drawLine( j, i, j, i+20 );
	    }

	    bitBlt( bgPixmap, ( w - wpPixmap->width() ) / 2,
		    ( h - wpPixmap->height() ) / 2, wpPixmap, 0, 0,
		    wpPixmap->width(), wpPixmap->height() );
	  }
	  break;

	case CentredWarp:
	  {
	    int i;

	    QPainter paint( bgPixmap );
	    paint.setPen( white );
	    for ( i=0; i < (int)w; i+=8 )
	      paint.drawLine( i, 0, w - i, h );
	    for ( i=0; i < (int)h; i+=8 )
	      paint.drawLine( 0, i, w, h - i );

	    bitBlt( bgPixmap, ( w - wpPixmap->width() ) / 2,
		    ( h - wpPixmap->height() ) / 2, wpPixmap, 0, 0,
		    wpPixmap->width(), wpPixmap->height() );
	  }
	  break;
	}

      delete wpPixmap;
      wpPixmap = 0;
	
      // background switch is deferred in case the user switches
      // again while the background is loading
      startTimer( 0 );
    }
}

void KBackground::cancel()
{
  killTimers();
  if ( bgPixmap )
    delete bgPixmap;
}

void KBackground::timerEvent( QTimerEvent * )
{
  killTimers();

  if ( !bgPixmap )
    return;

  qApp->desktop()->setBackgroundPixmap( *bgPixmap );
  delete bgPixmap;
  bgPixmap = 0;
  applied = true;
}

//----------------------------------------------------------------------------

