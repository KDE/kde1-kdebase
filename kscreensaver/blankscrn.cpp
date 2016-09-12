//-----------------------------------------------------------------------------
//
// kblankscrn - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//
// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>

#include <stdlib.h>
#include <qcolor.h>
#include <qlabel.h>
#include <kapp.h>
#include "kcolordlg.h"
#include "blankscrn.h"
#include "helpers.h"
#include <kbuttonbox.h>
#include <qlayout.h>

#include "blankscrn.moc"

static KBlankSaver *saver = NULL;

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

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
  return glocale->translate("Blank Screen");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
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

	setCaption( glocale->translate("Setup kblankscrn") );

	QVBoxLayout *tl = new QVBoxLayout(this, 10);
	QHBoxLayout *tl1 = new QHBoxLayout;
	tl->addLayout(tl1);

	QVBoxLayout *tl11 = new QVBoxLayout(5);
	tl1->addLayout(tl11);

	label = new QLabel( glocale->translate("Color:"), this );
	min_size(label);
	tl11->addWidget(label);

	colorPush = new QPushButton( "", this );
	min_height(colorPush);
	colorPush->setMinimumWidth(80);
	QColorGroup colgrp( black, color, color.light(),
		color.dark(), color.dark(120), black, white );
	colorPush->setPalette( QPalette( colgrp, colgrp, colgrp ) );
	connect( colorPush, SIGNAL( clicked() ), SLOT( slotColor() ) );
	tl11->addWidget(colorPush);
	tl11->addStretch(1);

	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new KBlankSaver( preview->winId() );
	tl1->addWidget(preview);

	KButtonBox *bbox = new KButtonBox(this);	
	bbox->addStretch(1);

	button = bbox->addButton( glocale->translate("OK"));	
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = bbox->addButton(glocale->translate("Cancel"));
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
	bbox->layout();
	tl->addWidget(bbox);

	tl->freeze();
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

