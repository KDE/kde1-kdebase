//-----------------------------------------------------------------------------
//
// kblob - Basic screen saver for KDE
//
// Copyright (c)  Tiaan Wessels, 1997
//

#ifndef __BLOB_H__
#define __BLOB_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include <qlistbox.h>
#include "saver.h"

#define RAMP		64
#define SPEED		10

enum blob_alg {
	ALG_LINEAR,
	ALG_HSINE,
	ALG_CIRB,
	ALG_POLARC,
	ALG_LAST,
	ALG_RANDOM = ALG_LAST };

class KBlobSaver : public kScreenSaver
{
    Q_OBJECT

public:
    KBlobSaver( Drawable drawable );
    virtual ~KBlobSaver();

    void setDimension(int d)
	{ dim = d; }
    void setShowlen(time_t s)
	{ showlen = s; }
    void setColorInc(int c)
	{ colorInc = c; }

public slots:
    void setAlgorithm(int);

public:
    typedef void (KBlobSaver::*AlgFunc)();
    struct KBSAlg
    {
	char Name[64];
	AlgFunc Init;
	AlgFunc NextFrame;
    };
private:

    QTimer	timer;
    uint	colors[RAMP];
    uint	lookup[256];
    int		colorContext, colorInc;
    int		tx, ty;
    int		dim;
    int		xhalf, yhalf;
    int		alg, newalg, newalgp;
    time_t	showlen, start;
    KBSAlg	Algs[ALG_LAST];
    int		ln_xinc, ln_yinc;
    float	hs_radians, hs_rinc, hs_flip, hs_per;
    float	cb_radians, cb_rinc, cb_sradians, cb_radius, cb_devradinc;
    float	cb_deviate;
    float	pc_angle, pc_radius, pc_inc, pc_crot, pc_div;

    void lnSetup();
    void hsSetup();
    void cbSetup();
    void pcSetup();

    void lnNextFrame();
    void hsNextFrame();
    void cbNextFrame();
    void pcNextFrame();

    void blank();
    void box(int, int);
    void readSettings();

protected slots:
    void slotTimeout();
};

class KBlobSetup : public QDialog
{
    Q_OBJECT

    int showtime;
    int alg;
    QListBox *algs;
    QLineEdit *stime;

public:
    KBlobSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
    void readSettings();

private slots:
    void slotOkPressed();
    void slotAbout();
};

#endif

