// $Id$

#ifndef _BSOD_H
#define _BSOD_H "$Id$"

#include <qobject.h>
#include <qtimer.h>

#include "saver.h"

class BSODSaver : public kScreenSaver
{
	Q_OBJECT
public:
	BSODSaver (Drawable drawable);
	virtual ~BSODSaver ();
protected:
        QTimer *timer;
	unsigned int delay;
	void readSettings ();
	void draw_bsod (Window win);
protected slots:
	void timeout ();
};

#endif
