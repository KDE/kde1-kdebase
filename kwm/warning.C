// klogout
// Copyright (C) 1997 Matthias Ettrich

#include "warning.moc"
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
#include <kcharsets.h>

#include "manager.h"

extern Manager* manager;

extern bool do_not_draw;


KWarning::KWarning( QWidget *parent, const char *name, WFlags f)
  : QDialog(parent, name, False, f){
    setMouseTracking(True);
    frame = new QFrame( this );
    frame->installEventFilter( this );
    frame->setMouseTracking(True);
    frame->setFrameStyle(QFrame::WinPanel | QFrame:: Raised);
    button = new QPushButton(klocale->translate("Ooops!"), this);
    button->setMouseTracking(True);
    button->setDefault( True );
    installEventFilter( this );
    button->installEventFilter( this );
    connect(button, SIGNAL(clicked()), SLOT(ok()));
    label = new QLabel(this);
    label->setAlignment(AlignCenter);
}

void KWarning::SetPointerGrab(QPoint pos){
  QWidget* w = QApplication::widgetAt( pos, true);
  if (!w)
    return;
  if (w->topLevelWidget() == this){
    if (w != mouseGrabber()){
      mouseGrabber()->releaseMouse();
      w->removeEventFilter(this);
      w->installEventFilter(this);
      w->setMouseTracking(true);
      w->grabMouse();
    }
  }
}

bool KWarning::eventFilter( QObject *ob, QEvent * e){
  if (e->type() == Event_MouseButtonPress){
    if (ob->isWidgetType() &&
	!rect().contains(
			 mapFromGlobal(
				       ((QWidget*)ob)->mapToGlobal(
								   ((QMouseEvent*)e)->pos())))){
      ok();
    }
  }
  if (e->type() == Event_KeyPress){
    int a = ((QKeyEvent*)e)->ascii();
    if (a == 3 || a == 7 || a == 27)
      ok();
  }
  if (e->type() == Event_MouseMove){
    QMouseEvent* mev = (QMouseEvent*) e;
    if (ob->isWidgetType()
	&& !(mev->state() & LeftButton)
	&& !(mev->state() & MidButton)
	&& !(mev->state() & RightButton)
	){
      SetPointerGrab(((QWidget*)ob)->mapToGlobal(mev->pos()));
    }
  }
  return False;
}

void KWarning::setText(const char* text, bool with_button){
  label->setText(text);
  label->adjustSize();
  int w = QMAX(label->width() + 20, 100);
  int bh = with_button?80:20;
  setGeometry(QApplication::desktop()->width()/2-w/2,
	      (QApplication::desktop()->height()-label->height()-bh)/2,
	      w, label->height()+bh);
  label->setGeometry(4,10,w-8,label->height());
  if (with_button){
    button->setGeometry(w/2-30, label->height()+20, 60, 30);
    button->show();
  }
  else {
    button->hide();
  }

  frame->setGeometry(0,0, width(), height());
}

bool KWarning::do_grabbing(){
  reactive = manager->current();
  if (reactive)
    reactive->setactive(False);
  manager->darkenScreen();
  XGrabServer(qt_xdisplay());
  do_not_draw = true;
  show();
  XSetInputFocus (qt_xdisplay(), winId(), RevertToParent, CurrentTime);
  if (XGrabKeyboard(qt_xdisplay(), winId(),True,GrabModeAsync,
  		    GrabModeAsync,CurrentTime) != GrabSuccess){
    XUngrabServer(qt_xdisplay());
    return False;
  }
  raise();
  button->grabMouse();
  SetPointerGrab(QCursor::pos());
  button->setFocus();

  return True;
}

void KWarning::ok(){
  release();
  manager->refreshScreen();
}


void KWarning::release(){
  XUngrabServer(qt_xdisplay());
  if (mouseGrabber())
    mouseGrabber()->releaseMouse();
  hide();
  do_not_draw = false;
  if (reactive && manager->hasClient( reactive )){
    reactive->setactive(True);
    XSetInputFocus (qt_xdisplay(), reactive->window,
		    RevertToPointerRoot, CurrentTime);
  }
}

