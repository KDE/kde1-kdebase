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

#define MAX_MODES 2 

typedef signed int T32bit;

class KScienceSaver : public kScreenSaver
{
	Q_OBJECT
public:
	KScienceSaver(Drawable drawable);
	virtual ~KScienceSaver();

	void do_refresh();
	void setMode      ( int mode );
	void setMoveX     ( signed int s );
	void setMoveY     ( signed int s );	
	void setSize      ( signed int s );
	void setIntensity ( signed int s );
	void setSpeed     ( signed int s );
	void setInverse   ( bool b );
	void setGravity   ( bool b );

private:
	void myAssert( bool term, char *sMsg );
	void readSettings();
	void initLens();
	void releaseLens();
	void (KScienceSaver::*applyLens)(int xs, int ys, int xd, int yd, int w, int h);

protected slots:
	void slotTimeout();

protected:
	void       grabRootWindow();
	void       initWhirlLens();
	void       initSphereLens();
	void       applyLens8bpp( int xs, int ys, int xd, int yd, int w, int h);
	void       applyLens16bpp(int xs, int ys, int xd, int yd, int w, int h);
	void       applyLens24bpp(int xs, int ys, int xd, int yd, int w, int h);
	void       applyLens32bpp(int xs, int ys, int xd, int yd, int w, int h);
	QTimer     timer;
	bool       refresh;
	int        mode;
	bool       inverse[MAX_MODES];
	bool       gravity[MAX_MODES];
	signed int size[MAX_MODES];
	signed int moveX[MAX_MODES];
	signed int moveY[MAX_MODES];
	signed int speed[MAX_MODES];
	signed int intensity[MAX_MODES];
	int        xcoord, ycoord;
	double     x, y, vx, vy;
	signed int bpp, side;
	int        border, radius, diam, origin;
	int        imgnext;
	T32bit     **offset;
	XImage     *buffer;
	XImage     *xRootWin;
};

class KScienceSetup : public QDialog 
{
	Q_OBJECT
public:
	KScienceSetup(QWidget *parent=0, const char *name=0);

protected:
	void updateSettings();
	void readSettings();
	virtual void paintEvent( QPaintEvent * );

private slots:
	void slotMode( int );
	void slotInverse();
	void slotGravity();
	void slotMoveX( int );
	void slotMoveY( int );
	void slotSize( int );
	void slotIntensity( int );
	void slotSpeed( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	KScienceSaver *saver;
	KSlider *slideSize, *slideSpeed, *slideIntensity;
	KSlider *slideMoveX, *slideMoveY;
	QCheckBox *checkInverse, *checkGravity;	

	int  mode;
	bool inverse  [MAX_MODES];
	bool gravity  [MAX_MODES];
	int  moveX    [MAX_MODES];
	int  moveY    [MAX_MODES];	
	int  size     [MAX_MODES]; 
	int  intensity[MAX_MODES];
	int  speed    [MAX_MODES];}; 

#endif


