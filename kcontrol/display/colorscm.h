//-----------------------------------------------------------------------------
//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//

#ifndef __COLORSCM_H__
#define __COLORSCM_H__

#include "kcolorbtn.h"
#include "savescm.h"
#include "widgetcanvas.h"

#include "display.h"
#include <X11/X.h>
#include <kcontrol.h>

#include <qlistbox.h>
#include <qslider.h>

class KColorScheme : public KDisplayModule
{
	Q_OBJECT
public:
	KColorScheme( QWidget *parent, int mode, int desktop = 0 );
	~KColorScheme();
	
	virtual void readSettings( int ) {};
	virtual void apply( bool Force = FALSE);
	virtual void applySettings();
	virtual void loadSettings();
	virtual void defaultSettings();
	
	QColor colorPushColor;
	
	Display *kde_display;
	Atom KDEChangePalette;
	
protected slots:
	void slotApply();
	void slotPreviewScheme( int );
	void slotHelp();
	void slotWidgetColor( int );
	void slotSelectColor( const QColor &col );
	void slotSave();
	void slotAdd();
	void slotRemove();
	void sliderValueChanged( int val );
	void resizeEvent( QResizeEvent * );

protected:
	void writeSettings();
	void writeNamedColor( KConfigBase *config, 
				const char *key, const char *name );
	void readSchemeNames();
	void readScheme( int index = 0 );
	void writeScheme();
	void setDefaults();
	
protected:
	QSlider *sb;
	QComboBox *wcCombo;
	KColorButton *colorButton;
	WidgetCanvas *cs;
	QPushButton *saveBt;
	QPushButton *addBt;
	QPushButton *removeBt;
	QListBox *sList;
	QStrList *schemeList;
	QStrList *sFileList;
	QString schemeFile;
	int nSysSchemes;
	
	bool changed;
	bool useRM;
	
	Window          root;
	int screen;

};

#endif

