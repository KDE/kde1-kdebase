
//
// KDE Display background setup module
//
// Copyright (c)  Martin R. Jones 1996
//
// Gradient fill added by Mark Donohoe 1997
//
// Converted to a kcc module by Matthias Hoelzer 1997
//

#include <qgrpbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qfiledlg.h>
#include <qradiobt.h>
#include <qpainter.h>
#include <qlayout.h>
#include <kapp.h>
#include <stdlib.h>
#include <qimage.h>

#include "kpixmap.h"

static bool kdither_32_to_8( const QImage *src, QImage *dst )
{
    register QRgb *p;
    uchar  *b;
    int	    y;
	
    if ( !dst->create(src->width(), src->height(), 8, 256) ) {
		return FALSE;
	}

    int ncols = 256;

    static uint bm[16][16];
    static int init=0;
    if (!init) {
		// Build a Bayer Matrix for dithering

		init = 1;
		int n, i, j;

		bm[0][0]=0;

		for (n=1; n<16; n*=2) {
	    	for (i=0; i<n; i++) {
			for (j=0; j<n; j++) {
		    	bm[i][j]*=4;
		    	bm[i+n][j]=bm[i][j]+2;
		    	bm[i][j+n]=bm[i][j]+3;
		    	bm[i+n][j+n]=bm[i][j]+1;
			}
	    	}
		}

		for (i=0; i<16; i++)
	    	for (j=0; j<16; j++)
			bm[i][j]<<=8;
    }

    dst->setNumColors( ncols );

    // quantization needed

#define MAX_R 2
#define MAX_G 2
#define MAX_B 2
#define INDEXOF(r,g,b) (((r)*(MAX_G+1)+(g))*(MAX_B+1)+(b))

	int rc, gc, bc;

	for ( rc=0; rc<=MAX_R; rc++ )		// build 2x2x2 color cube
	    for ( gc=0; gc<=MAX_G; gc++ )
		for ( bc=0; bc<=MAX_B; bc++ ) {
		    dst->setColor( INDEXOF(rc,gc,bc),
			qRgb( rc*255/MAX_R, gc*255/MAX_G, bc*255/MAX_B ) );
		}	

	int sw = src->width();

#define DITHER(p,d,m) ((uchar) ((((256 * (m) + (m) + 1)) * (p) + (d)) / 65536 ))

	for ( y=0; y < src->height(); y++ ) {
	    p = (QRgb *)src->scanLine(y);
	    b = dst->scanLine(y);
	    QRgb *end = p + sw;

	    // perform quantization
		int x = 0;
		while ( p < end ) {
		    uint d = bm[y&15][x&15];

		    rc = qRed( *p );
		    gc = qGreen( *p );
		    bc = qBlue( *p );

		    *b++ =
			INDEXOF(
			    DITHER(rc, d, MAX_R),
			    DITHER(gc, d, MAX_G),
			    DITHER(bc, d, MAX_B)
			);

		    p++;
		    x++;
		}
	}
	
#undef DITHER

#undef MAX_R
#undef MAX_G
#undef MAX_B
#undef INDEXOF

    return TRUE;
}

//----------------------------------------------------------------------------

void KPixmap::gradientFill( QColor color1, QColor color2, bool upDown )
{
	QPixmap cropped_pm;
    int y_size, steps;;
	float ratio;
	QColor col;
	uint *p, rgbcol;

    if ( upDown )
    	y_size=height();
    else
    	y_size=width();
        
    cropped_pm.resize( 40, y_size );
 
 	QImage image(40, y_size, 32);
	
	int red1 = color1.red();
	int green1 = color1.green();
	int blue1 = color1.blue();
	
	int red_dif = color2.red()-color1.red();
	int green_dif = color2.green()-color1.green();
	int blue_dif = color2.blue()-color1.blue();

	for ( int s = y_size-1; s > 0; s-- )
	{
		p = (uint *) image.scanLine( y_size - s - 1 );
		ratio = 1.0 * s / y_size;
		
		col.setRgb( red1 + (int) ( red_dif*ratio ) ,
					green1 + (int) ( green_dif*ratio ),
					blue1 + (int) ( blue_dif*ratio )	);
		
		rgbcol = col.rgb();
		
		for( int h = 0; h < 40; h++ ) {
			*p = rgbcol;
			p++;
		}
	}
		
	if ( QColor::numBitPlanes() == 8 ) {
		QImage tImage( image.width(), image.height(), 8, 256 );
		kdither_32_to_8( &image, &tImage );
		cropped_pm.convertFromImage( tImage );
	} else
		cropped_pm.convertFromImage( image );
		
	
	if ( upDown )
		steps = width() / 20 + 1;
	else
		steps = height() / 20 + 1;
	
	QPainter painter;
	painter.begin( this );
	
	if ( upDown )	
		for( int s=0; s<steps; s++ )
			painter.drawPixmap( 20*s, 0, cropped_pm, 10, 0, 20, y_size );
	else {
		QWMatrix matrix;
        matrix.translate( (float) width() - 1.0, 0.0  );
       	matrix.rotate( 90.0 );
       	painter.setWorldMatrix( matrix );
       	for ( int s=0; s<steps; s++ )
			painter.drawPixmap( 20*s, 0, cropped_pm, 10, 0, 20, y_size );
    }
    
	painter.end();
}

void KPixmap::patternFill( QColor color1, QColor color2, uint pattern[8] )
{
    QPixmap tile( 8, 8 );
    tile.fill( color2 );
	
    QPainter pt;
    pt.begin( &tile );
    pt.setBackgroundColor( color2 );
    pt.setPen( color1 );
    
    for ( int y = 0; y < 8; y++ ) {
		uint v = pattern[y];
		for ( int x = 0; x < 8; x++ ) {
	    	if ( v & 1 )
				pt.drawPoint( 7 - x, y );
	    	v /= 2;
		}
    }
    
    pt.end();

    int sx, sy = 0;
    while ( sy < height() ) {
		sx = 0;
		while (sx < width()) {
	    	bitBlt( this, sx, sy, &tile, 0, 0, 8, 8 );
	    	sx += 8;
		}
		sy += 8;
    }
}

bool KPixmap::load( const char *fileName, const char *format,
		    int conversion_flags )
{
    QImageIO io( fileName, format );

    bool result = io.read();
	
    if ( result ) {
	detach();
	result = convertFromImage( io.image(), conversion_flags );
    }
    return result;
}

bool KPixmap::load( const char *fileName, const char *format,
		    ColorMode mode )
{
    int conversion_flags = 0;
    switch (mode) {
      case Color:
	conversion_flags |= ColorOnly;
	break;
      case Mono:
	conversion_flags |= MonoOnly;
	break;
		case LowColor:
	conversion_flags |= LowOnly;
	break;
	  case WebColor:
	conversion_flags |= WebOnly;
	break;
      default:
	;// Nothing.
    }
    return load( fileName, format, conversion_flags );
}

bool KPixmap::convertFromImage( const QImage &img, ColorMode mode )
{
int conversion_flags = 0;
    switch (mode) {
      case Color:
	conversion_flags |= ColorOnly;
	break;
      case Mono:
	conversion_flags |= MonoOnly;
	break;
		case LowColor:
	conversion_flags |= LowOnly;
	break;
	  case WebColor:
	conversion_flags |= WebOnly;
	break;
      default:
	;	// Nothing.
    }
    return convertFromImage( img, conversion_flags );
}

bool KPixmap::convertFromImage( const QImage &img, int conversion_flags  )
{
	if ( img.isNull() ) {
#if defined(CHECK_NULL)
	warning( "KPixmap::convertFromImage: Cannot convert a null image" );
#endif
	return FALSE;
    }
    detach();					// detach other references
    
	if( ( conversion_flags != LowOnly && conversion_flags != WebOnly ) 
		||  QColor::numBitPlanes() > 8 ) {
		return QPixmap::convertFromImage ( img, conversion_flags );
	}
		
	if( conversion_flags == LowOnly ) {
		
		// If image uses icon palette don't dither it.
		if( img.numColors() > 0 && img.numColors() <=40 ) {
			if ( checkColorTable( img ) ) {
				return QPixmap::convertFromImage( img, QPixmap::Auto );
			}
		}
		
	
		QImage  image = img.convertDepth(32);
		
		QImage tImage( image.width(), image.height(), 8, 256 );
		kdither_32_to_8( &image, &tImage );
		
		return QPixmap::convertFromImage ( tImage);
	} else {
		QImage  image = img.convertDepth(32);
		return QPixmap::convertFromImage ( image, conversion_flags );
	}
}

bool KPixmap::checkColorTable( const QImage &image ) 
{
	QColor *iconPalette = new QColor[40];
	int i = 0;
	
	int ncols = image.numColors();

	// Standard palette
    iconPalette[i++] = red;
    iconPalette[i++] = green;
    iconPalette[i++] = blue;
    iconPalette[i++] = cyan;
    iconPalette[i++] = magenta;
    iconPalette[i++] = yellow;
    iconPalette[i++] = darkRed;
    iconPalette[i++] = darkGreen;
    iconPalette[i++] = darkBlue;
    iconPalette[i++] = darkCyan;
    iconPalette[i++] = darkMagenta;
    iconPalette[i++] = darkYellow;
    iconPalette[i++] = white;
    iconPalette[i++] = lightGray;
    iconPalette[i++] = gray;
    iconPalette[i++] = darkGray;
    iconPalette[i++] = black;

	// Pastels
	iconPalette[i++] = QColor( 255, 192, 192 );
	iconPalette[i++] = QColor( 192, 255, 192 );
	iconPalette[i++] = QColor( 192, 192, 255 );
	iconPalette[i++] = QColor( 255, 255, 192 );
	iconPalette[i++] = QColor( 255, 192, 255 );
	iconPalette[i++] = QColor( 192, 255, 255 );

	// Reds
	iconPalette[i++] = QColor( 64,   0,   0 );
	iconPalette[i++] = QColor( 192,  0,   0 );

	// Oranges
	iconPalette[i++] = QColor( 255, 128,   0 );
	iconPalette[i++] = QColor( 192,  88,   0 );
	iconPalette[i++] = QColor( 255, 168,  88 );
	iconPalette[i++] = QColor( 255, 220, 168 );

	// Blues
	iconPalette[i++] = QColor(   0,   0, 192 );

	// Turquoise
	iconPalette[i++] = QColor(   0,  64,  64 );
	iconPalette[i++] = QColor(   0, 192, 192 );

	// Yellows
	iconPalette[i++] = QColor(  64,  64,   0 );
	iconPalette[i++] = QColor( 192, 192,   0 );

	// Greens
	iconPalette[i++] = QColor(   0,  64,   0 );
	iconPalette[i++] = QColor(   0, 192,   0 );

	// Purples
	iconPalette[i++] = QColor( 192,   0, 192 );

	// Greys
	iconPalette[i++] = QColor(  88,  88,  88 );
	iconPalette[i++] = QColor(  48,  48,  48 );
	iconPalette[i++] = QColor( 220, 220, 220 );

	int j;
	QRgb* ctable = image.colorTable();
	
	// Allow one failure = transparent background
	int failures = 0;
	
	for ( i=0; i<ncols; i++ ) {
		for ( j=0; j<40; j++ ) {
			if ( iconPalette[j].red() == qRed( ctable[i] ) &&
				 iconPalette[j].green() == qGreen( ctable[i] ) &&
				 iconPalette[j].blue() == qBlue( ctable[i] ) ) {
				break;
			}
		}

		if ( j == 40 ) {
			failures ++;			
		}
	}
	
	if( failures > 1 )
		return FALSE;
	else
		return TRUE;
}
