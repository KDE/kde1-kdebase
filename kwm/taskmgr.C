// ktask
// Copyright (C) 1997 Matthias Ettrich

#include "taskmgr.moc"
#include <kapp.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qwindefs.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>

#include <kapp.h>
#include <kcharsets.h>
#include <kwm.h>

#include "manager.h"
#include "main.h"

extern Manager* manager;

extern bool do_not_draw;

KListBoxItem_Desktop::KListBoxItem_Desktop(const char *text) :
  QListBoxItem()
{
  _text = text;
}

void KListBoxItem_Desktop::paint(QPainter *p) {
  p->save();
  QFont f=p->font();
  f.setBold(true);
  p->setFont(f);
  QFontMetrics fm = p->fontMetrics();
  p->drawText( 3,  fm.ascent() + fm.leading()/2, _text.data() );
  p->restore();
}

const char *KListBoxItem_Desktop::text() const {
  return _text.data();
}

int KListBoxItem_Desktop::height(const QListBox *lb) const {
  QFont f = lb->font();
  f.setBold(true);
  QFontMetrics fm(f);
  return fm.lineSpacing();
}

int KListBoxItem_Desktop::width(const QListBox *lb) const {
  QFont f = lb->font();
  f.setBold(true);
  QFontMetrics fm(f);
  return fm.boundingRect(_text.data()).width();
}


KListBoxItem_Program::KListBoxItem_Program(QPixmap &pm, const char *text) :
  QListBoxItem()
{
  _pm = pm;
  _text = text;
}

void KListBoxItem_Program::paint(QPainter *p) {
  p->save();
  QFontMetrics fm = p->fontMetrics();
  p->drawPixmap(fm.boundingRect("XXXX").width(), 2, _pm);
  p->drawText( fm.boundingRect("XXXX").width() + 4 + 14 + 4,
	       fm.ascent() + fm.leading()/2 + 2, _text.data()+3 );
  p->restore();
}

const char *KListBoxItem_Program::text() const {
  return _text.data();
}

int KListBoxItem_Program::height(const QListBox *lb) const {
  QFontMetrics fm(lb->font());
  if(fm.lineSpacing() > 18)
    return fm.lineSpacing();
  else
    return 18;
}

int KListBoxItem_Program::width(const QListBox *lb) const {
  QFontMetrics fm(lb->font());
  return fm.boundingRect("XXXX").width() + 4 + 14 +
    4 + fm.boundingRect(_text.data()).width() ;
}


Ktask::Ktask( QWidget *parent, const char *name, WFlags f)
  : QDialog(parent, name, False, f){
    setMouseTracking(True);
    frame = new QFrame( this );
    frame->installEventFilter( this );
    frame->setMouseTracking(true);
    frame->setFrameStyle(QFrame::WinPanel | QFrame:: Raised);
    button = new QPushButton(klocale->translate("Switch to"), this);
    button_logout = new QPushButton(klocale->translate("Logout"), this);
    button_cancel = new QPushButton(klocale->translate("Cancel"), this);
    button->setMouseTracking(True);
    button_logout->setMouseTracking(True);
    button_cancel->setMouseTracking(True);
    button->setDefault( True );
    installEventFilter( this );
    button->installEventFilter( this );
    button_logout->installEventFilter( this );
    button_cancel->installEventFilter( this );
    connect(button, SIGNAL(clicked()), SLOT(buttonSelect()));
    connect(button_logout, SIGNAL(clicked()), SLOT(logout()));
    connect(button_cancel, SIGNAL(clicked()), SLOT(cleanup()));
    label = new QLabel(klocale->translate("Current session"), this);
    label->installEventFilter( this );
    label->setMouseTracking(true);
    label->setAlignment(AlignCenter);

    listbox = new QListBox(this);
    listbox->installEventFilter( this );
    listbox->setMouseTracking(true);
    connect(listbox, SIGNAL(selected(int)), SLOT(listboxSelect(int)));

}


void Ktask::prepareToShow(QStrList* strlist, int active){
  int w = 360;
  int h = 0;

  QFont fnt = kapp->generalFont;
  fnt.setBold(true);
  fnt.setPointSize(14);
  label->setFont(fnt);
  label->adjustSize();

  listbox->clear();

  FILE *f = fopen("/dev/console", "w");

  char *p;
  for(p = strlist->first(); p != 0; p = strlist->next()) {
    Client *c = 0;

    // check if this is a desktop item or a window
    if(strlen(p) < 4 || (c = manager->findClientByLabel(QString(p+3))) == 0) {
      // a desktop
      listbox->insertItem(new KListBoxItem_Desktop(p));
    } else {
      // a window, get miniicon
      QPixmap pm = KWM::miniIcon(c->window, 14, 14);
      listbox->insertItem(new KListBoxItem_Program(pm, p));
    }
  }
  if (f)
    fclose(f);

  listbox->setCurrentItem(active);
  listbox->show();
  listbox->setFocusPolicy( StrongFocus );

  label->move(w/2-label->width()/2, 15);
  h = label->geometry().bottom() + 15;

  listbox->setGeometry(5, h, w-10, 200);
  h = listbox->geometry().bottom() + 10;

  button->setGeometry((w/2-w/8-w/4)/2,
		      h,
		      w/4, 30);
  button_logout->setGeometry(w/2-w/8,
		      h,
		      w/4, 30);
  button_cancel->setGeometry(w/2+w/8 + (w/2-w/8-w/4)/2,
			     h,
			     w/4, 30);
  h = button_cancel->geometry().bottom()+10;

  setGeometry(QApplication::desktop()->width()/2 - w/2,
	      QApplication::desktop()->height()/2 - h/2,
	      w, h);
  frame->setGeometry(0,0, w, h);
}



void Ktask::resizeEvent(QResizeEvent *){
}

void Ktask::SetPointerGrab(QPoint pos){
  QWidget* w = QApplication::widgetAt( pos, true);
  if (!w)
    return;
  if (w->topLevelWidget() == this){
    if (w != mouseGrabber()){
      mouseGrabber()->releaseMouse();
      w->removeEventFilter(this);
      w->installEventFilter(this);
      w->setMouseTracking(true);
      w->grabMouse();
    }
  }
}

bool Ktask::eventFilter( QObject *ob, QEvent * e){
  if (e->type() == Event_MouseButtonPress){
    if (ob->isWidgetType() &&
	!rect().contains(
			 mapFromGlobal(
				       ((QWidget*)ob)->mapToGlobal(
								   ((QMouseEvent*)e)->pos())))){
      cleanup();
    }
  }
  if (e->type() == Event_KeyPress){
    int a = ((QKeyEvent*)e)->ascii();
    if (a == 3 || a == 7 || a == 27)
      cleanup();
  }
  if (e->type() == Event_MouseMove){
    QMouseEvent* mev = (QMouseEvent*) e;
    if (ob->isWidgetType()
	&& !(mev->state() & LeftButton)
	&& !(mev->state() & MidButton)
	&& !(mev->state() & RightButton)
	){
      SetPointerGrab(((QWidget*)ob)->mapToGlobal(mev->pos()));
    }
  }
  return False;
}

bool Ktask::do_grabbing(){
  reactive = manager->current();
  if (reactive)
    reactive->setactive(False);
  XGrabServer(qt_xdisplay());
  do_not_draw = true;
  show();
  XSetInputFocus (qt_xdisplay(), winId(), RevertToParent, CurrentTime);
  if (XGrabKeyboard(qt_xdisplay(), winId(),True,GrabModeAsync,
  		    GrabModeAsync,CurrentTime) != GrabSuccess){
    XUngrabServer(qt_xdisplay());
    return False;
  }
  raise();
  listbox->grabMouse();
  SetPointerGrab(QCursor::pos());
  listbox->setFocus();

  return True;
}

void Ktask::buttonSelect(){
  if (listbox->currentItem() != -1){
    listboxSelect(listbox->currentItem());
  }
  else
    cleanup();
}

void Ktask::logout(){
  cleanup();
  ::logout();
}

void Ktask::listboxSelect(int index){
  QString label = listbox->text(index);
  cleanup();
  emit changeToClient(label);
}

void Ktask::cleanup() {
  XUngrabServer(qt_xdisplay());
  if (mouseGrabber())
    mouseGrabber()->releaseMouse();
  hide();
  do_not_draw = false;
  if (reactive && manager->hasClient( reactive )){
    reactive->setactive(True);
    XSetInputFocus (qt_xdisplay(), reactive->window,
		    RevertToPointerRoot, CurrentTime);
  }
  XSync(qt_xdisplay(), false);
}
