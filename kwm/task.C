// ktask
// Copyright (C) 1997 Matthias Ettrich

#include "task.moc"
#include <kapp.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qwindefs.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>

#include <kapp.h>

#include "manager.h"

extern Manager* manager;

extern bool do_not_draw;


Ktask::Ktask( QWidget *parent, const char *name, WFlags f)
  : QDialog(parent, name, False, f){
    setMouseTracking(True);
    frame = new QFrame( this );
    frame->installEventFilter( this );
    frame->setFrameStyle(QFrame::WinPanel | QFrame:: Raised);
    button = new QPushButton(klocale->translate("Switch to"), this);
    button_logout = new QPushButton(klocale->translate("Logout"), this);
    button_cancel = new QPushButton(klocale->translate("Cancel"), this);
    button->setMouseTracking(True);
    button_logout->setMouseTracking(True);
    button_cancel->setMouseTracking(True);
    button->setDefault( True );
    installEventFilter( this );
    button->installEventFilter( this );
    button_logout->installEventFilter( this );
    button_cancel->installEventFilter( this );
    connect(button, SIGNAL(clicked()), SLOT(buttonSelect()));
    connect(button_logout, SIGNAL(clicked()), SLOT(logout()));
    connect(button_cancel, SIGNAL(clicked()), SLOT(cleanup()));
    label = new QLabel(klocale->translate("Current session"), this);
    label->installEventFilter( this );
    label->setAlignment(AlignCenter);

    listbox = new QListBox(this);
    listbox->installEventFilter( this );
    connect(listbox, SIGNAL(selected(int)), SLOT(listboxSelect(int)));

}

void Ktask::prepareToShow(const QStrList* strlist, int active){ 
  int w = 360;
  int h = 0;

  label->setFont(QFont("Helvetica", 14, QFont::Bold));
  label->adjustSize();
  
  listbox->clear();
  listbox->insertStrList(strlist);
  listbox->setCurrentItem(active);
  listbox->show();
  listbox->setFocusPolicy( StrongFocus );

  label->move(w/2-label->width()/2, 15);
  h = label->geometry().bottom() + 15;
  
  listbox->setGeometry(5, h, w-10, 200);
  h = listbox->geometry().bottom() + 10;

  button->setGeometry((w/2-w/8-w/4)/2, 
		      h,
		      w/4, 30);
  button_logout->setGeometry(w/2-w/8, 
		      h,
		      w/4, 30);
  button_cancel->setGeometry(w/2+w/8 + (w/2-w/8-w/4)/2, 
			     h,
			     w/4, 30);
  h = button_cancel->geometry().bottom()+10;
  
  setGeometry(QApplication::desktop()->width()/2 - w/2,
	      QApplication::desktop()->height()/2 - h/2,
	      w, h);
  frame->setGeometry(0,0, w, h);
}



void Ktask::resizeEvent(QResizeEvent *){
}

void Ktask::SetPointerGrab(QPoint pos){
  QWidget* w = QApplication::widgetAt( pos, TRUE);
  if (!w)
    return;
  if (w->topLevelWidget() == this){
    if (w != mouseGrabber()){
      mouseGrabber()->releaseMouse();
      w->removeEventFilter(this);
      w->installEventFilter(this);
      w->grabMouse();
      XChangeActivePointerGrab( qt_xdisplay(), 
 				ButtonPressMask | ButtonReleaseMask |
 				PointerMotionMask |
 				EnterWindowMask | LeaveWindowMask,
 				None, CurrentTime);
    }
  }
}

bool Ktask::eventFilter( QObject *ob, QEvent * e){
  if (e->type() == Event_MouseButtonPress){
    if (ob->isWidgetType() && 
	!rect().contains(
			 mapFromGlobal(
				       ((QWidget*)ob)->mapToGlobal(
								   ((QMouseEvent*)e)->pos())))){
      cleanup();
    }
  }
  if (e->type() == Event_KeyPress){
    int a = ((QKeyEvent*)e)->ascii();
    if (a == 3 || a == 7 || a == 27)
      cleanup();
  }
  if (e->type() == Event_MouseMove){
    QMouseEvent* mev = (QMouseEvent*) e;
    if (ob->isWidgetType()
	&& !(mev->state() & LeftButton)
	&& !(mev->state() & MidButton)
	&& !(mev->state() & RightButton)
	)
      SetPointerGrab(((QWidget*)ob)->mapToGlobal(mev->pos()));
  }
  return False;
}

bool Ktask::do_grabbing(){
  reactive = manager->current();
  if (reactive)
    reactive->setactive(False);
  XGrabServer(qt_xdisplay());
  do_not_draw = TRUE;
  show();
  XSetInputFocus (qt_xdisplay(), winId(), RevertToParent, CurrentTime);
  if (XGrabKeyboard(qt_xdisplay(), winId(),True,GrabModeAsync,
  		    GrabModeAsync,CurrentTime) != GrabSuccess){
    XUngrabServer(qt_xdisplay());
    return False;
  }
  raise();
  listbox->grabMouse();
  XChangeActivePointerGrab( qt_xdisplay(), 
			    ButtonPressMask | ButtonReleaseMask |
			    PointerMotionMask |
			    EnterWindowMask | LeaveWindowMask,
			    None, 0);
  SetPointerGrab(QCursor::pos());
  listbox->setFocus();
  
  return True;
}

void Ktask::buttonSelect(){
  if (listbox->currentItem() != -1){
    listboxSelect(listbox->currentItem());
  }
  else 
    cleanup();
}

void Ktask::logout(){
  cleanup();
  manager->logout();
}

void Ktask::listboxSelect(int index){
  cleanup();
  QString label = listbox->text(index);
  emit changeToClient(label);
}

void Ktask::cleanup(){
  XUngrabServer(qt_xdisplay());
  if (mouseGrabber())
    mouseGrabber()->releaseMouse();
  if (reactive){
    reactive->setactive(True);
    XSetInputFocus (qt_xdisplay(), reactive->window, 
		    RevertToPointerRoot, CurrentTime);
  }
  hide();
  do_not_draw = FALSE;
  XSync(qt_xdisplay(), FALSE);
}


