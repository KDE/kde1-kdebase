/*
 * krootwm.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */



#include "krootwm.moc"
#include "version.h"

#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include <kfm.h>
#include <kprocess.h>

void execute(const char* cmd){
  char* shell = NULL;
  if (!shell){
    if (getenv("SHELL"))
      shell = qstrdup(getenv("SHELL"));
    else
      shell = "/bin/sh";
  }
  KProcess proc;
  proc.setExecutable(shell);
  proc << "-c" << cmd;
  proc.start(KProcess::DontCare);
}


KRootWm::KRootWm(KWMModuleApplication* kwmmapp_arg)
  :QObject(){
    
    kwmmapp = kwmmapp_arg;

    XGCValues gv;
    unsigned long mask;

    XSelectInput(qt_xdisplay(), qt_xrootwin(), 
 		 ButtonPressMask | 
 		 ButtonReleaseMask |
 		 ButtonMotionMask
 		 );
    gv.function = GXxor;
    gv.line_width = 0;
    gv.foreground = WhitePixel(qt_xdisplay(), qt_xscreen())
      ^BlackPixel(qt_xdisplay(), qt_xscreen());
    gv.subwindow_mode = IncludeInferiors;
    mask = GCForeground | GCFunction | GCLineWidth
      | GCSubwindowMode;
	
    gc = XCreateGC(qt_xdisplay(), qt_xrootwin(), mask, &gv);

    rmb = new QPopupMenu;
    rmb->setMouseTracking(TRUE);
    rmb->installEventFilter(this);
    rmb->insertItem(klocale->translate("Help on desktop"), RMB_HELP);
    rmb->insertItem(klocale->translate("Execute command"), RMB_EXECUTE);
    rmb->insertItem(klocale->translate("Display properties"), RMB_DISPLAY_PROPERTIES);
    rmb->insertItem(klocale->translate("Refresh desktop"), RMB_REFRESH_DESKTOP);
    rmb->insertItem(klocale->translate("Arrange icons"), RMB_ARRANGE_ICONS);
    rmb->insertSeparator();
    rmb->insertItem(klocale->translate("Lock screen"), RMB_LOCK_SCREEN);
    rmb->insertItem(klocale->translate("Logout"), RMB_LOGOUT);
    connect(rmb, SIGNAL(activated(int)), this, SLOT(rmb_menu_activated(int)));

    mmb = new QPopupMenu;
    mmb->setMouseTracking(TRUE);
    mmb->installEventFilter(this);
    mmb->setCheckable(TRUE);
    connect(mmb, SIGNAL(activated(int)), this, SLOT(mmb_menu_activated(int)));

    QApplication::desktop()->installEventFilter(this);  

    kwmmapp->connectToKWM();
}

bool KRootWm::eventFilter( QObject *obj, QEvent * ev){
  if (ev->type() == Event_MouseButtonPress
      || ev->type() == Event_MouseButtonDblClick){
    QMouseEvent *e = (QMouseEvent*) ev;
    if (obj == QApplication::desktop()){
      switch (e->button()){
      case LeftButton:
	{
	  int x, y, dx, dy;
	  x = e->pos().x();
	  y = e->pos().y();
	  dx = dy = 0;
	  if (select_rectangle(x,y,dx,dy)){
	    KFM* kfm = new KFM; 
	    kfm->selectRootIcons(x, y, dx, dy,
				 (e->state() & ControlButton) == ControlButton);
	    delete kfm;
	  }
	}
      break;
      case MidButton:
	generateWindowlist(mmb);
	mmb->popup(e->pos());
	break;
      case RightButton:
	rmb->popup(e->pos());
	break;
      }
    }
  }
  return False;
}
	  

void KRootWm::rmb_menu_activated(int item){
  switch (item) {
  case RMB_DISPLAY_PROPERTIES:
    execute("kcc background colors screensaver style");
    break;
  case RMB_ARRANGE_ICONS:
    {
      KFM* kfm = new KFM; 
      kfm->sortDesktop();
      delete kfm;
    }
  break;
  case RMB_EXECUTE:
    KWM::sendKWMCommand("execute");
    break;
  case RMB_REFRESH_DESKTOP:
    {
      KFM* kfm = new KFM; 
      kfm->refreshDesktop();
      KWM::refreshScreen();
      delete kfm;
    }
  break;
  case RMB_HELP:
    execute ("kdehelp");
    break;
  case RMB_LOCK_SCREEN:
    execute ("klock");
    break;
  case RMB_LOGOUT:
    KWM::logout();
    break;
  default:
    // nothing
    break;
  }
}

void KRootWm::mmb_menu_activated(int item){
  if (item>1000){
    KWM::switchToDesktop(item-1000);
  }
  else {
    Window w = callbacklist[item];
    delete [] callbacklist;
    KWM::activate(w);
  }

}


void KRootWm::draw_selection_rectangle(int x, int y, int dx, int dy){
   XDrawRectangle(qt_xdisplay(), qt_xrootwin(), gc, x, y, dx, dy);
   if (dx>2) dx-=2;
   if (dy>2) dy-=2;
   XDrawRectangle(qt_xdisplay(), qt_xrootwin(), gc, x+1, y+1, dx, dy);
}

bool KRootWm::select_rectangle(int &x, int &y, int &dx, int &dy){
    int cx, cy, rx, ry;
    int ox, oy;
    XEvent ev;
    struct timeval t;

    XChangeActivePointerGrab( qt_xdisplay(), 
			      ButtonPressMask | ButtonReleaseMask |
			      PointerMotionMask |
			      EnterWindowMask | LeaveWindowMask,
			      arrowCursor.handle(), 0);
    ox = x;
    oy = y;

    cx = x;
    cy = y;
    
    XGrabServer(qt_xdisplay());
    
    draw_selection_rectangle(x, y, dx, dy);

    t.tv_sec = 0;
    t.tv_usec = 20*1000;

    while (XCheckMaskEvent(qt_xdisplay(), ButtonPress|ButtonReleaseMask, &ev) == 0) {
      rx = QCursor::pos().x();
      ry = QCursor::pos().y();
      if (rx == cx && ry == cy)
	continue;
      cx = rx;
      cy = ry;
      draw_selection_rectangle(x, y, dx, dy);

      if (cx > ox){	
	x = ox;
	dx = cx - x;
      }
      else {
	x = cx;
	dx = ox - x;
      }
      if (cy > oy){	
	y = oy;
	dy = cy - y;
      }
      else {
	y = cy;
	dy = oy - y;
      }
	
      draw_selection_rectangle(x, y, dx, dy);
      XFlush(qt_xdisplay());
      select(0, 0, 0, 0, &t);
      continue;
    }

    draw_selection_rectangle(x, y, dx, dy);
    XUngrabServer(qt_xdisplay());
    return True;
}

void KRootWm::generateWindowlist(QPopupMenu* p){
  p->clear();
  Window *w;
  int i = 0;
  int nw = kwmmapp->windows.count();
  callbacklist = new Window[nw];
  for (w = kwmmapp->windows.first(); w; w = kwmmapp->windows.next())
    callbacklist[i++]=*w;
  int d = 1;
  int nd = KWM::numberOfDesktops();
  int cd = KWM::currentDesktop();
  Window active_window = KWM::activeWindow();
  for (d=1; d<=nd; d++){
    p->insertItem(QString("&")+KWM::getDesktopName(d), 1000+d);
    if (!active_window && d == cd)
      p->setItemChecked(1000+d, TRUE);
    for (i=0; i<nw;i++){
      if (
	  (KWM::desktop(callbacklist[i]) == d
	   && !KWM::isSticky(callbacklist[i])
	   )
	  || 
	  (d == cd && KWM::isSticky(callbacklist[i]))
	  ){
	p->insertItem(QString("   ")+KWM::titleWithState(callbacklist[i]),i);
	if (callbacklist[i] == active_window)
	  p->setItemChecked(i, TRUE);
      }
    }
    if (d < nd)
      p->insertSeparator();
  }
}


int main( int argc, char *argv[] )
{
  KWMModuleApplication a (argc, argv);
  if (argc > 1){
    if (QString("-version") == argv[1]){
      printf(KROOTWM_VERSION);
      printf("\n");
      printf(klocale->translate("Copyright (C) 1997 Matthias Ettrich (ettrich@kde.org)\n"));
      ::exit(0);
    }
    else {
      printf(klocale->translate("Usage:"));
      printf("%s [-version]\n", argv[0]);
    }
    ::exit(1); 
  }
  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);
  KRootWm r(&a);
  return a.exec();
}
