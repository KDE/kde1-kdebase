//-----------------------------------------------------------------------------
//
// kbat - port of "bat" from xlock
//

#ifndef __BAT_H__
#define __BAT_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

class kBatSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kBatSaver( Drawable drawable );
	virtual ~kBatSaver();

	void setSpeed( int spd );
	void setLevels( int l );

protected:
	void readSettings();

protected slots:
	void slotTimeout();

protected:
	QTimer      timer;
	int         colorContext;

	int         speed;
	int			maxLevels;
};

class kBatSetup : public QDialog
{
	Q_OBJECT
public:
	kBatSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotSpeed( int );
	void slotLevels( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kBatSaver *saver;

	int			speed;
	int			maxLevels;
};

#endif

