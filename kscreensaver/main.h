
#ifndef __MAIN_H__
#define __MAIN_H__

#include <kapp.h>
#define MODE_NONE		0
#define MODE_INSTALL	1
#define MODE_SETUP		2
#define MODE_PREVIEW	3
#define MODE_TEST		4

class ssApp : public KApplication
{
	Q_OBJECT
public:
	ssApp( int &argc, char **argv );

protected:
	virtual bool x11EventFilter( XEvent * );

private:
	bool stars;

public slots:
	void slotPassOk();
	void slotPassCancel();
};

#endif

