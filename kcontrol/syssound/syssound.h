/* 

    $Id: syssound.h,v 1.2 1998/03/08 08:01:33 wuebben Exp $

    Copyright (C) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)
                  1998 Bernd Wuebben <wuebben@kde.org>

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

    $Log: syssound.h,v $
    Revision 1.2  1998/03/08 08:01:33  wuebben
    Bernd: implemented support for all sound events


*/  


#ifndef __SYSSOUND_H__
#define __SYSSOUND_H__

#include "kcontrol.h"

#include <qlist.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qchkbox.h>

#include <drag.h>
#include <mediatool.h>
#include <kaudio.h>


class KSoundWidget : public KConfigWidget{

	Q_OBJECT;

public:

	KSoundWidget(QWidget *parent, const char *name);
	virtual ~KSoundWidget();
	void readConfig();

        void loadSettings();
        void applySettings();
        
private slots:

	void eventSelected(int index);
        void soundSelected(const char *filename);
	void saveConfiguration();
	void playCurrentSound();

	void soundDropped(KDNDDropZone *zone);

private:

	bool addToSoundList(QString sound);
	
	int selected_event;
	QList<QString> soundnames;
	KAudio audio;
        QCheckBox *sounds_enabled;
	QListBox *eventlist, *soundlist;
	KDNDDropZone *audiodrop;
	QPushButton *btn_test;

};

#endif
