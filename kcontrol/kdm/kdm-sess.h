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

#ifndef __KDMSESS_H__
#define __KDMSESS_H__


#include <qlist.h>
#include <qstring.h>
#include <qlistbox.h>
#include <kdbtn.h>

class MyListBox : public QListBox
{
public:
	MyListBox(QWidget *parent) : QListBox(parent) {}
	bool isItemVisible(int id) { return itemVisible(id); }
};

class KDMSessionsWidget : public KConfigWidget
{
	Q_OBJECT

public:
	KDMSessionsWidget(QWidget *parent, const char *name, bool init = false);
	~KDMSessionsWidget();

        void loadSettings();
        void applySettings();
	void setupPage(QWidget*);

	enum { Non, All, RootOnly, ConsoleOnly };
	
protected:
	void moveSession(int);

protected slots:
        void slotSetAllowShutdown(int);
        void slotAddSessionType();
        void slotRemoveSessionType();
        void slotSessionHighlighted(int);
        void slotCheckNewSession(const char*);
        void slotSessionUp();
        void slotSessionDown();

private:
        KIconLoader  *iconloader;
	QComboBox    *sdcombo;
        QLineEdit    *restart_lined, *shutdown_lined, *session_lined;
	MyListBox     *sessionslb;
        QString      shutdownstr, restartstr;
        QStrList     sessions;
	int          sdMode;
        bool         gui;
        KDirectionButton *btnup, *btndown;
        QButton      *btnrm, *btnadd;
};


#endif


