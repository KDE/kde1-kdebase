//-----------------------------------------------------------------------------
//
// kattraction - xscreensaver port for KDE
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


#ifndef __ATTRACTION_H__
#define __ATTRACTION_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"


class kAttractionSaver : public kScreenSaver
{
	Q_OBJECT
public:
	kAttractionSaver( Drawable drawable );
	virtual ~kAttractionSaver();

	void setNumber( int num );
	void setGlow( bool c );
	void setMode( const char * );

protected slots:
	void slotTimeout();

private:
	void readSettings();

protected:
	QTimer		timer;
	int		colorContext;
	int		number;
	bool		glow;
	QString		mode;

public:
	bool getGlow() { return glow; }
	int getNumber() { return number; }
	const char * getMode() { return mode; }
};


class kAttractionSetup : public QDialog
{
	Q_OBJECT
public:
	kAttractionSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotNumber( int );
	void slotGlow( bool );
	void slotMode( const char * );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kAttractionSaver *saver;

	int number;
	bool glow;
	QString mode;
};

#endif

