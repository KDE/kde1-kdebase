//
// A widget for choosing fonts
//
// Copyright (c)  Mark Donohoe 1998
//

#ifndef __FONTCHOOSER_H__
#define __FONTCHOOSER_H__

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

#endif
