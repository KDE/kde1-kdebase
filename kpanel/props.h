
#ifndef KPROPS_H
#define KPROPS_H

#include <qlabel.h>
#include <qslider.h>

class mySlider: public QSlider
{
  Q_OBJECT
public:
  mySlider ( int minValue, int maxValue, int step, int value, Orientation, 
	     QLabel     * Label  = 0,
	     QWidget    * parent = 0,
	     const char * name   = 0 );

public slots:

  void setLabelText(int value);

protected:
  QLabel* label;
  int     lastval;
};

#endif



