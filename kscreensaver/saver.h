
#ifndef __SAVER_H__
#define __SAVER_H__

#include <qwidget.h>
#include <qlabel.h>
#include <qtimer.h>
#include <kapp.h>
#include <X11/Xlib.h>

class kScreenSaver : public QObject
{
	Q_OBJECT
public:
	kScreenSaver( Drawable drawable );
	virtual ~kScreenSaver();

protected:
	Drawable d;
	GC gc;
	unsigned width;
	unsigned height;
};

//-----------------------------------------------------------------------------

class KPasswordDlg : public QWidget
{
	Q_OBJECT
public:
	KPasswordDlg( QWidget *parent, bool s = true );

	void showStars();
	int tryPassword();
	void keyPressed( QKeyEvent * );

signals:
	void passOk();
	void passCancel();

protected slots:
	void timeout();
	void blinkTimeout();
	
private:
	QTimer timer, *blinkTimer;
	QLabel *label, *entry;
	QString password;
	int timerMode;
	bool stars;
	bool blink;
};

#endif

