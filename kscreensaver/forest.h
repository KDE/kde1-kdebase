//-----------------------------------------------------------------------------
//
// kforest - port of "forest" from xlock
//

#ifndef __FOREST_H__
#define __FOREST_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

class kForestSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kForestSaver( Drawable drawable );
	virtual ~kForestSaver();

	void setSpeed( int spd );
	void setPoints( int p );

protected:
	void readSettings();

protected slots:
	void slotTimeout();

protected:
	QTimer      timer;
	int         colorContext;

	int         speed;
	int			numPoints;
};

class kForestSetup : public QDialog
{
	Q_OBJECT
public:
	kForestSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotSpeed( int );
	void slotPoints( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kForestSaver *saver;

	int			speed;
	int			maxLevels;
	int			numPoints;
};

#endif

