//-----------------------------------------------------------------------------
//
// kbanner - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __BANNER_H__
#define __BANNER_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include <qpushbt.h>
#include "saver.h"


class KBannerSaver : public kScreenSaver
{
	Q_OBJECT
public:
	KBannerSaver( Drawable drawable );
	virtual ~KBannerSaver();

	void setSpeed( int spd );
	void setFont( const char *family, int size, const QColor &color,
			bool b, bool i );
	void setMessage( const char *msg );

private:
	void readSettings();
	void initialise();
	void blank();

protected slots:
	void slotTimeout();

protected:
	XFontStruct *fontInfo;
	QTimer		timer;
	QString		fontFamily;
	int			fontSize;
	bool		bold;
	bool		italic;
	QColor		fontColor;
	QString		message;
	int			xpos, ypos, step;
	int			fwidth;
	int			speed;
	int			colorContext;
};


class KBannerSetup : public QDialog
{
	Q_OBJECT
public:
	KBannerSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotFamily( const char * );
	void slotSize( int );
	void slotColor();
	void slotBold( bool );
	void slotItalic( bool );
	void slotSpeed( int );
	void slotMessage( const char * );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	QPushButton *colorPush;
	KBannerSaver *saver;

	QString	message;
	QString fontFamily;
	int		fontSize;
	QColor	fontColor;
	bool	bold;
	bool	italic;
	int		speed;
};

#endif

