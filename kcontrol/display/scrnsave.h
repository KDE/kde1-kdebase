//-----------------------------------------------------------------------------
//
// KDE Display screen saver setup module
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __SCRNSAVE_H__
#define __SCRNSAVE_H__

#include <kprocess.h>
#include <qwidget.h>
#include <qpushbt.h>
#include <qlistbox.h>
#include <qlined.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qpopmenu.h>
#include <qchkbox.h>
#include <qslider.h>

#include "display.h"

class CornerButton : public QLabel
{
	Q_OBJECT
public:
	CornerButton( QWidget *parent, int num, char _action );

signals:
	void cornerAction( int corner, char action );

protected:
	virtual void mousePressEvent( QMouseEvent * );
	void setActionText();

protected slots:
	void slotActionSelected( int );

protected:
	QPopupMenu popupMenu;
	int number;
	char action;
};

class KSSMonitor : public QWidget
{
	Q_OBJECT
public:
	KSSMonitor( QWidget *parent ) : QWidget( parent ) {}

	// we don't want no steenking palette change
	virtual void setPalette( const QPalette & ) {}
};

class KScreenSaver : public KDisplayModule
{
	Q_OBJECT
public:
	KScreenSaver( QWidget *parent, int mode, int desktop = 0 );
	~KScreenSaver();

	virtual void readSettings( int deskNum = 0 );
	virtual void apply( bool force = FALSE );
	virtual void loadSettings();
	virtual void applySettings();
	virtual void defaultSettings();
	virtual void updateValues();

protected slots:
	void slotApply();
	void slotScreenSaver( int );
	void slotSetup();
	void slotTest();
	void slotTimeoutChanged( const char *);
	void slotLock( bool );
	void slotAllowRoot( bool );
	void slotStars( bool );
	void slotPriorityChanged( int val );
	void slotSetupDone(KProcess*);
	void slotHelp();
	void slotCornerAction( int, char );
	void resizeEvent( QResizeEvent * );

protected:
	void writeSettings();
	void findSavers();
	void getSaverNames();
	void setMonitor();
	void setDefaults();

protected:
	KConfig *kssConfig;
	KProcess *ssSetup, *ssPreview;
	KSSMonitor* monitor;
	QPushButton *setupBt;
	QPushButton *testBt;
	QListBox *ssList;
	QLineEdit *waitEdit;
	QSlider *prioritySlider;
	QCheckBox *cb, *cbStars, *cbRoot;
	QLabel *monitorLabel;
	const QStrList *saverList;
	QStrList saverNames;
	QString saverLocation;
	QString saverFile;
	int lock;
	int priority;
	bool showStars;
	bool allowRoot;
	char cornerAction[5];

	int xtimeout, xinterval;
	int xprefer_blanking;
	int xallow_exposures;

	bool changed;
	bool bUseSaver;

private slots:
	void slotPreviewExited(KProcess *);
	// when selecting a new screensaver, the old preview will
	// be killed. -- This callback is responsible for restarting the
	// new preview
};

#endif

