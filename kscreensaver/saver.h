#ifndef __SAVER_H__
#define __SAVER_H__ "$Id"

#include <qwidget.h>
#include <qlabel.h>
#include <qtimer.h>
#include <kapp.h>
#include <X11/Xlib.h>

extern "C" {
  void startScreenSaver( Drawable d );
  void stopScreenSaver();
  int setupScreenSaver();
  const char *getScreenSaverName();
  void exposeScreenSaver(int x, int y, int width, int height);
}

class kScreenSaver : public QObject
{
	Q_OBJECT
public:
	kScreenSaver( Drawable drawable );
	virtual ~kScreenSaver();

	virtual void expose( int x, int y, int width, int height );

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

    void blank();

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

