//
// KDE Shortcut config module
//
// Copyright (c)  Mark Donohoe 1998
// Copyright (c)  Matthias Ettrich 1998
// Converted to generic key configuration module, Duncan Haldane 1998.

#ifndef __KEYCONFIG_H__
#define __KEYCONFIG_H__

#include <qwidget.h>
#include <qpushbt.h>
#include <qcombo.h>
#include <qlistbox.h>
#include <qradiobt.h>
#include <qlined.h>
#include <qbttngrp.h>
#include <kcontrol.h>

#include "savescm.h"
#include <kaccel.h>
#include <kkeydialog.h>

class KKeyConfig : public KConfigWidget
{
	Q_OBJECT
public:
	KAccel *keys;
	QDict<KKeyEntry> dict;
	KKeyChooser *kc;

	KKeyConfig( QWidget *parent, const char *name = 0 );
	~KKeyConfig ();

        virtual void loadSettings();
        virtual void applySettings();
        virtual void defaultSettings();

public slots:
	void slotPreviewScheme( int );
	void slotAdd();
	void slotSave();
	void slotRemove();
	void slotChanged();


protected:
	QListBox *sList;
	QStrList *sFileList;
	QDict<int> *globalDict;
	QPushButton *saveBt;
	QPushButton *addBt;
	QPushButton *removeBt;
	int nSysSchemes;

	void readSchemeNames();
	void readScheme( int index=0 );

	bool changed;

        QString KeyType ;
        QString KeyScheme ;
        QString KeySet ;

};
#endif










