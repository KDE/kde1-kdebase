// minicli
// Copyright (C) 1997 Matthias Ettrich

#include <qmsgbox.h>
#include <qframe.h>
#include <qdir.h>
#include <qwindefs.h>
#include <stdlib.h>
#include <kapp.h>
#include <X11/Xlib.h>
#include <qlist.h>
#include <qkeycode.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pwd.h>
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

/*
   Function determines whether what the user entered in the minicli
   box is executable or not. This code is eniterly based on the C
   version of the 'which' command in csh/bash. (Dawit A.)
*/
bool isExecutable ( const char *name )
{
	QString test;
    char *path = getenv( "PATH" );
	char *pc = path;
	bool found = false;

	while ( *pc != '\0' && found == false )
	{
		int len = 0;
		while ( *pc != ':' && *pc != '\0' )
		{
			len++;
			pc++;
		}
		char save = *pc;
		*pc = '\0';
        test.resize ( strlen (path) ); // OVERLY CAUTIOUS BUFFER OVER-FLOW PROTECTION !!
		test.sprintf( "%s/%s", pc-len, name);
		*pc = save;
		if (*pc) { pc++; }
		found = ( access(test.data(), 01) == 0 );  /* is it executable ? */
	}
    return found;
}

bool isValidShortURL ( const char * cmd )
{
    // NOTE : By design, this check disqualifies some valid
    // URL's that contain queries and *nix command characters.
    // This is an intentional trade off to best much the URL
    // with a local resource first.  This also allows minicli
    // to behave consistently with the way it does now. (Dawit A.)
    char lastchr = *( cmd + strlen (cmd) - 1 );
    if ( strchr ( cmd, ' ' ) != 0L || strchr ( cmd, ';' ) != 0L )
       return false;
    if ( *cmd  == '/'  ||  lastchr == '&' || lastchr == '.' )
        return  false;
    if ( strchr ( cmd, '<' ) != 0L ||  strchr ( cmd, '>' ) != 0L )
       return false;
    if ( strstr ( cmd, "||" ) != 0L || strstr ( cmd, "&&" ) != 0L )
       return false;
    // This will cut down on cmd being incorrectly matched as a valid short URL.
    // Unless the URL contains at least one '.', the command is disqualified as a
    // candidate for a short URL. Hence, users need to supply protocol for such cases. (Dawit A.)
    if ( strchr ( cmd, '.') == 0L && strstr ( cmd, "localhost" ) == 0L )
       return false;

    return true;
}

void execute ( const char* cmd )
{
  QString tmp;
  // Quoted all URLs (ONLY URLs) so that none of their parts
  // can be mis-interpreted as special characters by a shell.
  // This is an easy way of handling this issue because quotes
  // within URL's have to be encoded per RFC 1738. (Dawit A.)

  // Torben
  // WWW Adress ?
  if ( strnicmp( cmd, "www.", 4 ) == 0 )
  {
      tmp = "kfmclient exec \"http://";
      tmp += cmd;
      cmd = tmp.append("\"").data();
  }
  // FTP Adress ?
  else if ( strnicmp( cmd, "ftp.", 4 ) == 0 )
  {
      tmp = "kfmclient exec \"ftp://";
      tmp += cmd;
      cmd = tmp.append("\"").data();
  }
  // Looks like a KDEHelp thing ?
  else if ( strnicmp( cmd, "info:", 4) == 0 || strnicmp( cmd, "man:", 4) == 0 || cmd[0] == '#' )
  {
      tmp = "kdehelp \"";
      if ( strncmp ( cmd, "##", 2 ) == 0 )
      {
        tmp += "info:(";
        tmp += (strlen (cmd) == 2) ? "dir" : cmd + 2;
        tmp += ")";
      }
      else if ( cmd[0] == '#' )
      {
        tmp += "man:";
        tmp += (strlen (cmd) == 1) ? "(index)" : cmd + 1;
      }
      else
        tmp += cmd;
      cmd = tmp.append("\"").data();
  }

  // Looks like a valid URL ?
  else if ( strnicmp ( cmd, "http://", 7) == 0 || strnicmp ( cmd, "ftp://", 6) == 0  ||
            strnicmp ( cmd, "gopher://", 9 ) == 0 || strnicmp ( cmd, "news:", 5) == 0 ||
            strnicmp ( cmd, "mailto:", 7) == 0 || strnicmp ( cmd, "file:", 5 ) == 0 )
  {
      tmp = "kfmclient exec \"";
      tmp += cmd;
      cmd = tmp.append("\"").data();
  }
  // Local file and directory processing.
  else
  {
      struct stat buff;
      bool isLocalDir = false;  // Check if "cmd" is a local directory
      bool isLocalFile = false; // Check if "cmd" is a local file
      bool isLocalExec = false; // Check if "cmd" is locally executable
      QString uri ( cmd );

      // HOME directory ?
      if ( uri[0] == '~' )
      {
         int len = uri.length();
         int index = uri.find ( "/" );
         if ( len == 1 || index == 1 )
            uri.replace ( 0, 1, QDir::homeDirPath().data() );
         else
         {
           struct passwd *dir;
           dir = (index == -1) ? getpwnam(uri.mid(1,len).data()) : getpwnam(uri.mid(1,index-1).data());
           if ( !dir ) return; // Unknown user
           (index == -1) ? uri.replace (0, len,  dir->pw_dir) : uri.replace (0, index, dir->pw_dir);
         }
         cmd = uri.data();
      }
      // Determine if "cmd" is an absolute path to a local resource
      if ( stat( cmd , &buff ) == 0 )
      {
        isLocalFile = S_ISREG( buff.st_mode );
        isLocalDir = S_ISDIR( buff.st_mode );
      }
      // If "cmd" is not the absolute path to a file or a directory,
      // see if it is executable under the user's $PATH variable.
      if ( !isLocalDir && !isLocalFile )
        isLocalExec = isExecutable ( cmd );

      // Open with kfmclient if "cmd" is a non-executable local resource.
      if ( isLocalDir || ( isLocalFile && !isLocalExec ) )
      {
        tmp = "kfmclient exec \"file:";
        tmp += cmd;
        cmd = tmp.append ( "\"").data();
      }
      // If cmd is NOT a local resource or a valid "shortURL"
      // candidate, append "http://" as the default protocol.
      // FIXME: Make this option configurable !! (Dawit A.)
      else if ( !isLocalExec && isValidShortURL ( cmd ) )
      {
        tmp = "kfmclient exec \"http://";
        tmp += cmd;
        cmd = tmp.append("\"").data();
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

