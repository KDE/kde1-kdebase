// ----------------------------------------------------------------
//
// kscience - screen saver for KDE
//
// copyright (c)  Rene Beutler 1998
//

#ifndef __SCIENCE_H__
#define __SCIENCE_H__

#include <qtimer.h>
#include <qdialog.h>
#include "saver.h"

typedef signed int T32bit;

class KScienceSaver : public kScreenSaver
{
	Q_OBJECT
public:
	KScienceSaver(Drawable drawable);
	virtual ~KScienceSaver();

	void do_refresh();
	void setSize      ( signed int s );
	void setIntensity ( signed int s );
	void setSpeed     ( signed int s );

private:
	void myAssert( bool term, char *sMsg );
	void readSettings();
	void initLens();
	void releaseLens();
	void (KScienceSaver::*applyLens)();

protected slots:
	void slotTimeout();

protected:
	void       grabRootWindow();
	void       applyLens8bpp();
	void       applyLens16bpp();
	void       applyLens24bpp();
	void       applyLens32bpp();
	QTimer     timer;
	bool       refresh;
	signed int size;
	signed int speed;
	signed int intensity;
	signed int x, y, vx, vy, bpp, side;
	T32bit     **offset;
	XImage     *buffer;
	XImage     *xRootWin;
	int        xRootWidth;
	int        xRootHeight;
};

class KScienceSetup : public QDialog 
{
	Q_OBJECT
public:
	KScienceSetup(QWidget *parent=0, const char *name=0);

protected:
	void readSettings();
	virtual void paintEvent( QPaintEvent * );

private slots:
	void slotSize( int );
	void slotIntensity( int );
	void slotSpeed( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	KScienceSaver *saver;

	int size; 
	int intensity;
	int speed;
}; 

#endif


