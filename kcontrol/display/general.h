//-----------------------------------------------------------------------------
//
// KDE Display fonts, styles setup module
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
#include <kspinbox.h>
#include <kcontrol.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>


#include "display.h"


class KFontChooser : public QWidget
{
	Q_OBJECT
public:
	KFontChooser::KFontChooser( QWidget *parent=0, const char *name=0 );
	~KFontChooser() {};

signals:
	void fontChanged( QFont font );

protected slots:
	void slotSelectFont( const char *fname );
	void slotFontSize( );
	void slotCharset( const char * );
	void slotFontBold( bool );
	void slotFontItalic( bool );

protected:
	void fillCharsetCombo();
	void getFontList( QStrList &list, bool fixed = false );
	void getFontList( QStrList &list, const char *pattern );
	void addFont( QStrList &list, const char *xfont );
	
protected:
	QFont fnt;
	bool changed;
	
	KNumericSpinBox *sbSize;
	QComboBox *cmbFont;
	QComboBox *cmbCharset;
	QCheckBox *cbBold, *cbItalic;
	QButtonGroup *btnGrp;
	QPushButton *changeBt;
	
	QLabel *example_label;
	QLabel *charset_label;
	QStrList fontList;
	Bool defaultCharset;
};

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
	
	void slotChangeStyle();
	void slotApply();
	void slotPreviewFont( QFont fnt );
	void slotHelp();
	void setColors();
	void connectColor();

protected:

	void writeSettings();

	
protected:
	QComboBox *stCombo;
	KFontChooser *fntChooser;
	QLabel *lSample;
	QLabel *example_label;
	QCheckBox *cbStyle;
	
	QListBox *fontTypeList;
	Bool changed;
	
	Bool defaultCharset;
	Window root;
	int screen;
};

#endif

