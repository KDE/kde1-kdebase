
#ifndef __MAIN_H__
#define __MAIN_H__

#include <kapp.h>

class ssApp : public KApplication
{
	Q_OBJECT
public:
	ssApp( int &argc, char **argv );

protected:
	virtual bool x11EventFilter( XEvent * );


public slots:
	void slotPassOk();
	void slotPassCancel();
};

#endif

