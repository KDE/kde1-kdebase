/*
 * manager.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */


#include "manager.moc"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/cursorfont.h>
#include <qregexp.h>
#include <qdatetm.h>

#include <kapp.h>
#include <kwm.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif

#include <config.h>

extern bool kwm_error;

extern MyApp* myapp;
extern Manager* manager;

KGreyerWidget *greyer_widget = 0;

Window root;
GC rootgc;
GC rootfillgc;
// static char stipple_bits[] = {
//   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
//   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
//   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa};

extern bool ignore_badwindow; // for the X error handler
extern bool initting;


Manager::Manager(): QObject(){

  have_border_windows = false;
  has_standalone_menubars = false;
  current_border = None;

  top_border    = 0;
  bottom_border = 0;
  left_border   = 0;
  right_border  = 0;

  manager = this;
  current_desktop = KWM::currentDesktop();
  number_of_desktops = KWM::numberOfDesktops();

  proxy_hints = 0;
  proxy_props = 0;
  proxy_ignore = 0;

  readConfiguration();

  int dummy;
  XGCValues gv;
  GC gc;
  unsigned long mask;

  root = qt_xrootwin();
  default_colormap = DefaultColormap(qt_xdisplay(), qt_xscreen());

  wm_state = XInternAtom(qt_xdisplay(), "WM_STATE", False);
  wm_change_state = XInternAtom(qt_xdisplay(), "WM_CHANGE_STATE", False);
  wm_protocols = XInternAtom(qt_xdisplay(), "WM_PROTOCOLS", False);
  wm_delete_window = XInternAtom(qt_xdisplay(), "WM_DELETE_WINDOW", False);
  wm_take_focus = XInternAtom(qt_xdisplay(), "WM_TAKE_FOCUS", False);
  wm_save_yourself = XInternAtom(qt_xdisplay(), "WM_SAVE_YOURSELF", False);
  kwm_save_yourself = XInternAtom(qt_xdisplay(), "KWM_SAVE_YOURSELF", False);
  wm_client_leader = XInternAtom(qt_xdisplay(), "WM_CLIENT_LEADER", False);
  wm_client_machine = XInternAtom(qt_xdisplay(), "WM_CLIENT_MACHINE", False);
  wm_colormap_windows = XInternAtom(qt_xdisplay(), "WM_COLORMAP_WINDOWS", False);
  _motif_wm_hints = XInternAtom(qt_xdisplay(),"_MOTIF_WM_HINTS",False);

  kwm_current_desktop = XInternAtom(qt_xdisplay(),"KWM_CURRENT_DESKTOP",False);
  kwm_number_of_desktops = XInternAtom(qt_xdisplay(),"KWM_NUMBER_OF_DESKTOPS",False);
  kwm_active_window = XInternAtom(qt_xdisplay(),"KWM_ACTIVE_WINDOW",False);
  kwm_win_iconified = XInternAtom(qt_xdisplay(),"KWM_WIN_ICONIFIED",False);
  kwm_win_sticky = XInternAtom(qt_xdisplay(),"KWM_WIN_STICKY",False);
  kwm_win_maximized = XInternAtom(qt_xdisplay(),"KWM_WIN_MAXIMIZED",False);
  kwm_win_decoration = XInternAtom(qt_xdisplay(),"KWM_WIN_DECORATION",False);
  kwm_win_icon = XInternAtom(qt_xdisplay(),"KWM_WIN_ICON",False);
  kwm_win_desktop = XInternAtom(qt_xdisplay(),"KWM_WIN_DESKTOP",False);
  kwm_win_frame_geometry = XInternAtom(qt_xdisplay(),"KWM_WIN_FRAME_GEOMETRY",False);
  kwm_win_icon_geometry = XInternAtom(qt_xdisplay(),"KWM_WIN_ICON_GEOMETRY",False);

  kwm_command = XInternAtom(qt_xdisplay(), "KWM_COMMAND", False);
  kwm_do_not_manage = XInternAtom(qt_xdisplay(), "KWM_DO_NOT_MANAGE", False);
  kwm_keep_on_top = XInternAtom(qt_xdisplay(), "KWM_KEEP_ON_TOP", False);
  kwm_activate_window = XInternAtom(qt_xdisplay(), "KWM_ACTIVATE_WINDOW", False);
  kwm_maximize_window = XInternAtom(qt_xdisplay(), "KWM_MAXIMIZE_WINDOW", False);

  kwm_window_region_changed = XInternAtom(qt_xdisplay(), "KWM_WINDOW_REGION_CHANGED", False);

  kwm_running = XInternAtom(qt_xdisplay(), "KWM_RUNNING", False);

  // for the modules
  kwm_module = XInternAtom(qt_xdisplay(), "KWM_MODULE", False);
  module_init = XInternAtom(qt_xdisplay(), "KWM_MODULE_INIT", False);
  module_initialized = XInternAtom(qt_xdisplay(), "KWM_MODULE_INITIALIZED", False);
  module_desktop_change = XInternAtom(qt_xdisplay(), "KWM_MODULE_DESKTOP_CHANGE", False);
  module_desktop_name_change = XInternAtom(qt_xdisplay(), "KWM_MODULE_DESKTOP_NAME_CHANGE", False);
  module_desktop_number_change = XInternAtom(qt_xdisplay(), "KWM_MODULE_DESKTOP_NUMBER_CHANGE", False);

  module_win_add = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_ADD", False);
  module_dialog_win_add = XInternAtom(qt_xdisplay(), "KWM_MODULE_DIALOG_WIN_ADD", False);
  module_win_remove = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_REMOVE", False);
  module_win_change = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_CHANGE", False);
  module_win_raise = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_RAISE", False);
  module_win_lower = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_LOWER", False);
  module_win_activate = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_ACTIVATE", False);
  module_win_icon_change = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_ICON_CHANGE", False);
  dock_module = None;
  module_dockwin_add = XInternAtom(qt_xdisplay(), "KWM_MODULE_DOCKWIN_ADD", False);
  module_dockwin_remove = XInternAtom(qt_xdisplay(), "KWM_MODULE_DOCKWIN_REMOVE", False);


  kde_sound_event = XInternAtom(qt_xdisplay(), "KDE_SOUND_EVENT", False);
  kde_register_sound_event = XInternAtom(qt_xdisplay(), "KDE_REGISTER_SOUND_EVENT", False);
  kde_unregister_sound_event = XInternAtom(qt_xdisplay(), "KDE_UNREGISTER_SOUND_EVENT", False);

  qt_sizegrip = XInternAtom(qt_xdisplay(), "QT_SIZEGRIP", False);

  gv.function = GXxor;
  gv.line_width = 0;
  gv.foreground = WhitePixel(qt_xdisplay(), qt_xscreen())^BlackPixel(qt_xdisplay(), qt_xscreen());
  gv.subwindow_mode = IncludeInferiors;
  mask = GCForeground | GCFunction | GCLineWidth
    | GCSubwindowMode;
  gc = XCreateGC(qt_xdisplay(), qt_xrootwin(), mask, &gv);
  rootgc = gc;

  rootfillgc = XCreateGC(qt_xdisplay(), qt_xrootwin(), mask, &gv);
  // the following looks nicer (IMO), but needs some hardware graphics support:

  // Pixmap stipple = XCreateBitmapFromData(qt_xdisplay(), qt_xrootwin(), stipple_bits, 16, 16);
  //  XSetStipple(qt_xdisplay(), rootfillgc, stipple);
  //   XSetFillStyle(qt_xdisplay(), rootfillgc, FillStippled);

  shape = XShapeQueryExtension(qt_xdisplay(), &shape_event, &dummy);

  scanWins();
  if (!current()){
    noFocus();
  }

  {
    long data = 1;
    XChangeProperty(qt_xdisplay(), qt_xrootwin(), kwm_running, kwm_running, 32,
		    PropModeAppend, (unsigned char*) &data, 1);
  }

  delayed_focus_follow_mouse_client = 0;
  enable_focus_follow_mouse_activation = false;

  if (options.ElectricBorder > -1){
    createBorderWindows();
  }

}

// Electric Border Window management. Electric borders allow a user
// to change the virtual desktop by moving the mouse pointer to the
// borders. Technically this is done with input only windows. Since
// electric borders can be switched on and off, we have these two
// functions to create and destroy them.
void Manager::createBorderWindows(){

  if( have_border_windows)
    return;

    current_border = None;

    QRect r = QApplication::desktop()->rect();

    XSetWindowAttributes attributes;
    unsigned long valuemask;
    attributes.event_mask =  (EnterWindowMask | LeaveWindowMask |
			      VisibilityChangeMask);
    valuemask=  (CWEventMask | CWCursor );
    attributes.cursor = XCreateFontCursor(qt_xdisplay(),
					  XC_sb_up_arrow);
    top_border = XCreateWindow (qt_xdisplay(), qt_xrootwin(),
				0,0,
				r.width(),1,
				0,
				CopyFromParent, InputOnly,
				CopyFromParent,
				valuemask, &attributes);
    XMapWindow(qt_xdisplay(), top_border);

    attributes.cursor = XCreateFontCursor(qt_xdisplay(),
					  XC_sb_down_arrow);
    bottom_border = XCreateWindow (qt_xdisplay(), qt_xrootwin(),
				   0,r.height()-1,
				   r.width(),1,
				   0,
				   CopyFromParent, InputOnly,
				   CopyFromParent,
				   valuemask, &attributes);
    XMapWindow(qt_xdisplay(), bottom_border);

    attributes.cursor = XCreateFontCursor(qt_xdisplay(),
					  XC_sb_left_arrow);
    left_border = XCreateWindow (qt_xdisplay(), qt_xrootwin(),
				 0,0,
				 1,r.height(),
				 0,
				 CopyFromParent, InputOnly,
				 CopyFromParent,
				 valuemask, &attributes);
    XMapWindow(qt_xdisplay(), left_border);

    attributes.cursor = XCreateFontCursor(qt_xdisplay(),
					  XC_sb_right_arrow);
    right_border = XCreateWindow (qt_xdisplay(), qt_xrootwin(),
				  r.width()-1,0,
				  1,r.height(),
				  0,
				  CopyFromParent, InputOnly,
				  CopyFromParent,
				  valuemask, &attributes);
    XMapWindow(qt_xdisplay(), right_border);

    have_border_windows = true;
}


// Electric Border Window management. Electric borders allow a user
// to change the virtual desktop by moving the mouse pointer to the
// borders. Technically this is done with input only windows. Since
// electric borders can be switched on and off, we have these two
// functions to create and destroy them.
void Manager::destroyBorderWindows(){

  if( !have_border_windows)
    return;

  if(top_border)
    XDestroyWindow(qt_xdisplay(),top_border);
  if(bottom_border)
    XDestroyWindow(qt_xdisplay(),bottom_border);
  if(left_border)
    XDestroyWindow(qt_xdisplay(),left_border);
  if(right_border)
    XDestroyWindow(qt_xdisplay(),right_border);

  top_border    = 0;
  bottom_border = 0;
  left_border   = 0;
  right_border  = 0;

  have_border_windows = false;

}

// a window wants to move/change size or change otherwise
void Manager::configureRequest(XConfigureRequestEvent *e){

  XWindowChanges wc;
  Client *c;

  c = getClient(e->window);

  XEvent ev;

  if (c && c->window == e->window) { // client already exists for this window

      // compress configure requests
      while (XCheckTypedWindowEvent (qt_xdisplay(), c->window,
				     ConfigureRequest, &ev) ) {
	  if (ev.xconfigurerequest.value_mask == e->value_mask)
	      *e = ev.xconfigurerequest;
	  else {
	      XPutBackEvent(qt_xdisplay(), &ev);
	      break;
	  }
      }

    int x = c->geometry.x();
    int y = c->geometry.y();
    int dx = c->geometry.width();
    int dy = c->geometry.height();


    if (e->value_mask & CWX)
      x = e->x;
    if (e->value_mask & CWY)
      y = e->y;

    switch (c->getDecoration()){
    case KWM::noDecoration:
      if (e->value_mask & CWWidth)
	dx = e->width;
      if (e->value_mask & CWHeight)
	dy = e->height;
      break;
    case KWM::tinyDecoration:
      if (e->value_mask & CWWidth)
	dx = e->width + 2 * BORDER_THIN;
      if (e->value_mask & CWHeight)
	dy = e->height + 2 * BORDER_THIN;
      break;
    default:
      if (e->value_mask & CWWidth)
	dx = e->width + 2 * BORDER;
      if (e->value_mask & CWHeight)
	dy = e->height + 2 * BORDER + TITLEBAR_HEIGHT;
    break;
    }

    c->geometry.setRect(x, y, dx, dy);
	
    if (e->value_mask & CWBorderWidth)
      c->border = e->border_width;


    wc.x = x;
    wc.y = y;
    wc.width = dx;
    wc.height = dy;
    wc.border_width = 0;
    wc.sibling = e->above;
    wc.stack_mode = e->detail;
    e->value_mask |= CWBorderWidth;

    // e->parent can have a wrong (obsolete) meaning here!
    XConfigureWindow(qt_xdisplay(), c->winId(), e->value_mask, &wc);
    switch (c->getDecoration()){
    case KWM::noDecoration:
      wc.x = 0;
      wc.y = 0;
      break;
    case KWM::tinyDecoration:
      wc.x = BORDER_THIN;
      wc.y = BORDER_THIN;
      break;
    default:
      wc.x = BORDER;
      wc.y = BORDER+TITLEBAR_HEIGHT;
      break;
    }

    wc.width = e->width;
    wc.height = e->height;

    XConfigureWindow(qt_xdisplay(), e->window, e->value_mask, &wc);

    // take care about transient windows
    if (e->value_mask & CWStackMode){
      switch (e->detail){
      case Above:
      case TopIf:
 	raiseClient(c);
	break;
      case Below:
      case BottomIf:
 	lowerClient(c);
 	break;
      case Opposite:
      default:
 	break;
      }
    }
	
    sendConfig(c);

  }
  else if (!c) {
    wc.x = e->x;
    wc.y = e->y;
    wc.width = e->width;
    wc.height = e->height;
    wc.border_width = 0;

    wc.sibling = None;
    wc.stack_mode = Above;
    wc.sibling = e->above;
    wc.stack_mode = e->detail;
    e->value_mask |= CWBorderWidth;

    XConfigureWindow(qt_xdisplay(), e->window, e->value_mask, &wc);
  }
}

// a window wants to be mapped
void Manager::mapRequest(XMapRequestEvent *e){
    Client *c;

    c = getClient(e->window);

    if (!c){
      manage(e->window);
    }
    else {
      switch (c->state) {
      case IconicState:
	c->unIconify();
	break;
      case WithdrawnState:
	manage(c->window);
	break;
      case NormalState:
	c->showClient();
	raiseClient(c);
	setWindowState(c, NormalState);
	if (!CLASSIC_FOCUS)
	  activateClient(c);
        break;
      }
    }
}


// a window wants to be unmapped
void Manager::unmapNotify(XUnmapEvent *e){
    Client *c;
    c = getClient(e->window);
    if (c && c->window == e->window && c->winId() == e->event) {
      XGrabServer(qt_xdisplay());
      XEvent ev;
      Bool reparented;
      XSync(qt_xdisplay(), False);
      reparented = XCheckTypedWindowEvent (qt_xdisplay(), c->winId(),
                                           ReparentNotify, &ev);
      if(reparented){
	// if we are reparenting then X will send us a synthetic unmap
	// notify.
	reparentNotify(&(ev.xreparent));
 	XUngrabServer(qt_xdisplay());
	return;
      }

      if (c->unmap_events > 0){
	// kwm is so clever that windows on other than the current
	// virutal desktop are truely unmapped, not just hidden. This
	// saves a lot of CPU time if the window is for example an
	// animation application (video et. al.). Drawback: we can
	// recieve a lot of unmap notifies after a desktop switch
	// which have to be ignored. For that reason each client
	// counts self generated unmap events.
	c->unmap_events--;
 	XUngrabServer(qt_xdisplay());
	return;
      }
      switch (c->state) {
      case IconicState:
	// if we are in iconic state we will only withdraw if it큦 a
	// true send_event request.
  	if (e->send_event) {
	  withdraw(c);
 	}
	break;
      case WithdrawnState:
	withdraw(c);
	break;
      case NormalState:
	withdraw(c);
	break;
      }
      XUngrabServer(qt_xdisplay());
    }
}

// a window has been destroyed, so we have to remove it from our
// data structures.
void Manager::destroyNotify(XDestroyWindowEvent *e){
    Client *c;
    // the window is maybe a module communication or docking window?
    // In this case removeModule and removeDockWindow will remove all
    // references. Otherwise they do nothing. Sames is true for top windows.
    removeModule(e->window);
    removeDockWindow(e->window);
    removeTopWindow(e->window);
    c = getClient(e->window);
    if (c == 0 || c->window != e->window){
        return;
    }
    removeClient(c);
}


// client messages
void Manager::clientMessage(XEvent*  ev){
  Client *c;
  XClientMessageEvent* e = &ev->xclient;

  if (e->message_type == wm_change_state) {
    c = getClient(e->window);
    if (c && e->format == 32 && e->data.l[0] == IconicState) {
      if (c->state == NormalState)
	c->iconify();
    }
  }
  if (e->message_type == kwm_do_not_manage){
    char c[21];
    int i;
    for (i=0;i<20;i++)
      c[i] = e->data.b[i];
    c[i] = '\0';
    QString com = c;
    do_not_manage_titles.append(com);
  }

  if (e->message_type == kwm_command){
    bool forwardToModules = FALSE;
    char c[21];
    int i;
    for (i=0;i<20;i++)
      c[i] = e->data.b[i];
    c[i] = '\0';
    QString com = c;
    if (com == "refreshScreen")
      refreshScreen();
    else if (com == "darkenScreen")
      darkenScreen();
    else if (com == "logout")
      logout();
    else if (com == "commandLine")
      showMinicli();
    else if (com == "execute") // for compatibility
      showMinicli();
    else if (com == "taskManager")
      showTask ();
    else if (com.length()>3 && com.left(3) == "go:"){
      launchOtherWindowManager(com.right(com.length()-3));
    }
    else if (com == "configure"){
      emit reConfigure();
      Client::createGimmick();
      Client* t;
      for (t = clients_sorted.first(); t; t=clients_sorted.next())
	t->reconfigure();
    }
    else if (com == "restart"){
	myapp->cleanup();
	XSync(qt_xdisplay(), false);
	execlp("kwm", "kwm", "-nosession", NULL);
	exit(1);
    }
    else if (Client::operationFromCommand(com) > -1){
      if (current())
	current()->handleOperation(Client::operationFromCommand(com));
    }
    else if (com == "desktop+1") {
      int d = current_desktop + 1;
      if (d > number_of_desktops)
	d -= number_of_desktops;
      switchDesktop(d);
    }
    else if (com == "desktop+2") {
      int d = current_desktop + 2;
      if (d > number_of_desktops)
	d -= number_of_desktops;
      switchDesktop(d);
    }
    else if (com == "desktop-1") {
      int d = current_desktop - 1;
      if (d < 1)
	d += number_of_desktops;
      switchDesktop(d);
    }
    else if (com == "desktop-2") {
      int d = current_desktop - 2;
      if (d < 1)
	d += number_of_desktops;
      switchDesktop(d);
    }
    else if (com == "desktop%%2") {
      if (current_desktop % 2 == 1)
	switchDesktop(current_desktop + 1);
      else
	switchDesktop(current_desktop - 1);
    }
    else if (com.left(7) == "desktop"){
      switchDesktop(com.right(com.length()-7).toInt());
    }
    else if (com == "moduleRaised") {
      raiseElectricBorders();
    }

    //CT 12mar98 commands to rearrange the windows
    else if (com == "deskUnclutter") {
      deskUnclutter();
    }
    else if (com == "deskCascade") {
      deskCascade();
    }
    else if (com == "macStyleOn") {
	myapp->setupSystemMenuBar();
	forwardToModules = TRUE;
    }
    else if (com == "macStyleOff") {
	myapp->removeSystemMenuBar();
	forwardToModules = TRUE;
    }
    else {
	forwardToModules = TRUE;
    }

    if (forwardToModules) {
      // forward unknown command to the modules
      long mask = 0L;
      Window* mw;
      for (mw=modules.first(); mw; mw=modules.next()){
	ev->xclient.window = *mw;
 	if (ev->xclient.window == qt_xrootwin())
 	  mask = SubstructureRedirectMask;
 	XSendEvent(qt_xdisplay(), *mw,
		   False, mask, ev);
      }

    }
  }
  if (e->message_type == kwm_activate_window){
    Window w = (Window) e->data.l[0];
    c = getClient(w);
    if (c && c->state == NormalState){
      switchActivateClient(c,true);
    }
    else if (!c) {
	activateClient(0);
    }
  }
  if (e->message_type == kwm_module){
    Window w = (Window) e->data.l[0];
    if (KWM::isKWMModule(w))
      addModule(w);
    else
      removeModule(w);
  }

  if (e->message_type == kwm_keep_on_top){
      // a (probably unmanaged) window wants to stay on top. Might be
      // kpanel, for instance.
    Window w = (Window) e->data.l[0];
    addTopWindow(w);
  }

  if ( e->message_type == kwm_window_region_changed) {
      // someone informed us about a changed window region, just
      // update the menubars and the maximized windows
      updateMenuBars();
      updateMaximizedWindows();
      sendToModules(kwm_window_region_changed, 0);
  }

  if (e->message_type == kde_sound_event){
      long mask = 0L;
      Window* mw;
      for (mw=modules.first(); mw; mw=modules.next()){
	ev->xclient.window = *mw;
 	if (ev->xclient.window == qt_xrootwin())
 	  mask = SubstructureRedirectMask;
 	XSendEvent(qt_xdisplay(), *mw,
		   False, mask, ev);
      }
  }
  if (e->message_type == kde_register_sound_event){
    char c[21];
    int i;
    for (i=0;i<20;i++)
      c[i] = e->data.b[i];
    c[i] = '\0';
    QString com = c;
    sound_events.append(com);
    long mask = 0L;
    Window* mw;
    for (mw=modules.first(); mw; mw=modules.next()){
      ev->xclient.window = *mw;
      if (ev->xclient.window == qt_xrootwin())
	mask = SubstructureRedirectMask;
      XSendEvent(qt_xdisplay(), *mw,
		 False, mask, ev);
    }
  }

  if (e->message_type == kde_unregister_sound_event){
    char c[21];
    int i;
    for (i=0;i<20;i++)
      c[i] = e->data.b[i];
    c[i] = '\0';
    QString com = c;
    sound_events.remove(com);
    long mask = 0L;
    Window* mw;
    for (mw=modules.first(); mw; mw=modules.next()){
      ev->xclient.window = *mw;
      if (ev->xclient.window == qt_xrootwin())
	mask = SubstructureRedirectMask;
      XSendEvent(qt_xdisplay(), *mw,
		 False, mask, ev);
    }
  }
}

// color map issues
void Manager::colormapNotify(XColormapEvent *e){
  Client *c;
  int i;

  if (e->c_new){
    c = getClient(e->window);
    if (c) {
      c->cmap = e->colormap;
      if (c == current())
	colormapFocus(c);
    }
    else
      for (c = clients.first(); c; c = clients.next())
	for (i = 0; i < c->ncmapwins; i++)
	  if (c->cmapwins[i] == e->window) {
	    c->wmcmaps[i] = e->colormap;
	    if (c == current())
	      colormapFocus(c);
	    return;
	  }
  }
}

// a property of one of the windows changed
void Manager::propertyNotify(XPropertyEvent *e){
  static Atom da[32] = {0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0};
  Atom a;
  int del;
  Client *c;
  int i;
  if (!da[0]){
    for (i=0;i<32;i++){
      QString n;
      n.setNum(i+1);
      n.prepend("KWM_DESKTOP_NAME_");
      da[i] = XInternAtom(qt_xdisplay(), n.data(), False);
    }
  }

  a = e->atom;
  del = (e->state == PropertyDelete);

  if (e->window == qt_xrootwin()){
    // kwm observes a few properties of the root window for an easy
    // communication with all kind of clients.
    if (a == kwm_current_desktop){
      // current desktop changed => switch to the new current desktop
      switchDesktop(KWM::currentDesktop());
    }
    if (a == kwm_number_of_desktops){
      // the number of desktops changed.
      number_of_desktops = KWM::numberOfDesktops();
      sendToModules(module_desktop_number_change, 0, (Window) number_of_desktops);
    }
    for (i=0;i<32;i++){
      if (a == da[i]){
	// a desktop got a new name
	sendToModules(module_desktop_name_change, 0, (Window) i+1);
      }
    }
    return;
  }

  c = getClient(e->window);

  // if the window is none of our clients then just return
  if (c == 0 || c->window != e->window)
    return;

  // check for a couple of standard X11 and additional KDE window properties
  switch (a) {
  case XA_WM_ICON_NAME:
  case XA_WM_NAME:
    if (a== XA_WM_ICON_NAME){
      c->iconname = getTextProperty(c->window, XA_WM_ICON_NAME);
    }
    else
      c->name = getTextProperty(c->window, XA_WM_NAME);
    {
      QString tmp = c->label;
      c->setLabel();
      if (tmp != c->label)
	changedClient(c);
    }
    return;
  case XA_WM_TRANSIENT_FOR:
    // this probably never happens. Care anyway...
    getWindowTrans(c);
    if(c->trans != None){
      int i;
      if (c->buttons[0] && c->buttons[0] != c->buttonMenu)
	c->buttons[0]->hide();
      if (c->buttons[3] && options.buttons[3] != CLOSE)
	c->buttons[3]->hide();
      for (i=1; i<6; i++){
	if ( i != 3 && c->buttons[i])
	  c->buttons[i]->hide();
      }
    }
    return;
  case XA_WM_NORMAL_HINTS:
    getWMNormalHints(c);
    return;
  case XA_WM_COMMAND:
    c->command = xgetprop(c->window, XA_WM_COMMAND);
    return;
  }
  if (a == wm_colormap_windows) {
    getColormaps(c);
    if (c == current())
      colormapFocus(c);
  }
  else if (a == wm_protocols){
    getWindowProtocols(c);
  }
  else if (a == kwm_win_icon || a == XA_WM_HINTS){
    QPixmap pm = KWM::miniIcon(c->window, 14, 14);
    if (!pm.isNull() && c->buttonMenu)
      c->buttonMenu->setPixmap(pm);
    sendToModules(module_win_icon_change, c);
  }
  else if (a == kwm_win_iconified){
    if (KWM::isIconified(c->window))
      c->iconify();
    else
      c->unIconify();
  }
  else if (a == kwm_win_desktop){
    c->ontoDesktop(KWM::desktop(c->window));
  }
  else if (a == kwm_maximize_window ){
      if (KWM::isDoMaximize(c->window)) {
	  if (!c->isOnDesktop( current_desktop) )
	      switchDesktop(c->desktop);
	  c->maximize( KWM::doMaximizeMode(c->window), false);
	  KWM::doMaximize(c->window, false);
      }
  }
  else if (a == kwm_win_maximized ){
    if (c->isMaximized() != KWM::isMaximized(c->window)){
 	if (!c->isOnDesktop( current_desktop) )
 	    switchDesktop(c->desktop);
 	if (c->isMaximized())
 	    c->unMaximize();
 	else
 	    c->maximize( KWM::maximizeMode(c->window) );
    }
  }
  else if (a == kwm_win_sticky){
    if (c->isSticky() != KWM::isSticky(c->window)){
      c->buttonSticky->toggle();
    }
  }
  else if (a == kwm_win_decoration){
    // TODO this needs a cleanup very very very soon!!!
    long d = KWM::getDecoration(c->window);
    long dec = d & 255;
    if (c->getDecoration() != dec){
      if (dec){
	c->decoration = dec;
	gravitate(c, false);
	sendConfig(c);
	c->reconfigure();
      }
      else {
	gravitate(c, true);
	c->decoration = dec;
	sendConfig(c);
	c->reconfigure();
      }
    }

    c->stays_on_top = d & KWM::staysOnTop;

    bool is_menubar = (d & KWM::standaloneMenuBar) != 0;
    has_standalone_menubars |= is_menubar;
    c->is_menubar = is_menubar;
    bool wants_focus = (d & KWM::noFocus) == 0 && !is_menubar;
    if (!wants_focus && c->wantsFocus()){
      clients_traversing.removeRef(c);
      sendToModules(module_win_remove, c);
      c->wants_focus = false;
      c->hidden_for_modules = true;
    } else if (wants_focus && !c->wantsFocus()){
      if (c->hidden_for_modules){
	clients_traversing.insert(0,c);
	c->hidden_for_modules = false;
	if ( c->isDialog() )
	    sendToModules(module_dialog_win_add, c);
	sendToModules(module_win_add, c);
      }
      c->wants_focus = true;
    }
  }
}

// a window may be a shaped window
void Manager::shapeNotify(XShapeEvent *e){
  Client *c;
  c = getClient(e->window);
  if (c){
      if (hasShape(c)){
	  setShape(c);
      }
      else {	
	  XShapeCombineMask( qt_xdisplay(), c->winId(), ShapeBounding, 0, 0,
			       None, ShapeSet);
	  c->decoration = KWM::normalDecoration;
	  KWM::setDecoration(c->window, c->getDecoration());
	  gravitate(c, false);
	  sendConfig(c);
	  c->reconfigure();
      }
  }
}

// notification of reparenting events
void Manager::reparentNotify(XReparentEvent* e){
  // if sombody else reparents a window which is managed by kwm, we
  // have to give it free.
  Client* ec = getClient(e->event);
  if (!ec)
    return;
  Client* c = getClient(e->window);
  if (c && c==ec && e->parent != ec->winId())
    withdraw(c);
}

// notification of pointer movements
void Manager::motionNotify(XMotionEvent* e){
  Client *c;
  // store the information (user moved the mouse around). This will be
  // questioned in the enterNotify handler to determine, wether the
  // focus should be set (if in FocusFollowMouse policy)
  enable_focus_follow_mouse_activation = true;
  if (options.FocusPolicy == FOCUS_FOLLOWS_MOUSE){
    c = getClient(e->window);
    if (c && c == delayed_focus_follow_mouse_client && c != current()
	&& c->state != WithdrawnState){
      activateClient(c);
      raiseSoundEvent("Window Activate");
    }
    delayed_focus_follow_mouse_client = 0;
  }
}

// the pointer entered one of our windows. We may have to activate a
// window (when focus follows mouse) or handle electric borders.
void Manager::enterNotify(XCrossingEvent *e){
  Client *c;

  if (CLASSIC_FOCUS){
    // classic focus follows mouse is simple: if a window has been
    // entered that does not have the focus, it is activated. This
    // breaks a lot, including Alt-Tab, but some users seem to want it
    // this way...
    // Same applies to classic sloppy focus
    c = getClient(e->window);
    if (c != 0 && c != current() && c->state != WithdrawnState){
      activateClient(c);
      raiseSoundEvent("Window Activate");
    }
  }
  else if (options.FocusPolicy == FOCUS_FOLLOWS_MOUSE){
    // focus follows mouse is somewhat tricky. kwm does not do it so
    // simple that the focus is always under the mouse. Instead the
    // focus is only moved around if the user explitely moves the mouse
    // pointer.  This allows users to use the advanced alt-tab feature
    // in all focus policies. Also the desktop switching is able to
    // restore the focus correctly.
    delayed_focus_follow_mouse_client = 0;
    c = getClient(e->window);
    if (c != 0 && c != current() && c->state != WithdrawnState){
      XSync(qt_xdisplay(), False);
      timeStamp();
      XSync(qt_xdisplay(), False);
      if (enable_focus_follow_mouse_activation){
	// the user really moved the mouse around => activate
	activateClient(c);
	raiseSoundEvent("Window Activate");
      }
      else {
	if (e->x_root != QCursor::pos().x()
	    || e->y_root != QCursor::pos().y()){
	  // the mouse pointer definitely moved around => activate
	  activateClient(c);
	  raiseSoundEvent("Window Activate");
	}
	else{
	  // no mouse movement so far. Just wait a second.....
	  usleep(100);
	  // ... and check again.
	  if (e->x_root != QCursor::pos().x()
	      || e->y_root != QCursor::pos().y()){
	    // the mouse pointer definitely moved around => activate
	    activateClient(c);
	    raiseSoundEvent("Window Activate");
	  }
	  else {
	    // maybe some mouse movement will follow later. Store the client
	    // in delayed_focus_follow_mouse_client until then.
	    delayed_focus_follow_mouse_client = c;
	  }
	}
      }
    }
  }

  if (have_border_windows &&
      (e->window == top_border ||
       e->window == left_border ||
       e->window == bottom_border ||
       e->window == right_border)){
    // the user entered an electric border
    electricBorder(e->window);
  }
}

// the pointer left one of our windows. This will stop a possible
// autoraise process.The brain-dead classic focus follows mouse policy
// will also take the focus away from it.
void Manager::leaveNotify(XCrossingEvent * e){
  Client* c;
  if (options.FocusPolicy != CLICK_TO_FOCUS){
    c = getClient(e->window);
    if (c != 0 && c == current() && c->winId() == e->window &&
	c->state != WithdrawnState && !c->geometry.contains(QPoint(e->x_root, e->y_root))){
      c->stopAutoraise(false);
      if (options.FocusPolicy == CLASSIC_FOCUS_FOLLOWS_MOUSE)
	noFocus();
    }
  }
}

// switch to another virtual desktop according to the specified direction.
// Useful for keyboard shortcuts or electric borders.
void Manager::moveDesktopInDirection(DesktopDirection d, Client* c, bool move_pointer){

  int nd;

  switch (d){

  case Up:
    if (current_desktop % 2 == 1)
      nd = current_desktop + 1;
    else
      nd = current_desktop - 1;
    if (c){
      c->desktop = nd;
      KWM::moveToDesktop(c->window, c->desktop);
      changedClient(c);
    }
    switchDesktop(nd);

    if (!move_pointer)
      return;
    if(options.ElectricBorderPointerWarp == FULL_WARP){
      QCursor::setPos(QCursor::pos().x(), QApplication::desktop()->height()-3);
    }
    else if (options.ElectricBorderPointerWarp == MIDDLE_WARP){
      QCursor::setPos(QCursor::pos().x(), QApplication::desktop()->height()/2);
    }
    else{
      QCursor::setPos(QCursor::pos().x(), 3);
    }

    break;

  case Down:

    if (current_desktop % 2 == 1)
      nd = current_desktop + 1;
    else
      nd = current_desktop - 1;
    if (c){
      c->desktop = nd;
      KWM::moveToDesktop(c->window, c->desktop);
      changedClient(c);
    }
    switchDesktop(nd);

    if (!move_pointer)
      return;
    if(options.ElectricBorderPointerWarp == FULL_WARP){
      QCursor::setPos(QCursor::pos().x(),3);
    }
    else if (options.ElectricBorderPointerWarp == MIDDLE_WARP){
      QCursor::setPos(QCursor::pos().x(), QApplication::desktop()->height()/2);
    }
    else{
      QCursor::setPos(QCursor::pos().x(), QApplication::desktop()->height()-3);
    }

    break;

  case Left:

    nd = current_desktop - 2;
    if (nd < 1)
	nd += number_of_desktops;
    if (c){
      c->desktop = nd;
      KWM::moveToDesktop(c->window, c->desktop);
      changedClient(c);
    }
    switchDesktop(nd);

    if (!move_pointer)
      return;
    if(options.ElectricBorderPointerWarp == FULL_WARP){
      QCursor::setPos(QApplication::desktop()->width()-3, QCursor::pos().y());
    }
    else if (options.ElectricBorderPointerWarp == MIDDLE_WARP){
      QCursor::setPos(QApplication::desktop()->width()/2, QCursor::pos().y());
    }
    else{
      QCursor::setPos(3, QCursor::pos().y());
    }

    break;

  case Right:

    nd = current_desktop + 2;
    if (nd > number_of_desktops)
      nd -= number_of_desktops;
    if (c){
      c->desktop = nd;
      KWM::moveToDesktop(c->window, c->desktop);
      changedClient(c);
    }
    switchDesktop(nd);

    if (!move_pointer)
      return;
    if(options.ElectricBorderPointerWarp == FULL_WARP){
      QCursor::setPos(3,QCursor::pos().y());
    }
    else if (options.ElectricBorderPointerWarp == MIDDLE_WARP){
      QCursor::setPos(QApplication::desktop()->width()/2, QCursor::pos().y());
    }
    else{
      QCursor::setPos(QApplication::desktop()->width()-3, QCursor::pos().y());
    }
    break;
  }
}

// this function is called when the user entered an electric border
// with the mouse. It may switch to another virtual desktop
void Manager::electricBorder(Window border){
  static QTime electric_time;
  if(!have_border_windows || options.ElectricBorder < 0)
    return; // no electric borders

  if (current_border == border){
    current_border_push_count++;
    if (electric_time.msecsTo(QTime::currentTime())>options.ElectricBorder
	&& current_border_push_count < options.ElectricBorderNumberOfPushes){
      current_border_push_count = 1;
      electric_time = QTime::currentTime();
    }
    if (current_border_push_count > options.ElectricBorderNumberOfPushes){
      if (current_border == top_border){
	moveDesktopInDirection(Up);
      }
      else if (current_border == bottom_border){
	moveDesktopInDirection(Down);
      }
      else if (current_border == left_border){
	moveDesktopInDirection(Left);
      }
      else if (current_border == right_border){
	moveDesktopInDirection(Right);
      }
      current_border = 0;
    }
  }
  else {
    current_border = border;
    current_border_push_count = 1;
    electric_time = QTime::currentTime();
  }

  // reset the pointer to find out wether the user is really pushing
  QPoint pos =QCursor::pos();
  if (current_border == top_border){
    QCursor::setPos(pos.x(), pos.y()+1);
  }
  else if (current_border == bottom_border){
    QCursor::setPos(pos.x(), pos.y()-1);
  }
  else if (current_border == left_border){
    QCursor::setPos(pos.x()+1, pos.y());
  }
  else if (current_border == right_border){
    QCursor::setPos(pos.x()-1, pos.y());
  }
}

// electric borders (input only windows) have to be always on the
// top. For that reason kwm calls this function always after some
// windows have been raised.
void Manager::raiseElectricBorders(){

  if(have_border_windows){
    XRaiseWindow(qt_xdisplay(), top_border);
    XRaiseWindow(qt_xdisplay(), left_border);
    XRaiseWindow(qt_xdisplay(), bottom_border);
    XRaiseWindow(qt_xdisplay(), right_border);
  }
}


void Manager::readConfiguration(){
  KConfig* config = kapp->getConfig();

  // decoration tricks to be somewhat compatible with traditional window managers
  no_decoration_titles.clear();
  tiny_decoration_titles.clear();
  no_decoration_classes.clear();
  tiny_decoration_classes.clear();
  config->setGroup( "Decoration" );
  config->readListEntry("noDecorationTitles", no_decoration_titles);
  config->readListEntry("tinyDecorationTitles", tiny_decoration_titles);
  config->readListEntry("noDecorationClasses", no_decoration_classes);
  config->readListEntry("tinyDecorationClasses", tiny_decoration_classes);

  // same for supressing focus
  no_focus_titles.clear();
  no_focus_classes.clear();
  config->setGroup( "Focus" );
  config->readListEntry("noFocusTitles", no_focus_titles);
  config->readListEntry("noFocusClasses", no_focus_classes);

  //CT 03Nov1998 sticky windows
  sticky_titles.clear();
  sticky_classes.clear();
  config->setGroup( "Sticky");
  config->readListEntry("stickyTitles", sticky_titles);
  config->readListEntry("stickyClasses", sticky_classes);
  //CT
}

  // kwm supports different kinds of window placement. doPlacement
  // will select the appropriate method.
void Manager::doPlacement(Client* c){
  if((options.Placement == SMART_PLACEMENT)  ||
     (options.Placement == MANUAL_PLACEMENT) ||
     (options.interactive_trigger >= 0))
    smartPlacement(c);
  else if(options.Placement == CASCADE_PLACEMENT)
    cascadePlacement(c, False);
  else
    randomPlacement(c);
}


void Manager::randomPlacement(Client* c){
  static int px = TITLEBAR_HEIGHT + BORDER;
  static int py = 2 * TITLEBAR_HEIGHT + BORDER;
  int tx,ty;

  QRect maxRect = KWM::getWindowRegion(currentDesktop());
  if (myapp->systemMenuBar) {
      maxRect.setTop(myapp->systemMenuBar->geometry().bottom());
  }

  if (px < maxRect.x())
    px = maxRect.x();
  if (py < maxRect.y())
    py = maxRect.y();

  px += TITLEBAR_HEIGHT + BORDER;
  py += 2*TITLEBAR_HEIGHT + BORDER;

  if (px > maxRect.width()/2)
    px =  maxRect.x() + TITLEBAR_HEIGHT + BORDER;
  if (py > maxRect.height()/2)
    py =  maxRect.y() + TITLEBAR_HEIGHT + BORDER;
  tx = px;
  ty = py;
  if (tx + c->geometry.width() > maxRect.right()){
    tx = maxRect.right() - c->geometry.width();
    if (tx < 0)
      tx = 0;
    px =  maxRect.x();
  }
  if (ty + c->geometry.height() > maxRect.bottom()){
    ty = maxRect.bottom() - c->geometry.height();
    if (ty < 0)
      ty = 0;
    py =  maxRect.y();
  }
  c->geometry.moveTopLeft(QPoint(tx, ty));
}

/* cascadePlacement by Cristian Tibirna (ctibirna@gch.ulaval.ca).
 * Attempts to place the windows in cascade on each desktop, according
 * with their height and weight (30jan98)
 */
void Manager::cascadePlacement (Client* c, bool re_init) {

  // buffers: will keep x-es and y-es on each desktop
  static int x[] = {0,0,0,0,0,0,0,0};
  static int y[] = {0,0,0,0,0,0,0,0};
  static unsigned short int col[] = {0,0,0,0,0,0,0,0};
  static unsigned short int row[] = {0,0,0,0,0,0,0,0};

  // work coords
  int xp, yp;

  int delta_x = 2 * TITLEBAR_HEIGHT + BORDER;
  int delta_y = TITLEBAR_HEIGHT + BORDER;

  int d = currentDesktop() - 1;

  // get the maximum allowed windows space and desk's origin
  QRect maxRect = KWM::getWindowRegion(currentDesktop());
  if (myapp->systemMenuBar) {
      maxRect.setTop(myapp->systemMenuBar->geometry().bottom());
  }

  // initialize often used vars: width and height of c; we gain speed
  int ch = c->geometry.height();
  int cw = c->geometry.width();
  int H = maxRect.bottom();
  int W = maxRect.right();
  int X = maxRect.left();
  int Y = maxRect.top();

  //initialize if needed
  if (re_init) {
    x[d] = X;
    y[d] = Y;
    col[d] = row[d] = 0;
  }


  xp = x[d];
  yp = y[d];

  //here to touch in case people vote for resize on placement
  if ((yp + ch ) > H) yp = Y;

  if ((xp + cw ) > W)
    if (!yp) {
      smartPlacement(c);
      return;
    }
    else xp = X;

  //if this isn't the first window
  if ((!x[d]) && (!y[d])) {
    if (xp)
      if (!yp) xp = delta_x * (++col[d]);

    if (yp)
      if (!xp) yp = delta_y * (++row[d]);

    // last resort: if still doesn't fit, smart place it
    if ( ((xp + cw) > W) || ((yp + ch) > H) ) {
      smartPlacement(c);
      return;
    }
  }

  if (xp == 0) xp = X;
  if (yp == 0) yp = Y;
  // place the window
  c->geometry.moveTopLeft(QPoint(xp,yp));

  // new position
  x[d] = xp + delta_x;
  y[d] = yp + delta_y;

}

/* SmartPlacement by Cristian Tibirna (ctibirna@gch.ulaval.ca)
 * adapted for kwm (16-19jan98) after an implementation for fvwm by
 * Anthony Martin (amartin@engr.csulb.edu).
 *
 * This function will place the window of a new client such that there is
 * a minimum overlap with already existant windows on the current desktop.
 */

void Manager::smartPlacement(Client* c) {

  int x, y, overlap;
  int xopt, yopt, over_min;
  int other, temp;
  Client* l;

  // initialize often used vars: width and height of c; we gain speed
  int ch = (c->isShaded()?
	    (TITLEBAR_HEIGHT+2*BORDER):c->geometry.height());
  int cw = c->geometry.width();

  // get the maximum allowed windows space
  QRect maxRect = KWM::getWindowRegion(currentDesktop());

  if (myapp->systemMenuBar) {
      maxRect.setTop(myapp->systemMenuBar->geometry().bottom());
  }

  x = maxRect.left(); y = maxRect.top();

  //initialize and do a loop over possible positions
  overlap = -1;
  spGetOverlap(c, x, y, &overlap);
  over_min = overlap;
  xopt = x;
  yopt = y;

  while((overlap != 0) && (overlap != -1)) {
    // test if windows overlap ...
    if (overlap > 0) {

      other = maxRect.right();
      temp = other - cw;

      if(temp > x) other = temp;

      // compare to the position of each client on the current desk
      for(l = clients.first(); l ; l = clients.next()) {
	if(!l->isOnDesktop(currentDesktop()) ||
	   //12mar98 kill bug about avoiding iconified windows
	   (l->isIconified()) ||
	   (l == c))
	  continue;
	// if not enough room above or under the current tested client
	// determine the first non-overlapped x position
	if((y < (l->isShaded()?
		 (TITLEBAR_HEIGHT+2*BORDER):
		 l->geometry.height()) + l->geometry.y() ) &&
	   (l->geometry.y() < ch + y)) {
	  temp = l->geometry.width() + l->geometry.x();
	  if(temp > x) other = (other < temp)? other : temp;
	  temp = l->geometry.x() - cw;
	  if(temp > x) other = (other < temp)? other : temp;
	}
      }
      x = other;
    }

    // ... else => not enough x dimension (overlap was -2)
    else {
      x = maxRect.left();

      other = maxRect.bottom();
      temp = other - ch;

      if(temp > y) other = temp;

      //test the position of each window on current desk
      //07mar98. fixed bug which made iconified windows avoided as if visible
      for(l = clients.first(); l ; l = clients.next()) {
	if(!l->isOnDesktop(currentDesktop()) || (l == c) || c->isIconified())
	  continue;
	
	temp = (l->isShaded()?
		(TITLEBAR_HEIGHT+2*BORDER):
		l->geometry.height()) + l->geometry.y();
	if(temp > y) other = (other < temp)? other : temp;
	temp = l->geometry.y() - ch;
	if(temp > y) other = (other < temp)? other : temp;
      }
      y = other;
    }

    overlap = over_min;
    spGetOverlap(c, x, y, &overlap);

    // store better x and y corresponding to the so far smallest overlap
    if ((overlap >= 0) && (overlap < over_min)) {
      over_min = overlap;
      xopt = x;
      yopt = y;
    }
  }

  //CT 07mar98 verify whether to place interactively or not
  if(options.interactive_trigger >= 0) {
    if(options.interactive_trigger <
       (over_min*100/(maxRect.height() * maxRect.width())))
      options.Placement = MANUAL_PLACEMENT;
    else options.Placement = SMART_PLACEMENT;
  }

  // place the window
  c->geometry.moveTopLeft(QPoint(xopt, yopt));	

}

// help function for SmartPlacement --- CT 18jan98 ---
void Manager::spGetOverlap(Client* c, int x, int y, int* overlap) {

  int cxl, cxr, cyt, cyb;     //temp coords
  int  xl,  xr,  yt,  yb;     //temp coords
  int over_temp, dummy;       //temp overlap
  Client* l;

  // initialize often used vars: width and height of c; we gain speed
  int ch = c->isShaded()?(TITLEBAR_HEIGHT+2*BORDER):c->geometry.height();
  int cw = c->geometry.width();

  // get the maximum allowed windows space
  QRect maxRect = KWM::getWindowRegion(currentDesktop());

  //test if enough room in y direction
  if (y + ch > maxRect.bottom()) {
    *overlap = -1;
    return ;
  }

  //test if enough room in x direction
  if(x + cw > maxRect.right()) {
    *overlap = -2;
    return;
  }

  over_temp = 0;

  cxl = x;
  cxr = x + cw;
  cyt = y;
  cyb = y + ch;
  for(l = clients.first(); l ; l = clients.next()) {
    if(!l->isOnDesktop(currentDesktop()) ||
       l->isIconified() ||
       (l == c) )
      continue;
    xl = l->geometry.x();
    yt = l->geometry.y();
    xr = xl + l->geometry.width();
    yb = yt + (l->isShaded()?
	       (TITLEBAR_HEIGHT+2*BORDER):
	       l->geometry.height());

    //if windows overlap, calc the overlapping
    if((cxl < xr) && (cxr > xl) &&
       (cyt < yb) && (cyb > yt)) {
      xl = (cxl > xl)? cxl : xl;
      xr = (cxr < xr)? cxr : xr;
      yt = (cyt > yt)? cyt : yt;
      yb = (cyb < yb)? cyb : yb;
      dummy = (xr - xl) * (yb - yt);
      //to put here avoidance code if ever needed (see original fct)
      over_temp += dummy;
      if((over_temp > *overlap) && (*overlap != -1)) {
	*overlap = over_temp;
	return;
      }
    }
  }
  //return the overlap from this search
  *overlap = over_temp;
  return;
}

//CT 12mar98 - function to place clients on the current desk in order
//   to minimize the overlap
void Manager::deskUnclutter() {

  Client *cl, *bm; //a client and a bookmark

  // save interactive_trigger value and then get rid of it
  //   we don't need interactive placement here, even if set on
  int save_interactive_trigger = -1;
  if (options.interactive_trigger >=0) {
    save_interactive_trigger = options.interactive_trigger;
    options.interactive_trigger = -1;
  }
  //prefer to loop backwards (often badly placed windows expected to be
  //  the last placed ones. Have to trigger discussion about. Anybody else?
  for(cl = clients.last(); cl; cl = clients.prev()) {
    //save the current position in the clients list
    bm = clients.current();
    if((!cl->isOnDesktop(currentDesktop())) ||
       (cl->isIconified())                  ||
       (cl->isSticky())	||
       (cl->isMenuBar()) )
      continue;
    smartPlacement(cl);
    sendConfig(cl, false); //ask Matthias if this is the best way
    //restore the pos in clients list (munged by smartPlacement
    // Matthias: yes, Christian, I think so :-)
    clients.findRef(bm);
  }
  options.interactive_trigger = save_interactive_trigger;
}

//CT 12mar98 - function to place clients on the current desk in order
//   to organize them in cascade
void Manager::deskCascade() {

  Client *cl;
  cl = clients.first();
  if (!cl)
    return; //save our souls
  cascadePlacement(cl,True);
  sendConfig(cl, false);
  for(cl = clients.next(); cl; cl = clients.next()) {
    if((!cl->isOnDesktop(currentDesktop())) ||
       (cl->isIconified())                  ||
       (cl->isSticky())	||
       (cl->isMenuBar()))
      continue;
    cascadePlacement(cl,False);
    sendConfig(cl, false);
  }
}

//CT 16mar98, 27May98 - magics: BorderSnapZone, WindowSnapZone
void Manager::snapIt(Client *c) {

  int snap;        //snap trigger

  QRect maxRect = KWM::getWindowRegion(manager->currentDesktop());
  int xmin = maxRect.left();
  int xmax = maxRect.right();               //desk size
  int ymin = maxRect.top();
  int ymax = maxRect.bottom();
  int cx, cy, rx, ry;                 //these hopefully do not change

  int nx, ny;                         //buffers
  int deltaX = xmax, deltaY = ymax;   //minimum distance to other clients

  Client *l;
  int lx, ly, lrx, lry; //coords and size for the comparison client

  nx = cx = c->geometry.x();
  ny = cy = c->geometry.y();
  rx = cx + c->geometry.width();
  ry = c->isShaded()? cy + TITLEBAR_HEIGHT + 2*BORDER:
                      cy + c->geometry.height();

  // border snap
  snap = options.BorderSnapZone;
  if (snap) {
    if ( abs(cx-xmin) < snap ){
      deltaX = abs(cx - xmin);
      nx = xmin;
    }
    if ((abs(xmax-rx) < snap) && (abs(xmax-rx) < deltaX)) {
      deltaX = abs(xmax-rx);
      nx = xmax - c->geometry.width();
    }

    if ( abs(cy-ymin) < snap ){
      deltaY = abs(cy-ymin);
      ny = ymin;
    }
    if ((abs(ymax-ry) < snap)  && (abs(ymax-ry) < deltaY)) {
      deltaY = abs(ymax-ry);
      ny = ymax -  (c->isShaded()? TITLEBAR_HEIGHT + 2*BORDER:c->geometry.height());
    }
  }

  // windows snap
  snap = options.WindowSnapZone;
  for (l = clients.first();l;l = clients.next()) {
    if(!l->isOnDesktop(manager->currentDesktop()) ||
       l->isIconified() ||
       l->trans != None)
      continue;

    lx = l->geometry.x();
    ly = l->geometry.y();
    lrx = lx + l->geometry.width();
    lry = l->isShaded()? ly + TITLEBAR_HEIGHT + 2*BORDER:
      ly + l->geometry.height();

    if( ((cy <= lry) && (cy >= ly))  ||
	((ry >= ly)  && (ry <= lry)) ||
	((ly >= cy)  && (lry <= ry)) )  {
      if ( (abs(lrx - cx) < snap) && (abs(lrx -cx) < deltaX) ) {
	deltaX = abs(lrx - cx);
	nx = lrx;
      }
      if ( (abs(rx - lx) < snap) &&  (abs(rx - lx) < deltaX) ) {
	deltaX = abs(rx - lx);
	nx = lx - c->geometry.width();
      }
    }

    if( ((cx <= lrx) && (cx >= lx))  ||
	((rx >= lx)  && (rx <= lrx)) ||
	((lx >= cx)  && (lrx <= rx)) ){
      if ( (abs(lry - cy) < snap) && (abs(lry -cy) < deltaY) ) {
	deltaY = abs(lry - cy);
	ny = lry;
      }
      if ( (abs(ry-ly) < snap) &&  (abs(ry - ly) < deltaY) ) {
	deltaY = abs(ry - ly);
	ny = ly - (c->isShaded()? (TITLEBAR_HEIGHT + 2*BORDER):
		                   c->geometry.height());
      }
    }
  }

  c->geometry.moveTopLeft(QPoint(nx, ny));
}


// one of the central functions within kwm: manage a new window. If
// mapped is true, then the window is already mapped. This happens
// if kwm is started after some windows already appeard.
void Manager::manage(Window w, bool mapped){

  bool dohide;
  bool pseudo_session_management = false;
  int state;
  XClassHint klass;
  XWMHints *hints;
  XWindowAttributes attr;

  if (KWM::isDockWindow(w)){
    addDockWindow(w);
    return;
  }

  // test for manage prohibitation
  char* s;
  QRegExp r;
  if (!mapped) {
    QString t = getTextProperty(w, XA_WM_NAME);
    if (t.isEmpty()) {
	// XA_WM_NAME is not sufficient. If not defined try the icon name
	t = getTextProperty(w, XA_WM_ICON_NAME);
    }

    if (t.isEmpty()){
	// XA_WM_NAME is not sufficient. If not defined try the instance or class
	if (XGetClassHint(qt_xdisplay(), w, &klass) != 0) {
	    t = klass.res_name;
	    if (t.isEmpty())
		t = klass.res_class;
	    XFree(klass.res_name);
	    XFree(klass.res_class);
	}
    }
    if (!t.isEmpty()){
      for (s = do_not_manage_titles.first(); s ;
	   s = do_not_manage_titles.next()){
	r = s;
	if (r.match(t) != -1){
	  do_not_manage_titles.remove();
	  sendToModules(module_win_add, 0, w);
	  sendToModules(module_win_remove, 0, w);
	  return;
	}
      }
    }
  }

  // create a client
  if (!initting)
    XGrabServer(qt_xdisplay()); // we want to be alone...

  Client* c = getClient(w);
  if (!c){
      // create a very new client
      long d = 0;
      if (!getSimpleProperty(w, qt_sizegrip, d, XA_WINDOW))
	  getSimpleProperty( w, qt_sizegrip, d ); // old version with qt_sizegrip as type
      c = new Client(w, d);

    // overwrite Qt-defaults because we need SubstructureNotifyMask
    XSelectInput(qt_xdisplay(), c->winId(),
  		 KeyPressMask | KeyReleaseMask |
  		 ButtonPressMask | ButtonReleaseMask |
  		 KeymapStateMask |
   		 ButtonMotionMask |
   		 PointerMotionMask | // need this, too!
  		 EnterWindowMask | LeaveWindowMask |
  		 FocusChangeMask |
  		 ExposureMask |
		 StructureNotifyMask |
		 SubstructureRedirectMask |
		 SubstructureNotifyMask
  		 );

    getWindowProtocols(c);
    // pseudo sessionmanagement proxy
    c->command = xgetprop(c->window, XA_WM_COMMAND);
    c->machine = getprop(c->window, wm_client_machine);
    if (proxy_hints && !proxy_hints->isEmpty() && !c->command.isEmpty()){
      QString s = c->command + " @ " + c->machine;
      unsigned int i;
      for (i=0; i<proxy_hints->count(); i++){
	if (s == proxy_hints->at(i)){
	  QString d = proxy_props->at(i);
	  KWM::setProperties(c->window, d);
	  pseudo_session_management = true;
	  XSync(qt_xdisplay(), False);
	  proxy_hints->removeRef(0);
	  proxy_props->removeRef(0);
	  i = proxy_hints->count();
	}
      }
    }

    if (XGetWindowAttributes(qt_xdisplay(), w, &attr)){
      c->geometry.setRect(attr.x,
			  attr.y,
			  attr.width,
			  attr.height);
      c->backing_store = attr.backing_store;
    }
  }
  DEBUG_EVENTS2("manage..... Client", c,c->window)

  // get KDE specific decoration hint
  {
      long dec = KWM::getDecoration(c->window);
      c->decoration = dec & 255;
      c->is_menubar = (dec & KWM::standaloneMenuBar) != 0;
      has_standalone_menubars |= c->is_menubar;
      c->wants_focus = !c->is_menubar && (dec & KWM::noFocus) == 0;
      c->stays_on_top = (dec & KWM::staysOnTop) != 0;
  }

  if ( hasShape(c) )
      c->decoration = KWM::noDecoration;

  XSelectInput(qt_xdisplay(), c->window, ColormapChangeMask |
	       EnterWindowMask | PropertyChangeMask | PointerMotionMask);

  if (XGetClassHint(qt_xdisplay(), c->window, &klass) != 0) {   // Success
    c->instance = klass.res_name;
    c->klass = klass.res_class;
    XFree(klass.res_name);
    XFree(klass.res_class);
  }
  else {
    c->instance = "";
    c->klass = "";
  }
  c->iconname = getTextProperty(c->window, XA_WM_ICON_NAME);
  c->name = getTextProperty(c->window, XA_WM_NAME);
  c->setLabel();

#ifdef DEBUG_EVENTS_ENABLED
  printf("client %p with Window %ld and WinId %d has title '%s'\n", c, c->window, c->winId(), c->label.data());
#endif

  doGlobalDecorationAndFocusHints(c);

  if (!c->wantsFocus())
    c->hidden_for_modules = true;


  // find out the initial state. Several possibilities exist
  hints = XGetWMHints(qt_xdisplay(), c->window);
  state = WithdrawnState;
  if (hints && (hints->flags & StateHint))
    state = hints->initial_state;
  if (state == WithdrawnState)
    state = KWM::getWindowState(c->window);
  if (state == WithdrawnState)
    state = NormalState;
  dohide = (state == IconicState);
  setWindowState(c, state);
  if (hints && (hints->flags & WindowGroupHint) ) {
      c->leader = hints->window_group;
  }
  else {
      long result = None;
      getSimpleProperty(c->window, wm_client_leader, result, XA_WINDOW);
      c->leader = result;
  }
  if (c->leader == c->window)
      c->leader = None;
  if (hints)
    XFree(hints);

  getWMNormalHints(c);


  if (c->size.flags & PPosition) {
      // some obsolete hints, some old apps may still use them
      if (c->size.x != 0 && c->geometry.x() == 0 )
	  c->geometry.moveTopLeft(QPoint(c->size.x, c->geometry.y()));
      if (c->size.y != 0 && c->geometry.y() == 0 )
	  c->geometry.moveTopLeft(QPoint(c->geometry.x(), c->size.y));
  }

  getColormaps(c);
  getWindowTrans(c);
  getMwmHints(c);

  if(c->trans != None || c->getDecoration()!=KWM::normalDecoration){
    int i;
    if (c->buttons[0] &&
	(c->buttons[0] != c->buttonMenu ||
	 c->getDecoration()!=KWM::normalDecoration))
      c->buttons[0]->hide();
    if (c->buttons[3] &&
	(options.buttons[3] != CLOSE ||
	 c->getDecoration()!=KWM::normalDecoration)) {
	c->buttons[3]->hide();
    }
    for (i=1; i<6; i++){
      if (i != 3 && c->buttons[i])
	c->buttons[i]->hide();
    }
  }

  // readjust dx,dy,x,y so they are Client sizes
  if (!pseudo_session_management)
    gravitate(c,0);
  else{
    switch (c->getDecoration()){
    case KWM::noDecoration:
      break;
    case KWM::tinyDecoration:
      c->geometry.setWidth(c->geometry.width() + 2*BORDER);
      c->geometry.setHeight(c->geometry.height()+ 2*BORDER);
      break;
    default:
      c->geometry.setWidth(c->geometry.width() + 2*BORDER);
      c->geometry.setHeight(c->geometry.height()+ 2*BORDER+TITLEBAR_HEIGHT);
      break;
    }
  }


  bool didPlacement = FALSE;
  if (mapped || c->trans != None
      || (c->size.flags & PPosition && (c->geometry.x() != 0 || c->geometry.y() != 0 ) )
      ||c->size.flags & USPosition
      || pseudo_session_management
      ){

      QRect maxRect = KWM::getWindowRegion(currentDesktop());
      if (myapp->systemMenuBar && !c->isMenuBar()) {
	  maxRect.setTop(myapp->systemMenuBar->geometry().bottom());
      }

      if (c->geometry.x() < maxRect.x() || c->geometry.right() > maxRect.right()
	  || c->geometry.y() < maxRect.y() || c->geometry.bottom() > maxRect.bottom() ) {
	  // the settings are bogus, do customized placement again.
	  doPlacement(c);
	  didPlacement = TRUE;
      }
  }
  else {
    doPlacement(c);
    didPlacement = TRUE;
  }

  XSetWindowBorderWidth(qt_xdisplay(), c->window, 0);

  // it is important to reparent with the correct position! Otherwise the application window may
  // do a little jump after beeing fully mapped.
  switch (c->getDecoration()){
  case 0:
      XReparentWindow(qt_xdisplay(), c->window, c->winId(), 0, 0);
    break;
  case 2:
    XReparentWindow(qt_xdisplay(), c->window, c->winId() ,
		    (BORDER_THIN), (BORDER_THIN) );
    break;
  default:
    XReparentWindow(qt_xdisplay(), c->window, c->winId(),
		    (BORDER), (BORDER) + TITLEBAR_HEIGHT);
    break;
  }

  if (shape){
      XShapeSelectInput(qt_xdisplay(), c->window, ShapeNotifyMask);
      if (hasShape(c)) {
	  setShape(c);
      }
  }

  XAddToSaveSet(qt_xdisplay(), c->window);
  XSync(qt_xdisplay(), False);
  sendConfig(c, false);
  XSync(qt_xdisplay(), False);

  // get some KDE specific hints

  // transient windows on their parent's desktop
  Client* pc = 0;
  if (c->trans != None && c->trans != qt_xrootwin()){
    pc = getClient(c->trans);
    if (pc){
      c->desktop = pc->desktop;
      if (pc->isSticky())
	KWM::setSticky(c->window, True);
      KWM::moveToDesktop(c->window, c->desktop);
    }
  }
  if (!pc){
    int desktop_tmp = KWM::desktop(c->window);
    if (!kwm_error && (desktop_tmp>0 && desktop_tmp <= number_of_desktops) )
      c->desktop = desktop_tmp;
    else {
      KWM::moveToDesktop(c->window, c->desktop);
    }
  }

  c->iconified = KWM::isIconified(c->window);
  if (kwm_error){
    if (dohide){
      c->iconified = True;
    }
    KWM::setIconify(c->window, c->iconified);
  }

  if (KWM::isSticky(c->window))
    c->buttonSticky->toggle();

  dohide = (c->isIconified() || !c->isOnDesktop(currentDesktop()));


  if (KWM::isDoMaximize(c->window)) {
        KWM::doMaximize(c->window, false);
        c->maximize( KWM::doMaximizeMode(c->window), false);
  }
  else if (KWM::isMaximized(c->window)){
    QRect tmprec = KWM::geometryRestore(c->window);

    /* Rethink this. I cannot do maximize since this would
     * break the tree different maximize levels
     * (full, horizontal, vertical). I do not want to introduce
     * a integer flag for maximize. Let us try is this way.
     * The only drawback: a application that wants to start up
     * maximized (not session management) must call setMaximize
     * after it has been mapped. Should not be a problem.
     *
     *  additionally there is now a function doMaximize() in the
     * libkdecore/KWM that should solve all problems. See above
     */

    // avoid flickering
    c->maximized = true;
    c->maximize_mode = KWM::maximizeMode(c->window);
    c->buttonMaximize->toggle();

    c->geometry_restore = tmprec;
  }



  addClient(c);
  if ( c->isDialog() )
      sendToModules( module_dialog_win_add, c );
  sendToModules(module_win_add, c);

  if (dohide){
    c->hide();
    XUnmapWindow(qt_xdisplay(), c->window);
    //c->hideClient();
    setWindowState(c, IconicState);
  }
  else {
    c->showClient();
    setWindowState(c, NormalState);

    raiseClient(c);

    if (current() != c)
      colormapFocus(current());
  }

  if (c->trans)
    raiseSoundEvent("Window Trans New");
  else
    raiseSoundEvent("Window New");


  if (!dohide && c->wantsFocus() && !CLASSIC_FOCUS) {
      activateClient(c);
  }
  else
    c->setactive(false);

  if (c->isIconified())
    iconifyTransientOf(c);

  if (!initting)
    XUngrabServer(qt_xdisplay());

  if(options.Placement == MANUAL_PLACEMENT && didPlacement
     && c->isOnDesktop(manager->currentDesktop())
     && (c->trans == None)) {
    // ensure that the window is completely visible
    XSync(qt_xdisplay(), False);
    Window w = c->window;
    c = manager->getClient(w);
    if (!c)
      return;
    myapp->myProcessEvents();
    if (!c->isActive()) // may happen due to the crappy classic focus policies
      activateClient(c);
    c->handleOperation(OP_MOVE);
  }

  if (c->isMenuBar())
      updateMenuBar( c );
}

// put the client in withdraw state (which means it is not managed any
// longer)
void Manager::withdraw(Client* c){
  DEBUG_EVENTS2("widthdraw client", c,c->window)
  KWM::moveToDesktop(c->window, 0);

  // first of all we have to hide the window. We do not use
  // Client::hideClient() because this would also unmap the managed
  // window. In withdraw this is dangerous, because somebody else
  // could have taken over the window (swallowing), see below.
  c->hide();


  // if we still manage the window, then we should give it free. This
  // means: we have to reparent it to the root window. But maybe
  // somebody else already swallowed the window with another reparent
  // call? Then we recieved a syntetic unmap notify event and
  // therefore withdraw the window (see unmapNotify). Our reparent
  // call would be the last one and would win! Not very friendly
  // towards applications. Therefore check, wether we _really_ still
  // manage the window before reparenting.

  XEvent ev;
  if (XCheckTypedWindowEvent (qt_xdisplay(), c->window,
			      DestroyNotify, &ev) ) {
      debug("withdraw: have destroy in queue");
      destroyNotify(&ev.xdestroywindow);
      return;
  }

  unsigned int i, nwins;
  Window dw1, dw2, *wins;
  XQueryTree(qt_xdisplay(), c->winId(), &dw1, &dw2, &wins, &nwins);
  for (i = 0; i < nwins; i++) {
      if (wins[i] == c->window){
	  // we still manage it => do reparenting
	  DEBUG_EVENTS2("widthdraw we still manage => do reparenting", c,c->window)
	      gravitate(c, true);
	  XUnmapWindow(qt_xdisplay(), c->window);
	  XReparentWindow(qt_xdisplay(), c->window, qt_xrootwin(),
			  c->geometry.x() , c->geometry.y());
	  XRemoveFromSaveSet(qt_xdisplay(), c->window);
	  setWindowState(c, WithdrawnState);
	  break;
      }
  }
  XFree((void *) wins);
  XSelectInput(qt_xdisplay(), c->window, NoEventMask);
  XUngrabButton(qt_xdisplay(), AnyButton, AnyModifier, c->window);

  removeClient(c);
}


// get a pointer to the Client object from an X11 window. The window
// can be either the parent window (which is the kwm frame around a
// window) or the client window itself. Returns 0 if no client can
// be found.
Client* Manager::getClient(Window w){
  Client* result;
  for (result = clients.first();
       result && result->window != w && result->winId() != w;
       result = clients.next());
  return result;
}

// get a pointer to the Client object from the sizegrip
Client* Manager::getClientFromSizegrip(Window w){
  Client* result;
  for (result = clients.first();
       result && result->sizegrip != w;
       result = clients.next());
  return result;
}


// returns the current client (the client which has the focus) or 0
// if no client has the focus.
Client* Manager::current(){
  Client* result = clients_traversing.last();

  return (result && result->isActive()) ? result : (Client*)0;
}

// add a client into the manager큦 client lists
void Manager::addClient(Client* c){
  clients.append(c);
  clients_sorted.append(c);
  if (!c->hidden_for_modules )
    clients_traversing.insert(0,c);
}


// activate a client: Take care about visibility, floating submenus
// and focus.
void Manager::activateClient(Client* c, bool set_revert){
  if (c && !c->wantsFocus() )
    return;
  Client* cc = current();
  enable_focus_follow_mouse_activation = false;
  if (focus_grabbed())
    return;
  if (c == cc)
    return;
  if (cc){
    cc->setactive( false );
    if (!c || cc->mainClient() != c->mainClient())
      iconifyFloatingOf(cc->mainClient());
  }

  if (c) {
      c->setactive( true );
      unIconifyTransientOf(c->mainClient());
      focusToClient(c);
      colormapFocus(c);
      if (clients_traversing.removeRef(c))
	  clients_traversing.append(c);
      if (!set_revert && cc){
	  if (clients_traversing.removeRef(cc))
	      clients_traversing.insert(0,cc);
      }

      raiseMenubarsOf(c->mainClient());

      XChangeProperty(qt_xdisplay(), qt_xrootwin(), kwm_active_window,
		      kwm_active_window, 32,
		      PropModeReplace, (unsigned char *)&(c->window), 1);
      sendToModules(module_win_activate, c);
  }
  else {
      focusToNull();
  }
}

// remove a client from the manager큦 client list
void Manager::removeClient(Client* c){
  if (c->trans)
    raiseSoundEvent("Window Trans Delete");
  else
    raiseSoundEvent("Window Delete");

  bool do_nofocus = (current() == c);
  clients.removeRef(c);
  clients_sorted.removeRef(c);
  clients_traversing.removeRef(c);
  if (do_nofocus)
    noFocus();
  sendToModules(module_win_remove, c);
  delete c;

}



// raise a client. Take care about modal dialogs
void Manager::raiseClient(Client* c){
  QList <Client> tmp;
  QList <Client> sorted_tmp = clients_sorted;

  sorted_tmp.removeRef(c);

  // raise the client
  tmp.append(c);

  // raise all transient windows now
  Client *cc = 0;
  Client* ccc = 0;
  QList <Client> tmp2;
  tmp2.clear();
  do {
    for (ccc = tmp2.first(); ccc ; ccc = tmp2.next()){
      sorted_tmp.removeRef(ccc);
      tmp.append(ccc);
    }
    tmp2.clear();
    for (cc = sorted_tmp.first(); cc; cc = sorted_tmp.next()){
      for (ccc = tmp.first(); ccc; ccc = tmp.next()){
	if (cc->trans == ccc->window || cc->leader == ccc->window )
	  tmp2.append(cc);
      }
    }
  } while (!tmp2.isEmpty());


  // stays on top windows
  bool ignore_stays_on_top_windows = c->isStaysOnTop() ||  c->mainClient()->isStaysOnTop();
  if (!ignore_stays_on_top_windows) {
      for (c = clients_sorted.first(); c ; c = clients_sorted.next()){
	  if ( c->isStaysOnTop() && !tmp.contains( c ))
	      tmp.append( c );
      }
  }

  // finally do the raising

  for (c=tmp.first();c;c=tmp.next()){
    clients_sorted.removeRef(c);
    clients_sorted.append(c);
    sendToModules(module_win_raise, c);
  }
  // X Semantics are somewhat cryptic
  Window* new_stack = new Window[tmp.count()+1 + top_windows.count()];
  int i = 0;
  // top windows (such as the panel) are always taken into account
  for (Window* w = top_windows.last(); w; w = top_windows.prev() ) {
      new_stack[i] = *w;
      i++;
  }
  // take care about the gimmick: this should be always on top!
  if (options.GimmickMode && tmp.first() == current() && Client::gimmickWindow() != None){
    new_stack[i]=Client::gimmickWindow();
    i++;
  }


  for (c=tmp.last();c;c=tmp.prev()){
    new_stack[i] = c->winId();
    i++;
  }
  XRaiseWindow(qt_xdisplay(), new_stack[0]);
  XRestackWindows(qt_xdisplay(), new_stack, i);
  delete [] new_stack;

  raiseElectricBorders();
}

// lower a client. Take care about modal dialogs and kfm큦 root icons
void Manager::lowerClient(Client* c){
  QList <Client> tmp;
  clients_sorted.removeRef(c);
  clients_sorted.insert(0,c);
  sendToModules(module_win_lower, c);

  // also lower the root icons
  unsigned int i, nwins;
  Window dw1, dw2, *wins;
  XQueryTree(qt_xdisplay(), qt_xrootwin(), &dw1, &dw2, &wins, &nwins);

  // X Semantics are somewhat cryptic
  Window* new_stack = new Window[nwins+2];
  Window* hide_stack = new Window[nwins+2];
  unsigned int n = 0;
  unsigned int nh = 0;
  // take care about the gimmick: this should be always on top!
  if (options.GimmickMode && c == current() && Client::gimmickWindow() != None){
    new_stack[n++]=Client::gimmickWindow();
  }
  new_stack[n++] = c->winId();

  for (i = 0; i < nwins; i++) {
    if (!getClient(wins[i])){
      if (KWM::getDecoration(wins[i]) == KWM::desktopIcon){
	QRect r = KWM::geometry(wins[i]);
	if (c->geometry.contains(r) || c->geometry.intersects(r)){
	  XUnmapWindow(qt_xdisplay(), wins[i]);
	  hide_stack[nh++] = wins[i];
	}
	new_stack[n++] = wins[i];
      }
    }
  }

  XLowerWindow(qt_xdisplay(), new_stack[0]);
  XRestackWindows(qt_xdisplay(), new_stack, n);
  for (i=0; i<nh; i++)
    XMapWindow(qt_xdisplay(), hide_stack[i]);
  delete [] new_stack;
  XFree((void *) wins);
}

// close a client. clients with WM_DELETE_WINDOW protocol set
// (=Pdeletewindow) are closed via wm_delete_window ClientMessage.
// Others are destroyed.
void Manager::closeClient(Client* c){
  if (c->Pdeletewindow){
    sendClientMessage(c->window, wm_protocols, wm_delete_window);
  }
  else {
    // client will not react on wm_delete_window. We have not choice
    // but destroy his connection to the XServer.
    XKillClient(qt_xdisplay(), c->window);
    removeClient(c);
  }
}

// emits a signal to the kwm modules that a client has changed.
void Manager::changedClient(Client* c){
  sendToModules(module_win_change, c);
}


// checks whether the client is still existing
bool Manager::hasClient(Client* c)
{
    for (Client* i = clients.first(); i; i = clients.next() ) {
	if (i == c)
	    return  TRUE;
    }
    return FALSE;
}


// this is called if the current client loses focus. This can happen
// if the window is destroyed, the client becomes unmapped or the
// user has chosen the brain dead classic focus follows mouse policy
// and the mouse pointer has left the window. noFocus may give the
// focus to a window which had the focus before, or put the focus to
// a dummy window if there큦 no window left on this desktop or the
// user has chosen an archaic (=classic) desktop policy.
void Manager::noFocus(){
  Client* c;

  if (!CLASSIC_FOCUS){
    for (c = clients_traversing.last();
	 c && (c->isActive() || c->state != NormalState);
	 c = clients_traversing.prev());
    if (c && c->state == NormalState) {
      activateClient(c, false);
      return;
    }
  }

  c = current();
  if (c){
    c->setactive(False);
    iconifyFloatingOf(c->mainClient());
  }

  Client::hideGimmick();
  focusToNull();
  sendToModules(module_win_activate, 0);

}

// used by noFocus to put the X11 focus to a dummy window
void Manager::focusToNull(){
  static Window w = 0;
  int mask;
  XSetWindowAttributes attr;
  if (w == 0) {
    mask = CWOverrideRedirect;
    attr.override_redirect = 1;
    w = XCreateWindow(qt_xdisplay(), qt_xrootwin(), 0, 0, 1, 1, 0, CopyFromParent,
		      InputOnly, CopyFromParent, mask, &attr);
    XMapWindow(qt_xdisplay(), w);
  }
  XSetInputFocus(qt_xdisplay(), w, RevertToPointerRoot, timeStamp());
  colormapFocus(0);
  long tmp = 0;
  XChangeProperty(qt_xdisplay(), qt_xrootwin(), kwm_active_window,
		  kwm_active_window, 32,
		  PropModeReplace, (unsigned char *)&tmp, 1);
}


// auxiliary function to put the focus to a client window
void Manager::focusToClient(Client* c){
  if (c->isShaded())
    focusToNull();
  else {
    XSetInputFocus(qt_xdisplay(), c->window, RevertToPointerRoot, timeStamp());
    if (c->Ptakefocus)
      sendClientMessage(c->window, wm_protocols, wm_take_focus);
  }
}


// sets the state of a window (IconicState, NormalState, WithdrawnState)
void Manager::setWindowState(Client *c, int state){
  unsigned long data[2];

  data[0] = (unsigned long) state;
  data[1] = (unsigned long) None;

  c->state = state;

  XChangeProperty(qt_xdisplay(), c->window, wm_state, wm_state, 32,
		  PropModeReplace, (unsigned char *)data, 2);
}


// gets the transientfor hint of a client from the XServer and
// stores it in Client->trans
void Manager::getWindowTrans(Client* c){
  Window trans = None;
  if (XGetTransientForHint(qt_xdisplay(), c->window, &trans))
    c->trans = trans;
}


// switch to the specified virtual desktop
void Manager::switchDesktop(int new_desktop){
  if (new_desktop == current_desktop
      || new_desktop < 1 || new_desktop > number_of_desktops)
    return;

  if (current() && !current()->isSticky())
    current()->setactive(False); // no more current()!

  // optimized Desktop switching
  //   unmapping done from back to front
  //   mapping done from front to back
  //   => less exposure events
  Client* c;
  for (c=clients_sorted.first(); c ; c=clients_sorted.next()){
    if (c->isOnDesktop(current_desktop) && !c->isIconified() && !c->isSticky()){
      // protection for electric borders
      if (QWidget::mouseGrabber() != c){
	c->hideClient();
	setWindowState(c, IconicState);
      }
    }
  }
  current_desktop = new_desktop;

  KWM::switchToDesktop(current_desktop);
  sendToModules(module_desktop_change, 0, (Window) current_desktop);

  for (c=clients_sorted.last(); c ; c=clients_sorted.prev()){
    if (c->isOnDesktop(current_desktop) && !c->isIconified() && !c->isSticky()){
      // protection for electric borders
      if (c->state != NormalState){
	c->showClient();
	setWindowState(c, NormalState);
      }
    }
    if (c->isSticky() && ! c->isIconified() && c != current()){
      if (clients_traversing.removeRef(c))
       clients_traversing.insert(0,c);
    }
  }

  if (!current())
    noFocus();

  updateMenuBars();
}


// make the menubars fit to the current desktop region
void Manager::updateMenuBars()
{
 myapp->resetSystemMenuBar();
 if (!has_standalone_menubars)
     return;

 QListIterator<Client> it(clients);
 QRect r =  KWM::getWindowRegion(currentDesktop());
 for (it.toFirst(); it.current(); ++it)
     if (it.current()->isMenuBar() && it.current()->isOnDesktop(currentDesktop())) {
	 it.current()->geometry.setRect(r.x(),(r.y()-1)<=0?-2:r.y()-1, r.width(), // check panel top
					it.current()->geometry.height());
	 sendConfig( it.current() );
     }
}

// make the passed menubar fit to the current desktop region
void Manager::updateMenuBar(Client* c)
{
    if (!c->isMenuBar() || !has_standalone_menubars)
	return;
    if (c->isOnDesktop(currentDesktop())){
	QRect r =  KWM::getWindowRegion(currentDesktop());
	c->geometry.setRect(r.x(),(r.y()-1)<=0?-2:r.y()-1, r.width(), // check panel top
			    c->geometry.height());
	sendConfig( c );
    }

}

// make the maximized windows fit to the current desktop region
// make the menubars fit to the current desktop region
void Manager::updateMaximizedWindows()
{
    QListIterator<Client> it(clients);
    for (it.toFirst(); it.current(); ++it){
	if (it.current()->isMaximized() && it.current()->isOnDesktop(currentDesktop())){
	    it.current()->maximized = FALSE;
	    it.current()->geometry = it.current()->geometry_restore;
	    it.current()->maximize(it.current()->maximize_mode, FALSE);
	}
    }

}




// Tells the XServer and the client itself to sync the X window with
// the datas stored in kwm큦 client object. If emit_changed is true,
// then all kwm modules are informed about that change.
void Manager::sendConfig(Client* c, bool emit_changed){
  XConfigureEvent ce;

  if (c->isShaded()){
    c->setGeometry(c->geometry.x(), c->geometry.y(),
		   c->geometry.width(), 2*BORDER+TITLEBAR_HEIGHT);
  }
  else {
    c->setGeometry(c->geometry);
  }

  setQRectProperty(c->window, kwm_win_frame_geometry, c->geometry);

  ce.type = ConfigureNotify;
  ce.event = c->window;
  ce.window = c->window;

  switch (c->getDecoration()){
  case KWM::noDecoration:
    ce.x = c->geometry.x();
    ce.y = c->geometry.y();
    ce.width = c->geometry.width();
    ce.height = c->geometry.height();
    break;
  case KWM::tinyDecoration:
    ce.x = c->geometry.x() + BORDER_THIN;
    ce.y = c->geometry.y() + BORDER_THIN;
    ce.width = c->geometry.width() - 2*BORDER_THIN;
    ce.height = c->geometry.height() - 2*BORDER_THIN;
    break;
  default:
    ce.x = c->geometry.x() + BORDER;
    ce.y = c->geometry.y() + BORDER + TITLEBAR_HEIGHT;
    ce.width = c->geometry.width() - 2*BORDER;
    ce.height = c->geometry.height() - 2*BORDER - TITLEBAR_HEIGHT;
    break;
  }

  ce.border_width = c->border;
  ce.above = None;
  ce.override_redirect = 0;
  XSendEvent(qt_xdisplay(), c->window, False, StructureNotifyMask, (XEvent*)&ce);

  if (emit_changed)
    changedClient(c);
  c->placeGimmick();
}



// give all managed windows free and remove the kwm_running
// property.
void Manager::cleanup(bool kill){
  Client *c;
  XWindowChanges wc;

  for (c = clients.first(); c; c = clients.next()) {
    gravitate(c, true);

    XReparentWindow(qt_xdisplay(), c->window, qt_xrootwin(), c->geometry.x() , c->geometry.y());

    wc.border_width = c->border;
    XConfigureWindow(qt_xdisplay(), c->window, CWBorderWidth, &wc);
    if (kill)
      XKillClient(qt_xdisplay(), c->window);
  }

  Window* w;
  for (w = dock_windows.first(); w; w = dock_windows.next()) {
      XReparentWindow(qt_xdisplay(), *w, qt_xrootwin(), 0, 0);
      if (dock_module != None)
	sendClientMessage(dock_module, module_dockwin_remove, (long) *w);
  }
  XSetInputFocus(qt_xdisplay(), PointerRoot, RevertToPointerRoot, CurrentTime);
  colormapFocus(0);
  XDeleteProperty(qt_xdisplay(), qt_xrootwin(), kwm_running);
}

// repaint all windows. This is useful when we recieved a
// KDEChangeGeneral or KDEChangePalette event.
void Manager::repaintAll(){
  Client* c;
  for (c=clients_sorted.last(); c; c = clients_sorted.prev())
    c->paintState(false, true);
}


// syncs the window manager with the X server and returns a current
// timestamp.
Time Manager::timeStamp(){
  static Window w = 0;
  int mask;
  XSetWindowAttributes attr;
  if (w == 0) {
    mask = CWOverrideRedirect;
    attr.override_redirect = 1;
    w = XCreateWindow(qt_xdisplay(), qt_xrootwin(), 0, 0, 1, 1, 0, CopyFromParent,
		      InputOnly, CopyFromParent, mask, &attr);
    XSelectInput(qt_xdisplay(), w,
	       PropertyChangeMask);
    XMapWindow(qt_xdisplay(), w);
  }
  XEvent ev;
  XChangeProperty(qt_xdisplay(), w, kwm_running, kwm_running, 8,
		  PropModeAppend, (unsigned char *)"", 0);
  XWindowEvent(qt_xdisplay(), w, PropertyChangeMask, &ev);
  return ev.xproperty.time;
}


// help functions in the init process of the manager: scans the
// desktop to detect already mapped windows. Necessary if kwm
// appears after some windows.
void Manager::scanWins(){
  unsigned int i, nwins;
  Window dw1, dw2, *wins;
  XWindowAttributes attr;

  XQueryTree(qt_xdisplay(), qt_xrootwin(), &dw1, &dw2, &wins, &nwins);
  for (i = 0; i < nwins; i++) {
    if (KWM::isKWMModule(wins[i])){
      addModule(wins[i]);
    }
    XGetWindowAttributes(qt_xdisplay(), wins[i], &attr);
    if (attr.override_redirect )
      continue;
    if (attr.map_state != IsUnmapped)
      manage(wins[i], true);
  }
  XFree((void *) wins);
}


void Manager::installColormap(Colormap cmap){
  XInstallColormap(qt_xdisplay(), (cmap == None) ? default_colormap : cmap);
}
void Manager::colormapFocus(Client *c){
  int i, found;
  Client *cc;

  if (c == 0)
    installColormap(None);
  else if (c->ncmapwins != 0) {
    found = 0;
    for (i = c->ncmapwins-1; i >= 0; i--) {
      installColormap(c->wmcmaps[i]);
      if (c->cmapwins[i] == c->window)
	found++;
    }
    if (!found)
      installColormap(c->cmap);
  }
  else if (c->trans && (cc = getClient(c->trans)) != 0 && cc->ncmapwins != 0)
    colormapFocus(cc);
  else
    installColormap(c->cmap);
}

// access functions to standard X11 window properties. The results
// will be stored in attributes of the passed client object.
void Manager::getWMNormalHints(Client *c){
  long msize;
  bool fixedSize = c->fixedSize();
  if (XGetWMNormalHints(qt_xdisplay(), c->window, &c->size, &msize) == 0 || c->size.flags == 0)
    c->size.flags = PSize;      /* not specified - punt */
  if (c->fixedSize() != fixedSize)
    c->reconfigure();
}

void Manager::getColormaps(Client *c){
  int n, i;
  Window *cw;
  XWindowAttributes attr;

  XGetWindowAttributes(qt_xdisplay(), c->window, &attr);
  c->cmap = attr.colormap;

  n = _getprop(c->window, wm_colormap_windows, XA_WINDOW, 100L, (unsigned char **)&cw);
  if (c->ncmapwins != 0) {
    XFree((char *)c->cmapwins);
    free((char *)c->wmcmaps);
  }
  if (n <= 0) {
    c->ncmapwins = 0;
    return;
  }

  c->ncmapwins = n;
  c->cmapwins = cw;

  c->wmcmaps = (Colormap*)malloc(n*sizeof(Colormap));
  for (i = 0; i < n; i++) {
    if (cw[i] == c->window)
      c->wmcmaps[i] = c->cmap;
    else {
      XSelectInput(qt_xdisplay(), cw[i], ColormapChangeMask);
      XGetWindowAttributes(qt_xdisplay(), cw[i], &attr);
      c->wmcmaps[i] = attr.colormap;
    }
  }
}

// does the client need a shape combine mask around it?
bool Manager::hasShape(Client* c){
  int xws, yws, xbs, ybs;
  unsigned wws, hws, wbs, hbs;
  int boundingShaped, clipShaped;

  if (!shape)
      return false;

  /* cheat: don't try to add a border if the window is non-rectangular */
  XShapeQueryExtents(qt_xdisplay(), c->window,
		     &boundingShaped, &xws, &yws, &wws, &hws,
		     &clipShaped, &xbs, &ybs, &wbs, &hbs);

  return boundingShaped != 0;
}


// put an appropriate  shape combine mask around the client
void Manager::setShape(Client* c){
  if (hasShape(c)){

    switch (c->getDecoration()){
    case KWM::noDecoration:
      XShapeCombineShape(qt_xdisplay(), c->winId(), ShapeBounding,
			 0, 0,
			 c->window, ShapeBounding, ShapeSet);
      break;
    case KWM::tinyDecoration:
      XShapeCombineShape(qt_xdisplay(), c->winId(), ShapeBounding,
			 (BORDER_THIN), (BORDER_THIN),
			 c->window, ShapeBounding, ShapeSet);
      break;
    default:
      XShapeCombineShape(qt_xdisplay(), c->winId(), ShapeBounding,
			 (BORDER), (BORDER) + TITLEBAR_HEIGHT,
			 c->window, ShapeBounding, ShapeSet);
      break;
    }
  }
}

// auxiliary functions to travers all clients according the focus
// order. Usefull for kwm큦 Alt-tab feature.
Client* Manager::nextClient(Client* c){
  Client* result;
  if (!c)
    return clients_traversing.first();
  for (result = clients_traversing.first(); result && result != c;
       result = clients_traversing.next());
  if (result)
    result = clients_traversing.next();
  if (!result)
    result = clients_traversing.first();
  return result;
}

// auxiliary functions to travers all clients according the focus
// order. Usefull for kwm큦 Alt-tab feature.
Client* Manager::previousClient(Client* c){
  Client* result;
  if (!c)
    return clients_traversing.last();
  for (result = clients_traversing.last(); result && result != c;
       result = clients_traversing.prev());
  if (result)
    result = clients_traversing.prev();
  if (!result)
    result = clients_traversing.last();
  return result;
}

// auxiliary functions to travers all clients according the static
// order. Usefull for the CDE-style Alt-tab feature.
Client* Manager::nextStaticClient(Client* c){
 Client* result;
  if (!c)
    return clients.first();
  for (result = clients.first(); result && result != c;
       result = clients.next());
  if (result)
    result = clients.next();
  if (!result)
    result = clients.first();
  return result;
}

// auxiliary functions to travers all clients according the static
// order. Usefull for the CDE-style Alt-tab feature.
Client* Manager::previousStaticClient(Client* c){
 Client* result;
  if (!c)
    return clients.last();
  for (result = clients.last(); result && result != c;
       result = clients.prev());
  if (result)
    result = clients.prev();
  if (!result)
    result = clients.last();
  return result;
}
// auxiliary functions to travers all clients according the static
// order. Usefull for the CDE-style Alt-tab feature.
Client* Manager::topClientOnDesktop(){
  Client* result;
  for (result = clients_sorted.last(); result; result = clients_sorted.prev()){
    if (result->isOnDesktop(current_desktop) && !result->isMenuBar())
      return result;
  }
  return 0;
}


// returns wether a client with such a label is existing. Useful to
// determine wether a label has to be unified with <2>, <3>, <4>,
// ...
bool Manager::hasLabel(QString label_arg){
  bool Result = False;
  Client *c;
  for (c=clients.first(); c; c=clients.next()){
    Result = Result || c->label == label_arg;
  }
  return Result;
}


// access functions to standard X11 window properties. The results
// will be stored in attributes of the passed client object.
void Manager::getWindowProtocols(Client *c){
  Atom *p;
  int i,n;

  if (XGetWMProtocols(qt_xdisplay(), c->window, &p, &n)){
    for (i = 0; i < n; i++)
      if (p[i] == wm_delete_window)
	c->Pdeletewindow = true;
      else if (p[i] == wm_take_focus)
	c->Ptakefocus = true;
      else if (p[i] == wm_save_yourself){
	c->Psaveyourself = true;
      }
    if (n>0)
      XFree(p);
  }
}

// basic Motif support
struct PropMotifWmHints
{
  int      flags;
  int      functions;
  int      decorations;
  int      inputMode;
};

#define MWM_DECOR_ALL                 (1L << 0)
#define MWM_DECOR_BORDER              (1L << 1)
#define MWM_DECOR_RESIZEH             (1L << 2)
#define MWM_DECOR_TITLE               (1L << 3)
#define MWM_DECOR_MENU                (1L << 4)
#define MWM_DECOR_MINIMIZE            (1L << 5)
#define MWM_DECOR_MAXIMIZE            (1L << 6)

// this is for MWM hints (Motif window manager).
void Manager::getMwmHints(Client  *c){
  int actual_format;
  Atom actual_type;
  unsigned long nitems, bytesafter;
  PropMotifWmHints *mwm_hints;

  if(XGetWindowProperty (qt_xdisplay(), c->window, _motif_wm_hints, 0L, 20L,
			 False,_motif_wm_hints, &actual_type,
			 &actual_format, &nitems,
			 &bytesafter,(unsigned char **)&mwm_hints)==Success)
    {
      if(nitems >= 4)
	{
	  // Motif defines a lot of more or less useful properties. We
	  // are only interested in suppressing decorations. This is
	  // important for StarOffice-3.1큦 floating toolbars which
	  // would be shown within a strange floatwinshell otherwise.
	  c->decoration = mwm_hints->decorations?KWM::normalDecoration:0;
	  KWM::setDecoration(c->window, c->getDecoration());
	}
      XFree((char *)mwm_hints);
    }
}


// handles gravitation according to the gravity of the window.
void Manager::gravitate(Client* c, bool invert){
  int gravity, dx, dy, delta;
  if (c->getDecoration() == KWM::noDecoration)
    return;

  int titlebar_height = (c->getDecoration() != KWM::normalDecoration)?
    0:TITLEBAR_HEIGHT;

  int border = (c->getDecoration()==KWM::normalDecoration)?BORDER:BORDER_THIN;

  dx = dy = 0;

  gravity = NorthWestGravity;
  if (c->size.flags & PWinGravity)
    gravity = c->size.win_gravity;

  delta = border - 1;

  switch (gravity) {
  case NorthWestGravity:
    dx = 0;
    dy = 0;
    break;
  case NorthGravity:
    dx = -delta;
    dy = 0;
    break;
  case NorthEastGravity:
    dx = -2*delta;
    dy = 0;
    break;
  case WestGravity:
    dx = 0;
    dy = -delta-titlebar_height;
    break;
  case CenterGravity:
  case StaticGravity:
    dx = -delta;
    dy = -delta-titlebar_height;
    break;
  case EastGravity:
    dx = -2*delta;
    dy = -delta-titlebar_height;
    break;
  case SouthWestGravity:
    dx = 0;
    dy = -2*delta-titlebar_height;
    break;
  case SouthGravity:
    dx = -delta;
    dy = -2*delta-titlebar_height;
    break;
  case SouthEastGravity:
    dx = -2*delta;
    dy = -2*delta-titlebar_height;
    break;
  default:
    fprintf(stderr, "kwm: bad window gravity %d for 0x%lx\n", gravity, c->window);
  }
  if (invert) {
    c->geometry.moveBy(-dx, -dy);
    c->geometry.setWidth(c->geometry.width() - 2*border);
    c->geometry.setHeight(c->geometry.height()- 2*border-titlebar_height);
  }
  else {
    c->geometry.moveBy(dx, dy);
    c->geometry.setWidth(c->geometry.width() + 2*border);
    c->geometry.setHeight(c->geometry.height()+ 2*border+titlebar_height);
  }
}

// low level function to access an X11 property
int Manager::_getprop(Window w, Atom a, Atom type, long len, unsigned char **p){
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;

  status = XGetWindowProperty(qt_xdisplay(), w, a, 0L, len, False, type, &real_type, &format, &n, &extra, p);
  if (status != Success || *p == 0)
    return -1;
  if (n == 0)
    XFree((void*) *p);
  /* could check real_type, format, extra here... */
  return n;
}

// easier access to an X11 property of type XA_STRING
QString Manager::getprop(Window w, Atom a){
  QString result;
  unsigned char *p;
  if (_getprop(w, a, XA_STRING, 100L, &p) > 0){
    result = (char*) p;
    XFree((char *) p);
  }
  return result;
}

// xgetprop is like getprop but can handle 0-separated string
// lists. Will replace \0 with a blank then. Necessary for the WM_COMMAND
// property.
QString Manager::xgetprop(Window w, Atom a){
  QString result;
  char *p;
  int i,n;
  if ((n = _getprop(w, a, XA_STRING, 100L, (unsigned char **)&p)) > 0){
    result = p;
    for (i = 0; (i += strlen(p+i)+1) < n; result.append(p+i))
	result.append(" ");
    XFree((char *) p);
  }
  return result;
}

// kwm internally sometimes uses simple property (long values)
bool Manager::getSimpleProperty(Window w, Atom a, long &result, Atom type){
  long *p = 0;

  if (_getprop(w, a, type?type:a, 1L, (unsigned char**)&p) <= 0){
    return false;
  }

  result = p[0];
  XFree((char *) p);
  return true;
}

// kwm internally sometimes uses rectangle properties
void Manager::setQRectProperty(Window w, Atom a, const QRect &rect){
  long data[4];
  data[0] = rect.x();
  data[1] = rect.y();
  data[2] = rect.width();
  data[3] = rect.height();
  XChangeProperty(qt_xdisplay(), w, a, a, 32,
		  PropModeReplace, (unsigned char *)&data, 4);
}



// a better getrop used for icon names etc. It can also handle
// compound stuff. Seems like some old and weird programs rely on that.
QString Manager::getTextProperty (Window w, Atom a)
{
    XTextProperty tp;
    char **text;
    int count;
    QString result;
    if (XGetTextProperty (qt_xdisplay(), w, &tp, a) == 0 || tp.value == NULL)
	return result; // nothing found
    if (tp.encoding == XA_STRING) {
	result = (const char*) tp.value;
    }
    else { // assume compound text
	if (XmbTextPropertyToTextList (qt_xdisplay(), &tp, &text, &count) == Success && text != NULL) {
	    if (count > 0) {
		result = (const char*) text[0];
	    }
	    XFreeStringList (text);
	}
    }
    /* Free the data returned by XGetTextProperty */
    XFree (tp.value);	
    return result;
}


// sends a client message a x to the window w
void Manager::sendClientMessage(Window w, Atom a, long x){
  XEvent ev;
  long mask;

  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = w;
  ev.xclient.message_type = a;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = x;
  ev.xclient.data.l[1] = timeStamp();
  mask = 0L;
  if (w == qt_xrootwin())
    mask = SubstructureRedirectMask;        /* magic! */
  XSendEvent(qt_xdisplay(), w, False, mask, &ev);
}


// repaint everything and even force the clients to repaint thereselfs.
void Manager::refreshScreen(){
  XSetWindowAttributes attributes;
  unsigned long valuemask;

  // this is a bit tricky: Clients with backing_store set
  // would not get the exposure event! But I do not know
  // what to do, if a server always does backing store....
  // In this case, I have to save the screen manually
  // _before_ darkenScreen.

  // CC: if there is a greyer widget, remove it
  //     else, we have to do a full refresh of the display
  if (0 != greyer_widget) {
    delete greyer_widget;
    greyer_widget = 0;
  } else {
    valuemask = CWBackingStore;
    attributes.backing_store = NotUseful;
    Client *c;
    for (c = clients.first();c;c=clients.next()){
      if (c->backing_store != NotUseful){
	XChangeWindowAttributes(qt_xdisplay(), c->window,
				valuemask, &attributes);
      }
    }
    XSync (qt_xdisplay(), False);
    timeStamp();


    valuemask = (CWBackPixel | CWBackingStore);
    attributes.background_pixel = 0;
    Window w = XCreateWindow (qt_xdisplay(), qt_xrootwin(), 0, 0,
			      QApplication::desktop()->width(),
			      QApplication::desktop()->height(),
			      (unsigned int) 0,
			      CopyFromParent, (unsigned int) CopyFromParent,
			      (Visual *) CopyFromParent, valuemask,
			      &attributes);
    XMapWindow (qt_xdisplay(), w);
    XSync (qt_xdisplay(), False);
    timeStamp();
    XDestroyWindow (qt_xdisplay(), w);
    XSync (qt_xdisplay(), False);
    timeStamp();

    valuemask = CWBackingStore;
    for (c = clients.first();c;c=clients.next()){
      if (c->backing_store != NotUseful){
	attributes.backing_store = c->backing_store;
	XChangeWindowAttributes(qt_xdisplay(), c->window,
				valuemask, &attributes);
      }
    }
  }
}

// put a grey veil over the screen. Useful to show a nice logout
// dialog.
void Manager::darkenScreen(){
  if (0 != greyer_widget)
    delete greyer_widget;
  greyer_widget = new KGreyerWidget();
}

// do the X11R4 session management: send a SAVE_YOURSELF message to
// everybody who wants to know it and wait for the answers. Also
// handles KDE큦 session management additions as well as pseudo
// session management with the help of a build in proxy.  After
// finishing this functions the manager will emit a showLogout()
// signal.
void Manager::processSaveYourself(){
  Client* c;
  XEvent ev;

  // no user interactiong during session management. Therefore we grab
  // the mouse and the keyboard.
  XGrabKeyboard(qt_xdisplay(),
		qt_xrootwin(), False,
		GrabModeAsync, GrabModeAsync,
		CurrentTime);
  XGrabPointer(qt_xdisplay(), qt_xrootwin(), False,
	       Button1Mask|Button2Mask|Button3Mask,
	       GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

  QString command;
  QString machine;
  QString properties;
  {
    // this is maybe a KDE speciality: session management for
    // unmanaged toplevel windows. This is extremly useful for clients
    // like kfm who do session management on their own but needs to
    // recieve the saveYourself message for that purpose.
    unsigned int i, nwins;
    Window dw1, dw2, *wins;

    additional_commands.clear();
    additional_machines.clear();
    additional_proxy_hints.clear();
    additional_proxy_props.clear();

     XQueryTree(qt_xdisplay(), qt_xrootwin(), &dw1, &dw2, &wins, &nwins);
     for (i = 0; i < nwins; i++) {
       if (!getClient(wins[i])){
	 long result = 0;
	 getSimpleProperty(wins[i], kwm_save_yourself, result);
	 if (result){
	     XSelectInput(qt_xdisplay(), wins[i], PropertyChangeMask);
	     sendClientMessage(wins[i], wm_protocols, wm_save_yourself);
	     // wait for clients response
	     do {
		 XWindowEvent(qt_xdisplay(), wins[i], PropertyChangeMask, &ev);
		 propertyNotify(&ev.xproperty);
	     } while (ev.xproperty.atom != XA_WM_COMMAND);
	     XSelectInput(qt_xdisplay(), wins[i], NoEventMask);
	     command = xgetprop(wins[i], XA_WM_COMMAND);
	     if (!command.isEmpty()){
		 machine = getprop(wins[i], wm_client_machine);
		 additional_commands.append(command);
		 additional_machines.append(machine);
	     }
	 }
       }
     }
     if (nwins>0)
       XFree((void *) wins);
  }



  // ususal session management: send a saveYourself to all clients
  // which requested the protocol.

  for (c = clients.first(); c; c = clients.next()){
    long result = 0;
    getSimpleProperty(c->window, kwm_save_yourself, result);
    if (result){
      c->Psaveyourself = True;
    }

    if (c->Psaveyourself){
      command = xgetprop(c->window, XA_WM_COMMAND);
      machine = getprop(c->window, wm_client_machine);
      properties = KWM::getProperties(c->window);
      XSelectInput(qt_xdisplay(), c->window,
		   PropertyChangeMask| StructureNotifyMask );
      command = xgetprop(c->window, XA_WM_COMMAND);
      machine = getprop(c->window, wm_client_machine);
      XSync(qt_xdisplay(), false);
      sendClientMessage(c->window, wm_protocols, wm_save_yourself);
      // wait for clients response
      do {
	XWindowEvent(qt_xdisplay(), c->window, PropertyChangeMask
		     | StructureNotifyMask, &ev);
	XSync(qt_xdisplay(), false);
	if (ev.type != PropertyNotify){
	  // special code for clients like xfig which unamp their
	  // window instead of setting XA_WM_COMMAND....
	  XPutBackEvent(qt_xdisplay(), &ev);
	  if (!command.isEmpty()){
	    additional_commands.append(command);
	    additional_machines.append(machine);
	    additional_proxy_hints.append(command + " @ " + machine);
	    additional_proxy_props.append(properties);
	  }
	  c->command = "";
	  c->machine = "";
	  break;
	}
	else {
	  propertyNotify(&ev.xproperty);
	  if (ev.xproperty.atom == XA_WM_COMMAND){
	    c->machine = getprop(c->window, wm_client_machine);
	    break;
	  }
	}
      } while (1);
      XSelectInput(qt_xdisplay(), c->window, ColormapChangeMask |
		   EnterWindowMask | PropertyChangeMask | PointerMotionMask);
    }
  }

  // Finally do poor man큦 session management with the help of kwm큦
  // session management proxy.  This is done after the ususal
  // saveYourself because some clients may have set XA_WM_COMMAND for
  // other windows, too
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself){
      // poor man's "session" management a la xterm :-(
      c->command = xgetprop(c->window, XA_WM_COMMAND);
      if (c->command.isNull()){
	Window clw = c->leader;
	if (clw != None && clw != c->window && getClient(clw))
	    c->command = ""; // will be stored anyway
	  else
	    c->command = xgetprop(clw, XA_WM_COMMAND);
      }
    }
  }

  // release the mouse and the keyboard which we grabbed before
  XAllowEvents(qt_xdisplay(), ReplayPointer, CurrentTime);
  XAllowEvents(qt_xdisplay(), ReplayKeyboard, CurrentTime);
}

// commands from clients which can do session management
QStrList* Manager::getSessionCommands(){
  QString thismachine;
  QString domain;
  QString all;
  char buf[200];
  if (!gethostname(buf, 200))
    thismachine = buf;
  if (!getdomainname(buf, 200))
    domain = buf;
  if (!thismachine.isEmpty())
    all = thismachine + "." + domain;

  QStrList *result = new QStrList();
  Client* c;
  QString command;
  QString machine;
  for (c = clients.first(); c; c = clients.next()){
    if (!c->command.isEmpty() && !proxy_ignore->contains(c->command)){
      // create a machine dependend command
      command = c->command.data();
      if (!c->machine.isEmpty()){
	if (c->machine != thismachine && c->machine != all){
	  command.prepend(" ");
	  command.prepend(c->machine);
	  command.prepend(" ");
	  command.prepend(options.rstart);
	}
      }
      result->append(command);
    }
  }

  // check additional commands

  command = additional_commands.first();
  machine = additional_machines.first();
  while (!command.isEmpty()){
    if (!machine.isEmpty()){
      if (machine != thismachine && machine != all){
	command.prepend(" ");
	command.prepend(machine);
	command.prepend(" ");
	command.prepend(options.rstart);
      }
    }
    result->append(command);
    command = additional_commands.next();
    machine = additional_machines.next();
  }
  return result;
}

// clients which  can only be restarted
QStrList* Manager::getPseudoSessionClients(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself && !c->command.isEmpty()
	&& !proxy_ignore->contains(c->command)
	&& !KWM::unsavedDataHintDefined(c->window))
      result->append(c->label);
  }
  return result;
}

// clients which cannot do session management at all
QStrList* Manager::getNoSessionClients(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself && c->command.isNull()
	&& (c->trans == None || c->trans == qt_xrootwin())
	&& !KWM::unsavedDataHintDefined(c->window))
      result->append(c->label);
  }
  return result;
}

// kwm supports an unsaved data hint: these clients set this hint.
QStrList* Manager::getUnsavedDataClients(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (KWM::containsUnsavedData(c->window))
      result->append(c->label);
  }
  return result;
}

// returns all clients which are on the specified desktop. Used to
// build a list of clients in a listbox.
QStrList* Manager::getClientsOfDesktop(int desk){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (c->isOnDesktop(desk) &&
	!c->hidden_for_modules &&
	(!c->isSticky() || desk == currentDesktop())){
      if (c->isIconified())
	result->append(QString("(") + c->label + ")");
      else
	result->append(c->label);
    }
  }
  return result;
}

// returns all hints of the session management proxy
QStrList* Manager::getProxyHints(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself && !c->command.isEmpty()
	&& !proxy_ignore->contains(c->command))
      result->append(c->command + " @ " + c->machine);
  }
  QString add;
  for (add = additional_proxy_hints.first(); !add.isEmpty();
       add = additional_proxy_hints.next())
    result->append(add);
  return result;
}

// returns all properties  of the session management proxy
QStrList* Manager::getProxyProps(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself && !c->command.isEmpty()
	&& !proxy_ignore->contains(c->command))
      result->append(KWM::getProperties(c->window));
  }
  QString add;
  for (add = additional_proxy_props.first(); !add.isEmpty();
       add = additional_proxy_props.next())
    result->append(add);
  return result;
}


// sets all necessary properties for the session management
// proxy. This function is called from main after reading the data
// from the configuration file.
void Manager::setProxyData(QStrList* proxy_hints_arg,
			   QStrList* proxy_props_arg,
			   QStrList* proxy_ignore_arg){
  proxy_hints = proxy_hints_arg;
  proxy_props = proxy_props_arg;
  proxy_ignore = proxy_ignore_arg;
}


// kwm's nifty kill feature (accessible with Ctrl-Alt-Escape)
void Manager::killWindowAtPosition(int x, int y){
  Client* c;
  for (c = clients_sorted.last(); c; c = clients_sorted.prev()){
    if (c->geometry.contains(QPoint(x, y))){
      XKillClient(qt_xdisplay(), c->window);
      return;
    }
  }
}

// usefull helper function
Client* Manager::clientAtPosition(int x, int y){
  Client* c;
  for (c = clients_sorted.last(); c; c = clients_sorted.prev()){
    if (c->geometry.contains(QPoint(x, y))){
      return c;
    }
  }
  return 0;
}


// returns the client with the specified label or 0 if there is no
// such client.
 Client* Manager::findClientByLabel(QString label){
  Client *c;
  if (label.left(1)=="("){
    label.remove(0,1);
    label.remove(label.length()-1, 1);
  }
  for (c = clients.first(); c; c = clients.next()){
    if (c->label == label)
      return c;
  }
  return 0;
}


// kwm usually iconifies all transient windows of a client if the
// client itself is iconified. Same applies for desktop switching:
// transient windows are supposed to be always on the desktop as the
// main window.
 void Manager::iconifyTransientOf(Client* c){
  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
    if (it.current() != c && (it.current()->trans == c->window || it.current()->leader == c->window)){
      it.current()->iconify(False);
      if (!it.current()->hidden_for_modules) {
	  sendToModules(module_win_remove, it.current());
	  it.current()->hidden_for_modules = true;
	  clients_traversing.removeRef(it.current());
      }
    }
  }
}


// kwm usually iconifies all transient windows of a client if the
// client itself is iconified. Same applies for desktop switching:
// transient windows are supposed to be always on the desktop as the
// main window.
void Manager::unIconifyTransientOf(Client* c){
  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
    if (it.current() != c && (it.current()->trans == c->window || it.current()->leader == c->window)){
      if (it.current()->hidden_for_modules && !it.current()->isMenuBar()){
	it.current()->hidden_for_modules = false;
	if ( it.current()->isDialog() )
	    sendToModules(module_dialog_win_add, it.current());
	sendToModules(module_win_add, it.current());
	clients_traversing.insert(0,it.current());
      }
      it.current()->unIconify(False);
    }
  }
}

// kwm usually iconifies all transient windows of a client if the
// client itself is iconified. Same applies for desktop switching:
// transient windows are supposed to be always on the desktop as the
// main window.
void Manager::ontoDesktopTransientOf(Client* c){
  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
    if (it.current() != c && (it.current()->trans == c->window || it.current()->leader == c->window)){
      it.current()->ontoDesktop(c->desktop);
    }
  }
}


// kwm usually iconifies all transient windows of a client if the
// client itself is iconified. Same applies for desktop switching:
// transient windows are supposed to be always on the desktop as the
// main window.
void Manager::stickyTransientOf(Client* c, bool sticky){
  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
    if (it.current() != c && (it.current()->trans == c->window || it.current()->leader == c->window)){
      if (it.current()->isSticky() != sticky)
	it.current()->buttonSticky->toggle();
    }
  }
}

// if a window loses focus, then all floating windows are
// automatically iconified. These are floating toolbars or menubars
// (except in mac-like style when the menubar really sets the
// KWM::standaloneMenuBar decoration). We do not need a special
// deIconifyFloatingOf function, since floating windows are also
// transient windows. That means that unIconifyTransientOf already
// does this job.
 void Manager::iconifyFloatingOf(Client* c){
  // DON'T automatically iconify floating menues if the focus policy is
  // focus follows mouse, since otherwise it will be exceptionally hard
  // to get control over them back! (Marcin Dalecki)
  if (options.FocusPolicy == FOCUS_FOLLOWS_MOUSE)
    return;

  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
    if (it.current() != c && it.current()->trans == c->window){
      if (it.current()->getDecoration() == KWM::tinyDecoration && !it.current()->isMenuBar()){
	it.current()->iconify(False);
	sendToModules(module_win_remove, it.current());
	it.current()->hidden_for_modules = true;
	clients_traversing.removeRef(it.current());
      }
      iconifyFloatingOf(it.current());
    }
  }
}

// kwm sort of supports floating menubars. They are raised when
// the parent window gets the focus.
void Manager::raiseMenubarsOf(Client* c)
{
    if (!has_standalone_menubars) {
	myapp->raiseSystemMenuBar();
	return; // optimization
    }
    bool raisedSomething = False;
    QListIterator<Client> it(clients);
    for (it.toFirst(); it.current(); ++it){
	if (it.current() != c && it.current()->trans == c->window){
	    if (it.current()->isMenuBar()){
		it.current()->raise();
		raisedSomething |= True;
	    }
	}
    }
    if (!raisedSomething)
	myapp->raiseSystemMenuBar();
}


// send a sound event to the kwm sound module. Same function as in
// KWM from libkdecore.
 void Manager::raiseSoundEvent(const QString &event){
  XEvent ev;
  long mask = 0L;
  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = qt_xrootwin();
  ev.xclient.message_type = kde_sound_event;
  ev.xclient.format = 8;

  int i;
  const char* s = event.data();
  for (i=0;i<19 && s[i];i++)
    ev.xclient.data.b[i]=s[i];

  Window* mw;
  for (mw=modules.first(); mw; mw=modules.next()){
    ev.xclient.window = *mw;
    if (ev.xclient.window == qt_xrootwin())
      mask = SubstructureRedirectMask;
    XSendEvent(qt_xdisplay(), *mw,
	       False, mask, &ev);
  }
  XFlush(qt_xdisplay());
}

void Manager::addModule(Window w){
  Window *wp;
  for (wp = modules.first(); wp; wp = modules.next())
    if (*wp == w)
      return;
  wp = new Window;
  *wp = w;
  modules.append(wp);
  sendClientMessage(w, module_init, 0);

  if (KWM::isKWMDockModule(w)){
    // the module wants to be even more: a module that handles the
    // docking area
    if (dock_module != None){
      // we have already a dock module. That means the module can only
      // become a normal kwm module
      KWM::setKWMModule(w);
    }
    else{
      dock_module = w;
      Window *dw;
      for (dw = dock_windows.first(); dw; dw = dock_windows.next())
	sendClientMessage(dock_module, module_dockwin_add, (long) *dw);
    }
  }

  // inform the new module about the current state of the
  // desktop. This means the managed windows (which are not hidden for
  // modules) and its current stacking order (important for a pager,
  // for example)
  Client* c;
  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
      c = it.current();
      if (!c->hidden_for_modules){
	  if ( c->isDialog() )
	      sendClientMessage(w, module_dialog_win_add, c->window);
	  sendClientMessage(w, module_win_add, c->window);
      }
  }
  for (c=clients_sorted.first(); c; c=clients_sorted.next()){
    if (!c->hidden_for_modules)
      sendClientMessage(w, module_win_raise, (long) c->window);
  }
  if (current())
    sendClientMessage(w, module_win_activate, (long) current()->window);

  // last not least inform the new module about all soundevents that
  // have been registered so far.
  const char* s;
  for (s=sound_events.first(); s; s=sound_events.next()){
    XEvent ev;
    int status;
    long mask;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.message_type = kde_register_sound_event;
    ev.xclient.format = 8;
    int i;
    for (i=0;i<19 && s[i];i++)
      ev.xclient.data.b[i]=s[i];
    mask = 0L;
    Window* mw;
    for (mw=modules.first(); mw; mw=modules.next()){
      ev.xclient.window = *mw;
      if (ev.xclient.window == qt_xrootwin())
	mask = SubstructureRedirectMask;
      status = XSendEvent(qt_xdisplay(), ev.xclient.window,
			  False, mask, &ev);
    }
  }

  // finally tell the module that it is initilaized now
  sendClientMessage(w, module_initialized, 0);

  raiseElectricBorders();
}

// removes a module from the list. If w is not a module
// communication window then it does nothing.
void Manager::removeModule(Window w){
  Window *mw;
  for (mw=modules.first(); mw; mw=modules.next()){
    if (*mw == w){
      modules.remove();
      delete mw;
      break;
    }
  }
  if (dock_module == w)
    dock_module = None;
}

// send a messages to all modules. Usually these messages (stored in
// the atom a) are about a certain client. This client is passed as
// c. You may sometimes also want to pass a window directly. Then
// pass 0 as client and the window as w.  Clients can have
// hidden_for_modules set. In this case sendToModules will ignore
// your request.
void Manager::sendToModules(Atom a, Client* c, Window w){
  if (c){
    if (c->hidden_for_modules)
      return;
    w = c->window;
  }
  Window* mw;
  for (mw=modules.first(); mw; mw=modules.next())
    sendClientMessage(*mw, a, (long) w);

}

// adds a new dock window to the docking area. Informs the
// dock_module about the change.
void Manager::addDockWindow(Window w){
  bool already_there = False;
  Window* w2;
  for (w2=dock_windows.first(); w2; w2=dock_windows.next()){
    already_there = already_there || *w2 == w;
  }

  if (!already_there) {
    Window *wp = new Window;
    *wp = w;
    dock_windows.append(wp);

    XSelectInput(qt_xdisplay(), w,
		 StructureNotifyMask
		 );


    if (dock_module != None)
      sendClientMessage(dock_module, module_dockwin_add, (long) w);

  }
}

// removes a new dock window from the docking area. Informs the
// dock_module about the change. If w is no dock window it does
// nothing.
void Manager::removeDockWindow(Window w){
  Window *dw;
  for (dw=dock_windows.first(); dw; dw=dock_windows.next()){
    if (*dw == w){
      dock_windows.remove();
      delete dw;
      if (dock_module != None)
	sendClientMessage(dock_module, module_dockwin_remove, (long) w);
      break;
    }
  }
}

// adds a top window
void Manager::addTopWindow(Window w)
{
    //debug("add top window");
  bool already_there = False;
  Window* w2;
  for (w2=top_windows.first(); w2; w2=top_windows.next()){
    already_there = already_there || *w2 == w;
  }

  if (!already_there) {
    Window *wp = new Window;
    *wp = w;
    top_windows.append(wp);

    XSelectInput(qt_xdisplay(), w,
		 StructureNotifyMask
		 );
  }
}

// removes a top window
void Manager::removeTopWindow(Window w)
{
  Window *tw;
  for (tw=top_windows.first(); tw; tw=top_windows.next()){
    if (*tw == w){
	//debug("remove top window");
      top_windows.remove();
      delete tw;
      break;
    }
  }
}


  // decoration tricks to be somewhat compatible with traditional window managers.
  // same for supressing focus.
void Manager::doGlobalDecorationAndFocusHints(Client* c){
  char* s;
  QRegExp r;

  if (!c->label.isEmpty()){
    if (c->getDecoration() == KWM::normalDecoration){
      for (s = no_decoration_titles.first(); s ; s = no_decoration_titles.next()){
	r = s;
	if (r.match(c->label) != -1){
	  c->decoration = KWM::noDecoration;
	  break;
	}
      }
    }
    if (c->getDecoration() == KWM::normalDecoration){
      for (s = tiny_decoration_titles.first(); s ; s = tiny_decoration_titles.next()){
	r = s;
	if (r.match(c->label) != -1){
	  c->decoration = KWM::tinyDecoration;
	  break;
	}
      }
    }
    if (c->wantsFocus()){
      for (s = no_focus_titles.first(); s ; s = no_focus_titles.next()){
	r = s;
	if (r.match(c->label) != -1){
	  c->wants_focus = false;
	  break;
	}
      }
    }
    //CT 03Nov1998
    if(!c->isSticky()) {
      for (s = sticky_titles.first(); s; s = sticky_titles.next()) {
	r = s;
	if (r.match(c->label) != -1) {
	  c->sticky = true;
	  break;
	}
      }
    }
    //CT
  }
  if (!c->instance.isEmpty()){
    if (c->getDecoration() == KWM::normalDecoration){
      for (s = no_decoration_classes.first(); s ; s = no_decoration_classes.next()){
	r = s;
	if (r.match(c->instance) != -1){
	  c->decoration = KWM::noDecoration;
	  break;
	}
      }
    }
    if (c->getDecoration() == KWM::normalDecoration){
      for (s = tiny_decoration_classes.first(); s ; s = tiny_decoration_classes.next()){
	r = s;
	if (r.match(c->instance) != -1){
	  c->decoration = KWM::tinyDecoration;
	  break;
	}
      }
    }
    if (c->wantsFocus()){
      for (s = no_focus_classes.first(); s ; s = no_focus_classes.next()){
	r = s;
	if (r.match(c->instance) != -1){
	  c->wants_focus = false;
	  break;
	}
      }
    }
    //CT 03Nov1998
    if(!c->isSticky()) {
      for (s = sticky_classes.first(); s; s = sticky_classes.next()) {
	r = s;
	if (r.match(c->instance) != -1) {
	  c->sticky = true;
	  break;
	}
      }
    }
    //CT
  }
  if (!c->klass.isEmpty()){
    if (c->getDecoration() == KWM::normalDecoration){
      for (s = no_decoration_classes.first(); s ; s = no_decoration_classes.next()){
	r = s;
	if (r.match(c->klass) != -1){
	  c->decoration = KWM::noDecoration;
	  break;
	}
      }
    }
    if (c->getDecoration() == KWM::normalDecoration){
      for (s = tiny_decoration_classes.first(); s ; s = tiny_decoration_classes.next()){
	r = s;
	if (r.match(c->klass) != -1){
	  c->decoration = KWM::tinyDecoration;
	  break;
	}
      }
    }
    if (c->wantsFocus()){
      for (s = no_focus_classes.first(); s ; s = no_focus_classes.next()){
	r = s;
	if (r.match(c->klass) != -1){
	  c->wants_focus = false;
	  break;
	}
      }
    }
    //CT 03Nov1998
    if(!c->isSticky()) {
      for (s = sticky_classes.first(); s; s = sticky_classes.next()) {
	r = s;
	if (r.match(c->klass) != -1) {
	  c->sticky = true;
	  break;
	}
      }
    }
    //CT
  }
}


// kills all modules and launches another window manager
void Manager::launchOtherWindowManager(const char* other){
  Window* mw;
  for (mw=modules.first(); mw; mw=modules.next()){
    XKillClient(qt_xdisplay(), *mw);
  }
  myapp->cleanup();
  XSync(qt_xdisplay(), false);
  execlp(other, NULL);
}




//// CC: Implementation of the KDE Greyer Widget

// The greyer widget is used to put a grey veil over the screen in the
// darkenScreen method.
KGreyerWidget::KGreyerWidget():
  QWidget(0,0,WStyle_Customize|WStyle_NoBorder)
{
  setBackgroundMode(QWidget::NoBackground);
  setGeometry(0,0, QApplication::desktop()->width(),
		   QApplication::desktop()->height());
  show();
}

void KGreyerWidget::paintEvent(QPaintEvent *)
{
 QPainter p;
 QBrush b;

 b.setStyle(Dense4Pattern);
 p.begin(this);
 p.fillRect(geometry(),b);
 p.end();
}

