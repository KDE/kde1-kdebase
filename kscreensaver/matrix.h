//-----------------------------------------------------------------------------
//
// klaser - port of "laser" from xlock
//

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"
#include "xlock.h"

class kMatrixSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kMatrixSaver( Drawable drawable );
	virtual ~kMatrixSaver();

	void setSpeed( int spd );

protected:
	void readSettings();

protected slots:
	void slotTimeout();

protected:
	QTimer      timer;
	int         colorContext;

	int         speed;
	m_state *              matrix_state;
};


class kMatrixSetup : public QDialog
{
	Q_OBJECT
public:
	kMatrixSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
        void slotSpeed( int ); 
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kMatrixSaver *saver;

	int                     speed;
};


#endif

