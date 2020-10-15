
#ifndef __SPACE_H__
#define __SPACE_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

class kSpaceSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kSpaceSaver( Drawable drawable );
	virtual ~kSpaceSaver();

	void setSpeed( int spd );
	void setWarp( int l );
	void setPoints( int p );

protected:
	void readSettings();

protected slots:
	void slotTimeout();

protected:
	QTimer      timer;
	int         colorContext;

	int         counter;
	int			maxLevels;
	int			numPoints;
};

class kSpaceSetup : public QDialog
{
	Q_OBJECT
public:
	kSpaceSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotSpeed( int );
	void slotWarp( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kSpaceSaver *saver;

	int			warpinterval;
};

#endif

