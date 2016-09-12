    /*

    Small util to draw a nice background in Xsetup_*
    $Id: kdmdesktop.cpp,v 1.11 1998/10/24 20:17:30 bieker Exp $

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
#include <kpixmap.h>

#include <config.h>
#include <sys/types.h>

#include <kimgio.h>

#include <X11/Xlib.h>

enum { NoPic, Tile, Center, Scale, TopLeft, TopRight,
         BottomLeft, BottomRight, Fancy, Plain, Vertical, Horizontal  };

class DesktopConfig {
public:
     DesktopConfig();
     ~DesktopConfig();

     int pictureMode() { return bgmode; }
     int colorMode() { return cmode; }
     QString bgpic;
     bool have_bg_pic;
     QColor bgcolor1, bgcolor2;
protected:
     KConfig*       kc;
     int bgmode, cmode;
};

DesktopConfig::DesktopConfig()
{
     kc = kapp->getConfig();
     QString strmode;
     have_bg_pic = false;
     kc->setGroup( "KDMDESKTOP");

     strmode = kc->readEntry( "BackGroundColorMode", "Plain");
     if(strmode == "Plain")
       cmode = Plain;
     else if(strmode == "Horizontal")
       cmode = Horizontal;
     else if(strmode == "Vertical")
       cmode = Vertical;
     else
       cmode = Plain;

     if( kc->hasKey( "BackGroundPicture")) {
	  bgpic = kc->readEntry( "BackGroundPicture");
	  strmode = kc->readEntry( "BackGroundPictureMode", "Scale");
          if(strmode == "None")
	    bgmode = NoPic;
          else if(strmode == "Tile")
	    bgmode = Tile;
          else if(strmode == "Center")
	    bgmode = Center;
          else if(strmode == "Scale")
	    bgmode = Scale;
          else if(strmode == "TopLeft")
	    bgmode = TopLeft;
          else if(strmode == "TopRight")
	    bgmode = TopRight;
          else if(strmode == "BottomLeft")
	    bgmode = BottomLeft;
          else if(strmode == "BottomRight")
	    bgmode = BottomRight;
          else if(strmode == "Fancy")
	    bgmode = Fancy;
          else
            bgmode = Tile;

	  if(bgpic.isEmpty() || bgmode == None)
	    have_bg_pic = false;
	  else
	    have_bg_pic = true;
	    
     } else{
	  have_bg_pic = false;
     }

     bgcolor1  = kc->readColorEntry( "BackGroundColor1", &darkCyan);
     bgcolor2  = kc->readColorEntry( "BackGroundColor2", &darkBlue);
}

DesktopConfig::~DesktopConfig()
{
}

// NEW 26-2-97 added by Henk Punt < h.punt@wing.rug.nl > :
static void 
do_picture_background( DesktopConfig* dc)
{
     kimgioRegister();
     
     QWidget* desktop = qApp->desktop();
     KPixmap image;
     int appWidth  =  desktop->width();
     int appHeight =  desktop->height();
     
     //QColor::enterAllocContext();

     if(dc->have_bg_pic)
     {
       image.load( dc->bgpic);
       if( image.isNull())
	    dc->have_bg_pic = false;
     }
     KPixmap tmp;
     tmp.resize( appWidth, appHeight);
     
     int imWidth = image.width();
     int imHeight = image.height();
     
     int cx = appWidth/2-imWidth/2;
     int cy = appHeight/2-imHeight/2;
     
     if(dc->pictureMode() != Tile && dc->pictureMode() != Scale)
     {
       switch(dc->colorMode())
       {
         case Horizontal:
           tmp.gradientFill(dc->bgcolor1, dc->bgcolor2, false);
           break;
         case Vertical:
           tmp.gradientFill(dc->bgcolor1, dc->bgcolor2, true);
           break;
         case Plain:
         default:
           tmp.fill(dc->bgcolor1);
           break;
       }
     }

     if((imWidth>=appWidth)&&(imHeight>=appHeight)) { 
	  // picture bigger than app
	  if( dc->pictureMode() == Tile) 
	       bitBlt( &tmp, cx, cy, 
		       &image, 0, 0, imWidth, imHeight);
	  else 
	       bitBlt( &tmp, 0, 0,
		       &image, 0, 0, imWidth, imHeight);
     } else { //picture smaller than app.  
	switch ( dc->pictureMode() )
	{
	  case Tile:
	       tmp = image;
	       break;
	  case Center:
	       bitBlt( &tmp, cx, cy, 
		       &image, 0, 0, imWidth, imHeight);
	       break;
	  case Scale: {
	       QImage i;
               i = image;
	       image.convertFromImage(i.smoothScale(appWidth, appHeight), 0);
	       bitBlt( &tmp, 0, 0,
		       &image, 0, 0, appWidth, appHeight);
	       break;
	  }
	  case TopLeft:
	       bitBlt( &tmp, 0, 0,
		       &image, 0, 0, imWidth, imHeight);
	       break;
	  case TopRight:
	       bitBlt( &tmp, appWidth-imWidth, 0,
		       &image, 0, 0, imWidth, imHeight);
	       break;
	  case BottomRight:
	       bitBlt( &tmp, appWidth-imWidth, appHeight-imHeight,
		       &image, 0, 0, imWidth, imHeight);
	       break;
	  case BottomLeft:
	       bitBlt( &tmp, 0, appHeight-imHeight,
		       &image, 0, 0, imWidth, imHeight);
	       break;
	  default:
	       break;
	}
/*
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
*/
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

int main(int argc, char **argv)
{
     // Use same config file as kdm:
     KApplication app(argc, argv, "kdm");
     
     //Keep color resources after termination
     XSetCloseDownMode( qt_xdisplay(), RetainTemporary);
     DesktopConfig* dc = new DesktopConfig();
     
     if( dc->pictureMode() == Fancy)
	  do_fancy_background();
     else
	  do_picture_background( dc);
     
     XFlush( qt_xdisplay());

     return 0;
}
