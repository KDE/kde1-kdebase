/*
 * bg.cpp.  Part of the KDE Project.
 *
 * Copyright (C) 1997 Martin Jones
 *
 * Gradient fill added by Mark Donohoe 1997
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
#include <dither.h>

#include "bg.h"
#include "bg.moc"

//----------------------------------------------------------------------------

#define NO_WALLPAPER	"(none)"

//----------------------------------------------------------------------------

class GPixmap : public QPixmap
{
public:
    GPixmap();
    void gradientFill(QColor color1, QColor color2, bool updown = TRUE, 
	int num_colors = 8);
private:
    QPixmap cropped_pm;
    QPixmap full_pm;
    int x_size;
    int y_size;
    QColor col;
    int h, s;
    int red1, green1, blue1;
    int red_dif, green_dif, blue_dif;
    float rat;
    uint *p;
    uint rgbcol;
    int steps;
};

GPixmap::GPixmap()
{}

void GPixmap::gradientFill(QColor color1, QColor color2, bool updown, int num_colors)
{    
    if(updown)
    	y_size=height();
    else
    	y_size=width();
        
    cropped_pm.resize( 30, y_size );
 
    QImage image(30, y_size, 32);
	

    red1 = color1.red();
    green1 = color1.green();
    blue1 = color1.blue();
	
    red_dif = color2.red()-color1.red();
    green_dif = color2.green()-color1.green();
    blue_dif = color2.blue()-color1.blue();

    for ( s = y_size-1; s > 0; s-- )
    {
	p = (uint *) image.scanLine( y_size - s - 1 );
	rat=1.0*s/y_size;
	col.setRgb( red1+(int)(red_dif*rat),
	    green1+(int)(green_dif*rat), blue1+(int)(blue_dif*rat) );
	rgbcol= col.rgb();

	for( h = 0; h < 30; h++ ) {
	    *p = rgbcol;
	    p++;
	}
    }
	
    if(num_colors<2 || num_colors >256) {
	num_colors=8;
    }
	
    QColor *ditherPalette = new QColor[num_colors];
    for( s=0; s<num_colors; s++) {
	ditherPalette[s].setRgb(red1+red_dif*s/(num_colors-1),
	green1+green_dif*s/(num_colors-1),
	blue1+blue_dif*s/(num_colors-1) );
    }
	
    kFSDither dither( ditherPalette, num_colors );
    QImage tImage = dither.dither( image );
    cropped_pm.convertFromImage( tImage );
	
    if(updown)
	steps=width()/20+1;
    else
	steps=height()/20+1;
	
    QPainter p;
    p.begin( this );
    if(updown)	
	for(s=0;s<steps;s++)
	    p.drawPixmap(20*s, 0,cropped_pm, 5, 0 , 20, y_size );
    else {
	QWMatrix matrix;
        matrix.translate( (float)width()-1.0, 0.0);
       	matrix.rotate( 90.0 );
       	p.setWorldMatrix( matrix );
       	for(s=0;s<steps;s++)
	p.drawPixmap(20*s, 0,cropped_pm, 5, 0 , 20, y_size );
    }
    p.end();

    delete [] ditherPalette;
}	 

//----------------------------------------------------------------------------

KBackground::KBackground()
{
    wpMode = Tiled;
    gfMode = Flat;
    orMode = Portrait;

    bgPixmap = 0;
    applied = false;

    hasPm = false;
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

    KConfig config( cFile );

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

    wallpaper = NO_WALLPAPER;
    str = config.readEntry( "Wallpaper" );
    if ( !str.isEmpty() )
	wallpaper = str;

    /*
     * Create a name for this desktop based on its configuration.
     * This will be used for caching.
     */
    if ( wallpaper != NO_WALLPAPER )
    {
	name.sprintf( "%s_%d_#%02x%02x%02x", wallpaper.data(), wpMode,
	    color1.red(), color1.green(), color1.blue() );
	hasPm = true;
    }
    else
	name.sprintf( "#%02x%02x%02x_#%02x%02x%02x_%d_%d", color1.red(),
	    color1.green(), color1.blue(), color2.red(), color2.green(),
	    color2.blue(), gfMode, orMode );
}

QPixmap *KBackground::loadWallpaper()
{
    if ( wallpaper == NO_WALLPAPER )
	return 0;

    QString filename;

    if ( wallpaper[0] != '/' )
    {
	filename = kapp->kdedir().copy();
	filename += "/share/wallpapers/";
	filename += wallpaper;
    }
    else
	filename = wallpaper;

    QPixmap *wpPixmap = new QPixmap;

    if ( wpPixmap->load( filename ) == FALSE )
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

    if ( wpPixmap )
    {
	int w = QApplication::desktop()->width();
	int h = QApplication::desktop()->height();

	bgPixmap = new QPixmap;

	switch ( wpMode )
	{
	    case Centred:
		{
		    bgPixmap->resize( w, h );
		    bgPixmap->fill( color1 );
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

	    default:
		*bgPixmap = *wpPixmap;
	}

	delete wpPixmap;
	wpPixmap = 0;

	// background switch is deferred in case the user switches
	// again while the background is loading
	startTimer( 0 );
    }
    else
    {
	if ( gfMode == Gradient )
	{
	    int numColors = 8;
	    if ( QColor::numBitPlanes() > 8 )
		numColors = 16;

	    GPixmap pmDesktop;

	    if ( orMode == Portrait )
	    {
		pmDesktop.resize( 15, QApplication::desktop()->height() );
		pmDesktop.gradientFill( color2, color1, true, numColors );
	    }
	    else
	    {
		pmDesktop.resize( QApplication::desktop()->width(), 20 );
		pmDesktop.gradientFill( color2, color1, false, numColors );
	    }

	    bgPixmap = new QPixmap();
	    *bgPixmap = pmDesktop;

	    // background switch is deferred in case the user switches
	    // again while the background is loading
	    startTimer( 0 );
	}
	else
	{
	    qApp->desktop()->setBackgroundColor( color1 );
	    applied = true;
	}
    }
}

void KBackground::cancel()
{
    killTimers();
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

