//-----------------------------------------------------------------------------
//
// kbanner - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//

#include <stdlib.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qscrbar.h>
#include <qcombo.h>
#include <qchkbox.h>
#include <qgrpbox.h>
#include <qmsgbox.h>
#include <kapp.h>

#include "kslider.h"
#include "kcolordlg.h"
#include "banner.h"

#include "banner.moc"

static KBannerSaver *saver = NULL;

//-----------------------------------------------------------------------------
// standard screen saver interface functions
//
void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new KBannerSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	KBannerSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return "Banner";
}

//-----------------------------------------------------------------------------

static char *fonts[] = { "Courier", "Helvetica", "Times", NULL };
static int  sizes[] = { 12, 14, 18, 24, 48, 96, 0 };

KBannerSetup::KBannerSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	saver = NULL;
	speed = 50;

	readSettings();

	QString str;
	QLabel *label;
	QPushButton *button;
	KSlider *sb;
	QComboBox *combo;

	setCaption( "Setup kbanner" );

	QGroupBox *group = new QGroupBox( "Font", this );
	group->setGeometry( 15, 15, 180, 140 );

	label = new QLabel( "Family:", group );
	label->setGeometry( 15, 15, 50, 20 );

	combo = new QComboBox( FALSE, group );
	combo->setGeometry( 65, 15, 100, 25 );
	int i = 0;
	while ( fonts[i] )
	{
		combo->insertItem( fonts[i], i );
		if ( fontFamily == fonts[i] )
			combo->setCurrentItem( i );
		i++;
	}
	connect( combo, SIGNAL( activated( const char * ) ),
			SLOT( slotFamily( const char * ) ) );

	label = new QLabel( "Size:", group );
	label->setGeometry( 15, 45, 60, 20 );

	combo = new QComboBox( FALSE, group );
	combo->setGeometry( 65, 45, 100, 25 );
	i = 0;
	while ( sizes[i] )
	{
		QString num;
		num.setNum( sizes[i] );
		combo->insertItem( num, i );
		if ( fontSize == sizes[i] )
			combo->setCurrentItem( i );
		i++;
	}
	connect( combo, SIGNAL( activated( int ) ), SLOT( slotSize( int ) ) );

	label = new QLabel( "Color:", group );
	label->setGeometry( 15, 80, 50, 20 );

	colorPush = new QPushButton( group );
	colorPush->setGeometry( 65, 80, 100, 20 );
	QColorGroup colgrp( black, fontColor, fontColor.light(),
			fontColor.dark(), fontColor.dark(120), black, white );
	colorPush->setPalette( QPalette( colgrp, colgrp, colgrp ) );
	connect( colorPush, SIGNAL( clicked() ), SLOT( slotColor() ) );

	QCheckBox *cb = new QCheckBox( "Bold", group );
	cb->setGeometry( 15, 110, 60, 20 );
	cb->setChecked( bold );
	connect( cb, SIGNAL( toggled( bool ) ), SLOT( slotBold( bool ) ) );

	cb = new QCheckBox( "Italic", group );
	cb->setGeometry( 95, 110, 60, 20 );
	cb->setChecked( italic );
	connect( cb, SIGNAL( toggled( bool ) ), SLOT( slotItalic( bool ) ) );

	label = new QLabel( "Speed:", this );
	label->setGeometry( 15, 155, 60, 20 );

	sb = new KSlider( KSlider::Horizontal, this );
	sb->setGeometry( 15, 175, 180, 20 );
	sb->setRange( 0, 100 );
	sb->setSteps( 25, 50 );
	sb->setValue( speed );
	connect( sb, SIGNAL( valueChanged( int ) ), SLOT( slotSpeed( int ) ) );

	label = new QLabel( "Message:", this );
	label->setGeometry( 15, 205, 60, 20 );

	QLineEdit *ed = new QLineEdit( this );
	ed->setGeometry( 75, 205, 355, 20 );
	ed->setText( message );
	connect( ed, SIGNAL( textChanged( const char * ) ), 
			SLOT( slotMessage( const char * ) ) );

	preview = new QWidget( this );
	preview->setGeometry( 210, 15, 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new KBannerSaver( preview->winId() );

	button = new QPushButton( "About", this );
	button->setGeometry( 210, 245, 50, 25 );
	connect( button, SIGNAL( clicked() ), SLOT(slotAbout() ) );

	button = new QPushButton( "Ok", this );
	button->setGeometry( 315, 245, 50, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = new QPushButton( "Cancel", this );
	button->setGeometry( 380, 245, 50, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
}

// read settings from config file
void KBannerSetup::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = atoi( str );
	if ( speed > 100 )
		speed = 100;
	else if ( speed < 50 )
		speed = 50;

	str = config->readEntry( "Message" );
	if ( !str.isNull() )
		message = str;
	else
		message = "KDE";
	
	str = config->readEntry( "FontFamily" );
	if ( !str.isNull() )
		fontFamily = str;
	else
		fontFamily = "Times";
	
	str = config->readEntry( "FontSize" );
	if ( !str.isNull() )
		fontSize = atoi( str );
	else
		fontSize = 48;

	str = config->readEntry( "FontColor" );
	if ( !str.isNull() )
		fontColor.setNamedColor( str );
	else
		fontColor = red;

	str = config->readEntry( "FontBold" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		bold = TRUE;
	else
		bold = FALSE;

	str = config->readEntry( "FontItalic" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		italic = TRUE;
	else
		italic = FALSE;
}

void KBannerSetup::slotFamily( const char *fam )
{
	fontFamily = fam;
	if ( saver )
		saver->setFont( fontFamily, fontSize, fontColor, bold, italic );
}

void KBannerSetup::slotSize( int indx )
{
	fontSize = sizes[indx];
	if ( saver )
		saver->setFont( fontFamily, fontSize, fontColor, bold, italic );
}

void KBannerSetup::slotColor()
{
	if ( KColorDialog::getColor( fontColor ) == QDialog::Rejected )
		return;

	QColorGroup colgrp( black, fontColor, fontColor.light(),
		fontColor.dark(), fontColor.dark(120), black, white );
	colorPush->setPalette( QPalette( colgrp, colgrp, colgrp ) );

	if ( saver )
		saver->setFont( fontFamily, fontSize, fontColor, bold, italic );
}

void KBannerSetup::slotBold( bool state )
{
	bold = state;
	if ( saver )
		saver->setFont( fontFamily, fontSize, fontColor, bold, italic );
}

void KBannerSetup::slotItalic( bool state )
{
	italic = state;
	if ( saver )
		saver->setFont( fontFamily, fontSize, fontColor, bold, italic );
}

void KBannerSetup::slotSpeed( int num )
{
	speed = num;
	if ( saver )
		saver->setSpeed( speed );
}

void KBannerSetup::slotMessage( const char *msg )
{
	message = msg;
	if ( saver )
		saver->setMessage( message );
}

// Ok pressed - save settings and exit
void KBannerSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString sspeed;
	sspeed.setNum( speed );
	config->writeEntry( "Speed", sspeed );

	config->writeEntry( "Message", message );

	config->writeEntry( "FontFamily", fontFamily );

	QString fsize;
	fsize.setNum( fontSize );
	config->writeEntry( "FontSize", fsize );

	QString colName(10);
	colName.sprintf( "#%02x%02x%02x", fontColor.red(), fontColor.green(),
		fontColor.blue() );
	config->writeEntry( "FontColor", colName );

	config->writeEntry( "FontBold", bold ? "yes" : "no" );
	config->writeEntry( "FontItalic", italic ? "yes" : "no" );

	config->sync();

	accept();
}

void KBannerSetup::slotAbout()
{
	QMessageBox::message("About Banner", "Banner Version 0.1\n\r\n\rwritten by Martin R. Jones 1996\n\rmjones@kde.org", "Ok");
}

//-----------------------------------------------------------------------------

KBannerSaver::KBannerSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();
	initialise();

	colorContext = QColor::enterAllocContext();

	blank();

	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

KBannerSaver::~KBannerSaver()
{
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void KBannerSaver::setSpeed( int spd )
{
	timer.stop();
	speed = 101-spd;
	timer.start( speed );
}

void KBannerSaver::setFont( const char *family, int size, const QColor &color,
		bool b, bool i )
{
	fontFamily = family;
	fontSize = size;
	fontColor = color;
	bold = b;
	italic = i;

	blank();
	initialise();
}

void KBannerSaver::setMessage( const char *msg )
{
	XSetForeground( qt_xdisplay(), gc, black.pixel() );
	XDrawString( qt_xdisplay(), d, gc, xpos, ypos, message, message.length() );

	message = msg;
	fwidth = XTextWidth( fontInfo, message, message.length() );
}

// read settings from config file
void KBannerSaver::readSettings()
{
	QString str;

	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = 101 - atoi( str );
	else
		speed = 50;

	str = config->readEntry( "FontFamily" );
	if ( !str.isNull() )
		fontFamily = str;
	else
		fontFamily = "Times";

	str = config->readEntry( "FontSize" );
	if ( !str.isNull() )
		fontSize = atoi( str );
	else
		fontSize = 48;

	str = config->readEntry( "FontColor" );
	if ( !str.isNull() )
		fontColor.setNamedColor( str );
	else
		fontColor = red;

	str = config->readEntry( "FontBold" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		bold = TRUE;
	else
		bold = FALSE;

	str = config->readEntry( "FontItalic" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		italic = TRUE;
	else
		italic = FALSE;

	str = config->readEntry( "Message" );
	if ( !str.isNull() )
		message = str;
	else
		message = "KDE";
}

// initialise font
void KBannerSaver::initialise()
{
	QString font;
	int size;
	char ichar;

	if ( !stricmp( fontFamily, "Helvetica" ) )
		ichar = 'o';
	else
		ichar = 'i';

	size = fontSize * height / QApplication::desktop()->height();

	font.sprintf( "-*-%s-%s-%c-*-*-%d-*-*-*-*-*-*-*", fontFamily.data(),
			bold ? "bold" : "medium", italic ? ichar : 'r', size );
	fontInfo = XLoadQueryFont( qt_xdisplay(), font );

	XSetFont( qt_xdisplay(), gc, fontInfo->fid );

	fwidth = XTextWidth( fontInfo, message, message.length() );

	xpos = width;
	ypos = height / 2;
	step = 6 * width / QApplication::desktop()->width();
	if ( step == 0 )
		step = 1;
}

// erase old text and draw in new position
void KBannerSaver::slotTimeout()
{
	XSetForeground( qt_xdisplay(), gc, black.pixel() );
	XDrawString( qt_xdisplay(), d, gc, xpos, ypos, message, message.length() );

	xpos -= step;
	if ( xpos < -fwidth-(int)width/2 )
		xpos = width;

	XSetForeground( qt_xdisplay(), gc, fontColor.pixel() );
	XDrawString( qt_xdisplay(), d, gc, xpos, ypos, message, message.length() );
}

void KBannerSaver::blank()
{
	XSetWindowBackground( qt_xdisplay(), d, black.pixel() );
	XClearWindow( qt_xdisplay(), d );
}

