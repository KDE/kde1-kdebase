//-----------------------------------------------------------------------------
//
// KPixmap
//
// Copyright (c)  Mark Donohoe 1998
//

#ifndef __KPIXMAP_H__
#define __KPIXMAP_H__

#include <qpixmap.h>

const int WebOnly 	= 0x00000100;
const int LowOnly	= 0x00000200;

class KPixmap : public QPixmap
{
public:
	enum ColorMode { Auto, Color, Mono, LowColor, WebColor };
    
	KPixmap() {};
	~KPixmap() {};
    
	void gradientFill( QColor color1, QColor color2, bool upDown = TRUE );
	void patternFill( QColor color1, QColor color2, uint pattern[8] );
			
	bool convertFromImage( const QImage &img, int conversion_flags );
	bool convertFromImage( const QImage &img, ColorMode mode=WebColor );
	
	
	bool load( const char *fileName, const char *format, int conversion_flags );
	bool load( const char *fileName, const char *format=0,
		ColorMode mode=WebColor );
		
	bool checkColorTable(const QImage &image);	
};

#endif

