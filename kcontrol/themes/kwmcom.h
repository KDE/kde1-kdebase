// kwmcom.h
//
// Running proposal for a communication
// between kwm and modules like kpanel
//
// Part of the KDE project.
//
// Copyright (C) 1997 Matthias Ettrich
//

#ifndef KWMCOM_H
#define KWMCOM_H

#include <X11/Xlib.h>
#include <string.h>
#include <stdio.h>

void kwmcom_init(Display* dpy, Window win);
void kwmcom_send_to_kwm(Atom a, long a1=0, long a2=0, 
			long a3=0, long a4=0,long a5=0);
void kwmcom_send_to_kwm(Atom a, char * s); // s limited to 20 chars maximum!

extern Atom kwm_add_window;
extern Atom kwm_remove_window;
extern Atom kwm_un_minimize_window;
extern Atom kwm_register_module;
extern Atom kwm_unregister_module;
extern Atom kwm_command;
extern Atom kwm_desktop;
extern Atom kwm_active_window;
extern Atom kwm_define_maximize;   
extern Atom kwm_minimize_window;
extern Atom kwm_toggle_maximize;
extern Atom kwm_changed_window;
extern Atom kwm_release_window;
extern Atom kwm_change_color;

#endif // KWMCOM_H
