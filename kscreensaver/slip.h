//-----------------------------------------------------------------------------
//
// kslip - port of "slip" from xlock
//

#ifndef __SLIP_H__
#define __SLIP_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

class kSlipSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kSlipSaver( Drawable drawable );
	virtual ~kSlipSaver();

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

class kSlipSetup : public QDialog
{
	Q_OBJECT
public:
	kSlipSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotSpeed( int );
	void slotLevels( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kSlipSaver *saver;

	int			speed;
	int			maxLevels;
};

#endif

