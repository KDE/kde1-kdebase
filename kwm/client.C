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


bool do_not_draw = FALSE;
myPushButton::myPushButton(QWidget *parent, const char* name)
  : QPushButton( parent, name ){
    setFocusPolicy(NoFocus);
    flat = True;
    last_button = 0;
}

void myPushButton::enterEvent( QEvent * ){
  flat = False;
  if (!do_not_draw)
    repaint(FALSE);
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
		      height(), colorGroup(), TRUE );
    else
      qDrawShadePanel( painter, 0, 0, width(), 
		       height(), colorGroup(), TRUE, 2, 0L );
  }
  else if (!flat ) {
    if ( style() == WindowsStyle )
      qDrawWinButton( painter, 0, 0, width(), height(),
		      colorGroup(), FALSE );
    else {
      qDrawShadePanel( painter, 0, 0, width(), height(), 
		       colorGroup(), FALSE, 2, 0L );
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
    setDown( TRUE );
    repaint( FALSE );
    emit pressed();
  }
}

void myPushButton::mouseReleaseEvent( QMouseEvent *e){
  if ( !isDown() ){
    last_button = 0;
    return;
  }
  bool hit = hitButton( e->pos() );
  setDown( FALSE );
  if ( hit ){
    if ( isToggleButton() )
      setOn( !isOn() );
    repaint( FALSE );
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
      setDown(TRUE);
      repaint(FALSE);
      emit pressed();
    }
  } else {
    if ( isDown() ) {
      setDown(FALSE);
      repaint();
      emit released();
    }
  }
}
  
static QPixmap *shaded_pm_active = NULL;
static QPixmap *shaded_pm_inactive = NULL;
static QColor shaded_pm_active_color;
static QColor shaded_pm_inactive_color;



void animate_size_change(QRect before, QRect after, bool decorated, int o1, int o2){

  float lf,rf,tf,bf;

  if (!options.ResizeAnimation)
    return;

  lf = (after.left() - before.left())/400.0;
  rf = (after.right() - before.right())/400.0;
  tf = (after.top() - before.top())/400.0;
  bf = (after.bottom() - before.bottom())/400.0;


  QRect area = before;
  QRect area2;

  Time ts = manager->timeStamp();
  Time tt = ts;
  float diff;

  XGrabServer(qt_xdisplay());
  
  do {
    if (area2 != area){
      draw_animation_rectangle(area.left(), 
			       area.top(), 
			       area.width(), 
			       area.height(), 
			       decorated, o1, o2);
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
    if (area2 != area){
      draw_animation_rectangle(area2.left(), 
			       area2.top(), 
			       area2.width(), 
			       area2.height(), 
			       decorated, o1, o2);
    }
  } while (tt - ts < 400);
  if (area2 == area){
    draw_animation_rectangle(area2.left(), 
			     area2.top(), 
			     area2.width(), 
			     area2.height(), 
			     decorated, o1, o2);
  }
  XUngrabServer(qt_xdisplay());
}


Client::Client(Window w, QWidget *parent, const char *name_for_qt)
  : QLabel( parent, name_for_qt){
    //, WStyle_Customize | WStyle_NoBorder | WStyle_Tool ){
    window = w;

    reparenting = FALSE;
    backing_store = FALSE;
    state = WithdrawnState;
    maximized = False;
    iconified = False;
    sticky = False;
    cmap = None;
    border = 0;
    ncmapwins = 0;
    cmapwins = 0;
    wmcmaps = 0;
    is_active = FALSE;
    trans = None;
    decoration = 1;
    wants_focus = true;
    
    
    // standard window manager protocols
    Pdeletewindow = FALSE;
    Ptakefocus = FALSE;
    Psaveyourself = FALSE;

    dragging_state = dragging_nope;
    do_resize = 0;

    desktop = manager->currentDesktop();

    setMouseTracking(True);
    
    setFrameStyle( QFrame::WinPanel | QFrame::Raised );

    size.flags = 0;
    generateButtons();

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
    titlestring_offset_delta = 2;
    animation_is_active = FALSE;
    hidden_for_modules = FALSE;
    autoraised_stopped = FALSE;
    
    XGrabButton(qt_xdisplay(), AnyButton, AnyModifier, window, True, 
		ButtonPressMask, GrabModeSync, GrabModeAsync, 
		None, normal_cursor );
    if (!options.Button3Grab)
      XUngrabButton(qt_xdisplay(), Button3, AnyModifier, window); 
    unmap_events = 0;
}  

Client::~Client(){
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


void Client::showClient(){
  XMapWindow(qt_xdisplay(),window);
  show();
}

void Client::hideClient(){
  hide();
  XUnmapWindow(qt_xdisplay(), window);
  unmap_events++;
}







static QPixmap* pm_max = NULL;
static QPixmap* pm_max_down = NULL;
static QPixmap* pm_icon = NULL;
static QPixmap* pm_close = NULL;
static QPixmap* pm_pin_up = NULL;
static QPixmap* pm_pin_down = NULL;
static QPixmap* pm_menu = NULL;

QPixmap* loadIcon(const char* name){
  QPixmap *result = new QPixmap;
  *result = kapp->getIconLoader()->loadIcon(name);
  return result;
}


void Client::generateButtons(){
  int i;

  buttonMaximize = NULL;
  buttonSticky = NULL;
  buttonMenu = NULL;
  
  for (i=0;i<6;i++){
    buttons[i] = getNewButton( options.buttons[i]);
    if (getDecoration() == 1 && !trans){
      if (buttons[i])
	buttons[i]->show();
    }
    else {
      if (buttons[i] && (getDecoration() != 1 || i>0 || buttons[i] != buttonMenu))
	buttons[i]->hide();
    }
  }
  if (!buttonMaximize){
    buttonMaximize = new myPushButton;
    buttonMaximize->setToggleButton(TRUE);
    buttonMaximize->hide();
    connect( buttonMaximize, SIGNAL(toggled(bool)), SLOT(maximizeToggled(bool)));
  }
  if (!buttonSticky){
    buttonSticky = new myPushButton;
    buttonSticky->setToggleButton(TRUE);
    buttonSticky->hide();
    connect( buttonSticky, SIGNAL(toggled(bool)), SLOT(stickyToggled(bool)));
  }
}

void Client::layoutButtons(){
  int trX = 0;
  int trY = 0;
  int trW = 0;
  int trH = 0;
  
  if (getDecoration() != 1){
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
      trX += BUTTON_SIZE;
      trW -= BUTTON_SIZE;
      buttons[0]->setGeometry(BORDER, 
			      button_y,
			      BUTTON_SIZE, BUTTON_SIZE);
    }
    if (!trans){
      if( buttons[1] ){
	trX += BUTTON_SIZE;
	trW -= BUTTON_SIZE;
	buttons[1]->setGeometry(BORDER + BUTTON_SIZE, 
				button_y,
				BUTTON_SIZE, BUTTON_SIZE);
      }
      if( buttons[2] ){
	trX += BUTTON_SIZE;
	trW -= BUTTON_SIZE;
	buttons[2]->setGeometry(BORDER + 2*BUTTON_SIZE, 
				button_y,
				BUTTON_SIZE, BUTTON_SIZE);
      }
      
      if( buttons[3] ){
	trW -= BUTTON_SIZE;
	buttons[3]->setGeometry(trX + trW,
				button_y,
				BUTTON_SIZE, BUTTON_SIZE);
      }
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
  }   
  
  title_rect.setRect(trX,trY,trW,trH);
}

void Client::reconfigure(){
   int i;
   for (i=0;i<6;i++){
     if (buttons[i]){
       buttons[i]->hide();
       delete buttons[i];
       buttons[i] = NULL;
     }
   }
   generateButtons();
   layoutButtons();
   repaint();
   if (options.Button3Grab)
     XGrabButton(qt_xdisplay(), Button3, AnyModifier, window, True, 
		 ButtonPressMask, GrabModeSync, GrabModeAsync, 
		 None, normal_cursor );
   else
     XUngrabButton(qt_xdisplay(), Button3, AnyModifier, window); 
}

void Client::animateTitlebar(){
  if (animation_is_active)
    paintState(TRUE);
}


void Client::mousePressEvent( QMouseEvent *ev ){

  old_cursor_pos = ev->pos();
  
  if (ev->button() == RightButton){
    if (isActive() || !wantsFocus()){
      generateOperations();
      stopAutoraise();
      myapp->operations->popup(QCursor::pos());
    }
    else {
      if (!isActive())
	manager->raiseSoundEvent("Window Activate");
      manager->activateClient(this);
    }
    return;  
  }

  if ((ev->state() & AltButton) != AltButton){
    if (!isActive())
      manager->raiseSoundEvent("Window Activate");
    manager->activateClient( this );
    if (ev->button() == LeftButton)
      manager->raiseClient( this );
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
    if ((ev->state() & AltButton) == AltButton){
      do_resize = 0; // Alt-LMB does only move! (mwm-like)
    }
    dragging_state = dragging_smooth_wait;
    // set the current cursor
    XChangeActivePointerGrab( qt_xdisplay(), 
			      ButtonPressMask | ButtonReleaseMask |
			      PointerMotionMask |
			      EnterWindowMask | LeaveWindowMask,
			      current_cursor, 0);
  }

}  

void Client::mouseReleaseEvent( QMouseEvent* ev){
  if (ev->button() == MidButton && 
      !(ev->state() & AltButton))
    manager->lowerClient(this);

  dragging_state = dragging_nope;
  releaseMouse();
}


void Client::mouseDoubleClickEvent( QMouseEvent* ev){
  if (ev->button() != LeftButton)
    return; 
  if (ev->pos().x() >= title_rect.x() && ev->pos().x() <= title_rect.x()+title_rect.width() &&
      ev->pos().y() >= title_rect.y() && ev->pos().y() <= title_rect.y()+title_rect.height()){
    if (!isMaximized())
      maximize( options.MaximizeOnlyVertically?1:0);
    else
      unMaximize();
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
      if (do_resize == 0)
	grabMouse(sizeAllCursor);
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
  // this is ugly but I see no other way.... 
  QMouseEvent me( Event_MouseButtonPress,
		  mapFromGlobal(QCursor::pos()),
		  0, 0);
  mouseMoveEvent(&me);
  
}

void Client::leaveEvent( QEvent * ){
  if (getDecoration())
    set_x_cursor(normal_cursor);
}

void Client::paintEvent( QPaintEvent* ev ){
  QLabel::paintEvent(ev);
  QPainter p;
  p.begin(this);
  if (!options.ShapeMode || getDecoration() != 1){
    qDrawShadeRect( &p, width()-20, height()-20, 20, 20, colorGroup(), False);
  }
  else {
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
	p.drawPixmap(x,BORDER-options.shapePixmapTop->height()-1,
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
	p.drawPixmap(x,height()-BORDER,pm,
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

void Client::resizeEvent( QResizeEvent * ){
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
    
  if (options.ShapeMode && getDecoration() == 1){	

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
	p.drawPixmap(x,height()-BORDER,pm,
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
    XShapeCombineMask( qt_xdisplay(), winId(), 
		       ShapeBounding, 0, 0, shapemask.handle(), 
		       ShapeSet );
  }
  
}  

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
    return NULL;
  
  if (buttonFunction == MAXIMIZE){
    if (fixedSize())
      return NULL;
  }
  

  myPushButton *button = new myPushButton(this);
  switch(buttonFunction){
  case MAXIMIZE: 
    button->setPixmap(*pm_max);
    button->setToggleButton(TRUE);
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
    button->setToggleButton(TRUE);
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
    break;
  default:
    fprintf(stderr,"Bad Button Function %d\n", buttonFunction);
    break;
  } 
    
  return button;
  
}  

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
      repaint();
    XChangeProperty(qt_xdisplay(), window, a, XA_STRING, 8,
		    PropModeReplace, (unsigned char *)(label.data()), 
		    label.length()+1);
    
  }

}

Client* Client::operation_client = 0;


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
    myapp->operations->setItemEnabled(OP_MAXIMIZE, FALSE);



  myapp->operations->insertItem(KWM::getIconifyString(), 
				OP_ICONIFY);
  if (!wantsFocus() || (trans != None && trans != qt_xrootwin()))
    myapp->operations->setItemEnabled(OP_ICONIFY, FALSE);

  myapp->operations->insertItem(KWM::getMoveString(), 
				OP_MOVE);
  myapp->operations->insertItem(KWM::getResizeString(), 
				OP_RESIZE);

  if (fixedSize())
    myapp->operations->setItemEnabled(OP_RESIZE, FALSE);

  if (isSticky())
    myapp->operations->insertItem(KWM::getUnStickyString(), 
				  OP_STICKY);
  else
    myapp->operations->insertItem(KWM::getStickyString(), 
				  OP_STICKY);
  if (trans != None && trans != qt_xrootwin())
    myapp->operations->setItemEnabled(OP_STICKY, FALSE);

  myapp->desktopMenu->clear();
  int i;
  for (i=1; i <= manager->number_of_desktops; i++){
      QString b = "&";
      b.append(KWM::getDesktopName(i));
      myapp->desktopMenu->insertItem(b, i);
  }
  myapp->desktopMenu->setItemChecked(manager->currentDesktop(), TRUE);

  int dm = myapp->operations->insertItem(KWM::getToDesktopString(), 
					 myapp->desktopMenu);
  if (trans != None && trans != qt_xrootwin())
    myapp->operations->setItemEnabled(dm, FALSE);
  myapp->operations->insertSeparator();
  myapp->operations->insertItem(KWM::getCloseString(), 
				OP_CLOSE);
}

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

bool Client::fixedSize(){
  return (size.flags & PMaxSize ) && (size.flags & PMinSize)
    && (size.max_width <= size.min_width)
    && (size.max_height <= size.min_height);
}




void Client::setactive(bool on){
  static int NumLockMask = 0;
  if (!NumLockMask){
    XModifierKeymap* xmk = XGetModifierMapping(qt_xdisplay());
    int i;
    for (i=0; i<8; i++){
      if (xmk->modifiermap[xmk->max_keypermod * i] == 
	  XKeysymToKeycode(qt_xdisplay(), XK_Num_Lock))
	NumLockMask = (1<<i); 
    }
  }
  if (is_active && !on && myapp->operations->isVisible())
    myapp->operations->hide();

  is_active=on;
  if (!do_not_draw)
    paintState();
  if (is_active){
    if ( options.FocusPolicy == FOCUS_FOLLOW_MOUSE
	 && options.AutoRaise > 0){
      QTimer::singleShot(options.AutoRaise, this, SLOT(autoRaise()));
      autoraised_stopped = FALSE;
    }
  }
}  

void Client::paintState(bool only_label){
  QRect r = title_rect;

  if (r.width() <= 0 || r.height() <= 0 || getDecoration()!=1)
    return;
  int x;

  if (options.TitlebarLook == SHADED){
    if (is_active && shaded_pm_active && shaded_pm_active_color != myapp->activeTitleColor){
      delete shaded_pm_active;
      shaded_pm_active = NULL;
    }
    if (is_active && shaded_pm_inactive && shaded_pm_inactive_color != myapp->inactiveTitleColor){
      delete shaded_pm_inactive;
      shaded_pm_inactive = NULL;
    }
  }

  if (options.TitlebarLook == SHADED
      && 
      (is_active && (!shaded_pm_active)
       || (!is_active && !shaded_pm_inactive))
      ){
    int i,y; 
    QImage image (50, TITLEBAR_HEIGHT, 8, TITLEBAR_HEIGHT);
    QColor col = is_active ? myapp->activeTitleColor : myapp->inactiveTitleColor;
    for ( i=0; i<TITLEBAR_HEIGHT; i++ ){
      image.setColor( i, col.light(100+(i-TITLEBAR_HEIGHT/2)*3).rgb());
    }
    for (y=0; y<TITLEBAR_HEIGHT; y++ ) {
      uchar *pix = image.scanLine(y);
      for (x=0; x<50; x++)
        *pix++ = is_active ? TITLEBAR_HEIGHT-1-y : y;
    }
    if (is_active){
      shaded_pm_active = new QPixmap;
      *shaded_pm_active = image;
      shaded_pm_active_color = myapp->activeTitleColor;
    }
    else{
      shaded_pm_inactive = new QPixmap;
      *shaded_pm_inactive = image;
      shaded_pm_inactive_color = myapp->inactiveTitleColor;
    }
  }
    
  if (only_label){
    titlestring_offset += titlestring_offset_delta;
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
  QPainter p;
  p.begin( this );
    
  QPixmap *pm;
  p.setClipRect(r);
  p.setClipping(True);
  if (options.TitlebarLook == SHADED || options.TitlebarLook == PIXMAP){
    
    if (options.TitlebarLook == SHADED)
      pm = is_active ? shaded_pm_active : shaded_pm_inactive;
    else
      pm = is_active ? options.titlebarPixmapActive: options.titlebarPixmapInactive;    
    if (only_label){
      x = r.x()-(pm->width()-10);
      p.drawPixmap(x, r.y(), *pm);
      x = r.x() + r.width() - 10;
      p.drawPixmap(x, r.y(), *pm);
    }
    else {
      for (x = r.x(); x < r.x() + r.width(); x+=pm->width())
	p.drawPixmap(x, r.y(), *pm);
    }
  }
  else { // TitlebarLook == TITLEBAR_PLAIN
    p.setBackgroundColor( is_active ? myapp->activeTitleColor : myapp->inactiveTitleColor);
    if (only_label){
      p.eraseRect(QRect(r.x(), r.y(), r.x()+10, r.y() + r.height()));
      p.eraseRect(QRect(r.x()+r.width()-10, r.y(), 10, r.height()));
    }
    else {
      p.eraseRect(r);
    }
  }
  p.setClipping(False);
  if (is_active)
    qDrawShadePanel( &p, r, colorGroup(), True );
  
  p.setPen(is_active ? myapp->activeTextColor : myapp->inactiveTextColor);

  if (label){
    QFont fnt("Helvetica", 12, QFont::Bold);
    KApplication::getKApplication()->getCharsets()->setQFont(fnt);
    p.setFont(fnt);
    p.setClipRect(r);
    p.setClipping(True);
    animation_is_active = (p.fontMetrics().width(QString(" ")+label+" ")>r.width());
    if (titlestring_offset_delta > 0){
      if (titlestring_offset > 0
	  && titlestring_offset > r.width() - p.fontMetrics().width(QString(" ")+label+" ")){
	titlestring_offset_delta *= -1;
      }
    }
    else {
      if (titlestring_offset < 0
	  && titlestring_offset + 
	  p.fontMetrics().width(QString(" ")+label+" ") < r.width()){
	titlestring_offset_delta *= -1;
      }
    }

    if (!animation_is_active)
      titlestring_offset = 0;
    if (options.TitleAnimation)
      r.moveBy(titlestring_offset,0);
    p.drawText(r.x(), r.y()+p.fontMetrics().ascent(), 
	       QString(" ")+label+" ");
    p.setClipping(False);
  }
  p.end();
}



void Client::set_x_cursor(Cursor cur){
  if (cur != current_cursor){
    current_cursor = cur;
    XDefineCursor(qt_xdisplay(), winId(), current_cursor);
  }
}



void Client::iconifySlot(){
  iconify(True);
}

void Client::iconify(bool animation){
  if (isIconified())
    return;
  if (manager->current() && 
      this != manager->current()
      && manager->current()->trans == window)
    manager->activateClient(this);
  manager->iconifyTransientOf(this);
  iconified = True;

  if (state == NormalState){
    hideClient();
    if (animation){
      KWM::raiseSoundEvent("Window Iconify");
      animate_size_change(geometry, 
			  QRect(geometry.x()+geometry.width()/2,
				geometry.y()+geometry.height()/2,
				0,0),
			  getDecoration()==1,
			  title_rect.x(), 
			  width()-title_rect.right());
    }
    if (isActive())
      manager->noFocus();
  }
  KWM::setIconify(window, True);
  manager->changedClient(this);
  manager->setWindowState(this, IconicState); 
}

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
      animate_size_change(QRect(geometry.x()+geometry.width()/2,
				geometry.y()+geometry.height()/2,
				0,0), geometry,
			  getDecoration()==1,
			  title_rect.x(), 
			  width()-title_rect.right());
    } 
    showClient();
    manager->setWindowState(this, NormalState);
    manager->raiseClient( this );
  }
  else {
    manager->raiseClient( this );
  }
  manager->unIconifyTransientOf(this);
  if (isOnDesktop(manager->currentDesktop()) && animation){
    manager->activateClient(this);
  }
}


void Client::ontoDesktop(int new_desktop){
  if (new_desktop == desktop || isSticky())
    return;
  if (new_desktop < 1 || new_desktop > manager->number_of_desktops)
    return;

  if (trans != None && trans != qt_xrootwin()){
    // it is forbidden to move transient windows onto other
    // desktops than their parents
    Client* c = manager->getClient(trans);
    if (c && c->desktop != new_desktop)
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
    manager->activateClient( this );
  }

}


void Client::maximize(int mode){
  if (isMaximized())
    return;
  maximized = TRUE;
  geometry_restore = geometry;
  KWM::setGeometryRestore(window, geometry_restore);
  QRect maxRect = KWM::getWindowRegion(desktop);

  switch (mode) {
  case 1:
    geometry.moveTopLeft(QPoint(geometry.x(), maxRect.y()));
    geometry.setHeight(maxRect.height());
    break;
  case 2:
    geometry.moveTopLeft(QPoint(maxRect.x(),geometry.y()));
    geometry.setWidth(maxRect.width());
    break;
  default: //  
    geometry = maxRect;
    break;
  }

  adjustSize();
  if (state == NormalState)
    manager->raiseSoundEvent("Window Maximize");
    animate_size_change(geometry_restore, geometry,
			getDecoration()==1,
			title_rect.x(), 
			width()-title_rect.right());
  KWM::setMaximize(window, maximized);
  if (!buttonMaximize->isOn())
    buttonMaximize->toggle();
  
  manager->sendConfig(this);
  layoutButtons();
}

void Client::unMaximize(){
  if (!isMaximized())
    return;
  maximized = FALSE;
  if (geometry != geometry_restore && state == NormalState)
    manager->raiseSoundEvent("Window UnMaximize");
    animate_size_change(geometry, geometry_restore,
			getDecoration()==1,
			title_rect.x(), 
			width()-title_rect.right());
  geometry = geometry_restore;
  KWM::setMaximize(window, maximized);
  if (buttonMaximize->isOn())
    buttonMaximize->toggle();

  manager->sendConfig(this);
  layoutButtons();
}

void Client::maximizeToggled(bool depressed){
  bool do_not_activate = depressed == isMaximized();
  
  buttonMaximize->setPixmap(buttonMaximize->isOn() ? *pm_max_down : *pm_max);
  buttonMaximize->update();

  if (!do_not_activate){
    if ( depressed){
      switch (buttonMaximize->last_button){
      case MidButton:
	maximize(options.MaximizeOnlyVertically?0:1);
	break;
      case RightButton:
	maximize(2);
	break;
      default: //Leftbutton 
	maximize(options.MaximizeOnlyVertically?1:0);
	break;
      }
    }
    else
      unMaximize();
    
    if (state == NormalState){
      manager->raiseClient( this );
      manager->activateClient( this );
    }
  }
}


void Client::closeClicked(){
  manager->closeClient(this);
} 

void Client::stickyToggled(bool depressed){
  QPixmap* pm;

  if (depressed == isSticky())
    return;

  KWM::setSticky(window, depressed);
  sticky = depressed;

  if (depressed){
    pm = pm_pin_down;
    if (state != NormalState && !isIconified()){
      showClient();  
      manager->setWindowState(this, NormalState);
      manager->raiseClient( this );
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
  static QTime *clicktime = 0;
  if (!isActive()){
    myapp->operations->hide();
    manager->raiseClient(this);
    manager->activateClient(this);
  }
  
  if (clicktime && clicktime->msecsTo(QTime::currentTime())<700){
    // some kind of doubleclick => close
    closeClicked();
  }
  else {
    ignore_release_on_this = buttonMenu;
    showOperations();
  }
  
  if (!clicktime)
    clicktime = new QTime(QTime::currentTime());
  else
    *clicktime = QTime::currentTime();
};


void Client::  handleOperationsPopup(int i){
  switch (i){
  case OP_MOVE:
    manager->raiseClient(this);
    grabMouse();
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
    mouseMoveEvent(NULL);
    break;
  case OP_RESIZE:
    manager->raiseClient(this);
    grabMouse();
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
    resizedrag(this, 1);
    dragging_state = dragging_nope;
    releaseKeyboard();
    releaseMouse();
    break;
  case OP_MAXIMIZE:
    maximize();
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
  default:
    break;
  }
}


void Client::simple_move(){
  grabMouse();
  old_cursor_pos = mapFromGlobal(QCursor::pos());
  dragging_state = dragging_enabled;
  do_resize = 0;
  XChangeActivePointerGrab( qt_xdisplay(), 
			    ButtonPressMask | ButtonReleaseMask |
			    PointerMotionMask |
			    EnterWindowMask | LeaveWindowMask,
			    sizeAllCursor.handle(), 0);
  mouseMoveEvent(NULL);
}

void Client::simple_resize(){
  grabMouse();
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
  mouseMoveEvent(NULL);
}

bool Client::dragging_is_running(){
  return (dragging_state == dragging_runs);
}

void Client::autoRaise(){
  if (autoraised_stopped || do_not_draw)
    return;

  if (isActive()){
     if (XGrabPointer(qt_xdisplay(), qt_xrootwin(), False, 
 		     ButtonPressMask | ButtonReleaseMask |
 		     PointerMotionMask |
 		     EnterWindowMask | LeaveWindowMask,
		      GrabModeAsync, GrabModeAsync, None, 
		      None , CurrentTime) == GrabSuccess){ 
       XUngrabPointer(qt_xdisplay(), CurrentTime);
       XSync(qt_xdisplay(), FALSE);
//        // for FocusFollowMouse: discard all enter/leave events
//        XEvent ev;
//        while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
       manager->raiseClient( this );
     }
     else {
       if ( options.FocusPolicy == FOCUS_FOLLOW_MOUSE
	    && options.AutoRaise > 0){
	 QTimer::singleShot(options.AutoRaise, this, SLOT(autoRaise()));
	 autoraised_stopped = FALSE;
       }
     }
  }
}

void Client::stopAutoraise(){
  if (!autoraised_stopped
      && isActive()
      && !do_not_draw
      && options.FocusPolicy == FOCUS_FOLLOW_MOUSE
      && options.AutoRaise > 0){
    manager->raiseClient( this );
  }
  autoraised_stopped = TRUE;
}

Client* Client::mainClient(){
  if (trans != None && trans != qt_xrootwin() && trans != window){
    Client* c = manager->getClient(trans);
    if (c)
      return c->mainClient();
  }
  return this;
}

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
  case 0:
    break;
  case 2:
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
  case 0:
    break;
  case 2:
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
