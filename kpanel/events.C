// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//

#include "kpanel.h"
#include <qapp.h>
#include <qmsgbox.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <qkeycode.h>
#include <kfm.h>
#include <ksimpleconfig.h>


myFrame::myFrame(bool _autoHide, QWidget *parent, const char* name, WFlags f):QFrame(parent, name, f){
  hideTimer = new QTimer(this);
  connect( hideTimer, SIGNAL(timeout()),
	   this, SLOT(hideTimerDone()) );
  autoHide = _autoHide;
  autoHidden = false;
  if (autoHide)
    hideTimer->start(6000, TRUE);
}

void myFrame::enterEvent(QEvent *){
  hideTimer->start(4000, TRUE);
  if (!autoHidden)
    return;
  raise();
  autoHidden = false;
  emit showMe();
  KWM::sendKWMCommand("moduleRaised");
}

void myFrame::hideTimerDone(){
  if (!autoHide)
    return;
  bool do_hide = true;
  // check for popups
  if (XGrabPointer(qt_xdisplay(), qt_xrootwin(), False, 
		   ButtonPressMask | ButtonReleaseMask |
		   PointerMotionMask |
		   EnterWindowMask | LeaveWindowMask,
		   GrabModeAsync, GrabModeAsync, None, 
		   None , CurrentTime) == GrabSuccess){ 
    XUngrabPointer(qt_xdisplay(), CurrentTime);
    XSync(qt_xdisplay(), FALSE);
  }
  else
    do_hide = false;
  if (!do_hide || geometry().contains(QCursor::pos()))
    hideTimer->start(4000, TRUE);
  else {
    autoHidden = true;
    emit hideMe();
  }
}


void kPanel::showTaskbar(){
  doGeometry();
}

void kPanel::hideTaskbar(){
  raise();
  doGeometry();
  KWM::sendKWMCommand("moduleRaised");
}


void kPanel::kwmInit(){
  if (taskbar_buttons.count()>0)
    restart();
}
  

void kPanel::windowAdd(Window w){
  static QPixmap* defaultpm = NULL;

  { 
    // ignore transient windows
    Window trans = None;
    if (XGetTransientForHint(qt_xdisplay(), w, &trans)){
      if (trans != None && trans != qt_xrootwin())
	return;
    }
  }

  int nr = numberOfTaskbarRows();

  myTaskButton* b = new myTaskButton(taskbar);
  b->win = w;
  taskbar_buttons.append(b);

  // calculate a *unique* id.
  int id = 1;
  while (taskbar->find(id))
    id++;
  taskbar->insert(b, id);
  
  if (!defaultpm){
    defaultpm = new QPixmap;
    *defaultpm = KApplication::getKApplication()->getIconLoader()->loadApplicationMiniIcon("mini-default.xpm", 16, 16);
  }
  
  QPixmap pm = KWM::miniIcon(w, 16, 16);
  if (!pm.isNull())
    b->setPixmap(pm);
  else
     b->setPixmap(*defaultpm);
  
  QString t = KWM::titleWithState(w);
  b->setText(t);

  // swallowing?
  int i,bi;
  for (bi=0; bi<nbuttons && (entries[bi].swallow.isEmpty() || 
			     entries[bi].swallowed != 0 ||
			     entries[bi].swallow != t);bi++);
  if (bi<nbuttons){
    for (i=0; i<nbuttons && entries[i].swallowed != w;i++);
    if (i == nbuttons){
      entries[bi].button->setText("");
      entries[bi].swallowed = w;
      KWM::prepareForSwallowing(w);
      XSetWindowBackground(qt_xdisplay(),
			   w,
			   entries[bi].button->backgroundColor().pixel());
      XReparentWindow(qt_xdisplay(), w, 
 		      entries[bi].button->winId(), 3, 3);
      XSelectInput(qt_xdisplay(), w, EnterWindowMask | LeaveWindowMask);
      XResizeWindow(qt_xdisplay(), w,
		    entries[bi].button->width()-6,
		    entries[bi].button->height()-6);
      XMapWindow(qt_xdisplay(),w);

      
      // install a passive grab on this button to ensure that
      // kpanel recieves the button events.
      
      // Exception: do not install a passive grab for the left mouse
      // button if there is no Exec property in the kdelnk file.
      KSimpleConfig pConfig(entries[bi].pmi->fullPathName(),true);
      pConfig.setGroup("KDE Desktop Entry");
      QString aString = pConfig.readEntry("Exec", "");
      if (aString.isEmpty()){
	printf("passive grab ohne LMB!\n");
	XGrabButton(qt_xdisplay(),
		    Button2,
		    AnyModifier, w, True, 
		    ButtonPressMask,
		    GrabModeSync, GrabModeAsync, None, None);
	XGrabButton(qt_xdisplay(),
		    Button3,
		    AnyModifier, w, True, 
		    ButtonPressMask,
		    GrabModeSync, GrabModeAsync, None, None);
      }
      else
	XGrabButton(qt_xdisplay(), AnyButton, AnyModifier, w, True, 
		    ButtonPressMask,
		    GrabModeSync, GrabModeAsync, None, None);
      
      entries[bi].button->swallowed_window = w;
    }
  }
  
  b->virtual_desktop = KWM::desktop(w);
  if (nr != numberOfTaskbarRows())
    doGeometry();
  layoutTaskbar();
}

void kPanel::windowRemove(Window w){
  int nr = numberOfTaskbarRows();
  myTaskButton* b = taskButtonFromWindow(w);
  if (!b)
    return;
  taskbar_buttons.removeRef(b);
  taskbar->remove(b);
  delete b;
  if (nr != numberOfTaskbarRows())
    doGeometry();
  layoutTaskbar();
}
void kPanel::windowChange(Window w){
  myTaskButton* b = taskButtonFromWindow(w);
  if (!b)
    return;
  b->setText(KWM::titleWithState(w));
  int d = KWM::desktop(w);
  if (d != b->virtual_desktop){
    b->virtual_desktop = d;
    layoutTaskbar();
  }
}
void kPanel::windowActivate(Window w){
  myTaskButton* b = taskButtonFromWindow(w);
  if (!b)
    return;
  b->setActive();
}

void kPanel::windowIconChanged(Window w){
  myTaskButton* b = taskButtonFromWindow(w);
  if (!b)
    return;
  QPixmap pm = KWM::miniIcon(w, 16, 16);
  if (!pm.isNull())
    b->setPixmap(pm);
}

void kPanel::windowRaise(Window /* w */){
  if (panel_button_frame_standalone->isVisible())
    panel_button_frame_standalone->raise();
}


void kPanel::layoutDockArea(){
  if (kwmmapp->dock_windows.count() == 0){
    dock_area->hide();
    return;
  }
  Window* w;
  int i;
  if (orientation == vertical){
    dock_area->setGeometry(dock_area->x(),
			   dock_area->y() + dock_area->height()
			   - kwmmapp->dock_windows.count() * 24 - 2,
			   dock_area->width(),
			   kwmmapp->dock_windows.count() * 24 + 2);
    i = 0;
    for (w = kwmmapp->dock_windows.first(); w; 
	 w = kwmmapp->dock_windows.next()){
      XMoveResizeWindow(qt_xdisplay(), *w, 
			dock_area->width()/2-12, 1+ i * 24, 24, 24);
      i++;
    }
  }else {
    dock_area->setGeometry(dock_area->x() + dock_area->width()
			   - kwmmapp->dock_windows.count() * 24 - 2,
			   dock_area->y(),
			   kwmmapp->dock_windows.count() * 24 + 2,
			   dock_area->height());
    i = 0;
    for (w = kwmmapp->dock_windows.first(); w; 
	 w = kwmmapp->dock_windows.next()){
      XMoveResizeWindow(qt_xdisplay(), *w, 
			1 + i * 24, dock_area->height()/2-12, 24, 24);
      i++;
    }
  }
  dock_area->show();
}

void kPanel::dockWindowAdd(Window w){
  XReparentWindow(qt_xdisplay(), w, dock_area->winId(), 0, 0);
  XMapWindow(qt_xdisplay(), w);
  layoutDockArea();
}
void kPanel::dockWindowRemove(Window){
  layoutDockArea();
}

// void kPanel::playSound(QString e){
//   QDateTime d = QDateTime::currentDateTime();
//   printf("sound event: %s (%.2d:%.2d:%.2d)\n", 
// 	 e.data(),
// 	 d.time().hour(),
// 	 d.time().minute(),
// 	 d.time().second()
// 	 );
// }



myTaskButton* kPanel::taskButtonFromWindow(Window w){
  myTaskButton* b;
  for (b=taskbar_buttons.first(); b; b = taskbar_buttons.next()){
    if (b->win == w)
      return b;
  }
  return NULL;
}


void kPanel::kwmDesktopChange(int nd){
  int i;

  if ( edit_button != NULL)
    restore_editbutton( False );
  
  currentDesktop = nd;
  QPushButton* b;
  for (i=0; (b=(QPushButton*)desktopbar->find(i))!=0; i++){
    if ((i+1==currentDesktop && !b->isOn())
	||
	(i+1!=currentDesktop && b->isOn()))
      b->toggle();
  }

  if (panelHidden[currentDesktop])
    hidePanel();
  else
    showPanel();
  
}

void kPanel::kwmDesktopNameChange(int d, QString name){
  QPushButton* b = (QPushButton*)desktopbar->find(d-1);
  if (b){
    set_button_text(b, name);
  }
}

void kPanel::kwmDesktopNumberChange(int n){
  if (n != number_of_desktops)
    restart();
}

void kPanel::kwmCommandReceived(QString com){
  if (com == "kpanel:restart"){
    restart();
  }
  
  if (com == "kpanel:hide")
    hidePanel ();
  if (com == "kpanel:show")
    showPanel ();
  if (com == "kpanel:system")
    showSystem ();
  
  if (com.left(11) == "kpanel:icon"){
    if (com.mid(12, 1) != ":")
      return;
    int in = 0;
    if (com.mid(11,1) == "2") in =  1;
    if (com.mid(11,1) == "3") in =  2;
    if (com.mid(11,1) == "4") in =  3;
    // search for the identity
    QString identity = com.right(com.length() - 13);
    int i;
    for (i=0; i<nbuttons;i++){
      if (entries[i].identity == identity){
	if (entries[i].icon[in])
	  entries[i].button->setPixmap(*(entries[i].icon[in]));
      }
    }
  }

  // Command from krootwm: open GO-menu, Syntax: "kpanel:goxxxxyyyy"
  if (com.length()== 17 && com.left(9) == "kpanel:go"){
    QPoint pos(com.mid( 9, 4).toInt(), com.mid(13, 4).toInt());
    entries[0].popup->popup(pos);
  }

}
  




bool kPanel::eventFilter(QObject *ob, QEvent *ev){

  switch (ev->type()){
    
  case Event_KeyPress: 

    // people requested for this. I do not understand why (Matthias)
    tipSleepTimerDone();
    menu_tool_tips_disabled = true;

    // my preferred way:
//     if (info_label->isVisible())
//       info_label->hide();
//     tipTimer->stop();

    if ( edit_button != NULL && ((QKeyEvent*)ev)->key() == Key_Escape ) {
      restore_editbutton( false );
      return TRUE;
    };
    if ( edit_button != NULL && (((QKeyEvent*)ev)->key() == Key_Return || 
				 ((QKeyEvent*)ev)->key() == Key_Enter)  ) {
      restore_editbutton( true );
      return TRUE;
    };
    break;
    
  case Event_MouseButtonPress: case Event_MouseButtonDblClick: {


    if (info_label->isVisible())
      info_label->hide();
    tipTimer->stop();
    
    if ( ob == panel_button || QString("myTaskButton") == ob->className())
      break;
    QMouseEvent* mev = (QMouseEvent*)ev;

    if (ob == edit_button){
      if (!edit_button->rect().contains(mev->pos())){
	restore_editbutton(false);
	return true;
      }
      break;
    }



    if (ob->isWidgetType() && ((QWidget*)ob)->isPopup()){
      if (mev->button() == RightButton && ob != popup_item
	  && ob != windowlist
	  && ob != taskbarPopup){
	info_label_is_sleeping = FALSE;
	tipTimer->start(0, TRUE);
      }
      else {
	tipTimer->stop();
      }
      break;
    }

    if (mev->button()==MidButton){
      if (ob->isWidgetType() && !((QWidget*)ob)->isPopup()){
	moving_button = (QWidget*)ob;
	moving_button_offset = moving_button->mapToGlobal(mev->pos());
	if (moving_button->parentWidget() == desktopbar ||
	    moving_button->parentWidget() == control_group)
	  moving_button = control_group;
	if (moving_button == panel_button_standalone)
	  moving_button = panel_button_frame_standalone;
	
	position_of_new_item = moving_button->pos();
	moving_button_offset = moving_button->mapFromGlobal(moving_button_offset);
	// minipanel cannot be edited!
	if (moving_button->parentWidget() == miniPanel)
	  moving_button = NULL;
      }
      
      
      if (moving_button){
	if (moving_button != control_group
	    && moving_button != panel_button_frame_standalone){
 	  ((myPushButton*)moving_button)->flat = False;
 	  moving_button->repaint();
	}
	moving_button->raise();
	moving_button->setCursor(sizeAllCursor);
      }
    }
    if (mev->button() == RightButton && ob->isWidgetType() 
	&& !((QWidget*)ob)->isPopup()
	&& QString("myPushButton")!=((QWidget*)ob)->className()){
      QWidget* tmp = (QWidget*)ob;
      moving_button_offset = tmp->mapToGlobal(mev->pos());
	if (tmp->parentWidget() == desktopbar ||
	    tmp->parentWidget() == control_group)
	  tmp = control_group;
	if (tmp == panel_button_standalone)
	  tmp = panel_button_frame_standalone;
	
	moving_button_offset = tmp->mapFromGlobal(moving_button_offset);
	// minipanel cannot be edited!
	if (tmp->parentWidget() == miniPanel)
	  tmp = NULL;
	if (tmp){
	  popup_item->setItemEnabled(0, TRUE);
	  popup_item->setItemEnabled(1, FALSE);
	  popup_item->setItemEnabled(3, FALSE);
	  if (show_popup(popup_item, tmp) == 0){
	    moving_button = tmp;
	    moving_button->raise();
	    moving_button->setCursor(sizeAllCursor);
	    // the next line _IS_ necessary! 
	    XGrabPointer( qt_xdisplay(), moving_button->winId(), FALSE,
			  ButtonPressMask | ButtonReleaseMask |
			  PointerMotionMask | EnterWindowMask | LeaveWindowMask,
			  GrabModeAsync, GrabModeAsync,
			  None, None, CurrentTime );
	  }
	}
    }
  }
  break;


  case Event_MouseButtonRelease: {
    QMouseEvent* mev = (QMouseEvent*)ev;
    tipSleepTimerDone();

    if ( ob == panel_button)
      break;

    // ignore the right button on popups
    // since we use it for tooltips
    if (ob->isWidgetType() && ((QWidget*)ob)->isPopup()
	&& ob != popup_item && ob != windowlist
	&& ob != taskbarPopup
	&& mev->button() == RightButton)
      return TRUE;

    if (moving_button){
      if (moving_button != control_group
	  && moving_button != panel_button_frame_standalone){
 	((myPushButton*)moving_button)->flat = True;
 	moving_button->repaint();
      }
      moving_button->setCursor(arrowCursor);
      XUngrabPointer( qt_xdisplay(),0 );
      // moving_button->releaseMouse();

      if (moving_button != control_group
	  && moving_button != panel_button_frame_standalone)
	check_button_bounds(moving_button);
      else {
	if (orientation == horizontal){
	  bound_top_left = control_group->x();
	  bound_bottom_right = control_group->x() + control_group->width();
	}
	else {
	  bound_top_left = control_group->y();
	  bound_bottom_right = control_group->y() + control_group->height();
	}
	int i;
	for (i=0; i<nbuttons; i++){
	  check_button_bounds(entries[i].button);
	}
      }
      if (moving_button != panel_button_frame_standalone){
	moving_button = NULL;
	reposition();
	writeOutConfiguration();
      }
      else {
	moving_button = NULL;
      }
    }
  }
  break;

  case Event_MouseMove:{
    menu_tool_tips_disabled = false;
    QMouseEvent* mev = (QMouseEvent*)ev;

    if (info_label->isVisible()){
      if (info_label->rect().contains(info_label->mapFromGlobal(QCursor::pos()))){
	info_label->hide();
	tipTimer->stop();
      }
    }



    if (ob->isWidgetType() && ((QWidget*)ob)->isPopup()){
      if ((mev->state() & RightButton) == RightButton  && ob != popup_item
	  && ob != windowlist){
	info_label_is_sleeping = FALSE;
      }
    }

    if (moving_button){
      if (moving_button == panel_button_frame_standalone){
	moving_button->move(QCursor::pos() - moving_button_offset);
      }
      else {
	int x = mapFromGlobal(QCursor::pos()).x()
	  - moving_button_offset.x();
	int y = mapFromGlobal(QCursor::pos()).y()
	  - moving_button_offset.y();
	if (orientation == horizontal){
	  if (x<panel_button->x() + panel_button->width()) 
	    x=panel_button->x() + panel_button->width();
	  if (x + moving_button->width() > width()) 
	    x = width() - moving_button->width ();
	  y = moving_button->y();
	}
	else {
	  if (y<panel_button->y() + panel_button->height()) 
	    y=panel_button->y() + panel_button->height();
	  if (y + moving_button->height() > height()) 
	    y = height() - moving_button->height ();
	  x = moving_button->x();
	}
	moving_button->move(x, y);
      }
    }
  }
  break;
  case Event_Close:
    // this never occurs ?!
    break;
  };
  return FALSE;
}



void kPanel::resizeEvent( QResizeEvent * ){
  //  setGeometry(0,0,QApplication::desktop()->width(), menu->height());

}

void kPanel::enterEvent( QEvent * ){
  hideTimer->start(4000, TRUE);

  if (
      (orientation == horizontal && position == top_left &&
      taskbar_frame->autoHidden && taskbar_position == top)
      ||
      (orientation == horizontal && position == bottom_right &&
      taskbar_frame->autoHidden && taskbar_position == bottom)
      ){
    QEvent ev(Event_Enter);
    QApplication::sendEvent(taskbar_frame, &ev);
  }
  if (!autoHidden)
    return;
  raise();
  if (taskbar_frame->isVisible()){
    taskbar_frame->raise();
  }
  if (orientation == horizontal){
    if (position == top_left)
      move (0,0);
    else
      move (0, QApplication::desktop()->height()-height());
  }
  else {
    if (position == top_left)
      move (0,0);
    else
      move (QApplication::desktop()->width()-width(),0);
    
  }
  doGeometry();
  layoutTaskbar();
  autoHidden = false;
  KWM::sendKWMCommand("moduleRaised");
}
void kPanel::leaveEvent( QEvent * ){
}

void kPanel::hideTimerDone(){
  int bi;
  bool do_hide = true;
  if (!autoHide)
    return;


  // check for popups
  if (XGrabPointer(qt_xdisplay(), qt_xrootwin(), False, 
		   ButtonPressMask | ButtonReleaseMask |
		   PointerMotionMask |
		   EnterWindowMask | LeaveWindowMask,
		   GrabModeAsync, GrabModeAsync, None, 
		   None , CurrentTime) == GrabSuccess){ 
    XUngrabPointer(qt_xdisplay(), CurrentTime);
    XSync(qt_xdisplay(), FALSE);
  }
  else
    do_hide = false;



  for (bi=0; bi<nbuttons; bi++){
    do_hide = do_hide && (entries[bi].button->flat &&
			  !entries[bi].button->isDown());
  }
  do_hide = do_hide && !geometry().contains(QCursor::pos());
  if (!do_hide){
    hideTimer->start(4000, TRUE);
  }
  else {
    if (orientation == horizontal){
      if (position == top_left)
	move (x(), y()-height()+4);
      else
	move (x(), y()+height()-4);
    }
    else {
      if (position == top_left)
	move (x()-width()+4, y());
      else
	move (x()+width()-4, y());
	
    }
    doGeometry();
    layoutTaskbar();
    autoHidden = true;
  }
}

void kPanel::standalonePanelButtonClicked(){
  enterEvent(0);
  showPanel();
}

void kPanel::mousePressEvent( QMouseEvent*  ev  ){
  raise();
  KWM::sendKWMCommand("moduleRaised");
  if (ev->button() == RightButton){
    QPopupMenu* p = new QPopupMenu();
    p->insertItem(klocale->translate("Configure"), 
		  this, SLOT(configurePanel()));
    p->insertItem(klocale->translate("Restart"), 
		  this, SLOT(restart()));
    p->popup(mapToGlobal(ev->pos()));
  }
}


void kPanel::slotDropEvent( KDNDDropZone *_zone ){
  if (_zone == drop_zone){
    QString a = _zone->getData();
    // TODO kde link auf den button legen!!!
    if (a.left(5) == "file:"){
      a = a.right(a.length() - 5);
      if (a.right(1) == "/")
	a.truncate(a.length()-1);
      PMenuItem* pmi = pmenu->searchItem(a);
      
      if (pmi){
	int x = margin;
	int y = margin;
	if (orientation == vertical){
	  y = drop_zone->getMouseY()-box_height/2;
	  if (y<margin) y=margin;
	}
	else{
	  x = drop_zone->getMouseX()-box_width/2;
	  if (x<margin) x=margin;
	}
	addButtonInternal(pmi, x, y);
	writeOutConfiguration();
	return;
      }
    }
    
    QMessageBox::warning( 0, "Panel", 
			  "Cannot put this as button onto the panel!",
			  "Oops!");
    return;
  }


  int i;
  for (i=0;i<nbuttons&&entries[i].drop_zone != _zone;i++);
  if (i<nbuttons && entries[i].pmi){
    KFM* kfm = new KFM;
    QString com = entries[i].pmi->fullPathName();
    com.prepend("file:");
    kfm->exec(com.data(),_zone->getData());
    delete kfm;
    return;
  }
}

void kPanel::kdisplayPaletteChanged(){
  int i;
  QButton* tmp_button;
  load_and_set_some_fonts();
  doGeometry();
  layoutTaskbar();
  for (i=0; (tmp_button = desktopbar->find(i)); i++){
    set_button_text(tmp_button,
 		    KWM::getDesktopName(i+1));
   }
}

QWidget* kPanel::parentOfSwallowed(Window w){
  int bi;
  for (bi=0; bi<nbuttons && entries[bi].swallowed != w; bi++);
  if (bi<nbuttons)
    return entries[bi].button;
  return 0;
}
