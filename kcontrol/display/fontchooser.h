/* This file is part of the KDE libraries
    Copyright (C) 1998	Mark Donohoe <donohoe@kde.org>
						Stephan Kulow				  

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

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

/**
 * The KFontChooser class is a specialist widget that allows a user to change the
 * properties of a QFont. The typeface, weight, slant, size and character set of
 * the font can be changed.
 *
 * Every time a change is made the signal fontChanged() is emmited which passes
 * the new QFont as an argument. Connect to this signal to keep informed of the
 * changes a user makes.
 *
 * Fonts produced by KFontChooser are compatible with KDE charsets.
 */
class KFontChooser : public QWidget
{
	Q_OBJECT
	
public:
	
	/**
	 * Creates a KFontChooser widget with a parent and a name.
	 */
	KFontChooser( QWidget *parent = 0, const char *name = 0 );
	
	/**
	 * Detroys the KFontChooser.
	 */
	~KFontChooser() {};
	
	/**
	 * Sets a new font start_fnt, that the user can start work
	 * to alter.
	 *
	 * If the flag fixed is set to TRUE then only font families
	 * which have monospacing or character cell spacing are shown as
	 * alternatives.
	 */
	void setFont( QFont start_fnt, bool fixed = false );

signals:
	 /** 
	  * Connect to this signal to be informed when changes are made to the
	  * font. 
	  */
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
