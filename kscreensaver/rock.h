//-----------------------------------------------------------------------------
//
// krock - port of "rock" from xlock
//

#ifndef __ROCK_H__
#define __ROCK_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"


class kRockSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kRockSaver( Drawable d );
	virtual ~kRockSaver();

	void setSpeed( int spd );
	void setNumber( int num );
	void setMove( bool m );
	void setRotate( bool m );

protected slots:
	void slotTimeout();

private:
	void readSettings();

protected:
	QTimer		timer;
	int			speed;
	int			number;
	bool		move;
	bool		rotate;
	int			colorContext;
};

class kRockSetup : public QDialog
{
	Q_OBJECT
public:
	kRockSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotSpeed( int );
	void slotNumber( int );
	void slotMove( bool );
	void slotRotate( bool );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kRockSaver *saver;

	int speed;
	int number;
	bool move;
	bool rotate;
};

#endif

