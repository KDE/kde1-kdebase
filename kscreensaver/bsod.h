// $Id: bsod.h,v 1.1 1998/10/24 03:15:10 garbanzo Exp $

#ifndef _BSOD_H
#define _BSOD_H "$Id: bsod.h,v 1.1 1998/10/24 03:15:10 garbanzo Exp $"

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
