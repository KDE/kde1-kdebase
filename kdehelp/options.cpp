//
// KDE Help Options
//
// (c) Martin R. Jones 1996
//

#include <qbttngrp.h>
#include <qradiobt.h>
#include <qpushbt.h>
#include <qcombo.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <kapp.h>
#include <kconfig.h>
#include <X11/Xlib.h>

#include "kcolorbtn.h"
#include "options.h"

#include "dbnew.h"

#include "options.moc"

#include <klocale.h>

//-----------------------------------------------------------------------------

KFontOptions::KFontOptions( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	readOptions();

	QRadioButton *rb;
	QLabel *label;

	QButtonGroup *bg = new QButtonGroup( klocale->translate("Font Size"), this );
	bg->setExclusive( TRUE );
	bg->setGeometry( 15, 15, 300, 50 );

	rb = new QRadioButton( klocale->translate("Small"), bg );
	rb->setGeometry( 10, 20, 80, 20 );
	rb->setChecked( fSize == 3 );

	rb = new QRadioButton( klocale->translate("Medium"), bg );
	rb->setGeometry( 100, 20, 80, 20 );
	rb->setChecked( fSize == 4 );

	rb = new QRadioButton( klocale->translate("Large"), bg );
	rb->setGeometry( 200, 20, 80, 20 );
	rb->setChecked( fSize == 5 );

	label = new QLabel( klocale->translate("Standard Font"), this );
	label->setGeometry( 15, 90, 100, 20 );

	QComboBox *cb = new QComboBox( false, this );
	cb->setGeometry( 120, 90, 180, 25 );
	getFontList( standardFonts, "-*-*-*-*-*-*-*-*-*-*-p-*-*-*" );
	cb->insertStrList( &standardFonts );
	QStrListIterator sit( standardFonts );
	int i;
	for ( i = 0; sit.current(); ++sit, i++ )
	{
		if ( !strcmp( stdName, sit.current() ) )
			cb->setCurrentItem( i );
	}
	connect( cb, SIGNAL( activated( const char * ) ),
		SLOT( slotStandardFont( const char * ) ) );

	label = new QLabel( klocale->translate( "Fixed Font"), this );
	label->setGeometry( 15, 130, 100, 20 );

	cb = new QComboBox( false, this );
	cb->setGeometry( 120, 130, 180, 25 );
	getFontList( fixedFonts, "-*-*-*-*-*-*-*-*-*-*-m-*-*-*" );
	getFontList( fixedFonts, "-*-*-*-*-*-*-*-*-*-*-c-*-*-*" );
	cb->insertStrList( &fixedFonts );
	QStrListIterator fit( fixedFonts );
	for ( i = 0; fit.current(); ++fit, i++ )
	{
		if ( !strcmp( fixedName, fit.current() ) )
			cb->setCurrentItem( i );
	}
	connect( cb, SIGNAL( activated( const char * ) ),
		SLOT( slotFixedFont( const char * ) ) );

	connect( bg, SIGNAL( clicked( int ) ), SLOT( slotFontSize( int ) ) );
}

void KFontOptions::readOptions()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );
	
	QString fs = config->readEntry( "BaseFontSize" );
	if ( !fs.isEmpty() )
	{
		fSize = fs.toInt();
		if ( fSize < 3 )
			fSize = 3;
		else if ( fSize > 5 )
			fSize = 5;
	}
	else
		fSize = 3;

	stdName = config->readEntry( "StandardFont" );
	if ( stdName.isEmpty() )
		stdName = "times";

	fixedName = config->readEntry( "FixedFont" );
	if ( fixedName.isEmpty() )
		fixedName = "courier";
}

void KFontOptions::getFontList( QStrList &list, const char *pattern )
{
	int num;

	char **xFonts = XListFonts( qt_xdisplay(), pattern, 200, &num );

	for ( int i = 0; i < num; i++ )
	{
		addFont( list, xFonts[i] );
	}

	XFreeFontNames( xFonts );
}

void KFontOptions::addFont( QStrList &list, const char *xfont )
{
	const char *ptr = strchr( xfont, '-' );
	if ( !ptr )
		return;
	
	ptr = strchr( ptr + 1, '-' );
	if ( !ptr )
		return;

	QString font = ptr + 1;

	int pos;
	if ( ( pos = font.find( '-' ) ) > 0 )
	{
		font.truncate( pos );

		if ( font.find( "open look", 0, false ) >= 0 )
			return;

		QStrListIterator it( list );

		for ( ; it.current(); ++it )
			if ( it.current() == font )
				return;

		list.append( font );
	}
}

void KFontOptions::slotApplyPressed()
{
	QString o;

	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );

	QString fs;
	fs.setNum( fSize );
	o = config->writeEntry( "BaseFontSize", fs );
	if ( o.isNull() || o.toInt() != fSize )
		emit fontSize( fSize );

	o = config->writeEntry( "StandardFont", stdName );
	if ( o.isNull() || o != stdName )
		emit standardFont( stdName );

	o = config->writeEntry( "FixedFont", fixedName );
	if ( o.isNull() || o != fixedName )
		emit fixedFont( fixedName );

	config->sync();
}

void KFontOptions::slotFontSize( int i )
{
	fSize = i+3;
}

void KFontOptions::slotStandardFont( const char *n )
{
	stdName = n;
}

void KFontOptions::slotFixedFont( const char *n )
{
	fixedName = n;
}

//-----------------------------------------------------------------------------

KColorOptions::KColorOptions( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
	readOptions();

	KColorButton *colorBtn;
	QLabel *label;

	label = new QLabel( klocale->translate("Background Color:"), this );
	label->setGeometry( 35, 20, 150, 25 );

	colorBtn = new KColorButton( bgColor, this );
	colorBtn->setGeometry( 185, 20, 80, 30 );
	connect( colorBtn, SIGNAL( changed( const QColor & ) ),
		SLOT( slotBgColorChanged( const QColor & ) ) );

	label = new QLabel( klocale->translate("Normal Text Color:"), this );
	label->setGeometry( 35, 60, 150, 25 );

	colorBtn = new KColorButton( textColor, this );
	colorBtn->setGeometry( 185, 60, 80, 30 );
	connect( colorBtn, SIGNAL( changed( const QColor & ) ),
		SLOT( slotTextColorChanged( const QColor & ) ) );

	label = new QLabel( klocale->translate("URL Link Color:"), this );
	label->setGeometry( 35, 100, 150, 25 );

	colorBtn = new KColorButton( linkColor, this );
	colorBtn->setGeometry( 185, 100, 80, 30 );
	connect( colorBtn, SIGNAL( changed( const QColor & ) ),
		SLOT( slotLinkColorChanged( const QColor & ) ) );

	label = new QLabel( klocale->translate("Followed Link Color:"), this );
	label->setGeometry( 35, 140, 150, 25 );

	colorBtn = new KColorButton( vLinkColor, this );
	colorBtn->setGeometry( 185, 140, 80, 30 );
	connect( colorBtn, SIGNAL( changed( const QColor & ) ),
		SLOT( slotVLinkColorChanged( const QColor & ) ) );
}

void KColorOptions::readOptions()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );
	
	bgColor = config->readColorEntry( "BgColor", &white );
	textColor = config->readColorEntry( "TextColor", &black );
	linkColor = config->readColorEntry( "LinkColor", &blue );
	vLinkColor = config->readColorEntry( "VLinkColor", &magenta );

	changed = false;
}

void KColorOptions::slotApplyPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );

	config->writeEntry( "BgColor", bgColor );
	config->writeEntry( "TextColor", textColor );
	config->writeEntry( "LinkColor", linkColor );
	config->writeEntry( "VLinkColor", vLinkColor );

	if ( changed )
	    emit colorsChanged( bgColor, textColor, linkColor, vLinkColor );

	config->sync();
}

void KColorOptions::slotBgColorChanged( const QColor &col )
{
	if ( bgColor != col )
    	    changed = true;
	bgColor = col;
}

void KColorOptions::slotTextColorChanged( const QColor &col )
{
	if ( textColor != col )
	    changed = true;
	textColor = col;
}

void KColorOptions::slotLinkColorChanged( const QColor &col )
{
	if ( linkColor != col )
    	    changed = true;
	linkColor = col;
}

void KColorOptions::slotVLinkColorChanged( const QColor &col )
{
	if ( vLinkColor != col )
    	    changed = true;
	vLinkColor = col;
}

//-----------------------------------------------------------------------------

KHelpOptionsDialog::KHelpOptionsDialog( QWidget *parent, const char *name )
	: QTabDialog( parent, name )
{
	setCaption( klocale->translate("KDE Help Options") );

	resize( 350, 300 );

        setOKButton( klocale->translate("OK") );
        setCancelButton( klocale->translate("Cancel") );
        setApplyButton( klocale->translate("Apply") );

	fontOptions = new KFontOptions( this, klocale->translate("Fonts") );
	addTab( fontOptions, klocale->translate("Fonts") );
	connect( this, SIGNAL( applyButtonPressed() ),
		fontOptions, SLOT( slotApplyPressed() ) );

	colorOptions = new KColorOptions( this, klocale->translate("Colors") );
	addTab( colorOptions, klocale->translate("Colors") );
	connect( this, SIGNAL( applyButtonPressed() ),
		colorOptions, SLOT( slotApplyPressed() ) );
}

