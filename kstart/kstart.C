/*
 * kstart.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <qdir.h>

#include <kstart.moc>
#include "version.h"

#include <fcntl.h>
#include <kprocess.h>
#include <qregexp.h>

void execute(const char* cmd){
  KShellProcess proc;
  proc << cmd;
  proc.start(KShellProcess::DontCare);
}

KStart::KStart(KWMModuleApplication* kwmmapp_arg,
	       const char* command_arg,
	       const char* window_arg,
	       int desktop_arg,
	       bool activate_arg,
	       bool maximize_arg,
	       bool iconify_arg,
	       bool sticky_arg,
	       int decoration_arg
	       )
  :QObject(){
    kwmmapp = kwmmapp_arg;
    command = command_arg;
    window = window_arg;
    desktop = desktop_arg;
    activate = activate_arg;
    maximize = maximize_arg;
    iconify = iconify_arg;
    sticky = sticky_arg;
    decoration = decoration_arg;

    // just connect to the initialized() signal, we want to be
    // informed if we recieved all existing windows
    connect(kwmmapp, SIGNAL(initialized()), SLOT(initialized()));
    kwmmapp->connectToKWM();
}

void KStart::initialized(){
    // ok, we are initialized. Now connect to window add to get the NEW windows
    connect(kwmmapp, SIGNAL(windowAdd(Window)), SLOT(windowAdd(Window)));
    if (window) {
	KWM::doNotManage(window);
	XSync(qt_xdisplay(), False);
    }
    //finally execute the comand
    execute(command);
}

void KStart::windowAdd(Window w){
    if (window) {
	QString t = KWM::title(w);
	QRegExp r = window;
	if (r.match(t) != -1){
	    applyStyle( w );
	    ::exit(0);
	}
    }
    else {
	// not window specified, just take the first one
	applyStyle( w );
	::exit(0);
    }
}

void KStart::applyStyle(Window w) {
    if (window)
	KWM::prepareForSwallowing(w);
    if (desktop >= 0) {
	if (KWM::desktop(w) != desktop)
	    KWM::moveToDesktop(w, desktop);
    }
    if (maximize) {
	debug("do maximize");
	KWM::doMaximize(w, true);
    }
    if (iconify)
	KWM::setIconify(w, true);
    if (sticky) {
	KWM::setSticky(w, true);
    }
    if (decoration != KWM::normalDecoration) {
	KWM::setDecoration(w, decoration);
	if (decoration & KWM::staysOnTop )
	  XRaiseWindow( qt_xdisplay(), w);
    }

    XSync(qt_xdisplay(), False);
    if (window) {
	XMapWindow(qt_xdisplay(), w);
	XSync(qt_xdisplay(), False);
    }
    if (activate)
      KWM::activate(w);
    XSync(qt_xdisplay(), False);
}




int main( int argc, char *argv[] )
{
  if (argc <= 2) {
      KApplication dummyForI18n(argc,argv);
      printf(KSTART_VERSION);
      printf(i18n(
       "\n Copyright (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)\n"
       "\n Utility to launch legay applications with special KDE window properties"
       "\n such as iconified, maximized, a certain virtual desktop, a special decoration"
       "\n or sticky. Furthermore you can exclude the window from getting the focus or"
       "\n force the window manager to keep the window always on top."
       "\n "
       "\n In addition, the -activate switch will jump to the window even if it is"
       "\n started on a different virtual desktop"
       "\n "
       "\n Usage:"
       "\n %s <command> [-window <regular expression>] [-desktop <number>]"
       "\n              [-sticky] [-iconify] [-maximize] "
       "\n              [-decoration tiny|none] [-activate] [-nofocus] [-staysontop]"
       "\n "
       "\n If you do not specify a regular expression for the windows title,"
       "\n then the very first window to appear will be taken. Not recommended!"
       "\n "
       "\n Example usage:"
       "\n %s \"xclock -geometry 80x80-0+0\" -window xclock \\"
       "\n        -decoration tiny -sticky -nofocus -staysontop"
       "\n puts a tiny decorated, sticky xclock on the top right corner of the screen,"
       "\n that does not even get focus (and therefore has no entry in the taskbar)."
       "\n Note that you can still close it with the right mouse button."
       "\n "
       "\n "), argv[0], argv[0]);

      ::exit(0);
  }
  int desktop = 0;
  char* window = 0;
  bool activate = FALSE;
  bool maximize = FALSE;
  bool iconify = FALSE;
  bool sticky = FALSE;
  int noFocus = 0;
  int staysOnTop = 0;
  int decoration = KWM::normalDecoration;

  for (int i = 2; i < argc; i++)
  {
      if (!strcmp(argv[i],"-version")) {
	  printf(KSTART_VERSION);
	  printf(i18n("\n Copyright (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)\n"));
	  ::exit(0);
      }
    if (!strcmp(argv[i],"-window") && i+1 < argc) {
	window =  argv[++i];
    }
    if (!strcmp(argv[i],"-desktop") && i+1 < argc) {
	QString s =  argv[++i];
	desktop = s.toInt();
    }
    if (!strcmp(argv[i],"-decoration") && i+1 < argc) {
	QString s =  argv[++i];
	if (s == "tiny")
	    decoration = KWM::tinyDecoration;
	if (s == "none")
	    decoration = KWM::noDecoration;
    }
    if (!strcmp(argv[i],"-activate") ) activate = true;
    if (!strcmp(argv[i],"-maximize") ) maximize = true;
    if (!strcmp(argv[i],"-iconify") ) iconify = true;
    if (!strcmp(argv[i],"-sticky") ) sticky = true;
    if (!strcmp(argv[i],"-nofocus") ) noFocus = KWM::noFocus;
    if (!strcmp(argv[i],"-staysontop") ) staysOnTop = KWM::staysOnTop;
  }

  KWMModuleApplication a (argc, argv);

  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);


  new KStart(&a, argv[1], window, desktop, activate, maximize, iconify, sticky, decoration | noFocus | staysOnTop);


  return a.exec();
}
