//
// KFM  Options
//
// (c) Martin R. Jones 1996
// (c) Bernd Wuebben 1998

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
#include "htmlopts.h"
#include "htmlopts.moc"
#include <klocale.h>
#include "config-kfm.h"

#include "root.h" // for the gridsize definitions

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


void KFontOptions::getFontOpts(struct fontoptions& fntopts){

  fntopts.fontsize     = fSize;
  fntopts.standardfont = stdName;
  fntopts.standardfont.detach();
  fntopts.fixedfont    = fixedName;
  fntopts.fixedfont.detach();

  if(fontopts.fontsize != fSize || fontopts.standardfont != stdName ||
     fontopts.fixedfont != fixedName){

     fntopts.changed     = true;
  }
  else
    fntopts.changed      = false;

}




void KFontOptions::readOptions()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "KFM HTML Defaults" );		
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
	fontopts.fontsize = fSize;

	stdName = config->readEntry( "StandardFont" );
	if ( stdName.isEmpty() )
		stdName = DEFAULT_VIEW_FONT;
	fontopts.standardfont = stdName;
	fontopts.standardfont.detach();

	fixedName = config->readEntry( "FixedFont" );
	if ( fixedName.isEmpty() )
		fixedName = DEFAULT_VIEW_FIXED_FONT;

	fontopts.fixedfont = fixedName;
	fontopts.fixedfont.detach();

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
	label->setGeometry( 35, 20, 165, 25 );

	colorBtn = new KColorButton( bgColor, this );
	colorBtn->setGeometry( 200, 20, 80, 30 );
	connect( colorBtn, SIGNAL( changed( const QColor & ) ),
		SLOT( slotBgColorChanged( const QColor & ) ) );

	label = new QLabel( klocale->translate("Normal Text Color:"), this );
	label->setGeometry( 35, 60, 165, 25 );

	colorBtn = new KColorButton( textColor, this );
	colorBtn->setGeometry( 200, 60, 80, 30 );
	connect( colorBtn, SIGNAL( changed( const QColor & ) ),
		SLOT( slotTextColorChanged( const QColor & ) ) );

	label = new QLabel( klocale->translate("URL Link Color:"), this );
	label->setGeometry( 35, 100, 165, 25 );

	colorBtn = new KColorButton( linkColor, this );
	colorBtn->setGeometry( 200, 100, 80, 30 );
	connect( colorBtn, SIGNAL( changed( const QColor & ) ),
		SLOT( slotLinkColorChanged( const QColor & ) ) );

	label = new QLabel( klocale->translate("Followed Link Color:"), this );
	label->setGeometry( 35, 140, 165, 25 );

	colorBtn = new KColorButton( vLinkColor, this );
	colorBtn->setGeometry( 200, 140, 80, 30 );
	connect( colorBtn, SIGNAL( changed( const QColor & ) ),
		SLOT( slotVLinkColorChanged( const QColor & ) ) );

	cursorbox = new QCheckBox(klocale->translate("Change cursor over link."),
				  this);

	cursorbox->setGeometry(35,180,250,28);
	cursorbox->setChecked(changeCursor);
}

void KColorOptions::readOptions()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "KFM HTML Defaults" );	
	bgColor = config->readColorEntry( "BgColor", &HTML_DEFAULT_BG_COLOR );
	textColor = config->readColorEntry( "TextColor", &HTML_DEFAULT_TXT_COLOR );
	linkColor = config->readColorEntry( "LinkColor", &HTML_DEFAULT_LNK_COLOR );
	vLinkColor = config->readColorEntry( "VLinkColor", &HTML_DEFAULT_VLNK_COLOR);
	changeCursor = (bool) config->readNumEntry("ChangeCursor",0);
	changed = false;
}


void KColorOptions::slotBgColorChanged( const QColor &col )
{
	if ( bgColor != col )
	  bgColor = col;
	changed = true;

}

void KColorOptions::slotTextColorChanged( const QColor &col )
{
	if ( textColor != col )
	  textColor = col;
	changed = true;
}

void KColorOptions::slotLinkColorChanged( const QColor &col )
{
	if ( linkColor != col )
	  linkColor = col;
	changed = true;
}

void KColorOptions::slotVLinkColorChanged( const QColor &col )
{
	if ( vLinkColor != col )
	  vLinkColor = col;
	changed = true;
}

void KColorOptions::getColorOpts(struct coloroptions& coloropts){

  coloropts.bg      = bgColor;
  coloropts.text    = textColor;
  coloropts.link    = linkColor;
  coloropts.vlink   = vLinkColor;

  if(changeCursor != cursorbox->isChecked())
    changed = true;

  coloropts.changeCursoroverLink = cursorbox->isChecked();
  coloropts.changed = changed;
}

/************************************************************************/


KMiscOptions::KMiscOptions( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
  	readOptions();


	QLabel *label;

	label = new QLabel( klocale->translate(
			    "Horizontal Root Grid Spacing:"), this );

	label->setGeometry( 35, 20, 180, 25 );

	hspin  = new KNumericSpinBox(this);
	hspin ->setGeometry(225, 20, 40, 25 );
	hspin->setRange(0,DEFAULT_GRID_MAX - DEFAULT_GRID_MIN);

	
	if(gridwidth - DEFAULT_GRID_MIN < 0 )
	  gridwidth = DEFAULT_GRID_MIN;
	hspin->setValue(gridwidth - DEFAULT_GRID_MIN);

	label = new QLabel( klocale->translate(
			    "Vertical Root Grid Spacing:"), this );

	label->setGeometry( 35, 60, 180, 25 );

	vspin  = new KNumericSpinBox(this);
	vspin ->setGeometry(225, 60, 40, 25 );

	vspin->setRange(0,DEFAULT_GRID_MAX - DEFAULT_GRID_MIN);

	if(gridheight - DEFAULT_GRID_MIN < 0 )
	  gridheight = DEFAULT_GRID_MIN;

	vspin->setValue(gridheight - DEFAULT_GRID_MIN);

	iconstylebox = new QCheckBox(klocale->translate("Transparent Text for Root Icons."),
				  this);
	iconstylebox->setGeometry(35, 100, 280, 25);
	iconstylebox->setChecked(transparent);
}

void KMiscOptions::readOptions()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "KFM Misc Defaults" );	
	gridwidth = config->readNumEntry( "GridWidth", DEFAULT_GRID_WIDTH );
	gridheight = config->readNumEntry( "GridHeight", DEFAULT_GRID_HEIGHT );
	config->setGroup( "KFM Root Icons" );	
	transparent = (bool)config->readNumEntry("Style", DEFAULT_ROOT_ICONS_STYLE);

	changed = false;
}



void KMiscOptions::getMiscOpts(struct rootoptions& rootopts)
{
  if(gridwidth != (hspin->getValue()  + DEFAULT_GRID_MIN) || 
     gridheight != (vspin->getValue() + DEFAULT_GRID_MIN) )
  {
    changed = true;
  }
  if(transparent != iconstylebox->isChecked())
  {
    changed = true;
  }

  rootopts.gridwidth = hspin->getValue()+DEFAULT_GRID_MIN;
  rootopts.gridheight = vspin->getValue()+DEFAULT_GRID_MIN;

  if (iconstylebox->isChecked())
    rootopts.iconstyle = 1;
  else
    rootopts.iconstyle = 0;

  rootopts.changed = changed;
}
