//-----------------------------------------------------------------------------
//
// kpyro - port of "pyro" from xlock
//

#ifndef __PYRO_H__
#define __PYRO_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"


class kPyroSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kPyroSaver( Drawable drawable );
	virtual ~kPyroSaver();

	void setNumber( int num );
	void setCloud( bool c );

protected slots:
	void slotTimeout();

private:
	void readSettings();

protected:
	QTimer		timer;
	int			number;
	bool		cloud;
	int			colorContext;
};


class kPyroSetup : public QDialog
{
	Q_OBJECT
public:
	kPyroSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotNumber( int );
	void slotCloud( bool );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kPyroSaver *saver;

	int number;
	bool cloud;
};


#endif

