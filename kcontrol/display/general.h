//-----------------------------------------------------------------------------
//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//

#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <qwidget.h>
#include <qpushbt.h>
#include <qlistbox.h>
#include <qlined.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qchkbox.h>
#include "kcontrol.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>


#include "display.h"


class KGeneral : public KDisplayModule
{
	Q_OBJECT
public:
	KGeneral( QWidget *parent, int mode, int desktop = 0 );
	~KGeneral();

	virtual void readSettings( int deskNum = 0 );
	virtual void apply( bool Force = FALSE);

        virtual void loadSettings();
        virtual void applySettings();
	
	QFont generalFont;
	GUIStyle applicationStyle;
	
	Display *kde_display;
	Atom 	KDEChangeGeneral;

protected slots:
	void slotSelectFont( const char *fname );
	void slotFontSize( int );
	void slotCharset( int );
	void slotFontBold( bool );
	void slotFontItalic( bool );
	void slotChangeStyle(int );
	void slotApply();
	void slotPreviewFont(int);
	void slotHelp();
	void setColors();
	void connectColor();

protected:
	void writeSettings();
	void getFontList( QStrList &list, const char *pattern );
	void addFont( QStrList &list, const char *xfont );
	
protected:
	QComboBox *stCombo;
	QComboBox *fontCombo;
	QComboBox *charset_combo;
	QCheckBox *cb1, *cb2;
	QButtonGroup *btnGrp;
	QPushButton *changeBt;
	QListBox *fontTypeList;
	QLabel *example_label;
	QLabel *charset_label;
	QStrList fontList;
	
	Bool changed;
	
	Window root;
	int screen;
};

#endif

