//-----------------------------------------------------------------------------
//
// kbouboule.h - port of "bouboule" from xlockmore
//

#ifndef __BOUBOULE_H__
#define __BOUBOULE_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

class kBoubouleSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kBoubouleSaver( Drawable drawable );
	virtual ~kBoubouleSaver();

	void setSpeed( int spd );
	void setPoints( int p );
	void setSize( int p );
	void setColorCycle( int spd );
	void set3DMode( bool mode3d );

protected:
	void readSettings();

protected slots:
	void slotTimeout();

protected:
	QTimer      timer;
	int         colorContext;

	int         speed;
	int	    numPoints;
	int	    pointSize;
	int	    colorCycleDelay;
	bool	    flag_3dmode;
};

class kBoubouleSetup : public QDialog
{
	Q_OBJECT
public:
	kBoubouleSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotSpeed( int );
	void slotPoints( int );
	void slotSize( int );
	void slotColorCycle( int );
	void slot3DMode( bool );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kBoubouleSaver *saver;
	KSlider *freqslider;
	QLabel *freqlabel;

	int			speed;
	int			maxLevels;
	int			numPoints;
	int			pointSize;
	int			colorCycleDelay;
	bool	    		flag_3dmode;
};

#endif

