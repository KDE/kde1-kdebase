//
// A special widget which draws a sample of KDE widgets
// It is used to preview color schemes
//
// Copyright (c)  Mark Donohoe 1998
//

#ifndef __WIDGETCANVAS_H__
#define __WIDGETCANVAS_H__

#include <qpixmap.h>
#include <qdrawutl.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qscrbar.h>

#include <kapp.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kpixmap.h>

#define MAX_HOTSPOTS   19
#define SCROLLBAR_SIZE 16

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
	WidgetCanvas( QWidget *parent=0, const char *name=0 );
	void drawSampleWidgets();
	QPixmap smplw;
	
	QColor iaTitle;
	QColor iaTxt;
	QColor iaBlend;
	QColor aTitle;
	QColor aTxt;
	QColor aBlend;
	QColor back;
	QColor txt;
	QColor select;
	QColor selectTxt;
	QColor window;
	QColor windowTxt;
	
	int contrast;

signals:
	void widgetSelected( int );
	
protected:
	virtual void paintEvent( QPaintEvent * );
	virtual void mousePressEvent( QMouseEvent * );
	void paletteChange( const QPalette & );

	HotSpot hotspots[MAX_HOTSPOTS];
};

#endif
