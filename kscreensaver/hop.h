//-----------------------------------------------------------------------------
//
// khop - port of "hop" from xlock
//

#ifndef __HOP_H__
#define __HOP_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

class kHopSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kHopSaver( Drawable drawable );
	virtual ~kHopSaver();

	void setSpeed( int spd );
	void setLevels( int l );
	void setPoints( int p );

protected:
	void readSettings();

protected slots:
	void slotTimeout();

protected:
	QTimer      timer;
	int         colorContext;

	int         speed;
	int			maxLevels;
	int			numPoints;
};

class kHopSetup : public QDialog
{
	Q_OBJECT
public:
	kHopSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotSpeed( int );
	void slotLevels( int );
	void slotPoints( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kHopSaver *saver;

	int			speed;
	int			maxLevels;
	int			numPoints;
};

#endif

