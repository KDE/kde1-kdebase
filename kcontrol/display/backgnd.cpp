//
// KDE Display background setup module
//
// Copyright (c)  Martin R. Jones 1996
//
// Converted to a kcc module by Matthias Hoelzer 1997
// Gradient backgrounds by Mark Donohoe 1997
// Pattern backgrounds by Stephan Kulow 1998
// Randomizing & dnd & new display modes by Matej Koss 1998
//

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include <stdlib.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qdir.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#include <qlcdnumber.h>

#include <kfiledialog.h>
#include <kapp.h>
#include <kwm.h>
#include <kiconloader.h>
#include <kbuttonbox.h>
#include <kpixmap.h>
#include <kstring.h>
#include <kimgio.h>

#include <X11/Xlib.h>

#include "backgnd.h"
#include "backgnd.moc"
#include "../../kwmmodules/kbgndwm/config-kbgndwm.h"

KRenameDeskDlg::KRenameDeskDlg( const char *t, QWidget *parent )
  : QDialog( parent, 0, true )
{
  
  QVBoxLayout *vlayout = new QVBoxLayout( this, 5, 10 );
  
  vlayout->addSpacing(10);
    
  QLabel *label = new QLabel( this );
  label->setText( i18n( "Enter new desktop name" ) );
  label->setMinimumSize( label->sizeHint() );
  vlayout->addWidget( label );
    
  edit = new QLineEdit( this );
  edit->setText( t );
  edit->setFixedHeight( edit->sizeHint().height() );
  edit->setFocus();
  vlayout->addWidget( edit );
	
  vlayout->addStretch(10);
  vlayout->addSpacing(10);
    
  QFrame* tmpQFrame;
  tmpQFrame = new QFrame( this );
  tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  tmpQFrame->setMinimumHeight( tmpQFrame->sizeHint().height() );
    
  vlayout->addWidget( tmpQFrame );
    
  KButtonBox *bbox = new KButtonBox( this );
  bbox->addStretch( 10 );
    
  QPushButton *ok = bbox->addButton( i18n( "OK" ) );
  connect( ok, SIGNAL( clicked() ), SLOT( accept() ) );
    
  QPushButton *cancel = bbox->addButton( i18n( "Cancel" ) );
  connect( cancel, SIGNAL( clicked() ), SLOT( reject() ) );
    
  bbox->layout();
  vlayout->addWidget( bbox );
    
  vlayout->activate();
    
  resize(150,0);
}

//----------------------------------------------------------------------------

KBackground::KBackground( QWidget *parent, int mode, int desktop )
  : KDisplayModule( parent, mode, desktop )
{
  //debug("KBackground::KBackground");

  KIconLoader iconLoader;
      
  interactive = true;
  changed = false;
  maxDesks = 8;
  deskNum = 0;
  random = 0;
  currentItem.bUseWallpaper = false;
  rnddlg = 0L;

  kimgioRegister();

  setName( i18n("Desktop") );
      
  // if we are just initialising we don't need to create setup widget
  if ( mode == Init )
    {
      // readSettings( 0 );
      return;
    }

  if ( KWM::isKWMInitialized() )
    {
      maxDesks = KWM::numberOfDesktops();
      deskNum = KWM::currentDesktop() - 1;
    }
  else
    maxDesks = 1;


  KConfig *config = kapp->getConfig();
  config->setGroup( "Desktop Common" );
  oneDesktopMode = config->readBoolEntry( "OneDesktopMode",
                                         DEFAULT_ENABLE_COMMON_BGND );
  docking = config->readBoolEntry( "Docking", DEFAULT_ENABLE_DOCKING );

  if ( oneDesktopMode )
    deskNum = config->readNumEntry( "DeskNum", DEFAULT_DESKTOP );

  QGroupBox *group;
  QRadioButton *rb;
	
  QGridLayout *topLayout = new QGridLayout( this, 8, 6, 5 );

  //  topLayout->addRowSpacing(3,5);

  topLayout->addRowSpacing(4,1);
  topLayout->addRowSpacing(6,1);

  topLayout->setRowStretch(0,0);
  topLayout->setRowStretch(1,1);
  topLayout->setRowStretch(2,1);
  topLayout->setRowStretch(3,0);
  topLayout->setRowStretch(4,0);
  topLayout->setRowStretch(5,0);
  topLayout->setRowStretch(6,0);
  topLayout->setRowStretch(7,0);
    
  topLayout->setColStretch(0,0);
  topLayout->setColStretch(1,1);
  topLayout->setColStretch(2,0);
  topLayout->setColStretch(3,4);
  topLayout->setColStretch(4,0);
  topLayout->setColStretch(5,0);
    
  group = new QGroupBox( i18n("Desktop"), this );

  topLayout->addWidget( group, 1,1 );
	
  QBoxLayout *groupLayout = new QVBoxLayout( group, 5, 5 );

  deskListBox = new QListBox( group );

  getDeskNameList();
  deskListBox->setCurrentItem( deskNum );
  connect( deskListBox, SIGNAL( highlighted(int) ),
	   SLOT( slotSwitchDesk(int) ) );
  
  readSettings( deskNum );

  deskListBox->adjustSize();
  deskListBox->setMinimumSize(deskListBox->size());
  deskListBox->setEnabled( !oneDesktopMode );

  groupLayout->addSpacing( 15 );
  groupLayout->addWidget( deskListBox, 5 );
	
// suppress rename button (kcmpanel provides this, duncan@kde.org 19990822
//  renameButton = new QPushButton( i18n( "&Rename ..." ), group );
//  renameButton->adjustSize();
//  renameButton->setFixedHeight( renameButton->height() );
//  renameButton->setMinimumWidth( renameButton->width() );
//  renameButton->setEnabled( !oneDesktopMode );
//  if ( !KWM::isKWMInitialized() )
//    renameButton->setEnabled( false );
//  connect( renameButton, SIGNAL( clicked() ), SLOT( slotRenameDesk() ) );
	
//  groupLayout->addWidget( renameButton, 5 );

  oneDesktopButton = new QCheckBox( i18n("&Common Background"), group );
  oneDesktopButton->adjustSize();
  oneDesktopButton->setFixedHeight( oneDesktopButton->height() );
  oneDesktopButton->setMinimumWidth( oneDesktopButton->width() );
  oneDesktopButton->setChecked( oneDesktopMode );
  connect( oneDesktopButton, SIGNAL( clicked() ), SLOT( slotToggleOneDesktop() ) );

  groupLayout->addWidget( oneDesktopButton, 5 );
  groupLayout->activate();

  QPixmap p = iconLoader.loadIcon("monitor.xpm");

  monitorLabel = new QLabel( this );
  monitorLabel->setAlignment( AlignCenter );
  monitorLabel->setPixmap( p );
  monitorLabel->adjustSize();
  monitorLabel->setMinimumSize(monitorLabel->size());
	
  topLayout->addMultiCellWidget( monitorLabel, 1, 1, 2, 4 );

  monitorDrop = new KDNDDropZone(monitorLabel, DndURL);
  connect(monitorDrop, SIGNAL(dropAction(KDNDDropZone*)), 
	  SLOT(slotDropped(KDNDDropZone*)));

  monitor = new KBGMonitor( monitorLabel );
  monitor->resize( 157, 111 );
  monitor->setBackgroundColor( currentItem.color1 );
	
  group = new QGroupBox( i18n( "Colors" ), this );
  topLayout->addWidget( group, 2, 1 );

  QGridLayout *grid = new QGridLayout( group, 9, 4, 5, 5 );
    
  grid->setRowStretch(0,5);
  grid->setRowStretch(1,0);
  grid->setRowStretch(2,0);
  grid->setRowStretch(3,5);
  grid->setRowStretch(4,0);
  grid->setRowStretch(5,0);
  grid->setRowStretch(6,5);
  grid->setRowStretch(7,0);
  grid->setRowStretch(8,5);
    
  grid->setColStretch(0,0);
  grid->setColStretch(1,1);
  grid->setColStretch(2,9);
  grid->setColStretch(3,0);
    
  grid->addRowSpacing(0,5);
  grid->addRowSpacing(5,1);
    
  ncGroup = new QButtonGroup( this );
  ncGroup->hide();
  ncGroup->setExclusive( true );

  rb = new QRadioButton( i18n("&One Color"), group );
  rb->adjustSize();
  rb->setFixedHeight( rb->height() );
  rb->setMinimumWidth( rb->width() );
  ncGroup->insert( rb, OneColor );
	
  grid->addMultiCellWidget( rb, 1, 1, 1, 2 );

  colButton1 = new KColorButton( group );
  colButton1->adjustSize();
  colButton1->setFixedHeight(colButton1->height());
  colButton1->setMinimumWidth(colButton1->width());
  connect( colButton1, SIGNAL( changed( const QColor & ) ),
	   SLOT( slotSelectColor1( const QColor & ) ) );
		
  grid->addMultiCellWidget( colButton1, 2, 2, 2, 2 );

  rb = new QRadioButton( i18n("&Two Color"), group );
  rb->adjustSize();
  rb->setFixedHeight( rb->height() );
  rb->setMinimumWidth( rb->width() );
  ncGroup->insert( rb, TwoColor );
    
  grid->addMultiCellWidget( rb, 4, 4, 1, 2 );
    
  connect( ncGroup, SIGNAL( clicked( int ) ), SLOT( slotColorMode( int ) ) );
    
  colButton2 = new KColorButton( group );
  colButton2->adjustSize();
  colButton2->setFixedHeight(colButton2->height());
  colButton2->setMinimumWidth(colButton2->width());
  connect( colButton2, SIGNAL( changed( const QColor & ) ),
	   SLOT( slotSelectColor2( const QColor & ) ) );
			
  grid->addWidget( colButton2, 5, 2);

  changeButton = new QPushButton( i18n("Set&up ..."), group );
  changeButton->adjustSize();
  changeButton->setFixedHeight( changeButton->height() );
  changeButton->setMinimumWidth(changeButton->width());
  connect(changeButton, SIGNAL(clicked()) , SLOT(slotSetup2Color()) );
    
  grid->addWidget( changeButton, 7, 2 );
  grid->activate();
    
  group = new QGroupBox( i18n("Wallpaper"), this );
  topLayout->addMultiCellWidget( group, 2, 2, 2, 4 );

  //CT - 30Nov1998 - replaced with grip layout, "fixed" a little the logic of
  //   widgets placing    
  grid = new QGridLayout( group, 8, 3, 5, 5 );

  grid->addRowSpacing(0,10);
  grid->addRowSpacing(3,5);
  grid->addRowSpacing(5,10);
  grid->addRowSpacing(7,10);

  grid->setRowStretch(0,0);
  grid->setRowStretch(1,0);
  grid->setRowStretch(2,0);
  grid->setRowStretch(3,1);
  grid->setRowStretch(4,0);
  grid->setRowStretch(5,2);
  grid->setRowStretch(6,0);
  grid->setRowStretch(7,0);

  grid->setColStretch(0,0);
  grid->setColStretch(1,1);
  grid->setColStretch(2,2);
  //CT

  QString path = kapp->kde_wallpaperdir().copy();
  QDir d( path, "*", QDir::Name, QDir::Readable | QDir::Files );
  const QStrList *list = d.entryList();

  wpCombo = new QComboBox( false, group );
  wpCombo->insertItem( i18n("No wallpaper"), 0 );
  wpCombo->setCurrentItem( 0 );

  //CT 30Nov1998
  grid->addMultiCellWidget( wpCombo, 1,1,0,2 );
  //  groupLayout->addStretch( 3 );
    
  QStrListIterator it( *list );
  for ( int i = 1; it.current(); ++it, i++ )
    {
      wpCombo->insertItem( it.current() );
      if ( currentItem.wallpaper == it.current() )
	wpCombo->setCurrentItem( i );
    }

  if ( currentItem.bUseWallpaper && wpCombo->currentItem() == 0 )
    {
      wpCombo->insertItem( currentItem.wallpaper );
      wpCombo->setCurrentItem( wpCombo->count()-1 );
    }

  wpCombo->adjustSize();
  wpCombo->setFixedHeight(wpCombo->height());
  //CT  wpCombo->setMinimumWidth(wpCombo->width());

  connect( wpCombo, SIGNAL( activated( const char * ) ),
	   SLOT( slotWallpaper( const char * )  )  );
		
  /*CT 30Nov1998
  QBoxLayout *pushLayout = new QHBoxLayout( 5 ); 
  groupLayout->addLayout( pushLayout );
  */

  browseButton = new QPushButton( i18n("&Browse..."), group );
  browseButton->adjustSize();
  browseButton->setFixedHeight( browseButton->height() );
  browseButton->setMinimumWidth( browseButton->width() );
  connect( browseButton, SIGNAL( clicked() ), SLOT( slotBrowse() ) );
  grid->addWidget( browseButton,2,1 );//CT 30Nov1998

  //CT 30Nov1998
  QLabel *aLabel = new QLabel(i18n("Arrangement:"), group );
  aLabel->adjustSize();
  aLabel->setMinimumSize(aLabel->size());
  grid->addWidget(aLabel, 4, 0 );
  //CT

  wpModeCombo = new QComboBox( false, group );
  wpModeCombo->insertItem( i18n("Tiled"), 0 );
  wpModeCombo->insertItem( i18n("Mirrored"), 1 );
  wpModeCombo->insertItem( i18n("Center Tiled"), 2 );
  wpModeCombo->insertItem( i18n("Centred"), 3 );
  wpModeCombo->insertItem( i18n("Centred Brick"), 4 );
  wpModeCombo->insertItem( i18n("Centred Warp"), 5 );
  wpModeCombo->insertItem( i18n("Centred Maxpect"), 6 );
  wpModeCombo->insertItem( i18n("Symmetrical Tiled"), 7 );
  wpModeCombo->insertItem( i18n("Symmetrical Mirrored"), 8 );
  wpModeCombo->insertItem( i18n("Scaled"), 9 );
  wpModeCombo->setCurrentItem( 0 );
  wpModeCombo->setMinimumHeight( wpModeCombo->sizeHint().height() );//CT 30Nov1998
  wpModeCombo->setMinimumWidth( wpModeCombo->sizeHint().width() );
  connect( wpModeCombo, SIGNAL( activated( int ) ),
	   SLOT( slotWallpaperMode( int )  )  );

  grid->addMultiCellWidget( wpModeCombo, 4,4,1,2 ); //CT 30Nov1998
  /*CT 30Nov1998
  groupLayout->addStretch( 3 );
  
  pushLayout = new QHBoxLayout( 5 ); 
  groupLayout->addLayout( pushLayout );
  */

  randomButton = new QCheckBox( i18n("Ra&ndom"), group );
  randomButton->adjustSize();
  randomButton->setFixedHeight( randomButton->height() );
  randomButton->setMinimumWidth( randomButton->width() );
  connect( randomButton, SIGNAL( clicked() ), SLOT( slotToggleRandom() ) );

  grid->addWidget( randomButton, 6, 0 );//CT 30Nov1998
  //CT  pushLayout->addStretch( 5 );

  randomSetupButton = new QPushButton( i18n("Setu&p..."), group );
  randomSetupButton->adjustSize();
  randomSetupButton->setFixedHeight( randomSetupButton->height() );
  randomSetupButton->setMinimumWidth( randomSetupButton->width() );
  connect( randomSetupButton, SIGNAL( clicked() ), SLOT( slotSetupRandom() ) );

  grid->addWidget( randomSetupButton, 6, 1 );//CT 30Nov1998

  grid->activate();

  dockButton = new QCheckBox( i18n("&Dock into the panel"), this );
  dockButton->setChecked( docking );
  dockButton->adjustSize();
  dockButton->setMinimumSize( dockButton->size() );
  connect( dockButton, SIGNAL( clicked() ), SLOT( slotToggleDock() ) );

  topLayout->addWidget( dockButton, 5, 1 );//CT 30Nov1998
  /*CT 30Nov1998
  groupLayout->addStretch( 3 );
  groupLayout->activate();
  */

  /*CT 30 Nov 1998
  group = new QGroupBox( i18n("Cache size"), this );
  topLayout->addMultiCellWidget( group, 3, 3, 1, 2 );

  grid = new QGridLayout(group,2,3,10,5);
  grid->addRowSpacing(0,10);
  grid->setColStretch(0,0);
  grid->setColStretch(1,0);
  grid->setColStretch(2,1);
  */

  int cacheSize = config->readNumEntry("CacheSize", DEFAULT_CACHE_SIZE);
  //CT 30Nov1998
  aLabel = new QLabel(i18n("Cache size (kB):"), this);
  aLabel->adjustSize();
  aLabel->setMinimumSize(aLabel->size());
  topLayout->addWidget( aLabel, 5, 2 );
  //CT


  /*CT 04Dec1998 - Mark Donohoe suggestion
    aLabel = new QLabel( i18n("kb"), group );
    aLabel->adjustSize();
    aLabel->setMinimumSize(aLabel->size());
    grid->addWidget( aLabel, 9, 2 );
  */

  cacheSlider = new QSlider( 128, 5120, 0, cacheSize, KSlider::Horizontal, this );
  cacheSlider->setSteps(512, 1024);
  cacheSlider->adjustSize();
  cacheSlider->setMinimumSize(cacheSlider->size());
  topLayout->addWidget( cacheSlider, 5, 3 );

  cacheLCD = new QLCDNumber(4, this);
  cacheLCD->setFrameStyle( QFrame::NoFrame );
  cacheLCD->adjustSize();
  //CT 30Nov1998
  cacheLCD->setMinimumHeight(2/3*cacheLCD->height());
  cacheLCD->setMinimumWidth(cacheLCD->width()-20);
  //CT
  cacheLCD->display( cacheSize );
  topLayout->addMultiCellWidget( cacheLCD, 4, 6, 4, 4 );

  connect( cacheSlider, SIGNAL(valueChanged(int)), cacheLCD, SLOT(display(int)) );


  //CT  pushLayout->addWidget( cacheSlider, 5 );

  topLayout->activate();
    
  showSettings();
}


void KBackground::resizeEvent( QResizeEvent * )
{
  monitor->setGeometry( (monitorLabel->width()-200)/2+20,
			(monitorLabel->height()-160)/2+10, 157, 111 );
}


void KBackground::readSettings( int num )
{
  QString group;
  ksprintf( &group, "/desktop%drc", num);

  KConfig config(KApplication::kde_configdir() + group,
		  KApplication::localconfigdir() + group);

  config.setGroup( "Common" );
  randomMode = config.readBoolEntry( "RandomMode", DEFAULT_ENABLE_RANDOM_MODE);

  if ( randomMode || !interactive )
    ksprintf( &group, "Desktop%d", random );
  else
    ksprintf( &group, "Desktop%d", DEFAULT_DESKTOP);

  config.setGroup( group );
  
  QString str;
 
  str = config.readEntry( "Color1", DEFAULT_COLOR_1 );
  currentItem.color1.setNamedColor( str );
       
  str = config.readEntry( "Color2", DEFAULT_COLOR_2);
  currentItem.color2.setNamedColor( str );
	     
  currentItem.ncMode = DEFAULT_NUMBER_OF_COLORS;  // 1
  currentItem.stMode = DEFAULT_COLOR_MODE; // Flat
  str = config.readEntry( "ColorMode", "unset" );
  if ( str == "Gradient" ) {
     currentItem.ncMode = TwoColor;
     currentItem.stMode = Gradient;
  } else if (str == "Pattern") {
     currentItem.ncMode = TwoColor;
     currentItem.stMode = Pattern;
  }
	     
  QStrList strl;
  config.readListEntry("Pattern", strl);
  uint size = strl.count();
  if (size > 8) size = 8;
  uint i = 0;

  for (i = 0; i < 8; i++)
      currentItem.pattern[i] = (i < size) ? QString(strl.at(i)).toUInt() : 255;
   
  str = config.readEntry( "OrientationMode", "unset" );
  if ( str == "Landscape" )
     currentItem.orMode = Landscape;
  else
     currentItem.orMode = DEFAULT_ORIENTATION_MODE;

  currentItem.wpMode = DEFAULT_WALLPAPER_MODE;
  str = config.readEntry( "WallpaperMode", "unset" );
  if ( str == "Mirrored" )
     currentItem.wpMode = Mirrored;
  else if ( str == "CenterTiled" )
     currentItem.wpMode = CenterTiled;
  else if ( str == "Centred" )
     currentItem.wpMode = Centred;
  else if ( str == "CentredBrick" )
     currentItem.wpMode = CentredBrick;
  else if ( str == "CentredWarp" )
     currentItem.wpMode = CentredWarp;
  else if ( str == "CentredMaxpect" )
     currentItem.wpMode = CentredMaxpect;
  else if ( str == "SymmetricalTiled" )
     currentItem.wpMode = SymmetricalTiled;
  else if ( str == "SymmetricalMirrored" )
     currentItem.wpMode = SymmetricalMirrored;
  else if ( str == "Scaled" )
     currentItem.wpMode = Scaled; 
 
  currentItem.bUseWallpaper = config.readBoolEntry( "UseWallpaper", DEFAULT_USE_WALLPAPER ); 
  if ( currentItem.bUseWallpaper ) {
     currentItem.wallpaper = config.readEntry( "Wallpaper", DEFAULT_WALLPAPER );
  if ( !currentItem.wallpaper.isEmpty() && interactive )
     loadWallpaper( currentItem.wallpaper.data() );
  } else
     currentItem.wallpaper = DEFAULT_WALLPAPER;
}

			 
void KBackground::setDefaults()
{
  for ( int i = 0; i < maxDesks; i++ ) {
     currentItem.wpMode = DEFAULT_WALLPAPER_MODE;
     currentItem.ncMode = DEFAULT_NUMBER_OF_COLORS;
     currentItem.stMode = DEFAULT_COLOR_MODE;
     currentItem.orMode = DEFAULT_ORIENTATION_MODE;
     currentItem.color1 = DEFAULT_COLOR_1;
     currentItem.color2 = DEFAULT_COLOR_2;
     currentItem.bUseWallpaper = DEFAULT_USE_WALLPAPER;
     currentItem.wallpaper = DEFAULT_WALLPAPER;
     changed = true;
     writeSettings( i );
  }
  showSettings();
  changed = false;
}
					
void KBackground::defaultSettings()
{
  setDefaults();
}

void KBackground::writeSettings( int num )
{
  if ( !changed ) {
    return;
  }

  QString group;
  ksprintf( &group, "/desktop%drc", num);
  KConfig config(KApplication::kde_configdir() + group, 
		  KApplication::localconfigdir() + group);

  if ( randomMode || !interactive )
    ksprintf( &group, "Desktop%d", random );
  else
    group ="Desktop0";

  config.setGroup( group );
	
  QString col1Name(10);
  col1Name.sprintf("#%02x%02x%02x", currentItem.color1.red(), currentItem.color1.green(), currentItem.color1.blue());
  config.writeEntry( "Color1", col1Name );
	
  QString col2Name(10);
  col2Name.sprintf("#%02x%02x%02x", currentItem.color2.red(), currentItem.color2.green(), currentItem.color2.blue());
  config.writeEntry( "Color2", col2Name );

  config.writeEntry( "UseWallpaper", currentItem.bUseWallpaper );
  config.writeEntry( "Wallpaper", currentItem.wallpaper );

  switch ( currentItem.wpMode )
    {
    case Tiled:
      config.writeEntry( "WallpaperMode", "Tiled" );
      break;
    case Mirrored:
      config.writeEntry( "WallpaperMode", "Mirrored" );
      break;
    case CenterTiled:
      config.writeEntry( "WallpaperMode", "CenterTiled" );
      break;
    case Centred:
      config.writeEntry( "WallpaperMode", "Centred" );
      break;
    case CentredBrick:
      config.writeEntry( "WallpaperMode", "CentredBrick" );
      break;
    case CentredWarp:
      config.writeEntry( "WallpaperMode", "CentredWarp" );
      break;
    case CentredMaxpect:
      config.writeEntry( "WallpaperMode", "CentredMaxpect" );
      break;
    case SymmetricalTiled:
      config.writeEntry( "WallpaperMode", "SymmetricalTiled" );
      break;
    case SymmetricalMirrored:
      config.writeEntry( "WallpaperMode", "SymmetricalMirrored" );
      break;
    case Scaled:
      config.writeEntry( "WallpaperMode", "Scaled" );
      break;
    }
	
  if ( currentItem.ncMode == OneColor ) {
    config.writeEntry( "ColorMode", "Flat" );
  } else if ( currentItem.ncMode == TwoColor && currentItem.stMode == Gradient ) {
    config.writeEntry( "ColorMode", "Gradient" );		
  } else if ( currentItem.ncMode == TwoColor && currentItem.stMode == Pattern ) {
    config.writeEntry( "ColorMode", "Pattern" );
  }
	
  QStrList strl( true ); // deep copies
  for (uint i = 0; i < 8 ; i++) {
    char buffer[10];
    sprintf(buffer, "%d",currentItem.pattern[i]);
    strl.append(buffer);
  }
  config.writeEntry( "Pattern", strl);

  switch ( currentItem.orMode )
    {
    case Portrait:
      config.writeEntry( "OrientationMode", "Portrait" );
      break;
    case Landscape:
      config.writeEntry( "OrientationMode", "Landscape" );
      break;
    }

  config.setGroup( "Common" );
  config.writeEntry( "RandomMode", randomMode );

  changed = false;

  config.sync();

  KConfig *config2 = kapp->getConfig();
  config2->setGroup( "Desktop Common" );
  config2->writeEntry( "OneDesktopMode", oneDesktopMode );
  config2->writeEntry( "DeskNum", deskNum );
  config2->writeEntry( "Docking", dockButton->isChecked() );
  config2->writeEntry( "CacheSize", cacheSlider->value() );
  config2->sync();
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
    deskListBox->insertItem( i18n( "Default" ) );
    
  deskListBox->setCurrentItem( current );
  deskListBox->setUpdatesEnabled( true );
}

void KBackground::setDesktop( int desk )
{
  if ( deskNum == desk )
    return;

  if ( rnddlg )
    rnddlg->done( QDialog::Accepted );
  else
    writeSettings( deskNum );

  deskNum = desk;

  if ( deskNum <= 0 )
    deskNum = 0;

  if ( deskNum >= maxDesks - 1 )
    deskNum = maxDesks - 1;

  readSettings( deskNum );
  showSettings();

}

void KBackground::showSettings()
{ 

  colButton1->setColor( currentItem.color1 );
  colButton2->setColor( currentItem.color2 );
  ((QRadioButton *)ncGroup->find( OneColor ))->setChecked( currentItem.ncMode == OneColor );
  ((QRadioButton *)ncGroup->find( TwoColor ))->setChecked( currentItem.ncMode == TwoColor);

  slotColorMode( currentItem.ncMode );

  wpCombo->setCurrentItem( 0 );
  for ( int i = 1; i < wpCombo->count(); i++ )
    {
      if ( currentItem.wallpaper == wpCombo->text( i ) )
	{
	  wpCombo->setCurrentItem( i );
	  break;
	}
    }

  if ( currentItem.bUseWallpaper && wpCombo->currentItem() == 0 )
    {
      wpCombo->insertItem( currentItem.wallpaper );
      wpCombo->setCurrentItem( wpCombo->count()-1 );
    }

  wpModeCombo->setCurrentItem( currentItem.wpMode - 1 );

  randomButton->setChecked( randomMode );

  //CT 30Nov1998 - make sure all comes up rightly
  wpCombo->setEnabled(!randomMode);
  browseButton->setEnabled(!randomMode);
  //CT
  
  // wpModeCombo should be disabled when in randomMode, but only when there
  // is not random setup dialog opened
  if ( !rnddlg ) {
    wpModeCombo->setEnabled(!randomMode);
    randomSetupButton->setEnabled(randomMode);
  }

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
  float sx = (float)monitor->width() / 
    QApplication::desktop()->width();
  float sy = (float)monitor->height() / 
    QApplication::desktop()->height();
  QWMatrix matrix;
  matrix.scale( sx, sy );
	
  if ( wpPixmap.isNull() || !currentItem.bUseWallpaper ||
       currentItem.wpMode == Centred || currentItem.wpMode == CentredBrick ||
       currentItem.wpMode == CentredWarp || currentItem.wpMode ==CentredMaxpect) {

    KPixmap preview;
    preview.resize( monitor->width()+1, monitor->height()+1 );
	
    if ( currentItem.ncMode == OneColor ) {
      preview.fill( currentItem.color1 );
    } else if ( currentItem.stMode == Gradient ) {
		
      if( currentItem.orMode == Portrait ) 
	preview.gradientFill( currentItem.color2, currentItem.color1, true );
      else
	preview.gradientFill( currentItem.color2, currentItem.color1, false );
			
    } else
      preview.patternFill(currentItem.color1,currentItem.color2, currentItem.pattern);
    
    if ( currentItem.wpMode == CentredBrick ) {
      int i, j, k;

      int w = preview.width();

      QPainter paint( &preview );
      paint.setPen( white );
      for ( i=k=0; i < w; i+=5,k++ ) {
	paint.drawLine( 0, i, w, i );
	for (j=(k&1) * 5 + 2; j< w; j+=10) 
	  paint.drawLine( j, i, j, i+5 );
      }
    }
    else if ( currentItem.wpMode == CentredWarp ) {
      int i;

      int w = preview.width();
      int h = preview.height();

      QPainter paint( &preview );
      paint.setPen( white );
      for ( i=0; i < w; i+=4 )
	paint.drawLine( i, 0, w - i, h );
      for ( i=0; i < h; i+=4 )
	paint.drawLine( 0, i, w, h - i );

    }

    if ( ( currentItem.wpMode == Centred ) ||
	 ( currentItem.wpMode == CentredBrick ) || 
	 ( currentItem.wpMode == CentredWarp )  ||
	 ( currentItem.wpMode == CentredMaxpect) && currentItem.bUseWallpaper ) {
      QPixmap tmp = wpPixmap.xForm( matrix );

      bitBlt( &preview, (preview.width() - tmp.width())/2,
	      (preview.height() - tmp.height())/2, &tmp,
	      0, 0, tmp.width(), tmp.height() );
    }
	
    monitor->setBackgroundPixmap( preview );

  } else if ( !wpPixmap.isNull() )
    monitor->setBackgroundPixmap( wpPixmap.xForm( matrix ) );
    
}

// Attempts to load the specified wallpaper and creates a centred/scaled/mirrored etc.
// version if necessary.
// Note that centred pixmaps are placed on a full screen image of background
// color1, so if you want to save memory use a small tiled pixmap.
//
bool KBackground::loadWallpaper( const char *name, bool useContext )
{
  static int context = 0;
  QString filename;
  int rv = false;
  KPixmap tmp;

  QApplication::setOverrideCursor( waitCursor );

  if ( useContext )
    {
      if ( context )
	QColor::destroyAllocContext( context );
      context = QColor::enterAllocContext();
    }

  if ( name[0] != '/' )
    {
      filename = kapp->kde_wallpaperdir().copy();
      filename += "/";
      filename += name;
    }
  else
    filename = name;
	
  if ( tmp.load( filename, 0, KPixmap::LowColor ) )
    {
      int w = QApplication::desktop()->width();
      int h = QApplication::desktop()->height();

      if ( ( tmp.width() > w || tmp.height() > h ||
	   currentItem.wpMode == CentredMaxpect ) &&
	   currentItem.wpMode != Scaled ) {
	// shrink if image is bigger than desktop or mode is CentredMaxpect
	float sc;
	float S = (float)h / (float)w ;
	float I = (float)tmp.height() / (float)tmp.width() ;
	
	if (S < I)
	  sc= (float)h / (float)tmp.height();
	else
	  sc= (float)w / (float)tmp.width();
	
	QWMatrix scaleMat;
	scaleMat.scale(sc,sc);

	QPixmap tmp2 = tmp.xForm( scaleMat );
	tmp.resize( tmp2.width(), tmp2.height() );
	bitBlt( &tmp, 0, 0, &tmp2 );
      }

      switch ( currentItem.wpMode ) {

      case Mirrored:
	{
	  int w = tmp.width();
	  int h = tmp.height();

	  wpPixmap.resize( w * 2, h * 2);

	  /* quadrant 2 */
	  bitBlt( &wpPixmap, 0, 0, &tmp );
	    
	  /* quadrant 1 */
	  QWMatrix S(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
	  QPixmap newp = tmp.xForm( S );
 	  bitBlt( &tmp, 0, 0, &newp ); 
	  bitBlt( &wpPixmap, w, 0, &tmp ); 

	  /* quadrant 4 */
	  S.setMatrix(1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
	  newp = tmp.xForm( S );
 	  bitBlt( &tmp, 0, 0, &newp ); 
	  bitBlt( &wpPixmap, w, h, &tmp );

	  /* quadrant 3 */
	  S.setMatrix(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
	  newp = tmp.xForm( S );
 	  bitBlt( &tmp, 0, 0, &newp ); 
	  bitBlt( &wpPixmap, 0, h, &tmp );
	}
	break;

      case SymmetricalTiled:
      case SymmetricalMirrored:
	{
	  int fliph = 0;
	  int flipv = 0;
	  int w0 = tmp.width();
	  int h0 = tmp.height();
	  int mode = currentItem.wpMode;

	  wpPixmap.resize(w, h);

	  if (w == w0) {
	    /* horizontal center line */
	    int y, ay;
      
	    y = h0 - ((h/2)%h0); /* Starting point in picture to copy */
	    ay = 0;    /* Vertical anchor point */
	    while (ay < h) {
	      bitBlt( &wpPixmap, 0, ay, &tmp, 0, y );
	      ay += h0 - y;
	      y = 0;
	      if ( mode == SymmetricalMirrored ) {
		QWMatrix S(1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
		QPixmap newp = tmp.xForm( S );
		bitBlt( &tmp, 0, 0, &newp ); 
		flipv = !flipv;
	      }
	    }
	  }
	  else if (h == h0) {
	    /* vertical centerline */
	    int x, ax;
      
	    x = w0 - ((w/2)%w0); /* Starting point in picture to copy */
	    ax = 0;    /* Horizontal anchor point */
	    while (ax < w) {
	      bitBlt( &wpPixmap, ax, 0, &tmp, x, 0 );
	      ax += w0 - x;
	      x = 0;
	      if ( mode == SymmetricalMirrored ) {
		QWMatrix S(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
		QPixmap newp = tmp.xForm( S );
		bitBlt( &tmp, 0, 0, &newp ); 
		fliph = !fliph;
	      }
	    }
	  }
	  else {
	    /* vertical and horizontal centerlines */
	    int x,y, ax,ay;
      
	    y = h0 - ((h/2)%h0); /* Starting point in picture to copy */
	    ay = 0;    /* Vertical anchor point */
      
	    while (ay < h) {
	      x = w0 - ((w/2)%w0);/* Starting point in picture to cpy */
	      ax = 0;    /* Horizontal anchor point */
	      while (ax < w) {
		bitBlt( &wpPixmap, ax, ay, &tmp, x, y );
		if ( mode == SymmetricalMirrored ) {
		  QWMatrix S(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
		  QPixmap newp = tmp.xForm( S );
		  bitBlt( &tmp, 0, 0, &newp ); 
		  fliph = !fliph;
		}
		ax += w0 - x;
		x = 0;
	      }
	      if ( mode == SymmetricalMirrored ) {
		QWMatrix S(1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
		QPixmap newp = tmp.xForm( S );
		bitBlt( &tmp, 0, 0, &newp ); 
		flipv = !flipv;
		if (fliph) {   /* leftmost image is always non-hflipped */
		  S.setMatrix(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
		  newp = tmp.xForm( S );
		  bitBlt( &tmp, 0, 0, &newp ); 
		  fliph = !fliph;
		}
	      }
	      ay += h0 - y;
	      y = 0;
	    }
	  }
	}
	break;
	
      case CenterTiled:
	{
	  int i, j, x, y, w0, h0, ax, ay, w1, h1, offx, offy;

	  wpPixmap.resize(w, h);

	  w0 = tmp.width();  h0 = tmp.height();

	  /* compute anchor pt (top-left coords of top-left-most pic) */
	  ax = (w-w0)/2;  ay = (h-h0)/2;
	  while (ax>0) ax = ax - w0;
	  while (ay>0) ay = ay - h0;

	  for (i=ay; i < (int)h; i+=h0) {
	    for (j=ax; j < (int)w; j+=w0) {
	      /* if image goes off tmpPix, only draw subimage */
	  
	      x = j;  y = i;  w1 = w0;  h1 = h0;  offx = offy = 0;
	      if (x<0)           { offx = -x;  w1 -= offx;  x = 0; }
	      if (x+w1>w0) { w1 = (w0-x); }

	      if (y<0)           { offy = -y;  h1 -= offy;  y = 0; }
	      if (y+h1>h0)    { h1 = (h0-y); }
	  
	      bitBlt( &wpPixmap, x, y, &tmp, offx, offy );
	    }
	  }

	}
	break;

      case Centred:
      case CentredBrick:
      case CentredWarp:
      case CentredMaxpect:
	wpPixmap = tmp;
	break;

      case Scaled:
	{
	  float sx = (float)w / tmp.width();
	  float sy = (float)h / tmp.height();
			
	  wpPixmap.resize( w, h );
	  wpPixmap.fill( currentItem.color1 );
			
	  QWMatrix matrix;
	  matrix.scale( sx, sy );
	  QPixmap newp = tmp.xForm( matrix );
	  bitBlt( &wpPixmap, 0, 0, &newp ); 
	}
	break;

      default:
	wpPixmap.resize( tmp.width(), tmp.height() );
	wpPixmap.fill( currentItem.color1 );
	bitBlt( &wpPixmap, 0, 0, &tmp ); 
      }
      rv = true;
    }

  if ( useContext )
    QColor::leaveAllocContext();

  QApplication::restoreOverrideCursor();

  return rv;
}

void KBackground::slotSelectColor1( const QColor &col )
{
  currentItem.color1 = col;

  if ( currentItem.ncMode == Gradient || !currentItem.bUseWallpaper ||
       currentItem.wpMode == Centred || currentItem.wpMode == CentredBrick || currentItem.wpMode == CentredWarp )
    {
      // force the background to be made with different background
      if ( currentItem.wpMode == Centred || currentItem.wpMode == CentredBrick || currentItem.wpMode == CentredWarp )
	loadWallpaper( currentItem.wallpaper.data() );
      setMonitor();
    }

  changed = true;
  if ( rnddlg && interactive )
    rnddlg->copyCurrent();
}

void KBackground::slotSelectColor2( const QColor &col )
{
  currentItem.color2 = col;

  if ( currentItem.ncMode == Gradient || !currentItem.bUseWallpaper ||
       currentItem.wpMode == Centred || currentItem.wpMode == CentredBrick
       || currentItem.wpMode == CentredWarp )
    {
      setMonitor();
    }

  changed = true;
  if ( rnddlg && interactive )
    rnddlg->copyCurrent();
}

void KBackground::slotBrowse()
{
  QString path;

  static bool firsttime = true;

  if (firsttime) { // the file selector remembers the last path
    path = kapp->kde_wallpaperdir().copy();

    QDir dir( path );
    if ( !dir.exists() )
      path = 0;
	    
    firsttime = false;
  }

  QString filename = KFileDialog::getOpenFileName( path );
  slotWallpaper( filename );
  if ( !filename.isNull() && !strcmp( filename, currentItem.wallpaper) )
    {
      wpCombo->insertItem( currentItem.wallpaper );
      wpCombo->setCurrentItem( wpCombo->count() - 1 );
    }
}

void KBackground::slotWallpaper( const char *filename )
{
  if ( filename )
    {
      if ( !strcmp(filename, i18n("No wallpaper") ) )
	{
	  wpPixmap.resize(0, 0); // make NULL pixmap
	  currentItem.wallpaper = filename;
	  currentItem.bUseWallpaper = false;
	  setMonitor();
	  if ( rnddlg && interactive )
	    rnddlg->copyCurrent();
	}
      else if ( loadWallpaper( filename ) )
	{
	  currentItem.wallpaper = filename;
	  currentItem.bUseWallpaper = true;
	  setMonitor();
	  if ( rnddlg && interactive )
	    rnddlg->copyCurrent();
	}
	    
      changed = true;
    }
}

void KBackground::slotWallpaperMode( int m )
{
  currentItem.wpMode = m + 1;
  slotWallpaper( currentItem.wallpaper.data() );
}

void KBackground::slotColorMode( int m )
{
  currentItem.ncMode = m;

  switch ( currentItem.ncMode ) {

  case OneColor:
    {
      colButton2->setEnabled( False );
      changeButton->setEnabled( False );
    }
    break;
    
  default:
    {
      colButton2->setEnabled( True );
      changeButton->setEnabled( True );
      if ( currentItem.stMode == Flat )
	currentItem.stMode = Gradient;
    }
    break;
  }

  setMonitor();
  changed = true;
  if ( rnddlg && interactive )
    rnddlg->copyCurrent();
}

void KBackground::slotSetup2Color()
{
  KBPatternDlg dlg(currentItem.color1, currentItem.color2, currentItem.pattern, &currentItem.orMode, &currentItem.stMode, this);
  if (dlg.exec()) {
    setMonitor();
    changed = true;
    if ( rnddlg && interactive )
      rnddlg->copyCurrent();
  } 
}


void KBackground::slotSetupRandom()
{
  rnddlg = new KRandomDlg( deskNum, this );
  rnddlg->show();
  randomSetupButton->setEnabled( false );
  wpModeCombo->setEnabled( true );
}


void KBackground::slotToggleRandom()
{
  if ( rnddlg )
    rnddlg->done( QDialog::Accepted );

  randomMode = !randomMode;
  changed = true;
  randomSetupButton->setEnabled(randomButton->isChecked());
  wpCombo->setEnabled(!randomButton->isChecked());
  browseButton->setEnabled(!randomButton->isChecked());
  //AL 31Dic1998 - wpModeCombo should be disabled when in randomMode
  wpModeCombo->setEnabled(!randomButton->isChecked());
  //AL
}


void KBackground::slotToggleDock()
{
  docking = !docking;
  changed = true;
}


void KBackground::slotToggleOneDesktop()
{
  oneDesktopMode = !oneDesktopMode;

  if (rnddlg )
    rnddlg->done( QDialog::Accepted );
  
  oneDesktopButton->setChecked( oneDesktopMode );
//  renameButton->setEnabled( !oneDesktopMode );
  deskListBox->setEnabled( !oneDesktopMode );

  changed = true;
}


void KBackground::slotStyleMode( int m )
{
  currentItem.stMode = m;
  setMonitor();
  changed = true;
}

void KBackground::slotSwitchDesk( int num )
{
  setDesktop( num );
}

void KBackground::slotRenameDesk()
{
  KRenameDeskDlg dlg( KWM::getDesktopName( deskNum+1 ), this );

  dlg.setCaption( i18n( "Desktop names" ) );

  if ( dlg.exec() == QDialog::Accepted )
    {
      KWM::setDesktopName( deskNum+1, dlg.title() );
      getDeskNameList();
    }
}

void KBackground::slotHelp()
{
  kapp->invokeHTMLHelp( "kcmdisplay/kdisplay-3.html", "" );
}

void KBackground::loadSettings()
{
  readSettings(deskNum);
  showSettings();
}

void KBackground::applySettings()
{
  if ( rnddlg )
    rnddlg->done( QDialog::Accepted );
  else
    writeSettings(deskNum);

  apply(true);
}


bool KBackground::setNew( QString pic, int item )
{
  if ( !strcmp(pic, i18n("No wallpaper") ) )
    {
      wpPixmap.resize(0, 0); // make NULL pixmap
      currentItem.wallpaper = pic;
      currentItem.bUseWallpaper = false;
    }
  else if ( loadWallpaper( pic.data() ) )
    {
      currentItem.wallpaper = pic;
      currentItem.bUseWallpaper = true;
    }
  else {
    QMessageBox::warning(this, i18n("File error"),
			 i18n("Can't load image file") );
    return false;
  }

  changed = true;

  random = item;

  showSettings();

  return true;
}


void KBackground::slotDropped (KDNDDropZone *zone)
{
  QStrList &list = zone->getURLList();;

  if ( list.count() > 0 ) {

    QString url = list.first();

    if (strcmp("file:", url.left(5)))  // for now, only file URLs are supported
      QMessageBox::warning(this, i18n("Unsupported URL"),
        i18n( "Sorry, this type of URL is currently unsupported"\
		 "by the KDE Display Module" ) );
    else {
      url = url.right(url.length()-5); // strip the leading "file:"

      setNew( url, random );
    }
  }
}



KBPatternDlg::KBPatternDlg( QColor col1, QColor col2, uint *p, int *orient,
			    int *type, QWidget *parent, char *oname)
  : QDialog( parent, oname, true)
{
  int i;
  pattern = p;
  orMode = orient;
  tpMode = type;
  color1 = col1; 
  color2 = col2;
	
  if ( 	*tpMode == KBackground::Gradient &&
	*orMode == KBackground::Portrait ) {
    mode = Portrait;
  } else if ( *tpMode == KBackground::Gradient &&
	      *orMode == KBackground::Landscape ) {
    mode = Landscape;
  } else
    mode = Pattern;
	
  setCaption( i18n( "Two color backgrounds" ) );

  KConfig *config = kapp->getConfig();
  QString group = config->group();
  config->setGroup( "Defined Pattern" );
  int count = config->readNumEntry( "Count" );

  if (!count)
    count = savePatterns();

  QString name;
  for (i = 0; i < count; i++) {
    PatternEntry *entry = new PatternEntry();
    name.sprintf("Name%d",i);
    entry->name = config->readEntry( name );
    if (entry->name.isNull()) {
      delete entry;
      continue;
    }
    QStrList strl;
    name.sprintf("Pattern%d",i);
    config->readListEntry(name, strl);
    uint size = strl.count();
    if (size > 8) size = 8;
    for (uint j = 0; j < 8; j++)
      entry->pattern[j] = (j < size) ? 
	QString(strl.at(j)).toUInt() : 255;
    list.append(entry);
  }

  QVBoxLayout *toplevelHL = new QVBoxLayout(this, 10, 5);
	
  suGroup = new QButtonGroup( this );
  suGroup->hide();
  suGroup->setExclusive( true );
	
  QRadioButton *rb =
    new QRadioButton( i18n("Blend colors from &top to bottom"), this );
  rb->setFixedHeight( rb->sizeHint().height() );
  suGroup->insert( rb, Portrait );
	
  toplevelHL->addWidget( rb );
	
  rb = new QRadioButton( i18n("Blend colors from &right to left"), this );
  rb->setFixedHeight( rb->sizeHint().height() );
  suGroup->insert( rb, Landscape );
	
  toplevelHL->addWidget( rb );
	
  rb = new QRadioButton( i18n("Use &pattern"), this );
  rb->setFixedHeight( rb->sizeHint().height() );
  suGroup->insert( rb, Pattern ); 
	
  toplevelHL->addWidget( rb );
	
  connect( suGroup, SIGNAL( clicked( int ) ), SLOT( slotMode( int ) ) );
  ((QRadioButton *)suGroup->find( Portrait ))->setChecked( mode == Portrait );
  ((QRadioButton *)suGroup->find( Landscape ))->setChecked( mode ==Landscape );
  ((QRadioButton *)suGroup->find( Pattern ))->setChecked( mode == Pattern );
	
  QGridLayout *grid = new QGridLayout( 3, 4, 4);
    
  toplevelHL->addLayout( grid );
	
  grid->setRowStretch(0,10);
  grid->setRowStretch(1,10);
  grid->setRowStretch(2,10);
	
  grid->setColStretch(0,0);
  grid->setColStretch(1,10);
  grid->setColStretch(2,10);
  grid->setColStretch(3,0);
	
  grid->addColSpacing(0,15);
    
  lName = new QLabel( this );
  lName->setText( i18n( "Preview" ) );
  lName->setMinimumSize( lName->sizeHint() );
	
  grid->addWidget( lName, 0, 2 );
	
  listBox = new QListBox( this );
  connect(listBox, SIGNAL(highlighted(const char*)), 
	  SLOT(selected(const char*)));
    
  grid->addWidget( listBox, 1, 1);
	
  lPreview = new QLabel( listBox, i18n( "Pattern &name" ), this );
  lPreview->setMinimumSize( lPreview->sizeHint() );
	
  grid->addWidget( lPreview, 0, 1 );
    
  preview = new QLabel( this );
  preview->adjustSize();
  preview->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  preview->setLineWidth( 1 );
  preview->setMargin( 0 );
  preview->setFixedSize( 120, 120 );
    
  grid->addWidget( preview, 1, 2 );
    
  listBox->setMaximumHeight( preview->height() );
   
  QFrame* tmpQFrame;
  tmpQFrame = new QFrame( this );
  tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  tmpQFrame->setMinimumHeight( tmpQFrame->sizeHint().height() );
	
  toplevelHL->addSpacing( 5 );
  toplevelHL->addWidget( tmpQFrame );
  toplevelHL->addSpacing( 5 );
   
  KButtonBox *bbox = new KButtonBox( this );
  bbox->addStretch( 10 );
	
  QPushButton* okPB;
  okPB = bbox->addButton( i18n("&OK") );
  connect( okPB, SIGNAL(clicked()), SLOT( accept()) );
	
  QPushButton* cancelPB;
  cancelPB = bbox->addButton( i18n("&Cancel") );
  connect( cancelPB, SIGNAL(clicked()), SLOT( reject()) );
	
  bbox->layout();
  toplevelHL->addWidget( bbox );

  current = new PatternEntry( i18n("Current"), p);
  bool predefined = false;
  int index = 0;
  for ( PatternEntry *item=list.first(); item != 0; item=list.next() ) {
    if (*current == item->pattern ) {
      delete current;
      current = item;
      predefined = true;
      break;
    }
    index++;
  }
    
  if (!predefined) {
    listBox->insertItem( current->name );
  }

  for ( PatternEntry *item=list.first(); item != 0; item=list.next() )
    listBox->insertItem(item->name);
	
  listBox->setCurrentItem( index );
	
  // we do this here to let all items fit in
  listBox->adjustSize();
  listBox->setMinimumSize(listBox->size());
	
  slotMode( mode );
  selected( current->name );

  toplevelHL->activate();
	
  resize(350,0);

    
}

void KBPatternDlg::slotMode( int m )
{
  mode = m;
  switch ( m ) {

  case Landscape:
  case Portrait:
    listBox->setEnabled( False );
    lName->setEnabled( False );
    lPreview->setEnabled( False );
    break;
	
  default:
    listBox->setEnabled( true );
    lName->setEnabled( true );
    lPreview->setEnabled( true );
    break;
  }
  selected( listBox->text( listBox->currentItem() ) );
}

int KBPatternDlg::savePatterns() {
  KConfig *config = kapp->getConfig();

  config->writeEntry("Name0", i18n("Filled"), true, false, true);
  config->writeEntry("Pattern0", "255,255,255,255,255,255,255,255,");
    
  config->writeEntry("Name1", i18n("Fish net"), true, false, true);
  config->writeEntry("Pattern1", "135,206,236,120,062,103,115,225,");

  config->writeEntry("Name2", i18n("Triangles"), true, false, true);
  config->writeEntry("Pattern2", "120,248,249,251,255,8,24,56,");

  config->writeEntry("Name3", i18n("Flowers"), true, false, true);
  config->writeEntry("Pattern3", "225,115,39,2,64,228,206,135,");

  config->writeEntry("Name4", i18n("Rattan"), true, false, true);
  config->writeEntry("Pattern4", "7,139,221,184,112,232,221,142,");

  config->writeEntry("Name5", i18n("Cobbled Pavement"), true, false, true);
  config->writeEntry("Pattern5", "81,178,160,247,178,81,178,");

  config->writeEntry("Count", 6);

  config->sync();
  return 6;
}

void KBPatternDlg::selected( const char *selected )
{
  for ( PatternEntry *item=list.first(); item != 0; item=list.next() )
    if (*item == selected) {
		
      KPixmap tmp;
      tmp.resize(preview->width() + 2, preview->height() + 2);
      if( listBox->isEnabled() )
	tmp.patternFill( color1, color2, item->pattern);
      else {
	QColorGroup cg = colorGroup();
	tmp.patternFill( cg.mid(), cg.background(), item->pattern);
      }
      preview->setPixmap(tmp);
      current = item;
      break;
    }
}

void KBPatternDlg::done( int r ) 
{
  hide();
	
  int orient = KBackground::Portrait;
  int type = KBackground::Gradient;
	
  if( mode == Landscape ) {
    type = KBackground::Gradient;
    orient = KBackground::Landscape;
  } 
  if( mode == Pattern ) {
    type = KBackground::Pattern;
  }
	    
  if ( r == Rejected || 
       ( *current == pattern && *orMode == orient && *tpMode == type ) ) {
    setResult(Rejected);
    return;
  }
	
  *orMode = orient;
  *tpMode = type;
    
  for (uint i = 0; i < 8; i++)
    pattern[i] = current->pattern[i];

  setResult(Accepted);
    
}


KRandomDlg::KRandomDlg(int _desktop, KBackground *_kb, char *name)
  : QDialog( 0, name ), kb( _kb ), desktop( _desktop )
{

  ItemList.setAutoDelete( true );

  setCaption( i18n( "Random mode setup" ) );

  readSettings();

  QVBoxLayout *toplevelHL = new QVBoxLayout(this, 10, 5);
	
  dirCheckBox = new QCheckBox( i18n( "Pick files from &directory" ), this );
  dirCheckBox->setChecked( useDir );
  connect( dirCheckBox, SIGNAL( clicked() ), SLOT( changeDir() ) );
  toplevelHL->addWidget( dirCheckBox, 1 );

  QBoxLayout *pushLayout = new QHBoxLayout( 5 );
  toplevelHL->addLayout( pushLayout );

  dirLined = new QLineEdit( this );
  dirLined->setMinimumSize( dirLined->sizeHint() );
  dirLined->setText( picDir );
  dirLined->setEnabled( useDir );
  pushLayout->addWidget( dirLined, 5 );

  dirPushButton = new QPushButton( i18n( "&Browse ..." ), this );
  dirPushButton->adjustSize();
  dirPushButton->setFixedHeight( dirPushButton->height() );
  dirPushButton->setMinimumWidth( dirPushButton->width() );
  dirPushButton->setEnabled( useDir );
  connect( dirPushButton, SIGNAL( clicked() ), SLOT( slotBrowse() ) );
  pushLayout->addWidget( dirPushButton, 0 );

  QFrame* tmpQFrame;
  tmpQFrame = new QFrame( this );
  tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  tmpQFrame->setMinimumHeight( tmpQFrame->sizeHint().height() );

  toplevelHL->addSpacing( 5 );
  toplevelHL->addWidget( tmpQFrame );
  toplevelHL->addSpacing( 5 );

  pushLayout = new QHBoxLayout( 5 );
  toplevelHL->addLayout( pushLayout );

  timerLabel = new QLabel( this );
  timerLabel->setText( i18n( "Timer Delay in seconds :" ) );
  timerLabel->setMinimumSize( timerLabel->sizeHint() );
	
  pushLayout->addWidget( timerLabel );

  timerLined = new KIntegerLine( this );
  timerLined->setValue( delay );
  timerLined->setMinimumSize( timerLined->sizeHint() );

  pushLayout->addWidget( timerLined, 5 );
  pushLayout->addStretch( 5 );

  orderButton = new QCheckBox( i18n("In &order"), this );
  orderButton->setChecked( inorder );
  orderButton->setFixedHeight( orderButton->sizeHint().height() );
  orderButton->setFixedWidth( orderButton->sizeHint().width() );

  pushLayout->addWidget( orderButton, 5 );
  pushLayout->addStretch( 5 );

  tmpQFrame = new QFrame( this );
  tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  tmpQFrame->setMinimumHeight( tmpQFrame->sizeHint().height() );
	
  toplevelHL->addSpacing( 5 );
  toplevelHL->addWidget( tmpQFrame );
  toplevelHL->addSpacing( 5 );

  desktopLabel = new QLabel( this );
  desktopLabel->setText( i18n( "Desktops :" ) );
  desktopLabel->setMinimumSize( desktopLabel->sizeHint() );
	
  toplevelHL->addWidget( desktopLabel );
	
  listBox = new QListBox( this );
    
  if ( count > 0 ){
    for ( int i = 0; i < count; i++ )
      listBox->insertItem( ItemList.at(i)->wallpaper );
    item = 0;
    listBox->setCurrentItem( item );
  }

  // we do this here to let all items fit in
  listBox->adjustSize();
  listBox->setMinimumSize(listBox->size());
  listBox->setEnabled( !useDir );
  connect(listBox, SIGNAL(highlighted(int)), 
	  SLOT(selected(int)));
	
  toplevelHL->addWidget( listBox, 5 );
	
  picdrop = new KDNDDropZone(listBox, DndURL);
  connect(picdrop, SIGNAL(dropAction(KDNDDropZone*)), 
	  SLOT(picDropped(KDNDDropZone*)));

  tmpQFrame = new QFrame( this );
  tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  tmpQFrame->setMinimumHeight( tmpQFrame->sizeHint().height() );
	
  toplevelHL->addSpacing( 5 );
  toplevelHL->addWidget( tmpQFrame );
  toplevelHL->addSpacing( 5 );
   
  KButtonBox *bbox = new KButtonBox( this );
  bbox->addStretch( 10 );
	
  QPushButton* addPB;
  addPB = bbox->addButton( i18n("&Add") );
  connect( addPB, SIGNAL(clicked()), SLOT( slotAdd()) );

  QPushButton* deletePB;
  deletePB = bbox->addButton( i18n("&Delete") );
  connect( deletePB, SIGNAL(clicked()), SLOT( slotDelete()) );

  QPushButton* okPB;
  okPB = bbox->addButton( i18n("&OK") );
  connect( okPB, SIGNAL(clicked()), SLOT( accept()) );
	
  QPushButton* cancelPB;
  cancelPB = bbox->addButton( i18n("&Cancel") );
  connect( cancelPB, SIGNAL(clicked()), SLOT( reject()) );
	
  bbox->layout();
  toplevelHL->addWidget( bbox );
  toplevelHL->activate();

  setMinimumHeight( 400 );
}


void KRandomDlg::selected( int index )
{
  item = index;
  kb->random = item;
  kb->currentItem = *ItemList.at( item );
  kb->loadWallpaper( ItemList.at( item )->wallpaper );
  kb->showSettings();
}


void KRandomDlg::changeDir()
{
  useDir = (! useDir);

  dirLined->setEnabled( useDir );
  dirPushButton->setEnabled( useDir );
  listBox->setEnabled( !useDir );
}


void KRandomDlg::slotBrowse()
{
  QString tmp =  KFileDialog::getDirectory(dirLined->text(), this, "dirdlg");

  if ( tmp.isEmpty() )
    return;

  dirLined->setText( tmp );
}


void KRandomDlg::picDropped( KDNDDropZone *zone )
{ 
  QStrList &list = zone->getURLList();;
  int len = list.count();

  for ( int i = 0; i < len; i++) {
    
    QString url = list.at(i);

    if (strcmp("file:", url.left(5)))  // for now, only file URLs are supported
      QMessageBox::warning(this, i18n("Unsupported URL"),
        i18n( "Sorry, this type of URL is currently unsupported"\
		 "by the KDE Display Module" ) );
    else {
      url = url.right(url.length()-5); // strip the leading "file:"
      addToPicList( url );
    }
  }
}


void KRandomDlg::addToPicList( QString pic )
{
  item = listBox->count();

  kb->interactive = false;
  if ( kb->setNew( pic, item ) ) {
    listBox->insertItem(pic);
    disconnect( listBox, SIGNAL(highlighted(int)), this, SLOT(selected(int)) );
    listBox->setCurrentItem( item );
    connect(listBox, SIGNAL(highlighted(int)), SLOT(selected(int)));
    KItem *it = new KItem ( kb->currentItem );
    ItemList.append( it );
  }
  kb->interactive = true;
}


void KRandomDlg::slotAdd()
{
  addToPicList( i18n("No wallpaper") );
}


void KRandomDlg::slotDelete()
{
  if ( listBox->count() > 1 ) {
    disconnect( listBox, SIGNAL(highlighted(int)), this, SLOT(selected(int)) );
    listBox->removeItem( item );
    ItemList.remove( item );

    item = listBox->currentItem();
    kb->random = item;
    kb->currentItem = *ItemList.at( item );
    kb->loadWallpaper( ItemList.at( item )->wallpaper );
    kb->showSettings();
    
    connect(listBox, SIGNAL(highlighted(int)), SLOT(selected(int)));
  }
}


void KRandomDlg::copyCurrent()
{
  KItem *it = new KItem ( kb->currentItem );
  ItemList.remove( item );
  ItemList.insert( item, it );
  listBox->changeItem( ItemList.at(item)->wallpaper, item );
}


void KRandomDlg::readSettings()
{
  QString tmpf;
  ksprintf( &tmpf, "/desktop%drc", desktop);

  KConfig picturesConfig(KApplication::kde_configdir() + tmpf,
		  KApplication::localconfigdir() + tmpf);

  picturesConfig.setGroup( "Common" );
  count = picturesConfig.readNumEntry( "Count", DEFAULT_RANDOM_COUNT );
  delay = picturesConfig.readNumEntry( "Timer", DEFAULT_RANDOM_TIMER );
  inorder = picturesConfig.readBoolEntry( "InOrder", DEFAULT_RANDOM_IN_ORDER );
  useDir = picturesConfig.readBoolEntry( "UseDir", DEFAULT_RANDOM_USE_DIR );
  picDir = picturesConfig.readEntry( "Directory", KApplication::kde_wallpaperdir() );
  
  KItem *newitem = new KItem( kb->currentItem );
  ItemList.append( newitem );
  kb->interactive = false;
  for ( int i = 1; i < count; i++ ){
    kb->random = i;
    kb->readSettings( desktop );
    newitem = new KItem( kb->currentItem );
    ItemList.append( newitem );
  }
  kb->currentItem = *ItemList.first();
  kb->interactive = true;
}


void KRandomDlg::done( int r ) 
{
  if ( r == Rejected ) {
    hide();
  
    listBox->setCurrentItem( 0 );
    setResult(Rejected);

    kb->rnddlg = 0L;
    kb->random = 0;
    kb->randomSetupButton->setEnabled( true );
    kb->wpModeCombo->setEnabled( false );
  
    return;
  }

  QDir d( dirLined->text() );
  if ( !d.exists() ) {
    QMessageBox::warning(this, i18n("Directory error"),
			 i18n("Selected directory doesn't exists") );
    return;
  }

  kb->interactive = false;
  for ( int i = 0; i < (int)listBox->count(); i++ ) {
    kb->random = i;
    kb->currentItem = *ItemList.at( i );
    kb->changed = true;
    kb->writeSettings( kb->deskNum );
  }

  kb->currentItem = *ItemList.first();
  kb->interactive = true;

  QString tmpf;
  ksprintf( &tmpf, "/desktop%drc", desktop);

  KConfig picturesConfig(KApplication::kde_configdir() + tmpf,
		  KApplication::localconfigdir() + tmpf);

  picturesConfig.setGroup( "Common" );
  picturesConfig.writeEntry( "Count", listBox->count() );
  picturesConfig.writeEntry( "Timer", timerLined->value() );
  picturesConfig.writeEntry( "InOrder", orderButton->isChecked() );
  picturesConfig.writeEntry( "UseDir", dirCheckBox->isChecked() );
  picturesConfig.writeEntry( "Directory", dirLined->text() );

  hide();
  
  setResult(Accepted);

  kb->rnddlg = 0L;
  kb->random = 0;
  kb->randomSetupButton->setEnabled( true );
  kb->wpModeCombo->setEnabled( false );
}
