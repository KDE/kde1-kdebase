//-----------------------------------------------------------------------------
//
// kflame - port of "flame" from xlock
//

#ifndef __FLAME_H__
#define __FLAME_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

class kFlameSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kFlameSaver( Drawable drawable );
	virtual ~kFlameSaver();

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

class kFlameSetup : public QDialog
{
	Q_OBJECT
public:
	kFlameSetup( QWidget *parent = NULL, const char *name = NULL );

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
	kFlameSaver *saver;

	int			speed;
	int			maxLevels;
	int			numPoints;
};

#endif

