 /*
 * Client.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdlib.h>
#include <qapp.h>
#include <qpushbt.h>
#include <qcolor.h>
#include <qbitmap.h>
#include <qpalette.h>
#include <qpainter.h>
#include <qdrawutl.h>
#include <qcursor.h>
#include <qdatetm.h>

#include <qfiledlg.h>
#include <qpmcache.h>
#include <qimage.h>


#include "client.moc"
#include "manager.h"
#include "main.h"
#include <kiconloader.h>
#include <kwm.h>
#include <kcharsets.h>

#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xos.h>

#include <X11/extensions/shape.h>

#include "gradientFill.h"


extern Manager* manager;


Cursor bottom_right_cursor = 0;
Cursor bottom_left_cursor = 0;
Cursor top_left_cursor = 0;
Cursor top_right_cursor = 0;
Cursor bottom_side_cursor = 0;
Cursor top_side_cursor = 0;
Cursor left_side_cursor = 0;
Cursor right_side_cursor = 0;
Cursor normal_cursor = 0;

extern MyApp* myapp;
extern QPushButton* ignore_release_on_this;
#define TITLE_ANIMATION_STEP 2
#define TITLE_ANIMATION_DELAY 30

bool do_not_draw = false;
myPushButton::myPushButton(QWidget *parent, const char* name)
  : QPushButton( parent, name ){
    setFocusPolicy(NoFocus);
    flat = True;
    last_button = 0;
}

void myPushButton::enterEvent( QEvent * ){
  flat = False;
  if (!do_not_draw)
    repaint(false);
}

void myPushButton::leaveEvent( QEvent * ){
  flat = True;
  if (!do_not_draw)
    repaint();
}

void myPushButton::paint(QPainter *painter){
  if ( isDown() || (isOn() && !flat)) {
    if ( style() == WindowsStyle )
      qDrawWinButton( painter, 0, 0, width(),
		      height(), colorGroup(), true );
    else
      qDrawShadePanel( painter, 0, 0, width(),
		       height(), colorGroup(), true, 2, 0L );
  }
  else if (!flat ) {
    if ( style() == WindowsStyle )
      qDrawWinButton( painter, 0, 0, width(), height(),
		      colorGroup(), false );
    else {
      qDrawShadePanel( painter, 0, 0, width(), height(),
		       colorGroup(), false, 2, 0L );
//       painter->setPen(black);
//       painter->drawRect(0,0,width(),height());
    }
  }

  if ( pixmap() ) {
    int dx = ( width() - pixmap()->width() ) / 2;
    int dy = ( height() - pixmap()->height() ) / 2;
    if ( isDown() && style() == WindowsStyle ) {
      dx++;
      dy++;
    }
    painter->drawPixmap( dx, dy, *pixmap() );
  }
}

void myPushButton::mousePressEvent( QMouseEvent *e){

  if ( isDown())
    return;

  bool hit = hitButton( e->pos() );
  if ( hit ){
    last_button = e->button();
    setDown( true );
    repaint( false );
    emit pressed();
  }
}

void myPushButton::mouseReleaseEvent( QMouseEvent *e){
  if ( !isDown() ){
    last_button = 0;
    return;
  }
  bool hit = hitButton( e->pos() );
  setDown( false );
  if ( hit ){
    if ( isToggleButton() )
      setOn( !isOn() );
    repaint( false );
    if ( isToggleButton() )
      emit toggled( isOn() );
    emit released();
    emit clicked();
  }
  else {
    repaint();
    emit released();
  }
  last_button = 0;
}

void myPushButton::mouseMoveEvent( QMouseEvent *e ){

  if (!last_button)
    return;

  if ( !(e->state() & LeftButton) &&
       !(e->state() & MidButton) &&
       !(e->state() & RightButton))
    return;


  bool hit = hitButton( e->pos() );
  if ( hit ) {
    if ( !isDown() ) {
      setDown(true);
      repaint(false);
      emit pressed();
    }
  } else {
    if ( isDown() ) {
      setDown(false);
      repaint();
      emit released();
    }
  }
}

// used for  the old vertical shading code

// static QPixmap *shaded_pm_active = 0;
// static QPixmap *shaded_pm_inactive = 0;
// static QColor shaded_pm_active_color;
// static QColor shaded_pm_inactive_color;


// animate a size change of a window. This can be either transparent
// or opaque. Opque only if the window is visible and the
// options.WindowResizeType is Opqaue. "before" and "after" specify
// the source resp. target recangle. If the window is decorated (that
// means it has a titlebar) o1 and o2 specify the left resp. right
// border of the titelbar rectangle.
bool  animate_size_change(Client* c, QRect before, QRect after, bool decorated, int o1, int o2){

  // the function is a bit tricky since it will ensure that an
  // animation action needs always the same time regardless of the
  // performance of the machine or the X-Server.

  float lf,rf,tf,bf,step;

  if (!options.ResizeAnimation)
    return false;

  //CT 12Jun1998 set animation steps hence speed;
  //  smaller the step is, faster the anim should be
  //  so 11 is (max possible value of the ResizeAnimation  + 1)
  //  step goes form 40 (faster animation) to 400 (Matthias' hardcoded)
  step = 40. * (11 - options.ResizeAnimation);

  bool transparent = (options.WindowResizeType == TRANSPARENT);
  if (!transparent && !c->isVisible())
    transparent = true;

  lf = (after.left() - before.left())/step; //400.0; Matthias' hardcoded
  rf = (after.right() - before.right())/step; //400.0;
  tf = (after.top() - before.top())/step; //400.0;
  bf = (after.bottom() - before.bottom())/step; //400.0;


  QRect area = before;
  QRect area2;

  QRect saved_geometry = c->geometry;

  Time ts = manager->timeStamp();
  Time tt = ts;
  float diff;

  if (transparent)
    XGrabServer(qt_xdisplay());

  do {
    if (area2 != area){
      if (transparent){
	draw_animation_rectangle(area.left(),
				 area.top(),
				 area.width(),
				 area.height(),
				 decorated, o1, o2);
      }
      else {
	c->geometry = area;
	c->adjustSize();
	manager->sendConfig(c);
	XSync(qt_xdisplay(), False);
	Window w = c->window;
	myapp->myProcessEvents();
	c = manager->getClient(w);
	if (!c)
	  return true;
      }


      area2 = area;
    }
    XFlush(qt_xdisplay());
    XSync(qt_xdisplay(), False);
    tt = manager->timeStamp();
    diff = tt - ts;
    if (diff > 500)
      diff = 500;
    area.setLeft(before.left() + int(diff*lf));
    area.setRight(before.right() + int(diff*rf));
    area.setTop(before.top() + int(diff*tf));
    area.setBottom(before.bottom() + int(diff*bf));
    if (area2 != area && transparent){
      draw_animation_rectangle(area2.left(),
			       area2.top(),
			       area2.width(),
			       area2.height(),
			       decorated, o1, o2);
    }
  } while (tt - ts < step); //CT 400);
  if (area2 == area && transparent){
    draw_animation_rectangle(area2.left(),
			     area2.top(),
			     area2.width(),
			     area2.height(),
			     decorated, o1, o2);
  }
  if (transparent)
    XUngrabServer(qt_xdisplay());
  else {
    c->geometry = after;
    c->adjustSize();
    manager->sendConfig(c);
    XSync(qt_xdisplay(), False);
    Window w = c->window;
    myapp->myProcessEvents();
    c = manager->getClient(w);
    if (!c)
      return true;
    c->geometry = saved_geometry;
  }
  return false;
}


// auxiliary function to put a passive grab over a window. grabButton
// will add all combinations of the unused modifiers NumLock and Lock
// to the specified modifiers to allow kwm to work regardless wether
// these are active or not.
static void grabButton(int button, Window window, unsigned int mod){
  static int NumLockMask = 0;
  if (!NumLockMask){
    // try to find out the modifer that handles NumLock
    XModifierKeymap* xmk = XGetModifierMapping(qt_xdisplay());
    int i;
    for (i=0; i<8; i++){
      if (xmk->modifiermap[xmk->max_keypermod * i] ==
	  XKeysymToKeycode(qt_xdisplay(), XK_Num_Lock))
	NumLockMask = (1<<i);
    }
    XFreeModifiermap(xmk);
  }

  XGrabButton(qt_xdisplay(), button,
	      mod,
	      window, True,
	      ButtonPressMask, GrabModeSync, GrabModeAsync,
	      None, normal_cursor );

  XGrabButton(qt_xdisplay(), button,
	      mod | LockMask,
	      window, True,
	      ButtonPressMask, GrabModeSync, GrabModeAsync,
	      None, normal_cursor );

  XGrabButton(qt_xdisplay(), button,
	      mod | NumLockMask,
	      window, True,
	      ButtonPressMask, GrabModeSync, GrabModeAsync,
	      None, normal_cursor );

  XGrabButton(qt_xdisplay(), button, mod | NumLockMask | LockMask,
	      window, True,
	      ButtonPressMask, GrabModeSync, GrabModeAsync,
	      None, normal_cursor );
}


Client::Client(Window w, Window _sizegrip, QWidget *parent, const char *name_for_qt )
  : QWidget( parent, name_for_qt, WResizeNoErase){
    //, WStyle_Customize | WStyle_NoBorder | WStyle_Tool ){

    DEBUG_EVENTS2("Create new Client. Client/Window", this,w)

    window = w;
    sizegrip = _sizegrip;

    recently_resized = true;
    mouse_release_command = MyApp::MouseNothing;

    backing_store = false;
    state = WithdrawnState;
    maximized = False;
    maximize_mode = 0;
    iconified = False;
    sticky = False;
    shaded = False;
    stays_on_top = False;
    cmap = None;
    border = 0;
    ncmapwins = 0;
    cmapwins = 0;
    wmcmaps = 0;
    is_active = false;
    trans = None;
    leader = None;
    decoration = KWM::normalDecoration;
    wants_focus = true;
    is_menubar = false;
    do_close = false;

    // standard window manager protocols
    Pdeletewindow = false;
    Ptakefocus = false;
    Psaveyourself = false;

    dragging_state = dragging_nope;
    do_resize = 0;

    desktop = manager->currentDesktop();

    setMouseTracking(True);

    size.flags = 0;
    generateButtons();

    // create the cursor shapes for all the edges and corners of kwm´s
    // window decoration.
    if (!bottom_right_cursor)
      bottom_right_cursor = XCreateFontCursor(qt_xdisplay(), XC_bottom_right_corner);
    if (!bottom_left_cursor)
      bottom_left_cursor = XCreateFontCursor(qt_xdisplay(), XC_bottom_left_corner);
    if (!top_right_cursor)
      top_right_cursor = XCreateFontCursor(qt_xdisplay(), XC_top_right_corner);
    if (!top_left_cursor)
      top_left_cursor = XCreateFontCursor(qt_xdisplay(), XC_top_left_corner);
    if (!top_side_cursor)
      top_side_cursor = XCreateFontCursor(qt_xdisplay(), XC_top_side);
    if (!bottom_side_cursor)
      bottom_side_cursor = XCreateFontCursor(qt_xdisplay(), XC_bottom_side);
    if (!left_side_cursor)
      left_side_cursor = XCreateFontCursor(qt_xdisplay(), XC_left_side);
    if (!right_side_cursor)
      right_side_cursor = XCreateFontCursor(qt_xdisplay(), XC_right_side);
    if (!normal_cursor)
      normal_cursor = arrowCursor.handle();
    current_cursor = normal_cursor;

    installEventFilter(myapp);

    titlestring_offset = 0;
    titlestring_offset_delta = TITLE_ANIMATION_STEP;
    titlestring_too_large = false;
    titlestring_delay = 0;

    hidden_for_modules = false;
    autoraised_stopped = false;

    if (sizegrip != 0)
	XGrabButton(qt_xdisplay(), AnyButton, AnyModifier, sizegrip, True,
		    ButtonPressMask, GrabModeSync, GrabModeAsync,
		    None, bottom_right_cursor );


    doButtonGrab();
    unmap_events = 0;
}

Client::~Client(){
  DEBUG_EVENTS2("Destroy Client", this,window)
  if (ncmapwins != 0) {
    XFree((char *)cmapwins);
    free((char *)wmcmaps);
  }
  if (operation_client == this){
    if (myapp->operations->isVisible())
      myapp->operations->hide();
    operation_client = 0;
  }
}

QLabel* Client::gimmick=0;

// show the client. This means the decoration frame and the inner window
void Client::showClient(){
  DEBUG_EVENTS("ShowClient", this)
  if (!isShaded())
    XMapWindow(qt_xdisplay(),window);
  show();
}

// hide the client. This means the decoration frame and the inner window
void Client::hideClient(){
  DEBUG_EVENTS("HideClient", this)
  hide();
  if (!isShaded()){
    XUnmapWindow(qt_xdisplay(), window);
    unmap_events++;
  }
}


static QPixmap* pm_max = 0;
static QPixmap* pm_max_down = 0;
static QPixmap* pm_icon = 0;
static QPixmap* pm_close = 0;
static QPixmap* pm_pin_up = 0;
static QPixmap* pm_pin_down = 0;
static QPixmap* pm_menu = 0;

// simple encapsulation of the KDE iconloader
QPixmap* loadIcon(const char* name){
  QPixmap *result = new QPixmap;
  *result = kapp->getIconLoader()->loadIcon(name);
  return result;
}


// generate the window decoration buttons according to the settings in the options
void Client::generateButtons(){
  int i;

  buttonMaximize = 0;
  buttonSticky = 0;
  buttonMenu = 0;

  for (i=0;i<6;i++){
    buttons[i] = getNewButton( options.buttons[i]);
    if (getDecoration() == KWM::normalDecoration ) {
	if (!trans){
	    if (buttons[i])
		buttons[i]->show();
	}
	else { // transient windows, only menu and close on the first resp. last position possible
	    if ( i == 0 && buttons[i] == buttonMenu )
		continue;
	    if ( i == 3 && options.buttons[3] == CLOSE )
		continue;
	    if (buttons[i] )
		buttons[i]->hide();
	}
    }
    else {
	if (buttons[i] )
	    buttons[i]->hide();
    }
  }
  if (!buttonMaximize){
    buttonMaximize = new myPushButton(this);
    buttonMaximize->setToggleButton(true);
    buttonMaximize->hide();
    connect( buttonMaximize, SIGNAL(toggled(bool)), SLOT(maximizeToggled(bool)));
  }
  if (!buttonSticky){
    buttonSticky = new myPushButton(this);
    buttonSticky->setToggleButton(true);
    buttonSticky->hide();
    connect( buttonSticky, SIGNAL(toggled(bool)), SLOT(stickyToggled(bool)));
  }

  //CT 03Nov1998 - make sure sticky displays correctly
  if (this->isSticky())
    buttonSticky->setPixmap(*pm_pin_down);
  else
    buttonSticky->setPixmap(*pm_pin_up);
  //CT
}

// layout the window decoration buttons. Necessary after the window
// changed its size, for example.
void Client::layoutButtons(){
  int trX = 0;
  int trY = 0;
  int trW = 0;
  int trH = 0;

  if (getDecoration() != KWM::normalDecoration){
    // nothing
  } else {
    trX = BORDER;
    trY = BORDER;
    trW = width() - 2 * BORDER;
    trH = TITLEBAR_HEIGHT - TITLEWINDOW_SEPARATION;

    int button_y = BORDER + (TITLEBAR_HEIGHT
			     - TITLEWINDOW_SEPARATION
			     - BUTTON_SIZE)/ 2;
    if (button_y < 2)
      button_y = 2;

    if( buttons[0] && (!trans || buttons[0] == buttonMenu)){
      buttons[0]->setGeometry(trX,
			      button_y,
			      BUTTON_SIZE, BUTTON_SIZE);
      trX += BUTTON_SIZE;
      trW -= BUTTON_SIZE;
    }

    if (!trans){
      if( buttons[1] ){
	buttons[1]->setGeometry(trX,
				button_y,
				BUTTON_SIZE, BUTTON_SIZE);
	trX += BUTTON_SIZE;
	trW -= BUTTON_SIZE;
      }
      if( buttons[2] ){
	buttons[2]->setGeometry(trX,
				button_y,
				BUTTON_SIZE, BUTTON_SIZE);
	trX += BUTTON_SIZE;
	trW -= BUTTON_SIZE;
      }
    }
    if( buttons[3] && (!trans || options.buttons[3] == CLOSE)){
	trW -= BUTTON_SIZE;
	buttons[3]->setGeometry(trX + trW,
				button_y,
				BUTTON_SIZE, BUTTON_SIZE);
    }

    if (!trans) {
      if( buttons[4] ){
	trW -= BUTTON_SIZE;
	if (options.buttons[3] == CLOSE)
	  trW -= 2;
	buttons[4]->setGeometry(trX + trW,
				button_y,
				BUTTON_SIZE, BUTTON_SIZE);
      }
      if( buttons[5] ){
	trW -= BUTTON_SIZE;
	if (!buttons[4] && options.buttons[3] == CLOSE)
	  trW -= 2;
	buttons[5]->setGeometry(trX + trW,
				button_y,
				BUTTON_SIZE, BUTTON_SIZE);
      }
    }
    if (trX > BORDER) {
      trX += TITLEWINDOW_SEPARATION;
      trW -= TITLEWINDOW_SEPARATION;
    }
    if (trW < width() - trX - BORDER) {
      trW -= TITLEWINDOW_SEPARATION;
    }

    if (buttonMenu)
      // a menu button is always on top to allow the user to perfrom
      // all actions even if the window became too small
      buttonMenu->raise();
  }

  title_rect.setRect(trX,trY,trW,trH);
}

// the global options have changed => update the titlebar settings.
void Client::reconfigure(){
   int i;
   for (i=0;i<6;i++){
     if (buttons[i]){
       buttons[i]->hide();
       delete buttons[i];
       buttons[i] = 0;
     }
   }
   generateButtons();
   layoutButtons();
   paintState(false, true);
   doButtonGrab();
}

// if the titlestring is too large kwm can move it around (titlebar
// animation). animateTilebar() is invoked from a timer and will
// simply move it one single step further in this case.
void Client::animateTitlebar(){
  if (titlestring_too_large){
    paintState(true, false, true);
  }
}


void Client::mousePressEvent( QMouseEvent *ev ){

  old_cursor_pos = ev->pos();
  int com = MyApp::MouseNothing;
  mouse_release_command = MyApp::MouseNothing;

  if ((ev->state() & AltButton) == AltButton){
    // one of these "all" events with Alt on the titlebar or frame. Do
    // the same thing as if the user clicked inside the window
    if (ev->button() == LeftButton)
      com = options.CommandAll1;
    if (ev->button() == MidButton)
      com = options.CommandAll2;
    if (ev->button() == RightButton)
      com = options.CommandAll3;
    myapp->executeMouseBinding(this, com);
    return;
  }
  else  {
    // titlebar and frame event
    if (isActive() || !wantsFocus() ){
      if (ev->button() == LeftButton)
	com = options.CommandActiveTitlebar1;
      if (ev->button() == MidButton)
	com = options.CommandActiveTitlebar2;
      if (ev->button() == RightButton)
	com = options.CommandActiveTitlebar3;

    }
    else { // incactive
      if (ev->button() == LeftButton)
	com = options.CommandInactiveTitlebar1;
       if (ev->button() == MidButton)
	com = options.CommandInactiveTitlebar2;
      if (ev->button() == RightButton)
	com = options.CommandInactiveTitlebar3;
    }
    if (com == MyApp::MouseOperationsMenu){
	generateOperations();
	stopAutoraise();
	myapp->operations->popup(QCursor::pos());
	return;
    }

    if (ev->button() == MidButton) {
	// special hack for the middle mouse button to allow
	// movement without raise or lower
	if (com == MyApp::MouseActivateAndRaise) {
	    mouse_release_command = MyApp::MouseRaise;
	    com = MyApp::MouseActivate;
	}
	else if (com == MyApp::MouseActivateAndLower) {
	    mouse_release_command = MyApp::MouseLower;
	    com = MyApp::MouseActivate;
	}
	else if (com == MyApp::MouseRaise || com == MyApp::MouseLower) {
	    mouse_release_command = com;
	    com = MyApp::MouseNothing;
	}
    }

    myapp->executeMouseBinding(this, com);
  }

  dragging_state = dragging_nope;

  // resize on all corners
  do_resize = 0;
  if (current_cursor == bottom_right_cursor){
    old_cursor_pos = QPoint(width()-1, height()-1);
    QCursor::setPos(mapToGlobal(old_cursor_pos));
    do_resize = 1;
  }
  else if (current_cursor == bottom_left_cursor){
    old_cursor_pos = QPoint(0, height()-1);
    QCursor::setPos(mapToGlobal(old_cursor_pos));
    do_resize = 2;
  }
  else if (current_cursor == top_left_cursor){
    old_cursor_pos = QPoint(0, 0);
    QCursor::setPos(mapToGlobal(old_cursor_pos));
    do_resize = 3;
  }
  else if (current_cursor == top_right_cursor){
    old_cursor_pos = QPoint(width()-1, 0);
    QCursor::setPos(mapToGlobal(old_cursor_pos));
    do_resize = 4;
  }
  else if (current_cursor == left_side_cursor){
    old_cursor_pos = QPoint(0, old_cursor_pos.y());
    QCursor::setPos(mapToGlobal(old_cursor_pos));
    do_resize = 5;
  }
  else if (current_cursor == right_side_cursor){
    old_cursor_pos = QPoint(width()-1, old_cursor_pos.y());
    QCursor::setPos(mapToGlobal(old_cursor_pos));
    do_resize = 6;
  }
  else if (current_cursor == top_side_cursor){
    old_cursor_pos = QPoint(old_cursor_pos.x(), 0);
    QCursor::setPos(mapToGlobal(old_cursor_pos));
    do_resize = 7;
  }
  else if (current_cursor == bottom_side_cursor){
    old_cursor_pos = QPoint(old_cursor_pos.x(), height()-1);
    QCursor::setPos(mapToGlobal(old_cursor_pos));
    do_resize = 8;
  }
  else if (ev->pos().x() >= title_rect.x() && ev->pos().x() <= title_rect.x()+title_rect.width() &&
	ev->pos().y() >= title_rect.y() && ev->pos().y() <= title_rect.y()+title_rect.height()){
      dragging_state = dragging_smooth_wait;
    }


  if (do_resize > 0){
    dragging_state = dragging_smooth_wait;
    // set the current cursor
    XChangeActivePointerGrab( qt_xdisplay(),
			      ButtonPressMask | ButtonReleaseMask |
			      PointerMotionMask |
			      EnterWindowMask | LeaveWindowMask,
			      current_cursor, 0);
  }

}

void Client::mouseReleaseEvent( QMouseEvent* /* ev */){
  if (dragging_state == dragging_runs) {
      dragging_state = dragging_nope;
      releaseMouse();
      return;
  }
  dragging_state = dragging_nope;
  releaseMouse();
  if (mouse_release_command != MyApp::MouseNothing) {
      myapp->executeMouseBinding(this, mouse_release_command);
      mouse_release_command = MyApp::MouseNothing;
  }
}


void Client::mouseDoubleClickEvent( QMouseEvent* ev){
  if (ev->button() != LeftButton)
    return;
  if (ev->pos().x() >= title_rect.x() && ev->pos().x() <= title_rect.x()+title_rect.width() &&
      ev->pos().y() >= title_rect.y() && ev->pos().y() <= title_rect.y()+title_rect.height()){
    handleOperation(options.titlebar_doubleclick_command);
  }
}

void Client::mouseMoveEvent( QMouseEvent *ev ){
  if (dragging_state == dragging_runs)
    return;

  if (dragging_state != dragging_smooth_wait
      && dragging_state != dragging_enabled){
    old_cursor_pos = ev->pos();

    if (!getDecoration())
      return;
    // maybe we have to change the look of the cursor according
    // to the exact position
    if (ev->pos().x() >= BORDER && ev->pos().x() <= width()-1 - BORDER &&
	ev->pos().y() >= BORDER  &&
	ev->pos().y() <= height()-1 - BORDER){
      set_x_cursor(normal_cursor);
      return; // mouse over application window
    }

    int corner = 20 > BORDER ? 20 : BORDER;

    if (ev->pos().x() <= width() && ev->pos().x() >= width()-corner
	&& ev->pos().y() <= height() && ev->pos().y() >= height() - corner)
      set_x_cursor(bottom_right_cursor);
    else if (ev->pos().x() >= 0 && ev->pos().x() <= corner
	     && ev->pos().y() <= height() && ev->pos().y() >= height() - corner)
      set_x_cursor(bottom_left_cursor);
    else if (ev->pos().x() >= 0 && ev->pos().x() <= corner
	     && ev->pos().y() >= 0  && ev->pos().y() <= corner)
      set_x_cursor(top_left_cursor);
    else if (ev->pos().x() <= width() && ev->pos().x() >= width()-corner
	     && ev->pos().y() >= 0  && ev->pos().y() <= corner)
      set_x_cursor(top_right_cursor);
    else if (ev->pos().x() <= BORDER)
      set_x_cursor(left_side_cursor);
    else if (ev->pos().x() >= width() - BORDER)
      set_x_cursor(right_side_cursor);
    else if (ev->pos().y() <= BORDER)
      set_x_cursor(top_side_cursor);
    else if (ev->pos().y() >= height() - BORDER)
      set_x_cursor(bottom_side_cursor);
    else
      set_x_cursor(normal_cursor);

    return;
  }


  // this is for a smoother start of the  drag
  if (dragging_state == dragging_smooth_wait){
    if (
	(ev->pos().x() - old_cursor_pos.x())
	*
	(ev->pos().x() - old_cursor_pos.x())
	+
	(ev->pos().y() - old_cursor_pos.y())
	*
	(ev->pos().y() - old_cursor_pos.y())
	< 9){
      return;
    }
    else{
         dragging_state = dragging_enabled;
	 if (do_resize == 0){
	   grabMouse();
	   ensurePointerGrab();
	   XChangeActivePointerGrab( qt_xdisplay(),
				     ButtonPressMask | ButtonReleaseMask |
				     PointerMotionMask |
				     EnterWindowMask | LeaveWindowMask,
				     sizeAllCursor.handle(), 0);
	 }
    }
  }

  // dragging_state is dragging_enabled here

  dragging_state = dragging_runs;
  grabKeyboard();

  // the drags return true if the client crashed. This can only happend
  // in opaque drag since we have to process all events in the normal
  // fashion.
  // Do not change any variables then to avoid a segfault.
  if (!(do_resize > 0?resizedrag(this, do_resize):movedrag(this)))
    dragging_state = dragging_nope;
  releaseKeyboard();
  releaseMouse();

}

void Client::enterEvent( QEvent * ){
  // this is ugly but I see no other way.... fake a mouse move event
  // to set the right cursor shape.
  QMouseEvent me( Event_MouseButtonPress,
		  mapFromGlobal(QCursor::pos()),
		  0, 0);
  mouseMoveEvent(&me);

}

void Client::leaveEvent( QEvent * ){
  if (getDecoration() != KWM::noDecoration)
    set_x_cursor(normal_cursor);
}

void Client::paintEvent( QPaintEvent* e){
  QPainter p;
  p.begin(this);
  if (recently_resized) {
      recently_resized = FALSE;
      // do not set the clipping if recently_resized. Seems to be a
      // bug in the either e->rect() or the things kwm does in resizeEvent.
  }
  else
      p.setClipRect(e->rect());

  if (!options.ShapeMode || getDecoration() != KWM::normalDecoration){
      p.eraseRect(2,2,title_rect.x()-2, TITLEBAR_HEIGHT+BORDER-2);
      p.eraseRect(2,2,width()-4, BORDER-2);
      p.eraseRect(title_rect.right(),2, width()-title_rect.right()-2,
		  TITLEBAR_HEIGHT+BORDER-2-TITLEWINDOW_SEPARATION);
      p.eraseRect(2, TITLEBAR_HEIGHT+BORDER-TITLEWINDOW_SEPARATION, width()-4,
		  height()-TITLEBAR_HEIGHT-BORDER+TITLEWINDOW_SEPARATION-2);
      qDrawWinPanel(&p, rect(), colorGroup());
      p.eraseRect(width()-18, height()-18, 16, 16);
      if ( !fixedSize() )
	  qDrawShadeRect( &p, width()-20, height()-20, 20, 20, colorGroup(), False);
  }
  else {
    // the users wants shaped windows! A lot of code but more or less trivial....
    int x,y;
    // first the corners
    int w1 = options.shapePixmapTopLeft->width();
    int h1 = options.shapePixmapTopLeft->height();
    if (w1 > width()/2) w1 = width()/2;
    if (h1 > height()/2) h1 = height()/2;
    p.drawPixmap(0,0,*(options.shapePixmapTopLeft),
		 0,0,w1, h1);
    int w2 = options.shapePixmapTopRight->width();
    int h2 = options.shapePixmapTopRight->height();
    if (w2 > width()/2) w2 = width()/2;
    if (h2 > height()/2) h2 = height()/2;
    p.drawPixmap(width()-w2,0,*(options.shapePixmapTopRight),
		 options.shapePixmapTopRight->width()-w2,0,w2, h2);

    int w3 = options.shapePixmapBottomLeft->width();
    int h3 = options.shapePixmapBottomLeft->height();
    if (w3 > width()/2) w3 = width()/2;
    if (h3 > height()/2) h3 = height()/2;
    p.drawPixmap(0,height()-h3,*(options.shapePixmapBottomLeft),
		 0,options.shapePixmapBottomLeft->height()-h3,w3, h3);

    int w4 = options.shapePixmapBottomRight->width();
    int h4 = options.shapePixmapBottomRight->height();
    if (w4 > width()/2) w4 = width()/2;
    if (h4 > height()/2) h4 = height()/2;
    p.drawPixmap(width()-w4,height()-h4,*(options.shapePixmapBottomRight),
		 options.shapePixmapBottomRight->width()-w4,
		 options.shapePixmapBottomRight->height()-h4,
		 w4, h4);

    QPixmap pm;
    QWMatrix m;
    int n,s,w;
    //top
    pm = *(options.shapePixmapTop);

    s = width()-w2-w1;
    n = s/pm.width();
    w = n>0?s/n:s;
    m.reset();
    m.scale(w/(float)pm.width(), 1);
    pm = pm.xForm(m);

    x = w1;
    while (1){
      if (pm.width() < width()-w2-x){
	p.drawPixmap(x,BORDER-pm.height()-1,
		     pm);
	x += pm.width();
      }
      else {
	p.drawPixmap(x,BORDER-pm.height()-1,
		     pm,
		     0,0,width()-w2-x,pm.height());
	break;
      }
    }

    //bottom
    pm = *(options.shapePixmapBottom);

    s = width()-w4-w3;
    n = s/pm.width();
    w = n>0?s/n:s;
    m.reset();
    m.scale(w/(float)pm.width(), 1);
    pm = pm.xForm(m);

    x = w3;
    while (1){
      if (pm.width() < width()-w4-x){
	p.drawPixmap(x,height()-BORDER+1,pm);
	x += pm.width();
      }
      else {
	p.drawPixmap(x,height()-BORDER+1,pm,
		     0,0,width()-w4-x,pm.height());
	break;
      }
    }

    //left
    pm = *(options.shapePixmapLeft);

    s = height()-h3-h1;
    n = s/pm.height();
    w = n>0?s/n:s;
    m.reset();
    m.scale(1, w/(float)pm.height());
    pm = pm.xForm(m);

    y = h1;
    while (1){
      if (pm.height() < height()-h3-y){
	p.drawPixmap(BORDER-pm.width()-1, y,
		     pm);
	y += pm.height();
      }
      else {
	p.drawPixmap(BORDER-pm.width()-1, y,
		     pm,
		     0,0, pm.width(),
		     height()-h3-y);
	break;
      }
    }

    //right
    pm = *(options.shapePixmapRight);

    s = height()-h4-h2;
    n = s/pm.height();
    w = n>0?s/n:s;
    m.reset();
    m.scale(1, w/(float)pm.height());
    pm = pm.xForm(m);

    y = h2;
    while (1){
      if (pm.height() < height()-h4-y){
	p.drawPixmap(width()-BORDER+1, y,
		     pm);
	y += pm.height();
      }
      else {
	p.drawPixmap(width()-BORDER+1, y,
		     pm,
		     0,0, pm.width(),
		     height()-h4-y);
	break;
      }
    }

    p.eraseRect(BORDER-1, BORDER-1, width()-2*BORDER+2, height()-2*BORDER+2);
  }
  p.end();
  paintState();
}



// void myMoveResizeWindow(Display* dpy, Window win, int x, int y, int w, int h){
//   XMoveResizeWindow(dpy, win, x, y, w, h);
//   XWindowChanges wc;
//   wc.x = x;
//   wc.y = y;
//   wc.width = w;
//   wc.height = h;
//   XConfigureWindow(dpy, w, CWX|CWY|CWWidth|CWHeight, &wc);
// }


void Client::resizeEvent( QResizeEvent * ){
  // we have been resized => we have to layout the window decoration
  // again and adapt our swallowed application window
  recently_resized = true;

  layoutButtons();

  switch (getDecoration()){
  case 0:
    XMoveResizeWindow(qt_xdisplay(), window, 0,0,
		      geometry.width(), geometry.height());
    break;
  case 2:
    XMoveResizeWindow(qt_xdisplay(), window, (BORDER_THIN), (BORDER_THIN),
		      geometry.width() - 2*BORDER_THIN,
		      geometry.height() - 2*BORDER_THIN);
    break;
  default:
    XMoveResizeWindow(qt_xdisplay(), window, (BORDER), (BORDER) + TITLEBAR_HEIGHT,
		      geometry.width() - 2*BORDER,
		      geometry.height() - 2*BORDER - TITLEBAR_HEIGHT);
    break;
  }

  if (options.ShapeMode && getDecoration() == KWM::normalDecoration){	
    // the users wants shaped windows! After a resize we have to
    // recalculate the pixmaps and create a new shape combine mask.

    QBitmap shapemask(width(), height());
    shapemask.fill(color0);
    QPainter p;
    p.begin(&shapemask);
    p.setBrush(color1);
    p.setPen(color1);



    int x,y;
    // first the corners
    int w1 = options.shapePixmapTopLeft->width();
    int h1 = options.shapePixmapTopLeft->height();
    if (w1 > width()/2) w1 = width()/2;
    if (h1 > height()/2) h1 = height()/2;
    p.drawPixmap(0,0,*(options.shapePixmapTopLeft->mask()),
		 0,0,w1, h1);
    int w2 = options.shapePixmapTopRight->width();
    int h2 = options.shapePixmapTopRight->height();
    if (w2 > width()/2) w2 = width()/2;
    if (h2 > height()/2) h2 = height()/2;
    p.drawPixmap(width()-w2,0,*(options.shapePixmapTopRight->mask()),
		 options.shapePixmapTopRight->width()-w2,0,w2, h2);

    int w3 = options.shapePixmapBottomLeft->width();
    int h3 = options.shapePixmapBottomLeft->height();
    if (w3 > width()/2) w3 = width()/2;
    if (h3 > height()/2) h3 = height()/2;
    p.drawPixmap(0,height()-h3,*(options.shapePixmapBottomLeft->mask()),
		 0,options.shapePixmapBottomLeft->height()-h3,w3, h3);

    int w4 = options.shapePixmapBottomRight->width();
    int h4 = options.shapePixmapBottomRight->height();
    if (w4 > width()/2) w4 = width()/2;
    if (h4 > height()/2) h4 = height()/2;
    p.drawPixmap(width()-w4,height()-h4,*(options.shapePixmapBottomRight->mask()),
		 options.shapePixmapBottomRight->width()-w4,
		 options.shapePixmapBottomRight->height()-h4,
		 w4, h4);


    QPixmap pm;
    QWMatrix m;
    int n,s,w;
    //top
    pm = *(options.shapePixmapTop->mask());

    s = width()-w2-w1;
    n = s/pm.width();
    w = n>0?s/n:s;
    m.reset();
    m.scale(w/(float)pm.width(), 1);
    pm = pm.xForm(m);

    x = w1;
    while (1){
      if (pm.width() < width()-w2-x){
	p.drawPixmap(x,BORDER-pm.height()-1,
		     pm);
	x += pm.width();
      }
      else {
	p.drawPixmap(x,BORDER-pm.height()-1,
		     pm,
		     0,0,width()-w2-x,pm.height());
	break;
      }
    }

    //bottom
    pm = *(options.shapePixmapBottom->mask());

    s = width()-w4-w3;
    n = s/pm.width();
    w = n>0?s/n:s;
    m.reset();
    m.scale(w/(float)pm.width(), 1);
    pm = pm.xForm(m);

    x = w3;
    while (1){
      if (pm.width() < width()-w4-x){
	p.drawPixmap(x,height()-BORDER+1,pm);
	x += pm.width();
      }
      else {
	p.drawPixmap(x,height()-BORDER+1,pm,
		     0,0,width()-w4-x,pm.height());
	break;
      }
    }

    //left
    pm = *(options.shapePixmapLeft->mask());

    s = height()-h3-h1;
    n = s/pm.height();
    w = n>0?s/n:s;
    m.reset();
    m.scale(1, w/(float)pm.height());
    pm = pm.xForm(m);

    y = h1;
    while (1){
      if (pm.height() < height()-h3-y){
	p.drawPixmap(BORDER-pm.width()-1, y,
		     pm);
	y += pm.height();
      }
      else {
	p.drawPixmap(BORDER-pm.width()-1, y,
		     pm,
		     0,0, pm.width(),
		     height()-h3-y);
	break;
      }
    }

    //right
    pm = *(options.shapePixmapRight->mask());

    s = height()-h4-h2;
    n = s/pm.height();
    w = n>0?s/n:s;
    m.reset();
    m.scale(1, w/(float)pm.height());
    pm = pm.xForm(m);

    y = h2;
    while (1){
      if (pm.height() < height()-h4-y){
	p.drawPixmap(width()-BORDER+1, y,
		     pm);
	y += pm.height();
      }
      else {
	p.drawPixmap(width()-BORDER+1, y,
		     pm,
		     0,0, pm.width(),
		     height()-h4-y);
	break;
      }
    }

    p.fillRect(BORDER-1, BORDER-1, width()-2*BORDER+2, height()-2*BORDER+2, color1);

    p.end();

    // finally set the shape :-)
    setBackgroundMode( NoBackground );
    XShapeCombineMask( qt_xdisplay(), winId(),
		       ShapeBounding, 0, 0, shapemask.handle(),
		       ShapeSet );
  }
}

// create a new pushbutton associated with the specified button
// functions.
myPushButton * Client::getNewButton(BUTTON_FUNCTIONS buttonFunction){
  if (!pm_max){
    pm_max = loadIcon("maximize.xpm");
  }
  if (!pm_max_down){
    pm_max_down = loadIcon("maximizedown.xpm");
  }
  if (!pm_pin_up){
    pm_pin_up = loadIcon("pinup.xpm");
  }
  if (!pm_pin_down){
    pm_pin_down = loadIcon("pindown.xpm");
  }

  if(buttonFunction == NOFUNC)
    return 0;

  if (buttonFunction == MAXIMIZE){
    if (fixedSize())
      return 0;
  }


  myPushButton *button = new myPushButton(this);
  switch(buttonFunction){
  case MAXIMIZE:
    button->setPixmap(*pm_max);
    button->setToggleButton(true);
    buttonMaximize = button;
    connect( button, SIGNAL(toggled(bool)), SLOT(maximizeToggled(bool)));
    break;
  case ICONIFY:
    if (!pm_icon){
      pm_icon = loadIcon("iconify.xpm");
    }
    button->setPixmap(*pm_icon);
    connect( button, SIGNAL(clicked()), SLOT(iconifySlot()));
    break;
  case CLOSE:
    if (!pm_close){
      pm_close = loadIcon("close.xpm");
    }
    button->setPixmap(*pm_close);
    connect( button, SIGNAL(clicked()), SLOT(closeClicked()));
    break;
  case STICKY:
    button->setPixmap(*pm_pin_up);
    button->setToggleButton(true);
    buttonSticky = button;
    connect( button, SIGNAL(toggled(bool)), SLOT(stickyToggled(bool)));
    break;
  case MENU:
    if (!pm_menu)
      pm_menu = loadIcon("menu.xpm");
    {
      QPixmap pm = KWM::miniIcon(window, 16, 16);
      if (!pm.isNull())
	button->setPixmap(pm);
      else
	button->setPixmap(*pm_menu);
    }
    buttonMenu = button;
    connect( button, SIGNAL(pressed()), SLOT(menuPressed()));
    connect( button, SIGNAL(released()), SLOT(menuReleased()));
    break;
  default:
    fprintf(stderr,"Bad Button Function %d\n", buttonFunction);
    break;
  }

  return button;

}

// generates a sensefull label property from the name, icon, klass
// and instance. Also ensures that the label is unique by adding a
// number in brackets after the name.
void Client::setLabel(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_TITLE", False);

  QString rawlabel = name;
  QString oldlabel = label;
  label = "";
  if (rawlabel.isEmpty())
    rawlabel = iconname;
  if (rawlabel.isEmpty())
    rawlabel = instance;
  if (rawlabel.isEmpty())
    rawlabel = klass;
  if (rawlabel.isEmpty())
    rawlabel = "???";
  if (rawlabel.left(1) == "("){
    // (...) means iconified. MUST be unique!
    rawlabel.detach();
    rawlabel.prepend("\"");
    rawlabel.append("\"");
  }
  if (manager->hasLabel(rawlabel)){
    int i = 2;
    QString b,c;
    do {
      b.setNum(i);
      i++;
      c = rawlabel + " <" + b + ">";
    } while (manager->hasLabel(c));
    rawlabel = c;
  }
  label = rawlabel;

  if (oldlabel != label){
    if (isVisible())
	paintState( TRUE );
    XChangeProperty(qt_xdisplay(), window, a, XA_STRING, 8,
		    PropModeReplace, (unsigned char *)(label.data()),
		    label.length()+1);

  }

}

//  a static pointer used to indicate which client defined the
//  operation popup at present.
Client* Client::operation_client = 0;


// changes the global window operations popup to fit to this
// client. This means activating/deactivating the items
// appropriately and setting the right virual desktop.
void Client::generateOperations(){
  operation_client = this;
  myapp->operations->clear();
  if (isMaximized() && !fixedSize()){
    myapp->operations->insertItem(KWM::getUnMaximizeString(),
				  OP_RESTORE);
  }
  else {
    myapp->operations->insertItem(KWM::getMaximizeString(),
				  OP_MAXIMIZE);
  }

  if (fixedSize())
    myapp->operations->setItemEnabled(OP_MAXIMIZE, false);



  myapp->operations->insertItem(KWM::getIconifyString(),
				OP_ICONIFY);
  if (!wantsFocus() || (trans != None && trans != qt_xrootwin()))
    myapp->operations->setItemEnabled(OP_ICONIFY, false);

  myapp->operations->insertItem(KWM::getMoveString(),
				OP_MOVE);
  myapp->operations->insertItem(KWM::getResizeString(),
				OP_RESIZE);

  if (fixedSize())
    myapp->operations->setItemEnabled(OP_RESIZE, false);

  if (isSticky())
    myapp->operations->insertItem(KWM::getUnStickyString(),
				  OP_STICKY);
  else
    myapp->operations->insertItem(KWM::getStickyString(),
				  OP_STICKY);
  if (trans != None && trans != qt_xrootwin() && trans != window)
    myapp->operations->setItemEnabled(OP_STICKY, false);

  myapp->desktopMenu->clear();
  int i;
  for (i=1; i <= manager->number_of_desktops; i++){
      QString b = "&";
      b.append(KWM::getDesktopName(i));
      myapp->desktopMenu->insertItem(b, i);
  }
  myapp->desktopMenu->setItemChecked(manager->currentDesktop(), true);

  int dm = myapp->operations->insertItem(KWM::getToDesktopString(),
					 myapp->desktopMenu);
  if (trans != None && trans != qt_xrootwin() && trans != window)
    myapp->operations->setItemEnabled(dm, false);
  myapp->operations->insertSeparator();
  myapp->operations->insertItem(KWM::getCloseString(),
				OP_CLOSE);
}

// shows the operations popup at the right place near the menu button
void Client::showOperations(){
  stopAutoraise();
  generateOperations();
  switch (getDecoration()){
  case 0:
    myapp->operations->popup(geometry.topLeft());
    break;
  case 2:
    myapp->operations->popup(QPoint(geometry.x() + BORDER_THIN,
			     geometry.y() + BORDER_THIN));
    break;
  default:
    myapp->operations->popup(QPoint(geometry.x() + BORDER,
			     geometry.y() + BORDER + TITLEBAR_HEIGHT));
    break;
  }
}

// windows can have a fixed size, that means the window manager must
// not resize them in any way.
bool Client::fixedSize(){
  return (size.flags & PMaxSize ) && (size.flags & PMinSize)
    && (size.max_width <= size.min_width)
    && (size.max_height <= size.min_height);
}




// activate or deactivate a window.
void Client::setactive(bool on){
  if (is_active && !on && myapp->operations->isVisible())
    myapp->operations->hide();

  is_active=on;
  if (!do_not_draw)
    paintState();
  if (is_active){
      if ( options.FocusPolicy != CLICK_TO_FOCUS) {
	  if (options.AutoRaise)
	      QTimer::singleShot(options.AutoRaiseInterval, this, SLOT(autoRaise()));
	autoraised_stopped = false;
    }
    else {
	doButtonGrab();
    }
  }
  else {
    stopAutoraise(false);
    doButtonGrab();
  }
  placeGimmick();
}

// Repaint the state of a window. Active or inactive windows differ
// only in the look of the titlebar. If only_label is true then only
// the label string is repainted. This is used for the titlebar
// animation with animate = TRUE.
void Client::paintState(bool only_label, bool colors_have_changed, bool animate){
  QRect r = title_rect;
  bool double_buffering = false;
  QPixmap* buffer = 0;

  if (r.width() <= 0 || r.height() <= 0
      || getDecoration()!=KWM::normalDecoration)
    return;
  int x;


  TITLEBAR_LOOK look = options.TitlebarLook;

  if (look == H_SHADED || look == V_SHADED){
    // the new horizontal (and vertical) shading code
    if (colors_have_changed){
      aShadepm.resize(0,0);
      iaShadepm.resize(0,0);
    }

    // the user selected shading. Do a plain titlebar anyway if the
    // shading would be senseless (higher performance and less memory
    // consumption)
    if (is_active){
      if ( myapp->activeTitleColor ==  myapp->activeTitleBlend)
	look = PLAIN;
    }
    else {
      if ( myapp->inactiveTitleColor ==  myapp->inactiveTitleBlend)
	look = PLAIN;
    }
  }

  // the following is the old/handmade vertical shading code

//   if (options.TitlebarLook == SHADED){
//     if (is_active && shaded_pm_active && shaded_pm_active_color != myapp->activeTitleColor){
//       delete shaded_pm_active;
//       shaded_pm_active = 0;
//     }
//     if (is_active && shaded_pm_inactive && shaded_pm_inactive_color != myapp->inactiveTitleColor){
//       delete shaded_pm_inactive;
//       shaded_pm_inactive = 0;
//     }
//   }

//   if (options.TitlebarLook == SHADED
//       &&
//       (is_active && (!shaded_pm_active)
//        || (!is_active && !shaded_pm_inactive))
//       ){
//     int i,y;
//     QImage image (50, TITLEBAR_HEIGHT, 8, TITLEBAR_HEIGHT);
//     QColor col = is_active ? myapp->activeTitleColor : myapp->inactiveTitleColor;
//     for ( i=0; i<TITLEBAR_HEIGHT; i++ ){
//       image.setColor( i, col.light(100+(i-TITLEBAR_HEIGHT/2)*3).rgb());
//     }
//     for (y=0; y<TITLEBAR_HEIGHT; y++ ) {
//       uchar *pix = image.scanLine(y);
//       for (x=0; x<50; x++)
//         *pix++ = is_active ? TITLEBAR_HEIGHT-1-y : y;
//     }
//     if (is_active){
//       shaded_pm_active = new QPixmap;
//       *shaded_pm_active = image;
//       shaded_pm_active_color = myapp->activeTitleColor;
//     }
//     else{
//       shaded_pm_inactive = new QPixmap;
//       *shaded_pm_inactive = image;
//       shaded_pm_inactive_color = myapp->inactiveTitleColor;
//     }
//   }

  QPainter p;

  if (only_label && animate){
    double_buffering = (look == H_SHADED || look == V_SHADED || look == PIXMAP);
    if(titlestring_delay) {
      titlestring_delay--;
      return;
    }
    else
      titlestring_offset += titlestring_offset_delta;
    if (!double_buffering){
      if (titlestring_offset_delta > 0)
	bitBlt(this,
	       r.x()+titlestring_offset_delta, r.y(),
	       this,
	       r.x(), r.y(),
	       r.width()-titlestring_offset_delta, r.height());
      else
	bitBlt(this,
	       r.x(), r.y(),
	       this,
	       r.x()-titlestring_offset_delta, r.y(),
	       r.width()+titlestring_offset_delta, r.height());
    }
  }

  if (!double_buffering)
    p.begin( this );
  else {
    // enable double buffering to avoid flickering with horizontal shading
    buffer = new QPixmap(r.width(), r.height());
    p.begin(buffer);
    r.setRect(0,0,r.width(),r.height());
  }

  QPixmap *pm;
  p.setClipRect(r);
  p.setClipping(True);

  // old handmade vertical shading code

//   if (options.TitlebarLook == SHADED || options.TitlebarLook == PIXMAP){

//     if (options.TitlebarLook == SHADED)
//       pm = is_active ? shaded_pm_active : shaded_pm_inactive;
//     else



  if (look == PIXMAP){
      pm = is_active ? options.titlebarPixmapActive: options.titlebarPixmapInactive;
      for (x = r.x(); x < r.x() + r.width(); x+=pm->width())
	p.drawPixmap(x, r.y(), *pm);
  }
  else if (look == H_SHADED || look == V_SHADED ){
    // the new horizontal shading code
    QPixmap* pm = 0;
    if (is_active){
      if (aShadepm.size() != r.size()){
	aShadepm.resize(r.width(), r.height());
	kwm_gradientFill( aShadepm, myapp->activeTitleColor, myapp->activeTitleBlend, look == V_SHADED );
	//aShadepm.gradientFill( myapp->activeTitleColor, myapp->activeTitleBlend, look == V_SHADED );
      }
      pm = &aShadepm;
    }
    else {
      if (iaShadepm.size() != r.size()){
	iaShadepm.resize(r.width(), r.height());
	kwm_gradientFill( iaShadepm, myapp->inactiveTitleColor, myapp->inactiveTitleBlend, look == V_SHADED );
	//	iaShadepm.gradientFill( myapp->inactiveTitleColor, myapp->inactiveTitleBlend, look == V_SHADED );
      }
      pm = &iaShadepm;
    }

    p.drawPixmap( r.x(), r.y(), *pm );
  }
  else { // TitlebarLook == TITLEBAR_PLAIN
    p.setBackgroundColor( is_active ? myapp->activeTitleColor : myapp->inactiveTitleColor);
    if (only_label && !double_buffering && animate){
       p.eraseRect(QRect(r.x(), r.y(), TITLE_ANIMATION_STEP+1, r.height()));
       p.eraseRect(QRect(r.x()+r.width()-TITLE_ANIMATION_STEP-1, r.y(),
 			TITLE_ANIMATION_STEP+1, r.height()));
    }
    else {
      p.eraseRect(r);
    }
  }
  p.setClipping(False);

  //CT 02Dec1998 - optional shade, suggested by Nils Meier <nmeier@vossnet.de>
  if (is_active && options.framedActiveTitle)
    qDrawShadePanel( &p, r, colorGroup(), true );

  p.setPen(is_active ? myapp->activeTextColor : myapp->inactiveTextColor);

  /*CT 03Nov1998 - a customized titlebar font
  QFont fnt = kapp->generalFont;
  fnt.setPointSize(12);
  fnt.setBold(true);
  p.setFont(fnt);*/
  p.setFont(myapp->tFont);
  //CT

  titlestring_too_large = (p.fontMetrics().width(QString(" ")+label+" ")>r.width());
  if (titlestring_offset_delta > 0){
    if (!titlestring_delay){
      if (titlestring_offset > 0
      && titlestring_offset > r.width() - p.fontMetrics().width(QString(" ")+label+" ")){
        // delay animation before reversing direction
        titlestring_delay = TITLE_ANIMATION_DELAY;
        titlestring_offset_delta *= -1;
      }
    }
  }
  else {
    if(!titlestring_delay){
      if (titlestring_offset < 0
      && titlestring_offset + p.fontMetrics().width(QString(" ")+label+" ") < r.width()){
        if(titlestring_offset_delta != 0)
          // delay animation before reversing direction
          titlestring_delay = TITLE_ANIMATION_DELAY;
        titlestring_offset_delta *= -1;
      }
    }
  }

  p.setClipRect(r);
  p.setClipping(True);

  //CT 04Nov1998 - align the title in the bar
  int aln = 0;
  int need = p.fontMetrics().width(QString(" ")+label+" ");
  if (options.alignTitle == AT_LEFT || r.width() < need) aln = 0;
  else if (options.alignTitle == AT_MIDDLE )
    aln = r.width()/2 - need/2;
  else if (options.alignTitle == AT_RIGHT)
    aln = r.width() - need;
  //CT see next lines. Replaced two 0 by `aln`. Moved next two lines here
  //  from above.

  if (!titlestring_too_large)
    titlestring_offset = aln;

  //CT 02Dec1998 - optional noPixmap behind text,
  //     suggested by Nils Meier <nmeier@vossnet.de>
  if (!options.PixmapUnderTitleText && look == PIXMAP ) {
    /* NM 02Dec1998 - Clear bg behind text */
    p.setBackgroundColor(is_active ? myapp->activeTitleColor :
			 myapp->inactiveTitleColor);
    p.eraseRect(
		r.x()+(options.TitleAnimation?titlestring_offset:aln),
		r.y()+(r.height()-p.fontMetrics().height())/2,
		need,
		p.fontMetrics().height());
  }
  //CT

  //CT 02Dec1998 - vertical text centering,
  //     thanks to Nils Meier <nmeier@vossnet.de>
  p.drawText(r.x()+
	     (options.TitleAnimation?titlestring_offset:aln),

	     r.y()+
	     (r.height()-p.fontMetrics().height())/2+
	     p.fontMetrics().ascent(),

	     QString(" ")+label+" ");

  p.setClipping(False);
  p.end();
  if (double_buffering){
    bitBlt(this,
	   title_rect.x(), title_rect.y(),
	   buffer,
	   0,0,
	   buffer->width(), buffer->height());
    delete buffer;
  }
}

void Client::createGimmick(){
  if (gimmick)
    delete gimmick;
  gimmick = 0;
  if (options.GimmickMode){
    gimmick = new QLabel;
    gimmick->resize(options.gimmickPixmap->size());
    gimmick->setPixmap(*(options.gimmickPixmap));
    if (options.gimmickPixmap->mask()){
      XShapeCombineMask( qt_xdisplay(), gimmick->winId(),
			 ShapeBounding, 0, 0,
			 options.gimmickPixmap->mask()->handle(),
			 ShapeSet);
    }
  }
}

void Client::hideGimmick(){
  if(gimmick)
    gimmick->hide();
}
void Client::placeGimmick(){
  static Client*  gimmick_client = 0; // used to avoid flickering
  if (!is_active)
    return;
  if (options.GimmickMode){
    if (!gimmick)
      createGimmick();
    gimmick->show();
    gimmick->move(geometry.x()+geometry.width()*options.GimmickPositionX/100+
		  options.GimmickOffsetX,
		  geometry.y()+geometry.height()*options.GimmickPositionY/100+
		  options.GimmickOffsetY-gimmick->height());

      // put the gimmick right above the window itself in the stacking order
    if (gimmick_client != this){
      gimmick_client = this;
      Window* new_stack = new Window[2];
      new_stack[0]=winId();
      new_stack[1]=gimmick->winId();
      XRestackWindows(qt_xdisplay(), new_stack, 2);
      new_stack[0]=gimmick->winId();
      new_stack[1]=winId();
      XRestackWindows(qt_xdisplay(), new_stack, 2);
      delete [] new_stack;
    }
  }
}

// set the mouse pointer shape to the specified cursor. Also
// stores the defined shaped in current_cursor.
void Client::set_x_cursor(Cursor cur){
  if (cur != current_cursor){
    current_cursor = cur;
    XDefineCursor(qt_xdisplay(), winId(), current_cursor);
  }
}



void Client::iconifySlot(){
  iconify(True);
}

// iconify this client. Takes care about floating or transient
// windows. If animation is true, kwm may show some kind of
// animation (if set by the options)
void Client::iconify(bool animation){
  if (isIconified())
    return;

  if (animation == TRUE ) {
      if ( isDialog() ) { // transient windows or dialogs cannot be iconified, iconify everything
	  mainClient()->iconify();
	  return;
      }
  }

  if (manager->current() &&
      this != manager->current()
      && (manager->current()->trans == window || manager->current()->leader == window) )
    manager->activateClient(this);
  manager->iconifyTransientOf(this);
  iconified = True;

  if (state == NormalState){
    hideClient();
    if (animation){
      KWM::raiseSoundEvent("Window Iconify");
      if (animate_size_change(this, QRect(geometry.x(), geometry.y(), width(), height()),
			      KWM::iconGeometry(window),
			      getDecoration()==KWM::normalDecoration,
			      title_rect.x(),
			      width()-title_rect.right()))
	return; //client has been destroyed
    }
    if (isActive())
      manager->noFocus();
  }
  KWM::setIconify(window, True);
  manager->changedClient(this);
  manager->setWindowState(this, IconicState);

}

// unIconify this client. Takes care about floating or transient
// windows. If animation is true, kwm may show some kind of
// animation (if set by the options)
void Client::unIconify(bool animation){
  if (isWithdrawn())
    return;
  if (!isIconified())
    return;

  iconified = False;
  KWM::setIconify(window, False);
  manager->changedClient(this);
  if (isOnDesktop(manager->currentDesktop())){
    if (animation){
      KWM::raiseSoundEvent("Window DeIconify");
      if (animate_size_change(this,
			      KWM::iconGeometry(window),
			      QRect(geometry.x(), geometry.y(), width(), height()),
			      getDecoration()==KWM::normalDecoration,
			      title_rect.x(),
			      width()-title_rect.right()))
	return; //client has been destroyed
    }
    showClient();
    manager->setWindowState(this, NormalState);
    manager->raiseClient( this );
  }
  else {
    manager->raiseClient( this );
  }
  manager->unIconifyTransientOf(this);
  if (isOnDesktop(manager->currentDesktop()) && animation &&!CLASSIC_FOCUS){
    manager->activateClient(this);
  }
}


// move the client onto a new desktop. Will take floating and
// transient windows with it.
void Client::ontoDesktop(int new_desktop){
  if (new_desktop == desktop || isSticky())
    return;
  if (new_desktop < 1 || new_desktop > manager->number_of_desktops)
    return;

  if ( isDialog() && mainClient()->desktop != new_desktop) {
      // it is forbidden to move transient windows or dialogs onto other
      // desktops than their parents. Move everthing instead.
      mainClient()->ontoDesktop( new_desktop );
      return;
  }

  desktop = new_desktop;

  KWM::moveToDesktop(window, desktop);
  manager->changedClient(this);

  if (state == NormalState){
    hideClient();
    manager->setWindowState(this, IconicState);

    if (isActive())
      manager->noFocus();
  }


  manager->ontoDesktopTransientOf(this);

  if (desktop == manager->currentDesktop() && !isIconified()){
    showClient();
    manager->setWindowState(this, NormalState);
    manager->raiseClient( this );
    if (!CLASSIC_FOCUS)
      manager->activateClient( this );
  }
}


// maximize this client. Mode can be horizontal, vertical or fullscreen.
// Store the current geometry in geometry_restore
void Client::maximize(int mode, bool animate){
  if (isMaximized())
    return;

  if (isShaded())
      toggleShade();
  maximized = true;
  maximize_mode = mode;
  geometry_restore = geometry;
  KWM::setGeometryRestore(window, geometry_restore);

  QRect maxRect = KWM::getWindowRegion(desktop);

  if (myapp->systemMenuBar) {
      maxRect.setTop(myapp->systemMenuBar->geometry().bottom());
  }
  else
  {
      // check for some floating windows (maybe a menubar?)
      Client* c = manager->clientAtPosition(maxRect.x(),maxRect.y());
      if (c && c->trans == window && c->isMenuBar()) {
 	  maxRect.setTop(c->geometry.bottom());
      }
  }

  switch (mode) {
  case vertical:
    geometry.moveTopLeft(QPoint(geometry.x(), maxRect.y()));
    geometry.setHeight(maxRect.height());
    break;
  case horizontal:
    geometry.moveTopLeft(QPoint(maxRect.x(),geometry.y()));
    geometry.setWidth(maxRect.width());
    break;
  default: //
    geometry = maxRect;
    break;
  }

  adjustSize();

  if (animate) {
      if (state == NormalState)
	  manager->raiseSoundEvent("Window Maximize");
      if (animate_size_change(this, geometry_restore, geometry,
			      getDecoration()==KWM::normalDecoration,
			      title_rect.x(),
			      width()-title_rect.right()))
	  return; //client has been destroyed
  }
  KWM::setMaximize(window, maximized, mode);
  if (!buttonMaximize->isOn())
      buttonMaximize->toggle();

  manager->sendConfig(this);
  layoutButtons();
}

// unmaximize this client. Geometry will be as it was before
// (geometry_restore)
void Client::unMaximize(){
  if (!isMaximized())
    return;
  if (isShaded())
      toggleShade();
  maximized = false;
  if (geometry != geometry_restore && state == NormalState)
    manager->raiseSoundEvent("Window UnMaximize");
  if (animate_size_change(this, geometry, geometry_restore,
			  getDecoration()==KWM::normalDecoration,
			  title_rect.x(),
			  width()-title_rect.right()))
    return; //client has been destroyed
  geometry = geometry_restore;
  KWM::setMaximize(window, maximized);
  if (buttonMaximize->isOn())
    buttonMaximize->toggle();

  manager->sendConfig(this);
  layoutButtons();
}

void Client::maximizeToggled(bool depressed){

  bool do_not_activate = depressed == isMaximized();

  if (!do_not_activate){
    if ( depressed){
      switch (buttonMaximize->last_button){
      case MidButton:
	maximize(options.MaximizeOnlyVertically?fullscreen:vertical);
	break;
      case RightButton:
	maximize(horizontal);
	break;
      default: //Leftbutton
	maximize(options.MaximizeOnlyVertically?vertical:fullscreen);
	break;
      }
    }
    else
      unMaximize();

    if (state == NormalState){
      manager->raiseClient( this );
      if (!CLASSIC_FOCUS)
	manager->activateClient( this );
    }
  }
  buttonMaximize->setPixmap(buttonMaximize->isOn() ? *pm_max_down : *pm_max);
  buttonMaximize->update();
}


void Client::closeClicked(){
  // closing of clients if handled by the manager itself.
  manager->closeClient(this);
}

void Client::stickyToggled(bool depressed){
  QPixmap* pm;

  if (depressed == isSticky())
    return;

  if ( isDialog() && mainClient()->isSticky() != depressed) {
       // transient windows or dialogs cannot be made sticky alone, sticky everything instead
      buttonSticky->toggle();
      mainClient()->buttonSticky->toggle();
      return;
  }


  KWM::setSticky(window, depressed);
  sticky = depressed;

  if (depressed){
    pm = pm_pin_down;
    if (state != NormalState && !isIconified()){
      showClient();
      manager->setWindowState(this, NormalState);
      manager->raiseClient( this );
      if (!CLASSIC_FOCUS)
	manager->activateClient( this );
    }
  }
  else{
    desktop = manager->currentDesktop();
    KWM::moveToDesktop(window, desktop);
    pm = pm_pin_up;
  }

  if (state == NormalState && !trans) {
    if (depressed)
      manager->raiseSoundEvent("Window Sticky");
    else
      manager->raiseSoundEvent("Window UnSticky");
  }
  manager->changedClient(this);
  buttonSticky->setPixmap(*pm);
  buttonSticky->update();
  manager->stickyTransientOf(this, depressed);
}

void Client::menuPressed(){
  // the menu button is somewhat special since it does not only show a
  // popup menu but also reacts on doubleclicks.
  static QTime click_time;
  static bool click_time_initialized = FALSE;
  if (!click_time_initialized) {
      click_time = QTime::currentTime();
      click_time_initialized = TRUE;
      ignore_release_on_this = buttonMenu;
      showOperations();
      return;
  }
  if (!isActive()){
    myapp->operations->hide();
    manager->raiseClient(this);
    if (!CLASSIC_FOCUS)
      manager->activateClient(this);
  }
  if (click_time.msecsTo(QTime::currentTime())<700){
    // some kind of doubleclick => close. Will be checked in the
    // menuReleased handler.
    do_close = true;
  }
  else {
    ignore_release_on_this = buttonMenu;
    showOperations();
  }

  click_time = QTime::currentTime();
}


void Client::menuReleased(){
  if (do_close) closeClicked();
  do_close = false;
}


// handles a window operation
void Client::handleOperation(int i){
  switch (i){
  case OP_MOVE:
    manager->raiseClient(this);
    grabMouse();
    ensurePointerGrab();
    // use the PointerMotionMask, too
    XChangeActivePointerGrab( qt_xdisplay(),
			      ButtonPressMask | ButtonReleaseMask |
			      PointerMotionMask |
			      EnterWindowMask | LeaveWindowMask,
			      sizeAllCursor.handle(), 0);
    // correct positioning
    {
      QPoint tmp = mapToGlobal(QPoint(width()/2, TITLEBAR_HEIGHT/2));
      QCursor::setPos(tmp);
      old_cursor_pos = mapFromGlobal(tmp);
    }
    dragging_state = dragging_enabled;
    do_resize = 0;
    mouseMoveEvent(0);
    break;
  case OP_RESIZE:
    manager->raiseClient(this);
    grabMouse();
    ensurePointerGrab();
    // use the PointerMotionMask, too
    XChangeActivePointerGrab( qt_xdisplay(),
			      ButtonPressMask | ButtonReleaseMask |
			      PointerMotionMask |
			      EnterWindowMask | LeaveWindowMask,
			      bottom_right_cursor, 0);
    // correct positioning
    {
      QPoint tmp = mapToGlobal(QPoint(width(), height()));
      if (tmp.x() > QApplication::desktop()->width())
	tmp.setX(QApplication::desktop()->width());
      if (tmp.y() > QApplication::desktop()->height())
	tmp.setY(QApplication::desktop()->height());
      QCursor::setPos(tmp);
      old_cursor_pos = mapFromGlobal(tmp);
    }
    grabKeyboard();
    dragging_state = dragging_runs;
    // the drags return true if the client crashed. This can only happend
    // in opaque drag since we have to process all events in the normal
    // fashion.
    // Do not change any variables then to avoid a segfault.
    if (!resizedrag(this, 1))
      dragging_state = dragging_nope;
    releaseKeyboard();
    releaseMouse();
    break;
  case OP_MAXIMIZE:
    if (!isMaximized())
      maximize( options.MaximizeOnlyVertically?vertical:fullscreen);
    else
      unMaximize();
    break;
  case OP_RESTORE:
    unMaximize();
    break;
  case OP_ICONIFY:
    iconify();
    break;
  case OP_CLOSE:
    closeClicked();
    break;
  case OP_STICKY:
    buttonSticky->toggle();
    break;
  case OP_OPERATIONS:
    showOperations();
    break;
  case OP_SHADE:
    toggleShade();
    break;
  default:
    break;
  }
}


// set up everything to handle a move (used from Alt-LMB)
void Client::simple_move(){
  grabMouse();
  ensurePointerGrab();
  old_cursor_pos = mapFromGlobal(QCursor::pos());
  dragging_state = dragging_enabled;
  do_resize = 0;
  XChangeActivePointerGrab( qt_xdisplay(),
			    ButtonPressMask | ButtonReleaseMask |
			    PointerMotionMask |
			    EnterWindowMask | LeaveWindowMask,
			    sizeAllCursor.handle(), 0);
  mouseMoveEvent(0);
}

// set up everything to handle a resize (used from Alt-RMB)
void Client::simple_resize(){
  grabMouse();
  ensurePointerGrab();
  // correct positioning
  old_cursor_pos = mapFromGlobal(QCursor::pos());
  dragging_state = dragging_enabled;
  Cursor cursor = sizeAllCursor.handle();
  if (QCursor::pos().x() < geometry.x()+geometry.width()/2){
    if (QCursor::pos().y() < geometry.y()+geometry.height()/2){
      do_resize = 3;
      cursor = top_left_cursor;
    }
    else {
      do_resize = 2;
      cursor = bottom_left_cursor;
    }
  }
  else {
    if (QCursor::pos().y() < geometry.y()+geometry.height()/2){
      do_resize = 4;
      cursor = top_right_cursor;
    }
    else {
      do_resize = 1;
      cursor = bottom_right_cursor;
    }
  }
  XChangeActivePointerGrab( qt_xdisplay(),
			    ButtonPressMask | ButtonReleaseMask |
			    PointerMotionMask |
			    EnterWindowMask | LeaveWindowMask,
			    cursor, 0);
  mouseMoveEvent(0);
}

// indicates wether the user is currently dragging a window around.
bool Client::dragging_is_running(){
  return (dragging_state == dragging_runs);
}

// this slot is connect with a singleshot timer when we are doing
// auto raising (related to the focus follow mouse policy). If
// autoraised_stopped is true it will do nothing.  It will also
// check wether raising would cover a popup menu and avoid it in
// such a case.
void Client::autoRaise(){
  if (autoraised_stopped || do_not_draw){
    autoraised_stopped = false;
    return;
  }

  if (isActive()){
     if (XGrabPointer(qt_xdisplay(), qt_xrootwin(), False,
 		     ButtonPressMask | ButtonReleaseMask |
 		     PointerMotionMask |
 		     EnterWindowMask | LeaveWindowMask,
		      GrabModeAsync, GrabModeAsync, None,
		      None , CurrentTime) == GrabSuccess){
       XUngrabPointer(qt_xdisplay(), CurrentTime);
       XSync(qt_xdisplay(), false);
       manager->raiseClient( this );
     }
     else {
       if ( options.FocusPolicy != CLICK_TO_FOCUS
	    && options.AutoRaise){
	 if (options.AutoRaise)
	   QTimer::singleShot(options.AutoRaiseInterval, this, SLOT(autoRaise()));
	 autoraised_stopped = false;
       }
     }
  }
}

// stops the autoraise timer for this client. If do_raise is TRUE then
// the client might be raised for focus follows mouse policies *if*
// the ClickRaise option is set.
void Client::stopAutoraise(bool do_raise){
  if (!autoraised_stopped
      && isActive()
      && !do_not_draw
      && options.FocusPolicy != CLICK_TO_FOCUS
      &&
      (
       options.CommandWindow1 == MyApp::MouseRaise ||
       options.CommandWindow1 == MyApp::MouseActivateRaiseAndPassClick ||
       options.CommandWindow1 == MyApp::MouseActivateAndRaise
       )
      && do_raise
      && options.ClickRaise){
    manager->raiseClient( this );
    doButtonGrab();
  }
  autoraised_stopped = true;
}

// returns the client itself it is not transient. If it is transient
// it will return the main window recursively.
Client* Client::mainClient(){
  if (trans != None && trans != qt_xrootwin() && trans != window){
    Client* c = manager->getClient(trans);
    if (c)
      return c->mainClient();
  } else if (leader != None && leader != window) {
      Client* c = manager->getClient(leader);
      if (c)
	  return c->mainClient();
  }
  return this;
}

// an X11 window manager handles a lot of size hints, like minimum
// size or fixed increment values. adjustSize() will modify the
// geometry to fullfil the criteria.
void Client::adjustSize(){
  int dx = geometry.width();
  int dy = geometry.height();

  if (dx<1) dx = 1;
  if (dy<1) dy = 1;


  if (size.flags & PResizeInc) {
    dx = geometry_restore.width() -
      ((geometry_restore.width()-dx)/size.width_inc)*size.width_inc;
    dy = geometry_restore.height() -
      ((geometry_restore.height()-dy)/size.height_inc)*size.height_inc;
  }

  switch(getDecoration()){
  case KWM::noDecoration:
    break;
  case KWM::tinyDecoration:
    if (dx < 2*BORDER_THIN+BUTTON_SIZE)
      dx = 2*BORDER_THIN+BUTTON_SIZE+1;
    if (dy < 2*BORDER_THIN+20+1)
      dy = 2*BORDER_THIN+20+1;
    dx -= 2*BORDER_THIN;
    dy -= 2*BORDER_THIN;
    break;
  default:
    if (dx < 2*BORDER+BUTTON_SIZE)
      dx = 2*BORDER+BUTTON_SIZE+1;
    if (dy < 2*BORDER+TITLEBAR_HEIGHT+20+1)
      dy = 2*BORDER+TITLEBAR_HEIGHT+20+1;
    dx -= 2*BORDER;
    dy -= 2*BORDER+TITLEBAR_HEIGHT;
  break;
  }

  if (size.flags & PMaxSize) {
    if (dx > size.max_width)
      dx = size.max_width;
    if (dy > size.max_height)
      dy = size.max_height;
    }

  if (size.flags & PMinSize) {
    if (dx < size.min_width)
      dx = size.min_width;
    if (dy < size.min_height)
      dy = size.min_height;
    }
  else if (size.flags & PBaseSize) {
    if (dx < size.base_width)
      dx = size.base_width;
    if (dy < size.base_height)
      dy = size.base_height;
    }

  switch(getDecoration()){
  case KWM::noDecoration:
    break;
  case KWM::tinyDecoration:
    dx += 2*BORDER_THIN;
    dy += 2*BORDER_THIN;
    break;
  default:
    dx += 2*BORDER;
    dy += 2*BORDER + TITLEBAR_HEIGHT;
    break;
  }

  geometry.setWidth(dx);
  geometry.setHeight(dy);

}


// put a passive grab above the window for some button/modifier combinations.
void Client::doButtonGrab(){
  if (isActive()){
    XUngrabButton(qt_xdisplay(), AnyButton, AnyModifier, window);
    grabButton(AnyButton, window, Mod1Mask);
  }
  else {
    XGrabButton(qt_xdisplay(), AnyButton, AnyModifier, window, True,
		ButtonPressMask, GrabModeSync, GrabModeAsync,
		None, normal_cursor );
  }
  if (!options.Button3Grab)
    XUngrabButton(qt_xdisplay(), Button3, AnyModifier, window);
}


// we called grabMouse() but cannot be sure that we really got the
// grab! IMO this is a qt problem. Anyway this function solves it:
// it will wait until we _really_ have the pointer grab.
void Client::ensurePointerGrab(){
  while (XGrabPointer(qt_xdisplay(), winId(), False,
		      ButtonPressMask | ButtonReleaseMask |
		      PointerMotionMask |
		      EnterWindowMask | LeaveWindowMask,
		      GrabModeAsync, GrabModeAsync, None,
		      None , CurrentTime) != GrabSuccess)
    sleep(1);
}


// returns the id of an kwm command from a given string. (or -1 if
// there is no such command)
int Client::operationFromCommand(const QString &com){
  if (com == "winMove")
    return OP_MOVE;
  else if (com == "winResize")
    return OP_RESIZE;
  else if (com == "winMaximize")
    return OP_MAXIMIZE;
  else if (com == "winRestore")
    return OP_RESTORE;
  else if (com == "winIconify")
    return OP_ICONIFY;
  else if (com == "winClose")
    return OP_CLOSE;
  else if (com == "winSticky")
    return OP_STICKY;
  else if (com == "winShade")
    return OP_SHADE;
  else if (com == "winOperations")
    return OP_OPERATIONS;
  return -1;
}


// shade or unshade this client. "shaded" means that only the
// titlebar is visible.
void Client::toggleShade(){
  if (getDecoration() != KWM::normalDecoration)
    return;
  if (state != NormalState)
    return;

  shaded = !shaded;
  if (isShaded()){
    manager->raiseSoundEvent("Window Shade Up");
    if (isActive())
      manager->noFocus();
    XUnmapWindow(qt_xdisplay(), window);
    unmap_events++;
    manager->setWindowState(this, IconicState);
    state = NormalState;
  }
  else {
    manager->raiseSoundEvent("Window Shade Down");
    XMapWindow(qt_xdisplay(),window);
    manager->setWindowState(this, NormalState);
    if (isActive())
      manager->focusToClient(this);
  }
  manager->sendConfig(this, false);
}
