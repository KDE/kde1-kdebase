/*
 * bg.cpp.  Part of the KDE Project.
 *
 * Copyright (C) 1997 Martin Jones
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

#include <kapp.h>
#include <ksimpleconfig.h>

#include "kpixmap.h"

#include "bg.h"
#include "bg.moc"

//#define NO_WALLPAPER	"(none)"

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
}

KBackground::~KBackground()
{
}

/*
 * Read the settings from kdisplayrc
 */
void KBackground::readSettings( const char *group )
{
    QString cFile;

    hasPm = false;
    name = group;

    cFile = getenv( "HOME" );
    cFile += "/.kde/share/config/kdisplayrc";

    KSimpleConfig config( cFile, true );

    config.setGroup( group );

    QString str;

    str = config.readEntry( "Color1", "#CCCCCC" );
    color1.setNamedColor( str );

    str = config.readEntry( "Color2", "#CCCCCC" );
    color2.setNamedColor( str );

    gfMode=Flat;
    str = config.readEntry( "ColorMode" );
    if ( !str.isEmpty() && str == "Gradient" )
    {
	gfMode = Gradient;
	hasPm = true;
    }

    if (str == "Pattern") {
	gfMode = Pattern;
	QStrList strl;
	config.readListEntry("Pattern", strl);
	uint size = strl.count();
	if (size > 8) size = 8;
	uint i = 0;
	for (i = 0; i < 8; i++)
	    pattern[i] = (i < size) ? QString(strl.at(i)).toUInt() : 255;
    }

    orMode=Portrait;
    str = config.readEntry( "OrientationMode" );
    if ( !str.isEmpty() && str == "Landscape" )
	orMode = Landscape;

    wpMode = Tiled;
    str = config.readEntry( "WallpaperMode" );
    if ( !str.isEmpty() )
    {
	if ( str == "Centred" )
	    wpMode = Centred;
	else if ( str == "Scaled" )
	    wpMode = Scaled;
    }

	wallpaper.sprintf( i18n("No wallpaper") );
	bUseWallpaper = config.readBoolEntry( "UseWallpaper", false );
	if ( bUseWallpaper )
    	wallpaper = config.readEntry( "Wallpaper", i18n("No wallpaper") );

    //wallpaper = NO_WALLPAPER;
    //str = config.readEntry( "Wallpaper" );
    //if ( !str.isEmpty() )
	//wallpaper = str;

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

QPixmap *KBackground::loadWallpaper()
{
    //if ( wallpaper == NO_WALLPAPER )
	//return 0;
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

    if ( !wpPixmap || (wpMode == Centred) ) {
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
	    
	    switch ( wpMode )
		{
		case Centred:
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
			QWMatrix matrix;
			matrix.scale( sx, sy );
			*bgPixmap = wpPixmap->xForm( matrix );
		    }
		    break;
		    
		case Tiled:
		    *bgPixmap = *wpPixmap;
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

