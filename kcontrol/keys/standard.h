//
// KDE Shotcut config module
//
// Copyright (c)  Mark Donohoe 1998
// Copyright (c)  Matthias Ettrich 1998
//

#ifndef __STANDARD_H__
#define __STANDARD_H__

#include <qwidget.h>
#include <qpushbt.h>
#include <qcombo.h>
#include <qlistbox.h>
#include <qradiobt.h>
#include <qlined.h>
#include <qbttngrp.h>
#include <kcontrol.h>


#include <kaccel.h>
#include <kkeydialog.h>

class KStdConfig : public KConfigWidget
{
	Q_OBJECT
public:
	KAccel *keys;
	QDict<KKeyEntry> dict;
	KKeyChooser *kc;
	
	KStdConfig( QWidget *parent, const char *name = 0 );
	~KStdConfig ();
	
        virtual void loadSettings();
        virtual void applySettings();
        virtual void defaultSettings();

};


#endif

