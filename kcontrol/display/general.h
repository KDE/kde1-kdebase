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
	KFontChooser::KFontChooser( QWidget *parent = 0, const char *name = 0 );
	~KFontChooser() {};
	void setFont( QFont start_fnt, bool fixed = false );

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
	QStrList fixedList;
	Bool defaultCharset;
};

class FontUseItem
{
public:
    FontUseItem( const char *n, QFont default_fnt, bool fixed = false );
	void setRC( const char *group, const char *key, const char *rc = 0 );
	void readFont();
	void writeFont();
	void setFont( QFont fnt ) { _font = fnt; }
	QFont font() { return _font; }
	const char *rcFile() { return _rcfile.data(); }
	const char *rcGroup() { return _rcgroup.data(); }
	const char *rcKey() { return _rckey.data(); }
	const char *text()		{ return _text.data(); }
	bool spacing() { return fixed; }
	void	setSelect( bool flag )	{ selected = flag; }
	bool	select()		{ return selected; }

    private:
	bool	fixed;
	bool	selected;
	QString _text;
	QString _rcfile;
	QString _rcgroup;
	QString _rckey;
	QFont _font;
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
	void slotSetFont( QFont fnt );
	void slotPreviewFont( int index );
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
	
	QListBox *lbFonts;
	Bool changed;
	
	QList <FontUseItem> fontUseList;
	
	Bool defaultCharset;
	Window root;
	int screen;
};

#endif

