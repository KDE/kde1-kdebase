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

#include "display.h"
#include <X11/X.h>
#include <kcontrol.h>

#include <qlistbox.h>
#include <qslider.h>

#define MAX_HOTSPOTS   13

class HotSpot
{
public:
	HotSpot() {}
	HotSpot( const QRect &r, int num )
		{	rect = r; number = num; }

	QRect rect;
	int number;
};

class WidgetCanvas : public QWidget
{
	Q_OBJECT
public:
	WidgetCanvas( QWidget *);
	void drawSampleWidgets();
	QPixmap smplw;
	
	QColor inactiveTitleColor;
	QColor inactiveTextColor;
	QColor activeTitleColor;
	QColor activeTextColor;
	QColor backgroundColor;
	QColor textColor;
	QColor selectColor;
	QColor selectTextColor;
	QColor windowColor;
	QColor windowTextColor;
	
	int contrast;

signals:
	void widgetSelected( int );
	
protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent( QMouseEvent * );
	void paletteChange(const QPalette &);

	HotSpot hotspots[MAX_HOTSPOTS];
};


class KColorScheme : public KDisplayModule
{
	Q_OBJECT
public:
	KColorScheme( QWidget *parent, int mode, int desktop = 0 );
	~KColorScheme();
	
	//KConfig *systemConfig;

	virtual void readSettings( int deskNum = 0 );
	virtual void apply( bool Force = FALSE);

        virtual void loadSettings();
        virtual void applySettings();
	
	QColor colorPushColor;
	
	Display *kde_display;
	Atom 		KDEChangePalette;
	
	SaveScm *ss;
	void initSchemeList();
	void writeScheme();
	void installSchemes();

protected slots:
	void slotApply();
	void slotPreviewScheme(int);
	void slotHelp();
	void slotWidgetColor(int );
	void slotSelectColor( const QColor &col );
	void slotSave();
	void sliderValueChanged(int val);
	void resizeEvent( QResizeEvent * );

protected:
	void writeSettings();
	void writeNamedColor(KConfigBase *config, const char *key, const char *name);

protected:

	QSlider *sb;
	QComboBox *wcCombo;
	KColorButton *colorButton;
	WidgetCanvas* sampleWidgets;
	QPushButton *saveBt;
	QPushButton *removeBt;
	QListBox *sList;
	QStrList *schemeList;
	QString schemeFile;
	
	bool changed;
	
	Window          root;
	int screen;

};

#endif

