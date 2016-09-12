//-----------------------------------------------------------------------------
//
// kblankscrn - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __BLANKSCRN_H__
#define __BLANKSCRN_H__

#include <qdialog.h>
#include <qcolor.h>
#include <qpushbt.h>
#include "saver.h"


class KBlankSaver : public kScreenSaver
{
	Q_OBJECT
public:
	KBlankSaver( Drawable drawable );
	virtual ~KBlankSaver();

	void setColor( const QColor &col );

private:
	void readSettings();
	void blank();

private:
	QColor color;
};

class KBlankSetup : public QDialog
{
	Q_OBJECT
public:
	KBlankSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotColor();
	void slotOkPressed();

private:
	QWidget *preview;
	KBlankSaver *saver;
	QPushButton *colorPush;

	QColor color;
};

#endif

