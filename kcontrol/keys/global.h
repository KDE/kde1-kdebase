//
// KDE Shotcut config module
//
// Copyright (c)  Mark Donohoe 1998
//

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <qwidget.h>
#include <qpushbt.h>
#include <qcombo.h>
#include <qlistbox.h>
#include <qradiobt.h>
#include <qlined.h>
#include <qbttngrp.h>
#include <kcontrol.h>

#include "savescm.h"
#include "kaccel.h"
#include "kkeydialog.h"

class KGlobalConfig : public KConfigWidget
{
	Q_OBJECT
public:
	KAccel *keys;
	QDict<KKeyEntry> dict;
	KKeyChooser *kc;
	enum { Portrait = 1, Landscape };
	enum { Gradient = 1, Pattern };

	KGlobalConfig( QWidget *parent, const char *name = 0 );

	virtual void readSettings( );
	virtual void apply( bool force = FALSE );

        virtual void loadSettings();
        virtual void applySettings();

protected slots:
	void slotApply();
	void slotSelectColor1( const QColor &col );
	void slotSelectColor2( const QColor &col );
	void slotBrowse();
	void slotWallpaper( const char * );
	void slotWallpaperMode( int );
	void slotColorMode( int );
	void slotStyleMode( int );
	void slotSwitchDesk( int );
	void slotRenameDesk();
	void slotHelp();
	void slotSetup2Color();
	void resizeEvent( QResizeEvent * );
	void slotPreviewScheme( int );
	void slotAdd();
	void slotSave();
	void slotRemove();
	void slotChanged();

protected:
	void getDeskNameList();
	void setDesktop( int );
	void showSettings();
	void writeSettings( );
	void setMonitor();
	int  loadWallpaper( const char *filename, bool useContext = TRUE );
	void retainResources();
	void readSchemeNames( );
	void readScheme( int index = 0 );

protected:
	enum { Tiled = 1, Centred, Scaled };
	enum { OneColor = 1, TwoColor };
	
	QListBox *sList;
	QStrList *sFileList;
	QDict<int> *globalDict;
	QPushButton *saveBt;
	QPushButton *addBt;
	QPushButton *removeBt;
	QListBox     *deskListBox;
	QRadioButton *rbPattern;
	QRadioButton *rbGradient;
	QButtonGroup *ncGroup;
	QButtonGroup *stGroup;
	QButtonGroup *wpGroup;
	QPushButton  *changeButton;
	QComboBox *wpCombo;
	QColor color1;
	QColor color2;
	QString wallpaper;
	QPixmap wpPixmap;
	QString deskName;
	QStrList deskNames;
	int wpMode;
	int ncMode;
	int stMode;
	int orMode;
	int deskNum;
	int maxDesks;
	int nSysSchemes;

        uint pattern[8];

	bool changed;
};


#endif

