#include <stdio.h>
#include <string.h>
#include <qiodev.h>
#include <qcolor.h>
#include <qfile.h>
#include <qwmatrix.h>

#include "xview.h"
#include <config-kfm.h>

void write_xv_file( const char *_filename, QPixmap &_pixmap )
{
    // debugT("Saving to '%s'\n",_filename);
    
    QFile f( _filename );
    if ( !f.open( IO_WriteOnly ) )
	return;
 
    if ( _pixmap.isNull() )
    {
	f.close();
	return;
    }
    
    int w, h;
    
    if ( _pixmap.width() > _pixmap.height() )
    {
	if ( _pixmap.width() < 80 )
	    w = _pixmap.width();
	else
	    w = 80;

	h = (int)( (float)_pixmap.height() * ( (float)w / (float)_pixmap.width() ) );
    }
    else
    {
	if ( _pixmap.height() < 60 )
	    h = _pixmap.height();
	else
	    h = 60;

	w = (int)( (float)_pixmap.width() * ( (float)h / (float)_pixmap.height() ) );
    }
    
    QWMatrix matrix;
    matrix.scale( (float)w/_pixmap.width(), (float)h/_pixmap.height() );
    QPixmap tp = _pixmap.xForm( matrix );   
      
    char str[ 1024 ];

    // magic number must be "P7 332"
    f.writeBlock( "P7 332\n", 7 );

    // next line #XVVERSION
    f.writeBlock( "#XVVERSION:\n", 12 );

    // now it gets interesting, #BUILTIN means we are out.
    // if IMGINFO comes, we are happy!
    f.writeBlock( "#IMGINFO:\n", 10 );
    
    // after this an #END_OF_COMMENTS signals everything to be ok!
    f.writeBlock( "#END_OF_COMMENTS:\n", 18 );

    // now a last line with width, height, maxval which is supposed to be 255
    sprintf( str, "%i %i 255\n", w, h );
    f.writeBlock( str, strlen( str ) );

    QImage image = tp.convertToImage();
    if ( image.depth() == 1 )
    {
	// debugT("Converted image\n");
	image = image.convertDepth( 8 );
    }
    
    uchar buffer[ 128 ];

    // debugT("Image of depth %i\n",image.depth());
    
    for ( int py = 0; py < h; py++ )
    {
	uchar *data = image.scanLine( py );
	for ( int px = 0; px < w; px++ )
	{
	    int r, g, b;
	    if ( image.depth() == 32 )
	    {
		QRgb *data32 = (QRgb*) data;
		r = qRed( *data32 ) >> 5;
		g = qGreen( *data32 ) >> 5;		
		b = qBlue( *data32 ) >> 6;
		data += sizeof( QRgb );
	    }
	    else 
	    {
		QRgb color = image.color( *data );
		r = qRed( color ) >> 5;
		g = qGreen( color ) >> 5;		
		b = qBlue( color ) >> 6;
		data++;
	    }
	    buffer[ px ] = ( r << 5 ) | ( g << 2 ) | b;
	}
	f.writeBlock( (const char*)buffer, w );
    }
    
    f.close();
}





