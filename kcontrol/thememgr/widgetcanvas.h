/*
 * widgetcanvas.h
 *
 * Copyright (c) 1997 Mark Donohoe 
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
#ifndef WIDGETCANVAS_H
#define WIDGETCANVAS_H

#include "kcolorbtn.h"
#include "savescm.h"

#include "display.h"
#include <X11/X.h>
#include <kcontrol.h>

#include <qlistbox.h>
#include <kslider.h>

#define MAX_HOTSPOTS   13

class HotSpot
{
public:
  HotSpot() {}
  HotSpot( const QRect &r, int num )
    {	rect = r; number = num; }

  QRect rect;
  int number;
};


class WidgetCanvas : public QWidget
{
  Q_OBJECT
public:
  WidgetCanvas(QWidget *);
  void drawSampleWidgets();
  QPixmap smplw;
	
  QColor inactiveTitleColor;
  QColor inactiveTextColor;
  QColor activeTitleColor;
  QColor activeTextColor;
  QColor backgroundColor;
  QColor textColor;
  QColor selectColor;
  QColor selectTextColor;
  QColor windowColor;
  QColor windowTextColor;
	
  int contrast;

signals:
  void widgetSelected(int);
	
protected:
  virtual void paintEvent(QPaintEvent *);
  virtual void mousePressEvent( QMouseEvent * );
  void paletteChange(const QPalette &);

  HotSpot hotspots[MAX_HOTSPOTS];
};

#endif /*WIDGETCANVAS_H*/
