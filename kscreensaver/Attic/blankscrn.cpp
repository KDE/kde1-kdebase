//-----------------------------------------------------------------------------
//
// kblankscrn - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//

#include <stdlib.h>
#include <qcolor.h>
#include <qlabel.h>
#include <kapp.h>
#include "kcolordlg.h"
#include "blankscrn.h"

#include "blankscrn.moc"

static KBlankSaver *saver = NULL;

//-----------------------------------------------------------------------------
// standard screen saver interface functions
//
void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new KBlankSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	KBlankSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return "Blank Screen";
}

//-----------------------------------------------------------------------------
// dialog to setup screen saver parameters
//
KBlankSetup::KBlankSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	readSettings();

	QLabel *label;
	QPushButton *button;

	setCaption( "Setup kblankscrn" );

	label = new QLabel( "Color:", this );
	label->setGeometry( 15, 15, 60, 20 );

	colorPush = new QPushButton( "", this );
	colorPush->setGeometry( 15, 40, 90, 25 );
	QColorGroup colgrp( black, color, color.light(),
		color.dark(), color.dark(120), black, white );
	colorPush->setPalette( QPalette( colgrp, colgrp, colgrp ) );
	connect( colorPush, SIGNAL( clicked() ), SLOT( slotColor() ) );

	preview = new QWidget( this );
	preview->setGeometry( 130, 15, 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new KBlankSaver( preview->winId() );

	button = new QPushButton( "Ok", this );
	button->setGeometry( 235, 210, 50, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = new QPushButton( "Cancel", this );
	button->setGeometry( 300, 210, 50, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
}

// read settings from config file
void KBlankSetup::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Color" );
	if ( !str.isNull() )
		color.setNamedColor( str );
	else
		color = black;
}

void KBlankSetup::slotColor()
{
	if ( KColorDialog::getColor( color ) == QDialog::Accepted )
	{
		saver->setColor( color );
		QColorGroup colgrp( black, color, color.light(),
			color.dark(), color.dark(120), black, white );
		colorPush->setPalette( QPalette( colgrp, colgrp, colgrp ) );
	}
}

// Ok pressed - save settings and exit
void KBlankSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString sColor;
	sColor.sprintf("#%02x%02x%02x", color.red(), color.green(), color.blue());
	config->writeEntry( "Color", sColor );

	config->sync();

	accept();
}

//-----------------------------------------------------------------------------


KBlankSaver::KBlankSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();

	blank();
}

KBlankSaver::~KBlankSaver()
{
}

// set the color
void KBlankSaver::setColor( const QColor &col )
{
	color = col;
	blank();
}

// read configuration settings from config file
void KBlankSaver::readSettings()
{
	QString str;

	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	str = config->readEntry( "Color" );
	if ( !str.isNull() )
		color.setNamedColor( str );
	else
		color = black;
}

void KBlankSaver::blank()
{
	XSetWindowBackground( qt_xdisplay(), d, color.pixel() );
	XClearWindow( qt_xdisplay(), d );
}

