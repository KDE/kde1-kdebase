//-----------------------------------------------------------------------------
//
// kpolygon - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//

#include <stdlib.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qscrbar.h>
#include <qmsgbox.h>
#include <kapp.h>
#include <time.h>

#include "kslider.h"

#include "polygon.h"

#include "polygon.moc"

#define MAXLENGTH	65
#define MAXVERTICES	19

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kPolygonSaver *saver = NULL;

//-----------------------------------------------------------------------------
// standard screen saver interface functions
//
void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kPolygonSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	kPolygonSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("Polygons");
}

//-----------------------------------------------------------------------------
// dialog to setup screen saver parameters
//
kPolygonSetup::kPolygonSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	saver = NULL;
	length = 10;
	vertices = 3;
	speed = 50;

	readSettings();

	QString str;
	QLabel *label;
	QPushButton *button;
	KSlider *sb;

	setCaption( glocale->translate("Setup kpolygon") );

	label = new QLabel( glocale->translate("Length:"), this );
	label->setGeometry( 15, 15, 60, 20 );

	sb = new KSlider( KSlider::Horizontal, this );
	sb->setGeometry( 15, 35, 90, 20 );
	sb->setRange( 1, MAXLENGTH );
	sb->setSteps( 8, 32 );
	sb->setValue( length );
	connect( sb, SIGNAL( valueChanged( int ) ), SLOT( slotLength( int ) ) );

	label = new QLabel( glocale->translate("Vertices:"), this );
	label->setGeometry( 15, 60, 60, 20 );

	sb = new KSlider( KSlider::Horizontal, this );
	sb->setGeometry( 15, 80, 90, 20 );
	sb->setRange( 3, MAXVERTICES );
	sb->setSteps( 2, 2 );
	sb->setValue( vertices );
	connect( sb, SIGNAL( valueChanged( int ) ), SLOT( slotVertices( int ) ) );

	label = new QLabel( glocale->translate("Speed:"), this );
	label->setGeometry( 15, 105, 60, 20 );

	sb = new KSlider( KSlider::Horizontal, this );
	sb->setGeometry( 15, 125, 90, 20 );
	sb->setRange( 0, 100 );
	sb->setSteps( 25, 50 );
	sb->setValue( speed );
	connect( sb, SIGNAL( valueChanged( int ) ), SLOT( slotSpeed( int ) ) );

	preview = new QWidget( this );
	preview->setGeometry( 130, 15, 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new kPolygonSaver( preview->winId() );

	button = new QPushButton( glocale->translate("About"), this );
	button->setGeometry( 130, 210, 50, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( slotAbout() ) );

	button = new QPushButton( glocale->translate("Ok"), this );
	button->setGeometry( 235, 210, 50, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = new QPushButton( glocale->translate("Cancel"), this );
	button->setGeometry( 300, 210, 50, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
}

// read settings from config file
void kPolygonSetup::readSettings()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Length" );
	if ( !str.isNull() )
		length = atoi( str );
	if ( length > MAXLENGTH )
		length = MAXLENGTH;
	else if ( length < 1 )
		length = 1;

	str = config->readEntry( "Vertices" );
	if ( !str.isNull() )
		vertices = atoi( str );
	if ( vertices > MAXVERTICES )
		vertices = MAXVERTICES;
	else if ( vertices < 3 )
		vertices = 3;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = atoi( str );
	if ( speed > 100 )
		speed = 100;
	else if ( speed < 50 )
		speed = 50;
}

void kPolygonSetup::slotLength( int len )
{
	length = len;
	if ( saver )
		saver->setPolygon( length, vertices );
}

void kPolygonSetup::slotVertices( int num )
{
	vertices = num;
	if ( saver )
		saver->setPolygon( length, vertices );
}

void kPolygonSetup::slotSpeed( int num )
{
	speed = num;
	if ( saver )
		saver->setSpeed( speed );
}

// Ok pressed - save settings and exit
void kPolygonSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString slength;
	slength.setNum( length );
	config->writeEntry( "Length", slength );

	QString svertices;
	svertices.setNum( vertices );
	config->writeEntry( "Vertices", svertices );

	QString sspeed;
	sspeed.setNum( speed );
	config->writeEntry( "Speed", sspeed );

	config->sync();

	accept();
}

void kPolygonSetup::slotAbout()
{
	QMessageBox::message(glocale->translate("About Polygon"), 
			     glocale->translate("Polygon Version 0.1\n\n"\
					       "written by Martin R. Jones 1996\n"\
					       "mjones@kde.org"), 
			     glocale->translate("Ok"));
}

//-----------------------------------------------------------------------------


kPolygonSaver::kPolygonSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	polygons.setAutoDelete( TRUE );

	readSettings();

	srand((int)time((time_t *)NULL));

	directions.resize( numVertices );
	colorContext = QColor::enterAllocContext();

	blank();

	initialiseColor();
	initialisePolygons();

	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kPolygonSaver::~kPolygonSaver()
{
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

// set polygon properties
void kPolygonSaver::setPolygon( int len, int ver )
{
	timer.stop();
	numLines = len;
	numVertices = ver;

	directions.resize( numVertices );
	polygons.clear();
	initialisePolygons();
	blank();

	timer.start( speed );
}

// set the speed
void kPolygonSaver::setSpeed( int spd )
{
	timer.stop();
	speed = 100-spd;
	timer.start( speed );
}

// read configuration settings from config file
void kPolygonSaver::readSettings()
{
	QString str;

	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	str = config->readEntry( "Length" );
	if ( !str.isNull() )
		numLines = atoi( str );
	else
		numLines = 10;

	str = config->readEntry( "Vertices" );
	if ( !str.isNull() )
		numVertices = atoi( str );
	else
		numVertices = 3;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = 100 - atoi( str );
	else
		speed = 50;

	if ( numLines > 50 )
		numLines = 50;
	else if ( numLines < 1 )
		numLines = 1;
	
	if ( numVertices > 20 )
		numVertices = 20;
	else if ( numVertices < 3 )
		numVertices = 3;
}

// draw next polygon and erase tail
void kPolygonSaver::slotTimeout()
{
	if ( polygons.count() > numLines )
	{
		XSetForeground( qt_xdisplay(), gc, black.pixel() );
		XDrawLines( qt_xdisplay(), d, gc, polygons.first()->vertices.data(),
			numVertices+1, CoordModeOrigin );
	}

	nextColor();

	XDrawLines( qt_xdisplay(), d, gc, polygons.last()->vertices.data(),
		numVertices+1, CoordModeOrigin );

	if ( polygons.count() > numLines )
		polygons.removeFirst();

	polygons.append( new Polygon( *(polygons.last()) ) );
	moveVertices();
}

void kPolygonSaver::blank()
{
	XSetWindowBackground( qt_xdisplay(), d, black.pixel() );
	XClearWindow( qt_xdisplay(), d );
}

// initialise the polygon
void kPolygonSaver::initialisePolygons()
{
	int i;

	polygons.append( new Polygon( numVertices ) );

	Polygon *poly = polygons.last();

	for ( i = 0; i < numVertices; i++ )
	{
		poly->vertices[i].x = rand() % width;
		poly->vertices[i].y = rand() % height;
		directions[i].x = 16 - (rand() % 8) * 4;
		if ( directions[i].x == 0 )
			directions[i].x = 1;
		directions[i].y = 16 - (rand() % 8) * 4;
		if ( directions[i].y == 0 )
			directions[i].y = 1;
	}

	poly->vertices[i].x = poly->vertices[0].x;
	poly->vertices[i].y = poly->vertices[0].y;
}

// move polygon in current direction and change direction if a border is hit
void kPolygonSaver::moveVertices()
{
	int i;
	Polygon *poly = polygons.last();

	for ( i = 0; i < numVertices; i++ )
	{
		poly->vertices[i].x += directions[i].x;
		if ( poly->vertices[i].x >= (int)width )
		{
			directions[i].x = -(rand() % 4 + 1) * 4;
			poly->vertices[i].x = (int)width;
		}
		else if ( poly->vertices[i].x < 0 )
		{
			directions[i].x = (rand() % 4 + 1) * 4;
			poly->vertices[i].x = 0;
		}

		poly->vertices[i].y += directions[i].y;
		if ( poly->vertices[i].y >= (int)height )
		{
			directions[i].y = -(rand() % 4 + 1) * 4;
			poly->vertices[i].y = height;
		}
		else if ( poly->vertices[i].y < 0 )
		{
			directions[i].y = (rand() % 4 + 1) * 4;
			poly->vertices[i].y = 0;
		}
	}

	poly->vertices[i].x = poly->vertices[0].x;
	poly->vertices[i].y = poly->vertices[0].y;
}

// create a color table of 64 colors
void kPolygonSaver::initialiseColor()
{
	QColor color;

	for ( int i = 0; i < 64; i++ )
	{
		color.setHsv( i * 360 / 64, 255, 255 );
		colors[i] = color.alloc();
	}
}

// set foreground color to next in the table
void kPolygonSaver::nextColor()
{
	static int col = 0;

	XSetForeground( qt_xdisplay(), gc, colors[col] );

	col++;

	if ( col > 63 )
		col = 0;
}

