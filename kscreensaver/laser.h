//-----------------------------------------------------------------------------
//
// klaser - port of "laser" from xlock
//

#ifndef __LASER_H__
#define __LASER_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

class kLaserSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kLaserSaver( Drawable drawable );
	virtual ~kLaserSaver();

	void setSpeed( int spd );

protected:
	void readSettings();

protected slots:
	void slotTimeout();

protected:
	QTimer      timer;
	int         colorContext;

	int         speed;
};


class kLaserSetup : public QDialog
{
	Q_OBJECT
public:
	kLaserSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotSpeed( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kLaserSaver *saver;

	int			speed;
};


#endif

