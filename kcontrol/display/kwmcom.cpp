// kwmcom.C
//
// Running proposal for a communication
// between kwm and modules like kpanel
//
// Part of the KDE project.
//
// Copyright (C) 1997 Matthias Ettrich
//

#include <kapp.h>
#include "kwmcom.h"

Atom kwm_add_window;
Atom kwm_remove_window;
Atom kwm_un_minimize_window;
Atom kwm_register_module;
Atom kwm_unregister_module;
Atom kwm_command;
Atom kwm_desktop;
Atom kwm_active_window;
Atom kwm_define_maximize; 
Atom kwm_minimize_window;
Atom kwm_toggle_maximize;
Atom kwm_changed_window; 
Atom kwm_release_window;
Atom kwm_change_color;


Display *kwmcom_display;
Window kwmcom_rootwindow;
Window kwmcom_window;

void kwmcom_init(Display* dpy, Window win){
  kwmcom_display = dpy;
  kwmcom_rootwindow = XDefaultRootWindow(kwmcom_display);
  kwmcom_window = win;

  kwm_add_window = XInternAtom(kwmcom_display, "KWM_ADD_WINDOW", False);
  kwm_remove_window = XInternAtom(kwmcom_display, "KWM_REMOVE_WINDOW", False);
  kwm_un_minimize_window = XInternAtom(kwmcom_display, "KWM_UN_MINIMIZE_WINDOW", False);
  kwm_register_module = XInternAtom(kwmcom_display, "KWM_REGISTER_MODULE", False);
  kwm_unregister_module = XInternAtom(kwmcom_display, "KWM_UNREGISTER_MODULE", False);
  kwm_command = XInternAtom(kwmcom_display,  "KWM_COMMAND", False);
  kwm_desktop = XInternAtom(kwmcom_display, "KWM_DESKTOP", False);
  kwm_active_window = XInternAtom(kwmcom_display, "KWM_ACTIVE_WINDOW", False);
  kwm_define_maximize = XInternAtom(kwmcom_display, "KWM_DEFINE_MAXIMIZE", False);     
  kwm_minimize_window = XInternAtom(kwmcom_display, "KWM_MINIMIZE_WINDOW", False);
  kwm_toggle_maximize = XInternAtom(kwmcom_display, "KWM_TOGGLE_MAXIMIZE", False);
  kwm_changed_window = XInternAtom(kwmcom_display, "KWM_CHANGED_WINDOW", False);     
  kwm_release_window = XInternAtom(kwmcom_display, "KWM_RELEASE_WINDOW", False);     
  kwm_change_color = XInternAtom(kwmcom_display, "KWM_CHANGE_COLOR", False); 
}

void kwmcom_send_to_kwm(Atom a, long a1, long a2, 
			long a3, long a4, long a5){
  XEvent ev;
  int status;
  long mask;
  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = kwmcom_window;
  ev.xclient.message_type = a;
  ev.xclient.format = 32;
  
  ev.xclient.data.l[0]=a1;
  ev.xclient.data.l[1]=a2;
  ev.xclient.data.l[2]=a3;
  ev.xclient.data.l[3]=a4;
  ev.xclient.data.l[4]=a5;
  
  mask = SubstructureRedirectMask;        /* magic! */

  status = XSendEvent(kwmcom_display, 
		      kwmcom_rootwindow, 
		      False, mask, &ev);
  if (status == 0){
    fprintf(stderr, klocale->translate("kwmcom: communication with kwm failed\n"));
  }
  XSync(kwmcom_display, False);
}


void kwmcom_send_to_kwm(Atom a, char * s){
  XEvent ev;
  int status;
  long mask;
  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = kwmcom_window;
  ev.xclient.message_type = a;
//  ev.xclient.format = 32;
  ev.xclient.format = 8;   // patch from Christian Czezatke

  int i;
  for (i=0;i<19 && s[i];i++)
    ev.xclient.data.b[i]=s[i];
  
  mask = SubstructureRedirectMask;        /* magic! */

  status = XSendEvent(kwmcom_display, 
		      kwmcom_rootwindow, 
		      False, mask, &ev);
  if (status == 0){
    fprintf(stderr, klocale->translate("kwmcom: communication with kwm failed\n"));
  }
  XSync(kwmcom_display, False);
}



