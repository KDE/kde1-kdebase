//-----------------------------------------------------------------------------
// Slider control
//
// Copyright (C) Martin R. Jones 1997
//

#ifndef __KSLIDER_H__
#define __KSLIDER_H__

#include <qrangect.h>
#include <qwidget.h>

class KSlider : public QWidget, public QRangeControl
{
	Q_OBJECT
public:
	enum Orientation { Horizontal, Vertical };

	KSlider( Orientation o, QWidget *parent = NULL, const char *name = NULL );
	KSlider( int _minValue, int _maxValue, int _lineStep, int _pageStep,
		 Orientation o, QWidget *parent = NULL, const char *name = NULL );

	void setOrientation( Orientation o )
		{	_orientation = o; }
	Orientation orientation() const
		{	return _orientation; }

signals:
	void valueChanged( int value );

protected:
	virtual void valueChange();
	virtual void drawArrow( QPainter &painter, bool show, const QPoint &pos );
	virtual QSize sizeHint() const;

private:
	QPoint calcArrowPos( int val );
	void moveArrow( const QPoint &pos );

	virtual void paintEvent( QPaintEvent * );
	virtual void mousePressEvent( QMouseEvent *e );
	virtual void mouseMoveEvent( QMouseEvent *e );

protected:
	Orientation _orientation;
};

//-----------------------------------------------------------------------------

#endif

