/*
 * kslidercontrol.h
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef K_SLIDER_CONTROL_H
#define K_SLIDER_CONTROL_H

#include <qwidget.h>
class QLabel;
class QSize;
class KSlider;

/**
  * KSliderControl combines a KSlider and a pair of labels to make an
  * easy to use control for setting some integer parameter. This is
  * especially ncie for configuration dialogs, which can have many
  * such combination controls.
  *
  * A special feature of KSliderControl, designed specifically for
  * the situation when there are several KSliderControls in a column,
  * is that you can specify what portion of the control is taken by the
  * main label (the remaining portion is used by the slider). This makes
  * it very simple to have all the sliders in a column be the same size.
  * 
  */
class KSliderControl: public QWidget
{
  Q_OBJECT
public:
  /**
    * Constructor
    *
    * @param label main label for the control
    * @param lower lower bound on range
    * @param upper upper bound on range
    * @param step  step size for the KSlider
    * @param value initial value for the control
    * @param units the units for the control
    * @param parent parent QWidget
    * @param name internal name for this widget
    */
  KSliderControl(const char *label, 
	  const int lower, const int upper,
	  const int step, const int value,
	  const char *units, 
	  QWidget *parent=0, const char *name=0);
  ~KSliderControl();

  /**
    * This method returns the minimum size necessary to display the
    * control. The minimum size is enough to show all the labels
    * in the current font (font change may invalidate the return value).
    * 
    * @return the minimum size necessary to show the control
    */
  QSize minimumSize() const;

  /**
    * Sets the value of the control.
    */
  void setValue(int);

  /**
    * @return the current value
    */
  int intValue();

  /**
    * Sets the spacing of tockmarks for the slider.
    *
    * @param minor minor tickmark separation
    * @param major major tickmark separation
    */
  void setSteps(int minor, int major);

  /**
    * Sets the alignment of the main control label. The value label,
    * including the specified units, is always centered under the
    * slider.
    *
    * @param a one of AlignLeft, AlignCenter, AlignRight
    */
  void setLabelAlignment(int a);

  /**
    * Sets the fraction of the controls width taken by the main label.
    * 1-frac is taken by the slider and value label.
    *
    * @param frac fraction of width taken by main label
    */
  void setLabelSize(float frac);

protected slots:
  void resetValueField(int);

protected:
  void computeMinimumSize();
  void resizeEvent ( QResizeEvent * );
  
  QLabel *main_label;
  KSlider *slider;
  QLabel *value_label;
  QString *unit_str;
  QSize qs;

  int lower_bound, upper_bound;
  int h_spacing, v_spacing;
  int label_align;
  float label_frac;

  int slider_value;
};

#endif // K_SLIDER_CONTROL_H
