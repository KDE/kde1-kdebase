
//
// KDE Display background setup module
//
// Copyright (c)  Martin R. Jones 1996
//
// Gradient fill added by Mark Donohoe 1997
//
// Converted to a kcc module by Matthias Hoelzer 1997
//

#include <qgrpbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qfiledlg.h>
#include <qradiobt.h>
#include <qpainter.h>
#include <qlayout.h>
#include <kapp.h>
#include <kwm.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "backgnd.h"
#include "dither.h"

#include "backgnd.moc"

#include <kiconloader.h>
#include <kwm.h>
#include <ksimpleconfig.h>

#ifdef HAVE_LIBGIF
#include "gif.h"
#endif

#ifdef HAVE_LIBJPEG
#include "jpeg.h"
#endif

#define NO_WALLPAPER	klocale->translate("(none)")

//----------------------------------------------------------------------------

class GPixmap : public QPixmap
{
public:
    GPixmap();
    void gradientFill(QColor color1, QColor color2, bool updown = TRUE, 
    		int num_colors = 8);
private:
	QPixmap cropped_pm;
    QPixmap full_pm;
    int x_size;
    int y_size;
    QColor col;
	int h, s;
	int red1, green1, blue1;
	int red_dif, green_dif, blue_dif;
	float rat;
	uint *p;
	uint rgbcol;
	int steps;
};

GPixmap::GPixmap()
{}

void GPixmap::gradientFill(QColor color1, QColor color2, bool updown, int num_colors)
{    
    if(updown)
    	y_size=height();
    else
    	y_size=width();
        
    cropped_pm.resize( 30, y_size );
 
 	QImage image(30, y_size, 32);
	

	red1 = color1.red();
	green1 = color1.green();
	blue1 = color1.blue();
	
	red_dif = color2.red()-color1.red();
	green_dif = color2.green()-color1.green();
	blue_dif = color2.blue()-color1.blue();

	for ( s = y_size-1; s > 0; s-- )
	{
		p = (uint *) image.scanLine( y_size - s - 1 );
		rat=1.0*s/y_size;
		col.setRgb( red1+(int)(red_dif*rat),
						green1+(int)(green_dif*rat), blue1+(int)(blue_dif*rat) );
		rgbcol= col.rgb();
		
		for( h = 0; h < 30; h++ ) {
			*p = rgbcol;
			p++;
		}
	}
	
	if(num_colors<2 || num_colors >256) {
		num_colors=8;
	}
	
	QColor *ditherPalette = new QColor[num_colors];
	for( s=0; s<num_colors; s++) {
		ditherPalette[s].setRgb(red1+red_dif*s/(num_colors-1),
		green1+green_dif*s/(num_colors-1),
		blue1+blue_dif*s/(num_colors-1) );
	}
	
	kFSDither dither( ditherPalette, num_colors );
	QImage tImage = dither.dither( image );
	cropped_pm.convertFromImage( tImage );
	
	if(updown)
		steps=width()/20+1;
	else
		steps=height()/20+1;
	
	QPainter p;
	p.begin( this );
	if(updown)	
		for(s=0;s<steps;s++)
			p.drawPixmap(20*s, 0,cropped_pm, 5, 0 , 20, y_size );
	else {
		QWMatrix matrix;
        matrix.translate( (float)width()-1.0, 0.0);
       	matrix.rotate( 90.0 );
       	p.setWorldMatrix( matrix );
       	for(s=0;s<steps;s++)
			p.drawPixmap(20*s, 0,cropped_pm, 5, 0 , 20, y_size );
    }
    p.end();

	delete [] ditherPalette;
}	 

//----------------------------------------------------------------------------

KRenameDeskDlg::KRenameDeskDlg( const char *t, QWidget *parent )
    : QDialog( parent, 0, true )
{
    QPushButton *ok, *cancel;
    QVBoxLayout *vlayout = new QVBoxLayout( this, 20 );

    edit = new QLineEdit( this );
    edit->setText( t );
    edit->setFixedHeight( edit->sizeHint().height() );
    edit->setFocus();
    vlayout->addWidget( edit );

    QHBoxLayout *hlayout = new QHBoxLayout;
    vlayout->addLayout( hlayout );

    hlayout->addStretch();

    ok = new QPushButton( klocale->translate( "Ok" ), this );
    ok->setDefault( true );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );
    hlayout->addWidget( ok );

    cancel = new QPushButton( klocale->translate( "Cancel" ), this );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
    hlayout->addWidget( cancel );

    QSize btnSize = ok->sizeHint();
    if ( btnSize.width() < cancel->sizeHint().width() )
    btnSize = cancel->sizeHint();

    ok->setFixedSize( btnSize );
    cancel->setFixedSize( btnSize );

    vlayout->freeze();
}

//----------------------------------------------------------------------------

KBackground::KBackground( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{
#ifdef HAVE_LIBGIF
    QImageIO::defineIOHandler("GIF", "^GIF[0-9][0-9][a-z]", 
			  0, read_gif_file, NULL);
#endif
#ifdef HAVE_LIBJPEG
    QImageIO::defineIOHandler("JFIF","^\377\330\377\340", 
			  0, read_jpeg_jfif, NULL);
#endif

    KIconLoader iconLoader;

    changed = FALSE;
    maxDesks = 8;
    deskNum = desktop;

    readSettings( deskNum );

    setName( klocale->translate("Desktop") );

    // if we are just initialising we don't need to create setup widget
    if ( mode == Init )
	    return;

    if ( KWM::isKWMInitialized() )
	maxDesks = KWM::numberOfDesktops();
    else
	maxDesks = 1;

    QLabel *label;
    QPushButton *button;
    QGroupBox *group;
    QRadioButton *rb;

    QPixmap p = iconLoader.loadIcon("monitor.xpm");

    label = new QLabel( this );
    label->setPixmap( p );
    label->setMinimumSize( label->sizeHint() );
    label->move( 250, 15 );

    monitor = new KBGMonitor( label );
    monitor->setGeometry( 20, 10, 157, 111 );
    monitor->setBackgroundColor( color1 );

    group = new QGroupBox( klocale->translate("Desktop"), this );
    group->setGeometry( 15, 15, 210, 155 );

    deskListBox = new QListBox( group );
    deskListBox->setGeometry( 20, 25, 120, 75 );
    getDeskNameList();
    connect( deskListBox, SIGNAL( highlighted(int) ),
	SLOT( slotSwitchDesk(int) ) );

    button = new QPushButton( klocale->translate( "Rename..." ), group );
    button->setGeometry( 20, 115, 120, 25 );
    if ( !KWM::isKWMInitialized() )
	button->setEnabled( false );
    connect( button, SIGNAL( clicked() ), SLOT( slotRenameDesk() ) );

    group = new QGroupBox( klocale->translate("Color"), this );
    group->setGeometry( 15, 190, 210, 130 );

    colButton1 = new KColorButton( group );
    colButton1->setGeometry( 15, 25, 75, 30 );
    connect( colButton1, SIGNAL( changed( const QColor & ) ),
	    SLOT( slotSelectColor1( const QColor & ) ) );

    colButton2 = new KColorButton( group );
    colButton2->setGeometry( 15, 80, 75, 30 );
    connect( colButton2, SIGNAL( changed( const QColor & ) ),
	    SLOT( slotSelectColor2( const QColor & ) ) );


    gfGroup = new QButtonGroup( this );
    gfGroup->hide();
    gfGroup->setExclusive( TRUE );

    rb = new QRadioButton( klocale->translate("Flat"), group );
    rb->setGeometry( 100, 20, 100, 20 );
    gfGroup->insert( rb, Flat );

    rb = new QRadioButton( klocale->translate("Gradient"), group );
    rb->setGeometry( 100, 40, 100, 20 );
    gfGroup->insert( rb, Gradient );

    connect( gfGroup, SIGNAL( clicked( int ) ), SLOT( slotGradientMode( int ) ) );

    orGroup = new QButtonGroup( this );
    orGroup->hide();
    orGroup->setExclusive( TRUE );

    rbPortrait = new QRadioButton( klocale->translate("Portrait"), group );
    rbPortrait->setGeometry( 100, 75, 100, 20 );
    orGroup->insert( rbPortrait, Portrait );

    rbLandscape = new QRadioButton( klocale->translate("Landscape"), group );
    rbLandscape->setGeometry( 100, 95, 100, 20 );
    orGroup->insert( rbLandscape, Landscape );

    connect( orGroup, SIGNAL( clicked( int ) ), SLOT( slotOrientMode( int ) ) );


    group = new QGroupBox( klocale->translate("Wallpaper"), this );
    group->setGeometry( 240, 190, 215, 130 );

    QString path = kapp->kdedir();
    path += "/share/wallpapers";
    QDir d( path, "*", QDir::Name, QDir::Readable | QDir::Files );
    const QStrList *list = d.entryList();

    wpCombo = new QComboBox( group );
    wpCombo->setGeometry( 15, 20, 190, 25 );
    wpCombo->insertItem( NO_WALLPAPER, 0 );
    wpCombo->setCurrentItem( 0 );

    QStrListIterator it( *list );
    for ( int i = 1; it.current(); ++it, i++ )
    {
	wpCombo->insertItem( it.current() );
	if ( wallpaper == it.current() )
	    wpCombo->setCurrentItem( i );
    }

    if ( wallpaper != NO_WALLPAPER && wpCombo->currentItem() == 0 )
    {
	wpCombo->insertItem( wallpaper );
	wpCombo->setCurrentItem( wpCombo->count()-1 );
    }
    connect( wpCombo, SIGNAL( activated( const char * ) ),
		SLOT( slotWallpaper( const char * )  )  );

    wpGroup = new QButtonGroup( this );
    wpGroup->hide();
    wpGroup->setExclusive( TRUE );

    rb = new QRadioButton( klocale->translate("Tiled"), group );
    rb->setGeometry( 20, 50, 85, 25 );
    wpGroup->insert( rb, Tiled );

    rb = new QRadioButton( klocale->translate("Centred"), group );
    rb->setGeometry( 20, 75, 85, 25 );
    wpGroup->insert( rb, Centred );

    rb = new QRadioButton( klocale->translate("Scaled"), group );
    rb->setGeometry( 20, 100, 85, 25 );
    wpGroup->insert( rb, Scaled );

    connect( wpGroup, SIGNAL( clicked( int ) ), SLOT( slotWallpaperMode( int ) ) );

    button = new QPushButton( klocale->translate("Browse..."), group );
    button->setGeometry( 125, 55, 80, 25 );
    connect( button, SIGNAL( clicked() ), SLOT( slotBrowse() ) );

    button = new QPushButton( klocale->translate("Help"), group );
    button->setGeometry( 125, 90, 80, 25 );
    connect( button, SIGNAL( clicked() ), SLOT( slotHelp() ) );
    
    showSettings();
}

void KBackground::readSettings( int num )
{
    char group[80];
    sprintf( group, "Desktop%d", num + 1 );

	KConfig *config = kapp->getConfig();
	config->setGroup( group );

	QString str;

	str = config->readEntry( "Color1" );
	if ( !str.isNull() )
		color1.setNamedColor( str );
	else
		color1 = gray;
		
	str = config->readEntry( "Color2" );
	if ( !str.isNull() )
		color2.setNamedColor( str );
	else
		color2 = gray;

	gfMode=Flat;
	str = config->readEntry( "ColorMode" );
	if ( !str.isNull() )
	{
		if ( str == "Gradient" )
			gfMode = Gradient;
	}
	
	orMode=Portrait;
	str = config->readEntry( "OrientationMode" );
	if ( !str.isNull() )
	{
		if ( str == "Landscape" )
			orMode = Landscape;
	}

	wpMode = Tiled;
	str = config->readEntry( "WallpaperMode" );
	if ( !str.isNull() )
	{
		if ( str == "Centred" )
			wpMode = Centred;
		else if ( str == "Scaled" )
			wpMode = Scaled;
	}

	wallpaper = NO_WALLPAPER;
	str = config->readEntry( "Wallpaper" );
	if ( !str.isNull() )
	{
		wallpaper = str;
		loadWallpaper( str );
	}
}

void KBackground::writeSettings( int num )
{
	if ( !changed )
		return;

    char group[80];
    sprintf( group, "Desktop%d", num + 1 );

	KConfig *config = kapp->getConfig();
	config->setGroup( group );

	QString col1Name(10);
	col1Name.sprintf("#%02x%02x%02x", color1.red(), color1.green(), color1.blue());
	config->writeEntry( "Color1", col1Name );
	
	QString col2Name(10);
	col2Name.sprintf("#%02x%02x%02x", color2.red(), color2.green(), color2.blue());
	config->writeEntry( "Color2", col2Name );

	config->writeEntry( "Wallpaper", wallpaper );

	switch ( wpMode )
	{
		case Tiled:
			config->writeEntry( "WallpaperMode", "Tiled" );
			break;
		case Centred:
			config->writeEntry( "WallpaperMode", "Centred" );
			break;
		case Scaled:
			config->writeEntry( "WallpaperMode", "Scaled" );
			break;
	}
	
	switch ( gfMode )
	{
		case Flat:
			config->writeEntry( "ColorMode", "Flat" );
			break;
		case Gradient:
			config->writeEntry( "ColorMode", "Gradient" );
			break;
	}
	
	switch ( orMode )
	{
		case Portrait:
			config->writeEntry( "OrientationMode", "Portrait" );
			break;
		case Landscape:
			config->writeEntry( "OrientationMode", "Landscape" );
			break;
	}

	changed = FALSE;

	config->sync();
}

void KBackground::getDeskNameList()
{
    int i;
    int current = deskListBox->currentItem();

    if ( current < 0 )
	current = 0;

    deskListBox->setUpdatesEnabled( false );
    deskListBox->clear();

    if ( KWM::isKWMInitialized() )
    {
	for ( i = 0; i < maxDesks; i++ )
	    deskListBox->insertItem( KWM::getDesktopName(i+1) );
    }
    else
       deskListBox->insertItem( klocale->translate( "Default" ) );
    
    deskListBox->setCurrentItem( current );
    deskListBox->setUpdatesEnabled( true );
}

void KBackground::setDesktop( int desk )
{
    if ( deskNum == desk )
	return;

    writeSettings( deskNum );

    deskNum = desk;

    if ( deskNum <= 0 )
	deskNum = 0;

    if ( deskNum >= maxDesks - 1 )
	deskNum = maxDesks - 1;

    readSettings( deskNum );
    showSettings();
    debug( "Change to desk: %d", deskNum );
}

void KBackground::showSettings()
{ 
    colButton1->setColor( color1 );
    colButton2->setColor( color2 );
    ((QRadioButton *)gfGroup->find( Flat ))->setChecked( gfMode == Flat );
    ((QRadioButton *)gfGroup->find( Gradient ))->setChecked(gfMode == Gradient);
    if( gfMode == Flat ) {
	rbLandscape->hide();
	rbPortrait->hide();
	colButton2->hide();
    } else {
	rbLandscape->show();
	rbPortrait->show();
	colButton2->show();
    }
    ((QRadioButton *)orGroup->find( Portrait ))->setChecked( orMode==Portrait );
    ((QRadioButton *)orGroup->find( Landscape ))->setChecked(orMode==Landscape);

    wpCombo->setCurrentItem( 0 );
    for ( int i = 1; i < wpCombo->count(); i++ )
    {
	if ( wallpaper == wpCombo->text( i ) )
	{
	    wpCombo->setCurrentItem( i );
	    break;
	}
    }

    if ( wallpaper != NO_WALLPAPER && wpCombo->currentItem() == 0 )
    {
	wpCombo->insertItem( wallpaper );
	wpCombo->setCurrentItem( wpCombo->count()-1 );
    }

    ((QRadioButton *)wpGroup->find( Tiled ))->setChecked( wpMode == Tiled );
    ((QRadioButton *)wpGroup->find( Centred ))->setChecked( wpMode == Centred );
    ((QRadioButton *)wpGroup->find( Scaled ))->setChecked( wpMode == Scaled );

    setMonitor();
}

void KBackground::slotApply()
{
	writeSettings( deskNum );
	KApplication::getKApplication()->getConfig()->sync();
	apply( true );
}

void KBackground::apply( bool force )
{
	if ( !changed && !force )
		return;

	// tell background module to reload settings
	QString cmd = "kbgwm_reconfigure";
	KWM::sendKWMCommand( cmd );

/*
	if ( wallpaper != NO_WALLPAPER && gfMode == Flat)
	{
		// reload wallpaper without color context
		loadWallpaper( wallpaper, FALSE );
		QApplication::desktop()->setBackgroundPixmap( wpPixmap );
		retainResources();
		
		return;
	} else if ( gfMode == Gradient ) {
		QApplication::setOverrideCursor( waitCursor );
		
		GPixmap pmDesktop;
		pmDesktop.resize(QApplication::desktop()->width(),
			QApplication::desktop()->height());

		if( orMode == Portrait )
			pmDesktop.gradientFill( color2, color1 );
		else
			pmDesktop.gradientFill( color2, color1, FALSE );
			
		qApp->desktop()->setBackgroundPixmap( pmDesktop );
		QApplication::restoreOverrideCursor();
		retainResources();
		
		return;
	} else {
		debug( "setting bgcolor only" );
		qApp->desktop()->setBackgroundColor( color1 );
		retainResources();
	}
*/
}

void KBackground::retainResources() {
	// Retain server resources after the client exits.
	Display *dpy;
	dpy = x11Display();
	XKillClient(dpy, AllTemporary);
	XSetCloseDownMode(dpy, RetainTemporary);
}

void KBackground::setMonitor()
{
	if ( !wallpaper.isNull() && wallpaper != NO_WALLPAPER )
	{
		float sx = (float)monitor->width() / QApplication::desktop()->width();
		float sy = (float)monitor->height() / QApplication::desktop()->height();

		QWMatrix matrix;
		matrix.scale( sx, sy );
		monitor->setBackgroundPixmap( wpPixmap.xForm( matrix ) );
	}
	else if ( gfMode == Gradient ) 
	{
		GPixmap preview;
		preview.resize( monitor->width()+1, monitor->height()+1 );

		if( orMode == Portrait ) 
			preview.gradientFill( color2, color1, TRUE, 3 );
		else
			preview.gradientFill( color2, color1, FALSE, 3 );

		monitor->setBackgroundPixmap( preview );	
	}
	else monitor->setBackgroundColor( color1 );
}

// Attempts to load the specified wallpaper and creates a centred/scaled
// version if necessary.
// Note that centred pixmaps are placed on a full screen image of background
// color1, so if you want to save memory use a small tiled pixmap.
//
int KBackground::loadWallpaper( const char *name, bool useContext )
{
	static int context = 0;
	QString filename;
	int rv = FALSE;
	QPixmap tmp;

	QApplication::setOverrideCursor( waitCursor );

	if ( useContext )
	{
		if ( context )
			QColor::destroyAllocContext( context );
		context = QColor::enterAllocContext();
	}

	if ( name[0] != '/' )
	{
		filename = kapp->kdedir();
		filename += "/share/wallpapers/";
		filename += name;
	}
	else
		filename = name;
	
	if ( tmp.load( filename ) == TRUE )
	{
		int w = QApplication::desktop()->width();
		int h = QApplication::desktop()->height();

		switch ( wpMode )
		{
			case Centred:
				{
					wpPixmap.resize( w, h );
					wpPixmap.fill( color1 );
					bitBlt( &wpPixmap, (w - tmp.width())/2,
							(h - tmp.height())/2, &tmp, 0, 0,
							tmp.width(), tmp.height() );
				}
				break;

			case Scaled:
				{
					float sx = (float)w / tmp.width();
					float sy = (float)h / tmp.height();
					QWMatrix matrix;
					matrix.scale( sx, sy );
					wpPixmap = tmp.xForm( matrix );
				}
				break;

			default:
				wpPixmap = tmp;
		}
		rv = TRUE;
	}

	if ( useContext )
		QColor::leaveAllocContext();

	QApplication::restoreOverrideCursor();

	return rv;
}

void KBackground::slotSelectColor1( const QColor &col )
{
	color1 = col;

	if ( gfMode == Gradient || wpMode == Centred || wallpaper == NO_WALLPAPER )
	{
		// force the background to be made with different background
		if ( wpMode == Centred )
		    loadWallpaper( wallpaper );
		setMonitor();
	}

	changed = TRUE;
}

void KBackground::slotSelectColor2( const QColor &col )
{
	color2 = col;

	if ( gfMode == Gradient || wpMode == Centred || wallpaper == NO_WALLPAPER )
	{
		setMonitor();
	}

	changed = TRUE;
}

void KBackground::slotBrowse()
{
	QString path;

	path = kapp->kdedir();
	path += "/share/wallpapers";

	QDir dir( path );
	if ( !dir.exists() )
		path = NULL;

	QString filename = QFileDialog::getOpenFileName( path );
	slotWallpaper( filename );
	if ( !filename.isNull() && !strcmp( filename, wallpaper) )
	{
		wpCombo->insertItem( wallpaper );
		wpCombo->setCurrentItem( wpCombo->count() - 1 );
	}
}

void KBackground::slotWallpaper( const char *filename )
{
	if ( filename )
	{
		if ( !strcmp( filename, NO_WALLPAPER ) )
		{
			wallpaper = filename;
			setMonitor();
		}
		else if ( loadWallpaper( filename ) == TRUE )
		{
			wallpaper = filename;
			setMonitor();
		}

		changed = TRUE;
	}
}

void KBackground::slotWallpaperMode( int m )
{
	wpMode = m;
	if ( loadWallpaper( wallpaper ) == TRUE )
	{
		setMonitor();
		changed = TRUE;
	}
}

void KBackground::slotGradientMode( int m )
{
	gfMode = m;
	
	if( gfMode == Flat ) {
		rbLandscape->hide();
		rbPortrait->hide();
		colButton2->hide();
	} else {
		rbLandscape->show();
		rbPortrait->show();
		colButton2->show();
	}
		
	setMonitor();
	changed = TRUE;
}

void KBackground::slotOrientMode( int m )
{
	orMode = m;
	setMonitor();
	changed = TRUE;
}

void KBackground::slotSwitchDesk( int num )
{
    setDesktop( num );
}

void KBackground::slotRenameDesk()
{
    KRenameDeskDlg dlg( KWM::getDesktopName( deskNum+1 ), this );

    dlg.setCaption( klocale->translate( "Desktop Title" ) );

    if ( dlg.exec() == QDialog::Accepted )
    {
	KWM::setDesktopName( deskNum+1, dlg.title() );
	getDeskNameList();
    }
}

void KBackground::slotHelp()
{
	kapp->invokeHTMLHelp( "kdisplay/kdisplay-3.html", "" );
}

void KBackground::loadSettings()
{
  readSettings(deskNum);
  showSettings();
}

void KBackground::applySettings()
{
  writeSettings(deskNum);
  apply(TRUE);
}
