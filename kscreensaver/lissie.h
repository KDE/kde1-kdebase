//-----------------------------------------------------------------------------
//
// klissie - port of "lissie" from xlock
//

#ifndef __LISSIE_H__
#define __LISSIE_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

class kLissieSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kLissieSaver( Drawable drawable );
	virtual ~kLissieSaver();

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

class kLissieSetup : public QDialog
{
	Q_OBJECT
public:
	kLissieSetup( QWidget *parent = NULL, const char *name = NULL );

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
	kLissieSaver *saver;

	int			speed;
	int			maxLevels;
	int			numPoints;
};

#endif

