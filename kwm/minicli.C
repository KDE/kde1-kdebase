// minicli 
// Copyright (C) 1997 Matthias Ettrich

#include <qmsgbox.h>
#include <qframe.h>
#include <qwindefs.h>
#include <stdlib.h>
#include <kapp.h>
#include <X11/Xlib.h>
#include <qlist.h>
#include <qkeycode.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include "manager.h"

#include "minicli.moc"

extern Manager* manager;
extern bool do_not_draw;

// global history list
QList <char> *history = NULL;
QListIterator <char> *it;

static void catch_child(int){
  int status;
  wait(&status);
  signal(SIGCHLD, catch_child);
}

void execute(const char* cmd){
  static char* shell = NULL;
  if (!shell){
    if (getenv("SHELL"))
      shell = qstrdup(getenv("SHELL"));
    else
      shell = "/bin/sh"; 
  }
  signal(SIGCHLD, catch_child);
  if (!(fork())){ // child
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
    setsid();
    execl(shell, shell, "-c", cmd, NULL);
    exit(1);
  }
}


Minicli::Minicli( QWidget *parent, const char *name, WFlags f)
  : QFrame(parent, name, f){
    if (!history){
      history = new QList <char>;
      history->append("");
      it = new QListIterator <char> (*history);
    }
    if (!it->isEmpty())
      it->toLast();
    setFrameStyle(QFrame::WinPanel | QFrame:: Raised);
    lineedit = new QLineEdit(this);
    lineedit->installEventFilter( this );
    connect(lineedit, SIGNAL(returnPressed()), SLOT(return_pressed()));
    label = new QLabel(klocale->translate("Command:"), this);
    label->adjustSize();
    setGeometry(QApplication::desktop()->width()/2 - 200,
		QApplication::desktop()->height()/2 - 20,
		400, 38);
}
void Minicli::resizeEvent(QResizeEvent *){
  label->move(5, (height()-label->height())/2);
  lineedit->setGeometry(10 + label->width(), 5, 
			width()-15-label->width(), 
			height()-10);
}
bool Minicli::eventFilter( QObject *ob, QEvent * e){
  if (e->type() == Event_MouseMove)
    return True;
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
    else if (((QKeyEvent*)e)->key() == Key_Up){
      if (!it->atFirst())
	--(*it);
      lineedit->setText(it->current());
      return True;
    }
    else if (((QKeyEvent*)e)->key() == Key_Down){
      if (!it->atLast())
	++(*it);
      lineedit->setText(it->current());
      return True;
    }
  }
  return False;
}

bool Minicli::do_grabbing(){
  show();
  reactive = manager->current();
  if (reactive)
    reactive->setactive(False);
  lineedit->setText("");
  XSetInputFocus (qt_xdisplay(), winId(), RevertToParent, CurrentTime);
  if (XGrabKeyboard(qt_xdisplay(), lineedit->winId(),True,GrabModeAsync,
		    GrabModeAsync,CurrentTime) != GrabSuccess)
    return False;
  lineedit->grabMouse();
  XGrabServer(qt_xdisplay());
  do_not_draw = TRUE;
  raise();
  lineedit->setFocus();
  return True;
}
void Minicli::return_pressed(){
  QString s = lineedit->text();
  s.stripWhiteSpace();
  history->removeLast();
  history->append(qstrdup(s.data()));
  history->append("");
  cleanup();
  if (s == "logout")
    manager->logout();
  else
    execute(s.data());
}
void Minicli::cleanup(){
  it->toLast();
  lineedit->setText("");
  XUngrabServer(qt_xdisplay());
  lineedit->releaseMouse();
  if (reactive){
    reactive->setactive(True);
    XSetInputFocus (qt_xdisplay(), reactive->window, 
		    RevertToPointerRoot, CurrentTime);
  }
  hide();
  do_not_draw = FALSE;
  XSync(qt_xdisplay(), FALSE);
}

