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
#include <kapp.h>
#include <Kconfig.h>
#include <X11/Xlib.h>

#include "options.h"

#include "dbnew.h"

#include "options.moc"


//-----------------------------------------------------------------------------

KFontOptions::KFontOptions( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	readOptions();

	QRadioButton *rb;
	QLabel *label;

	QButtonGroup *bg = new QButtonGroup( "Font Size", this );
	bg->setExclusive( TRUE );
	bg->setGeometry( 15, 15, 300, 50 );

	rb = new QRadioButton( "Small", bg );
	rb->setGeometry( 10, 20, 60, 20 );
	rb->setChecked( fSize == 3 );

	rb = new QRadioButton( "Medium", bg );
	rb->setGeometry( 100, 20, 60, 20 );
	rb->setChecked( fSize == 4 );

	rb = new QRadioButton( "Large", bg );
	rb->setGeometry( 200, 20, 60, 20 );
	rb->setChecked( fSize == 5 );

	label = new QLabel( "Standard Font", this );
	label->setGeometry( 15, 90, 100, 20 );

	QComboBox *cb = new QComboBox( false, this );
	cb->setGeometry( 120, 90, 180, 25 );
	getFontList( standardFonts, "-*-*-*-*-*-*-*-*-*-*-p-*-*-*" );
	cb->insertStrList( &standardFonts );
	QStrListIterator sit( standardFonts );
	for ( int i = 0; sit.current(); ++sit, i++ )
	{
		if ( !strcmp( stdName, sit.current() ) )
			cb->setCurrentItem( i );
	}
	connect( cb, SIGNAL( highlighted( const char * ) ),
		SLOT( slotStandardFont( const char * ) ) );

	label = new QLabel( "Fixed Font", this );
	label->setGeometry( 15, 130, 100, 20 );

	cb = new QComboBox( false, this );
	cb->setGeometry( 120, 130, 180, 25 );
	getFontList( fixedFonts, "-*-*-*-*-*-*-*-*-*-*-m-*-*-*" );
	cb->insertStrList( &fixedFonts );
	QStrListIterator fit( fixedFonts );
	for ( int i = 0; fit.current(); ++fit, i++ )
	{
		if ( !strcmp( fixedName, fit.current() ) )
			cb->setCurrentItem( i );
	}
	connect( cb, SIGNAL( highlighted( const char * ) ),
		SLOT( slotFixedFont( const char * ) ) );

	connect( bg, SIGNAL( clicked( int ) ), SLOT( slotFontSize( int ) ) );
}

void KFontOptions::readOptions()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Appearance" );
	
	QString fs = config->readEntry( "BaseFontSize" );
	if ( !fs.isNull() )
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
	if ( stdName.isNull() )
		stdName = "times";

	fixedName = config->readEntry( "FixedFont" );
	if ( fixedName.isNull() )
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

//	config->sync();
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

KHelpOptionsDialog::KHelpOptionsDialog( QWidget *parent, const char *name )
	: QTabDialog( parent, name )
{
	setCaption( "KDE Help Options" );

	resize( 350, 300 );

	setCancelButton();
	setApplyButton();

	fontOptions = new KFontOptions( this, "Fonts" );
	addTab( fontOptions, "Fonts" );
	connect( this, SIGNAL( applyButtonPressed() ),
		fontOptions, SLOT( slotApplyPressed() ) );
}

