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
#include <qlayout.h>
#include <qradiobutton.h>
#include <kspinbox.h>
#include <kcontrol.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "display.h"
#include "fontchooser.h"

class FontUseItem
{
public:
    FontUseItem( const char *n, QFont default_fnt, bool fixed = false );
	QString fontString( QFont rFont );
	void setRC( const char *group, const char *key, const char *rc = 0 );
	void readFont();
	void writeFont();
	void setDefault();
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
	QString _text;
	QString _rcfile;
	QString _rcgroup;
	QString _rckey;
	QFont _font;
	QFont _default;
	bool fixed;
	bool selected;
};

class FontPreview : public QLabel
{
	Q_OBJECT
	
public:
	FontPreview( QWidget *parent=0, const char * name=0 )
		: QLabel( parent, name ) {};
	~FontPreview() {};
	void setPreviewFont( const QFont &fnt );
	
protected slots:
	void fontChange( const QFont & );
	
private:
	QFont _fnt;
};

// DF 13-Mar-99
// This class is a part of the "style" tab.
// It's separated from the KGeneral class, in case it has to be moved
// somewhere else later.
class KIconStyle // : public KDisplayModule
{
    //Q_OBJECT
public:
	KIconStyle( QWidget *parent, QBoxLayout * topLayout );
	~KIconStyle();

	void apply();
	void readSettings();
	
	void writeSettings();
	void setDefaults();

protected:
	static const char *appName [];
	static const int nApp;

	QDict <char> m_dictSettings; // initial setting for each application
	QDict <QRadioButton> m_dictCBNormal; // checkbox 'normal' for each application
	QDict <QRadioButton> m_dictCBLarge;  // checkbox 'large' for each application
};

class KGeneral : public KDisplayModule
{
	Q_OBJECT
	
public:
	KGeneral( QWidget *parent, int mode, int desktop = 0 );
	~KGeneral();

	virtual void readSettings( int deskNum = 0 );
	virtual void apply( bool Force = FALSE);
	virtual void loadSettings() {};
	virtual void applySettings();
	virtual void defaultSettings();
	
	QFont generalFont;
	GUIStyle applicationStyle;
	
	Display *kde_display;
	Atom 	KDEChangeGeneral;

protected slots:
	void slotChangeStyle();
	void slotUseResourceManager();
	void slotMacStyle();//CT
	void slotApply();
	void slotHelp();

protected:
	void writeSettings();
	void setDefaults();
	
protected:
	QCheckBox *cbStyle, *cbRes;
	QCheckBox *cbMac;//CT
	Bool changed;
	
	bool useRM;
	bool macStyle;//CT
	Window root;
	int screen;

	KIconStyle * iconStyle;
};

class KFonts : public KDisplayModule
{
	Q_OBJECT
	
public:
	KFonts( QWidget *parent, int mode, int desktop = 0 );
	~KFonts();

	virtual void readSettings( int deskNum = 0 );
	virtual void apply( bool Force = FALSE);
	virtual void loadSettings() {};
	virtual void applySettings();
	virtual void defaultSettings();
	
	QFont generalFont;
	
	Display *kde_display;
	Atom 	KDEChangeGeneral;

protected slots:
	void slotApply();
	void slotSetFont( QFont fnt );
	void slotPreviewFont( int index );
	void slotHelp();

protected:
	void writeSettings();
	void setDefaults();
	
protected:
	KFontChooser *fntChooser;
	FontPreview *lSample;
	QListBox *lbFonts;
	bool useRM;
	
	Bool changed;
	
	QList <FontUseItem> fontUseList;
	
	Bool defaultCharset;
	Window root;
	int screen;
};

#endif

