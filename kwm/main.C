/*
 * main.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <qpushbt.h>
#include <qbitmap.h>
#include <qwindefs.h>
#include <qmsgbox.h>
#include <qdialog.h>
#include <qaccel.h>
#include <qstring.h>

#include <qwidget.h>
#include <qmenubar.h>

#include "client.h"
#include "minicli.h"
#include "warning.h"
#include "logout.h"
#include "taskmgr.h"
#include <kwm.h>
#include "version.h"

#include <kapp.h>
#include <kiconloader.h>

#include "main.moc"
#include "manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <X11/X.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/shape.h>
#include <X11/cursorfont.h>

#define INT8 _X11INT8
#define INT32 _X11INT32
#include <X11/Xproto.h>
#undef INT8
#undef INT32


int BORDER;

bool initting;
bool ignore_badwindow;

int handler(Display *d, XErrorEvent *e){
    char msg[80], req[80], number[80];
    bool ignore_badwindow = true; //maybe temporary

    if (initting &&
	(
	 e->request_code == X_ChangeWindowAttributes
	 || e->request_code == X_GrabKey
	 )
	&& (e->error_code == BadAccess)) {
      fprintf(stderr, klocale->translate("kwm: it looks like there's already a window manager running.  kwm not started\n"));
      exit(1);
    }

    if (ignore_badwindow && (e->error_code == BadWindow || e->error_code == BadColor))
      return 0;

    XGetErrorText(d, e->error_code, msg, sizeof(msg));
    sprintf(number, "%d", e->request_code);
    XGetErrorDatabaseText(d, "XRequest", number, "<unknown>", req, sizeof(req));

    fprintf(stderr, "kwm: %s(0x%lx): %s\n", req, e->resourceid, msg);

    if (initting) {
        fprintf(stderr, klocale->translate("kwm: failure during initialisation; aborting\n"));
        exit(1);
    }
    return 0;
}

static Minicli* minicli = 0;
static KWarning* kwarning = 0;
static Klogout* klogout = 0;
static Ktask* ktask = 0;
static QLabel* infoLabel = 0;
static myPushButton* infoIcon = 0;
static QFrame* infoFrame = 0;
static QFrame* infoFrameInner = 0;
static QFrame* infoFrameSeparator = 0;

static Client* top_client_before_button_press = 0;
extern bool do_not_draw;

Manager* manager;
MyApp* myapp = 0;

static int infoBoxVirtualDesktop = 0;
static Client* infoBoxClient = 0;
static QPixmap* pm_unknown = 0;
static void hideInfoBox(){
  infoFrame->hide();
  infoFrame->releaseMouse();
  do_not_draw = False;
}
static void createInfoBox(){
  if (!infoFrame){
    infoFrame = new QFrame(0, 0,
			   WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
    infoFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    infoFrameInner = new QFrame(infoFrame);
    infoFrameInner->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    infoFrameInner->show();
    infoLabel = new QLabel(infoFrame);
    infoLabel->show();
    infoFrameSeparator = new QLabel(infoFrame);
    infoFrameSeparator->setFrameStyle( QFrame::Panel | QFrame::Raised );
    infoIcon = new myPushButton(infoFrame);
  }
}
static QList<Window> *infoBoxWindowsWindow = 0;
static QList<QLabel> *infoBoxWindowsLabel = 0;
static void setInfoBoxText(QString text, Window w){
  if (!infoFrame)
    createInfoBox();
  if (!infoFrame->isVisible()){
    QFont fnt = kapp->generalFont;
    fnt.setBold(true);
    fnt.setPointSize(14);
    infoLabel->setFont(fnt);
  }

  infoLabel->setText("");
  int d = 0;
  if (w != None){
    infoIcon->setPixmap(KWM::icon(w, 48, 48));
    if (infoIcon->pixmap()->isNull()){
      if (!pm_unknown)
	pm_unknown = loadIcon("unknown.xpm");
      infoIcon->setPixmap(*pm_unknown);
      if (infoIcon->pixmap()->isNull())
	w = None; // no icon :-(
    }
  }
  if (w != None){
    d = 54;
    infoLabel->setAlignment(AlignHCenter|AlignVCenter);
  }
  else {
    infoLabel->setAlignment(AlignHCenter|AlignVCenter);
  }
  int nw = 0;
  if (infoBoxWindowsLabel)
    nw = 10+infoBoxWindowsLabel->count() * 22;
  if (nw < infoLabel->fontMetrics().boundingRect(text).width())
    nw = infoLabel->fontMetrics().boundingRect(text).width();

  if (nw > QApplication::desktop()->width()*2/3-12-d){
    infoFrame->setGeometry(0,
			   QApplication::desktop()->height()/2-30,
			   QApplication::desktop()->width(),
			   w!=None?83:60);
    if (nw > QApplication::desktop()->width()-12-d)
      infoLabel->setAlignment(AlignVCenter);
  }
  else
    if (nw > QApplication::desktop()->width()/3-12-d){
      infoFrame->setGeometry(QApplication::desktop()->width()/6,
			     QApplication::desktop()->height()/2-30,
			     QApplication::desktop()->width()*2/3,
			     w!=None?83:60);
      if (infoLabel->fontMetrics().boundingRect(text).width() >
	  QApplication::desktop()->width()-12-d)
	infoLabel->setAlignment(AlignVCenter);
    }
    else
      infoFrame->setGeometry(QApplication::desktop()->width()/3,
			     QApplication::desktop()->height()/2-30,
			     QApplication::desktop()->width()/3,
			     w!=None?83:60);
  infoFrameInner->setGeometry(3, 3, infoFrame->width()-6, infoFrame->height()-6);

  if (w != None){
    infoIcon->setGeometry(6, 6, 48, 48);
    infoIcon->show();
    infoLabel->setGeometry(6+d, 6, infoFrame->width()-d-12,
			   infoFrame->height()-12-20);
    infoFrameSeparator->setGeometry(6, 55, infoFrame->width()-12, 2);
    infoFrameSeparator->show();
    if (infoBoxWindowsLabel){
      QLabel* l;
      for (l=infoBoxWindowsLabel->first();l;l=infoBoxWindowsLabel->next()){
	if (*(infoBoxWindowsWindow->at(infoBoxWindowsLabel->at())) == w){
	  l->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	  l->update();
	}
	else if (l->frameStyle() != QFrame::NoFrame){
	  l->setFrameStyle(QFrame::NoFrame);
	  l->update();
	}
      }
    }
  }
  else {
    infoIcon->hide();
    infoLabel->setGeometry(6, 6, infoFrame->width()-12,
			   infoFrame->height()-12);
    infoFrameSeparator->hide();
  }
  infoLabel->setText(text);

  if (!infoFrame->isVisible()){
    infoFrame->show();
    infoFrame->raise();
    infoFrame->grabMouse();
    do_not_draw = True;
  }
}

void setInfoBoxWindows(Client* c, bool traverse_all = false){
 static QPixmap* pm_menu = 0;
 if (!pm_menu)
   pm_menu = loadIcon("menu.xpm");


  if (!infoFrame)
    createInfoBox();
  if (!infoBoxWindowsWindow){
    infoBoxWindowsWindow = new QList<Window>;
    infoBoxWindowsWindow->setAutoDelete(true);
  }
  if (!infoBoxWindowsLabel)
    infoBoxWindowsLabel = new QList<QLabel>;
  QLabel* l;
  infoBoxWindowsWindow->clear();
  for (l=infoBoxWindowsLabel->first();l;l=infoBoxWindowsLabel->next())
    delete l;
  infoBoxWindowsLabel->clear();
  int x = 7;
  if (c) {
    Client* o = c;
    Window* w = 0;
    QPixmap pm;
    do {
      if (traverse_all ||
	  o->isOnDesktop(manager->currentDesktop())){
	l = new QLabel(infoFrame);
	l->setGeometry(x, 58, 20, 20);
	l->show();
	infoBoxWindowsLabel->append(l);
	w = new Window;
	*w = o->window;
	pm = KWM::miniIcon(*w, 16, 16);
	if (!pm.isNull())
	  l->setPixmap(pm);
	else
	  l->setPixmap(*pm_menu);
	infoBoxWindowsWindow->append(w);
	x += 22;
      }
      o = manager->previousClient(o);
    } while (o != c);
  }
}



kwmOptions options;

void debug_events(const char* s, long int l){
  printf("%s: %ld\n", s,l);
}
void debug_events(const char* s, void* v, long int l = 0){
  if (!l)
    printf("%s: %p\n", s,v);
  else
    printf("%s: %p / %ld\n", s,v,l);
}

QPushButton* ignore_release_on_this = 0;



void sighandler(int) {
    myapp->cleanup();
    QApplication::exit();
}

bool focus_grabbed(){
  return minicli?minicli->isVisible():False || kwarning?kwarning->isVisible():False;
}

void showMinicli(){
  if (!minicli){
    minicli = new Minicli(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
  }
  while (!minicli->do_grabbing());
}

void showWarning(const char* text, bool with_button = true){
  XEvent ev;
  if (!kwarning){
    kwarning = new KWarning(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
  }
  kwarning->setText(text, with_button);
  manager->timeStamp();
  while (XCheckMaskEvent(qt_xdisplay(), EnterWindowMask, &ev));
  myapp->processEvents();
  while (!kwarning->do_grabbing());
}

static void setStringProperty(const char* atomname, const char* value){
  Atom a = XInternAtom(qt_xdisplay(), atomname, False);
  QString str = value;
  XChangeProperty(qt_xdisplay(), qt_xrootwin(), a, XA_STRING, 8,
		  PropModeReplace, (unsigned char *)(str.data()),
		  str.length()+1);
}


// show the modal logout dialog
static void showLogout(){
  manager->raiseSoundEvent("Logout Message");
  if (!klogout){
    klogout = new Klogout(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
    // next is a dirty hack to fix a qt-1.2 bug
    // (should be unnecessary with 1.3)
     unsigned long data[2];
     data[0] = (unsigned long) NormalState;
     data[1] = (unsigned long) None;
     Atom wm_state = XInternAtom(qt_xdisplay(), "WM_STATE", False);
     XChangeProperty(qt_xdisplay(), klogout->winId(), wm_state, wm_state, 32,
 		    PropModeReplace, (unsigned char *)data, 2);
    myapp->connect(klogout, SIGNAL(doLogout()), myapp, SLOT(doLogout()));
    myapp->connect(klogout, SIGNAL(changeToClient(QString)),
		   myapp, SLOT(changeToClient(QString)));
  }
  QStrList* no_session = manager->getNoSessionClients();
  QStrList* pseudo_session = manager->getPseudoSessionClients();
  QStrList* unsaved_data = manager->getUnsavedDataClients();
  klogout->prepareToShow(unsaved_data, pseudo_session, no_session);
  delete no_session;
  delete unsaved_data;
  while (!klogout->do_grabbing());
}

void showTask(){
  if (!ktask){
    ktask = new Ktask(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
    // next is a dirty hack to fix a qt-1.2 bug
    // (should be unnecessary with 1.3)
     unsigned long data[2];
     data[0] = (unsigned long) NormalState;
     data[1] = (unsigned long) None;
     Atom wm_state = XInternAtom(qt_xdisplay(), "WM_STATE", False);
     XChangeProperty(qt_xdisplay(), ktask->winId(), wm_state, wm_state, 32,
 		    PropModeReplace, (unsigned char *)data, 2);
    myapp->connect(ktask, SIGNAL(changeToClient(QString)),
 		   myapp, SLOT(changeToTaskClient(QString)));
  }
  QStrList* liste = new QStrList;
  QString a;
  int i;
  int active = 0;
  for (i=1; i <= manager->number_of_desktops; i++){
    if (i == manager->currentDesktop())
      active = liste->count();
    liste->append(KWM::getDesktopName(i));
    QStrList* clients = manager->getClientsOfDesktop(i);
    for (a=clients->first();!a.isNull();a=clients->next()){
      if (manager->current() &&
	  manager->current()->label == a)
	active = liste->count();
      liste->append(QString("   ")+a);
    }
    delete clients;
  }
  ktask->prepareToShow(liste, active);
  delete liste;
  while (!ktask->do_grabbing());
}

QRect &stringToRect(QString s, QRect *q){
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;

  s.detach(); // a copy for changing

  int a;
  a = s.find('+', 0);
  if (a>0){
    x = s.left(a).toInt();
    s = s.remove(0, a+1);
  }
  a = s.find('+', 0);
  if (a>0){
    y = s.left(a).toInt();
    s = s.remove(0, a+1);
  }
 a = s.find('+', 0);
  if (a>0){
    w = s.left(a).toInt();
    s = s.remove(0, a+1);
  }
  h = s.toInt();
  q->setRect(x, y, w, h);
  return *q;
}

static QPixmap stretchPixmap(QPixmap& src, bool stretchVert){
  QPixmap dest;
  QBitmap *srcMask, *destMask;
  int w, h, w2, h2;
  QPainter p;

  if (src.isNull()) return src;

  w = src.width();
  h = src.height();

  if (stretchVert){
    w2 = w;
    for (h2=h; h2<100; h2=h2<<1)
      ;
  }
  else{
    h2 = h;
    for (w2=w; w2<100; w2=w2<<1)
      ;
  }
  if (w2==w && h2==h) return src;

  dest = src;
  dest.resize(w2, h2);

  p.begin(&dest);
  p.drawTiledPixmap(0, 0, w2, h2, src);
  p.end();

  srcMask = (QBitmap*)src.mask();
  if (srcMask){
    destMask = (QBitmap*)dest.mask();
    p.begin(destMask);
    p.drawTiledPixmap(0, 0, w2, h2, *srcMask);
    p.end();
  }

  debug("stretchPixmap %dx%d to %dx%d", w, h, w2, h2);
  return dest;
}

static QString rectToString(QRect r){
  QString result;
  QString a;
  result.setNum(r.x());
  a.setNum(r.y());
  result.append("+");
  result.append(a);
  a.setNum(r.width());
  result.append("+");
  result.append(a);
  a.setNum(r.height());
  result.append("+");
  result.append(a);
  return result;
}

static void grabKey(KeySym keysym, unsigned int mod){
  static int NumLockMask = 0;
  if (!keysym||!XKeysymToKeycode(qt_xdisplay(), keysym)) return;
  if (!NumLockMask){
    XModifierKeymap* xmk = XGetModifierMapping(qt_xdisplay());
    int i;
    for (i=0; i<8; i++){
      if (xmk->modifiermap[xmk->max_keypermod * i] ==
	  XKeysymToKeycode(qt_xdisplay(), XK_Num_Lock))
	NumLockMask = (1<<i);
    }
  }
  XGrabKey(qt_xdisplay(),
	   XKeysymToKeycode(qt_xdisplay(), keysym), mod,
	   qt_xrootwin(), True,
	   GrabModeAsync, GrabModeSync);
  XGrabKey(qt_xdisplay(),
	   XKeysymToKeycode(qt_xdisplay(), keysym), mod | LockMask,
	   qt_xrootwin(), True,
	   GrabModeAsync, GrabModeSync);
  XGrabKey(qt_xdisplay(),
	   XKeysymToKeycode(qt_xdisplay(), keysym), mod | NumLockMask,
	   qt_xrootwin(), True,
	   GrabModeAsync, GrabModeSync);
  XGrabKey(qt_xdisplay(),
	   XKeysymToKeycode(qt_xdisplay(), keysym), mod | LockMask | NumLockMask,
	   qt_xrootwin(), True,
	   GrabModeAsync, GrabModeSync);


}


// Like manager->activateClient but also raises the window and sends a
// sound event.
void switchActivateClient(Client* c, bool do_not_raise){
  if (!c->geometry.intersects(QApplication::desktop()->rect())){
    // window not visible => place it again.
    manager->doPlacement(c);
    manager->sendConfig(c);
  }
  if (!do_not_raise)
    manager->raiseClient(c);
  if (!CLASSIC_FOCUS){
    manager->activateClient(c);
    manager->raiseSoundEvent("Window Activate");
  }
}


void logout(){
  showWarning(klocale->translate("Preparing session ... "), false);
  XUngrabServer(qt_xdisplay());
  XSync(qt_xdisplay(), false);
  kapp->processEvents();
  manager->processSaveYourself();
  kwarning->release();
  showLogout();
}

MyApp::MyApp(int &argc, char **argv , const QString& rAppName):KApplication(argc, argv, rAppName ){

  DEBUG_EVENTS("RootWindow", qt_xrootwin());

  manager = 0;
  process_events_mode = false;
  systemMenuBarParent = 0;
  systemMenuBar = 0;

  int i;
  bool restore_session = true;
  for (i=1; i<argc; i++){
    if (QString("-version") == argv[i]){
      printf(KWM_VERSION);
      printf("\n");
      printf(klocale->translate("Copyright (C) 1997 Matthias Ettrich (ettrich@kde.org)\n"));
      ::exit(0);
    }
    else if (QString("-nosession") == argv[i]){
      restore_session = false;
    }
    else {
      printf(klocale->translate("Usage: "));
      printf("%s [-version] [-nosession]\n", argv[0]);
      ::exit(1);
    }
  }

  myapp = this;
  initting = true;
  XSetErrorHandler(handler);

  // these should be internationalized!
  setStringProperty("KWM_STRING_MAXIMIZE",   klocale->translate("&Maximize"));
  setStringProperty("KWM_STRING_UNMAXIMIZE", klocale->translate("&Restore"));
  setStringProperty("KWM_STRING_ICONIFY",    klocale->translate("&Iconify"));
  setStringProperty("KWM_STRING_UNICONIFY",  klocale->translate("&DeIconify"));
  setStringProperty("KWM_STRING_STICKY",     klocale->translate("&Sticky"));
  setStringProperty("KWM_STRING_UNSTICKY",   klocale->translate("&UnSticky"));
  setStringProperty("KWM_STRING_MOVE",       klocale->translate("Mo&ve"));
  setStringProperty("KWM_STRING_RESIZE",     klocale->translate("Resi&ze"));
  setStringProperty("KWM_STRING_CLOSE",      klocale->translate("&Close"));
  setStringProperty("KWM_STRING_TODESKTOP",  klocale->translate("&To desktop"));
  setStringProperty("KWM_STRING_ONTOCURRENTDESKTOP",
		    klocale->translate("&Onto current desktop"));

  desktopMenu = new QPopupMenu;
  desktopMenu->installEventFilter(this);
  desktopMenu->setMouseTracking(True);
  desktopMenu->setCheckable(true);

  QObject::connect(desktopMenu, SIGNAL(activated(int)), this, SLOT(handleDesktopPopup(int)));


  operations = new QPopupMenu ();
  CHECK_PTR( operations );
  operations->installEventFilter(this);
  operations->setMouseTracking(True);

  QObject::connect(operations, SIGNAL(activated(int)), this, SLOT(handleOperation(int)));


  grabKey(XK_Tab, Mod1Mask);
  grabKey(XK_Tab, Mod1Mask | ShiftMask);
  grabKey(XK_Tab, ControlMask);
  grabKey(XK_Tab, ControlMask | ShiftMask);

  createKeybindings();

  options.titlebarPixmapActive = new QPixmap;
  options.titlebarPixmapInactive = new QPixmap;

  //CT 07mar98 prepare the placement stuff
  options.interactive_trigger = -1;

  readConfiguration();

  KConfig* config = getKApplication()->getConfig();

  config->setGroup( "Desktops");
  if (!config->hasKey("NumberOfDesktops"))
    config->writeEntry("NumberOfDesktops", 4);
  int number_of_desktops = config->readNumEntry("NumberOfDesktops");
  if (!config->hasKey("Desktop1"))
    config->writeEntry("Desktop1", klocale->translate("One"),
		       true, false, true);
  if (!config->hasKey("Desktop2"))
    config->writeEntry("Desktop2", klocale->translate("Two"),
		       true, false, true);
  if (!config->hasKey("Desktop3"))
    config->writeEntry("Desktop3", klocale->translate("Three"),
		       true, false, true);
  if (!config->hasKey("Desktop4"))
    config->writeEntry("Desktop4", klocale->translate("Four"),
		       true, false, true);
  if (!config->hasKey("Desktop5"))
    config->writeEntry("Desktop5", klocale->translate("Five"),
		       true, false, true);
  if (!config->hasKey("Desktop6"))
    config->writeEntry("Desktop6", klocale->translate("Six"),
		       true, false, true);
  if (!config->hasKey("Desktop7"))
    config->writeEntry("Desktop7", klocale->translate("Seven"),
		       true, false, true);
  if (!config->hasKey("Desktop8"))
    config->writeEntry("Desktop8", klocale->translate("Eight"),
		       true, false, true);

  number_of_desktops = (number_of_desktops/2) * 2;
  if (number_of_desktops < 2)
    number_of_desktops = 2;
  if (number_of_desktops > 32)
    number_of_desktops = 32;

  KWM::setNumberOfDesktops(number_of_desktops);
  for (i=1; i <= 32; i++){
    QString a = "";
    a.setNum(i);
    a.prepend("Desktop");
    QString b = config->readEntry(a,klocale->translate("Unnamed Desktop"));
    b.stripWhiteSpace();
    KWM::setDesktopName(i, b);
    a.append("Region");
    b = config->readEntry(a);
    if (!b.isEmpty()){
      QRect g;
      QRect r = stringToRect(b, &g);
      if (r.isEmpty())
	r = QApplication::desktop()->geometry();
      KWM::setWindowRegion(i, r);
    }
    else {
      KWM::setWindowRegion(i, QApplication::desktop()->geometry());
    }
  }

  KWM::switchToDesktop(1);

  config->sync();

  XSync(qt_xdisplay(), False);

  XGrabServer(qt_xdisplay());
  XSelectInput(qt_xdisplay(), qt_xrootwin(),
	       KeyPressMask |
	       // 		 ButtonPressMask | ButtonReleaseMask |
	       PropertyChangeMask |
	       ColormapChangeMask |
	       SubstructureRedirectMask |
	       SubstructureNotifyMask
	       );

  XSync(qt_xdisplay(), False);

  manager = new Manager;
  connect(manager, SIGNAL(reConfigure()), this, SLOT(reConfigure()));
  XUngrabServer(qt_xdisplay());
  initting = false;

  setupSystemMenuBar();

  if (restore_session)
    restoreSession();
}


void MyApp::setupSystemMenuBar()
{
  if (systemMenuBar)
      return;
  kapp->getConfig()->reparseConfiguration();
  systemMenuBarParent = new QWidget;
  systemMenuBarParent->setGeometry(0, qApp->desktop()->width()+10,100,40);
  systemMenuBar = new KMenuBar(systemMenuBarParent);

  QPopupMenu* file = new QPopupMenu;
  file->insertItem(KWM::getCloseString(), this, SLOT( slotWindowClose() ) );
  fileSystemMenuId = systemMenuBar->insertItem( klocale->translate("File"), file);

  systemMenuBar->setGeometry(0, qApp->desktop()->width()+10,100,40);
  systemMenuBarParent->show();
  systemMenuBarParent->hide();
  if (systemMenuBar->menuBarPos() != KMenuBar::FloatingSystem) {
      delete systemMenuBarParent;
      systemMenuBarParent = 0;
      systemMenuBar = 0;
  }
}

void MyApp::removeSystemMenuBar()
{
    delete systemMenuBar;
    systemMenuBar = 0;
    delete systemMenuBarParent;
    systemMenuBarParent = 0;
}

void MyApp::resetSystemMenuBar()
{
    if (!systemMenuBar)
	return;
    QRect r =  KWM::getWindowRegion(manager->currentDesktop());
    systemMenuBar->setGeometry(r.x(),(r.y()-1)<=0?-2:r.y()-1, r.width(), // check panel top
			       systemMenuBar->heightForWidth(r.width()));
}

void MyApp::raiseSystemMenuBar()
{
    if (!systemMenuBar)
	return;
    systemMenuBar->setItemEnabled( fileSystemMenuId, manager->current() != 0 );
    systemMenuBar->raise();
}


BUTTON_FUNCTIONS getFunctionFromKey(const char* key_arg){
  QString key = key_arg;
  if( key == "Off" )
    return NOFUNC;
  else if( key == "Maximize" )
    return MAXIMIZE;
  else if( key == "Iconify" )
    return ICONIFY;
  else if( key == "Close" )
    return CLOSE;
  else if( key == "Sticky" )
    return STICKY;
  else if( key == "Menu" )
    return MENU;
  else
    return UNDEFINED;
}

const char* default_buttons[6] =
{"Menu", "Sticky", "Off", "Close", "Maximize", "Iconify"};



void MyApp::readConfiguration(){
  KConfig* config;
  QString key;
  int i;

  killTimers();

  config = getKApplication()->getConfig();

  // this belongs in kapp....
  config->setGroup("WM");
  activeTitleBlend = config->readColorEntry( "activeBlend", &black );
  inactiveTitleBlend = config->readColorEntry( "inactiveBlend", &backgroundColor);

  //CT 03Nov1998 - a titlebar font
  tFont=kapp->generalFont;
  tFont.setPointSize(12);
  tFont.setBold(true);
  tFont=config->readFontEntry( "titleFont", &tFont);
  //CT

  config->setGroup( "General" );

  BORDER = 4;

  key = config->readEntry("WindowMoveType");
  if( key == "Transparent")
    options.WindowMoveType = TRANSPARENT;
  else if( key == "Opaque")
    options.WindowMoveType = OPAQUE;
  else{
    config->writeEntry("WindowMoveType","Opaque");
    options.WindowMoveType = OPAQUE;
  }

  key = config->readEntry("WindowResizeType");
  if( key == "Transparent")
    options.WindowResizeType = TRANSPARENT;
  else if( key == "Opaque")
    options.WindowResizeType = OPAQUE;
  else{
    config->writeEntry("WindowResizeType","Opaque");
    options.WindowResizeType = OPAQUE;
  }

  key = config->readEntry("FocusPolicy");
  if( key == "ClickToFocus")
    options.FocusPolicy = CLICK_TO_FOCUS;
  else if( key == "FocusFollowsMouse" || key == "FocusFollowMouse")
    options.FocusPolicy = FOCUS_FOLLOWS_MOUSE;
  else if( key == "ClassicFocusFollowsMouse" || key == "ClassicFocusFollowMouse")
    options.FocusPolicy = CLASSIC_FOCUS_FOLLOWS_MOUSE;
  else if( key == "ClassicSloppyFocus")
    options.FocusPolicy = CLASSIC_SLOPPY_FOCUS;
  else{
    config->writeEntry("FocusPolicy","ClickToFocus");
    options.FocusPolicy = CLICK_TO_FOCUS;
  }

  key = config->readEntry("AltTabMode");
  if( key == "KDE")
    options.AltTabMode = KDE_STYLE;
  else if( key == "CDE")
    options.AltTabMode = CDE_STYLE;
  else{
    config->writeEntry("AltTabMode","KDE");
    options.AltTabMode = KDE_STYLE;
  }

  if (CLASSIC_FOCUS)
    options.AltTabMode = CDE_STYLE;

  key = config->readEntry("TitlebarLook");
  if( key == "shadedHorizontal")
    options.TitlebarLook = H_SHADED;
  else if( key == "shadedVertical")
    options.TitlebarLook = V_SHADED;
  else if( key == "plain")
    options.TitlebarLook = PLAIN;
  else if( key == "pixmap")
    options.TitlebarLook = PIXMAP;
  else{
    config->writeEntry("TitlebarLook", "shadedHorizontal");
    options.TitlebarLook = H_SHADED;
  }

  //CT 04Nov1998 - titlebar align
  key = config->readEntry("TitleAlignment");
  if (key == "left")
    options.alignTitle = AT_LEFT;
  else if (key == "middle")
    options.alignTitle = AT_MIDDLE;
  else if (key == "right")
    options.alignTitle = AT_RIGHT;
  else {
    config->writeEntry("TitleAlignment", "left");
    options.alignTitle = AT_LEFT;
  }

  //CT 02Dec1998 - optional shaded frame on titlebar
  key = config->readEntry("TitleFrameShaded");
  if (key == "no")
    options.framedActiveTitle = false;
  else if (key == "yes")
    options.framedActiveTitle = true;
  else {
    config->writeEntry("TitleFrameShaded","yes");
    options.framedActiveTitle = true;
  }
  //CT

  //CT 02Dec1998 - optional pixmap under the title text
  key = config->readEntry("PixmapUnderTitleText");
  if (key == "no")
    options.PixmapUnderTitleText = false;
  else if (key == "yes")
    options.PixmapUnderTitleText = true;
  else {
    config->writeEntry("PixmapUnderTitleText","yes");
    options.PixmapUnderTitleText = true;
  }
  //CT


  //CT 23Sep1998 - fixed the name of the titlebar pixmaps to become
  //   consistent with the buttons pixmaps definition technique
  if (options.TitlebarLook == PIXMAP) {
    *(options.titlebarPixmapActive) = getIconLoader()
      ->reloadIcon("activetitlebar.xpm");
    *(options.titlebarPixmapInactive) = getIconLoader()
      ->reloadIcon("inactivetitlebar.xpm");

    if (options.titlebarPixmapInactive->size() == QSize(0,0))
      *options.titlebarPixmapInactive = *options.titlebarPixmapActive;

    if (options.titlebarPixmapActive->size() == QSize(0,0))
      options.TitlebarLook = PLAIN;
  }

  //CT 12Jun1998 - variable animation speed from 0 (none!!) to 10 (max)
  if (config->hasKey("ResizeAnimation")) {
    options.ResizeAnimation = config->readNumEntry("ResizeAnimation");
    if( options.ResizeAnimation < 1 ) options.ResizeAnimation = 0;
    if( options.ResizeAnimation > 10 ) options.ResizeAnimation = 10;
  }
  else{
    options.ResizeAnimation = 1;
    config->writeEntry("ResizeAnimation", options.ResizeAnimation);
  }

  key = config->readEntry("ControlTab");
  if( key == "on")
    options.ControlTab = true;
  else if( key == "off")
    options.ControlTab = false;
  else{
    config->writeEntry("ControlTab", "on");
    options.ControlTab = true;
  }

  key = config->readEntry("Button3Grab");
  if( key == "on")
    options.Button3Grab = true;
  else if( key == "off")
    options.Button3Grab = false;
  else{
    config->writeEntry("Button3Grab", "on");
    options.Button3Grab = true;
  }

  key = config->readEntry("MaximizeOnlyVertically");
  if( key == "on")
    options.MaximizeOnlyVertically = true;
  else if( key == "off")
    options.MaximizeOnlyVertically = false;
  else{
    config->writeEntry("MaximizeOnlyVertically", "off");
    options.MaximizeOnlyVertically = false;
  }

  if (config->hasKey("TitleAnimation")){
    options.TitleAnimation = config->readNumEntry("TitleAnimation");
    if (options.TitleAnimation < 0)
      options.TitleAnimation = 0;
  }
  else{
    options.TitleAnimation = 50;
    config->writeEntry("TitleAnimation", options.TitleAnimation);
  }

  if (options.TitleAnimation)
      startTimer(options.TitleAnimation);

  key = config->readEntry("AutoRaise");
  if( key == "on")
    options.AutoRaise = true;
  else if( key == "off")
    options.AutoRaise = false;
  else{
    config->writeEntry("AutoRaise", "off");
    options.AutoRaise = false;
  }

  key = config->readEntry("ClickRaise");
  if( key == "on")
    options.ClickRaise = true;
  else if( key == "off")
    options.ClickRaise = false;
  else{
    config->writeEntry("ClickRaise", "off");
    options.ClickRaise = true;
  }

  if (config->hasKey("AutoRaiseInterval")){
    options.AutoRaiseInterval = config->readNumEntry("AutoRaiseInterval");
    if (options.AutoRaiseInterval < 0)
      options.AutoRaiseInterval = 0;
  }
  else{
    options.AutoRaiseInterval = 0;
    config->writeEntry("AutoRaiseInterval", options.AutoRaiseInterval);
  }

  if (config->hasKey("ElectricBorder")){
    options.ElectricBorder = config->readNumEntry("ElectricBorder");
    if (options.ElectricBorder < -1)
      options.ElectricBorder = -1;
  }
  else{
    options.ElectricBorder = -1;
    config->writeEntry("ElectricBorder", options.ElectricBorder);
  }

  if (config->hasKey("ElectricBorderNumberOfPushes")){
    options.ElectricBorderNumberOfPushes = config->readNumEntry("ElectricBorderNumberOfPushes");
    if (options.ElectricBorderNumberOfPushes < -1)
      options.ElectricBorderNumberOfPushes = 5;
  }
  else{
    options.ElectricBorderNumberOfPushes = 5;
    config->writeEntry("ElectricBorderNumberOfPushes", options.ElectricBorderNumberOfPushes);
  }

  key = config->readEntry("ElectricBorderPointerWarp");
  if( key == "NoWarp")
    options.ElectricBorderPointerWarp = NO_WARP;
  else if( key == "MiddleWarp")
    options.ElectricBorderPointerWarp = MIDDLE_WARP;
  else if( key == "FullWarp")
    options.ElectricBorderPointerWarp = FULL_WARP;
  else{
    config->writeEntry("ElectricBorderPointerWarp", "NoWarp");
    options.ElectricBorderPointerWarp = FULL_WARP;
  }


  if(options.ElectricBorder > -1){
    if( manager)
      manager->createBorderWindows();
  }
  else{
    if( manager)
      manager->destroyBorderWindows();
  }

  key = config->readEntry("ShapeMode");
  if( key == "on")
    options.ShapeMode = true;
  else if( key == "off")
    options.ShapeMode = false;
  else{
    config->writeEntry("ShapeMode", "off");
    options.ShapeMode = false;
  }

  if (options.ShapeMode){

    options.shapePixmapTop = new QPixmap;
    options.shapePixmapLeft = new QPixmap;
    options.shapePixmapBottom = new QPixmap;
    options.shapePixmapRight = new QPixmap;
    options.shapePixmapTopLeft = new QPixmap;
    options.shapePixmapTopRight = new QPixmap;
    options.shapePixmapBottomLeft = new QPixmap;
    options.shapePixmapBottomRight = new QPixmap;


    //CT 23Sep1998 - fixed the name of the shaped borders pixmaps to become
    //   consistent with the buttons pixmaps definition technique
    *(options.shapePixmapTop) = getIconLoader()	->loadIcon("wm_top.xpm");
    *(options.shapePixmapBottom) = getIconLoader()->loadIcon("wm_bottom.xpm");
    *(options.shapePixmapLeft) = getIconLoader()->loadIcon("wm_left.xpm");
    *(options.shapePixmapRight) = getIconLoader()->loadIcon("wm_right.xpm");
    *(options.shapePixmapTopLeft) = getIconLoader()->loadIcon("wm_topleft.xpm");
    *(options.shapePixmapTopRight) = getIconLoader()->loadIcon("wm_topright.xpm");
    *(options.shapePixmapBottomLeft) = getIconLoader()->loadIcon("wm_bottomleft.xpm");
    *(options.shapePixmapBottomRight) = getIconLoader()->loadIcon("wm_bottomright.xpm");


    if (
	options.shapePixmapTop->isNull() ||
	options.shapePixmapLeft->isNull() ||
	options.shapePixmapBottom->isNull() ||
	options.shapePixmapRight->isNull() ||
	options.shapePixmapTopLeft->isNull() ||
	options.shapePixmapTopRight->isNull() ||
	options.shapePixmapBottomLeft->isNull() ||
	options.shapePixmapBottomRight->isNull()){
      fprintf(stderr, klocale->translate("Some pixmaps are not valid: ShapeMode dissabled\n"));
      options.ShapeMode = false;
    }
    else {
      *(options.shapePixmapTop) = stretchPixmap(*(options.shapePixmapTop), false);
      *(options.shapePixmapBottom) = stretchPixmap(*(options.shapePixmapBottom), false);
      *(options.shapePixmapLeft) = stretchPixmap(*(options.shapePixmapLeft), true);
      *(options.shapePixmapRight) = stretchPixmap(*(options.shapePixmapRight), true);

      if (options.shapePixmapTop->height() > BORDER)
	BORDER = options.shapePixmapTop->height();
      if (options.shapePixmapBottom->height() > BORDER)
	BORDER = options.shapePixmapBottom->height();
      if (options.shapePixmapLeft->width() > BORDER)
	BORDER = options.shapePixmapLeft->width();
      if (options.shapePixmapRight->width() > BORDER)
	BORDER = options.shapePixmapRight->width();
    }
  }



  // Windows Placement config --- CT 18jan98 ---
  key = config->readEntry("WindowsPlacement");
  //CT isn't this completely dangerous? what if there are white spaces?
  if( key.left(11) == "interactive") {
    options.Placement = INTERACTIVE_PLACEMENT;
    int comma_pos = key.find(',');
    if (comma_pos < 0)
      options.interactive_trigger = 0;
    else
      options.interactive_trigger =
	key.right(key.length()-comma_pos).toUInt(0);
  }
  else {
    options.interactive_trigger = -1;
    if( key == "smart")
      options.Placement = SMART_PLACEMENT;
    else if( key == "cascade")
      options.Placement = CASCADE_PLACEMENT;
    //CT 30jan98
    else if( key == "random")
      options.Placement = RANDOM_PLACEMENT;
    //CT 07mar98 bad hack. To clean
    else if( key == "manual")
      options.Placement = MANUAL_PLACEMENT;
    else {
      config->writeEntry("WindowsPlacement", "smart");
      options.Placement = SMART_PLACEMENT;
    }
  }

  //CT 17mar98 - magics
  if (config->hasKey("BorderSnapZone")) {
    options.BorderSnapZone = config->readNumEntry("BorderSnapZone");
    if (options.BorderSnapZone < 0) options.BorderSnapZone = 0;
    if (options.BorderSnapZone > 50) options.BorderSnapZone = 50;
  }
  else{
    options.BorderSnapZone = 10;
    config->writeEntry("BorderSnapZone", options.BorderSnapZone);
  }

  if (config->hasKey("WindowSnapZone")) {
    options.WindowSnapZone = config->readNumEntry("WindowSnapZone");
    if (options.WindowSnapZone < 0) options.WindowSnapZone = 0;
    if (options.WindowSnapZone > 50) options.WindowSnapZone = 50;
  }
  else{
    options.WindowSnapZone = 10;
    config->writeEntry("WindowSnapZone", options.WindowSnapZone);
  }
  //CT ---


  options.rstart = qstrdup(config->readEntry("RstartProtocol", "rstart -v"));
  config->writeEntry("RstartProtocol", options.rstart);

  QString s  = config->readEntry("TitlebarDoubleClickCommand", "winShade");
  if (Client::operationFromCommand(s) < 0)
    s = "winShade";
  options.titlebar_doubleclick_command = Client::operationFromCommand(s);
  config->writeEntry("TitlebarDoubleClickCommand", s);

  key = config->readEntry("TraverseAll");
  if( key == "on")
    options.TraverseAll = true;
  else if( key == "off")
    options.TraverseAll = false;
  else{
    config->writeEntry("TraverseAll", "off");
    options.TraverseAll = false;
  }

  config->setGroup( "Gimmick");

  options.GimmickMode = false;
  key = config->readEntry("GimmickMode");
  options.GimmickMode =( key == "on");
  options.GimmickPositionX=config->readNumEntry("GimmickPositionX", 0);
  options.GimmickPositionY=config->readNumEntry("GimmickPositionY", 0);
  options.GimmickOffsetX=config->readNumEntry("GimmickOffsetX", 0);
  options.GimmickOffsetY=config->readNumEntry("GimmickOffsetY", 0);
  if (options.GimmickPositionX<0)options.GimmickPositionX=0;
  if (options.GimmickPositionY<0)options.GimmickPositionY=0;
  if (options.GimmickOffsetX<0)options.GimmickOffsetX=0;
  if (options.GimmickOffsetY<0)options.GimmickOffsetY=0;
  if (options.GimmickPositionX>100)options.GimmickPositionX=100;
  if (options.GimmickPositionY>100)options.GimmickPositionY=100;
  if (options.GimmickOffsetX>100)options.GimmickOffsetX=100;
  if (options.GimmickOffsetY>100)options.GimmickOffsetY=100;
  if (options.GimmickMode){
    options.gimmickPixmap = new QPixmap;
    if (config->hasKey("GimmickPixmap")){
      *(options.gimmickPixmap) = getIconLoader()
	->loadIcon(config->readEntry("GimmickPixmap"));
      if (options.gimmickPixmap->isNull()){
	fprintf(stderr, klocale->translate("Some pixmaps are not valid: GimmickMode dissabled\n"));
	options.GimmickMode = false;
      }
    }
  }

  config->setGroup( "MouseBindings");
  options.CommandActiveTitlebar1 = mouseBinding(config->readEntry("CommandActiveTitlebar1","Raise"));
  options.CommandActiveTitlebar2 = mouseBinding(config->readEntry("CommandActiveTitlebar2","Lower"));
  options.CommandActiveTitlebar3 = mouseBinding(config->readEntry("CommandActiveTitlebar3","Operations menu"));
  options.CommandInactiveTitlebar1 = mouseBinding(config->readEntry("CommandInactiveTitlebar1","Activate and raise"));
  options.CommandInactiveTitlebar2 = mouseBinding(config->readEntry("CommandInactiveTitlebar2","Activate and lower"));
  options.CommandInactiveTitlebar3 = mouseBinding(config->readEntry("CommandInactiveTitlebar3","Activate"));
  options.CommandWindow1 = mouseBinding(config->readEntry("CommandWindow1","Activate, raise and pass click"));
  options.CommandWindow2 = mouseBinding(config->readEntry("CommandWindow2","Activate and pass click"));
  options.CommandWindow3 = mouseBinding(config->readEntry("CommandWindow3","Activate and pass click"));
  options.CommandAll1 = mouseBinding(config->readEntry("CommandAll1","Move"));
  options.CommandAll2 = mouseBinding(config->readEntry("CommandAll2","Toggle raise and lower"));
  options.CommandAll3 = mouseBinding(config->readEntry("CommandAll3","Resize"));
							

  config->setGroup( "Buttons");

  for (i=0; i<6; i++){
    QString s = "Button?";
    s[6] =  (char) ('A'+i);
    options.buttons[i] = getFunctionFromKey(config->readEntry(s));
    if (options.buttons[i] == UNDEFINED){
      config->writeEntry(s,default_buttons[i]);
      options.buttons[i] = getFunctionFromKey(default_buttons[i]);
    }
  }

  config->sync();
}

// returns a mouse binding for a given string
int MyApp::mouseBinding(const char* arg)
{
  QString com = arg;
  if (com == "Raise") return MouseRaise;
  if (com == "Lower") return MouseLower;
  if (com == "Operations menu") return MouseOperationsMenu;
  if (com == "Toggle raise and lower") return MouseToggleRaiseAndLower;
  if (com == "Activate and raise") return MouseActivateAndRaise;
  if (com == "Activate and lower") return MouseActivateAndLower;
  if (com == "Activate") return MouseActivate;
  if (com == "Activate, raise and pass click") return MouseActivateRaiseAndPassClick;
  if (com == "Activate and pass click") return MouseActivateAndPassClick;
  if (com == "Move") return MouseMove;
  if (com == "Resize") return MouseResize;
  if (com == "Nothing") return MouseNothing;
  return MouseNothing;
}


// execute one of the configurable mousebindings. Returns true it the
// binding was bound to something
bool  MyApp::executeMouseBinding(Client* c, int command){
  if (!c)
    return false;
  switch (command){
  case MouseRaise:
    manager->raiseClient(c);
    break;
  case MouseLower:
    manager->lowerClient(c);
    break;
  case MouseOperationsMenu:
    // must be handled by the client itself since it needs the position of the event!
    break;
  case MouseToggleRaiseAndLower:
    if (c == top_client_before_button_press)
      manager->lowerClient(c);
    else
      manager->raiseClient(c);
    break;
  case MouseActivateAndRaise:
    switchActivateClient(c);
    break;
  case MouseActivateAndLower:
    switchActivateClient(c,true);
    manager->lowerClient(c);
    break;
  case MouseActivate:
    switchActivateClient(c,true);
    break;
  case MouseActivateRaiseAndPassClick:
    switchActivateClient(c);
    return false;
    break;
  case MouseActivateAndPassClick:
    switchActivateClient(c,true);
    return false;
    break;
  case MouseMove:
    c->simple_move();
    break;
  case MouseResize:
    c->simple_resize();
    break;
  case MouseNothing:
    return false;
    break;
  default:
    return false;
  }
  return true;
}


void MyApp::writeConfiguration(){
  KConfig* config;
  QString key;
  int i;

  config = getKApplication()->getConfig();
  config->setGroup( "Desktops");

  int n = manager->number_of_desktops;
  if (n<8) n = 8;
  config->writeEntry("NumberOfDesktops", KWM::numberOfDesktops());
  for (i=1; i<=n; i++){
    key.setNum(i);
    key.prepend("Desktop");
    config->writeEntry(key, KWM::getDesktopName(i),
		       true, false, true);
    key.append("Region");
    QRect r = KWM::getWindowRegion(i);
    if (r == QApplication::desktop()->geometry())
      r.setRect(0,0,0,0);
    config->writeEntry(key, rectToString(r));
  }
  config->sync();
}


void MyApp::saveSession(){
  KConfig* config = getKApplication()->getConfig();
  config->setGroup( "Session" );

  QStrList* sl = 0;

  sl = manager->getSessionCommands();
  config->writeEntry("tasks", *sl);
  delete sl;
  sl = manager->getProxyHints();
  config->writeEntry("proxyhints", *sl);
  delete sl;
  sl = manager->getProxyProps();
  config->writeEntry("proxyprops", *sl);
  config->sync();
}

void MyApp::restoreSession(){
  KConfig* config = getKApplication()->getConfig();
  config->setGroup( "Session" );
  QString command;

  QStrList* com = new QStrList;
  QStrList* ph = new QStrList;
  QStrList* pp = new QStrList;
  QStrList* pi = new QStrList;

  config->readListEntry("tasks", *com);
  for (command = com->first(); !command.isNull(); command = com->next())
    execute(command.data());
  delete com;
  config->readListEntry("proxyhints", *ph);
  config->readListEntry("proxyprops", *pp);
  config->readListEntry("proxyignore", *pi);
  manager->setProxyData(ph, pp, pi);
}

void MyApp::cleanup()
{
  delete keys;
  writeConfiguration();
  manager->cleanup();
}

// put the focus on the window with the specified label.  Will switch
// to the appropriate desktop and eventually deiconiy the window
void MyApp::changeToClient(QString label){
  Client* c = manager->findClientByLabel(label);
  if (c){
    if (!c->isOnDesktop(manager->currentDesktop()))
      manager->switchDesktop(c->desktop);
    if (c->isIconified())
      c->unIconify();
    else {
      switchActivateClient(c);
    }
  }
}


 // Same as changeToClient above, but you can also specify names of
 // virtual desktops as label argument. Window names therefore have to
 // start with three blanks. changeToTaskClient is used in the current
 // session manager.
void MyApp::changeToTaskClient(QString label){
  if (label.left(3) == "   "){
    // client
    label.remove(0,3);
    changeToClient(label);
  }
  else {
    // desktop
    int i;
    for (i=1; i <= manager->number_of_desktops; i++){
      if (label == KWM::getDesktopName(i)){
	manager->switchDesktop(i);
	return;
      }
    }
  }
}

// process the logout: save session and exit.
void MyApp::doLogout(){
  manager->raiseSoundEvent("Logout");
  saveSession();
  writeConfiguration();
  manager->cleanup(true);
  exit();
}

// reread the kwm configuration files
void MyApp::reConfigure(){
  getKApplication()->getConfig()->reparseConfiguration();
  readConfiguration();
  manager->readConfiguration();
  keys->readSettings();
}

static void freeKeyboard(bool pass){
    XSync(qt_xdisplay(), False);
    if (!pass)
      XAllowEvents(qt_xdisplay(), AsyncKeyboard, CurrentTime);
    else
      XAllowEvents(qt_xdisplay(), ReplayKeyboard, CurrentTime);
    XSync(qt_xdisplay(), False);
}

static bool tab_grab = False;
static bool control_grab = False;


bool MyApp::handleKeyPress(XKeyEvent key){
  int kc = XKeycodeToKeysym(qt_xdisplay(), key.keycode, 0);
  int km = key.state & (ControlMask | Mod1Mask | ShiftMask);

  if (!control_grab){

    if( (kc == XK_Tab)  &&
	( km == (Mod1Mask | ShiftMask)
	  || km == (Mod1Mask)
	  )){
      freeKeyboard(False);
      if (!tab_grab){
 	if (options.AltTabMode == CDE_STYLE){
 	  // CDE style raise / lower
	  Client* c = manager->topClientOnDesktop();
	  Client* nc = c;
	  if (km & ShiftMask){
	    do {
	      nc = manager->previousStaticClient(nc);
	    } while (nc && nc != c &&
		     (!nc->isOnDesktop(manager->currentDesktop()) ||
		      nc->isIconified()));

	  }
	  else
	    do {
	      nc = manager->nextStaticClient(nc);
	    } while (nc && nc != c &&
		     (!nc->isOnDesktop(manager->currentDesktop()) ||
		      nc->isIconified()));
	  if (c && c != nc)
	    manager->lowerClient(c);
	  if (nc)
	    switchActivateClient(nc);
 	  return True;
 	}
	XGrabKeyboard(qt_xdisplay(),
		      qt_xrootwin(), False,
		      GrabModeAsync, GrabModeAsync,
		      CurrentTime);
	tab_grab = True;
	if (manager->current())
	  setInfoBoxWindows(manager->current(), options.TraverseAll);
	else
	  setInfoBoxWindows(manager->nextClient(0), options.TraverseAll);
	infoBoxClient = manager->current();
      }
      Client* sign = infoBoxClient;
      do {
	if (infoBoxClient != sign && !sign)
	  sign = infoBoxClient;
	if (km & ShiftMask)
	  infoBoxClient = manager->nextClient(infoBoxClient);
	else
	  infoBoxClient = manager->previousClient(infoBoxClient);
      } while (infoBoxClient != sign && infoBoxClient &&
	       !options.TraverseAll &&
	       !infoBoxClient->isOnDesktop(manager->currentDesktop()));

      if (!options.TraverseAll && infoBoxClient
	  && !infoBoxClient->isOnDesktop(manager->currentDesktop()))
	infoBoxClient = 0;
      if (infoBoxClient){
	QString s;
	if (!infoBoxClient->isOnDesktop(manager->currentDesktop())){
	  s = KWM::getDesktopName(infoBoxClient->desktop);
	  s.append(": ");
	}
	if (infoBoxClient->isIconified())
	  setInfoBoxText(s + QString("(")+infoBoxClient->label+")",
			 infoBoxClient->window);
	else
	  setInfoBoxText(s + infoBoxClient->label,
			 infoBoxClient->window);
      }
      else
	setInfoBoxText(klocale->translate("*** No Tasks ***"), None);
      return False;
    }
  }

  if (!tab_grab){


    if( (kc == XK_Tab)  &&
	( km == (ControlMask | ShiftMask)
	  || km == (ControlMask)
	  )){
      if (!options.ControlTab){
	freeKeyboard(True);
	return True;
      }
      if (!control_grab){
	XGrabKeyboard(qt_xdisplay(),
		      qt_xrootwin(), False,
		      GrabModeAsync, GrabModeAsync,
		      CurrentTime);
	control_grab = True;
	setInfoBoxWindows(0);
	infoBoxVirtualDesktop = manager->currentDesktop();
      }
      if (km & ShiftMask){
	infoBoxVirtualDesktop--;
	if (infoBoxVirtualDesktop == 0)
	  infoBoxVirtualDesktop = manager->number_of_desktops;
      }
      else{
	infoBoxVirtualDesktop++;
	if (infoBoxVirtualDesktop > manager->number_of_desktops)
	  infoBoxVirtualDesktop = 1;
      }
      setInfoBoxText(KWM::getDesktopName(infoBoxVirtualDesktop), None);
      return False;
    }
  }

  if (control_grab || tab_grab){
    if (kc == XK_Escape){
      XUngrabKeyboard(qt_xdisplay(), CurrentTime);
      hideInfoBox();
      tab_grab = False;
      control_grab = False;
      return True;
    }
    return False;
  }

  freeKeyboard(False);
  return False;
}

void MyApp::handleKeyRelease(XKeyEvent key){
  int i;
  if (tab_grab){
    XModifierKeymap* xmk = XGetModifierMapping(qt_xdisplay());
    for (i=0; i<xmk->max_keypermod; i++)
      if (xmk->modifiermap[xmk->max_keypermod * Mod1MapIndex + i]
	  == key.keycode){
	XUngrabKeyboard(qt_xdisplay(), CurrentTime);
	hideInfoBox();
	tab_grab = false;
	if (infoBoxClient){
	  if (!infoBoxClient->isOnDesktop(manager->currentDesktop()))
	    manager->switchDesktop(infoBoxClient->desktop);
	
	  if (infoBoxClient->state == NormalState){
	    switchActivateClient(infoBoxClient);
	  }
	  else{ // IconicState
	    infoBoxClient->unIconify();
	  }
	}
      }
  }
  if (control_grab){
    XModifierKeymap* xmk = XGetModifierMapping(qt_xdisplay());
    for (i=0; i<xmk->max_keypermod; i++)
      if (xmk->modifiermap[xmk->max_keypermod * ControlMapIndex + i]
	  == key.keycode){
	XUngrabKeyboard(qt_xdisplay(), CurrentTime);
	hideInfoBox();
	control_grab = False;
	manager->switchDesktop(infoBoxVirtualDesktop);
      }
  }
}



void  MyApp::timerEvent( QTimerEvent * ){
  if (manager->current())
    manager->current()->animateTitlebar();
}

 void MyApp::createKeybindings(){
  keys = new KGlobalAccel();
  #include "kwmbindings.cpp"
  keys->connectItem( "Task manager", this, SLOT( slotTaskManager() ) );
  keys->connectItem( "Kill window mode", this, SLOT( slotKillWindowMode() ) );
  keys->connectItem( "Execute command", this, SLOT( slotExecuteCommand() ) );
  keys->connectItem( "Pop-up window operations menu", this, SLOT( slotWindowOperations() ) );
  keys->connectItem( "Window raise", this, SLOT( slotWindowRaise() ) );
  keys->connectItem( "Window lower", this, SLOT( slotWindowLower() ) );
  keys->connectItem( "Window close", this, SLOT( slotWindowClose() ) );
  keys->connectItem( "Window iconify", this, SLOT( slotWindowIconify() ) );
  keys->connectItem( "Window resize", this, SLOT( slotWindowResize() ));
  keys->connectItem( "Window move", this, SLOT( slotWindowMove() ) );
  keys->connectItem( "Window toggle sticky", this, SLOT( slotWindowToggleSticky() ) );
  keys->connectItem( "Pop-up system menu", this, SLOT( slotKMenu() ) );


  keys->connectItem( "Switch one desktop to the right", this, SLOT( slotSwitchOneDesktopRight() ) );
  keys->connectItem( "Switch one desktop to the left", this, SLOT( slotSwitchOneDesktopLeft() ) );
  keys->connectItem( "Switch one desktop up", this, SLOT( slotSwitchOneDesktopUp() ) );
  keys->connectItem( "Switch one desktop down", this, SLOT( slotSwitchOneDesktopDown() ) );


  keys->connectItem( "Switch to desktop 1", this, SLOT( slotSwitchDesktop1() ));
  keys->connectItem( "Switch to desktop 2", this, SLOT( slotSwitchDesktop2() ));
  keys->connectItem( "Switch to desktop 3", this, SLOT( slotSwitchDesktop3() ));
  keys->connectItem( "Switch to desktop 4", this, SLOT( slotSwitchDesktop4() ));
  keys->connectItem( "Switch to desktop 5", this, SLOT( slotSwitchDesktop5() ));
  keys->connectItem( "Switch to desktop 6", this, SLOT( slotSwitchDesktop6() ));
  keys->connectItem( "Switch to desktop 7", this, SLOT( slotSwitchDesktop7() ));
  keys->connectItem( "Switch to desktop 8", this, SLOT( slotSwitchDesktop8() ));

  keys->readSettings();
}

// keybinding slots

void MyApp::slotTaskManager(){
  showTask();
}
void MyApp::slotKillWindowMode(){
  static Cursor kill_cursor = 0;
  if (!kill_cursor)
       kill_cursor = XCreateFontCursor(qt_xdisplay(), XC_pirate);
     if (XGrabPointer(qt_xdisplay(), qt_xrootwin(), False,
		      ButtonPressMask | ButtonReleaseMask |
		      PointerMotionMask |
		      EnterWindowMask | LeaveWindowMask,
		      GrabModeAsync, GrabModeAsync, None,
		      kill_cursor, CurrentTime) == GrabSuccess){
       XGrabKeyboard(qt_xdisplay(),
		     qt_xrootwin(), False,
		     GrabModeAsync, GrabModeAsync,
		     CurrentTime);
       killSelect();
       XUngrabKeyboard(qt_xdisplay(), CurrentTime);
       XUngrabPointer(qt_xdisplay(), CurrentTime);
     }
}

void MyApp::slotExecuteCommand(){
  showMinicli();
}
void MyApp::slotWindowOperations(){
  if (manager->current())
    manager->current()->showOperations();
}
void MyApp::slotWindowRaise(){
  if (manager->current())
    manager->raiseClient(manager->current());
}
void MyApp::slotWindowLower(){
  if (manager->current())
    manager->lowerClient(manager->current());
}
void MyApp::slotWindowClose(){
  if (minicli && minicli->isVisible()){
    minicli->cleanup();
    return;
  }
  if (klogout && klogout->isVisible()){
    klogout->cleanup();
    return;
  }
  if (ktask && ktask->isVisible()){
    ktask->cleanup();
    return;
  }
  doOperation(OP_CLOSE);
}
void MyApp::slotWindowIconify(){
  doOperation(OP_ICONIFY);
}
void MyApp::slotWindowResize(){
  doOperation(OP_RESIZE);
}
void MyApp::slotWindowMove(){
  doOperation(OP_MOVE);
}
void MyApp::slotWindowToggleSticky(){
  doOperation(OP_STICKY);
}

void MyApp::slotKMenu(){
  KWM::sendKWMCommand("kpanel:system");
}

void MyApp::slotSwitchOneDesktopLeft(){
  manager->moveDesktopInDirection(Manager::Left, 0, FALSE);
}
void MyApp::slotSwitchOneDesktopRight(){
  manager->moveDesktopInDirection(Manager::Right, 0, FALSE);
}
void MyApp::slotSwitchOneDesktopUp(){
  manager->moveDesktopInDirection(Manager::Up, 0, FALSE);
}
void MyApp::slotSwitchOneDesktopDown(){
  manager->moveDesktopInDirection(Manager::Down, 0, FALSE);
}

void MyApp::slotSwitchDesktop1(){
  manager->switchDesktop(1);
}
void MyApp::slotSwitchDesktop2(){
  manager->switchDesktop(2);
}
void MyApp::slotSwitchDesktop3(){
  manager->switchDesktop(3);
}
void MyApp::slotSwitchDesktop4(){
  manager->switchDesktop(4);
}
void MyApp::slotSwitchDesktop5(){
  manager->switchDesktop(5);
}
void MyApp::slotSwitchDesktop6(){
  manager->switchDesktop(6);
}
void MyApp::slotSwitchDesktop7(){
  manager->switchDesktop(7);
}
void MyApp::slotSwitchDesktop8(){
  manager->switchDesktop(8);
}



bool MyApp::eventFilter( QObject *obj, QEvent * ev){

    if (obj == operations || obj == desktopMenu){
      if (ev->type() == Event_MouseButtonRelease){
	// ignore the popup-close in some cases
	if (ignore_release_on_this){
	  QEvent ev(Event_Leave);
	  QMouseEvent mev (Event_MouseButtonRelease,
			   QCursor::pos(), LeftButton, LeftButton);
	  QApplication::sendEvent(ignore_release_on_this, &ev);
	  QApplication::sendEvent(ignore_release_on_this, &mev);
	  if (ignore_release_on_this->rect().
	      contains(ignore_release_on_this
		       ->mapFromGlobal(QCursor::pos()))){
	    ignore_release_on_this = 0;
	    return True;
	  }
	  ignore_release_on_this = 0;
	}
      }
    }
    return False;
}

bool MyApp::buttonPressEventFilter( XEvent * ev)
{

    top_client_before_button_press = manager->topClientOnDesktop();
    Client *c = manager->getClient(ev->xbutton.window);
    if (c) {
	c->stopAutoraise( (ev->xbutton.state & Mod1Mask) != Mod1Mask);

	if (c->isMenuBar()) {
	    XAllowEvents(qt_xdisplay(), ReplayPointer, CurrentTime);
	    XUngrabPointer(qt_xdisplay(), CurrentTime);
	    XSync(qt_xdisplay(), false);
	    return true;
	}
    }
	

    if (c && ev->xbutton.window == c->window){
	bool no_replay = false;
	bool mod1 = (ev->xbutton.state & Mod1Mask) == Mod1Mask;
	if ( mod1){
	    switch (ev->xbutton.button) {
	    case Button1:
		no_replay = executeMouseBinding(c, options.CommandAll1);
		break;	
	    case Button2: 	
		no_replay = executeMouseBinding(c, options.CommandAll2);
		break;
	    case Button3:
		no_replay = executeMouseBinding(c, options.CommandAll3);
		break;
	    }
	}
	else if (!c->isActive()){
	    switch (ev->xbutton.button) {
	    case Button1:
		no_replay = executeMouseBinding(c, options.CommandWindow1);
		break;
	    case Button2:
		no_replay = executeMouseBinding(c, options.CommandWindow2);
		break;
	    case Button3:
		no_replay = executeMouseBinding(c, options.CommandWindow3);
		break;
	    default:
		switchActivateClient(c,true);
		break;
	    }
	}
 	// unfreeze the passive grab which is active currently
	if (no_replay ) {
	    // wait for the button release, do not ungrab.
	  XAllowEvents(qt_xdisplay(), SyncPointer, CurrentTime);
	}
	else {
	  XAllowEvents(qt_xdisplay(), ReplayPointer, CurrentTime);
	  XUngrabPointer(qt_xdisplay(), CurrentTime);
	  XSync(qt_xdisplay(), false);
	}
	return true;
    }
    else {
	c = manager->getClientFromSizegrip(ev->xbutton.window);
	if (c) {
	    QCursor::setPos(c->mapToGlobal(QPoint(c->width()-1, c->height()-1)));
	    c->simple_resize();
	    return true;
	}
    }
    return false;
}


bool MyApp::x11EventFilter( XEvent * ev){
  static Atom kwm_win_frame_geometry = 0;
  if (! kwm_win_frame_geometry)
    kwm_win_frame_geometry = XInternAtom(qt_xdisplay(),"KWM_WIN_FRAME_GEOMETRY",False);

    if (process_events_mode) {
	switch (ev->type) {
	case EnterNotify:
	    return TRUE; //ignore these
	case LeaveNotify:
	    //ignore these if for a client
	    if (manager->getClient(ev->xcrossing.window))
		return TRUE;
	    break;
	case PropertyNotify:
	  {
	    // ignore kwm_win_frame_geometry (we generate lots of the
	    // when doing opaque resizing, so the event stack would
	    // overflow
	    if (ev->xproperty.atom == kwm_win_frame_geometry)
	      return TRUE;
	  }
	case ButtonPress:
	case ButtonRelease:
	case KeyPress:
	case KeyRelease:
	case ClientMessage:
	    // process these later
	    if (events_count == 49) {
		events_count--;
	    }
	    events[events_count++] = *ev;
 	    // debug("events: %d", events_count);
	    return TRUE;
	    break ;
	}
    }


    if (!tab_grab && ! control_grab) { // don't process accelerators in tab or control mode
	if (keys->x11EventFilter(ev))
	    return true;
    }

  // do some KApp client messages.
  // we cannot call the KApplication::x11EventFilter always,
  // since the drag'n'drop protocoll will let us segfault.
  // A Windowmanager isn't really a usual xclient!
  if (ev->type == ClientMessage){
    if (ev->xclient.message_type == KDEChangePalette
	|| ev->xclient.message_type == KDEChangeGeneral){
      KApplication::x11EventFilter(ev);
      // TODO this is only due to missing blending handling in kapp:
      reConfigure();
      manager->repaintAll();
      return true;
    }
  }

  switch (ev->type) {
  case KeyPress:
    DEBUG_EVENTS("KeyPress", ev->xkey.window)
    return handleKeyPress(ev->xkey);
    break;
  case KeyRelease:
    DEBUG_EVENTS("KeyRelease", ev->xkey.window)
    handleKeyRelease(ev->xkey);
    return false;
    break;
  case ButtonPress:
    DEBUG_EVENTS("ButtonPress", ev->xbutton.window)
    return buttonPressEventFilter(ev);
  break;
  case ButtonRelease:
    DEBUG_EVENTS("ButtonRelease", ev->xbutton.window)
    break;
  case CreateNotify:
    DEBUG_EVENTS("CreationNotify", ev->xcreatewindow.window)
    return true;
    break;
  case MapRequest:
    DEBUG_EVENTS("MapRequest", ev->xmaprequest.window)
    manager->mapRequest(&ev->xmaprequest);
    break;
  case ConfigureRequest:
    DEBUG_EVENTS("ConfigureRequest", ev->xconfigurerequest.window)
    manager->configureRequest(&ev->xconfigurerequest);
    break;
  case CirculateRequest:
    DEBUG_EVENTS("CirculateRequest", ev->xcirculaterequest.window)
    manager->circulateRequest(&ev->xcirculaterequest);
    break;
  case UnmapNotify:
    DEBUG_EVENTS("UnmapNotify", ev->xunmap.window)
    DEBUG_EVENTS("UnmapNotifyEvent", ev->xunmap.event)
    manager->unmapNotify(&ev->xunmap);
    if (ev->xunmap.window != ev->xunmap.event){
      return true;
    }
    break;
  case DestroyNotify:
    DEBUG_EVENTS("DestroyNotify", ev->xdestroywindow.window)
    manager->destroyNotify(&ev->xdestroywindow);
    if (ev->xdestroywindow.window != ev->xdestroywindow.event){
      return true;
    }
    break;
  case ClientMessage:
    DEBUG_EVENTS("ClientMessage", ev->xclient.window)
    manager->clientMessage(ev);
    break;
  case ColormapNotify:
    DEBUG_EVENTS("ColormapNotify", ev->xcolormap.window)
    manager->colormapNotify(&ev->xcolormap);
    break;
  case PropertyNotify:
    DEBUG_EVENTS("PropertyNotify", ev->xproperty.window)
    manager->propertyNotify(&ev->xproperty);
    break;
  case SelectionClear:
    break;
  case SelectionNotify:
    break;
  case SelectionRequest:
    break;
  case EnterNotify:
    //DEBUG_EVENTS("EnterNotify", ev->xcrossing.window)
    if (!operations->isVisible())
      manager->enterNotify(&ev->xcrossing);
    break;
  case LeaveNotify:
    //DEBUG_EVENTS("LeaveNotify", ev->xcrossing.window)
    manager->leaveNotify(&ev->xcrossing);
    break;
  case ReparentNotify:
    DEBUG_EVENTS("ReparentNotify", ev->xreparent.window)
    DEBUG_EVENTS("ReparentNotifyParent:", ev->xreparent.parent)
    DEBUG_EVENTS("ReparentNotifyEvent:", ev->xreparent.event)
    manager->reparentNotify(&ev->xreparent);
    return true; //do not confuse Qt with these events...
    break;
  case MotionNotify:
    //DEBUG_EVENTS("MotionNotify", ev->xmotion.window)
    manager->motionNotify(&ev->xmotion);
    break;
  case ConfigureNotify:
    DEBUG_EVENTS("ConfigureNotify", ev->xconfigure.window)
    // this is because Qt cannot handle (usually does not need to)

	// SubstructureNotify events.
    if (ev->xconfigure.window != ev->xconfigure.event){
      return true;
    }
    break;
  case MapNotify:
    DEBUG_EVENTS("MapNotify", ev->xmap.window)
    if (ev->xmap.window != ev->xmap.event){
      return true;
    }
    break;
  case Expose:
    break;
  case FocusIn:
    break;
  case FocusOut:
      {
 	  // handle multi screen displays. The other window manager
 	  // might take the focus away.
  	  Client *c = manager->getClient(ev->xfocus.window);
  	  if (ev->xfocus.mode == NotifyNormal &&
	      ev->xfocus.detail != NotifyPointer &&
	      c && c == manager->current()){
	      c->setactive(False);
  	  }
      }

    break;
  case MappingNotify:
    break;
  default:
    if (manager->shape && ev->type == manager->shape_event)
      manager->shapeNotify((XShapeEvent *)ev);
    break;
  }

  return false;
}


void MyApp::myProcessEvents() {
    static bool inHere = FALSE;
    if (inHere)
	return;
    inHere = TRUE;
    events_count = 0;
    process_events_mode = true;
    processEvents();
    process_events_mode = false;
    int i;
    for (i = events_count - 1; i >= 0; i--) {
	XPutBackEvent(qt_xdisplay(), &events[i]);
    }
    events_count = 0;
    inHere = FALSE;
}

void MyApp::handleOperation(int itemId){
  if (Client::operation_client)
    Client::operation_client->handleOperation(itemId);
}

void MyApp::handleDesktopPopup(int itemId){
  if (Client::operation_client)
    Client::operation_client->ontoDesktop(itemId);
}

void MyApp::doOperation(int itemId){
  if (manager->current())
    manager->current()->handleOperation(itemId);
}



int main( int argc, char ** argv ){
  MyApp a(argc, argv,"kwm");

  if (signal(SIGTERM, sighandler) == SIG_IGN)
    signal(SIGTERM, SIG_IGN);
  if (signal(SIGINT, sighandler) == SIG_IGN)
    signal(SIGINT, SIG_IGN);
  if (signal(SIGHUP, sighandler) == SIG_IGN)
    signal(SIGHUP, SIG_IGN);

  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);
  return a.exec();
}


