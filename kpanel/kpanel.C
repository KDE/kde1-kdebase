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
#include <math.h>
#include <kfm.h>
#include <qbitmap.h>
#include <ksimpleconfig.h>

#define DEFAULT_BOX_WIDTH 45

/* 1 is the initial speed, hide_show_animation is the top speed. */
#define PANEL_SPEED(x, c) (int)((1.0-2.0*fabs((x)-(c)/2.0)/c)*hide_show_animation+1.0)

extern void execute(const char*);

DesktopEntry::DesktopEntry(){
  button = 0;
  popup = 0;
  pmi = 0;
  drop_zone = 0;
  app_id = 0;
  swallow="";
  swallowed = 0;
  identity = "";
  icon[0] = 0;
  icon[1] = 0;
  icon[2] = 0;
  icon[3] = 0;
};


kPanel::kPanel( KWMModuleApplication* kwmapp_arg,
		QWidget *parent, const char *name )
  : QFrame( parent, name,WStyle_Customize | WStyle_NoBorder | WStyle_Tool ){
    //: QFrame( parent, name){

    initing = true;

    kwmmapp = kwmapp_arg;
    int i;

    tab = 0;

    last_tip_widget = 0;

    setFrameStyle(QFrame::Panel| QFrame::Raised);

    setMouseTracking(true);

    orientation = horizontal;
    position = bottom_right;

    currentDesktop = KWM::currentDesktop();

    defaultPixmap = KApplication::getKApplication()->getIconLoader()->loadApplicationMiniIcon("mini-default.xpm", 16, 16);


    box_width = 47;
    box_height = 47;
    margin = 4;
    dbhs = 4;
    dbrows = 2;

    menu_tool_tips = 1000;
    menu_tool_tips_disabled = false;

    has_kdisknav_button = false;
    has_windowlist_button = false;

    tbhs = 4;
    tbmhs = 3;
    number_of_desktops = KWM::numberOfDesktops();
    if (number_of_desktops == 1)
       number_of_desktops = 0;

    tipTimer = new QTimer( this );
    connect( tipTimer, SIGNAL(timeout()),
	     this, SLOT(tipTimerDone()) );
    tipSleepTimer = new QTimer( this );
    connect( tipSleepTimer, SIGNAL(timeout()),
	     this, SLOT(tipSleepTimerDone()) );

    hideTimer = new QTimer( this );
    connect( hideTimer, SIGNAL(timeout()),
	     this, SLOT(hideTimerDone()) );

    // parse the configuration
    KConfig *config = KApplication::getKApplication()->getConfig();
    // Don't write dirty entries at destruction time.
    config->rollback();

    config->setGroup("kpanel");
    if (!config->hasKey("Style")) {
      config->writeEntry("Style", "normal");
      config->writeEntry("BoxWidth", DEFAULT_BOX_WIDTH);
      config->writeEntry("BoxHeight", DEFAULT_BOX_WIDTH);
      config->writeEntry("Margin",0);
      config->writeEntry("TaskbarButtonHorizontalSize",4);
      config->writeEntry("TaskbarButtonMinimumHorizontalSize",3);
      config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
      config->writeEntry("DesktopButtonRows",2);
      config->writeEntry("DateFont","*-times-medium-i-normal--12-*");
    }

    QString a = config->readEntry("Position");

    if ( a == "left"){
      orientation = vertical;
      position = top_left;
    }
    else if ( a == "right"){
      // no longer valid! Do left instead
      orientation = vertical;
      //position = top_left;
      position = bottom_right;

    }
    else if ( a == "top"){
      orientation = horizontal;
      position = top_left;
    }
    else if ( a == "bottom"){
      orientation = horizontal;
      position = bottom_right;
    }
	
    // Read in our background pixmap (if any )
    if ( config->hasKey("BackgroundTexture") )
      {
	mBackTexture = kapp->getIconLoader()
	  ->loadIcon(config->readEntry( "BackgroundTexture" ) );
	if (!mBackTexture.isNull()) {
	  //CT 15Jan1998 - little trick (rotate pixmap) to make vertical kpanel
	  // beautier -  thanks to Stefan Taferner for the idea
	  if (orientation == vertical) {
	    QWMatrix tmp_mtx;
	    tmp_mtx.rotate(90);
	    mBackTexture = mBackTexture.xForm(tmp_mtx);
	  }
	  //CT
	  setBackgroundPixmap( mBackTexture );
	}
      }

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



    if (config->hasKey("MenuToolTips"))
      menu_tool_tips = config->readNumEntry("MenuToolTips");
    else
      config->writeEntry("MenuToolTips", menu_tool_tips);


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

    //panel: autoHide, speed, delay
    autoHide = false;
    if (config->hasKey("AutoHide"))
      autoHide = (config->readEntry("AutoHide") == "on");
    else
      config->writeEntry("AutoHide", "off");

    //CT 16Oct1998 delay
    if (config->hasKey("AutoHideDelay")) {
      autoHideDelay = config->readNumEntry("AutoHideDelay");
      if (autoHideDelay < 10) autoHideDelay =  10;
      if (autoHideDelay > 10000) autoHideDelay = 10000;
    }
    else
      config->writeEntry ("AutoHideDelay", autoHideDelay=6000);
    //CT

    autoHidden = False;
    if (autoHide)
      hideTimer->start(autoHideDelay+2000, true);

    if (config->hasKey("AutoHideSpeed")) {
      autoHideSpeed = config->readNumEntry("AutoHideSpeed");
      if (autoHideSpeed < 1) autoHideSpeed = 1;//CT
    }
    else
      config->writeEntry("AutoHideSpeed", autoHideSpeed=4);

    //taskbar: autoHide, speed, delay
    autoHideTaskbar = false;
    if (config->hasKey("AutoHideTaskbar"))
      autoHideTaskbar = (config->readEntry("AutoHideTaskbar") == "on");
    else
      config->writeEntry("AutoHideTaskbar", "off");

    //CT 16Oct1998 delay
    if (config->hasKey("AutoHideTaskbarDelay")) {
      autoHideTaskbarDelay = config->readNumEntry("AutoHideTaskbarDelay");
      if (autoHideTaskbarDelay < 10) autoHideTaskbarDelay =  10;
      if (autoHideTaskbarDelay >10000) autoHideTaskbarDelay = 10000;
    }
    else
      config->writeEntry ("AutoHideTaskbarDelay", autoHideTaskbarDelay=6000);

    if (config->hasKey("AutoHideTaskbarSpeed")) {
      autoHideTaskbarSpeed = config->readNumEntry("AutoHideTaskbarSpeed");
      if (autoHideTaskbarSpeed < 1) autoHideTaskbarSpeed = 1;//CT
    }
    else
      config->writeEntry("AutoHideTaskbarSpeed", autoHideTaskbarSpeed=4);


    clockAmPm = false;
    if (config->hasKey("ClockAmPm"))
      clockAmPm = (config->readEntry("ClockAmPm") == "on");
    else
      config->writeEntry("ClockAmPm", "off");

    clockBeats = false;
    if( config->hasKey("ClockBeats"))
      clockBeats = (config->readEntry("ClockBeats") == "on");
    else
      config->writeEntry("ClockBeats", "off" );

    QString panelHiddenString = "00000000";
    panelHiddenString = config->readEntry("PanelHidden",
					  panelHiddenString);

    for (i=1;i<=8;i++)
      panelHidden[i] = (panelHiddenString.mid(i-1,1)=="1");

    QString panelHiddenLeftString = "11111111";
    panelHiddenLeftString = config->readEntry("PanelHiddenLeft",
					  panelHiddenLeftString);

    for (i=1;i<=8;i++)
      panelHiddenLeft[i] = (panelHiddenLeftString.mid(i-1,1)=="1");
    panelCurrentlyHidden = panelHidden[currentDesktop];
    panelCurrentlyLeft = panelHiddenLeft[currentDesktop];
    miniPanelHidden = True;

    /* Let's read the top speed here:  */
    /*CT verify a bit ( negative values were allowed rendering edge
      buttons unuseful */
    if (config->hasKey("HideShowAnimation")) {
      hide_show_animation = config->readNumEntry("HideShowAnimation");
      if (hide_show_animation <  0) hide_show_animation =   0;
      if (hide_show_animation >200) hide_show_animation = 200;
    }
    else
      config->writeEntry("HideShowAnimation",hide_show_animation=50);
    //CT

    nbuttons = 0;
    moving_button = 0;
    wait_cursor_button = 0;


    popup_item = new myPopupMenu;
    CHECK_PTR(popup_item);
    popup_item->insertItem(i18n("&Move"));
    popup_item->insertItem(i18n("&Remove"));
    popup_item->insertSeparator();
    popup_item->insertItem(i18n("&Properties"));
    init_popup(popup_item);

    windows = new myPopupMenu;
    CHECK_PTR( windows );
    init_popup(windows);

    windowlist = new myPopupMenu;
    CHECK_PTR( windowlist );
    windowlist->setCheckable(true);
    connect( windowlist, SIGNAL(activated( int )),
	     SLOT(windowlistActivated(int)) );
    init_popup(windowlist);
// --sven: kdisknav button start --
    kdisknav = new myPopupMenu;
    CHECK_PTR( kdisknav );
    windowlist->setCheckable(true);
    //init_popup(kdisknav); Not needed? Guess so...
// --sven: kdisknav button end --

    QPushButton* tmp_push_button = 0;
    QButton* tmp_button = 0;
    int w = QApplication::desktop()->width();
    int h = QApplication::desktop()->height();


    label_date = new QLabel(this);
    bool label_date_visible = true;
    if (config->hasKey("DateVisible")){
      QString aString = config->readEntry("DateVisible");
      if (aString == "false"){
	label_date_visible = false;
	label_date->hide();
      }
    }
    label_date->setText("KDE 88");
    label_date->adjustSize();

    dock_area = new QFrame(this);
    dock_area->setFrameStyle(QFrame::Panel| QFrame::Sunken);
    dock_area->hide();

    // Note: nbuttons _must_ be 0 here!
    entries[nbuttons++].button = (myPushButton*) new myPushButton( this, klocale->translate("Go!"));
    entries[nbuttons-1].button->setCursor(waitCursor);
    entries[nbuttons-1].button->installEventFilter( this );
    QToolTip::add(entries[nbuttons-1].button, klocale->translate("Where do you want to go tomorrow?"));

    connect( entries[nbuttons-1].button, SIGNAL(clicked()),
	     SLOT(button_clicked()) );
    connect( entries[nbuttons-1].button, SIGNAL(pressed()),
	     SLOT(button_pressed()) );
    kde_button = entries[nbuttons-1].button;
    kmenu = 0;
     if (!mBackTexture.isNull())
       kde_button->setBackgroundPixmap( mBackTexture );


    // The control group
    control_group = new QFrame( this);
    control_group->setMouseTracking( true );;
    control_group->setBackgroundColor(backgroundColor());
    control_group->installEventFilter( this );
    if ( !mBackTexture.isNull() )
      control_group->setBackgroundPixmap( mBackTexture );

    desktopbar = new QButtonGroup(control_group);
    CHECK_PTR( desktopbar );
    desktopbar->setFrameStyle(QFrame::NoFrame);
    desktopbar->setExclusive(true);
    desktopbar->setBackgroundColor(backgroundColor());
    if ( !mBackTexture.isNull() )
      desktopbar->setBackgroundPixmap( mBackTexture );
    desktopbar->installEventFilter( this );
    edit_button = 0;


    //     QColorGroup motif_nor( black, lightGray,
    // 			   white, lightGray.dark(), gray,
    // 			   black, white );

//     QColor col = backgroundColor().light(96);
//     QColorGroup colgrp = QColorGroup( black, col, white, col.dark(), col.dark(120),
// 				      black, white );
//     QPalette pal = QPalette(colgrp,colgrp,colgrp);



    for (i=0; i < number_of_desktops; i++){
      tmp_push_button = new QPushButton("Desktop", desktopbar);
      tmp_push_button->installEventFilter( this );
      tmp_push_button->setToggleButton(true);
      tmp_push_button->setFocusPolicy(NoFocus);
    }
    tmp_push_button = (QPushButton*) desktopbar->find(currentDesktop-1);
    if (tmp_push_button)
      tmp_push_button->toggle();
    connect(desktopbar, SIGNAL(clicked(int)), SLOT(desktop_change(int)));

    taskbar_frame = new myFrame(autoHideTaskbar, autoHideTaskbarDelay, 0, 0,
			   WStyle_Customize | WStyle_NoBorder | WStyle_Tool);

    connect(taskbar_frame, SIGNAL(showMe()), SLOT(showTaskbar()));
    connect(taskbar_frame, SIGNAL(hideMe()), SLOT(hideTaskbar()));

    info_label = new QLabel(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
    info_label->setMouseTracking(true);
    info_label_is_sleeping = False;
    {
      QColorGroup g( black, QColor(255,255,220),
		     QColor(96,96,96), black, black,
		     black, QColor(255,255,220) );
      info_label->setPalette( QPalette( g, g, g ) );
      if ( QApplication::style() == MotifStyle )
	info_label->setFrameStyle( QFrame::Plain | QFrame::Box );
      else
	info_label->setFrameStyle( QFrame::Raised | QFrame::Panel );
      info_label->setLineWidth( 1 );
      info_label->setMargin( 1 );
      info_label->setAlignment( AlignLeft | AlignTop );
      info_label->setAutoResize( true );
      info_label->setText(klocale->translate("No info available"));
    }


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

    load_and_set_some_fonts();



    exit_button = new QPushButton("Exit", control_group);
    exit_button->setFocusPolicy(NoFocus);
    exit_button->setMouseTracking(true);
    QToolTip::add(exit_button, klocale->translate("Logout"));
    connect(exit_button, SIGNAL(clicked()), SLOT(ask_logout()));
    exit_button->installEventFilter( this );
    //     exit_button->setPalette(pal);

    lock_button = new QPushButton("lock", control_group);
    lock_button->setFocusPolicy(NoFocus);
    lock_button->setMouseTracking(true);
    QToolTip::add(lock_button, klocale->translate("Lock screen"));
    connect(lock_button, SIGNAL(clicked()), SLOT(call_klock()));
    lock_button->installEventFilter( this );
    //     lock_button->setPalette(pal);


    panel_button = new QPushButton("panel", this);
    panel_button->setFocusPolicy(NoFocus);
    panel_button->setMouseTracking(true);
    QToolTip::add(panel_button, klocale->translate("Hide the panel"));
    panel_button->installEventFilter( this );

    connect( panel_button, SIGNAL(clicked()),
	     SLOT(hidePanelLeft()) );

	panel_button2 = new QPushButton("panel2", this );
	panel_button2->setFocusPolicy( NoFocus );
	panel_button2->setMouseTracking( true );
	QToolTip::add( panel_button2, klocale->translate( "Hide the panel" ) );
	panel_button2->installEventFilter( this );

	connect( panel_button2, SIGNAL( clicked() ),
			 SLOT( hidePanelRight() ) );

    panel_button_frame_standalone  = new QFrame(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool );
    panel_button_frame_standalone->setMouseTracking(true);
    panel_button_frame_standalone->installEventFilter( this );
    panel_button_standalone  = new QPushButton("panel", panel_button_frame_standalone);
    QToolTip::add(panel_button_standalone, klocale->translate("Show the panel"));
    panel_button_standalone->setFocusPolicy(NoFocus);
    panel_button_standalone->setMouseTracking(true);
    panel_button_standalone->installEventFilter( this );

    connect( panel_button_standalone, SIGNAL(clicked()),
	     SLOT(standalonePanelButtonClicked()) );

    panel_button_frame_standalone2  = new QFrame(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool );
    panel_button_frame_standalone2->setMouseTracking(true);
    panel_button_frame_standalone2->installEventFilter( this );
    panel_button_standalone2  = new QPushButton("panel", panel_button_frame_standalone2);
    QToolTip::add(panel_button_standalone2, klocale->translate("Show the panel"));
    panel_button_standalone2->setFocusPolicy(NoFocus);
    panel_button_standalone2->setMouseTracking(true);
    panel_button_standalone2->installEventFilter( this );

    connect( panel_button_standalone2, SIGNAL(clicked()),
	     SLOT(standalonePanelButton2Clicked()) );

    if (orientation == horizontal){


      if (dbrows == 2){
	exit_button->setGeometry(0,0,
				 (box_width-1)/2,
				 (box_height-1)/2
				 );
	
	lock_button->setGeometry(0,
				 box_height - exit_button->height(),
				 exit_button->width(),
				 exit_button->height());

	//CT 29Jan1999 - fix for more than 8 buttons and cleaner code
	for (int i=0; (tmp_button = desktopbar->find(i)); i++){
	  tmp_button->setGeometry( (i / 2)*(exit_button->width() * dbhs+1)+1,
				  !(i % 2)?0:lock_button->y(),
				    exit_button->width() * dbhs,
				    exit_button->height());
	}
	//CT	
	
	desktopbar->setGeometry(exit_button->width(),
				0,
				(exit_button->width() * dbhs + 1) * (number_of_desktops/2),
				box_height);
	
	control_group->setGeometry(
				   5 * box_width,
// 				   w/2 - (exit_button->width() + desktopbar->width())/2,
				   margin,
				   exit_button->width() + desktopbar->width(),
				   box_height);
      }
      else {
	// dbrows == 1
 	exit_button->setGeometry(0, 0,
 				 box_width,
 				 box_height
 				 );
	
 	lock_button->setGeometry(exit_button->width()+1,
 				0,
 				 box_width,
 				 box_height);
	
	for (i=0; (tmp_button = desktopbar->find(i)); i++){
	  tmp_button->setGeometry(i*(box_width/2) * dbhs + (i+1),
				  0,
				  box_width/2 * dbhs,
				  box_height);
	}
	
	desktopbar->setGeometry(lock_button->x() + lock_button->width(),
				0,
				(box_width/2 * dbhs +1) * number_of_desktops ,
				box_height);
	
	control_group->setGeometry(
				   5 * box_width,
// 				   w/2 - (2 * box_width + 1 + desktopbar->width())/2,
				   margin,
				   2*box_width + 1 + desktopbar->width() ,
				   box_height);

	

      }

      panel_button->setGeometry(0,
				0,
				12,
				box_height+2*margin);

      panel_button2->setGeometry( w -12, 0, 12,
				  box_height+2*margin );


      kde_button->setGeometry(panel_button->x()+panel_button->width()
			      + box_width,
			      margin?margin:1,
 			      box_width-(margin?0:2), box_height-(margin?0:2));

      bound_top_left = control_group->x();
      bound_bottom_right = control_group->x() + control_group->width();

      if (label_date_visible)
	label_date->setGeometry(w - margin - label_date->width() - 5 - panel_button2->width(),
				3,
				label_date->width(),
				box_height+2*margin-6);
      else
	label_date->setGeometry(w,
				3,
				1,
				box_height+2*margin-6);

      label_date->setAlignment( AlignRight|AlignVCenter );

      dock_area->setGeometry(label_date->x() - 2*box_width + 6 - 4,
			     3,
			     2 * box_width - 6,
			     box_height+2*margin-6);



      if (position == top_left) {
	setGeometry(0,
		    0,
		    w,
		    box_height+2*margin);
      }
      else {
	setGeometry(0,
		    h - box_height-2*margin,
		    w,
		    box_height+2*margin);
      }
    }
    else { // orientation == vertical


      exit_button->setGeometry(0,0,
			       box_width/2,
			       box_height/2
			       );

      lock_button->setGeometry(box_width-exit_button->width(),
			       0,
			       exit_button->width(),
			       exit_button->height());

      for (i=0; (tmp_button = desktopbar->find(i)); i++){
	tmp_button->setGeometry(0,i*(box_height/2) + (i+1),
				box_width, box_height/2);
      }

      desktopbar->setGeometry(0,
			      exit_button->height(),
			      box_width,
			      number_of_desktops*(box_height/2+1));

      control_group->setGeometry(margin,
				 h/2 - (exit_button->height() + desktopbar->height())/2,
				 box_width,
				 exit_button->height() + desktopbar->height());

      panel_button->setGeometry(0,
				0,
				box_width + 2*margin,
				12);

	  panel_button2->setGeometry( 0, h-12, box_width + 2*margin, 12 );

      kde_button->setGeometry(margin?margin:1,
 			      panel_button->y()+panel_button->height()
			      + box_height,
 			      box_width-(margin?0:2), box_height-(margin?0:2));


      bound_top_left = control_group->y();
      bound_bottom_right = control_group->y() + control_group->height();


      if (label_date_visible)
	label_date->setGeometry(3,
				h - margin - 2*label_date->height() -5 - panel_button2->height(),
				box_width+2*margin-6,
				2*label_date->height());

      else
	label_date->setGeometry(3,
				h,
				box_width+2*margin-6,
				1);


      label_date->setAlignment( AlignHCenter|AlignBottom);

      dock_area->setGeometry(3,
			     label_date->y() - box_height - 2*margin - 4,
			     box_width+2*margin-6,
			     box_height);


      if (position == top_left){
	setGeometry(0,
		    0,
		    box_width+2*margin,
		    h);
      }
      else{
	setGeometry(w - box_width - 2*margin,
		    0,
		    box_width+2*margin,
		    h);
      }
    }

    lock_button->setPixmap(kapp->getIconLoader()
			   ->loadIcon("key.xpm"));
// 				      lock_button->width()-4,
// 				      lock_button->height()-4));
    exit_button->setPixmap(kapp->getIconLoader()
			   ->loadIcon("exit.xpm"));
// 				      exit_button->width()-4,
// 				      exit_button->height()-4));


//     if (orientation == vertical){
//       QWMatrix m;
//       m.rotate(90);
//       QPixmap pm = kapp->getIconLoader()
// 			      ->loadIcon("kpanel.xpm",
// 					 panel_button->height(),
// 					 panel_button->width());
//       panel_button->setPixmap(pm.xForm(m));
//     }else{
//       panel_button->setPixmap(kapp->getIconLoader()
// 			      ->loadIcon("kpanel.xpm",
// 					 panel_button->width(),
// 					 panel_button->height()));

//     }


    {
      // create the stipple pixmap for the panel button
      QPixmap pm;
      QBitmap bm;
      QPainter paint;
      QPainter paint2;
      pm.resize(9, (orientation == horizontal)?panel_button->height():panel_button->width());
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
	  panel_button2->setPixmap(pm2.xForm(m));
	  panel_button_standalone2->setPixmap(pm.xForm(m));
    }

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

    panel_button_frame_standalone2->setGeometry(x() + width() - panel_button2->width(),
						y() + panel_button2->y(),
						panel_button2->width(),
						panel_button2->height() );
    panel_button_standalone2->setGeometry(0, 0, panel_button2->width(), panel_button2->height());

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

    // fix label_date size (PGB)
    QFont tmpfont = label_date->font();
    QString teststr = (clockAmPm ? "00:00AM" : "00:00");
    while(label_date->fontMetrics().width(teststr) >
	  label_date->width()) {
      tmpfont.setPointSize(tmpfont.pointSize()-1);
      label_date->setFont(tmpfont);
    }
    label_date->adjustSize();

    QTime cur_time = QTime::currentTime();
    clock_timer_id = 0;
    int next_minute_break = (60 - cur_time.second()) * 1000 - cur_time.msec();
    QTimer::singleShot(next_minute_break, this, SLOT(slotUpdateClock()));


    pmenu = new PMenu;
    pmenu->setAltSort(foldersFirst);

    readInConfiguration();
    KApplication::getKApplication()->getConfig()->sync();
    doGeometry();
    if (panelHiddenString=="00000000"){
      for (i=1;i<=8;i++)
	KWM::setWindowRegion(i, KWM::getWindowRegion(currentDesktop));
    }
    layoutTaskbar();
    for (i=0; i < number_of_desktops; i++){
      set_button_text(desktopbar->find(i),
		      KWM::getDesktopName(i+1));
    }

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
    edit_button->raise();
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
    if ( edit_button != 0)
      restore_editbutton( False );
    currentDesktop = new_desk;
    KWM::switchToDesktop(currentDesktop);
  }
};

void kPanel::restore_editbutton( bool takeit ) {
  if ( takeit ) {
    KWM::setDesktopName(currentDesktop, edit_button->text());
  }
  if (edit_button != 0) {
    edit_button->releaseKeyboard();
    edit_button->releaseMouse();
    edit_button->hide();
    delete edit_button;
    edit_button = 0;
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

      popup_item->setItemEnabled(0, true);
      popup_item->setItemEnabled(1, true);
      popup_item->setItemEnabled(3, true);

      if (button_to_be_modified == kde_button){
	popup_item->setItemEnabled(0, true);
	popup_item->setItemEnabled(1, false);
	popup_item->setItemEnabled(3, true);
      }
      if (entries[i].popup == windowlist){
	popup_item->setItemEnabled(0, true);
	popup_item->setItemEnabled(1, true);
	popup_item->setItemEnabled(3, false);
      }

      switch (show_popup(popup_item, entries[i].button)){
      case 3:
	  if (button_to_be_modified == kde_button) {
	      editMenus();
	  }
	  else if (entries[i].pmi){
	  KFM* kfm = new KFM;
	  QString a = entries[i].pmi->getFullPathName().copy();
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
	  XGrabPointer( qt_xdisplay(), moving_button->winId(), false,
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
	  button_to_be_modified = 0;
	  writeOutConfiguration();
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
      if (b->isActive() && KWM::desktop(b->win) == KWM::currentDesktop())
	  KWM::setIconify(b->win, TRUE );
      else
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
  pop.setMouseTracking(true);

  if (KWM::isMaximized(b->win) && !KWM::fixedSize(b->win))
    pop.insertItem(KWM::getUnMaximizeString(),
		   OP_RESTORE);
  else
    pop.insertItem(KWM::getMaximizeString(),
		   OP_MAXIMIZE);

  if (KWM::fixedSize(b->win))
    pop.setItemEnabled(OP_MAXIMIZE, false);

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
    pop.setItemEnabled(OP_ONTO_CURRENT_DESKTOP, false);

  pop.insertSeparator();
  pop.insertItem(i18n("Iconify other &windows"), OP_ICONIFY_OTHER_WINDOWS);
  if (b->virtual_desktop != KWM::currentDesktop())
    pop.setItemEnabled(OP_ICONIFY_OTHER_WINDOWS, false);
  pop.insertSeparator();
  pop.insertItem(KWM::getCloseString(),
		 OP_CLOSE);


  switch (show_popup(&pop, b, true)){
  case OP_MAXIMIZE:
    KWM::setMaximize(b->win, true);
    break;
  case OP_RESTORE:
    KWM::setMaximize(b->win, false);
    break;
  case OP_ICONIFY:
    KWM::setIconify(b->win, true);
    break;
  case OP_UNICONIFY:
    KWM::setIconify(b->win, false);
    break;
  case OP_CLOSE:
    KWM::close(b->win);
  case OP_STICKY:
    KWM::setSticky(b->win, !KWM::isSticky(b->win));
    break;
  case OP_ONTO_CURRENT_DESKTOP:
    KWM::moveToDesktop(b->win, KWM::currentDesktop());
    break;
  case OP_ICONIFY_OTHER_WINDOWS:
      {
	  myTaskButton* bi;
	  int cd = KWM::currentDesktop();
	  for (bi=taskbar_buttons.first(); bi; bi = taskbar_buttons.next()){
	      if ( b != bi && KWM::desktop(bi->win) == cd)
		  KWM::setIconify(bi->win, true);
	  }
      }
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
   entries[nbuttons-1].popup = 0;
   entries[nbuttons-1].pmi = pmi;
   entries[nbuttons-1].swallow = "";
   entries[nbuttons-1].swallow = "";
   entries[nbuttons-1].swallowed = 0;
   entries[nbuttons-1].identity = "";
   entries[nbuttons-1].icon[0] = 0;
   entries[nbuttons-1].icon[1] = 0;
   entries[nbuttons-1].icon[2] = 0;
   entries[nbuttons-1].icon[3] = 0;
   entries[nbuttons-1].drop_zone = 0;

   connect( entries[nbuttons-1].button, SIGNAL(clicked()),
	    SLOT(button_clicked()) );
   connect( entries[nbuttons-1].button, SIGNAL(pressed()),
	    SLOT(button_pressed()) );

   if (pmi){
     if (pmi->getType() == submenu){
       PMenu* pm = new PMenu;
       pm->setAltSort(foldersFirst);
       pm->parse(QDir(pmi->getFullPathName()));
       PMenuItem* pmi2 = new PMenuItem;
       QFileInfo fi(pmi->getFullPathName());
       pmi2->parse(&fi, pm);
       pmi = pmi2;
       pm->createMenu(pmi->getQPopupMenu(), this);
       entries[nbuttons-1].pmi = pmi;

       entries[nbuttons-1].button->setPixmap(create_arrow_pixmap( load_pixmap(pmi->getBigIconName(), True)));
     }
     else{
       entries[nbuttons-1].button->setPixmap(load_pixmap(pmi->getBigIconName()));

       QFile myfile(pmi->getFullPathName());
       if (myfile.exists()){
	 if (myfile.open ( IO_ReadOnly )){
	   // kalle	   QTextStream mystream(&myfile);

	   myfile.close(); // kalle
	   KSimpleConfig pConfig(pmi->getFullPathName(),true);
	   pConfig.setGroup("KDE Desktop Entry");
	   QString aString;
	   if (pConfig.hasKey("SwallowTitle")){
	     entries[nbuttons-1].swallow = QString(pConfig.readEntry("SwallowTitle")).copy();
	     if (!entries[nbuttons-1].swallow.isEmpty() &&
		 pConfig.hasKey("SwallowExec")){
	       KWM::doNotManage(entries[nbuttons-1].swallow);
	       aString = QString(pConfig.readEntry("SwallowExec")).copy();
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
     if (!pmi->getComment().isEmpty())
       QToolTip::add(entries[nbuttons-1].button, pmi->getComment());
     // PI: no useless comments
     // else
     //    QToolTip::add(entries[nbuttons-1].button,
     //	      klocale->translate("No comment available"));


   }
   else {
     if (name == "windowlist"){
       entries[nbuttons-1].popup = windowlist;
       entries[nbuttons-1].button->setPixmap(create_arrow_pixmap(load_pixmap("window_list.xpm")));
       QToolTip::add(entries[nbuttons-1].button, klocale->translate("Windowlist"));
     }
     // --sven: kdisknav button start --
     else if (name == "kdisknav"){
       entries[nbuttons-1].popup = kdisknav;
       // store this buttonand check if release is over this button!!!
       entries[nbuttons-1].button->setPixmap(create_arrow_pixmap(load_pixmap("kdisknav.xpm")));
       QToolTip::add(entries[nbuttons-1].button,
		     klocale->translate("Quickly navigate through the filesystem"));
     }
     // --sven: kdisknav button end --
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
    }
  else {
      if (panelCurrentlyLeft) {
	  panel_button_frame_standalone->show();
	  panel_button_frame_standalone->raise();
	  panel_button_frame_standalone2->hide();
      }
      else {
	  panel_button_frame_standalone2->show();
	  panel_button_frame_standalone2->raise();
	  panel_button_frame_standalone->hide();
      }
    showMiniPanel();
  }

  if (taskbar_position != hidden)
   {
    taskbar_frame->show();
   }
}

void kPanel::showPanel()
{
    if (!panelCurrentlyHidden)
	return;
    if (panelCurrentlyLeft)
	showPanelFromLeft();
    else
	showPanelFromRight();
}

extern bool in_animation;

void kPanel::hidePanelLeft(){
  Bool old = panelHidden[currentDesktop];

  panelHidden[currentDesktop] = True;
  panelHiddenLeft[currentDesktop] = True;

  if (in_animation)
      return;

  if (panelCurrentlyHidden && !panelCurrentlyLeft) {
      showPanelFromRight(False);
      panelHidden[currentDesktop] = True;
      panelHiddenLeft[currentDesktop] = True;
  }

  if (!panelCurrentlyHidden){
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
    panelCurrentlyLeft = True;

    QRect geom = geometry();

    if(hide_show_animation) {
      in_animation = true;
      if (orientation == vertical) {
	for (int i = 0; i < geom.height() - panel_button_frame_standalone->height();
	     i+=PANEL_SPEED(i,geom.height())){
	  move(geom.x(), geom.y()-i);
	  qApp->syncX();
	  qApp->processEvents();
	}
      } else {
	for (int i = 0; i < geom.width() - panel_button_frame_standalone->width();
	     i += PANEL_SPEED(i,geom.width())){
	  move(geom.x()-i, geom.y());
	  qApp->syncX();
	  qApp->processEvents();
	}
      }
    }
    panel_button_frame_standalone->show();
    panel_button_frame_standalone->raise();
    QFrame::hide();
    move(geom.x(), geom.y());
    in_animation = false;


    showMiniPanel();
    //panel_button_frame_standalone->show();
    //panel_button_frame_standalone->raise();


    doGeometry();
    layoutTaskbar (); //geometry changed
   }

  if (!panelHidden[currentDesktop]){
     showPanelFromLeft();
     return;
  }

  if (old != panelHidden[currentDesktop]){
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup("kpanel");
    QString a;
    int i;
    for (i=1;i<=8;i++)
      a.append(panelHidden[i]?"1":"0");
    config->writeEntry("PanelHidden", a);
    a = "";
    for (i=1;i<=8;i++)
      a.append(panelHiddenLeft[i]?"1":"0");
    config->writeEntry("PanelHiddenLeft", a);
    config->sync();
  }
}

void kPanel::hidePanelRight(){
  Bool old = panelHidden[currentDesktop];

  panelHidden[currentDesktop] = True;
  panelHiddenLeft[currentDesktop] = False;

  if (in_animation)
      return;

  if (panelCurrentlyHidden && panelCurrentlyLeft){
      showPanelFromLeft(False);
      panelHidden[currentDesktop] = True;
      panelHiddenLeft[currentDesktop] = False;
  }

  if (!panelCurrentlyHidden){
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
    panel_button_frame_standalone2->setGeometry(x() + width() - panel_button2->width(),
						p.y() + panel_button2->y(),
						panel_button2->width(),
						panel_button2->height());
    panelCurrentlyHidden = True;
    panelCurrentlyLeft = False;

    QRect geom = geometry();

    if(hide_show_animation) {
      in_animation = true;

      if (orientation == vertical) {
	for (int i = 0; i < geom.height() - panel_button_frame_standalone2->height();
	     i += PANEL_SPEED(i,geom.height())){
	  move(geom.x(), geom.y()+i);
	  qApp->syncX();
	  qApp->processEvents();
	}
      } else {
	for (int i = 0; i < geom.width() - panel_button_frame_standalone2->width();
	     i += PANEL_SPEED(i,geom.width())){
	  move(geom.x()+i, geom.y());
	  qApp->syncX();
	  qApp->processEvents();
	}
      }
    }
    panel_button_frame_standalone2->show();
    panel_button_frame_standalone2->raise();
    QFrame::hide();
    move(geom.x(), geom.y());
    in_animation = false;


    showMiniPanel();
    //panel_button_frame_standalone2->show();
    //panel_button_frame_standalone2->raise();


    doGeometry();
    layoutTaskbar (); //geometry changed
   }

  if (!panelHidden[currentDesktop]){
     showPanelFromRight();
     return;
  }

  if (old != panelHidden[currentDesktop]){
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup("kpanel");
    QString a;
    int i;
    for (i=1;i<=8;i++)
      a.append(panelHidden[i]?"1":"0");
    config->writeEntry("PanelHidden", a);
    a = "";
    for (i=1;i<=8;i++)
      a.append(panelHiddenLeft[i]?"1":"0");
    config->writeEntry("PanelHiddenLeft", a);
    config->sync();
  }
}

void kPanel::showPanelFromLeft( bool smooth){
  Bool old = panelHidden[currentDesktop];

  panelHidden[currentDesktop] = False;
  if (in_animation)
      return;

  panel_button_frame_standalone->hide();


  if (panelCurrentlyHidden){
    panelCurrentlyHidden = False;

    QRect geom = geometry();
    in_animation = true;
    move(-10000, -10000);
    QFrame::show();
    if(hide_show_animation) {
      if (orientation == vertical) {
	for (int i = geom.height(); i>12;i-=smooth?(PANEL_SPEED(i,geom.height())):hide_show_animation){
	  move(geom.x(), geom.y()-i);
	  qApp->syncX();
	  qApp->processEvents();
	}
      } else {
	for (int i = geom.width(); i>12;i-=smooth?(PANEL_SPEED(i,geom.width())):hide_show_animation){
	  move(geom.x()-i, geom.y());
	  qApp->syncX();
	  qApp->processEvents();
	}
      }
    }
    move(geom.x(), geom.y());
    in_animation = false;

  }
  if (!smooth)
      return;
  hideMiniPanel();
  doGeometry();
  layoutTaskbar();

  if (panelHidden[currentDesktop]){
     hidePanelLeft();
     return;
  }

  if (old != panelHidden[currentDesktop]){
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup("kpanel");
    QString a;
    int i;
    for (i=1;i<=8;i++)
      a.append(panelHidden[i]?"1":"0");
    config->writeEntry("PanelHidden", a);
    config->sync();
  }
}

void kPanel::showPanelFromRight(bool smooth){
  Bool old = panelHidden[currentDesktop];

  panelHidden[currentDesktop] = False;
  if (in_animation)
      return;

  panel_button_frame_standalone2->hide();


  if (panelCurrentlyHidden){
    panelCurrentlyHidden = False;

    QRect geom = geometry();
    in_animation = true;
    move(-10000, -10000);
    QFrame::show();
    if(hide_show_animation) {
      if (orientation == vertical) {
	for (int i = geom.height(); i>12;i-=smooth?(PANEL_SPEED(i,geom.height())):hide_show_animation){
	  move(geom.x(), geom.y()+i);
	  qApp->syncX();
	  qApp->processEvents();
	}
      } else {
	for (int i = geom.width(); i>12;i-=smooth?(PANEL_SPEED(i,geom.width())):hide_show_animation){
	  move(geom.x()+i, geom.y());
	  qApp->syncX();
	  qApp->processEvents();
	}
      }
    }
    move(geom.x(), geom.y());
    in_animation = false;

  }
  if (!smooth)
      return;
  hideMiniPanel();
  doGeometry();
  layoutTaskbar();

  if (panelHidden[currentDesktop]){
     hidePanelRight();
     return;
  }

  if (old != panelHidden[currentDesktop]){
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup("kpanel");
    QString a;
    int i;
    for (i=1;i<=8;i++)
      a.append(panelHidden[i]?"1":"0");
    config->writeEntry("PanelHidden", a);
    config->sync();
  }
}

//////////////////////////////////////////////////////////////////////////////
// Set the same WindowRegion for all desktops.
// Called when a window is added/removed and the number of taskbar rows 
// changes.
void kPanel::syncWindowRegions() {
  QRect region = KWM::getWindowRegion(currentDesktop);
  int nd = KWM::numberOfDesktops();

  for (int i = 1; i <= nd; i++)
    if (i != currentDesktop)
      KWM::setWindowRegion(i, region);
}

void kPanel::doGeometry (bool do_not_change_taskbar) {

   int space = 0;

   int w = QApplication::desktop()->width();
   int h = QApplication::desktop()->height();
   int pw = width()+space; // panel...
   int ph = height()+space;
   int px = x();
   int py = y();
   int pxR = x(); // for the window region
   int pyR = y(); // for the window region
   int pwR = width()+space; // for the window region
   int phR = height()+space;// for the window region
   int mw = 0;       // mini panel...
   int mh = 0;
   int sw = 0;      // standalone...
   int sh = 0;
   int sx = 0;

   int tfx = 0; //correction values for the taskbar autoHide feature
   int tfy = 0;
   int tfxR = 0; //correction values for the taskbar autoHide feature used
   int tfyR = 0; // to set the window region

   int th = (taskbar_position==hidden) ? 0 :
	   taskbar_height * numberOfTaskbarRows();

   int tbw = taskbar_frame->width();
   int tbh = taskbar_frame->height();


   taskbar_frame_geometry = taskbar_frame->geometry();



   if (autoHideTaskbar){
     if (taskbar_position == top){
       tfyR = -th+4;
     }
     else if (taskbar_position == bottom){
       tfyR = th-4;
     }
     else { //taskbar_top_left
	 tfxR = -tbhs*taskbar_height+4;
     }
   }

   if (taskbar_frame->autoHidden){
       tfy = tfyR;
       tfx = tfxR;
   }


   if (panelCurrentlyHidden) // i.e. standalone shown
    {
     pw = pwR = 0;
     ph = phR = 0;
     px = pxR = 0;
     py = pyR = 0;
     sw = panel_button_frame_standalone->width()+space;
     sh = panel_button_frame_standalone->height()+space;
     {
       mw = 3*taskbar_height+space; // miniPanelFrame
       mh = taskbar_height+space; // miniPanelFrame
     }

     sx = sw;
     if (!panelCurrentlyLeft) {
	 //right panel button, grrrrr it is becoming hacky......
	 if (orientation == horizontal) {
	     if (position == top_left) {
		 if (taskbar_position == top)
		     sx = 0;
		 else if (taskbar_position == bottom)
			  ;
		 else
		     sx = 0;
	     }
	     else { // position == bottom_right
		 if (taskbar_position == top)
		     ;
		 else if (taskbar_position == bottom)
		     sx = 0;
		 else
		     sx = 0;
	     }
	 }
	 else { // orientation == vertical
	     if (position == top_left) {
		 if (taskbar_position == top)
		     sx = 0;
		 else if (taskbar_position == bottom)
			  ;
		 else
		     sx = 0;
	     }
	     else { // position == bottom_right
		 if (taskbar_position == top)
		     { sw = 0; sx = 0; }
		 else if (taskbar_position == bottom)
		     ;
		 else
		     sx = 0;
	     }
	 }
     }
     else {
	 // some corrections....
	 if (orientation == vertical) {
	     if (taskbar_position == bottom)
		 { sx = 0; sw = 0; }
	 }
     }
    }
   else if (autoHide) {
       // panel is in autohide mode and not hidden. Adjust the window region to the hidden state.
       if (!autoHidden) {
	   if (orientation == horizontal){
	       if (position == top_left)
		   pyR = pyR -  height() + 4;
	       else
		   pyR = pyR +  height() - 4;
	   }
	   else {
	       if (position == top_left)
		   pxR = pxR - width() + 4;
	       else
		   pxR = pxR + width() - 4;
	   }
       }

       if (pxR<0){
	   pwR += pxR;
	   pxR = 0;
       }
       if (pyR<0){
	   phR += pyR;
	   pyR = 0;
       }
       if (pxR + pwR >w){
	   pwR = w-pxR+space;
       }
       if (pyR + phR >h){
	   phR = h-pyR+space;
       }


       if (autoHidden) {
	   pw = pwR;
	   ph = phR;
	   px = pxR;
	   py = pyR;
       }
   }


   if (orientation == horizontal)
    {
     if (position == top_left)
      {
       if (taskbar_position == top)
	{
	 taskbar_frame_geometry.setRect(tfx+px+sx+mw, tfy+py+ph, w-mw-sw, taskbar_height);
	 KWM::setWindowRegion(currentDesktop,
			      QRect(0, tfyR+pyR+phR+taskbar_height+space,
				    w, -tfyR+h-phR-taskbar_height-space));
	}
       else if (taskbar_position == bottom)
	{
	 taskbar_frame_geometry.setRect(tfx+px+mw, tfy+h-th,
				    w-mw, th);
	 KWM::setWindowRegion(currentDesktop,
			      QRect(0, phR, w, tfyR+h-phR-th-space));
	}
       else
	{
// 	 taskbar_frame_geometry.setRect(tfx+px+sw, tfy+py+ph+mh,
// 				    tbhs*th, th);
	 taskbar_frame_geometry.moveTopLeft(QPoint(tfx+px+sx, tfy+py+ph+mh));
	 if (taskbar_position == taskbar_top_left)
	   KWM::setWindowRegion(currentDesktop,
				QRect(tfxR+tbhs*th+space+sx, phR,
				      -tfxR+w-tbhs*th-sw-space, h-phR));
	 else // if (taskbar_position=hidden)
	   KWM::setWindowRegion(currentDesktop,
				QRect(0, phR, w, h-phR));
	}
      }
     else // (if position == bottom_right)
      {
       if (taskbar_position == bottom)
	{
	 taskbar_frame_geometry.setRect(tfx+px+mw+sx, tfy+h-ph-th,
				    w-mw-sw, th);
	   KWM::setWindowRegion(currentDesktop,
				QRect(0, 0, w, tfyR+h-phR-th-space));
	}
       else if (taskbar_position == top)
	{
	 taskbar_frame_geometry.setRect(tfx+px+mw, tfy+0, w-mw, th);
	 KWM::setWindowRegion(currentDesktop,
			      QRect(0, tfyR+th+space, w, -tfyR+h-phR-th-space));
	}
       else
	{
// 	 taskbar_frame_geometry.setRect(tfx+0, tfy+mh,tbhs*taskbar_height,
// 				    taskbar_height);
	 taskbar_frame_geometry.moveTopLeft(QPoint(tfx+0, tfy+mh));
	 if (taskbar_position == taskbar_top_left)
	   KWM::setWindowRegion(currentDesktop,
				QRect(tfxR+tbhs*taskbar_height+space, 0,
				      -tfxR+w-tbhs*taskbar_height-space, h-phR));
	 else
	   KWM::setWindowRegion(currentDesktop,
				QRect(0, 0, w, h-phR));
	}
      }
    }
   else // if (orientation == vertical)
    {
     if (position == top_left)
      {
       if (taskbar_position == top)
	{
	 taskbar_frame_geometry.setRect(tfx+px+pw+mw+sx, tfy+py, w-pw-mw-sw, th);
	 KWM::setWindowRegion(currentDesktop,
			      QRect(pwR, tfyR+taskbar_frame->height()+space,
				    w-pwR, -tfyR+h-taskbar_frame->height()-space));
	}
       else if (taskbar_position == bottom)
	{
	 taskbar_frame_geometry.setRect(tfx+px+pw+mw+sx, tfy+h-th,
				    w-pw-mw-sw, th);
	 KWM::setWindowRegion(currentDesktop,
			      QRect(pwR, 0, w-pwR, tfyR+h - taskbar_frame->height()-space));
	}
       else
	{
// 	 taskbar_frame_geometry.setRect(tfx+px+pw, tfy+py+mh,
// 				    tbhs*taskbar_height, taskbar_height);
	 taskbar_frame_geometry.moveTopLeft(QPoint(tfx+px+pw+sx, tfy+py+mh));
	 if (taskbar_position == taskbar_top_left)
	 KWM::setWindowRegion(currentDesktop,
			      QRect(tfxR+pwR+tbhs*taskbar_height+space, 0,
 			      -tfxR+w-pwR-tbhs*taskbar_height-space, h));
	 else
	 KWM::setWindowRegion(currentDesktop,
 			      QRect(pwR, 0,
 			      w-pwR, h));
	}
      }
     else // if (position == bottom_right)
      {
       if (taskbar_position == top)
	{
	 taskbar_frame_geometry.setRect(tfx+mw, tfy+0, w-pw-mw-sw, th);
	 KWM::setWindowRegion(currentDesktop,
			      QRect(0, tfyR+taskbar_frame->height()+space,
				    w-pwR, -tfyR+h-taskbar_frame->height()-space));
	}
       else if (taskbar_position == bottom)
	{
	 taskbar_frame_geometry.setRect(tfx+mw, tfy+h-th,
				    w-pw-mw-sw, th);
	 KWM::setWindowRegion(currentDesktop,
			      QRect(0, 0,
				    w-pwR, tfyR+h-taskbar_frame->height()-space));
	}
       else
	{
// 	 taskbar_frame_geometry.setRect(tfx+0, tfy+mh,
// 				    tbhs*th, taskbar_height);
	 taskbar_frame_geometry.moveTopLeft(QPoint(tfx+0, tfy+mh));
	 if (taskbar_position == taskbar_top_left)
	 KWM::setWindowRegion(currentDesktop,
 			      QRect(tfxR+tbhs*taskbar_height+space, 0,
				    -tfxR+w-tbhs*taskbar_height-space-pwR, h));
	 else
	 KWM::setWindowRegion(currentDesktop,
 			      QRect(0, 0,
				    w-pwR, h));
	}
      }
    }
   if (do_not_change_taskbar)
       return;

   taskbar_frame->setGeometry(taskbar_frame_geometry);
   taskbar->resize(taskbar_frame->width(), taskbar_frame->height());
   if (taskbar_frame->width() != tbw || taskbar_frame->height() != tbh){
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
//     tmpfont.setRawMode(true);
//     tmpfont.setFamily(config->readEntry("MenuFont"));
//     KApplication::setFont(tmpfont, True);
//   }

  if (config->hasKey("DateFont")){
    //QFont tmpfont;
    //tmpfont.setRawMode(true);
    //tmpfont.setFamily(config->readEntry("DateFont"));
    QFont tmpfont = config->readFontEntry( "DateFont" );
    label_date->setFont(tmpfont);
    label_date->adjustSize();
  }

  if (config->hasKey("DesktopButtonFont")){
    //QFont tmpfont;
    //tmpfont.setRawMode(true);
    //tmpfont.setFamily(config->readEntry("DesktopButtonFont"));
	QFont tmpfont = config->readFontEntry( "DesktopButtonFont" );
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

    if( clockBeats == true )
      clock_timer_id = t->start(86000);
    else
      clock_timer_id = t->start(60000);
  }

}
