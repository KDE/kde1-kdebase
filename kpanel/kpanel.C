// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//

#include "kpanel.h"
#include <qapp.h>
#include <qmsgbox.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <kfm.h>
#include <qbitmap.h>

extern void execute(const char*);

DesktopEntry::DesktopEntry(){

  button    = 0;
  popup     = 0;
  pmi       = 0;
  drop_zone = 0;
  app_id    = 0;
  swallow   = "";
  swallowed = 0;
  identity  = "";
  icon[0]   = 0;
  icon[1]   = 0;
  icon[2]   = 0;
  icon[3]   = 0;
};

void kPanel::createPixmap()
{
  
  // create the stipple pixmap for the panel button
  QPixmap pm;
  QBitmap bm;
  QPainter paint;
  QPainter paint2;
  pm.resize(9, 
 	    (orientation == horizontal) 
 	    ? panel_button->height() 
 	    : panel_button->width());
  bm.resize(pm.width(), pm.height());
  bm.fill(color0);
  pm.setMask(bm);
  paint.begin(&pm);
  paint2.begin(pm.mask());
  int stipple_height = 3;
  paint.setPen( colorGroup().light() );
  paint2.setPen( color1 );
  while ( stipple_height < pm.height()-4 ) {
    paint.drawPoint( 1, stipple_height+1);
    paint.drawPoint( 4, stipple_height);
    paint2.drawPoint( 1, stipple_height+1);
    paint2.drawPoint( 4, stipple_height);
    stipple_height+=3;
  }
  paint.setPen( colorGroup().dark() );
  stipple_height = 4;
  while ( stipple_height < pm.height()-4 ) {
    paint.drawPoint( 2, stipple_height+1);
    paint.drawPoint( 5, stipple_height);
    paint2.drawPoint( 2, stipple_height+1);
    paint2.drawPoint( 5, stipple_height);
    stipple_height+=3;
  }
  
  paint.end();
  paint2.end();
  QPixmap pm2 = pm;
  arrow_on_pixmap(&pm, LeftArrow);
  arrow_on_pixmap(&pm2, RightArrow);
  
  QWMatrix m;
  m.rotate((orientation == horizontal)?0:90);
  panel_button->setPixmap(pm.xForm(m));
  panel_button_standalone->setPixmap(pm2.xForm(m));    
}    


bool kPanel::initConfigFile ( KConfig *config )
{
  if (! config)
    return FALSE;
  
  config->setGroup("kpanel");
  if (! config->hasKey("Style")) {
    
    // write default settings for normal style
    writeInitStyle (config, normal);
  }
  
  return TRUE;
}


kPanel::kPanel( KWMModuleApplication* kwmapp_arg,
		QWidget *parent, const char *name )
  : QFrame( parent, name,WStyle_Customize | WStyle_NoBorder | WStyle_Tool ){
    //: QFrame( parent, name){
    
    int i;

    initing = true;

    kwmmapp = kwmapp_arg;
    
    tab = 0;
    setFrameStyle(QFrame::Panel| QFrame::Raised);
    
    setMouseTracking(TRUE);
    
    // desktops
    tbhs = 4;
    tbmhs = 3;
    currentDesktop = KWM::currentDesktop();
    number_of_desktops = KWM::numberOfDesktops();
    if (number_of_desktops == 1)
      number_of_desktops = 0;
    
    // timer
    tipTimer = new QTimer( this );
    connect( tipTimer, SIGNAL(timeout()),
	     this, SLOT(tipTimerDone()) );
    
    tipSleepTimer = new QTimer( this );
    connect( tipSleepTimer, SIGNAL(timeout()),
  	     this, SLOT(tipSleepTimerDone()) );
    
    hideTimer = new QTimer( this );
    connect( hideTimer, SIGNAL(timeout()),
	     this, SLOT(hideTimerDone()) );
    
    // parse the configuration file
    KConfig *config = KApplication::getKApplication()->getConfig();
    // Don't write dirty entries at destruction time.
    config->rollback();

    initConfigFile (config);

    // position and orientation
    QString a   = config->readEntry("Position");
    orientation = horizontal;
    position    = bottom_right;
    
    if ( (a == "left") || (a == "right")){
      orientation = vertical;
      position = top_left;
    }
    else if ( a == "top"){
      orientation = horizontal;
      position = top_left;
    }
    else if ( a == "bottom"){
      orientation = horizontal;
      position = bottom_right;
    }
	
    // read background pixmap (if any )
    if ( config->hasKey("BackgroundTexture") ) {
      
      mBackTexture = kapp->getIconLoader()
	->loadIcon(config->readEntry( "BackgroundTexture" ) );
      if (!mBackTexture.isNull())
	setBackgroundPixmap( mBackTexture );
    }
    
    // geometry defaults
    box_width  = 47;
    box_height = 47;
    margin     = 4;
    dbhs       = 4;
    dbrows     = 2;
    
    if (config->hasKey("BoxWidth"))
      box_width = config->readNumEntry("BoxWidth");
    if (config->hasKey("BoxHeight"))
      box_height = config->readNumEntry("BoxHeight");
    if (config->hasKey("Margin"))
      margin = config->readNumEntry("Margin");
    
    if (config->hasKey("DesktopButtonHorizontalSize"))
      dbhs = config->readNumEntry("DesktopButtonHorizontalSize");
    
    if (dbhs<1) dbhs = 1;
    if (dbhs>6) dbhs = 6;
    
    if (config->hasKey("DesktopButtonRows"))
      dbrows = config->readNumEntry("DesktopButtonRows");
    
    if (dbrows<1) dbrows = 1;
    if (dbrows>2) dbrows = 2;
    
    // tool tips
    last_tip_widget         = 0;
    menu_tool_tips          = 1000;
    menu_tool_tips_disabled = False;
    
    if (config->hasKey("MenuToolTips"))
      menu_tool_tips = config->readNumEntry("MenuToolTips");
    else
      config->writeEntry("MenuToolTips", menu_tool_tips);
    
    // auto hide of panel and taskbar
    autoHide = False;
    if (config->hasKey("AutoHide"))
      autoHide = (config->readEntry("AutoHide") == "on");
    else
      config->writeEntry("AutoHide", "off");
    autoHidden = False;
    if (autoHide)
      hideTimer->start(6000, TRUE);
    
    autoHideTaskbar = false;
    if (config->hasKey("AutoHideTaskbar"))
      autoHideTaskbar = (config->readEntry("AutoHideTaskbar") == "on");
    else
      config->writeEntry("AutoHideTaskbar", "off");
    
    // time format
    clockAmPm = false;
    if (config->hasKey("ClockAmPm"))
      clockAmPm = (config->readEntry("ClockAmPm") == "on");
    else
      config->writeEntry("ClockAmPm", "off");
    
    // panel shown?
    QString panelHiddenString = "00000000";
    panelHiddenString = config->readEntry("PanelHidden", 
					  panelHiddenString);
    
    for (i=1; i<=MAX_DESKTOPS; i++)
      panelHidden[i] = (panelHiddenString.mid(i-1,1)=="1");
    
    panelCurrentlyHidden = panelHidden[currentDesktop];
    miniPanelHidden = True;
     
    // panel buttons...
    nbuttons = 0;
    moving_button = 0;
    wait_cursor_button = 0;
    
    // popup menu for any kpanel item
    popup_item = new myPopupMenu;
    CHECK_PTR( popup_item );
    popup_item->insertItem(klocale->translate("Move"));
    popup_item->insertItem(klocale->translate("Remove"));
    popup_item->insertSeparator();
    popup_item->insertItem(klocale->translate("Properties"));
    init_popup(popup_item);

    windows = new myPopupMenu;
    CHECK_PTR( windows );
    init_popup(windows);

    // window list popup
    windowlist = new myPopupMenu;
    CHECK_PTR( windowlist );
    windowlist->setCheckable(TRUE);
    connect( windowlist, SIGNAL(activated( int )), 
	     SLOT(windowlistActivated(int)) );
    init_popup(windowlist);

    QPushButton* tmp_push_button = NULL;
    QButton* tmp_button = NULL;
    int w = QApplication::desktop()->width();
    int h = QApplication::desktop()->height();

    // main menu buttons
    entries[nbuttons].button = 
      (myPushButton*) new myPushButton( this, klocale->translate("Go!"));
    entries[nbuttons].button->installEventFilter( this );
    QToolTip::add(entries[nbuttons].button, 
		  klocale->translate("Where do you want to go tomorrow?"));

    connect( entries[nbuttons].button, 
	     SIGNAL(clicked()), 
	     SLOT(button_clicked()) );
    connect( entries[nbuttons].button, 
	     SIGNAL(pressed()), 
	     SLOT(button_pressed()) );
    kde_button = entries[nbuttons].button;
    if (!mBackTexture.isNull())
      kde_button->setBackgroundPixmap( mBackTexture );

    // The control group
    control_group = new QFrame( this);
    control_group->setMouseTracking( TRUE );;
    control_group->setBackgroundColor(backgroundColor());
    control_group->installEventFilter( this );
    if ( !mBackTexture.isNull() )
      control_group->setBackgroundPixmap( mBackTexture );

    // for switching desktops
    desktopbar = new QButtonGroup(control_group);
    CHECK_PTR( desktopbar );
    desktopbar->setFrameStyle(QFrame::NoFrame);
    desktopbar->setExclusive(TRUE);
    desktopbar->setBackgroundColor(backgroundColor());
    if ( !mBackTexture.isNull() )
      desktopbar->setBackgroundPixmap( mBackTexture );
    desktopbar->installEventFilter( this );
    edit_button = NULL;

    for (i=0; i < number_of_desktops; i++){
      tmp_push_button = new QPushButton("Desktop", desktopbar);
      tmp_push_button->installEventFilter( this );
      tmp_push_button->setToggleButton(TRUE);
      tmp_push_button->setFocusPolicy(NoFocus);
    }
    tmp_push_button = (QPushButton*) desktopbar->find(currentDesktop-1);
    if (tmp_push_button)
      tmp_push_button->toggle();
    connect(desktopbar, SIGNAL(clicked(int)), SLOT(desktop_change(int)));

    // taskbar
    taskbar_frame = new myFrame(autoHideTaskbar, 0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);

    connect(taskbar_frame, SIGNAL(showMe()), SLOT(showTaskbar()));
    connect(taskbar_frame, SIGNAL(hideMe()), SLOT(hideTaskbar()));

    info_label = new QLabel(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
    info_label->setMouseTracking(TRUE);
    info_label_is_sleeping = False;
    
    do {

      QColorGroup g( black, QColor(255,255,220),
		     QColor(96,96,96), black, black,
		     black, QColor(255,255,220) );
      info_label->setPalette( QPalette( g, g, g ) );
      if ( QApplication::style() == MotifStyle )
	info_label->setFrameStyle( QFrame::Plain | QFrame::Box );
      else
	info_label->setFrameStyle( QFrame::Raised | QFrame::Panel );
      info_label->setLineWidth( 1 );
      info_label->setMargin( 3 );
      info_label->setAlignment( AlignLeft | AlignTop );
      info_label->setAutoResize( TRUE );
      info_label->setText(klocale->translate("No info available"));
    } while (0);
    
    taskbar = new QButtonGroup(taskbar_frame);
    taskbar->setFrameStyle(QFrame::Panel| QFrame::Raised);
    connect( taskbar, SIGNAL(clicked( int )), 
	     SLOT(taskbarClicked(int)) );
    connect( taskbar, SIGNAL(pressed( int )), 
	     SLOT(taskbarPressed(int)) );
    
    taskbar_position = taskbar_top_left;
    a = config->readEntry("TaskbarPosition");
    if ( a == "top"){
      taskbar_position = top;
    }
    else if (a == "bottom"){
      taskbar_position = bottom;
    }
    else if (a == "hidden"){
      taskbar_position = hidden;
    }
    else if (a == "top_left"){
      taskbar_position = taskbar_top_left;
    }
    
    if (config->hasKey("TaskbarButtonHorizontalSize"))
      tbhs = config->readNumEntry("TaskbarButtonHorizontalSize");

    if (config->hasKey("TaskbarButtonMinimumHorizontalSize"))
      tbmhs = config->readNumEntry("TaskbarButtonMinimumHorizontalSize");

    // clock and calender
    label_date = new QLabel(this);
    if (config->hasKey("DateVisible")){
      QString aString = config->readEntry("DateVisible");
      if (aString == "false"){
	label_date->hide();
      }
    }

    // Dock area
    dock_area = new QFrame(this);
    dock_area->setFrameStyle(QFrame::Panel| QFrame::Sunken);
    dock_area->hide();
    

    load_and_set_some_fonts();

    // logout
    exit_button = new QPushButton("Exit", control_group);
    exit_button->setFocusPolicy(NoFocus);
    exit_button->setMouseTracking(TRUE);
    QToolTip::add(exit_button, klocale->translate("Logout"));
    connect(exit_button, SIGNAL(clicked()), SLOT(ask_logout()));
    exit_button->installEventFilter( this );
    
    // klock
    lock_button = new QPushButton("lock", control_group);
    lock_button->setFocusPolicy(NoFocus);
    lock_button->setMouseTracking(TRUE);
    QToolTip::add(lock_button, klocale->translate("Lock screen"));
    connect(lock_button, SIGNAL(clicked()), SLOT(call_klock()));
    lock_button->installEventFilter( this );
    
    // panel button
    panel_button = new QPushButton("panel", this);
    panel_button->setFocusPolicy(NoFocus);
    panel_button->setMouseTracking(TRUE);
    QToolTip::add(panel_button, klocale->translate("Hide the panel"));
    panel_button->installEventFilter( this );

    connect( panel_button, SIGNAL(clicked()), 
	     SLOT(hidePanel()) );
    
    panel_button_frame_standalone  = 
      new QFrame(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool );
    panel_button_frame_standalone->setMouseTracking(TRUE);
    panel_button_frame_standalone->installEventFilter( this );
    panel_button_standalone  = new QPushButton("panel", panel_button_frame_standalone);
    QToolTip::add(panel_button_standalone, klocale->translate("Show the panel"));
    panel_button_standalone->setFocusPolicy(NoFocus);
    panel_button_standalone->setMouseTracking(TRUE);
    panel_button_standalone->installEventFilter( this );
    
    connect( panel_button_standalone, SIGNAL(clicked()), 
	     SLOT(standalonePanelButtonClicked()) );

    int bwidth;
    int bheight;
      
    if (orientation == horizontal) {
      
      if ( dbrows == 1) {
	
	bwidth  = box_width - 1;
	bheight = box_width - 1;
	
	exit_button->setGeometry(0, 0,
				 bwidth, bheight);
	
	lock_button->setGeometry(bwidth + 1, 0,
				 bwidth, bheight);
	
	for (i=0; i < MAX_DESKTOPS; i++) {
	  
	  // treat even and odd numbers
	  if ( ! (tmp_button = desktopbar->find(i)) ) {
	    break;
	  }
	  
	  tmp_button->setGeometry(( bwidth * dbhs + 1) * i, 0, 
				  bwidth * dbhs, bheight );
	}
	
	// the desktop buttons geometry
	desktopbar->setGeometry( 2* (bwidth + 1), 0, 
				 ( bwidth * dbhs + 1 ) * i, bheight + 1);
	
	// desktop buttons but control buttons
	control_group->setGeometry(0, margin,
				   2 * (bwidth + 1)  + desktopbar->width(), desktopbar->height());
	
      } else {
	
	bwidth  = (box_width - 1) / 2;
	bheight = (box_width - 1) / 2;
	
	exit_button->setGeometry(0, 0,
				 bwidth, bheight);
	
	lock_button->setGeometry(0, bheight + 1,
				 bwidth, bheight);
	
	int stop = FALSE;
	for (i=0; i < 4; i++) {
	  
	  for (int j=0; j < 2; j++) {
	    
	    // treat even and odd numbers
	    if ( ! (tmp_button = desktopbar->find((i * 2) + j)) ) {
	      stop = TRUE; break;
	    }
	    
	    tmp_button->setGeometry(( bwidth * dbhs + 1) * i, ( bheight + 1 ) * j, 
				    bwidth * dbhs, bheight );
	  }
	  
	  if (stop)
	    break;
	}
	
	// the desktop buttons geometry
	desktopbar->setGeometry( bwidth + 1, 0, 
				 ( bwidth * dbhs + 1 ) * i, (bheight + 1) * dbrows);
	
	// desktop buttons but control buttons
	control_group->setGeometry(0, margin,
				   bwidth + 1 + desktopbar->width(), desktopbar->height());
	
      }
      
      // panel buttons geometry
      panel_button->setGeometry(0, 0,
				12, box_height+2*margin);
      
      kde_button->setGeometry(panel_button->x()+panel_button->width()
			      + box_width,
			      margin?margin:1,
 			      box_width-(margin?0:2), box_height-(margin?0:2));
      
      bound_top_left = control_group->x();
      bound_bottom_right = control_group->x() + control_group->width();
      
      label_date->setGeometry(w - margin - 2*box_width - 3, 3,
			      2*box_width-6, box_height+2*margin-6);
      label_date->setAlignment( AlignRight|AlignVCenter );
      
      dock_area->setGeometry(label_date->x() - box_width,
			     3,
			     2 * box_width - 6,
			     box_height+2*margin-6);
      
      if (position == top_left) {
	setGeometry(0, 0, 
		    w, box_height+2*margin);
      }
      else {
	setGeometry(0, h - box_height-2*margin, 
		    w, box_height+2*margin);
      }
    }
    else { // orientation == vertical
      
      bwidth  = (box_width - 1) / 2;
      bheight = (box_width - 1) / 2;

      exit_button->setGeometry(0,0,
			       bwidth, bheight);
      
      lock_button->setGeometry(bwidth + 1, 0,
			       bwidth, bheight);
      
      for (i=0; i < MAX_DESKTOPS; i++) {

	if ( ! (tmp_button = desktopbar->find(i)) )
	  break;
	
	tmp_button->setGeometry(0, (bheight + 1) * i, 
				bwidth * 2 + 1, bheight + 1);
      }
      
      desktopbar->setGeometry(0, bheight + 1,
			      bwidth * 2 + 1, (bheight + 1) * i);
      
      control_group->setGeometry(margin, h/2 - (bheight + 1 + desktopbar->height())/2,
				 box_width, bheight + 1 + desktopbar->height());
      
      panel_button->setGeometry(0, 0,
				box_width + 2*margin, 12);
      
      kde_button->setGeometry(margin?margin:1,
 			      panel_button->y()+panel_button->height()
			      + box_height,
 			      box_width-(margin?0:2), box_height-(margin?0:2));
      
      
      bound_top_left = control_group->y();
      bound_bottom_right = control_group->y() + control_group->height();
      
      
      label_date->setGeometry(3,
			      h - margin - 3-box_height,
			      box_width+2*margin-6, 
			      box_height);
      
      label_date->setAlignment( AlignHCenter|AlignBottom);
      
      
      if (position == top_left){
	setGeometry(0, 0, 
		    box_width+2*margin, h);
      }

      else{
	setGeometry(w - box_width - 2*margin,
		    0,
		    box_width+2*margin, 
		    h);
      }

      dock_area->setGeometry(3,
			     label_date->y() - box_height,
			     box_width+2*margin-6, 
			     box_height);

    }

    
    lock_button->setPixmap(kapp->getIconLoader()
			   ->loadIcon("key.xpm"));
    exit_button->setPixmap(kapp->getIconLoader()
			   ->loadIcon("exit.xpm"));

    if (margin == 0){
      margin++;
      box_width-=2;
      box_height-=2;
    }

    kde_button->setPixmap(create_arrow_pixmap(load_pixmap("go.xpm")));

    panel_button_frame_standalone->setGeometry(x() + panel_button->x(),
					       y() + panel_button->y(),
					       panel_button->width(),
					       panel_button->height());
    panel_button_standalone->setGeometry(0,0,panel_button->width(), panel_button->height());


    taskbar_frame->hide();

    drop_zone = new KDNDDropZone(this,  
				 DndURL);
    connect(drop_zone, 
	    SIGNAL( dropAction( KDNDDropZone *) ), 
	    this, 
	    SLOT( slotDropEvent( KDNDDropZone *) ) );

    // to make the clock more precise, start the clock
    // on the next minute break
    set_label_date();
    QTime cur_time = QTime::currentTime();
    clock_timer_id = 0;
    int next_minute_break = (60 - cur_time.second()) * 1000 - cur_time.msec();
    QTimer::singleShot(next_minute_break, this, SLOT(slotUpdateClock()));

    // menus...
    foldersFirst = true;
    if (config->hasKey("FoldersFirst"))
      foldersFirst = (config->readEntry("FoldersFirst") == "on");
    else
      config->writeEntry("FoldersFirst", "on");
    
    personalFirst = false;
    if (config->hasKey("PersonalFirst"))
      personalFirst = (config->readEntry("PersonalFirst") == "on");
    else
      config->writeEntry("PersonalFirst", "off");
    
    // the main menu
    PMenu* systemmenu = new PMenu;
    systemmenu->setAltSort(foldersFirst);
    PMenu* personalmenu = new PMenu;
    personalmenu->setAltSort(foldersFirst);
    
    config->setGroup("KDE Desktop Entries");

    // read personal configuration
    QString temp = QDir::homeDirPath() +"/.kde/share/applnk";
    QString personaldir  = config->readEntry("PersonalPath", temp.data() );

    // read system configuration
    temp = KApplication::kdedir()+"/share/applnk";
    QString sysapplnkdir = config->readEntry("Path", temp.data() );

    systemmenu->parse(QDir(sysapplnkdir));
    systemmenu->setAltSort(foldersFirst);
    personalmenu->parse(QDir(personaldir));
    personalmenu->setAltSort(foldersFirst);

    primary_menu = systemmenu;
    secondary_menu = 0;

    personalmenu->createMenu(new myPopupMenu, this);

    if ( personalmenu->getQPopupMenu() && 
	 personalmenu->getQPopupMenu()->count()>0 ) {
      if ( personalFirst ) {
	
	primary_menu = personalmenu;
	secondary_menu = systemmenu;
	
      } else {

	secondary_menu = personalmenu;
      }
    }

    if ( secondary_menu ) {
      
      PMenuItem* additem = new PMenuItem ;
      QFileInfo fi(personaldir);
      additem->parse(&fi, secondary_menu);
      primary_menu->add( new PMenuItem((EntryType) separator) );
      primary_menu->add( additem );
    }

    primary_menu->add( new PMenuItem(separator) );
    addapp_menu = new PMenu(*primary_menu);

    PMenu *panel_menu = new PMenu;
    panel_menu->
      add(new PMenuItem(add_but, 
			klocale->translate("Add application"), 
			0, 0, addapp_menu,
			0, 0, new myPopupMenu, FALSE, 0, 
			klocale->translate("Add an application or a submenu onto the panel")));
    panel_menu->
      add( new PMenuItem(prog_com, klocale->translate("Add windowlist"), 
			 0, 0, 0, 
			 this, SLOT(add_windowlist()), 0, FALSE, 0, 
			 klocale->translate("Add a windowlist menu onto the panel")) );
    panel_menu->
      add( new PMenuItem(prog_com, klocale->translate("Configure"), 
			 0, 0, 0, 
			 this, SLOT(configure_panel()), 0, FALSE, 0, 
			 klocale->translate("Configure panel")) );
    panel_menu->
      add( new PMenuItem(prog_com, klocale->translate("Restart"), 
			 0, 0, 0, 
			 this, SLOT(restart()), 0, FALSE, 0, 
			 klocale->translate("Restart panel")) );
    
    primary_menu->
      add( new PMenuItem(submenu, klocale->translate("Panel"), 
			 0, 0, panel_menu,
			 0, 0, new myPopupMenu) );
    primary_menu->
      add( new PMenuItem(prog_com, klocale->translate("Lock Screen"), 
			 0, 0, 0,
			 this, SLOT(call_klock()), 0, FALSE, 0, 
			 klocale->translate("Lock screen")) );
    primary_menu->
      add( new PMenuItem(prog_com, klocale->translate("Logout"), 
			 0, 0, 0,
			 this, SLOT(ask_logout()), 0, FALSE, 0, 
			 klocale->translate("Logout")) );

    primary_menu->createMenu(new myPopupMenu, this);
    entries[nbuttons++].popup = primary_menu->getQPopupMenu();

    read_in_configuration();
    KApplication::getKApplication()->getConfig()->sync();
    doGeometry();
    if (panelHiddenString=="00000000"){
      for (i = 1; i <= MAX_DESKTOPS; i++)
	KWM::setWindowRegion(i, KWM::getWindowRegion(currentDesktop));
    }
    layoutTaskbar();
    for (i=0; i < number_of_desktops; i++){
      set_button_text(desktopbar->find(i),
		      KWM::getDesktopName(i+1));
    }
    
    createPixmap ();

    initing = false;
}

void kPanel::restart(){
  cleanup();
  restart_the_panel();
}

void kPanel::desktop_change(int new_desk) {
  new_desk++;
  if ( new_desk == currentDesktop) {
    QPushButton *tmp_button = (QPushButton*)desktopbar->find(new_desk-1);
    edit_button = new QLineEdit(desktopbar,"edit_button");
    edit_button->setGeometry(tmp_button->geometry());
    edit_button->setText(KWM::getDesktopName(new_desk));
    tmp_button->hide();
    hidden_button = tmp_button;
    edit_button->show();
    edit_button->grabKeyboard();
    edit_button->grabMouse();
    edit_button->setFocus();
    edit_button->installEventFilter(this);
    QColorGroup g = tmp_button->palette().active(); 
    g = QColorGroup( g.foreground(), 
		     g.background(),
		     g.light(),
		     g.dark(),
		     g.mid(),
		     g.text(),
		     g.background());
    edit_button->setPalette( QPalette( g, g, g) );
  } else {
    if ( edit_button != NULL)
      restore_editbutton( False );
    currentDesktop = new_desk;
    KWM::switchToDesktop(currentDesktop);  
  }
};

void kPanel::restore_editbutton( bool takeit ) {
  if ( takeit ) {
    KWM::setDesktopName(currentDesktop, edit_button->text());
  }
  if (edit_button != NULL) {
    edit_button->releaseKeyboard();
    edit_button->releaseMouse();
    edit_button->hide();
    delete edit_button;
    hidden_button->show();
    hidden_button->setOn(true);
    edit_button = NULL; 
  };
};



void kPanel::button_clicked(){
  int i;
  for (i=0; i<nbuttons && entries[i].button != myPushButton::most_recent_pressed;i++);
  if (i<nbuttons){
    if (entries[i].button->last_button == LeftButton && 
	entries[i].pmi && entries[i].pmi->getType() == unix_com){
      entries[i].pmi->exec();
      //  	// visual feedback to avoid doubleclicks
      //       if (wait_cursor_button)
      // 	wait_cursor_button->setCursor(arrowCursor);
      //       wait_cursor_button = active_button;
      //       wait_cursor_button->setCursor(waitCursor);
    }	
  }
}

void kPanel::button_pressed(){
  
  int i;
  for (i=0; i<nbuttons && entries[i].button != myPushButton::most_recent_pressed;i++);
  if (i<nbuttons){
    if (entries[i].button->last_button == LeftButton && entries[i].popup){
      show_popup(entries[i].popup, entries[i].button);
    }
    else if (entries[i].button->last_button == LeftButton && entries[i].pmi && entries[i].pmi->getQPopupMenu()){
      show_popup(entries[i].pmi->getQPopupMenu(), entries[i].button);
    }
    else if (entries[i].button->last_button == RightButton){
      button_to_be_modified = entries[i].button;
      
      popup_item->setItemEnabled(0, TRUE);
      popup_item->setItemEnabled(1, TRUE);
      popup_item->setItemEnabled(3, TRUE);
      
      if (button_to_be_modified == kde_button){
	popup_item->setItemEnabled(0, TRUE);
	popup_item->setItemEnabled(1, FALSE);
	popup_item->setItemEnabled(3, FALSE);
      }
      if (entries[i].popup == windowlist){
	popup_item->setItemEnabled(0, TRUE);
	popup_item->setItemEnabled(1, TRUE);
	popup_item->setItemEnabled(3, FALSE);
      }
      
      switch (show_popup(popup_item, entries[i].button)){
      case 3:  
	if (entries[i].pmi){
	  KFM* kfm = new KFM;
	  QString a = entries[i].pmi->fullPathName().copy();
	  a.prepend("file:");
	  if (entries[i].pmi->getType() == submenu)
	    a.append("/.directory");
	  kfm->openProperties(a);
	  delete kfm;
	}
	break;
      case 0: //move
	{
	  moving_button = button_to_be_modified;
	  moving_button_offset = QPoint(moving_button->width()/2,
					moving_button->height()/2);
	  if (moving_button != control_group){
	    ((myPushButton*)moving_button)->flat = False;
	    moving_button->repaint();
	  }
	  moving_button->raise();
	  moving_button->setCursor(sizeAllCursor);
	  // the next line _IS_ necessary! 
	  XGrabPointer( qt_xdisplay(), moving_button->winId(), FALSE,
			ButtonPressMask | ButtonReleaseMask |
			PointerMotionMask | EnterWindowMask | LeaveWindowMask,
			GrabModeAsync, GrabModeAsync,
			None, None, CurrentTime );
	  //moving_button->grabMouse();
	  return;
	  
	}
	
	break;
      case 1: // remove
	{
	  if (entries[i].swallowed)
	    KWM::close(entries[i].swallowed);
	  delete_button(button_to_be_modified);
	  button_to_be_modified = NULL;
	  write_out_configuration();
	}
	break;
      default:
	break;
      }
    }
  }
}


void kPanel::taskbarClicked(int item){
  myTaskButton* b = (myTaskButton*) taskbar->find(item);
  if (!b)
    return;
  
  switch (b->last_button){
  case MidButton:
    KWM::setIconify(b->win, !KWM::isIconified(b->win));
    break;
  case RightButton:
    
    break;
  default:
    KWM::activate(b->win);
    break;
  }
}

void kPanel::taskbarPressed(int item){
  myTaskButton* b = (myTaskButton*) taskbar->find(item);
  if (!b)
    return;
  if (b->last_button != RightButton)
    return;
  
  QPopupMenu pop;
  pop.setMouseTracking(TRUE);

  if (KWM::isMaximized(b->win) && !KWM::fixedSize(b->win))
    pop.insertItem(KWM::getUnMaximizeString(), 
		   OP_RESTORE);
  else
    pop.insertItem(KWM::getMaximizeString(), 
		   OP_MAXIMIZE);
  
  if (KWM::fixedSize(b->win))
    pop.setItemEnabled(OP_MAXIMIZE, FALSE);
  
  if (KWM::isIconified(b->win))
    pop.insertItem(KWM::getUnIconifyString(), 
		   OP_UNICONIFY);
  else
    pop.insertItem(KWM::getIconifyString(), 
		   OP_ICONIFY);
  
  if (KWM::isSticky(b->win))
    pop.insertItem(KWM::getUnStickyString(), 
		   OP_STICKY);
  else
    pop.insertItem(KWM::getStickyString(), 
		   OP_STICKY);
  
  
  pop.insertItem(KWM::getOntoCurrentDesktopString(), 
		 OP_ONTO_CURRENT_DESKTOP);
  if (b->virtual_desktop == KWM::currentDesktop())
    pop.setItemEnabled(OP_ONTO_CURRENT_DESKTOP, FALSE);
    
  pop.insertSeparator();
  pop.insertItem(KWM::getCloseString(), 
		 OP_CLOSE);
  

  switch (show_popup(&pop, b, TRUE)){
  case OP_MAXIMIZE:
    KWM::setMaximize(b->win, TRUE);
    break;
  case OP_RESTORE:
    KWM::setMaximize(b->win, FALSE);
    break;
  case OP_ICONIFY:
    KWM::setIconify(b->win, TRUE);
    break;
  case OP_UNICONIFY:
    KWM::setIconify(b->win, FALSE);
    break;
  case OP_CLOSE:
    KWM::close(b->win);
  case OP_STICKY:
    KWM::setSticky(b->win, !KWM::isSticky(b->win));
    break;
  case OP_ONTO_CURRENT_DESKTOP:
    KWM::moveToDesktop(b->win, KWM::currentDesktop());
    break;
  default:
    break;
  }
}



void kPanel::addButtonInternal(PMenuItem* pmi, int x, int y, QString name){
  QPixmap pm;
  entries[nbuttons++].button = new myPushButton( this); 
  if (!mBackTexture.isNull())
    entries[nbuttons-1].button->setBackgroundPixmap( mBackTexture );
  
  entries[nbuttons-1].button->installEventFilter( this );
  entries[nbuttons-1].popup     = 0;
  entries[nbuttons-1].pmi       = pmi;
  entries[nbuttons-1].swallow   = "";
  entries[nbuttons-1].swallow   = "";
  entries[nbuttons-1].swallowed = 0;
  entries[nbuttons-1].identity  = "";
  entries[nbuttons-1].icon[0]   = 0;
  entries[nbuttons-1].icon[1]   = 0;
  entries[nbuttons-1].icon[2]   = 0;
  entries[nbuttons-1].icon[3]   = 0;
  entries[nbuttons-1].drop_zone = 0;
  
  connect( entries[nbuttons-1].button, SIGNAL(clicked()), 
	   SLOT(button_clicked()) );
  connect( entries[nbuttons-1].button, SIGNAL(pressed()), 
	   SLOT(button_pressed()) );
  
  if (pmi){
    if (pmi->getType() == submenu){
      
      // the next paragraph is a workaound for qt-1.3,
      // since Qt has problems with standalone submenus
      // of other menus :-(
      
      PMenu* pm = new PMenu;
      pm->setAltSort(foldersFirst);
      pm->parse(QDir(pmi->fullPathName()));
      PMenuItem* pmi2 = new PMenuItem;
      QFileInfo fi(pmi->fullPathName());
      pmi2->parse(&fi, pm);
      pmi = pmi2;
      pm->createMenu(pmi->getQPopupMenu(), this);
      entries[nbuttons-1].pmi = pmi;
      // end workaround
      
      entries[nbuttons-1].button->
	setPixmap(create_arrow_pixmap( load_pixmap(pmi->bigIconName(), True)));
    }
    else{
      entries[nbuttons-1].button->setPixmap(load_pixmap(pmi->bigIconName()));
      
      QFile myfile(pmi->fullPathName());
      if (myfile.exists()){
	if (myfile.open ( IO_ReadOnly )){
	  // kalle	   QTextStream mystream(&myfile);
	  
	  myfile.close(); // kalle
	  KConfig pConfig(pmi->fullPathName() );
	  pConfig.setGroup("KDE Desktop Entry");
	  QString aString;
	  if (pConfig.hasKey("SwallowTitle")){
	    entries[nbuttons-1].swallow = QString(pConfig.readEntry("SwallowTitle")).copy();
	    if (!entries[nbuttons-1].swallow.isEmpty() &&
		pConfig.hasKey("SwallowExec")){
	      KWM::doNotManage(entries[nbuttons-1].swallow);
	      aString = QString(pConfig.readEntry("SwallowExec")).copy();
	      // 	     printf("execute %s\n", aString.data());
	      if (!initing) 
		execute(aString.data());
	      else
		swallowed_applications.append(aString);
	    }
	  }
	  if (pConfig.hasKey("PanelIdentity")){
	    entries[nbuttons-1].icon[0] = new QPixmap();
	    *(entries[nbuttons-1].icon[0]) = *entries[nbuttons-1].button->pixmap();
	    entries[nbuttons-1].identity = QString(pConfig.readEntry("PanelIdentity")).copy();
	    if (pConfig.hasKey("Icon2")){
	      entries[nbuttons-1].icon[1] = new QPixmap();
	      *(entries[nbuttons-1].icon[1]) = load_pixmap(pConfig.readEntry("Icon2"));
	    }
	    if (pConfig.hasKey("Icon3")){
	      entries[nbuttons-1].icon[2] = new QPixmap();
	      *(entries[nbuttons-1].icon[2]) = load_pixmap(pConfig.readEntry("Icon3"));
	    }
	    if (pConfig.hasKey("Icon4")){
	      entries[nbuttons-1].icon[3] = new QPixmap();
	      *(entries[nbuttons-1].icon[3]) = load_pixmap(pConfig.readEntry("Icon4"));
	    }
	  }
	}
      }
      
      entries[nbuttons-1].drop_zone = new KDNDDropZone(entries[nbuttons-1].button, 
						       DndURL);
      connect(entries[nbuttons-1].drop_zone, 
	      SIGNAL( dropAction( KDNDDropZone *) ), 
	      this, 
	      SLOT( slotDropEvent( KDNDDropZone *) ) );
      
    }
    if (pmi->getComment().isEmpty())
      QToolTip::add(entries[nbuttons-1].button,
		    klocale->translate("No comment available"));
    else
      QToolTip::add(entries[nbuttons-1].button, pmi->getComment());
    
  }
  else {
    if (name == "windowlist"){
      entries[nbuttons-1].popup = windowlist;
      entries[nbuttons-1].button->setPixmap(create_arrow_pixmap(load_pixmap("window_list.xpm")));
      QToolTip::add(entries[nbuttons-1].button, klocale->translate("Windowlist"));
    }
  }
  
  
  if (x != -1 || y != -1)
    entries[nbuttons-1].button->setGeometry(x, y, 
					    box_width, box_height);
  else
    find_a_free_place();
  
  entries[nbuttons-1].button->show();
  check_button_bounds(entries[nbuttons-1].button);
  reposition();
}


void kPanel::launchSwallowedApplications(){
  char* s;
  for (s = swallowed_applications.first(); s;
       s = swallowed_applications.next())
    execute(s);
}

void kPanel::show(){
  if (!panelCurrentlyHidden) {
     QFrame::show();
     QFrame::raise();
    }
  else {
    panel_button_frame_standalone->show();
    panel_button_frame_standalone->raise();
    showMiniPanel();
  }

  if (taskbar_position != hidden)
   {
    taskbar_frame->show();
    taskbar_frame->raise();
   }
}

void kPanel::hidePanel(){
  
  Bool old = panelHidden[currentDesktop];
  
  panelHidden[currentDesktop] = True;
  
  if (! panelCurrentlyHidden){
    // where are we now?
    QPoint p = pos();
    if (autoHidden){
      
      if (orientation == horizontal){
	if (position == top_left)
	  p.setY(y()+height()-4);
	else
	  p.setY(y()-height()+4);
      }
      else {
	if (position == top_left)
	  p.setX(x()+width()-4);
	else
	  p.setX(x()-width()+4);
      } 
    }  
    
    panel_button_frame_standalone->setGeometry(p.x() + panel_button->x(),
					       p.y() + panel_button->y(),
					       panel_button->width(),
					       panel_button->height());
    panelCurrentlyHidden = True;
    QFrame::hide();
    showMiniPanel();
    panel_button_frame_standalone->show();
    panel_button_frame_standalone->raise();
    
    doGeometry();
    layoutTaskbar (); //geometry changed 
  }

  if (old != panelHidden[currentDesktop]){
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup("kpanel");   
    QString a;
    int i;
    for (i=1;i<=MAX_DESKTOPS;i++)
      a.append(panelHidden[i]?"1":"0");
    config->writeEntry("PanelHidden", a);
    config->sync();
  }
}

void kPanel::showPanel(){
  
  Bool old = panelHidden[currentDesktop];
  
  panel_button_frame_standalone->hide();
  
  panelHidden[currentDesktop] = False;
  if (panelCurrentlyHidden){
    panelCurrentlyHidden = False;
    hideMiniPanel();
    show();
    raise();
  }

  doGeometry();
  layoutTaskbar(); 

  if (old != panelHidden[currentDesktop]){
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup("kpanel");   
    QString a;
    int i;
    for (i=1;i<=MAX_DESKTOPS;i++)
      a.append(panelHidden[i]?"1":"0");
    config->writeEntry("PanelHidden", a);
    config->sync();
  }
}

void kPanel::doGeometry () {

   int w = QApplication::desktop()->width();
   int h = QApplication::desktop()->height();
   int pw = width()+1; // panel...
   int ph = height()+1;
   int px = x();
   int py = y();
   int mw = 0;       // mini panel...
   int mh = 0;
   int sw = 0;      // standalone...
   int sh = 0; 

   int tfx = 0; //correction values for the taskbar autoHide feature
   int tfy = 0;

   int th = taskbar_height * numberOfTaskbarRows();

   int tbw = taskbar_frame->width();

   if (px<0){
     pw += px;
     px = 0;
   }
   if (py<0){
     ph += py;
     py = 0;
   }
   if (px + pw >w){
     pw = w-px;
   }
   if (py + ph >h){
     ph = h-py+1;
   }


   if (taskbar_frame->autoHidden){
     if (taskbar_position == top){
       tfy = -taskbar_frame->height()+4;
     }
     else if (taskbar_position == bottom){
       tfy = taskbar_frame->height()-4;
     }
     else { //taskbar_top_left
	 tfx = -tbhs*taskbar_height+4;
     }
   }

   if (panelCurrentlyHidden) // i.e. standalone shown
    {
     pw = 0;
     ph = 0;
     px = 0;
     py = 0;
     sw = panel_button_frame_standalone->width()+1;
     sh = panel_button_frame_standalone->height()+1;
     {
       mw = 2*taskbar_height+1; // miniPanelFrame
       mh = taskbar_height+1; // miniPanelFrame
     }    
    }
   
   // panel_button_frame_standalone->setGeometry(px, py, sw, sh);
   
   if (orientation == horizontal)
    {
     if (position == top_left)
      {
       if (taskbar_position == top) 
	{
	 taskbar_frame->setGeometry(tfx+px+sw+mw, tfy+py+ph, w-mw-sw, taskbar_height);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(0, tfy+py+ph+taskbar_height+1,
				    w, -tfy+h-ph-taskbar_height-1));
	}
       else if (taskbar_position == bottom)
	{
	 taskbar_frame->setGeometry(tfx+px+mw, tfy+h-th, 
				    w-mw, th);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(0, ph, w, tfy+h-ph-th-1));
	}
       else
	{
// 	 taskbar_frame->setGeometry(tfx+px+sw, tfy+py+ph+mh,
// 				    tbhs*th, th);
	 taskbar_frame->move(tfx+px+sw, tfy+py+ph+mh);
	 if (taskbar_position == taskbar_top_left)
	   KWM::setWindowRegion(currentDesktop, 
				QRect(tfx+tbhs*th+1+sw, ph,
				      -tfx+w-tbhs*th-sw-1, h-ph));
	 else // if (taskbar_position=hidden)
	   KWM::setWindowRegion(currentDesktop, 
				QRect(0, ph, w, h-ph));
	}
      }
     else // (if position == bottom_right) 
      {
       if (taskbar_position == bottom)
	{
	 taskbar_frame->setGeometry(tfx+px+mw+sw, tfy+h-ph-th, 
				    w-mw-sw, th);
	   KWM::setWindowRegion(currentDesktop, 
				QRect(0, 0, w, tfy+h-ph-th-1));
	}
       else if (taskbar_position == top)
	{
	 taskbar_frame->setGeometry(tfx+px+mw, tfy+0, w-mw, th);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(0, tfy+th+1, w, -tfy+h-ph-th-1));
	}
       else
	{
// 	 taskbar_frame->setGeometry(tfx+0, tfy+mh,tbhs*taskbar_height, 
// 				    taskbar_height);
	 taskbar_frame->move(tfx+0, tfy+mh);
	 if (taskbar_position == taskbar_top_left)
	   KWM::setWindowRegion(currentDesktop, 
				QRect(tfx+tbhs*taskbar_height+1, 0,
				      -tfx+w-tbhs*taskbar_height-1, h-ph));
	 else
	   KWM::setWindowRegion(currentDesktop, 
				QRect(0, 0, w, h-ph));
	}
      }
    }
   else // if (orientation == vertical)
    {
     if (position == top_left)
      {
       if (taskbar_position == top)
	{
	 taskbar_frame->setGeometry(tfx+px+pw+mw+sw, tfy+py, w-pw-mw-sw, th);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(pw, tfy+taskbar_frame->height()+1,
				    w-pw, -tfy+h-taskbar_frame->height()-1));
	}
       else if (taskbar_position == bottom)
	{
	 taskbar_frame->setGeometry(tfx+px+pw+mw, tfy+h-th, 
				    w-pw-mw, th);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(pw, 0, w-pw, tfy+h - taskbar_frame->height()-1));
	}
       else
	{
// 	 taskbar_frame->setGeometry(tfx+px+pw, tfy+py+mh,
// 				    tbhs*taskbar_height, taskbar_height);
	 taskbar_frame->move(tfx+px+pw, tfy+py+mh);
	 if (taskbar_position == taskbar_top_left)
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(tfx+pw+tbhs*taskbar_height+1, 0,
 			      -tfx+w-pw-tbhs*taskbar_height-1, h));
	 else
	 KWM::setWindowRegion(currentDesktop, 
 			      QRect(pw, 0,
 			      w-pw, h));
	}
      }
     else // if (position == bottom_right)
      {
       if (taskbar_position == top)
	{
	 taskbar_frame->setGeometry(tfx+mw, tfy+0, w-pw-mw-sw, th);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(0, tfy+taskbar_frame->height()+1,
				    w-pw, -tfy+h-taskbar_frame->height()-1));
	}
       else if (taskbar_position == bottom)
	{
	 taskbar_frame->setGeometry(tfx+mw, tfy+h-th, 
				    w-pw-mw, th);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(0, 0,
				    w-pw, tfy+h-taskbar_frame->height()-1));
	}
       else
	{
// 	 taskbar_frame->setGeometry(tfx+0, tfy+mh,
// 				    tbhs*th, taskbar_height);
	 taskbar_frame->move(tfx+0, tfy+mh);
	 if (taskbar_position == taskbar_top_left)
	 KWM::setWindowRegion(currentDesktop, 
 			      QRect(tfx+tbhs*taskbar_height+1, 0,
				    -tfx+w-tbhs*taskbar_height-1-pw, h));
	 else
	 KWM::setWindowRegion(currentDesktop, 
 			      QRect(0, 0,
				    w-pw, h));
	}
      }
    }
   taskbar->resize(taskbar_frame->width(), taskbar_frame->height());
   if (taskbar_frame->width() != tbw){
     doGeometry();
   }
}
 


void kPanel::load_and_set_some_fonts(){
  int i;
  QButton* tmp_button;

  KConfig *config = KApplication::getKApplication()->getConfig();

  config->setGroup("kpanel");

//   if (config->hasKey("MenuFont")){
//     QFont tmpfont;
//     tmpfont.setRawMode(TRUE);
//     tmpfont.setFamily(config->readEntry("MenuFont"));
//     KApplication::setFont(tmpfont, True);
//   }
  
  if (config->hasKey("DateFont")){
    QFont tmpfont;
    tmpfont.setRawMode(TRUE);
    tmpfont.setFamily(config->readEntry("DateFont"));
    label_date->setFont(tmpfont);
  }
  
  if (config->hasKey("DesktopButtonFont")){
    QFont tmpfont;
    tmpfont.setRawMode(TRUE);
    tmpfont.setFamily(config->readEntry("DesktopButtonFont"));
    for (i=0; (tmp_button = desktopbar->find(i)); i++){
      tmp_button->setFont(tmpfont);
    }
  }

  taskbar_height = taskbar->fontMetrics().height()+10;

}

void kPanel::slotUpdateClock() {
  set_label_date();
  
  // if this is the first time this function is called, install a timer
  if(clock_timer_id == 0) {
    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()),
	    this, SLOT(slotUpdateClock()));
    clock_timer_id = t->start(60000);
  }
}
