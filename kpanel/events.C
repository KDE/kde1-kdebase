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


void kPanel::kwmInit(){
  if (taskbar_buttons.count()>0)
    restart();
}
  

void kPanel::windowAdd(Window w){
  static QPixmap* defaultpm = NULL;
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
      entries[bi].swallowed = w;
      KWM::prepareForSwallowing(w);
      XSetWindowBackground(qt_xdisplay(),
			   w,
			   entries[bi].button->backgroundColor().pixel());
      XReparentWindow(qt_xdisplay(), w, 
		      entries[bi].button->winId(), 3, 3);
      XResizeWindow(qt_xdisplay(), w,
		    entries[bi].button->width()-6,
		    entries[bi].button->height()-6);
      XMapWindow(qt_xdisplay(),w);
      entries[bi].button->check_rect_for_leave = TRUE;
    }
  }
  
  b->virtual_desktop = KWM::desktop(w);
  layoutTaskbar();
}

void kPanel::windowRemove(Window w){
  myTaskButton* b = taskButtonFromWindow(w);
  if (!b)
    return;
  taskbar_buttons.removeRef(b);
  taskbar->remove(b);
  delete b;
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
    for (i=0; i<nbuttons && entries[i].identity != identity;i++);
    if (i<nbuttons){
      if (entries[i].icon[in])
	entries[i].button->setPixmap(*(entries[i].icon[in]));
    }
  }
}





bool kPanel::eventFilter(QObject *ob, QEvent *ev){

  switch (ev->type()){
    
  case Event_KeyPress: 
    if (info_label->isVisible())
      info_label->hide();
    tipTimer->stop();

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
	write_out_configuration();
      }
      else {
	moving_button = NULL;
      }
    }
  }
  break;
  
  case Event_MouseMove:{
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


void kPanel::mousePressEvent( QMouseEvent */* ev */  ){
  raise();
}


void kPanel::slotDropEvent( KDNDDropZone *_zone ){
  if (_zone == drop_zone){
    QString a = _zone->getData();
    // TODO kde link auf den button legen!!!
    if (a.left(5) == "file:"){
      a = a.right(a.length() - 5);
      if (a.right(1) == "/")
	a.truncate(a.length()-1);
      printf("search for %s\n", a.data());
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
	write_out_configuration();
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
    KFM kfm;
    QString com = entries[i].pmi->fullPathName();
    com.prepend("file:");
    kfm.exec(com.data(),_zone->getData());
    return;
  }
}

void  kPanel::timerEvent( QTimerEvent * ){
  set_label_date();
}

void kPanel::kdisplayPaletteChanged(){
  int i;
  QButton* tmp_button;
  load_and_set_some_fonts();
  layoutTaskbar();
  for (i=0; (tmp_button = desktopbar->find(i)); i++){
    set_button_text(tmp_button,
 		    KWM::getDesktopName(i+1));
   }
}
