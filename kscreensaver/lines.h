//-----------------------------------------------------------------------------
//
// klines 0.1.1 - Basic screen saver for KDE
// by Dirk Staneker 1997
// based on kpolygon 0.3 by Martin R. Jones 1996
//

#ifndef __LINES_H__
#define __LINES_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"


class Lines{
	private:
	struct Ln{
		Ln* next;
		int x1, y1, x2, y2;
	};
	Ln *start, *end, *akt;
	int offx1, offy1, offx2, offy2;
	uint numLn;
	public:
	Lines(int);
	~Lines();
	inline void reset();
	inline void getKoord(int&, int&, int&, int&);
	inline void setKoord(const int&, const int&, const int&, const int&);
	inline void next(void);
	void turn(const int&, const int&);
};

class kLinesSaver:public kScreenSaver{
	Q_OBJECT
	public:
	kLinesSaver(Drawable drawable);
	virtual ~kLinesSaver();

	void setLines(int len);
	void setSpeed(int spd);
	void setColor(const QColor&, const QColor&, const QColor&);

	private:
	void readSettings();
	void blank();
	void initialiseLines();
	void initialiseColor();

	protected slots:
	void slotTimeout();

	protected:
	QTimer timer;
	unsigned numLines;
	int colorContext, speed;
	uint colors[64];
        QColor colstart, colmid, colend;
	double colscale;
	Lines* lines;
};

class kLinesSetup:public QDialog{
	Q_OBJECT
	public:
	kLinesSetup(QWidget *parent=NULL, const char *name=NULL);

	protected:
	void readSettings();

	private slots:
	void slotLength(int);
	void slotSpeed(int);
	void slotOkPressed();
	void slotColstart();
	void slotColmid();
	void slotColend();
	void slotAbout();

	private:
	QPushButton *colorPush0, *colorPush1, *colorPush2;
	QWidget *preview;
	kLinesSaver *saver;
	int length, speed;
	QColor colstart, colmid, colend;
};

#endif

