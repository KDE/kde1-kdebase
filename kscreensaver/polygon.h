//-----------------------------------------------------------------------------
//
// kpolygon - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __POLYGON_H__
#define __POLYGON_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"


class Polygon
{
public:
	Polygon( int numVert )
		{	vertices.resize( numVert + 1 ); }
	Polygon( Polygon &p )
		{	vertices = p.vertices.copy(); }

	QArray<XPoint> vertices;
};

class kPolygonSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kPolygonSaver( Drawable drawable );
	virtual ~kPolygonSaver();

	void setPolygon( int len, int ver );
	void setSpeed( int spd );

private:
	void readSettings();
	void blank();
	void initialisePolygons();
	void moveVertices();
	void initialiseColor();
	void nextColor();

protected slots:
	void slotTimeout();

protected:
	QTimer		timer;
	unsigned	numLines;
	int			numVertices;
	int			colorContext;
	int			speed;
	uint		colors[64];
	QList<Polygon> polygons;
	QArray<XPoint> directions;
};

class kPolygonSetup : public QDialog
{
	Q_OBJECT
public:
	kPolygonSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotLength( int );
	void slotVertices( int );
	void slotSpeed( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kPolygonSaver *saver;

	int length;
	int vertices;
	int speed;
};

#endif

