//-----------------------------------------------------------------------------
//
// kslidescreen - xscreensaver port for KDE
//
// Ported by: Tom Vijlbrief 1998 (tom.vijlbrief@knoware.nl)
//
// Based on:

/* xscreensaver, Copyright (c) 1992, 1995, 1996, 1997
 *  Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 */                     


#ifndef __SLIDESCREEN_H__
#define __SLIDESCREEN_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"


class kSlideScreenSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kSlideScreenSaver( Drawable drawable );
	virtual ~kSlideScreenSaver();

	virtual void expose(int x, int y, int width, int height);

protected slots:
	void slotTimeout();

private:
	void readSettings();

protected:
	QTimer  timer;
	int		colorContext;
    bool    restart;

public:
};


class kSlideScreenSetup : public QDialog
{
	Q_OBJECT
public:
	kSlideScreenSetup( QWidget *parent = NULL, const char *name = NULL );

private slots:
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kSlideScreenSaver *saver;

};

#endif

