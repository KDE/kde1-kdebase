//
// Patrick D. Dowler (c) 1998
// 
// License: GPL
//

#include <qlabel.h>
#include <qsize.h>
#include <kslider.h>

#include "kslidercontrol.h"

KSliderControl::KSliderControl(const char *label, 
		 const int lower, const int upper,
		 const int step, const int value,
		 const char *units,
		 QWidget *parent, const char *name)
  : QWidget( parent, name )
{
  h_spacing = v_spacing = 10; // 10 pixels of space between widgets
  lower_bound = lower;
  upper_bound = upper;

  // allocate GUI objects
  main_label = new QLabel( label, this );
  slider = new KSlider( lower, upper, step, value, KSlider::Horizontal, this );
  value_label = new QLabel( this );
  unit_str = new QString( units );
  connect( slider, SIGNAL( valueChanged(int) ), SLOT( resetValueField(int) ));


  // default values
  int major = (upper-lower)/10; // default to 10 major ticks
  slider->setSteps( major/2, major );

  setLabelAlignment( AlignCenter );

  setLabelSize(0.3); // this should be after value gets set (after connect)
}

void KSliderControl::setSteps(int minor, int major)
{
  slider->setSteps( minor, major );
}
void KSliderControl::setValue(int val)
{
  slider->setValue(val);
}
int KSliderControl::intValue()
{
  return slider_value;
}

void KSliderControl::setLabelAlignment(int a)
{
  label_align = a;
}

void KSliderControl::setLabelSize(float frac)
{
  label_frac = frac;
  computeMinimumSize();
}

void KSliderControl::resetValueField(int val)
{
  slider_value = val;
  QString str;
  str.setNum(val);
  str.append(" ");
  str.append(*unit_str);
  value_label->setText(str );
}

QSize KSliderControl::minimumSize() const
{
  return qs;
}

void KSliderControl::computeMinimumSize()
{
  main_label->adjustSize();

  int tmp = slider->value();
  resetValueField(upper_bound); // use max value to set field size
  value_label->adjustSize();
  resetValueField(tmp);

  // min horizontal size must fit the main label in the leftmost
  // w/label_frac pixels and the value label in the remaining pixels
  int wL = (int) ( main_label->width() / label_frac );
  int wR = (int) ( value_label->width() / (1.0-label_frac) );
  int w = wL > wR ? wL : wR;
  w += h_spacing;

  // added a fudge factor because the height() after adjustSize isn't
  // quite big enough
  int h = (int) (1.5*slider->height()) + value_label->height() + v_spacing;
  qs.setWidth(w);
  qs.setHeight(h);
  setMinimumSize(w,h);
}

void KSliderControl::resizeEvent ( QResizeEvent * )
{
  int w = width();
  int h = height();

  int left_frac = (int) ( w*label_frac );

  // label gets placed according to alignment and label_frac
  int lx = 0;
  if ( label_align == AlignLeft )
    lx = 0;
  else if (label_align == AlignRight)
    lx = left_frac - main_label->width() - h_spacing/2;
  else
    lx = ( left_frac - main_label->width() )/2;
  int ly = ( h - main_label->height() )/2;
  main_label->move(lx, ly);

  // slider gets stretched horizontally to fill remainder
  int sw = w - left_frac - h_spacing/2;
  int sh = slider->height(); // keep height
  int sx = left_frac + h_spacing/2;
  int sy = h/2 - sh - v_spacing/2;
  //  slider->setGeometry(sx, sy, sw, sh);
  slider->move(sx, sy);
  slider->resize(sw, sh);

  // value label gets centered under slider if it is smaller
  int vx = sx + sw/2 - value_label->width()/2;
  int vy = sy + sh + v_spacing;
  value_label->move(vx, vy);
}

KSliderControl::~KSliderControl()
{
  delete main_label;
  delete slider;
  delete value_label;
  delete unit_str;
}

#include "kslidercontrol.moc"
