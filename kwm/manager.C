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
#include <X11/cursorfont.h>

#include <kwm.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif

#include <kmisc.h>

extern bool kwm_error;

extern Manager* manager;

KGreyerWidget *greyer_widget = 0;

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
extern bool initting;

extern bool focus_grabbed();
extern void showMinicli();
extern void showTask();

Manager::Manager(): QObject(){
  manager = this;
  current_desktop = KWM::currentDesktop();
  number_of_desktops = KWM::numberOfDesktops();

  proxy_hints = NULL;
  proxy_props = NULL;
  proxy_ignore = NULL;
  
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

  kwm_command = XInternAtom(qt_xdisplay(), "KWM_COMMAND", False);
  kwm_do_not_manage = XInternAtom(qt_xdisplay(), "KWM_DO_NOT_MANAGE", False);
  kwm_activate_window = XInternAtom(qt_xdisplay(), "KWM_ACTIVATE_WINDOW", False);

  kwm_running = XInternAtom(qt_xdisplay(), "KWM_RUNNING", False);

  // for the modules
  kwm_module = XInternAtom(qt_xdisplay(), "KWM_MODULE", False);
  module_init = XInternAtom(qt_xdisplay(), "KWM_MODULE_INIT", False);
  module_desktop_change = XInternAtom(qt_xdisplay(), "KWM_MODULE_DESKTOP_CHANGE", False);
  module_desktop_name_change = XInternAtom(qt_xdisplay(), "KWM_MODULE_DESKTOP_NAME_CHANGE", False);
  module_desktop_number_change = XInternAtom(qt_xdisplay(), "KWM_MODULE_DESKTOP_NUMBER_CHANGE", False);
  
  module_win_add = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_ADD", False);
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
  
  {
    long data = 1;
    XChangeProperty(qt_xdisplay(), qt_xrootwin(), kwm_running, kwm_running, 32,
		    PropModeAppend, (unsigned char*) &data, 1);
  }

  delayed_focus_follow_mouse_client = NULL;
  enable_focus_follow_mouse_activation = false;

  // electric borders
  {
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
	c->showClient();
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
    if (c && c->window == e->window) {
      if(c->reparenting){
	c->reparenting = FALSE;
	return;
      }
      if (c->unmap_events > 0){
	c->unmap_events--;
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
}

void Manager::destroyNotify(XDestroyWindowEvent *e){
    Client *c;
    removeModule(e->window);
    removeDockWindow(e->window);
    c = getClient(e->window);
    if (c == 0 || c->window != e->window){
        return;
    }
    removeClient(c);
}


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
    else if (com == "configure"){
      emit reConfigure();
      Client* t;
      for (t = clients_sorted.first(); t; t=clients_sorted.next())
	t->reconfigure();
    }
    else if (Client::operationFromCommand(com) > -1){
      if (current())
	current()->handleOperation(Client::operationFromCommand(com));
    }
    else if (com == "desktop1") {
      switchDesktop(1);
    }
    else if (com == "desktop1") {
      switchDesktop(1);
    }
    else if (com == "desktop2") {
      switchDesktop(2);
    }
    else if (com == "desktop3") {
      switchDesktop(3);
    }
    else if (com == "desktop4") {
      switchDesktop(4);
    }
    else if (com == "desktop5") {
      switchDesktop(5);
    }
    else if (com == "desktop6") {
      switchDesktop(6);
    }
    else if (com == "desktop7") {
      switchDesktop(7);
    }
    else if (com == "desktop8") {
      switchDesktop(8);
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
    else if (com == "moduleRaised") {
      raiseElectricBorders();
    }
    else {
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
      activateClient(c);
      raiseSoundEvent("Window Activate");
    }
  }
  if (e->message_type == kwm_module){
    Window w = (Window) e->data.l[0];
    if (KWM::isKWMModule(w)) 
      addModule(w);
    else
      removeModule(w);
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
  static Atom da[8] = {0,0,0,0,0,0,0,0};
  Atom a;
  int del;
  Client *c;
  int i;
  if (!da[0]){
    for (i=0;i<7;i++){
      QString n;
      n.setNum(i+1);
      n.prepend("KWM_DESKTOP_NAME_");
      da[i] = XInternAtom(qt_xdisplay(), n.data(), False);
    }
  }
  
  a = e->atom;
  del = (e->state == PropertyDelete);

  if (e->window == qt_xrootwin()){
    if (a == kwm_current_desktop){
      switchDesktop(KWM::currentDesktop());
    }
    if (a == kwm_number_of_desktops){
      number_of_desktops = KWM::numberOfDesktops();
      sendToModules(module_desktop_number_change, 0, (Window) number_of_desktops);
    }
    for (i=0;i<8;i++){
      if (a == da[i]){
	sendToModules(module_desktop_name_change, 0, (Window) i+1);
      }
    }
    return;
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
    if(c->trans != None){
      int i;
      if (c->buttons[0] && c->buttons[0] != c->buttonMenu)
	c->buttons[0]->hide();
      for (i=1; i<6; i++){
	if (c->buttons[i])
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
  else if (a == kwm_win_maximized){
    if (c->isMaximized() != KWM::isMaximized(c->window)){
      if (c->isMaximized())
	c->unMaximize();
      else
	c->maximize();
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
	gravitate(c, FALSE);
	sendConfig(c);
      }
      else {
	gravitate(c, TRUE);
	c->decoration = dec;
	sendConfig(c);
      }
    }

    bool wants_focus = (d & KWM::noFocus) == 0;
    if (!wants_focus && c->wantsFocus()){
      clients_traversing.removeRef(c);
      sendToModules(module_win_remove, c);
      c->wants_focus = false;
      c->hidden_for_modules = true;
    } else if (wants_focus && !c->wantsFocus()){
      if (c->hidden_for_modules){
	clients_traversing.insert(0,c);
	c->hidden_for_modules = false;
	sendToModules(module_win_add, c);
      }
      c->wants_focus = true;
    }
  }
}

void Manager::shapeNotify(XShapeEvent *e){
  Client *c;
    c = getClient(e->window);
    if (c)
      setShape(c);
}


void Manager::motionNotify(XMotionEvent* e){
  Client *c;
  enable_focus_follow_mouse_activation = true;
  if (options.FocusPolicy == FOCUS_FOLLOW_MOUSE){ 
    c = getClient(e->window);
    if (c && c == delayed_focus_follow_mouse_client && c != current()
	&& c->state != WithdrawnState){
      activateClient(c);
      raiseSoundEvent("Window Activate");
    }
    delayed_focus_follow_mouse_client = NULL;
  }
}

void Manager::enterNotify(XCrossingEvent *e){
  Client *c;
  if (options.FocusPolicy == FOCUS_FOLLOW_MOUSE){ 
    delayed_focus_follow_mouse_client = NULL;
    c = getClient(e->window);
    if (c != 0 && c != current() && c->state != WithdrawnState){
      XSync(qt_xdisplay(), False);
      timeStamp(); 
      XSync(qt_xdisplay(), False);
      if (enable_focus_follow_mouse_activation){
	activateClient(c);
	raiseSoundEvent("Window Activate");
      }
      else {
	if (e->x_root != QCursor::pos().x()
	    || e->y_root != QCursor::pos().y()){
	  activateClient(c);
	  raiseSoundEvent("Window Activate");
	}
	else{
	  usleep(100);
	  if (e->x_root != QCursor::pos().x()
	      || e->y_root != QCursor::pos().y()){
	    activateClient(c);
	    raiseSoundEvent("Window Activate");
	  }
	  else
	    delayed_focus_follow_mouse_client = c;
	}
      }
    }
  }

  // electric borders
  if (e->window == top_border ||
      e->window == left_border ||
      e->window == bottom_border ||
      e->window == right_border){
    if (options.ElectricBorder > -1){
      QTimer::singleShot(options.ElectricBorder, 
			 this, SLOT(electricBorder()));
      current_border = e->window;
    }
  }
}
void Manager::leaveNotify(XCrossingEvent *e){
  // electric borders
  if (e->window == top_border ||
      e->window == left_border ||
      e->window == bottom_border ||
      e->window == right_border){
    current_border = None;
  }
}

void Manager::moveDesktopInDirection(DesktopDirection d, Client* c){

  int nd;

  switch (d){

  case Up:
    if (current_desktop % 2 == 1)
      nd = current_desktop + 1;
    else
      nd = current_desktop - 1;
    if (c)
      c->desktop = nd;
    switchDesktop(nd);

    if(!options.ElectricBorderMovePointer){
      QCursor::setPos(QCursor::pos().x(), QApplication::desktop()->height()-3);
    }
    else{
      QCursor::setPos(QCursor::pos().x(), QApplication::desktop()->height()/2);
    }

    break;

  case Down:

    if (current_desktop % 2 == 1)
      nd = current_desktop + 1;
    else
      nd = current_desktop - 1;
    if (c)
      c->desktop = nd;
    switchDesktop(nd);

    if(!options.ElectricBorderMovePointer){
      QCursor::setPos(QCursor::pos().x(),3);
    }
    else{
      QCursor::setPos(QCursor::pos().x(), QApplication::desktop()->height()/2);
    }

    break;

  case Left:

    nd = current_desktop - 2;
    if (nd < 1)
	nd += number_of_desktops;
    if (c)
      c->desktop = nd;
    switchDesktop(nd);

    if(!options.ElectricBorderMovePointer){
      QCursor::setPos(QApplication::desktop()->width()-3, QCursor::pos().y());
    }
    else{
      QCursor::setPos(QApplication::desktop()->width()/2, QCursor::pos().y());
    }

    break;

  case Right:

    nd = current_desktop - 2;
    if (nd < 1)
      nd += number_of_desktops;
    if (c)
      c->desktop = nd;
    switchDesktop(nd);

    if(!options.ElectricBorderMovePointer){
      QCursor::setPos(3,QCursor::pos().y());
    }
    else{
      QCursor::setPos(QApplication::desktop()->width()/2, QCursor::pos().y());
    }

    break;
  }
}

void Manager::electricBorder(){
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
}

void Manager::raiseElectricBorders(){
  // electric borders
  XRaiseWindow(qt_xdisplay(), top_border);
  XRaiseWindow(qt_xdisplay(), left_border);
  XRaiseWindow(qt_xdisplay(), bottom_border);
  XRaiseWindow(qt_xdisplay(), right_border);
}


void Manager::randomPlacement(Client* c){
  static int px = TITLEBAR_HEIGHT + BORDER;
  static int py = 2 * TITLEBAR_HEIGHT + BORDER;
  int tx,ty;

  QRect maxRect = KWM::getWindowRegion(currentDesktop());

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
void Manager::cascadePlacement (Client* c) {

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

  // get the maximum allowed windows space
  QRect maxRect = KWM::getWindowRegion(currentDesktop()); 

  // initialize often used vars: width and height of c; we gain speed
  int ch = c->geometry.height();
  int cw = c->geometry.width();
  int H = maxRect.height();
  int W = maxRect.width();

  xp = x[d];
  yp = y[d];

  //here to touch in case people vote for resize on placement
  if ((yp + ch ) > H) yp = 0;

  if ((xp + cw ) > W) 
    if (!yp) {
      smartPlacement(c);
      return;
    }
    else xp = 0;
  
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
  int ch = c->geometry.height();
  int cw = c->geometry.width();

  // get the maximum allowed windows space
  QRect maxRect = KWM::getWindowRegion(currentDesktop()); 
  
  // initialize with null the current overlap
  x = maxRect.x(); y = maxRect.y();

  //initialize and do a loop over possible positions
  overlap = -1;
  spGetOverlap(c, x, y, &overlap); 
  over_min = overlap;
  xopt = x;
  yopt = y;
  
  while((overlap != 0) && (overlap != -1)) {
    // test if windows overlap ...
    if (overlap > 0) {

      other = maxRect.width();
      temp = other - cw;
      
      if(temp > x) other = temp;
      
      // compare to the position of each client on the current desk
      for(l = clients.first(); l ; l = clients.next()) {
	if(!l->isOnDesktop(currentDesktop()) || (l == c)) 
	  continue;
	// if not enough room above or under the current tested client
	// determine the first non-overlapped x position
	if((y < l->geometry.height() + l->geometry.y() ) && 
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
      x = maxRect.x();
      
      other = maxRect.height();
      temp = other - ch;
      
      if(temp > y) other = temp;
      
      //test the position of each window on current desk
      //07mar98. fixed bug which made iconified windows avoided as if visible
      for(l = clients.first(); l ; l = clients.next()) {
	if(!l->isOnDesktop(currentDesktop()) || (l == c) || c->isIconified()) 
	  continue;
	
	temp = l->geometry.height() + l->geometry.y();
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
  int ch = c->geometry.height();
  int cw = c->geometry.width();

  // get the maximum allowed windows space
  QRect maxRect = KWM::getWindowRegion(currentDesktop());

  //test if enough room in y direction
  if (y + ch > maxRect.height()) {
    *overlap = -1;
    return ;
  }
  
  //test if enough room in x direction
  if(x + cw > maxRect.width()) {
    *overlap = -2;
    return;
  }
  
  over_temp = 0;

  cxl = x;
  cxr = x + cw;
  cyt = y;
  cyb = y + ch;
  for(l = clients.first(); l ; l = clients.next()) {
    if(!l->isOnDesktop(currentDesktop()) || (l == c)) 
      continue;
    xl = l->geometry.x();
    yt = l->geometry.y();
    xr = xl + l->geometry.width();
    yb = yt + l->geometry.height();

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


void Manager::manage(Window w, bool mapped){

  bool dohide;
  bool pseudo_session_management = FALSE;
  int state;
  XClassHint klass;
  XWMHints *hints;
  XWindowAttributes attr;
  
  if (KWM::isDockWindow(w)){
    addDockWindow(w);
    return;
  }

  // test for manage prohibitation
  if (!mapped) {
    char* s;
    for (s = do_not_manage_titles.first(); s ; 
	 s = do_not_manage_titles.next()){
      if (getprop(w, XA_WM_NAME) == s){
	do_not_manage_titles.remove();
	sendToModules(module_win_add, 0, w);
	sendToModules(module_win_remove, 0, w);
	return;
      } 
    }
  }
  
  // create a client
  if (!initting)
    XGrabServer(qt_xdisplay()); // we want to be alone...

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
    c->command = xgetprop(c->window, XA_WM_COMMAND);
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
      c->decoration = 0;
    }
  }

  // get KDE specific decoration hint
  if (c->getDecoration() == KWM::normalDecoration){
    long dec = KWM::getDecoration(c->window);
    c->decoration = dec & 255;
    c->wants_focus = (dec & KWM::noFocus) == 0;
    if (!c->wantsFocus())
      c->hidden_for_modules = true;
  }

  XSelectInput(qt_xdisplay(), c->window, ColormapChangeMask | 
	       EnterWindowMask | PropertyChangeMask | PointerMotionMask);
  
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

  if(c->trans != None || c->getDecoration()!=KWM::normalDecoration){
    int i;
    if (c->buttons[0] && 
	(c->buttons[0] != c->buttonMenu || 
	 c->getDecoration()!=KWM::normalDecoration))
      c->buttons[0]->hide();
    for (i=1; i<6; i++){
      if (c->buttons[i])
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
  
  if (mapped || c->trans != None 
      // ||c->size.flags & PPosition
      ||c->size.flags & USPosition 
      || pseudo_session_management
      ){
      // nothing
  }
  else {
    if((options.Placement == SMART_PLACEMENT)  ||
       (options.Placement == MANUAL_PLACEMENT) ||
       (options.interactive_trigger >= 0))
      smartPlacement(c);
    else if(options.Placement == CASCADE_PLACEMENT)
      cascadePlacement(c);
    else
      randomPlacement(c);
  }

  if (mapped)
    c->reparenting = TRUE;
  XSetWindowBorderWidth(qt_xdisplay(), c->window, 0);
  switch (c->getDecoration()){
  case KWM::noDecoration:
    XReparentWindow(qt_xdisplay(), c->window, c->winId(), 0, 0);
    break;
  case KWM::tinyDecoration:
    XReparentWindow(qt_xdisplay(), c->window, c->winId(), (BORDER_THIN), (BORDER_THIN));
    break;
  default:
    XReparentWindow(qt_xdisplay(), c->window, c->winId(), (BORDER), (BORDER) + 
		    TITLEBAR_HEIGHT);
  break;
  }
  
  if (shape) {
    XShapeSelectInput(qt_xdisplay(), c->window, ShapeNotifyMask);
    ignore_badwindow = TRUE;       /* magic */
    setShape(c);
    ignore_badwindow = FALSE;
  }
  
  XAddToSaveSet(qt_xdisplay(), c->window);
  XSync(dpy, False);
  sendConfig(c, FALSE);
  XSync(qt_xdisplay(), False);

  // get some KDE specific hints

  // transient windows on their parent's desktop
  Client* pc = NULL;
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

  addClient(c);
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
 
  if (c->trans)
    raiseSoundEvent("Window Trans New");
  else
    raiseSoundEvent("Window New");


  if (!dohide && c->getDecoration() != KWM::noDecoration) {
    activateClient(c);
  }
  else
    c->setactive(FALSE);

  if (c->isIconified())
    iconifyTransientOf(c);

  if (!initting)
    XUngrabServer(qt_xdisplay()); 

  if(options.Placement == MANUAL_PLACEMENT)
    c->handleOperation(Client::operationFromCommand("winMove"));
  
}

void Manager::withdraw(Client* c){
  KWM::moveToDesktop(c->window, 0);
  c->hideClient();
  gravitate(c, TRUE);
  XReparentWindow(qt_xdisplay(), c->window, qt_xrootwin(), 
		  c->geometry.x() , c->geometry.y());
  XRemoveFromSaveSet(qt_xdisplay(), c->window);
  setWindowState(c, WithdrawnState);
  XSelectInput(qt_xdisplay(), c->window, NoEventMask);
  XUngrabButton(qt_xdisplay(), AnyButton, AnyModifier, c->window);
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
  if (!c->hidden_for_modules)
    clients_traversing.insert(0,c);
}


void Manager::activateClient(Client* c, bool set_revert){
  if (!c->wantsFocus())
    return;
  Client* cc = current();
  enable_focus_follow_mouse_activation = false;
  if (focus_grabbed())
    return;
  if (c == cc)
    return;
  if (cc){
    cc->setactive( FALSE );
    if (cc->mainClient() != c->mainClient())
      iconifyFloatingOf(cc->mainClient());
  }

  c->setactive( TRUE );
  unIconifyTransientOf(c->mainClient());

  focusToClient(c);

  colormapFocus(c);
  if (clients_traversing.removeRef(c))
    clients_traversing.append(c);
  if (!set_revert && cc){
    if (clients_traversing.removeRef(cc))
      clients_traversing.insert(0,cc);
  }
  XChangeProperty(qt_xdisplay(), qt_xrootwin(), kwm_active_window, 
		  kwm_active_window, 32,
		  PropModeReplace, (unsigned char *)&(c->window), 1);
  sendToModules(module_win_activate, c);
}

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
    sendToModules(module_win_raise, c);
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
  
  raiseElectricBorders();
}

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
  Window* new_stack = new Window[nwins+1];
  Window* hide_stack = new Window[nwins+1];
  unsigned int n = 0;
  unsigned int nh = 0;
  new_stack[n++] = c->winId();

  for (i = 0; i < nwins; i++) {
    if (!getClient(wins[i])){
      if (KWM::getDecoration(wins[i]) == 1024){
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
    XKillClient(qt_xdisplay(), c->window);
    removeClient(c);
  }
}

void Manager::changedClient(Client* c){
  sendToModules(module_win_change, c);
}

void Manager::noFocus(){
  Client* c;
  for (c = clients_traversing.last();
       c && (c->isActive() || c->state != NormalState); 
       c = clients_traversing.prev());
  if (c && c->state == NormalState) {
    activateClient(c, false);
    return;
  }

  c = current();
  if (c){
    c->setactive(False);
    iconifyFloatingOf(c->mainClient());
  }
  focusToNull();
  sendToModules(module_win_activate, 0);
  
}

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


void Manager::focusToClient(Client* c){
  if (c->isShaded())
    focusToNull();
  else {
    XSetInputFocus(qt_xdisplay(), c->window, RevertToPointerRoot, timeStamp());
    if (c->Ptakefocus)
      sendClientMessage(c->window, wm_protocols, wm_take_focus);
  }
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
      c->hideClient();
      setWindowState(c, IconicState); 
    }
  }
  current_desktop = new_desktop;

  KWM::switchToDesktop(current_desktop);
  sendToModules(module_desktop_change, 0, (Window) current_desktop);

  for (c=clients_sorted.last(); c ; c=clients_sorted.prev()){
    if (c->isOnDesktop(current_desktop) && !c->isIconified() && !c->isSticky()){
      c->showClient();
      setWindowState(c, NormalState);
    }
    if (c->isSticky() && ! c->isIconified() && c != current()){
      if (clients_traversing.removeRef(c))
       clients_traversing.insert(0,c);
    }
  }

  if (!current())
    noFocus();
}








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
  XDeleteProperty(qt_xdisplay(), qt_xrootwin(), kwm_running);
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
  	  if (c->getDecoration() == KWM::normalDecoration){
  	    if (!mwm_hints->decorations)
	      c->decoration = 0;
	    KWM::setDecoration(c->window, c->getDecoration());
	  }
	}
      XFree((char *)mwm_hints);
    }
  
}


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

bool Manager::getSimpleProperty(Window w, Atom a, long &result){
  long *p = 0;
  
  if (_getprop(w, a, a, 1L, (unsigned char**)&p) <= 0){
    return FALSE;
  }
  
  result = p[0];
  XFree((char *) p);
  return TRUE;
}

void Manager::setQRectProperty(Window w, Atom a, const QRect &rect){
  long data[4];
  data[0] = rect.x();
  data[1] = rect.y();
  data[2] = rect.width();
  data[3] = rect.height();
  XChangeProperty(qt_xdisplay(), w, a, a, 32,
		  PropModeReplace, (unsigned char *)&data, 4);
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

void Manager::darkenScreen(){
  if (0 != greyer_widget)
    delete greyer_widget;
  greyer_widget = new KGreyerWidget();
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
      XSync(qt_xdisplay(), FALSE);
      sendClientMessage(c->window, wm_protocols, wm_save_yourself);
      // wait for clients response
      do {
	XWindowEvent(qt_xdisplay(), c->window, PropertyChangeMask
		     | StructureNotifyMask, &ev);
	XSync(qt_xdisplay(), FALSE);
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
  // do this afterwards because some clients may have set XA_WM_COMMAND for
  // other windows, too
  for (c = clients.first(); c; c = clients.next()){
    if (!c->Psaveyourself){
      // poor man's "session" management a la xterm :-(
      c->command = xgetprop(c->window, XA_WM_COMMAND);
      if (c->command.isNull()){
	long result = None;
	getSimpleProperty(c->window, wm_client_leader, result);
	Window clw = (Window) result;
	if (clw != None && clw != c->window && getClient(clw))
	    c->command = ""; // will be stored anyway
	  else
	    c->command = xgetprop(clw, XA_WM_COMMAND);
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

QStrList* Manager::getUnsavedDataClients(){
  QStrList *result = new QStrList();
  Client* c;
  for (c = clients.first(); c; c = clients.next()){
    if (KWM::containsUnsavedData(c->window))
      result->append(c->label);
  }
  return result;
}

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


void Manager::setProxyData(QStrList* proxy_hints_arg, 
			   QStrList* proxy_props_arg,
			   QStrList* proxy_ignore_arg){
  proxy_hints = proxy_hints_arg;
  proxy_props = proxy_props_arg;
  proxy_ignore = proxy_ignore_arg;
}


void Manager::killWindowAtPosition(int x, int y){
  Client* c;
  for (c = clients_sorted.last(); c; c = clients_sorted.prev()){
    if (c->geometry.contains(QPoint(x, y))){
      XKillClient(qt_xdisplay(), c->window);
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


void Manager::iconifyTransientOf(Client* c){
  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
    if (it.current() != c && it.current()->trans == c->window){
      it.current()->iconify(False);
      sendToModules(module_win_remove, it.current());
      it.current()->hidden_for_modules = true; 
      clients_traversing.removeRef(it.current());
    }
  }
}


void Manager::unIconifyTransientOf(Client* c){
  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
    if (it.current() != c && it.current()->trans == c->window){
      if (it.current()->hidden_for_modules){
	it.current()->hidden_for_modules = false;
	sendToModules(module_win_add, it.current());
	clients_traversing.insert(0,it.current());
      }
      it.current()->unIconify(False);
    }
  }
}

void Manager::ontoDesktopTransientOf(Client* c){
  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
    if (it.current() != c && it.current()->trans == c->window){
      it.current()->ontoDesktop(c->desktop);
    }
  }
}


void Manager::stickyTransientOf(Client* c, bool sticky){
  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
    if (it.current() != c && it.current()->trans == c->window){
      if (it.current()->isSticky() != sticky)
	it.current()->buttonSticky->toggle();
    }
  }
}

void Manager::iconifyFloatingOf(Client* c){
  // DON'T automatically iconify floating menues if the focus policy is
  // focus follows mouse, since otherwise it will be exceptionally hard
  // to get control over them back! (Marcin Dalecki)
  if (options.FocusPolicy == FOCUS_FOLLOW_MOUSE)
    return;
    
  QListIterator<Client> it(clients);
  for (it.toFirst(); it.current(); ++it){
    if (it.current() != c && it.current()->trans == c->window){
      if (it.current()->getDecoration() == KWM::tinyDecoration){
	it.current()->iconify(False);
	sendToModules(module_win_remove, it.current());
	it.current()->hidden_for_modules = true; 
	clients_traversing.removeRef(it.current());
      }
      iconifyFloatingOf(it.current());
    }
  }
}


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
    if (dock_module != None) //we have already a dock module
      KWM::setKWMModule(w);
    else{
      dock_module = w;
      Window *dw;
      for (dw = dock_windows.first(); dw; dw = dock_windows.next())
	sendClientMessage(dock_module, module_dockwin_add, (long) *dw);
    }
  }

  Client* c;
  for (c=clients.first(); c; c=clients.next()){
    if (!c->hidden_for_modules)
      sendClientMessage(w, module_win_add, c->window);
  }
  for (c=clients_sorted.first(); c; c=clients_sorted.next()){
    if (!c->hidden_for_modules)
      sendClientMessage(w, module_win_raise, (long) c->window);
  }
  if (current())
    sendClientMessage(w, module_win_activate, (long) current()->window);

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
  raiseElectricBorders();
}
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

//// CC: Implementation of the KDE Greyer Widget

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

