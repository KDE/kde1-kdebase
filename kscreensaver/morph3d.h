//-----------------------------------------------------------------------------
//
// kmorph3d - port of "morph3d" from xlock
//

#ifndef __MORPH3D_H__
#define __MORPH3D_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

class kMorph3dSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kMorph3dSaver( Drawable drawable );
	virtual ~kMorph3dSaver();

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

class kMorph3dSetup : public QDialog
{
	Q_OBJECT
public:
	kMorph3dSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotSpeed( int );
	void slotLevels( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kMorph3dSaver *saver;

	int			speed;
	int			maxLevels;
};

#endif

