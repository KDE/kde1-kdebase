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

#ifndef __KDMAPPEAR_H__
#define __KDMAPPEAR_H__

#include <qdir.h>
#include <qimage.h>
#include <qfileinf.h>
#include <kcolorbtn.h>
#include <kfm.h>
#include <kurl.h>
#include <drag.h>

#include "klangcombo.h"


class KDMAppearanceWidget : public KConfigWidget
{
	Q_OBJECT

public:
	KDMAppearanceWidget(QWidget *parent, const char *name, bool init = false);
	~KDMAppearanceWidget();

        void loadSettings();
        void applySettings();
	void setupPage(QWidget*);

private slots:
        void slotSetGUI(int);
	void slotPixDropped(KDNDDropZone *zone);
        void slotLogoPixChanged(const char*);
        void slotLogoPixTextChanged();

private:
	KDNDDropZone *logopixdrop;
        KIconLoader *iconloader;
        KIconLoaderButton *logobutton;
        QLineEdit    *greetstr_lined, *logo_lined;
	QComboBox    *guicombo;
	QString      greetstr, logopath, logofile, guistr;
        bool         gui;
        KLanguageCombo *langcombo;
};

#endif


