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

#ifndef __KDMUSERS_H__
#define __KDMUSERS_H__

#include <qlist.h>
#include <qstring.h>
#include <qimage.h>
#include <qlistbox.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qchkbox.h>

#include <kfm.h>
#include <drag.h>
#include <kcolorbtn.h>
#include <kurl.h>

#include <pwd.h>

class KDMUsersWidget : public KConfigWidget
{
	Q_OBJECT

public:
	KDMUsersWidget(QWidget *parent, const char *name, bool init = false);
	~KDMUsersWidget();

        void loadSettings();
        void applySettings();
	void setupPage(QWidget*);

private slots:
        void slotUserSelected(int);
        void slotUserShowMode(int);
        void slotUserShow(bool);
        void slotUserSort(bool);
        void slotAllToNo();
        void slotAllToUsr();
        void slotUsrToAll();
        void slotNoToAll();
	void slotPixDropped(KDNDDropZone *zone);
        void slotUserPixChanged(const char*);

private:

        KIconLoader *iconloader;
	QButtonGroup *usrGroup, *shwGroup;
        QCheckBox    *cbusrshw, *cbusrsrt;
	KDNDDropZone *userpixdrop;
        KIconLoaderButton *userbutton;
        QLabel       *userlabel;
	QListBox     *alluserlb, *nouserlb, *userlb;
        QStrList     no_users, users, allusers;
        bool         showallusers, showusers, sortusers, changed, gui;
};

#endif


