    /*

    Small util to draw a nice background in Xsetup_*
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen
                             stefh@mip.ou.dk


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */
 

#include <kapp.h>
#include <qstring.h>
#include <qimage.h>
#include <qpainter.h>
#include <qfile.h>
#include <qtstream.h>
#include <qpainter.h>
#include <kconfig.h>

// NEW 26-2-97 added by Henk Punt < h.punt@wing.rug.nl > :
#include <math.h>
//#include "gif.h"
#include "jpeg.h"

#include <X11/Xlib.h>

class DesktopConfig {
public:
     DesktopConfig();
     ~DesktopConfig();
     QString bgpic;
     bool bgpictile;
     bool bgpiccenter;
     bool fancybg;
     bool have_bg_pic;
     QColor bgcolor;
protected:
     KConfig*       kc;
};

DesktopConfig::DesktopConfig()
{
     kc = kapp->getConfig();
     QString _bgcolor;
     have_bg_pic = false;
     kc->setGroup( "KDMDESKTOP");

     fancybg  = false;
     if( kc->hasKey( "BackgroundPicture")) {
	  bgpic = kc->readEntry( "BackgroundPicture");
	  bgpictile = (bool) kc->readNumEntry( "BackgroundPictureTile",0);
	  bgpiccenter = (bool) kc->readNumEntry( "BackgroundPictureCenter",0);
	  if(bgpic.isEmpty())
	    have_bg_pic = false;
	  else
	    have_bg_pic = true;
	    
     } else{
	  have_bg_pic = false;
     }
     if( kc->hasKey("FancyBackground")) {
	  fancybg          = (bool) kc->readNumEntry("FancyBackground",0);
     } 
     _bgcolor  = kc->readEntry( "BackgroundColor");
     if( !_bgcolor.isNull())
	  bgcolor.setNamedColor( _bgcolor.data());
     else
	  bgcolor = darkCyan;
     //delete kc;
}

DesktopConfig::~DesktopConfig()
{
}
// NEW 26-2-97 added by Henk Punt < h.punt@wing.rug.nl > :
static void 
do_picture_background( DesktopConfig* dc)
{
  /*     QImageIO::defineIOHandler("GIF", "^GIF[0-9][0-9][a-z]", 0, 
			       read_gif_file, NULL);*/
     QImageIO::defineIOHandler("JFIF","^\377\330\377\340..JFIF", 0,
			       read_jpeg_jfif, NULL);
     
     QWidget* desktop = qApp->desktop();
     int appWidth  =  desktop->width();
     int appHeight =  desktop->height();
     
     //QColor::enterAllocContext();
     QPixmap image( dc->bgpic);
     if( image.isNull())
	  return;
     QPixmap tmp( appWidth, appHeight);
     
     int imWidth = image.width();
     int imHeight = image.height();
     
     int cx = appWidth/2-imWidth/2;
     int cy = appHeight/2-imHeight/2;
     
     if((imWidth>=appWidth)&&(imHeight>=appHeight)) { 
	  // picture bigger than app
	  if( dc->bgpictile) 
	       bitBlt( &tmp, cx, cy, 
		       &image, 0, 0, imWidth, imHeight);
	  else 
	       bitBlt( &tmp, 0, 0,
		       &image, 0, 0, imWidth, imHeight);
     } else { //picture smaller than app.  
	  if(( dc->bgpictile)&&( !dc->bgpiccenter)) {
	       tmp = image;
	  }
	  if(( !dc->bgpictile)&&( dc->bgpiccenter)) {
	       tmp.fill( dc->bgcolor);
	       bitBlt( &tmp, cx, cy, 
		       &image, 0, 0, imWidth, imHeight);
	  }
	  if(( !dc->bgpictile)&&( !dc->bgpiccenter)) {
	       tmp.fill( dc->bgcolor);
	       bitBlt( &tmp, 0, 0,
		       &image, 0, 0, imWidth, imHeight);
	  }
	  if(( dc->bgpictile)&&( dc->bgpiccenter)) {
	       int start_x=cx-imWidth*((int)(ceil((cx)/(float)imWidth)));
	       int start_y=cy-imHeight*((int)(ceil((cy)/(float)imHeight)));
	       
	       int filled_x=start_x;
	       int filled_y=start_y;
	       while(filled_y<appHeight) {
		    while(filled_x<appWidth) {
			 bitBlt( &tmp, filled_x , filled_y, 
				 &image, 0,0, imWidth, imHeight);
			 filled_x+=imWidth;
		    }
		    filled_x=start_x;
		    filled_y+=imHeight;
	       }
	  }
     }
     desktop->setBackgroundPixmap( tmp);
     //QColor::leaveAllocContext();
}

static void
do_fancy_background()
{
     QWidget* desktop = qApp->desktop();
     int appWidth  =  desktop->width();
     int appHeight =  desktop->height();
     int offset;
     int w = appWidth+appHeight/2;
     int h = appHeight+appHeight/2;
     QImage image( w, h, 8, 128 );             // create image
     QPixmap pm;
     int i;
     int tmp;
     QFont f( "utopia", 72, QFont::Black );
     QString title = "KDE Desktop Environment";

     for ( i=0; i<128; i++ )                   // build color table
	  image.setColor( i, qRgb(0,0,i*3/4) );
     for (offset=0;offset<1; offset+=2){
	  for ( int y=0; y<h; y++ ) {                     // set image pixels
	       uchar *p = image.scanLine(y);
	       for ( int x=0; x<w; x++ ){
		    tmp = ((int)(((float)(y)/(float)(appHeight/2))*128.0)+offset)%256;
		    if (tmp>=128)
			 tmp = 255-tmp;
		    *p++ = tmp;
	       }
	  }
	  pm = image; // convert image to pixmap
	  {
	       QPainter p;                           
	       p.begin( &pm );                       
	       f.setStyleHint( QFont::Times );
	       p.setFont(f);
	       QRect br = p.fontMetrics().boundingRect( title+"I");
	       p.rotate (45);
	       int y = -appHeight - offset;
	       int x = -appWidth; //  + offset;
	       while (y < 2 * appHeight){
		    tmp = ((int)(((float)(y+2*appHeight)/(float)(appHeight/2))*128.0)+offset)%256;
		    if (tmp>=128)
			 tmp = 255-tmp;
		    p.setPen(image.color(tmp));
		    p.drawText(x + 2, y + 2, title.data());
		    tmp +=10;
		    if (tmp>=128)
			 tmp = 255-tmp;
		    p.setPen(image.color(tmp));
		    p.drawText(x, y, title.data());
		    x += br.width();
		    if (x > 2 * appWidth){
			 x -= 3 * appWidth + br.width();
			 y += (int) (br.height() * 1.5);
		    }
	       }
	       p.end();
	  }
     }
     //bitBlt( desktop, 0, 0, pm, 0, 0, appWidth, appHeight);
     desktop->setBackgroundPixmap( pm);
}

static void 
do_normal_background( DesktopConfig *dc)
{
     QWidget* desktop = qApp->desktop();
     desktop->setBackgroundColor( dc->bgcolor);
}

int main(int argc, char **argv)
{
     // Use same config file as kdm:
     KApplication app(argc, argv, "kdm");
     
     //Keep color resources after termination
     XSetCloseDownMode( qt_xdisplay(), RetainTemporary);
     DesktopConfig* dc = new DesktopConfig();
     
     if( dc->fancybg)
	  do_fancy_background();
     else if( dc->have_bg_pic )
	  do_picture_background( dc);
     else
	  do_normal_background( dc);
     
     XFlush( qt_xdisplay());
     return 0;
}
