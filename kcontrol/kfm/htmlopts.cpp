//
// KFM  Options
//
// (c) Martin R. Jones 1996
// (c) Bernd Wuebben 1998
// KControl port & modifications
// (c) Torben Weis 1998

#include <qbttngrp.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <kapp.h>
#include <kconfig.h>
#include <X11/Xlib.h>
#include <qcolor.h>

#include <klocale.h>

#include "htmlopts.h"

// browser/tree window color defaults -- Bernd
#define HTML_DEFAULT_BG_COLOR white
#define HTML_DEFAULT_LNK_COLOR blue
#define HTML_DEFAULT_TXT_COLOR black
#define HTML_DEFAULT_VLNK_COLOR magenta

// lets be modern .. -- Bernd
#define DEFAULT_VIEW_FONT "helvetica"
#define DEFAULT_VIEW_FIXED_FONT "courier"

//-----------------------------------------------------------------------------

KFontOptions::KFontOptions( QWidget *parent, const char *name )
	: KConfigWidget( parent, name )
{
  QLabel *label;

  QButtonGroup *bg = new QButtonGroup( klocale->translate("Font Size"), this );
  bg->setExclusive( TRUE );
  bg->setGeometry( 15, 15, 300, 50 );
  
  m_pSmall = new QRadioButton( klocale->translate("Small"), bg );
  m_pSmall->setGeometry( 10, 20, 80, 20 );
  
  m_pMedium = new QRadioButton( klocale->translate("Medium"), bg );
  m_pMedium->setGeometry( 100, 20, 80, 20 );

  m_pLarge = new QRadioButton( klocale->translate("Large"), bg );
  m_pLarge->setGeometry( 200, 20, 80, 20 );
  
  label = new QLabel( klocale->translate("Standard Font"), this );
  label->setGeometry( 15, 90, 100, 20 );
  
  m_pStandard = new QComboBox( false, this );
  m_pStandard->setGeometry( 120, 90, 180, 25 );
  getFontList( standardFonts, "-*-*-*-*-*-*-*-*-*-*-p-*-*-*" );
  m_pStandard->insertStrList( &standardFonts );
  connect( m_pStandard, SIGNAL( activated( const char * ) ),
	   SLOT( slotStandardFont( const char * ) ) );
  
  label = new QLabel( klocale->translate( "Fixed Font"), this );
  label->setGeometry( 15, 130, 100, 20 );
  
  m_pFixed = new QComboBox( false, this );
  m_pFixed->setGeometry( 120, 130, 180, 25 );
  getFontList( fixedFonts, "-*-*-*-*-*-*-*-*-*-*-m-*-*-*" );
  m_pFixed->insertStrList( &fixedFonts );
  
  connect( m_pFixed, SIGNAL( activated( const char * ) ),
	   SLOT( slotFixedFont( const char * ) ) );
  
  connect( bg, SIGNAL( clicked( int ) ), SLOT( slotFontSize( int ) ) );

  loadSettings();
  
  setMinimumSize( 330, 180 );
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

void KFontOptions::loadSettings()
{
  g_pConfig->setGroup( "KFM HTML Defaults" );		
  QString fs = g_pConfig->readEntry( "BaseFontSize" );
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
  
  stdName = g_pConfig->readEntry( "StandardFont" );
  if ( stdName.isEmpty() )
    stdName = DEFAULT_VIEW_FONT;
  
  fixedName = g_pConfig->readEntry( "FixedFont" );
  if ( fixedName.isEmpty() )
    fixedName = DEFAULT_VIEW_FIXED_FONT;  

  QStrListIterator sit( standardFonts );
  int i;
  for ( i = 0; sit.current(); ++sit, i++ )
  {
    if ( !strcmp( stdName, sit.current() ) )
      m_pStandard->setCurrentItem( i );
  }

  QStrListIterator fit( fixedFonts );
  for ( i = 0; fit.current(); ++fit, i++ )
  {
    if ( !strcmp( fixedName, fit.current() ) )
      m_pFixed->setCurrentItem( i );
  }

  m_pSmall->setChecked( fSize == 3 );
  m_pMedium->setChecked( fSize == 4 );
  m_pLarge->setChecked( fSize == 5 );
}

void KFontOptions::saveSettings()
{
  g_pConfig->setGroup( "KFM HTML Defaults" );			
  g_pConfig->writeEntry( "BaseFontSize", fSize );
  g_pConfig->writeEntry( "StandardFont", stdName );
  g_pConfig->writeEntry( "FixedFont", fixedName );
  g_pConfig->sync();
}

void KFontOptions::defaultSettings()
{
  g_pConfig->setGroup( "KFM HTML Defaults" );			
  g_pConfig->writeEntry( "BaseFontSize", 4 );
  g_pConfig->writeEntry( "StandardFont", DEFAULT_VIEW_FONT );
  g_pConfig->writeEntry( "FixedFont", DEFAULT_VIEW_FIXED_FONT );
  g_pConfig->sync();

  loadSettings();
}

void KFontOptions::applySettings()
{
  saveSettings();
  // HACK
}

//-----------------------------------------------------------------------------

KColorOptions::KColorOptions( QWidget *parent, const char *name )
    : KConfigWidget( parent, name )
{
  QLabel *label;

  label = new QLabel( klocale->translate("Background Color:"), this );
  label->setGeometry( 35, 20, 165, 25 );

  m_pBg = new KColorButton( bgColor, this );
  m_pBg->setGeometry( 200, 20, 80, 30 );
  connect( m_pBg, SIGNAL( changed( const QColor & ) ),
	   SLOT( slotBgColorChanged( const QColor & ) ) );

  label = new QLabel( klocale->translate("Normal Text Color:"), this );
  label->setGeometry( 35, 60, 165, 25 );
  
  m_pText = new KColorButton( textColor, this );
  m_pText->setGeometry( 200, 60, 80, 30 );
  connect( m_pText, SIGNAL( changed( const QColor & ) ),
	   SLOT( slotTextColorChanged( const QColor & ) ) );

  label = new QLabel( klocale->translate("URL Link Color:"), this );
  label->setGeometry( 35, 100, 165, 25 );

  m_pLink = new KColorButton( linkColor, this );
  m_pLink->setGeometry( 200, 100, 80, 30 );
  connect( m_pLink, SIGNAL( changed( const QColor & ) ),
	   SLOT( slotLinkColorChanged( const QColor & ) ) );

  label = new QLabel( klocale->translate("Followed Link Color:"), this );
  label->setGeometry( 35, 140, 165, 25 );

  m_pVLink = new KColorButton( vLinkColor, this );
  m_pVLink->setGeometry( 200, 140, 80, 30 );
  connect( m_pVLink, SIGNAL( changed( const QColor & ) ),
	   SLOT( slotVLinkColorChanged( const QColor & ) ) );

  cursorbox = new QCheckBox(klocale->translate("Change cursor over link."),
				  this);
  cursorbox->setGeometry(35,180,250,28);
  cursorbox->setChecked(changeCursor);

  loadSettings();

  setMinimumSize( 300, 180 );
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

void KColorOptions::loadSettings()
{
  g_pConfig->setGroup( "KFM HTML Defaults" );	
  bgColor = g_pConfig->readColorEntry( "BgColor", &HTML_DEFAULT_BG_COLOR );
  textColor = g_pConfig->readColorEntry( "TextColor", &HTML_DEFAULT_TXT_COLOR );
  linkColor = g_pConfig->readColorEntry( "LinkColor", &HTML_DEFAULT_LNK_COLOR );
  vLinkColor = g_pConfig->readColorEntry( "VLinkColor", &HTML_DEFAULT_VLNK_COLOR);
  changeCursor = (bool) g_pConfig->readNumEntry("ChangeCursor",0);
  changed = false;

  m_pBg->setColor( bgColor );
  m_pText->setColor( textColor );
  m_pLink->setColor( linkColor );
  m_pVLink->setColor( vLinkColor );
  cursorbox->setChecked( changeCursor );
}

void KColorOptions::defaultSettings()
{
  g_pConfig->setGroup( "KFM HTML Defaults" );			
  g_pConfig->writeEntry( "BgColor", HTML_DEFAULT_BG_COLOR );
  g_pConfig->writeEntry( "TextColor", HTML_DEFAULT_TXT_COLOR );
  g_pConfig->writeEntry( "LinkColor", HTML_DEFAULT_LNK_COLOR );
  g_pConfig->writeEntry( "VLinkColor", HTML_DEFAULT_VLNK_COLOR) ;
  g_pConfig->writeEntry( "ChangeCursor", 1 );
  g_pConfig->sync();

  loadSettings();
}

void KColorOptions::saveSettings()
{
  g_pConfig->setGroup( "KFM HTML Defaults" );			
  g_pConfig->writeEntry( "BgColor", bgColor );
  g_pConfig->writeEntry( "TextColor", textColor );
  g_pConfig->writeEntry( "LinkColor", linkColor);
  g_pConfig->writeEntry( "VLinkColor", vLinkColor );
  g_pConfig->writeEntry( "ChangeCursor", changeCursor );
  g_pConfig->sync();
}

void KColorOptions::applySettings()
{
  saveSettings();
  // HACK
}

#include "htmlopts.moc"
