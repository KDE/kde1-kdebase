#include "gradientFill.h"
#include <dither.h>
#include <qimage.h>
#include <stdio.h>
#include <endian.h>

void kwm_gradientFill(KPixmap &pm, QColor ca, QColor cb, bool upDown) {
  if(upDown == FALSE && QColor::numBitPlanes() >= 15) {    
    int w = pm.width();
    int h = pm.height();
    
    int c_red_a = ca.red() << 16;
    int c_green_a = ca.green() << 16;
    int c_blue_a = ca.blue() << 16;

    int c_red_b = cb.red() << 16;
    int c_green_b = cb.green() << 16;
    int c_blue_b = cb.blue() << 16;
    
    int d_red = (c_red_b - c_red_a) / w;
    int d_green = (c_green_b - c_green_a) / w;
    int d_blue = (c_blue_b - c_blue_a) / w;

    QImage img(w, h, 32);
    uchar *p = img.scanLine(0);

    int r = c_red_a, g = c_green_a, b = c_blue_a;
    for(int x = 0; x < w; x++) {
#if BYTE_ORDER == BIG_ENDIAN
      *p++ = 0;
      *p++ = b >> 16;
      *p++ = g >> 16;
      *p++ = r >> 16;
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
      *p++ = r >> 16;
      *p++ = g >> 16;
      *p++ = b >> 16;
      *p++ = 0;
#endif

      r += d_red;
      g += d_green;
      b += d_blue;
    }

    uchar *src = img.scanLine(0);
    for(int y = 1; y < h; y++)
      memcpy(img.scanLine(y), src, 4*w);

    pm.convertFromImage(img);
  } else
    pm.gradientFill(ca, cb, upDown);
}
