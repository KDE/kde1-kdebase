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


DesktopEntry::DesktopEntry(){
  button = NULL;
  popup = NULL;
  pmi = NULL;
  drop_zone = NULL;
  app_id = 0;
  swallow="";
  swallowed = 0;
  identity = "";
  icon[0] = NULL;
  icon[1] = NULL;
  icon[2] = NULL;
  icon[3] = NULL;
};


kPanel::kPanel( KWMModuleApplication* kwmapp_arg,
		QWidget *parent, const char *name )
  : QFrame( parent, name,WStyle_Customize | WStyle_NoBorder | WStyle_Tool ){
    //    : QFrame( parent, name){

    kwmmapp = kwmapp_arg;
    int i;
    
    tab = NULL;

    last_tip_widget = NULL;

    setFrameStyle(QFrame::Panel| QFrame::Raised);
    
    setMouseTracking(TRUE);

    orientation = horizontal;
    position = bottom_right;

    currentDesktop = KWM::currentDesktop();
     
    //     box_width = 40;
    //     box_height = 40;
    //     margin = 0;
    //     pm_scale_factor = 0.8;
    
    
    box_width = 52;
    box_height = 52;
    margin = 4;
    pm_scale_factor = 1;
    dbhs = 6;
    dbrows = 2;

    menu_tool_tips = 1000;
    
    tbhs = 4;
    number_of_desktops = KWM::numberOfDesktops();

    tipTimer = new QTimer( this );
    connect( tipTimer, SIGNAL(timeout()),
	     this, SLOT(tipTimerDone()) );
    tipSleepTimer = new QTimer( this );
    connect( tipSleepTimer, SIGNAL(timeout()),
	     this, SLOT(tipSleepTimerDone()) );
    
    
    // parse the configuration
    KConfig *config = KApplication::getKApplication()->getConfig();
    // Don't write dirty entries at destruction time.
    config->rollback();

    config->setGroup("kpanel");
    if (!config->hasKey("Style")) {
      config->writeEntry("Style", "large");
      config->writeEntry("BoxWidth",52);
      config->writeEntry("BoxHeight",52);
      config->writeEntry("Margin",4);
      config->writeEntry("TaskbarButtonHorizontalSize",4);
      config->writeEntry("IconScale","1.0");
      config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--14-*");
      config->writeEntry("DesktopButtonRows",2);
      config->writeEntry("DateFont","*-times-bold-i-normal--12-*");
      config->writeEntry("TaskbarFont","*-helvetica-medium-r-normal--10-*");
    }

    QString a = config->readEntry("Position");
    
    if ( a == "left"){
      orientation = vertical;
      position = top_left;
    }
    else if ( a == "right"){
      // no longer valid! Do left instead
      orientation = vertical;
      position = top_left;
//       position = bottom_right;

    }
    else if ( a == "top"){
      orientation = horizontal;
      position = top_left;
    }
    else if ( a == "bottom"){
      orientation = horizontal;
      position = bottom_right;
    }

    if (config->hasKey("BoxWidth"))
      box_width = config->readNumEntry("BoxWidth");
    if (config->hasKey("BoxHeight"))
      box_height = config->readNumEntry("BoxHeight");
    if (config->hasKey("Margin"))
      margin = config->readNumEntry("Margin");
    if (margin == 0)
      margin = 1;
      
    if (config->hasKey("IconScale")){
      pm_scale_factor = config->readEntry("IconScale").toFloat();
    }
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


    QString panelHiddenString = "00000000";
    panelHiddenString = config->readEntry("PanelHidden", 
					  panelHiddenString);
    
    for (i=1;i<=8;i++)
      panelHidden[i] = (panelHiddenString.mid(i-1,1)=="1");
    
    panelCurrentlyHidden = panelHidden[currentDesktop];
    miniPanelHidden = True;
     

    nbuttons = 0;
    moving_button = NULL;
    wait_cursor_button = NULL;
    
    int tx, ty;



    popup_item = new myPopupMenu;
    CHECK_PTR( popup_item);
    popup_item->insertItem(klocale->translate("Move"));
    popup_item->insertItem(klocale->translate("Remove"));
    popup_item->insertSeparator();
    popup_item->insertItem(klocale->translate("Properties"));
    init_popup(popup_item);

    windows = new myPopupMenu;
    CHECK_PTR( windows );
    init_popup(windows);

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


    label_date = new QLabel(this);
    set_label_date();
    if (config->hasKey("DateVisible")){
      QString aString = config->readEntry("DateVisible");
      if (aString == "false"){
	label_date->hide();
      }
    }

    
    entries[nbuttons++].button = new myPushButton( this, klocale->translate("System"));
    entries[nbuttons-1].button->installEventFilter( this );
    QToolTip::add(entries[nbuttons-1].button, klocale->translate("System"));

    connect( entries[nbuttons-1].button, SIGNAL(clicked()), 
	     SLOT(button_clicked()) );
    connect( entries[nbuttons-1].button, SIGNAL(pressed()), 
	     SLOT(button_pressed()) );
    kde_button = entries[nbuttons-1].button;
    kde_button->setPixmap(create_arrow_pixmap(load_pixmap("system.xpm")));


    // The control group
    control_group = new QFrame( this);
    control_group->setMouseTracking( TRUE );;
    control_group->setBackgroundColor(backgroundColor());
    control_group->installEventFilter( this );

    desktopbar = new QButtonGroup(control_group);
    CHECK_PTR( desktopbar );
    desktopbar->setFrameStyle(QFrame::NoFrame);
    desktopbar->setExclusive(TRUE);
    desktopbar->setBackgroundColor(backgroundColor());
    desktopbar->installEventFilter( this );
    edit_button = NULL;


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
      tmp_push_button->setToggleButton(TRUE);
      tmp_push_button->setFocusPolicy(NoFocus);
    }
    tmp_push_button = (QPushButton*) desktopbar->find(currentDesktop-1);
    tmp_push_button->toggle();
    connect(desktopbar, SIGNAL(clicked(int)), SLOT(desktop_change(int)));

    taskbar_frame = new QFrame(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
    info_label = new QLabel(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
    info_label->setMouseTracking(TRUE);
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
      info_label->setMargin( 3 );
      info_label->setAlignment( AlignLeft | AlignTop );
      info_label->setAutoResize( TRUE );
      info_label->setText(klocale->translate("No info available"));
    }


    taskbar = new QButtonGroup(taskbar_frame);
    taskbar->setFrameStyle(QFrame::Panel| QFrame::Raised);
    connect( taskbar, SIGNAL(clicked( int )), 
	     SLOT(taskbarClicked(int)) );
    connect( taskbar, SIGNAL(pressed( int )), 
	     SLOT(taskbarPressed(int)) );

    taskbar_position = hidden;
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

    load_and_set_some_fonts();

    

    exit_button = new QPushButton("Exit", control_group);
    exit_button->setFocusPolicy(NoFocus);
    exit_button->setMouseTracking(TRUE);
    QToolTip::add(exit_button, klocale->translate("Logout"));
    connect(exit_button, SIGNAL(clicked()), SLOT(ask_logout()));
    exit_button->installEventFilter( this );
    //     exit_button->setPalette(pal);
    exit_button->setPixmap(load_pixmap("exit.xpm"));
    
    lock_button = new QPushButton("lock", control_group);
    lock_button->setFocusPolicy(NoFocus);
    lock_button->setMouseTracking(TRUE);
    QToolTip::add(lock_button, klocale->translate("Lock screen"));
    connect(lock_button, SIGNAL(clicked()), SLOT(call_klock()));
    lock_button->installEventFilter( this );
    //     lock_button->setPalette(pal);
    lock_button->setPixmap(load_pixmap("key.xpm"));
    

    panel_button = new QPushButton("panel", this);
    panel_button->setFocusPolicy(NoFocus);
    panel_button->setMouseTracking(TRUE);
    panel_button->setPixmap(load_pixmap("kpanel.xpm"));
    QToolTip::add(panel_button, klocale->translate("Hide the panel"));
    panel_button->installEventFilter( this );
    if (orientation == vertical){
      QWMatrix m;
      m.rotate(90);
      panel_button->setPixmap(panel_button->pixmap()->xForm(m));
	
    }

    connect( panel_button, SIGNAL(clicked()), 
	     SLOT(hidePanel()) );
    
    panel_button_frame_standalone  = new QFrame(NULL, NULL, WStyle_Customize | WStyle_NoBorder | WStyle_Tool );
    panel_button_frame_standalone->setMouseTracking(TRUE);
    panel_button_frame_standalone->installEventFilter( this );
    panel_button_standalone  = new QPushButton("panel", panel_button_frame_standalone);
    QToolTip::add(panel_button_standalone, klocale->translate("Show the panel"));
    panel_button_standalone->setFocusPolicy(NoFocus);
    panel_button_standalone->setPixmap(*panel_button->pixmap());
    panel_button_standalone->setMouseTracking(TRUE);
    panel_button_standalone->installEventFilter( this );
    
    connect( panel_button_standalone, SIGNAL(clicked()), 
	     SLOT(showPanel()) );


    if (orientation == horizontal){


      if (dbrows == 2){
	exit_button->setGeometry(0,0,
				 box_width/2 - margin/4,
				 box_height/2 - margin/4
				 );
	
	lock_button->setGeometry(0,
				 box_height - exit_button->height(),
				 exit_button->width(),
				 exit_button->height());
	
	for (i=0; (tmp_button = desktopbar->find(i)); i++){
	  switch (i){
	  case 0:
	    tmp_button->setGeometry(0, 0,
				    exit_button->width() * dbhs, 
				    exit_button->height());
	    break;
	  case 2:
	    tmp_button->setGeometry(exit_button->width() * dbhs + margin/3, 0, 
				    exit_button->width() * dbhs, 
				    exit_button->height());
	    break;
	  case 4:
	    tmp_button->setGeometry(2 * exit_button->width() * dbhs + margin/3, 0, 
				    exit_button->width() * dbhs, 
				    exit_button->height());
	    break;
	  case 6:
	    tmp_button->setGeometry(3 * exit_button->width() * dbhs + margin/3, 0, 
				    exit_button->width() * dbhs, 
				    exit_button->height());
	    break;
	  case 1:
	    tmp_button->setGeometry(0, 
				    lock_button->y(),
				    exit_button->width() * dbhs, 
				    exit_button->height());
	    break;
	  case 3:    
	    tmp_button->setGeometry(exit_button->width() * dbhs + margin/3,
				    lock_button->y(),
				    exit_button->width() * dbhs, 
				    exit_button->height());
	    break;
	  case 5:    
	    tmp_button->setGeometry(2 * exit_button->width() * dbhs + margin/3,
				    lock_button->y(),
				    exit_button->width() * dbhs, 
				    exit_button->height());
	    break;
	  case 7:    
	    tmp_button->setGeometry(3 * exit_button->width() * dbhs + margin/3,
				    lock_button->y(),
				    exit_button->width() * dbhs, 
				  exit_button->height());
	    break;
	  }
	}
	
	
	desktopbar->setGeometry(exit_button->width() + margin*2/3,
				0, 
				exit_button->width() * dbhs * (number_of_desktops/2) + margin/3, 
				box_height);
	
	control_group->setGeometry(w/2 - (exit_button->width() + margin*2/3 + desktopbar->width())/2,
				   margin,
				   exit_button->width() + margin*2/3 + desktopbar->width(),
				   box_height);
      }
      else {
	// dbrows == 1 
	exit_button->setGeometry(0, 2,
				 box_width-4,
				 box_height-4
				 );
	
	lock_button->setGeometry(exit_button->width(), 
				2,
				 box_width-4, 
				 box_height-4);
	
	for (i=0; (tmp_button = desktopbar->find(i)); i++){
	  tmp_button->setGeometry(i*box_width/2 * dbhs, 
				  0,
				  box_width/2 * dbhs, 
				  box_height-4);
	}
	
	desktopbar->setGeometry(lock_button->x() + lock_button->width() + margin*2/3,
				2, 
				box_width /2 * dbhs * number_of_desktops + margin/3, 
				box_height-4);
	
	control_group->setGeometry(w/2 - (box_width-4 + margin*2/3 + desktopbar->width())/2,
				   margin,
				   2*box_width-8 + margin*2/3 + desktopbar->width(),
				   box_height);

	// we also may have to rescale the pixmaps
	if (pm_scale_factor != 1){
	  float scale = pm_scale_factor;
	  pm_scale_factor *= 2;
	  lock_button->setPixmap(load_pixmap("key.xpm"));
	  exit_button->setPixmap(load_pixmap("exit.xpm"));
	  pm_scale_factor = scale;
	}

      }

      panel_button->setGeometry(margin,
				margin,
				(box_width-(margin/2))/2,
				box_height);

      kde_button->setGeometry(panel_button->x()+panel_button->width(),
				  margin,
				  box_width, box_height);
      
      bound_top_left = control_group->x();
      bound_bottom_right = control_group->x() + control_group->width();

      label_date->setGeometry(w - margin - 2*box_width - 3,
			      3,
			      2*box_width-6, 
			      box_height+2*margin-6);
      label_date->setAlignment( AlignRight|AlignVCenter );
      
      
      ty = margin;
	
      tx = 5+24+12;
      for (i=0; i < nbuttons; i++){
	switch (i){
	case 3:
	  tx = w - 2*box_width - box_width/2;
	  break;
	case 6:
	  tx = w - box_width-3;
	  break;
	}
	entries[i].button->setGeometry(tx, ty, box_width, box_height);
	tx += box_width;
	
      }


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
			       box_width/2 - margin/4,
			       box_height/2 - margin/4
			       );

      lock_button->setGeometry(box_width-exit_button->width(),
			       0,
			       exit_button->width(),
			       exit_button->height());

      for (i=0; (tmp_button = desktopbar->find(i)); i++){
	tmp_button->setGeometry(0,i*box_height/2, 
				box_width, box_height/2);
      }

      desktopbar->setGeometry(0,
			      exit_button->height() + margin*2/3,
			      box_width,
			      (number_of_desktops/2)*box_height);

      control_group->setGeometry(margin,
				 h/2 - (exit_button->height() + margin*2/3 + desktopbar->height())/2,
				 box_width,
				 exit_button->height() + margin*2/3 + desktopbar->height());

      panel_button->setGeometry(margin,
				margin,
				box_width,
				(box_height-(margin/2))/2);

      kde_button->setGeometry(margin,
				  panel_button->y()+panel_button->height(),
				  box_width, box_height);
     

      bound_top_left = control_group->y();
      bound_bottom_right = control_group->y() + control_group->height();


      label_date->setGeometry(3,
			      h - margin - 3-box_height,
			      box_width+2*margin-6, 
			      box_height);

      label_date->setAlignment( AlignHCenter|AlignBottom);


      tx = margin;
      ty = panel_button->y()+panel_button->height();
      for (i=0; i < nbuttons; i++){
	switch (i){
	case 2:
	  ty = h/4;
	  break;
	case 3:
	  ty = h - 4*box_height - box_height/2;
	  break;
	case 6:
	  ty = h - box_height;
	  break;
	}
	entries[i].button->setGeometry(tx, ty, box_width, box_height);
	ty += box_height;
      }

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




    pmenu = new PMenu;
    
    config->setGroup("KDE Desktop Entries");
    QString temp = QDir::homeDirPath() +"/Personal";
    QString personal = config->readEntry("PersonalPath", temp.data() );
    temp = KApplication::kdedir()+"/share/applnk";
    QString kde_apps = config->readEntry("Path", temp.data() );

    pmenu->parse(QDir(kde_apps));
    
    if (personal){

      pmenu->add( new PMenuItem((EntryType) separator) );

      p_pmenu = new PMenu;
      p_pmenu->parse(QDir(personal));
      PMenuItem* pmi = new PMenuItem ;
      pmi->parse(&QFileInfo(personal), p_pmenu);
      
      pmenu->add( pmi );

    }
    
    pmenu_add = new PMenu(*pmenu);
    
    pmenu->add( new PMenuItem(separator) );

    PMenu *panel_menu = new PMenu;
    panel_menu->add( new PMenuItem(add_but, klocale->translate("Add application"), NULL, NULL, pmenu_add,
				   NULL, NULL, new myPopupMenu, FALSE, NULL, 
				   klocale->translate("Add an application or a submenu onto the panel")));
    panel_menu->add( new PMenuItem(prog_com, klocale->translate("Add windowlist"), NULL, NULL, NULL, 
				   this, SLOT(add_windowlist()), NULL, FALSE, NULL, 
				   klocale->translate("Add a windowlist menu onto the panel")) );
    panel_menu->add( new PMenuItem(prog_com, klocale->translate("Configure"), NULL, NULL, NULL, 
				   this, SLOT(configure_panel()), NULL, FALSE, NULL, 
				   klocale->translate("Configure panel")) );
    panel_menu->add( new PMenuItem(prog_com, klocale->translate("Restart"), NULL, NULL, NULL, 
				   this, SLOT(restart()), NULL, FALSE, NULL, 
				   klocale->translate("Restart panel")) );
    pmenu->add( new PMenuItem(submenu, klocale->translate("Panel"), NULL, NULL, panel_menu,
			      NULL, NULL, new myPopupMenu) );
    pmenu->add( new PMenuItem(prog_com, klocale->translate("Lock Screen"), NULL, NULL, NULL,
			      this, SLOT(call_klock()), NULL, FALSE, NULL, 
			      klocale->translate("Lock screen")) );
    pmenu->add( new PMenuItem(prog_com, klocale->translate("Logout"), NULL, NULL, NULL,
			      this, SLOT(ask_logout()), NULL, FALSE, NULL, klocale->translate("Logout")) );

    pmenu->createMenu(new myPopupMenu, this);
    entries[0].popup = pmenu->getQPopupMenu();



    read_in_configuration();
    startTimer( 60000 );
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
	  KFM kfm;
	  QString a = entries[i].pmi->fullPathName().copy();
	  a.prepend("file:");
	  if (entries[i].pmi->getType() == submenu)
	    a.append("/.directory");
	  kfm.openProperties(a);
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

  if (KWM::isMaximized(b->win))
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
   entries[nbuttons++].button = new myPushButton( this ); 
   entries[nbuttons-1].button->installEventFilter( this );
   entries[nbuttons-1].popup = NULL;
   entries[nbuttons-1].pmi = pmi;
   entries[nbuttons-1].swallow = "";
   entries[nbuttons-1].swallow = "";
   entries[nbuttons-1].swallowed = 0;
   entries[nbuttons-1].identity = "";
   entries[nbuttons-1].icon[0] = NULL;
   entries[nbuttons-1].icon[1] = NULL;
   entries[nbuttons-1].icon[2] = NULL;
   entries[nbuttons-1].icon[3] = NULL;
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
       pm->parse(QDir(pmi->fullPathName()));
       PMenuItem* pmi2 = new PMenuItem;
       pmi2->parse(&QFileInfo(pmi->fullPathName()), pm);
       pmi = pmi2;
       pm->createMenu(pmi->getQPopupMenu(), this);
       entries[nbuttons-1].pmi = pmi;
       // end workaround
       
       entries[nbuttons-1].button->setPixmap(create_arrow_pixmap( load_pixmap(pmi->bigIconName())));
     }
     else{
       entries[nbuttons-1].button->setPixmap(load_pixmap(pmi->bigIconName()));

       QFile myfile(pmi->fullPathName());
       if (myfile.exists()){
	 if (myfile.open ( IO_ReadOnly )){
	   QTextStream mystream(&myfile);
	   KConfig pConfig(&mystream );
	   pConfig.setGroup("KDE Desktop Entry");
	   QString aString;
	   if (pConfig.hasKey("SwallowTitle"))
	  entries[nbuttons-1].swallow = pConfig.readEntry("SwallowTitle").copy();
	   if (pConfig.hasKey("SwallowExec")){
	     aString = pConfig.readEntry("SwallowExec").copy();
	     aString.append(" &");
	     system(aString.data());
	   }
	   if (pConfig.hasKey("PanelIdentity")){
	     entries[nbuttons-1].icon[0] = new QPixmap();
	     *(entries[nbuttons-1].icon[0]) = *entries[nbuttons-1].button->pixmap();
	     entries[nbuttons-1].identity = (pConfig.readEntry("PanelIdentity")).copy();
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
  
  if (!panelCurrentlyHidden){
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
    for (i=1;i<=8;i++)
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
    doGeometry();
    layoutTaskbar (); 
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


void kPanel::doGeometry () {

   int w = QApplication::desktop()->width();
   int h = QApplication::desktop()->height();
   int pw = width(); // panel...
   int ph = height();
   int px = x();
   int py = y();
   int mw = 0;       // mini panel...
   int mh = 0;
   int sw = 0;      // standalone...
   int sh = 0; 
   
   if (panelCurrentlyHidden) // i.e. standalone shown
    {
     pw = 0;
     ph = 0;
     px = 0;
     py = 0;
     sw = panel_button_frame_standalone->width();
     sh = panel_button_frame_standalone->height();
     {
       mw = 2*taskbar_height; // miniPanelFrame->width();
       mh = taskbar_height; // miniPanelFrame->height();
     }    
    }
   
   // panel_button_frame_standalone->setGeometry(px, py, sw, sh);
   
   if (orientation == horizontal)
    {
     if (position == top_left)
      {
       if (taskbar_position == top) 
	{
	 taskbar_frame->setGeometry(px+sw+mw, py+ph, w-mw-sw, taskbar_height);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(0, ph+taskbar_height,
				    w, h-ph-taskbar_height));
	}
       else if (taskbar_position == bottom)
	{
	 taskbar_frame->setGeometry(px+mw, h-taskbar_height, 
				    w-mw, taskbar_height);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(0, ph, w, h-ph-taskbar_height));
	}
       else
	{
	 taskbar_frame->setGeometry(px+sw, py+ph+mh,
				    tbhs*taskbar_height, taskbar_height);
	 if (taskbar_position == taskbar_top_left)
	   KWM::setWindowRegion(currentDesktop, 
				QRect(taskbar_frame->width()+sw, ph,
				      w-taskbar_frame->width()-sw, h-ph));
	 else // if (taskbar_position=hidden)
	   KWM::setWindowRegion(currentDesktop, 
				QRect(0, ph, w, h-ph));
	}
      }
     else // (if position == bottom_right) 
      {
       if (taskbar_position == bottom)
	{
	 taskbar_frame->setGeometry(px+mw+sw, h-ph-taskbar_height, 
				    w-mw-sw, taskbar_height);
	   KWM::setWindowRegion(currentDesktop, 
				QRect(0, 0, w, h-ph-taskbar_height));
	}
       else if (taskbar_position == top)
	{
	 taskbar_frame->setGeometry(px+mw, 0, w-mw, taskbar_height);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(0, taskbar_height, w, h-ph-taskbar_height));
	}
       else
	{
	 taskbar_frame->setGeometry(0, mh,tbhs*taskbar_height, 
				    taskbar_height);
	 if (taskbar_position == taskbar_top_left)
	   KWM::setWindowRegion(currentDesktop, 
				QRect(taskbar_frame->width(), 0,
				      w-taskbar_frame->width(), h-ph));
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
	 taskbar_frame->setGeometry(px+pw+mw+sw, py, w-pw-mw-sw, taskbar_height);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(pw, taskbar_frame->height(),
				    w-pw, h-taskbar_frame->height()));
	}
       else if (taskbar_position == bottom)
	{
	 taskbar_frame->setGeometry(px+pw+mw, h-taskbar_height, 
				    w-pw-mw, taskbar_height);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(pw, 0, w-pw, h - taskbar_frame->height()));
	}
       else
	{
	 taskbar_frame->setGeometry(px+pw, py+mh,
				    tbhs*taskbar_height, taskbar_height);
	 if (taskbar_position == taskbar_top_left)
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(pw+taskbar_frame->width(), 0,
 			      w-pw-taskbar_frame->width(), h));
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
	 taskbar_frame->setGeometry(mw, 0, w-pw-mw-sw, taskbar_height);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(0, taskbar_frame->height(),
				    w-pw, h-taskbar_frame->height()));
	}
       else if (taskbar_position == bottom)
	{
	 taskbar_frame->setGeometry(mw, h-taskbar_height, 
				    w-pw-mw, taskbar_height);
	 KWM::setWindowRegion(currentDesktop, 
			      QRect(0, 0,
				    w-pw, h-taskbar_frame->height()));
	}
       else
	{
	 taskbar_frame->setGeometry(0, mh,
				    tbhs*taskbar_height, taskbar_height);
	 if (taskbar_position == taskbar_top_left)
	 KWM::setWindowRegion(currentDesktop, 
 			      QRect(taskbar_frame->width(), 0,
				    w-taskbar_frame->width()-pw, h));
	 else
	 KWM::setWindowRegion(currentDesktop, 
 			      QRect(0, 0,
				    w-pw, h));
	}
      }
    }
   taskbar->resize(taskbar_frame->width(), taskbar_frame->height());
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
  if (config->hasKey("TaskbarFont")){
    QFont tmpfont;
    tmpfont.setRawMode(TRUE);
    tmpfont.setFamily(config->readEntry("TaskbarFont"));
    taskbar->setFont(tmpfont);
    for (tmp_button = taskbar_buttons.first(); tmp_button;
	 tmp_button = taskbar_buttons.next())
      tmp_button->setFont(tmpfont);
    taskbar_height = taskbar->fontMetrics().height()+10;
  }
}
