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
#include <sys/stat.h>
#include <sys/wait.h>
#include <kprocess.h>
#include "manager.h"
#include "main.h"

#include "minicli.moc"

extern Manager* manager;
extern bool do_not_draw;

// global history list
QList <char> *history = 0;
QListIterator <char> *it;

void execute( const char* cmd){
  QString tmp;

  // Torben
  // WWW Adress ?
  if ( strncmp( cmd, "www.", 4 ) == 0 ) {
      tmp = "kfmclient openURL http://";
      tmp += cmd;
      cmd = tmp.data();
  }
  // FTP Adress ?
  else if ( strncmp( cmd, "ftp.", 4 ) == 0 ) {
      tmp = "kfmclient openURL ftp://";
      tmp += cmd;
      cmd = tmp.data();
  }
  // Looks like a KDEHelp thing ?
  else if ( strstr( cmd, "man:" ) != 0L || cmd[0] == '#' )
  {
      tmp = "kdehelp \"";
      if ( cmd[0] == '#' )
      {
	  tmp += "man:";
	  tmp += cmd + 1;
      }
      else
	  tmp += cmd;
      tmp += "\"";
      cmd = tmp.data();
  }
  // Looks like an URL ?
  else if ( strstr( cmd, "://" ) != 0L )
  {
      tmp = "kfmclient openURL ";
      tmp += cmd;
      cmd = tmp.data();
  }
  // Usual file or directory
  else
  {
      struct stat buff;
      const char *p = cmd;
      if ( strncmp( p, "file:", 5 ) == 0 )
	  p = p + 5;
      // An absolute path ?
      if ( ( *p == '/') &&
      // Just a document ?
          stat( p, &buff ) == 0 &&
           ( S_ISREG( buff.st_mode ) || S_ISDIR( buff.st_mode ) ) )
      {
	  // Tell KFM to open the document
	  tmp = "kfmclient openURL ";
	  tmp += cmd;
	  cmd = tmp.data();
      }
  }

  KShellProcess proc;
  proc << cmd;
  proc.start(KShellProcess::DontCare);
}


Minicli::Minicli( QWidget *parent, const char *name, WFlags f)
  : QFrame(parent, name, f){
    if (!history){
      history = new QList <char>;

      // read in the saved history list (M.H.)
      KConfig *config = kapp->getConfig();
      QStrList hist;
      config->setGroup("MiniCli");
      config->readListEntry("History", hist);
      //CT 15Jan1999 - limit in-memory history too
      max_hist = config->readNumEntry("HistoryLength", 50);
      for (unsigned int i=0; i< QMIN(hist.count(), max_hist); i++)
        if (strcmp(hist.at(i),"") != 0)
          history->append(qstrdup(hist.at(i)));	

      it = new QListIterator <char> (*history);
      history->append("");
      it->toLast();
    }
    if (!it->isEmpty())
      it->toLast();
    setFrameStyle(QFrame::WinPanel | QFrame:: Raised);
    lineedit = new QLineEdit(this);
    lineedit->installEventFilter( this );
    connect(lineedit, SIGNAL(returnPressed()), SLOT(return_pressed()));
    // Torben
    connect( &kurlcompletion, SIGNAL (setText (const char *)),
	     lineedit, SLOT (setText (const char *)));
    connect ( lineedit, SIGNAL (textChanged(const char *)),
	      &kurlcompletion, SLOT (edited(const char *)));
    label = new QLabel(klocale->translate("Command:"), this);
    label->adjustSize();
    setGeometry(QApplication::desktop()->width()/2 - 200,
		QApplication::desktop()->height()/2 - 20,
		400, 38);
}
void Minicli::resizeEvent(QResizeEvent *){
  label->adjustSize();
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
    else if (((QKeyEvent*)e)->key() == Key_Tab){
      QString s = lineedit->text();
      kurlcompletion.make_completion();
      if (s == lineedit->text())
	commandCompletion();
      return True;
    }
    // Torben
    else if ( ( ((QKeyEvent*)e)->state() == ControlButton ) && ((QKeyEvent*)e)->key() == Key_D ) {
	kurlcompletion.make_rotation();
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
//   XGrabServer(qt_xdisplay());
  do_not_draw = true;
  raise();
  lineedit->setFocus();
  return True;
}
void Minicli::return_pressed(){
  QString s = lineedit->text();
  s.stripWhiteSpace();
  history->removeLast();
  if (s != history->last() && !s.simplifyWhiteSpace().isEmpty()) {
    history->append(qstrdup(s.data()));
  }

  //CT 15Jan1999 - limit in-memory history too
  while (history->count() > max_hist)
    history->removeFirst();
  //CT

  history->append("");
  cleanup();
  if (s == "logout")
    logout();
  else
    execute(s.data());
}
void Minicli::cleanup(){
  it->toLast();
  lineedit->setText("");
//   XUngrabServer(qt_xdisplay());
  lineedit->releaseMouse();
  hide();
  do_not_draw = false;
  if (reactive && manager->hasClient( reactive )){
      reactive->setactive(True);
      XSetInputFocus (qt_xdisplay(), reactive->window,
		      RevertToPointerRoot, CurrentTime);
  }
  XSync(qt_xdisplay(), false);


  // save history list (M.H.)

  KConfig *config = kapp->getConfig();
  config->setGroup("MiniCli");

  //CT  unsigned int max = config->readNumEntry("HistoryLength", 50);
  QStrList hist;
  if (history)
    for (char *entry = history->first(); entry != 0; entry = history->next())
      if (strcmp(entry,"") != 0)
        hist.append(entry);
  while (hist.count() > max_hist)
    hist.removeFirst();

  config->writeEntry("History", hist);
  config->sync();
}

void Minicli::commandCompletion(){
  QListIterator <char> *it;
  it = new QListIterator <char> (*history);
  QString s,t,u;
  s = lineedit->text();
  bool abort = True;

  if (!it->isEmpty()){
    it->toLast();
    while (!it->atFirst()){
      --(*it);
      t = it->current();
      if (t.length()>=s.length() && t.left(s.length()) == s){
	if (t.length()>s.length()){
	  u = t.left(s.length()+1);
	  abort = False;
	}
	it->toFirst();
      }
    }
    if (!abort){
      it->toLast();
      while (!it->atFirst()){
	--(*it);
	t = it->current();
	if (t.length()>=s.length() && t.left(s.length()) == s){
	  if (t.length()<u.length()
	      || t.left(u.length()) != u){
	    abort = True;
	    it->toFirst();
	  }
	}
      }
    }

    if (!abort){
      lineedit->setText(u.data());
      commandCompletion();
    }
  }
  delete it;
  return;
}

