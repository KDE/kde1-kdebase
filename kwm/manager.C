/*
 * manager.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include "manager.moc"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <kwm.h>

extern bool kwm_error;
extern QList<Window> own_toplevel_windows;

extern Manager* manager;

Window root;
Display *dpy;
GC rootgc;
GC rootfillgc;
GC rootfillsolidgc;
static char stipple_bits[] = {
  0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
  0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
  0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa};

extern bool ignore_badwindow; // for the X error handler


Manager::Manager(): QObject(){
  manager = this;
  current_desktop = KWM::currentDesktop();
  number_of_desktops = KWM::numberOfDesktops();

  proxy_hints = NULL;
  proxy_props = NULL;
  
  int dummy;
  XGCValues gv;
  unsigned long mask;
  
  dpy = qt_xdisplay();
  root = qt_xrootwin();
  default_colormap = DefaultColormap(qt_xdisplay(), qt_xscreen());
  
  wm_state = XInternAtom(qt_xdisplay(), "WM_STATE", False);
  wm_change_state = XInternAtom(qt_xdisplay(), "WM_CHANGE_STATE", False);
  wm_protocols = XInternAtom(qt_xdisplay(), "WM_PROTOCOLS", False);
  wm_delete_window = XInternAtom(qt_xdisplay(), "WM_DELETE_WINDOW", False);
  wm_take_focus = XInternAtom(qt_xdisplay(), "WM_TAKE_FOCUS", False);
  wm_save_yourself = XInternAtom(qt_xdisplay(), "WM_SAVE_YOURSELF", False);
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
  kwm_win_decorated = XInternAtom(qt_xdisplay(),"KWM_WIN_DECORATED",False);
  kwm_win_icon = XInternAtom(qt_xdisplay(),"KWM_WIN_ICON",False);

  kwm_command = XInternAtom(qt_xdisplay(), "KWM_COMMAND", False);
  kwm_activate_window = XInternAtom(qt_xdisplay(), "KWM_ACTIVATE_WINDOW", False);

  kwm_running = XInternAtom(qt_xdisplay(), "KWM_RUNNING", False);

  // for the modules
  kwm_module = XInternAtom(qt_xdisplay(), "KWM_MODULE", False);
  module_init = XInternAtom(qt_xdisplay(), "KWM_MODULE_INIT", False);
  module_desktop_change = XInternAtom(qt_xdisplay(), "KWM_MODULE_DESKTOP_CHANGE", False);
  
  module_win_add = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_ADD", False);
  module_win_remove = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_REMOVE", False);
  module_win_change = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_CHANGE", False);
  module_win_raise = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_RAISE", False);
  module_win_lower = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_LOWER", False);
  module_win_activate = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_ACTIVATE", False);
  dock_module = None;
  module_dockwin_add = XInternAtom(qt_xdisplay(), "KWM_MODULE_DOCKWIN_ADD", False);
  module_dockwin_remove = XInternAtom(qt_xdisplay(), "KWM_MODULE_DOCKWIN_REMOVE", False);

  gv.function = GXxor;
  gv.line_width = 0;
  gv.foreground = WhitePixel(qt_xdisplay(), qt_xscreen())^BlackPixel(qt_xdisplay(), qt_xscreen());
  gv.subwindow_mode = IncludeInferiors;
  mask = GCForeground | GCFunction | GCLineWidth
    | GCSubwindowMode;
  gc = XCreateGC(qt_xdisplay(), qt_xrootwin(), mask, &gv);
  rootgc = gc;

  rootfillgc = XCreateGC(qt_xdisplay(), qt_xrootwin(), mask, &gv);
  gv.function = GXcopy;
  gv.foreground = BlackPixel(qt_xdisplay(), qt_xscreen());
  rootfillsolidgc = XCreateGC(qt_xdisplay(), qt_xrootwin(), mask, &gv);
  
  Pixmap stipple = XCreateBitmapFromData(qt_xdisplay(), qt_xrootwin(), stipple_bits, 16, 16);
  XSetStipple(qt_xdisplay(), rootfillgc, stipple);
  XSetFillStyle(qt_xdisplay(), rootfillgc, FillStippled);
  XSetStipple(qt_xdisplay(), rootfillsolidgc, stipple);
  XSetFillStyle(qt_xdisplay(), rootfillsolidgc, FillStippled);
  
  shape = XShapeQueryExtension(qt_xdisplay(), &shape_event, &dummy);

  scanWins();
  if (!current()){
    noFocus();
  }
}


void Manager::createNotify(XCreateWindowEvent *e){
  if (e->parent== qt_xrootwin()){ 
    if (KWM::isKWMModule(e->window))
      addModule(e->window);
    if (!getClient(e->window)){
      Window *wp;
      for (wp = own_toplevel_windows.first(); 
	   wp && *wp != e->window; 
	   wp = own_toplevel_windows.next());
      if (!wp){
	XSelectInput(qt_xdisplay(), e->window, PropertyChangeMask);
      }
      
    }
  }
}


void Manager::configureRequest(XConfigureRequestEvent *e){

  XWindowChanges wc;
  Client *c;

  c = getClient(e->window);

  if (c && c->window == e->window) { // client already exists for this window

    int x = c->geometry.x();
    int y = c->geometry.y();
    int dx = c->geometry.width();
    int dy = c->geometry.height();


    if (e->value_mask & CWX)
      x = e->x;
    if (e->value_mask & CWY)
      y = e->y;

    if (!c->isDecorated()){
      if (e->value_mask & CWWidth)
	dx = e->width;
      if (e->value_mask & CWHeight)
	dy = e->height;
    }
    else {
      if (e->value_mask & CWWidth)
	dx = e->width + 2 * BORDER;
      if (e->value_mask & CWHeight)
	dy = e->height + 2 * BORDER + TITLEBAR_HEIGHT;
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
    if (c->isDecorated()){
      wc.x = BORDER;
      wc.y = BORDER+TITLEBAR_HEIGHT;
    }
    else {
      wc.x = 0;
      wc.y = 0;
    }
    wc.width = e->width;
    wc.height = e->height;
    
    XConfigureWindow(qt_xdisplay(), e->window, e->value_mask, &wc);
    sendConfig(c); 

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
  }
  else {
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
        XMapWindow(qt_xdisplay(), c->winId());
        XMapWindow(qt_xdisplay(), c->window);
        raiseClient(c);
        setWindowState(c, NormalState);
	activateClient(c);
        break;
      }
    }
}


void Manager::unmapNotify(XUnmapEvent *e){
    Client *c;
    c = getClient(e->window);
    if (c) {
      if(c->reparenting){
	c->reparenting = FALSE;
	return;
      }
      switch (c->state) {
      case IconicState:
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
    }
    removeDockWindow(e->window);
}

void Manager::destroyNotify(XDestroyWindowEvent *e){
    Client *c;
    removeModule(e->window);
    c = getClient(e->window);
    if (c == 0 || c->window != e->window){
        return;
    }
    removeClient(c);
}

void Manager::clientMessage(XClientMessageEvent *  e){
  Client *c;
  
  if (e->message_type == wm_change_state) {
    c = getClient(e->window);
    if (c && e->format == 32 && e->data.l[0] == IconicState) {
      if (c->state == NormalState)
	c->iconify();
    }
  }
  if (e->message_type == kwm_command){
    QString com = e->data.b;
    if (com == "refreshScreen")
      refreshScreen();
    else if (com == "darkenScreen")
      darkenScreen();
    else if (com == "logout")
      logout();
    else if (com == "configure"){
      emit reConfigure();
      Client* t;
      for (t = clients_sorted.first(); t; t=clients_sorted.next())
	t->reconfigure();
    }
    else {
      // send unknown command to the modules
      XEvent ev;
      int status;
      long mask;
      memset(&ev, 0, sizeof(ev));
      ev.xclient.type = ClientMessage;
      ev.xclient.message_type = kwm_command;
      ev.xclient.format = 8;
      int i;
      const char* s = com.data();
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
  }
  if (e->message_type == kwm_activate_window){
    Window w = (Window) e->data.l[0];
    c = getClient(w);
    if (c && c->state == NormalState)
      activateClient(c);
  }

}

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

void Manager::propertyNotify(XPropertyEvent *e){
  Atom a;
  int del;
  Client *c;
  
  a = e->atom;
  del = (e->state == PropertyDelete);

  if (e->window == qt_xrootwin()){
    if (a == kwm_current_desktop){
      switchDesktop(KWM::currentDesktop());
    }
    if (a == kwm_number_of_desktops){
      number_of_desktops = KWM::numberOfDesktops();
    }
    return;
  }
  
  if (a == kwm_module){
    if (KWM::isKWMModule(e->window))
      addModule(e->window);
    else
      removeModule(e->window);
  }

  c = getClient(e->window);
  if (c == 0 || c->window != e->window)
    return;
  
  switch (a) {
  case XA_WM_ICON_NAME:
  case XA_WM_NAME:
    if (a== XA_WM_ICON_NAME){
      c->iconname = getprop(c->window, XA_WM_ICON_NAME);
    }
    else
      c->name = getprop(c->window, XA_WM_NAME);
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
    if(c->trans){
      int i;
      for (i=0; i<6; i++){
	if (c->buttons[i])
	  c->buttons[i]->hide();
      }
    }
    return;
  case XA_WM_NORMAL_HINTS:
    getWMNormalHints(c);
    return;
  case XA_WM_COMMAND:
    c->command = getprop(c->window, XA_WM_COMMAND);
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
  }
  else if (a == kwm_win_iconified){
    if (KWM::isIconified(c->window))
      c->iconify();
    else
      c->unIconify();
  }
  else if (a == kwm_win_maximized){
    if (c->isMaximized() != KWM::isMaximized(c->window)){
      if (c->isMaximized())
	c->unMaximize();
      else
	c->maximize();
    }
  }
  else if (a == kwm_win_sticky){
    if (c->isSticky() != KWM::isSticky(c->window))
      c->buttonSticky->toggle();
  }
  else if (a == kwm_win_decorated){
    if (c->isDecorated() != KWM::isDecorated(c->window)){
      if (KWM::isDecorated(c->window)){
	if (c->decoration_not_allowed)
	  KWM::setDecoration(c->window, FALSE);
	else {
	  c->decorated = TRUE;
	  gravitate(c, FALSE);
	  sendConfig(c);
	}
      }
      else {
	gravitate(c, TRUE);
	c->decorated = FALSE;
	sendConfig(c);
      }
    }
  }
}

void Manager::shapeNotify(XShapeEvent *e){
  Client *c;
    c = getClient(e->window);
    if (c)
      setShape(c);
}

void Manager::enterNotify(XCrossingEvent *e){
  Client *c;
  if (options.FocusPolicy == FOCUS_FOLLOW_MOUSE){ 
    c = getClient(e->window);
    if (c != 0 && c != current() && c->state != WithdrawnState) {
      activateClient(c);
    }
  }
}


void Manager::randomPlacement(Client* c){
  static int px = TITLEBAR_HEIGHT + BORDER;
  static int py = 2 * TITLEBAR_HEIGHT + BORDER;
  int tx,ty;

  QRect maxRect = KWM::getWindowRegion(currentDesktop());

  if (px < maxRect.x())
    px = maxRect.x() + TITLEBAR_HEIGHT + BORDER;
  if (py < maxRect.y())
    py = maxRect.y() + 2 * TITLEBAR_HEIGHT + BORDER;
  

  px += TITLEBAR_HEIGHT + BORDER;
  py += 2*TITLEBAR_HEIGHT + BORDER;

  if (px > maxRect.width()/2) 
    px =  maxRect.x() + TITLEBAR_HEIGHT + BORDER;
  if (py > maxRect.height()/2) 
    py =  maxRect.y() + 2*TITLEBAR_HEIGHT + BORDER;
  tx = px;
  ty = py;
  if (tx + c->geometry.width() > maxRect.right()){
    tx = maxRect.right() - c->geometry.width();
    if (tx < 0)
      tx = 0;
    px =  maxRect.x() + TITLEBAR_HEIGHT + BORDER;
  }
  if (ty + c->geometry.height() > maxRect.bottom()){
    ty = maxRect.bottom() - c->geometry.height();
    if (ty < 0)
      ty = 0;
    py =  maxRect.y() + TITLEBAR_HEIGHT + BORDER;
  }
  c->geometry.moveTopLeft(QPoint(tx, ty));
}


void Manager::manage(Window w, bool mapped){

  bool dohide;
  bool pseudo_session_management = FALSE;
  int state;
  XClassHint klass;
  XWMHints *hints;
  XWindowAttributes attr;
  
  XGrabServer(qt_xdisplay()); // we want to be alone...
  
  // create a client
  if (KWM::isDockWindow(w)){
    addDockWindow(w);
    return;
  }
  Client* c = getClient(w);
  if (!c){
    // create a very new client
    c = new Client(w);
    
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
    c->command = getprop(c->window, XA_WM_COMMAND);
    c->machine = getprop(c->window, wm_client_machine);
    if (proxy_hints && !proxy_hints->isEmpty() && !c->command.isEmpty()){
      QString s = c->command + " @ " + c->machine;
      unsigned int i;
      for (i=0; i<proxy_hints->count(); i++){
	if (s == proxy_hints->at(i)){
	  QString d = proxy_props->at(i);
	  KWM::setProperties(c->window, d);
	  pseudo_session_management = TRUE;
	  XSync(qt_xdisplay(), False);
	  proxy_hints->removeRef(NULL);
	  proxy_props->removeRef(NULL);
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
  {
    int n, order;
    // don't show any visible decoration, if the window is shaped
    XShapeGetRectangles(qt_xdisplay(), c->window, ShapeBounding, &n, &order);
    if ( n > 1 ) {
      c->decoration_not_allowed = TRUE;
      c->decorated = FALSE;
    }
  }

  // get KDE specific decoration hint
  if (c->isDecorated())
    c->decorated = KWM::isDecorated(c->window);

  XSelectInput(qt_xdisplay(), c->window, ColormapChangeMask | EnterWindowMask | PropertyChangeMask);
  
  if (XGetClassHint(qt_xdisplay(), c->window, &klass) != 0) {   // Success
    c->instance = klass.res_name;
    c->klass = klass.res_class;
  }
  else {
    c->instance = "";
    c->klass = "";
  }
  c->iconname = getprop(c->window, XA_WM_ICON_NAME);
  c->name = getprop(c->window, XA_WM_NAME); 
  c->setLabel();

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
  if (hints)
    XFree(hints);

  getWMNormalHints(c);
  getColormaps(c);
  getWindowTrans(c);
  getMwmHints(c);

  if(c->trans || !c->isDecorated()){
    int i;
    for (i=0; i<6; i++){
      if (c->buttons[i])
	c->buttons[i]->hide();
    }
  }

  // readjust dx,dy,x,y so they are Client sizes
  if (!pseudo_session_management)
    gravitate(c,0);
  else if (c->isDecorated()) { 
    c->geometry.setWidth(c->geometry.width() + 2*BORDER);
    c->geometry.setHeight(c->geometry.height()+ 2*BORDER+TITLEBAR_HEIGHT);
  }
  
  if (mapped || c->trans 
      ||c->size.flags & PPosition
      ||c->size.flags & USPosition 
      || pseudo_session_management
      ){
      // nothing
  }
  else {
    randomPlacement(c);
  }

  if (mapped)
    c->reparenting = TRUE;
  XSetWindowBorderWidth(qt_xdisplay(), c->window, 0);
  if (c->isDecorated())
    XReparentWindow(qt_xdisplay(), c->window, c->winId(), (BORDER), (BORDER) + 
		    TITLEBAR_HEIGHT);
  else
    XReparentWindow(qt_xdisplay(), c->window, c->winId(), 0, 0);
  
  if (shape) {
    XShapeSelectInput(qt_xdisplay(), c->window, ShapeNotifyMask);
    ignore_badwindow = TRUE;       /* magic */
    setShape(c);
    ignore_badwindow = FALSE;
  }
  
  XAddToSaveSet(qt_xdisplay(), c->window);
  XSync(dpy, False);
  sendConfig(c);
  XSync(qt_xdisplay(), False);

  // get some KDE specific hints
  int desktop_tmp = KWM::desktop(c->window);
  if (!kwm_error)
    c->desktop = desktop_tmp;
  else {
    KWM::moveToDesktop(c->window, c->desktop);
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

  addClient(c);

  if (dohide){
    XUnmapWindow(qt_xdisplay(), c->window);
    c->hide();
    setWindowState(c, IconicState);
  }
  else {
    c->show();
    XMapWindow(qt_xdisplay(), c->window);
    setWindowState(c, NormalState);
    raiseClient(c);
    
    if (current() != c)
      colormapFocus(current());
  }

  if (KWM::isMaximized(c->window)){
    QRect tmprec = KWM::geometryRestore(c->window);

    /* Rethink this TODO. I cannot do maximize since this would
     * break the tree different maximize levels 
     * (full, horizontal, vertical). I do not want to introduce
     * a integer flag for maximize. Let us try is this way.
     * The only drawback: a application that wants to start up
     * maximized (not session management) must call setMaximize
     * after it has been mapped. Should not be a problem.
     */

    // avoid flickering
    c->maximized = TRUE;
    c->buttonMaximize->toggle();
    
//     if (mapped || pseudo_session_management){
//       // avoid flickering
//       c->maximized = TRUE;
//       c->buttonMaximize->toggle();
//     }
//     else {
//       c->maximize();
//     }
    c->geometry_restore = tmprec;
  }
  
  if (!dohide){
    if (c->isDecorated())
      activateClient(c);
    else
      c->setactive(FALSE);
  }
 
  sendToModules(module_win_add, c->window);
  XUngrabServer(qt_xdisplay()); 
  
}

void Manager::withdraw(Client* c){
  XUnmapWindow(qt_xdisplay(), c->winId());
  gravitate(c, TRUE);
  XReparentWindow(qt_xdisplay(), c->window, qt_xrootwin(), 
		  c->geometry.x() , c->geometry.y());
  XRemoveFromSaveSet(qt_xdisplay(), c->window);
  setWindowState(c, WithdrawnState);
  
  removeClient(c);
}


Client* Manager::getClient(Window w){
  Client* result;
  for (result = clients.first();
       result && result->window != w && result->winId() != w;
       result = clients.next());
  return result;
}

Client* Manager::current(){
  Client* result = clients_traversing.last();

  return (result && result->isActive()) ? result : (Client*)NULL;
}

void Manager::addClient(Client* c){
  clients.append(c);
  clients_sorted.append(c);
  clients_traversing.insert(0,c);
}


void Manager::activateClient(Client* c, bool set_revert){
  Client* cc = current();
  if (c == cc)
    return;
  if (cc)
    cc->setactive( FALSE );
  c->setactive( TRUE );
  
  XSetInputFocus(qt_xdisplay(), c->window, RevertToPointerRoot, timeStamp());
  
  // for FocusFollowMouse: discard all enter/leave events
  XEvent ev;
  while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));

  if (c->Ptakefocus)
    sendClientMessage(c->window, wm_protocols, wm_take_focus);
  colormapFocus(c);
  clients_traversing.removeRef(c);
  clients_traversing.append(c);
  if (!set_revert && cc){
    clients_traversing.removeRef(cc);
    clients_traversing.insert(0,cc);
  }
  XChangeProperty(qt_xdisplay(), qt_xrootwin(), kwm_active_window, 
		  kwm_active_window, 32,
		  PropModeReplace, (unsigned char *)&(c->window), 1);
  sendToModules(module_win_activate, c->window);
}

void Manager::removeClient(Client* c){
  bool do_nofocus = (current() == c);
  clients.removeRef(c);
  clients_sorted.removeRef(c);
  clients_traversing.removeRef(c);
  if (do_nofocus)
    noFocus();
  sendToModules(module_win_remove, c->window);
  delete c;

}



void Manager::raiseClient(Client* c){
  QList <Client> tmp;
  QList <Client> sorted_tmp = clients_sorted;

  sorted_tmp.removeRef(c);
  // raise the client
  tmp.append(c);

  // raise all transient windows now
  Client *cc = NULL;
  Client* ccc = NULL;
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
	if (cc->trans == ccc->window)
	  tmp2.append(cc);
      }
    }
  } while (!tmp2.isEmpty());

  // finally do the raising

  for (c=tmp.first();c;c=tmp.next()){
    clients_sorted.removeRef(c);
    clients_sorted.append(c);
    sendToModules(module_win_raise, c->window);
  }
  // X Semantics are somewhat cryptic
  Window* new_stack = new Window[tmp.count()];
  int i = 0;
  for (c=tmp.last();c;c=tmp.prev()){
    new_stack[i] = c->winId();
    i++;
  }
  XRaiseWindow(qt_xdisplay(), new_stack[0]);
  XRestackWindows(qt_xdisplay(), new_stack, i);
  delete [] new_stack;
}

void Manager::lowerClient(Client* c){
  clients_sorted.removeRef(c);
  clients_sorted.insert(0,c);
  sendToModules(module_win_lower, c->window);
  XLowerWindow(qt_xdisplay(), c->winId());
}

void Manager::closeClient(Client* c){
  // clients with WM_DELETE_WINDOW protocol set (=Pdeletewindow) are
  // closed via wm_delete_window ClientMessage.
  // Others are destroyed.

  if (c->Pdeletewindow){
    sendClientMessage(c->window, wm_protocols, wm_delete_window);
  }
  else {
    // client will not react on wm_delete_window. We have not choice
    // but destroy his connection to the XServer.
    XDestroyWindow(qt_xdisplay(), c->window);
    removeClient(c);
  }
}

void Manager::changedClient(Client* c){
  sendToModules(module_win_change, c->window);
}

void Manager::noFocus(){
  static Window w = 0;
  int mask;
  XSetWindowAttributes attr;
  Client* c;
  for (c = clients_traversing.last();
       c && (c->isActive() || c->state != NormalState); 
       c = clients_traversing.prev());
  if (c && c->state == NormalState) {
    activateClient(c, False);
    return;
  }
  
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
  sendToModules(module_win_activate, 0);
  
}

void Manager::setWindowState(Client *c, int state){
  unsigned long data[2];
  
  data[0] = (unsigned long) state;
  data[1] = (unsigned long) None;
  
  c->state = state;
  
  XChangeProperty(qt_xdisplay(), c->window, wm_state, wm_state, 32,
		  PropModeReplace, (unsigned char *)data, 2);
}



void Manager::getWindowTrans(Client* c){
  Window trans = None;
  if (XGetTransientForHint(qt_xdisplay(), c->window, &trans))
    c->trans = trans;
}


void Manager::switchDesktop(int new_desktop){
  if (new_desktop == current_desktop)
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
      c->hide();
      XUnmapWindow(qt_xdisplay(), c->window);
      setWindowState(c, IconicState); 
    }
  }
  current_desktop = new_desktop;
  for (c=clients_sorted.last(); c ; c=clients_sorted.prev()){
    if (c->isOnDesktop(current_desktop) && !c->isIconified() && !c->isSticky()){
      c->show();
      XMapWindow(qt_xdisplay(), c->window);
      setWindowState(c, NormalState);
    }
  }
  if (!current())
    noFocus();
  KWM::switchToDesktop(current_desktop);
  sendToModules(module_desktop_change, (Window) current_desktop);
}








void Manager::sendConfig(Client* c){
  XConfigureEvent ce;

  c->setGeometry(c->geometry);

  ce.type = ConfigureNotify;
  ce.event = c->window;
  ce.window = c->window;

  
  if (c->isDecorated()){
    ce.x = c->geometry.x() + BORDER;
    ce.y = c->geometry.y() + BORDER + TITLEBAR_HEIGHT;
    ce.width = c->geometry.width() - 2*BORDER;
    ce.height = c->geometry.height() - 2*BORDER - TITLEBAR_HEIGHT;
    }
  else {
    ce.x = c->geometry.x();
    ce.y = c->geometry.y();
    ce.width = c->geometry.width();
    ce.height = c->geometry.height();
  }
  
  ce.border_width = c->border;
  ce.above = None;
  ce.override_redirect = 0;
  XSendEvent(qt_xdisplay(), c->window, False, StructureNotifyMask, (XEvent*)&ce);
  changedClient(c);
}



void Manager::cleanup(){
  Client *c;
  XWindowChanges wc;
  
  for (c = clients.first(); c; c = clients.next()) {
    gravitate(c, TRUE);
    
    XReparentWindow(qt_xdisplay(), c->window, qt_xrootwin(), c->geometry.x() , c->geometry.y());
    
    wc.border_width = c->border;
    XConfigureWindow(qt_xdisplay(), c->window, CWBorderWidth, &wc);
  }
  XSetInputFocus(qt_xdisplay(), PointerRoot, RevertToPointerRoot, CurrentTime);
  colormapFocus(0);
}

void Manager::repaintAll(){
  Client* c;
  for (c=clients_sorted.last(); c; c = clients_sorted.prev())
    c->paintState();
}


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


void Manager::scanWins(){
  unsigned int i, nwins;
  Window dw1, dw2, *wins;
  XWindowAttributes attr;
  
  XQueryTree(qt_xdisplay(), qt_xrootwin(), &dw1, &dw2, &wins, &nwins);
  for (i = 0; i < nwins; i++) {
    if (KWM::isKWMModule(wins[i])){
      XSelectInput(qt_xdisplay(), wins[i], PropertyChangeMask);
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


void Manager::setShape(Client* c){
  int n, order;
  XRectangle *rect;
  
  /* cheat: don't try to add a border if the window is non-rectangular */
  rect = XShapeGetRectangles(qt_xdisplay(), c->window, ShapeBounding, &n, &order);
  if (n > 1){
    if (c->isDecorated())
      XShapeCombineShape(qt_xdisplay(), c->winId(), ShapeBounding, 
			 (BORDER), (BORDER) + TITLEBAR_HEIGHT,
			 c->window, ShapeBounding, ShapeSet);
      
    else 
      XShapeCombineShape(qt_xdisplay(), c->winId(), ShapeBounding, 
			 0, 0,
			 c->window, ShapeBounding, ShapeSet);
    
  }
  XFree((void*)rect);
}

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

bool Manager::hasLabel(QString label_arg){
  bool Result = False;
  Client *c;
  for (c=clients.first(); c; c=clients.next()){
    Result = Result || c->label == label_arg;
  }
  return Result;
}


void Manager::getWindowProtocols(Client *c){
  Atom *p;
  int i,n;
  
  if (XGetWMProtocols(qt_xdisplay(), c->window, &p, &n)){
    for (i = 0; i < n; i++)
      if (p[i] == wm_delete_window)
	c->Pdeletewindow = TRUE;
      else if (p[i] == wm_take_focus)
	c->Ptakefocus = TRUE;
      else if (p[i] == wm_save_yourself){
	c->Psaveyourself = TRUE;
      }
    if (n>0)
      XFree(p);
  }
}

// Motif support (Matthias)
struct PropMotifWmHints
{
  INT32      flags;
  INT32      functions;
  INT32      decorations;
  INT32      inputMode;
};

#define MWM_DECOR_ALL                 (1L << 0)
#define MWM_DECOR_BORDER              (1L << 1)
#define MWM_DECOR_RESIZEH             (1L << 2)
#define MWM_DECOR_TITLE               (1L << 3)
#define MWM_DECOR_MENU                (1L << 4)
#define MWM_DECOR_MINIMIZE            (1L << 5)
#define MWM_DECOR_MAXIMIZE            (1L << 6)

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
	  // only interested in decorations
  	  if (c->isDecorated()){
  	    c->decorated = (mwm_hints->decorations != 0);
	    KWM::setDecoration(c->window, c->isDecorated());
	  }
	}
      XFree((char *)mwm_hints);
    }
  
}


void Manager::gravitate(Client* c, bool invert){
  int gravity, dx, dy, delta;
  if (!c->isDecorated())
    return;
  dx = dy = 0;

  gravity = NorthWestGravity;
  if (c->size.flags & PWinGravity)
    gravity = c->size.win_gravity;
  
  //  delta = c->border-BORDER;
  delta = BORDER;
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
    dy = -delta-TITLEBAR_HEIGHT;
    break;
  case CenterGravity:
  case StaticGravity:
    dx = -delta;
    dy = -delta-TITLEBAR_HEIGHT;
    break;
  case EastGravity:
    dx = -2*delta;
    dy = -delta-TITLEBAR_HEIGHT;
    break;
  case SouthWestGravity:
    dx = 0;
    dy = -2*delta-TITLEBAR_HEIGHT;
    break;
  case SouthGravity:
    dx = -delta;
    dy = -2*delta-TITLEBAR_HEIGHT;
    break;
  case SouthEastGravity:
    dx = -2*delta;
    dy = -2*delta-TITLEBAR_HEIGHT;
    break;
  default:
    fprintf(stderr, "kwm: bad window gravity %d for 0x%lx\n", gravity, c->window);
  }
  if (invert) {
    c->geometry.moveBy(-dx, -dy);
    c->geometry.setWidth(c->geometry.width() - 2*BORDER);
    c->geometry.setHeight(c->geometry.height()- 2*BORDER-TITLEBAR_HEIGHT);
  }
  else {
    c->geometry.moveBy(dx, dy);
    c->geometry.setWidth(c->geometry.width() + 2*BORDER);
    c->geometry.setHeight(c->geometry.height()+ 2*BORDER+TITLEBAR_HEIGHT);
  }
}

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

QString Manager::getprop(Window w, Atom a){
  QString result;
  unsigned char *p;
  if (_getprop(w, a, XA_STRING, 100L, &p) > 0){
    result = (char*) p;
    XFree((char *) p);
  }
  return result;
}


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


void Manager::refreshScreen(){
  XSetWindowAttributes attributes;
  unsigned long valuemask;

  // this is a bit tricky: Clients with backing_store set
  // would not get the exposure event! But I do not know
  // what to do, if a server always does backing store....
  // In this case, I have to save the screen manually
  // _before_ darkenScreen.

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

void Manager::darkenScreen(){
  XFillRectangle(qt_xdisplay(), qt_xrootwin(), rootfillsolidgc, 
		 0,0,
		 QApplication::desktop()->width(),
		 QApplication::desktop()->height());
}

void Manager::logout(){
  Client* c;
  XEvent ev;
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
    // unmanaged root windows
    unsigned int i, nwins;
    Window dw1, dw2, *wins;

    additional_commands.clear();
    additional_machines.clear();
    additional_proxy_hints.clear();
    additional_proxy_props.clear();

    XQueryTree(qt_xdisplay(), qt_xrootwin(), &dw1, &dw2, &wins, &nwins);
    for (i = 0; i < nwins; i++) {
      if (!getClient(wins[i])){
	  Atom *p;
	  int i2,n;
	  if (XGetWMProtocols(qt_xdisplay(), wins[i], &p, &n)){
	    for (i2 = 0; i2 < n; i2++){
	      if (p[i2] == wm_save_yourself){
		sendClientMessage(wins[i], wm_protocols, wm_save_yourself);
		// wait for clients response
		do {
		  XWindowEvent(qt_xdisplay(), wins[i], PropertyChangeMask, &ev);
		  propertyNotify(&ev.xproperty);
		} while (ev.xproperty.atom != XA_WM_COMMAND);
		command = getprop(wins[i], XA_WM_COMMAND);
		if (!command.isEmpty()){
		  machine = getprop(wins[i], wm_client_machine);
		  additional_commands.append(command);
		  additional_machines.append(machine);
		}
	      }
	    }
	    if (n>0)
	      XFree(p);
	  }
      }
    }
    if (nwins>0)
      XFree((void *) wins);   
  }
  


  for (c = clients.first(); c; c = clients.next()){
    if (c->Psaveyourself){
      sendClientMessage(c->window, wm_protocols, wm_save_yourself);
      // wait for clients response
      XSelectInput(qt_xdisplay(), c->window, 
		   PropertyChangeMask| StructureNotifyMask );
      command = getprop(c->window, XA_WM_COMMAND);
      machine = getprop(c->window, wm_client_machine);
      properties = KWM::getProperties(c->window);
      do {
	XWindowEvent(qt_xdisplay(), c->window, PropertyChangeMask
		     | StructureNotifyMask, &ev);
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
	    return;
	  }
	}
      } while (1);
      XSelectInput(qt_xdisplay(), c->window, ColormapChangeMask | 
		   EnterWindowMask | PropertyChangeMask);
    }
  }
  // do this afterwards because some clients may have set XA_WM_COMMAND for
  // other windows, too
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself){
      // poor man's "session" management a la xterm :-(
      c->command = getprop(c->window, XA_WM_COMMAND);
      if (c->command.isNull()){
	long *p;
	int n =_getprop(c->window, wm_client_leader, wm_client_leader, 1L, (unsigned char**)&p);
	if (n > 0){
	  Window w = (Window) *p;
	  if (w != c->window && getClient(w))
	    c->command = ""; // will be stored anyway
	  else
	    c->command = getprop(w, XA_WM_COMMAND);
	  XFree((char *) p);
	}
      }
    }
  }
  XAllowEvents(qt_xdisplay(), ReplayPointer, CurrentTime);
  XAllowEvents(qt_xdisplay(), ReplayKeyboard, CurrentTime);
  emit showLogout();
}

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
    if (!c->command.isEmpty()){
      // create a machine dependend command
      command = c->command.data();
      if (!c->machine.isEmpty()){
	if (c->machine != thismachine && c->machine != all){
	  command.prepend(" ");
	  command.prepend(c->machine);
	  command.prepend("rstart ");
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
	command.prepend("rstart ");
      }
    }
    result->append(command);
    command = additional_commands.next();
    machine = additional_machines.next();
  }
  return result;
}

QStrList* Manager::getPseudoSessionClients(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself && !c->command.isEmpty() 
	&& !KWM::unsavedDataHintDefined(c->window))
      result->append(c->label);
  }
  return result;
}

QStrList* Manager::getNoSessionClients(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself && c->command.isNull()
	&& (c->trans == None || c->trans == qt_xrootwin()))
      result->append(c->label);
  }
  return result;
}

QStrList* Manager::getUnsavedDataClients(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (c->Psaveyourself && KWM::containsUnsavedData(c->window))
      result->append(c->label);
  }
  return result;
}

QStrList* Manager::getClientsOfDesktop(int desk){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (c->isOnDesktop(desk) &&
	(!c->isSticky() || desk == currentDesktop())){
      if (c->isIconified())
	result->append(QString("(") + c->label + ")");
      else
	result->append(c->label);
    }
  }
  return result;
}

QStrList* Manager::getProxyHints(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself && !c->command.isEmpty())
      result->append(c->command + " @ " + c->machine);
  }
  QString add;
  for (add = additional_proxy_hints.first(); !add.isEmpty(); 
       add = additional_proxy_hints.next())
    result->append(add);
  return result;
}

QStrList* Manager::getProxyProps(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself && !c->command.isEmpty())
      result->append(KWM::getProperties(c->window));
  }
  QString add;
  for (add = additional_proxy_props.first(); !add.isEmpty(); 
       add = additional_proxy_props.next())
    result->append(add);
  return result;
}


void Manager::setProxyData(QStrList* proxy_hints_arg, QStrList* proxy_props_arg){
  proxy_hints = proxy_hints_arg;
  proxy_props = proxy_props_arg;
}


void Manager::killWindowAtPosition(int x, int y){
  Client* c;
  for (c = clients_sorted.last(); c; c = clients_sorted.prev()){
    if (c->geometry.contains(QPoint(x, y))){
      XDestroyWindow(qt_xdisplay(), c->window);
      return;
    }
  }
}

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
  return NULL;
}


void Manager::addModule(Window w){
  Window *wp;
  for (wp = modules.first(); wp; wp = modules.next())
    if (*wp == w)
      return;
  wp = new Window;
  *wp = w;
  modules.append(wp);
  if (KWM::isKWMDockModule(w)){
    if (dock_module != None) //we have already a dock module
      KWM::setKWMModule(w);
    else{
      dock_module = w;
      Window *dw;
      for (dw = dock_windows.first(); dw; dw = dock_windows.next())
	sendClientMessage(dock_module, module_dockwin_add, (long) *dw);
    }
  }
  sendClientMessage(w, module_init, 0);
  Client* c;
  for (c=clients.first(); c; c=clients.next())
    sendClientMessage(w, module_win_add, c->window);
  for (c=clients_sorted.first(); c; c=clients_sorted.next())
    sendClientMessage(w, module_win_raise, (long) c->window);
  if (current())
    sendClientMessage(w, module_win_activate, (long) current()->window);
}
void Manager::removeModule(Window w){
  Window *mw;
  for (mw=modules.first(); mw; mw=modules.next()){
    if (*mw == w){
      modules.remove();
      delete mw;
    }
  }
  if (dock_module == w)
    dock_module = None;
}

void Manager::sendToModules(Atom a, Window w){
  Window* mw;
  for (mw=modules.first(); mw; mw=modules.next())
    sendClientMessage(*mw, a, (long) w);
  
}

void Manager::addDockWindow(Window w){
  Window *wp = new Window;
  *wp = w;
  dock_windows.append(wp);

  if (dock_module != None)
    sendClientMessage(dock_module, module_dockwin_add, (long) w);
}

void Manager::removeDockWindow(Window w){
  Window *dw;
  for (dw=dock_windows.first(); dw; dw=dock_windows.next()){
    if (*dw == w){
      dock_windows.remove();
      delete dw;
      if (dock_module != None)
	sendClientMessage(dock_module, module_dockwin_remove, (long) w);
    }
  }
}




