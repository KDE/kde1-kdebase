/*
 * drag.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <stdio.h>

#include "main.h"

#include <X11/X.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <qcursor.h>
#include <sys/time.h>  

//CT 17mar98
#include <stdlib.h>

#include "manager.h"
#include <kwm.h>

#define ButtonMask  (ButtonPressMask|ButtonReleaseMask)


extern MyApp* myapp;
extern Manager* manager;

extern Window root;
extern GC rootgc;
extern GC rootfillgc;

// local prototypes

void resizecalc(Client *c, int x, int y);
void resizecalc_bl(Client *c, int x, int y); 
void resizecalc_tl(Client *c, int x, int y); 
void resizecalc_tr(Client *c, int x, int y); 
void resizecalc_l(Client *c, int x, int y); 
void resizecalc_r(Client *c, int x, int y); 
void resizecalc_t(Client *c, int x, int y); 
void resizecalc_b(Client *c, int x, int y); 


void resizecalc(Client *c, int x, int y){
  x+=1;
  y+=1;
  c->geometry.setWidth(x - c->geometry.x());
  c->geometry.setHeight(y - c->geometry.y());
  c->adjustSize();
}


void resizecalc_bl(Client *c, int x, int y){
  int dx = c->geometry.width();
  y+=1;
  c->geometry.setWidth(c->geometry.x() + c->geometry.width() - x);
  c->geometry.setHeight(y - c->geometry.y());
  c->adjustSize();
  c->geometry.moveBy(dx - c->geometry.width(),0);
  
}  


void resizecalc_tl(Client *c, int x, int y){
  int dx = c->geometry.width();
  int dy = c->geometry.height();
  c->geometry.setWidth( c->geometry.x() + c->geometry.width() - x);
  c->geometry.setHeight(c->geometry.y() + c->geometry.height() - y);
  c->adjustSize();
  c->geometry.moveBy(dx - c->geometry.width(),
		      dy - c->geometry.height());
  
}  

void resizecalc_tr(Client *c, int x, int y){
  int dy = c->geometry.height();
  x+=1;
  c->geometry.setWidth( x - c->geometry.x());
  c->geometry.setHeight(c->geometry.y() + c->geometry.height() - y);
  c->adjustSize();
  c->geometry.moveBy(0, dy - c->geometry.height());
}  


void resizecalc_l(Client *c, int x, int /* y */){
  int dx = c->geometry.width();
  c->geometry.setWidth( c->geometry.x() + c->geometry.width() - x);
  c->adjustSize();
  c->geometry.moveBy(dx - c->geometry.width(), 0);
}  

void resizecalc_r(Client *c, int x, int /* y */){
    x+=1;
    c->geometry.setWidth( x - c->geometry.x());
    c->adjustSize();
} 

void resizecalc_t(Client *c, int /* x */, int y){
  int dy = c->geometry.height();
  c->geometry.setWidth( c->geometry.width());
  c->geometry.setHeight(c->geometry.y() + c->geometry.height() - y);
  c->adjustSize();
  c->geometry.moveBy(0, dy - c->geometry.height());
} 

void resizecalc_b(Client *c, int /* x */, int y){
    y+=1;
    c->geometry.setHeight(y - c->geometry.y());
    c->adjustSize();
} 


void dragcalc(Client* c, int x, int y) {
  c->geometry.moveTopLeft(QPoint(QPoint(x,y)- c->old_cursor_pos));
  if (options.WindowSnapZone > 0) manager->snapToWindow(c);
  if (options.BorderSnapZone > 0) manager->snapToBorder(c);
}

void draw_selection_rectangle(int x, int y, int dx, int dy){
   XDrawRectangle(qt_xdisplay(), root, rootgc, x, y, dx, dy);
   if (dx>2) dx-=2;
   if (dy>2) dy-=2;
   XDrawRectangle(qt_xdisplay(), root, rootgc, x+1, y+1, dx, dy);
}


void draw_animation_rectangle(int x, int y, int dx, int dy, bool decorated, int o1, int o2){
   XRectangle rects[3];
  if (dx <= 7)
    dx = 7;
  if (dy <= 7)
    dy = 7;
  
   rects[0].x = x;
   rects[0].y = y;
   rects[0].width = dx;
   rects[0].height = dy;
   rects[1].x = x+1;
   rects[1].y = y+1;
   rects[1].width = dx-2;
   rects[1].height = dy-2;
   rects[2].x = x+2;
   rects[2].y = y+2;
   rects[2].width = dx-4;
   rects[2].height = dy-4;
   XDrawRectangles(qt_xdisplay(),root,rootgc,rects,3);
   if (decorated){
     if (dy > TITLEBAR_HEIGHT + 3 + BORDER){
       XDrawLine(qt_xdisplay(), root, rootgc, x+3, y+BORDER+TITLEBAR_HEIGHT, 
		 x+dx-3, y+BORDER+TITLEBAR_HEIGHT);
       XDrawLine(qt_xdisplay(), root, rootgc, x+3, y+BORDER+TITLEBAR_HEIGHT+1, 
		 x+dx-3, y+BORDER+TITLEBAR_HEIGHT+1);
       if (dx > o1 + o2){
	 XFillRectangle(qt_xdisplay(), root, rootfillgc,
			x+o1, y+BORDER+1,
			dx-o1-o2, 
			TITLEBAR_HEIGHT - TITLEWINDOW_SEPARATION -2);
       }
     }
   }
}



// draw a transparent representation of the specified client
void drawbound(Client* c){
    int x, y, dx, dy;


    x = c->geometry.x();
    y = c->geometry.y();
    dx = c->geometry.width();
    dy = c->geometry.height();
    if (dx < 0) {
        x += dx;
        dx = -dx;
    }
    if (dy < 0) {
        y += dy;
        dy = -dy;
    }
    draw_animation_rectangle(x,y,dx,dy, 
			     c->getDecoration()==KWM::normalDecoration,
			     c->title_rect.x(), 
			     c->width()-c->title_rect.right());
    
}


// an electric border has fired! => switch to the appropriate virtual desktop
bool electricBorder(Client* c, bool grab_server, int &x, int &y){
  if (options.ElectricBorder < 0)
    return false;
  struct timeval value;
  XEvent ev;
  int n = options.ElectricBorder * 1000;
  if (n > 0){
    value.tv_usec = n % 1000000;
    value.tv_sec = n / 1000000;
    (void) select(1, 0, 0, 0, &value);       
  }
  if (XCheckMaskEvent(qt_xdisplay(), PointerMotionMask, &ev)){
    while (XCheckMaskEvent(qt_xdisplay(), PointerMotionMask, &ev));
    x = ev.xmotion.x_root;
    y = ev.xmotion.y_root;
  }
  if (x < 1 || y < 1 ||
      x >= QApplication::desktop()->width()-1 ||
      y >= QApplication::desktop()->height()-1){
    Manager::DesktopDirection d;
    if (y< 1)
      d = Manager::Up;
    else if (y >= QApplication::desktop()->height()-1)
      d = Manager::Down;
    else if (x< 1)
      d = Manager::Left;
    else if (x >= QApplication::desktop()->width()-1)
      d = Manager::Right;
    else{
      return false;
    }

    if (grab_server){
      drawbound(c);
      XFlush(qt_xdisplay());
      XUngrabServer(qt_xdisplay());
    }
    manager->moveDesktopInDirection(d, c);
    manager->timeStamp();
    while (XCheckMaskEvent(qt_xdisplay(), EnterWindowMask, &ev));
    myapp->processEvents(); 
    if (grab_server){
      XGrabServer(qt_xdisplay());
    }
    QPoint pos = QCursor::pos();
    x = pos.x();
    y = pos.y();
    if (XCheckMaskEvent(qt_xdisplay(), PointerMotionMask, &ev)){
      while (XCheckMaskEvent(qt_xdisplay(), PointerMotionMask, &ev));
      x = ev.xmotion.x_root;
      y = ev.xmotion.y_root;
    }
    return true;
  }
  return false;
}



// general function do deal with transparent and opaque interactive
// movement or resizing. The different behaviour lies in the specified
// recalc function which is called to recalculate the size and
// position of the client.
bool sweepdrag(Client* c,void (*recalc)( Client *, int, int) ){
	      
    XEvent ev;
    int cx, cy, rx, ry;
    QRect other;

    bool do_not_clear_rectangle = false;
    
    bool transparent = false;
    if (recalc == dragcalc)
      transparent = (options.WindowMoveType == TRANSPARENT);
    else
      transparent = (options.WindowResizeType == TRANSPARENT);

    cx = rx = c->geometry.x() + c->old_cursor_pos.x();
    cy = ry = c->geometry.y() + c->old_cursor_pos.y();
    bool return_pressed = false;

    if (recalc != dragcalc)
      manager->raiseSoundEvent("Window Resize Start");
    else
      manager->raiseSoundEvent("Window Move Start");
    XFlush(qt_xdisplay());
    manager->timeStamp();

    // set the focus policy to ClickToFocus to avoid flickering
    FOCUS_POLICY oldFocusPolicy = options.FocusPolicy;
    options.FocusPolicy = CLICK_TO_FOCUS;
    
    if (transparent){
      XGrabServer(qt_xdisplay());
      drawbound(c);
    }
    
    while (c->dragging_is_running() && !return_pressed){
      
      XMaskEvent(qt_xdisplay(), ButtonMask|KeyPressMask|PointerMotionMask, &ev);
      return_pressed = ev.type == ButtonRelease;
      if (ev.type == KeyPress){
	int kc = XKeycodeToKeysym(qt_xdisplay(), ev.xkey.keycode, 0);
	int mx = 0;
	int my = 0;
	return_pressed = (kc == XK_Return) || (kc == XK_space)
	  || (kc == XK_Escape);
	if (kc == XK_Left) mx = -10;
	if (kc == XK_Right) mx = 10;
	if (kc == XK_Up) my = -10;
	if (kc == XK_Down) my = 10;
	if (ev.xkey.state & ControlMask){
	  mx /= 10;
	  my /= 10;
	}
	QCursor::setPos(QCursor::pos()+QPoint(mx, my));
	continue;
      }
      else if (ev.type == MotionNotify){
	rx = ev.xmotion.x_root;
	ry = ev.xmotion.y_root;
	// electric borders
	if (rx < 1 || ry < 1 ||
	    rx >= QApplication::desktop()->width()-1 ||
	    ry >= QApplication::desktop()->height()-1)
	  if (recalc == dragcalc)
	    do_not_clear_rectangle =
	      electricBorder(c, options.WindowMoveType == TRANSPARENT,
			     rx, ry);
      }
      if (rx == cx && ry == cy)
	continue;
      cx = rx;
      cy = ry;

      other = c->geometry;
      recalc(c, rx, ry);
      if ( other == c->geometry)
	continue;
      c->geometry = other;
      if (transparent && !do_not_clear_rectangle)
	drawbound(c);
      do_not_clear_rectangle = false;
      recalc(c, rx, ry);
      if (transparent)
	drawbound(c);
      else {
	manager->sendConfig(c);
	XSync(qt_xdisplay(), False);
	while (XCheckMaskEvent(qt_xdisplay(), EnterWindowMask, &ev));
	Window w = c->window;
	myapp->processEvents(); 
	c = manager->getClient(w);
	if (!c)
	  return true;
      }
      XFlush(qt_xdisplay());
      continue;
    }

    if (transparent)
      drawbound(c);
    
    if (c->geometry.width() < 0) {
      c->geometry.moveBy(c->geometry.width(),0);
      c->geometry.setWidth(  -c->geometry.width());
    }
    if (c->geometry.height() < 0) {
      c->geometry.moveBy(0, c->geometry.height());
      c->geometry.setHeight(-c->geometry.height());
    }

    if (options.WindowSnapZone > 0) manager->snapToWindow(c);
    if (options.BorderSnapZone > 0) manager->snapToBorder(c);

    manager->sendConfig(c);
    
    if (transparent)
      XUngrabServer(qt_xdisplay());

    while (XCheckMaskEvent(qt_xdisplay(), EnterWindowMask, &ev));

    if (c->isMaximized()){
      c->maximized = FALSE;
      c->geometry_restore = c->geometry;
      if (c->buttonMaximize->isOn())
	c->buttonMaximize->toggle();
    }

    options.FocusPolicy =  oldFocusPolicy;

    if (recalc != dragcalc)
      manager->raiseSoundEvent("Window Resize End");
    else
      manager->raiseSoundEvent("Window Move End");

    return false;
}




// interactive resizing of a client. The mode argument specifies the
// corner or edge on which the user drags.
bool resizedrag(Client *c, int mode){

    if (c->size.flags & PResizeInc) {
      if (!c->size.width_inc)
	c->size.width_inc = 1;
      if (!c->size.height_inc)
	c->size.height_inc = 1;
    }


    c->geometry_restore = c->geometry;

    switch (mode){
    case 1:
      return sweepdrag(c,resizecalc);
    case 2:
      return sweepdrag(c,resizecalc_bl);
    case 3:
      return sweepdrag(c,resizecalc_tl);
    case 4:
      return sweepdrag(c,resizecalc_tr);
    case 5:
      return sweepdrag(c,resizecalc_l);
    case 6:
      return sweepdrag(c,resizecalc_r);
    case 7:
      return sweepdrag(c,resizecalc_t);
    case 8:
      return sweepdrag(c,resizecalc_b);
    }
    return false;
}


// interactive moving of a client.
bool movedrag(Client *c){
  return sweepdrag(c,dragcalc);
}


// shows a skull and lets the user select a window that will be killed
// with manager->killWindowAtPosition() later.
void killSelect(){
	      
    XEvent ev;
    int return_pressed = 0;
    int escape_pressed = 0;
    int button_1_released = 0;

    XGrabServer(qt_xdisplay());

    while (!return_pressed &&
	   ! escape_pressed &&
	   ! button_1_released){
      XMaskEvent(qt_xdisplay(), KeyPressMask | ButtonMask | 
		 PointerMotionMask, &ev);
      if (ev.type == KeyPress){
	int kc = XKeycodeToKeysym(qt_xdisplay(), ev.xkey.keycode, 0);
	int mx = 0;
	int my = 0;
	return_pressed = (kc == XK_Return) || (kc == XK_space);
	escape_pressed = (kc == XK_Escape);
	if (kc == XK_Left) mx = -10;
	if (kc == XK_Right) mx = 10;
	if (kc == XK_Up) my = -10;
	if (kc == XK_Down) my = 10;
	if (ev.xkey.state & ControlMask){
	  mx /= 10;
	  my /= 10;
	}
	QCursor::setPos(QCursor::pos()+QPoint(mx, my));
      }
      if (ev.type == ButtonRelease){
	button_1_released = (ev.xbutton.button == Button1);
	manager->killWindowAtPosition(ev.xbutton.x_root, ev.xbutton.y_root);
      }
      continue;
    }
    if (return_pressed){
      manager->killWindowAtPosition(QCursor::pos().x(), QCursor::pos().y());
    }

    XUngrabServer(qt_xdisplay());

}



