// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//

#include <stdio.h>
#include <unistd.h>

#include "kpanel.h"
#include "kfm.h"

#include <qapp.h>
#include <qmsgbox.h>
#include <qdrawutl.h>
#include <qpmcache.h>
#include <qmenudta.h>
#include <qkeycode.h>
#include <qbitmap.h>


myPopupMenu::myPopupMenu( QWidget *parent, const char *name )
  : QPopupMenu( parent, name ){
    setMouseTracking(TRUE);
}

int myPopupMenu::height(){
//   if (badSize){
//     QPoint p = pos();
//     move(-1000,-1000);
//     show();
//     hide();
//     move(p);
//   }
  return QPopupMenu::height();
}
int myPopupMenu::width(){
//   if (badSize){
//     QPoint p = pos();
//     move(-1000,-1000);
//     show();
//     hide();
//     move(p);
//   }
  return QPopupMenu::width();
}


myPushButton::myPushButton(QWidget *parent, const char* name)
  : QPushButton( parent, name ){
    setMouseTracking(TRUE);
    setFocusPolicy(NoFocus);
    flat = True;
    never_flat = False;
    flat_means_down = False;
    draw_down = False;
    check_rect_for_leave = FALSE;
    last_button = 0;
}

myPushButton::~myPushButton () {
  if (most_recent_pressed == this)
    most_recent_pressed = NULL;
}

myPushButton* myPushButton::most_recent_pressed = NULL;

void myPushButton::enterEvent( QEvent * ){
  if (!flat)
    return;
  flat = False;
  if (check_rect_for_leave)
    XUnmapSubwindows(qt_xdisplay(), winId());
  if (!never_flat){
    repaint();
  }
}

void myPushButton::leaveEvent( QEvent * ){
//    if (check_rect_for_leave
//        && rect().contains(mapFromGlobal(QCursor::pos()), TRUE))
//      return;
  if (check_rect_for_leave)
    XMapSubwindows(qt_xdisplay(), winId());
  if (flat)
    return;
  flat = True;
  if (!never_flat)
    repaint();
}

void myPushButton::paint(QPainter *painter){
  draw_down = (isDown() || (isOn()) && (never_flat || !flat));
  if (flat_means_down && !never_flat && flat)
    draw_down = TRUE;
	
  drawButtonLabel(painter);
  
  if (draw_down ) {
    if ( style() == WindowsStyle )
      qDrawWinButton( painter, 0, 0, width(), 
		      height(), colorGroup(), TRUE );
    else        
	  qDrawShadePanel( painter, 0, 0, width(), 
	  	       height(), colorGroup(), TRUE, 2, 0L );
  }
  else if (!flat || never_flat) {
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

}

void myPushButton::drawButtonLabel(QPainter *painter){
  if ( pixmap() ) {
    int dx = ( width() - pixmap()->width() ) / 2;
    int dy = ( height() - pixmap()->height() ) / 2;
    if ( draw_down && style() == WindowsStyle ) {
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
    most_recent_pressed = this;
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

myTaskButton* myTaskButton::active = NULL;

myTaskButton::myTaskButton(QWidget *parent, const char* name)
  :myPushButton(parent, name){
    win = None;
    virtual_desktop = 1;
    never_flat = True;
    flat_means_down = True;
}

myTaskButton::~myTaskButton () {
  if (active == this)
    active = NULL;
}

void myTaskButton::setActive(bool value){
  if (value){
    myTaskButton* tmp = active;
    active = this;
    if (tmp && tmp != this)
      tmp->setActive(FALSE);
    never_flat = FALSE;
    flat = !(rect().contains(mapFromGlobal(QCursor::pos()), TRUE));
    repaint();
  }
  else {
    never_flat = TRUE;
    flat = !(rect().contains(mapFromGlobal(QCursor::pos()), TRUE));
    repaint();
  }
}

void myTaskButton::setText(const char* arg){
  if (s != arg){
    s = arg;
    QToolTip::add(this, s);
    repaint();
  }
}

void myTaskButton::drawButtonLabel( QPainter *painter ){
  if (this == active){
    if (QApplication::style() == WindowsStyle)
      painter->fillRect(rect(), QBrush( white, Dense4Pattern ));
    else
      painter->fillRect(rect(), QBrush( colorGroup().mid(), Dense4Pattern ));
  }

  if ( pixmap() ) {
    int dx = ( 32 - pixmap()->width() ) / 2;
    int dy = ( height() - pixmap()->height() ) / 2;
    if ( draw_down && style() == WindowsStyle ) {
      dx++;
      dy++;
    }
    painter->drawPixmap( dx, dy, *pixmap() );
  }
  if (!s.isNull()){
    if ( draw_down && style() == WindowsStyle ) 
      painter->drawText( 33, 1, width()-31, height()+1, AlignLeft|AlignVCenter, s);
    else
      painter->drawText( 32, 0, width()-32, height(), AlignLeft|AlignVCenter, s);
  }
}


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


void kPanel::init_popup(QPopupMenu* popup){
  popup->installEventFilter( this );
  
//   popup->move(-1000, -1000);
//   popup->show();
//   popup->hide();
}


void kPanel::generateWindowlist(QPopupMenu* p){
  Window *w;
  int i = 0;
  int nw = kwmmapp->windows.count();
  p->clear();
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


void kPanel::windowlistActivated(int item){
  if (item>1000){
    KWM::switchToDesktop(item-1000);
  }
  else {
    Window w = callbacklist[item];
    delete [] callbacklist;
    KWM::activate(w);
  }
}



int kPanel::show_popup(QPopupMenu* popup, QWidget* button, bool isTaskButton){
  int xp;
  int yp;

  if (popup == windowlist)
    generateWindowlist(windowlist);

  popup->move(-1000,-1000);
  popup->show();

  if (!isTaskButton){
    if (orientation == vertical){
      yp = button->mapToGlobal(QPoint(0,0)).y();
      if (position == top_left)
	xp = button->mapToGlobal(QPoint(0,0)).x() + button->width();
      else
	xp = button->mapToGlobal(QPoint(0,0)).x() - popup->width();
    }
    else {
      xp = button->mapToGlobal(QPoint(0,0)).x();
      if (position == top_left)
	yp = button->mapToGlobal(QPoint(0,0)).y() + button->height();
      else
	yp = button->mapToGlobal(QPoint(0,0)).y() - popup->height();
    }
  }
  else {
    if (taskbar_position == taskbar_top_left){
      xp = button->mapToGlobal(QPoint(0,0)).x() + button->width();
      yp = button->mapToGlobal(QPoint(0,0)).y();
    }
    else {
      xp = button->mapToGlobal(QPoint(0,0)).x();
      if (taskbar_position == top)
	yp = button->mapToGlobal(QPoint(0,0)).y() + button->height();
      else
	yp = button->mapToGlobal(QPoint(0,0)).y() - popup->height();
    }
  }
  
  popup->move(QPoint(xp, yp));
  int v = popup->exec();
  QEvent ev(Event_Leave);
  QMouseEvent mev (Event_MouseButtonRelease, 
		   QCursor::pos(), LeftButton, LeftButton);
  QApplication::sendEvent(button, &ev);
  QApplication::sendEvent(button, &mev);
  return v;
}

void kPanel::set_button_text(QButton* button, const char* s){

  QToolTip::add(button, s);
  QPixmap pm(button->width(), button->height());
  pm.fill(button->backgroundColor());
  
  QPainter p;
  int a = 2+ (pm.height() - button->fontMetrics().height())/2;
  if (a > button->fontMetrics().height()/2)
    a = button->fontMetrics().height()/2;
  int b = (pm.height() + button->fontMetrics().height())/2-button->fontMetrics().descent();
  if (a<3) a = 3;

  p.begin( &pm ); 
  p.setFont(button->font());
  p.setPen(colorGroup().text());
  p.drawText(a,b,s);
  p.end();
  QBitmap bm(pm.width(), pm.height());
  bm.fill( color0 );
  p.begin(&bm); 
  p.setFont(button->font());
  p.drawText(a,b,s);
  p.end();
  pm.setMask(bm);
  button->setPixmap(pm);
}

QPixmap kPanel::create_arrow_pixmap(QPixmap pm){
  QColorGroup colgrp = QColorGroup( black, backgroundColor(), white, black, black,
  				    black, white );
  QColorGroup colgrp2 = QColorGroup( color1, color0, color1, color1, color1,
 				     color1, color1 );

  QPixmap pm2 = QPixmap(box_width, box_height);
  QBitmap bm = QBitmap(box_width, box_height);
  bool haveTexture = !mBackTexture.isNull();
  
  if ( !haveTexture )
  	{
  	bm.fill(color0);
  	pm2.setMask(bm);
	}

  QPainter p;
  QPainter p2;
  
  if ( !haveTexture )
  	pm2.fill( backgroundColor() );
	  	
  p.begin( &pm2 );
  
  if ( haveTexture )
  	p.drawPixmap( 0, 0, mBackTexture, 0, 0, box_width, box_height );
  else
  	p2.begin( pm2.mask());
	
  if (!pm.isNull()){
    p.drawPixmap( (pm2.width()-pm.width())/2, 
		  (pm2.height()-pm.height())/2, 
		  pm, 0, 0, pm.width(), pm.height());
 	if ( !haveTexture )
		{
    	if (pm.mask())
      		p2.drawPixmap( (pm2.width()-pm.mask()->width())/2, 
		     (pm2.height()-pm.mask()->height())/2, 
		     *(pm.mask()), 0, 0, pm.mask()->width(), pm.mask()->height());
	  	}
  }
  
  if (orientation == horizontal){
    if (position == top_left){
      qDrawArrow( &p, DownArrow, WindowsStyle, FALSE,
		  box_width-6, box_height-5, 0, 0, colgrp);
	  if ( !haveTexture )
      	qDrawArrow( &p2, DownArrow, WindowsStyle, FALSE,
 		  box_width-6, box_height-5, 0, 0, colgrp2);
    }
    else{
      qDrawArrow( &p, UpArrow, WindowsStyle, FALSE,
		  box_width-6, 4, 0, 0, colgrp);
      if ( !haveTexture )
	  	qDrawArrow( &p2, UpArrow, WindowsStyle, FALSE,
		  box_width-6, 4, 0, 0, colgrp2);
    }
  }
  else{ // position == vertical
    if (position == top_left){
      qDrawArrow( &p, RightArrow, WindowsStyle, FALSE,
		  box_width-5, box_height-6, 0, 0, colgrp);
	  if ( !haveTexture )
      	qDrawArrow( &p2, RightArrow, WindowsStyle, FALSE,
 		  box_width-5, box_height-6, 0, 0, colgrp2);
    }
    else{
      qDrawArrow( &p, LeftArrow, WindowsStyle, FALSE,
		  5, box_height-6, 0, 0, colgrp);
      if ( !haveTexture )
	  	qDrawArrow( &p2, LeftArrow, WindowsStyle, FALSE,
		  5, box_height-6, 0, 0, colgrp2);
    }
  }
  p.end();
  if ( !haveTexture )
	  p2.end();
  return pm2;
}


void kPanel::arrow_on_pixmap(QPixmap* pm, ArrowType rt){
  QColorGroup colgrp = QColorGroup( black, backgroundColor(), white, black, black,
				    black, white );
  QColorGroup colgrp2 = QColorGroup( color1, color0, color1, color1, color1,
				     color1, color1 );

  QPainter paint;
  QPainter paint2;
  paint.begin(pm);
  paint2.begin(pm->mask());
  qDrawArrow( &paint, rt, WindowsStyle, FALSE,
	      pm->width()/2, pm->height()/2,
	      0, 0,
	      colgrp);
  qDrawArrow( &paint2, rt, WindowsStyle, FALSE,
	      pm->width()/2, pm->height()/2,
	      0, 0,
	      colgrp2);
  paint.end();
  paint2.end();
}


QPixmap kPanel::load_pixmap(const char* name, bool is_folder){
  QPixmap pm;
  pm.resize(0,0);
  if (!name || name[0]=='\0')
    name = is_folder?"folder.xpm":"exec.xpm";

  if (box_width-10 > 16 || box_height-10 > 16){
    pm =  kapp->getIconLoader()
      ->loadApplicationIcon(name, box_width-4, box_height-4);
    if (pm.isNull())
      pm = kapp->getIconLoader()
	->loadApplicationIcon(is_folder?"folder.xpm":"exec.xpm", 
			      box_width-4, box_height-4);
  }
  else {
    pm =  kapp->getIconLoader()
      ->loadApplicationMiniIcon(name, box_width-4, box_height-4);
    if (pm.isNull())
      pm = kapp->getIconLoader()
	->loadApplicationMiniIcon(is_folder?"folder.xpm":"exec.xpm", 
				  box_width-4, box_height-4);
  }
  return pm;
}


void kPanel::set_label_date(){
  QDateTime d = QDateTime::currentDateTime();
  QString s;
  s.sprintf(" %.2d:%.2d \n %s %d ",
	    d.time().hour(),
	    d.time().minute(),
// 	    d.date().dayName(d.date().dayOfWeek()),
	    d.date().monthName(d.date().month()),
	    d.date().day());
  
  //   s = d.time().toString() + "\n" +d.date().toString();
  
  QToolTip::add(label_date, s);

  if (label_date->fontMetrics().height() * 2 > label_date->height()){
    s.sprintf(" %.2d:%.2d",
	      d.time().hour(),
	      d.time().minute());
  }
  
  label_date->setText(s);
  if ( !mBackTexture.isNull() )
    label_date->setBackgroundPixmap( mBackTexture );
}

void kPanel::add_windowlist(){
  addButtonInternal(NULL, -1, -1, "windowlist");
  write_out_configuration();
}

void kPanel::ask_logout(){
  KWM::logout();
}


void kPanel::call_klock(){
//   QMessageBox::message( "Hi!", "This will invoke the fabulous KLOCK (xlock replacement)");
  execute("klock");
}

 void kPanel::call_help(){
//     QMessageBox::message( "Hi!", "This will invoke the KDE Help system with the\n KDE Startup Screen." );
  execute("kdehelp");
}


void kPanel::showToolTip(QString s){
  if (menu_tool_tips_disabled)
    return;
  if (menu_tool_tips == -1)
    return;

  if (s.isEmpty())
    s = klocale->translate("No comment available");

  if (s.length() > 60){
    QString s2 = "";
    int w = 0;
    unsigned int i = 0;
    for (i=0; i<s.length(); i++){
      if (w > 55 && s.mid(i,1) == " "){
        s2.append("\n");
        w = -1;
      }
      else {
        s2.append(s.mid(i,1));
      }
      w++;
    }
    s = "";
    s.append(s2);
   }


   tipSleepTimer->start(4000, TRUE);
   if (s != ""){
       int t = 1000;
       t = menu_tool_tips;
       if (!info_label_is_sleeping && t > 0)
 	t =100;
       info_label->hide();
       info_label->setText(s);
       info_label->adjustSize();
       info_label->move(QCursor::pos().x()+6, QCursor::pos().y()+16);
       if (t>=0){
 	tipTimer->start(t, TRUE);
       }
   }
   else{
     info_label->hide();
     tipTimer->stop();
     info_label->setText("");
   }

}

void kPanel::addButton(PMenuItem* pmi)
{
  QString s = pmi->getSaveName().copy();
  s.remove(0,1);
  PMenuItem* pmi2 = pmenu->searchItem(s);
  addButtonInternal(pmi2);
  write_out_configuration();
}



void kPanel::delete_button(QWidget* button){
  int i;
  for (i=0; i<nbuttons && entries[i].button!=button; i++);
  if (i<nbuttons && button){
    if (entries[i].drop_zone)
      delete entries[i].drop_zone;
    if (entries[i].swallowed)
      KWM::close(entries[i].swallowed);
    nbuttons--;
    for (;i<nbuttons;i++)
      entries[i] = entries[i+1];
    delete button;
    if (kde_button == button)
      kde_button = NULL;
  }
}

void kPanel::cleanup(){
     int i;
     for (i=0; i<nbuttons; i++)
       if (entries[i].swallowed)
	 KWM::close(entries[i].swallowed);
}

void kPanel::showSystem(){
  if (entries[0].popup->isVisible())
    return;

  if (info_label->isVisible())
    info_label->hide();
  tipTimer->stop();
  if (panelHidden[currentDesktop]){
    miniButtons(1); 
  }
  else {
    show_popup(entries[0].popup, entries[0].button);
  }

//    // generate a Motion event for the qt popup:
//    QCursor::setPos(entries[0].popup->pos() + 
//  		  QPoint(entries[0].popup->width()-5,
//  			 entries[0].popup->height()-5));
//    QCursor::setPos(entries[0].popup->pos() + 
//  		  QPoint(entries[0].popup->width()-6,
//  			 entries[0].popup->height()-5));
//    XSync(qt_xdisplay(), 0);
//    qApp->processEvents();
//    QCursor::setPos(entries[0].popup->pos() +
//  		  QPoint(entries[0].popup->width(),0));
//    QKeyEvent ev(Event_KeyPress, Key_Down, 9, NoButton);
//    qApp->sendEvent(entries[0].popup , &ev);
}


void kPanel::tipTimerDone(){
  info_label->move(QCursor::pos().x()+6, QCursor::pos().y()+16);
  // make sure it is visible
  if (info_label->y() + info_label->height() > QApplication::desktop()->height())
    info_label->move(info_label->x(), 
		     QCursor::pos().y()-16 - info_label->height());
  if (info_label->x() + info_label->width() > QApplication::desktop()->width())
    info_label->move(QCursor::pos().x()-6 - info_label->width(),
		     info_label->y());
  
  if (QString("") != info_label->text()){
    // one might think the check below is stupid since I also
    // check the leave-events, BUT.....
    // when you leave a window during you position another
    // (new) window for kwm, you get NO leave event!
    if (!last_tip_widget || 
	last_tip_widget->rect().contains(last_tip_widget->mapFromGlobal(QCursor::pos()))){
      info_label->raise();
      {
	// a bit crude...
	QColorGroup g( black, QColor(255,255,220),
		       QColor(96,96,96), black, black,
		       black, QColor(255,255,220) );
	info_label->setPalette( QPalette( g, g, g ) );
      }
      info_label->show();
    }
  }
  else 
    info_label->hide();
  info_label_is_sleeping = False;
  tipSleepTimer->start(4000, TRUE);
}

void kPanel::tipSleepTimerDone(){
  info_label->hide();
  info_label_is_sleeping = True;
  tipTimer->stop();
}



void kPanel::showMiniPanel ()
{

  if (!miniPanelHidden) // if allready here do nothing
    return;
  
  miniPanelHidden = false;
  
  int mh = taskbar_height;
  
  int h = QApplication::desktop()->height();
  
  miniPanelFrame = new QFrame(0, 0, 
			      WStyle_Customize|WStyle_NoBorder|WStyle_Tool);
  miniPanelFrame->setMouseTracking( TRUE );
  miniPanelFrame->installEventFilter (this);
  
  miniPanel = new QButtonGroup(miniPanelFrame);
  miniPanel->setFrameStyle(QFrame::Panel| QFrame::Raised);
  miniPanel->setMouseTracking( TRUE );
  miniPanel->installEventFilter (this);
  connect( miniPanel, SIGNAL( pressed( int )), this, SLOT( miniButtons(int)) );
  
  miniSystem = new myPushButton(miniPanel);
  QToolTip::add(miniSystem, "Where do you want to go tomorrow?");
  miniSystem->setFocusPolicy(NoFocus);
  miniSystem->setPixmap(kapp->getIconLoader()
			->loadMiniIcon("go.xpm", mh,mh)); 

  miniSystem->setMouseTracking( TRUE );
  miniSystem->installEventFilter (this);
  
  miniDesk = new myPushButton(miniPanel);
  QToolTip::add(miniDesk, klocale->translate("Windowlist"));
  miniDesk->setFocusPolicy(NoFocus);
  miniDesk->setPixmap(kapp->getIconLoader()
		      ->loadMiniIcon("window_list.xpm", mh, mh));
  miniDesk->setMouseTracking( TRUE );
  miniDesk->installEventFilter (this);
  
  miniPanel->setGeometry (0, 0, 2*mh, mh);
  miniPanel->insert(miniSystem, 1);
  miniPanel->insert(miniDesk, 2);
  
  miniSystem->setGeometry(1, 1, mh-1, mh-2);
  miniDesk->setGeometry(mh, 1, mh-1, mh-2);
  
  int sx=0; int sx1 =0;
  if (position == top_left)
    sx = x() + panel_button->x() + panel_button->width()+1;
  if (position == bottom_right && orientation == horizontal)
    sx1 = x() + panel_button->x() + panel_button->width()+1;
  
  if (taskbar_position == taskbar_top_left)
    miniPanelFrame->setGeometry(sx, 0, 2*mh, mh);
  else if (taskbar_position == hidden)
    miniPanelFrame->setGeometry(sx, 0, 2*mh, mh);
  else if (taskbar_position == bottom)
    miniPanelFrame->setGeometry(sx1, h-mh, 2*mh, mh);
  else if (taskbar_position == top)
    miniPanelFrame->setGeometry(sx, 0, 2*mh, mh);
  
  miniPanelFrame->show();
  miniPanelFrame->raise();
}
 
 void kPanel::hideMiniPanel() {
   if (miniPanelHidden)
     return;              // do nothing if not there
   miniPanelHidden=true;
   miniPanelFrame->hide();
   delete miniPanelFrame;
 }

void kPanel::miniButtons(int i){
  switch (i)
    {
    case 1:
      if (taskbar_position == bottom){
	entries[0].popup->move(-1000,-1000);
	entries[0].popup->show();
	entries[0].popup->move(QPoint(miniPanelFrame->x()+miniSystem->x(), 
				      miniPanelFrame->y()-
				      entries[0].popup->height()));
      }
      else
	entries[0].popup->move(QPoint(miniPanelFrame->x()+miniSystem->x(),
				      miniSystem->y()+
				      miniPanelFrame->height()));
      entries[0].popup->exec();
      break;
      
    case 2:
      generateWindowlist(windowlist);
      if (taskbar_position == bottom){
	windowlist->move(-1000,-1000);
	windowlist->show();
	windowlist->move(QPoint(miniPanelFrame->x()+miniDesk->x(), 
				miniPanelFrame->y()-
				windowlist->height()));
	}
      else
	windowlist->move(QPoint(miniPanelFrame->x()+miniDesk->x(),
				miniDesk->y()+
				miniPanelFrame->height()));
      
      windowlist->exec();
      break;
      
    }
  
  QButton* button = miniPanel->find(i);
  QEvent ev(Event_Leave);
  QMouseEvent mev (Event_MouseButtonRelease, 
		   QCursor::pos(), LeftButton, LeftButton);
  QApplication::sendEvent(button, &ev);
  QApplication::sendEvent(button, &mev);
}
