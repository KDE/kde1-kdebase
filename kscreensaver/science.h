// ----------------------------------------------------------------
//
// kscience - screen saver for KDE
//
// copyright (c)  Rene Beutler 1998
//

#ifndef __SCIENCE_H__
#define __SCIENCE_H__

#include <qrect.h>
#include <qtimer.h>
#include <qdialog.h>
#include "saver.h"

#define MAX_MODES  6


typedef signed int T32bit;


class KScienceSaver;


class KPrepareDlg : public QWidget 
{
        Q_OBJECT
public:
        KPrepareDlg( QWidget *parent );
	virtual ~KPrepareDlg();
	void   hide();
	void   show();
	void   setText( const char *msg );
	XImage *save;
	int    x, y, w, h, bpl;

private:
        QLabel *label;
	QFrame *frame;	
};


class KPreviewWidget : public QWidget
{
	Q_OBJECT
public:
	KPreviewWidget( QWidget *parent );
	void paintEvent( QPaintEvent *event );
	void notifySaver( KScienceSaver *s = 0 );
private:
	KScienceSaver *saver;
};

                                                       
class KScienceSaver : public kScreenSaver
{
	Q_OBJECT
public:
	KScienceSaver(Drawable drawable, bool setup=false, bool gP=false);
	virtual ~KScienceSaver();

    virtual void expose ( int x, int y, int width, int height );

	void do_refresh( const QRect & rect );
	void setMode        ( int mode );
	void setMoveX       ( signed int s );
	void setMoveY       ( signed int s );
	void setMove        ( bool s );	
	void setSize        ( signed int s );
	void setIntensity   ( signed int s );
	void setSpeed       ( signed int s );
	void setInverse     ( bool b );
	void setGravity     ( bool b );
	void setHideBG      ( bool b );

	void myAssert( bool term, char *sMsg );

private:
	void readSettings();
	void initLens();
	void initialize();
	void releaseLens();
	void (KScienceSaver::*applyLens)(int xs, int ys, int xd, int yd, int w, int h);

protected slots:
	void slotTimeout();

protected:
	void       grabRootWindow();
	void       grabPreviewWidget();
	void       initWhirlLens();
	void       initSphereLens();
	void       initExponentialLens();
	void       initWaveLens();
	void       initCurvatureLens();
	void       blackPixel( int x, int y );
	void       blackPixelUndo( int x, int y);
	void       applyLens8bpp( int xs, int ys, int xd, int yd, int w, int h);
	void       applyLens16bpp(int xs, int ys, int xd, int yd, int w, int h);
	void       applyLens24bpp(int xs, int ys, int xd, int yd, int w, int h);
	void       applyLens32bpp(int xs, int ys, int xd, int yd, int w, int h);
	KPrepareDlg *dlg;
	QTimer     timer;
	bool       moveOn;
	bool       setup;
	bool       grabPixmap;	bool       showDialog;
	int        mode;
	bool       inverse[MAX_MODES];
	bool       gravity[MAX_MODES];
	bool       hideBG[MAX_MODES];
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
	char       blackRestore[4];
	T32bit     **offset;
	XImage     *buffer;
	XImage     *xRootWin;
};


class KScienceSetup : public QDialog 
{
	Q_OBJECT
public:
	KScienceSetup(QWidget *parent=0, const char *name=0);
	~KScienceSetup();
protected:
	void updateSettings();
	void readSettings();

private slots:
	void slotMode( int );
	void slotInverse();
	void slotGravity();
	void slotHideBG();
	void slotMoveX( int );
	void slotMoveY( int );
	void slotSize( int );
	void slotIntensity( int );
	void slotSliderPressed();
	void slotSliderReleased();
	void slotSpeed( int );
	void slotOkPressed();
	void slotAbout();

private:
	KPreviewWidget *preview;
	KScienceSaver *saver;
	KSlider *slideSize, *slideSpeed, *slideIntensity;
	KSlider *slideMoveX, *slideMoveY;
	QCheckBox *checkInverse, *checkGravity, *checkHideBG;	
	
	int  mode;
	bool inverse  [MAX_MODES];
	bool gravity  [MAX_MODES];
	bool hideBG   [MAX_MODES];
	int  moveX    [MAX_MODES];
	int  moveY    [MAX_MODES];	
	int  size     [MAX_MODES]; 
	int  intensity[MAX_MODES];
	int  speed    [MAX_MODES];
}; 
#endif


