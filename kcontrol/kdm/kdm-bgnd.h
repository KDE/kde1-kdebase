/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#ifndef __KDMBGND_H__
#define __KDMBGND_H__

#include <qfiledlg.h>
#include <qimage.h>
#include <qradiobt.h>
#include <qbttngrp.h>
#include <kcolordlg.h>
#include <kcolorbtn.h>
#include <kpixmap.h>

class KBGMonitor : public QWidget
{
	Q_OBJECT
public:
	KBGMonitor( QWidget *parent ) : QWidget( parent ) { allowdrop = false;};

	// we don't want no steenking palette change
	virtual void setPalette( const QPalette & ) {};
	void setAllowDrop(bool);
protected:
	bool allowdrop;
};

class KDMBackgroundWidget : public KConfigWidget
{
	Q_OBJECT

public:
	KDMBackgroundWidget(QWidget *parent, const char *name, bool init = false);
	~KDMBackgroundWidget();

	enum { NoPic, Tile, Center, Scale, TopLeft, TopRight,
                                  BottomLeft, BottomRight, Fancy, Plain, Vertical, Horizontal };
        void loadSettings();
        void applySettings();
	void setupPage(QWidget*);

protected slots:
	void slotSelectColor1( const QColor &col );
	void slotSelectColor2( const QColor &col );
	void slotBrowse();
	void slotWallpaper( const char * );
	void slotWallpaperMode( int );
	void slotColorMode( int );
	void slotQDrop( QDropEvent* );
	void slotQDragLeave( QDragLeaveEvent* );
	void slotQDragEnter( QDragEnterEvent* );

protected:
        void setMonitor();
        void showSettings();
        int  loadWallpaper( const char *name, bool useContext = true);

        KIconLoader *iconloader;
	KBGMonitor  *monitor;
	KPixmap      wpPixmap;
	QString      wallpaper;
	KColorButton *colButton1, *colButton2;
	QButtonGroup *wpGroup, *cGroup;
	QComboBox    *wpCombo;
	QColor       color1, color2;
	int          wpMode, colorMode;
        bool         gui;
        QPushButton *button;
};


#endif


