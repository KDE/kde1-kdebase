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
    bool ignore_badwindow = TRUE; //maybe temporary

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

static Minicli* minicli = NULL;
static Klogout* klogout = NULL;
static Ktask* ktask = NULL;
static QLabel* infoLabel = NULL;
static myPushButton* infoIcon = NULL;
static QFrame* infoFrame = NULL;
static QFrame* infoFrameInner = NULL;
extern bool do_not_draw;

static int infoBoxVirtualDesktop = 0;
static Client* infoBoxClient = NULL;
static QPixmap* pm_unknown = NULL;
static void hideInfoBox(){
  infoFrame->hide();
  infoFrame->releaseMouse();
  do_not_draw = False;
}
static void setInfoBoxText(QString text, Window w){
  if (!infoFrame){
    infoFrame = new QFrame(0, 0, 
			   WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
    infoFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    infoFrameInner = new QFrame(infoFrame);
    infoFrameInner->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    infoFrameInner->show();
    infoLabel = new QLabel(infoFrame);
    infoLabel->show();
    infoIcon = new myPushButton(infoFrame);
  }
  if (!infoFrame->isVisible())
    infoLabel->setFont(QFont("Helvetica", 14, QFont::Bold));

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
  if (infoLabel->fontMetrics().boundingRect(text).width() > 
      QApplication::desktop()->width()*2/3-12-d){
    infoFrame->setGeometry(0,
			   QApplication::desktop()->height()/2-30,
			   QApplication::desktop()->width(),
			   60);
    if (infoLabel->fontMetrics().boundingRect(text).width() > 
	QApplication::desktop()->width()-12-d)
      infoLabel->setAlignment(AlignVCenter);
  }
  else
    if (infoLabel->fontMetrics().boundingRect(text).width() > 
	QApplication::desktop()->width()/3-12-d){
      infoFrame->setGeometry(QApplication::desktop()->width()/6,
			     QApplication::desktop()->height()/2-30,
			     QApplication::desktop()->width()*2/3,
			     60);
      if (infoLabel->fontMetrics().boundingRect(text).width() > 
	  QApplication::desktop()->width()-12-d)
	infoLabel->setAlignment(AlignVCenter);
    }
    else
      infoFrame->setGeometry(QApplication::desktop()->width()/3,
			     QApplication::desktop()->height()/2-30,
			     QApplication::desktop()->width()/3,
			     60);
  infoFrameInner->setGeometry(3, 3, infoFrame->width()-6, infoFrame->height()-6);
  
  if (w != None){
    infoIcon->setGeometry(6, 6, 48, 48);
    infoIcon->show();
    infoLabel->setGeometry(6+d, 6, infoFrame->width()-d-12, 
			   infoFrame->height()-12);
  }
  else {
    infoIcon->hide();
    infoLabel->setGeometry(6, 6, infoFrame->width()-12, 
			   infoFrame->height()-12);
  }
  infoLabel->setText(text);

  if (!infoFrame->isVisible()){
    infoFrame->show();
    infoFrame->raise();
    infoFrame->grabMouse();
    do_not_draw = True;
  }
}


Manager* manager;
MyApp* myapp = NULL;

kwmOptions options;

QPushButton* ignore_release_on_this = NULL;



void sighandler(int) {
    myapp->cleanup();
    QApplication::exit();
}



bool focus_grabbed(){
  return minicli?minicli->isVisible():False;
}

void show_minicli(){
  if (!minicli){
    minicli = new Minicli(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
  }
  while (!minicli->do_grabbing());
}

static void setStringProperty(const char* atomname, const char* value){
  Atom a = XInternAtom(qt_xdisplay(), atomname, False);
  QString str = value;
  XChangeProperty(qt_xdisplay(), qt_xrootwin(), a, XA_STRING, 8,
		  PropModeReplace, (unsigned char *)(str.data()), 
		  str.length()+1);
}


static void showLogout(){
  KWM::raiseSoundEvent("Logout Message");
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

static void showTask(){
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
  if (!XKeysymToKeycode(qt_xdisplay(), keysym)) return; 
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




MyApp::MyApp(int &argc, char **argv , const QString& rAppName):KApplication(argc, argv, rAppName ){
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
  initting = TRUE;
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
  desktopMenu->setCheckable(TRUE);
  
  QObject::connect(desktopMenu, SIGNAL(activated(int)), this, SLOT(handleDesktopPopup(int)));

  
  operations = new QPopupMenu ();
  CHECK_PTR( operations );
  operations->installEventFilter(this);
  operations->setMouseTracking(True);
  
  QObject::connect(operations, SIGNAL(activated(int)), this, SLOT(handleOperationsPopup(int)));

  
  grabKey(XK_Escape,ControlMask);
  grabKey(XK_Escape,Mod1Mask);
  grabKey(XK_Escape,ControlMask | Mod1Mask);
  grabKey(XK_F2, Mod1Mask);
  grabKey(XK_F3, Mod1Mask);
  grabKey(XK_F4, Mod1Mask);
  grabKey(XK_Tab, Mod1Mask);
  grabKey(XK_Tab, Mod1Mask | ShiftMask);
  grabKey(XK_Tab, ControlMask);
  grabKey(XK_Tab, ControlMask | ShiftMask);

  grabKey(XK_F1, ControlMask);
  grabKey(XK_F2, ControlMask);
  grabKey(XK_F3, ControlMask);
  grabKey(XK_F4, ControlMask);
  grabKey(XK_F5, ControlMask);
  grabKey(XK_F6, ControlMask);
  grabKey(XK_F7, ControlMask);
  grabKey(XK_F8, ControlMask);

  options.titlebarPixmapActive = new QPixmap;
  options.titlebarPixmapInactive = new QPixmap;
  
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
  if (number_of_desktops > 8)
    number_of_desktops = 8;
  
  KWM::setNumberOfDesktops(number_of_desktops);
  for (i=1; i <= 8; i++){
    QString a = "";
    a.setNum(i);
    a.prepend("Desktop"); 
    QString b = config->readEntry(a);
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
  connect(manager, SIGNAL(showLogout()), this, SLOT(showLogout()));
  XUngrabServer(qt_xdisplay()); 
  initting = FALSE;
  if (restore_session)
    restoreSession();
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
  
  config = getKApplication()->getConfig();
  config->setGroup( "General" );

  BORDER = 4;

  key = config->readEntry("WindowMoveType");
  if( key == "Transparent")
    options.WindowMoveType = TRANSPARENT;
  else if( key == "Opaque")
    options.WindowMoveType = OPAQUE;
  else{
    config->writeEntry("WindowMoveType","Transparent");
    options.WindowMoveType = TRANSPARENT;
  }

  key = config->readEntry("FocusPolicy");
  if( key == "ClickToFocus")
    options.FocusPolicy = CLICK_TO_FOCUS;
  else if( key == "FocusFollowMouse")
    options.FocusPolicy = FOCUS_FOLLOW_MOUSE;
  else{
    config->writeEntry("FocusPolicy","ClickToFocus");
    options.FocusPolicy = CLICK_TO_FOCUS;
  }

  key = config->readEntry("TitlebarLook");
  if( key == "shaded")
    options.TitlebarLook = SHADED;
  else if( key == "plain")
    options.TitlebarLook = PLAIN;
  else if( key == "pixmap")
    options.TitlebarLook = PIXMAP;
  else{
    config->writeEntry("TitlebarLook", "shaded");
    options.TitlebarLook = SHADED;
  }

  if (config->hasKey("TitlebarPixmapActive")){
    *(options.titlebarPixmapActive) = getIconLoader()
      ->loadIcon(config->readEntry("TitlebarPixmapActive"));
  }
  if (config->hasKey("TitlebarPixmapInactive")){
    *(options.titlebarPixmapInactive) = getIconLoader()
      ->loadIcon(config->readEntry("TitlebarPixmapInactive"));
  }

  if (options.titlebarPixmapInactive->size() == QSize(0,0))
    *options.titlebarPixmapInactive = *options.titlebarPixmapActive;

  if (options.TitlebarLook == PIXMAP){
    if (options.titlebarPixmapActive->size() == QSize(0,0))
      options.TitlebarLook = PLAIN;
  }

  key = config->readEntry("ResizeAnimation");
  if( key == "on")
    options.ResizeAnimation = true;
  else if( key == "off")
    options.ResizeAnimation = false;
  else{
    config->writeEntry("ResizeAnimation", "on");
    options.ResizeAnimation = true;
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
  else
    killTimers();

  if (config->hasKey("AutoRaise")){
    options.AutoRaise = config->readNumEntry("AutoRaise");
    if (options.AutoRaise < 0)
      options.AutoRaise = 0;
  }
  else{
    options.AutoRaise = 0;
    config->writeEntry("AutoRaise", options.AutoRaise);
  }


  key = config->readEntry("ShapeMode");
  if( key == "on")
    options.ShapeMode = true;
  else if( key == "off")
    options.ShapeMode = false;
  else{
    config->writeEntry("ShapeMode", "on");
    options.ControlTab = true;
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



    if (config->hasKey("ShapePixmapTop")){
      *(options.shapePixmapTop) = getIconLoader()
	->loadIcon(config->readEntry("ShapePixmapTop"));
    }
    if (config->hasKey("ShapePixmapBottom")){
      *(options.shapePixmapBottom) = getIconLoader()
	->loadIcon(config->readEntry("ShapePixmapBottom"));
    }
    if (config->hasKey("ShapePixmapLeft")){
      *(options.shapePixmapLeft) = getIconLoader()
	->loadIcon(config->readEntry("ShapePixmapLeft"));
    }
    if (config->hasKey("ShapePixmapRight")){
      *(options.shapePixmapRight) = getIconLoader()
	->loadIcon(config->readEntry("ShapePixmapRight"));
    }
    if (config->hasKey("ShapePixmapTopLeft")){
      *(options.shapePixmapTopLeft) = getIconLoader()
	->loadIcon(config->readEntry("ShapePixmapTopLeft"));
    }
    if (config->hasKey("ShapePixmapTopRight")){
      *(options.shapePixmapTopRight) = getIconLoader()
	->loadIcon(config->readEntry("ShapePixmapTopRight"));
    }
    if (config->hasKey("ShapePixmapBottomLeft")){
      *(options.shapePixmapBottomLeft) = getIconLoader()
	->loadIcon(config->readEntry("ShapePixmapBottomLeft"));
    }
    if (config->hasKey("ShapePixmapBottomRight")){
      *(options.shapePixmapBottomRight) = getIconLoader()
	->loadIcon(config->readEntry("ShapePixmapBottomRight"));
    }


    
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
  

  options.rstart = config->readEntry("RstartProtocol", "rstart -v");
  config->writeEntry("RstartProtocol", options.rstart);

  config->setGroup( "Buttons");

  for (i=0; i<6; i++){
    QString s = "Button?";
    s.data()[6] =  (char) ('A'+i);
    options.buttons[i] = getFunctionFromKey(config->readEntry(s));
    if (options.buttons[i] == UNDEFINED){
      config->writeEntry(s,default_buttons[i]);
      options.buttons[i] = getFunctionFromKey(default_buttons[i]);
    }
  }
 
  config->sync();
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

  QStrList* sl = NULL;

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

void MyApp::cleanup(){
  writeConfiguration();
  manager->cleanup();
}

void MyApp::changeToClient(QString label){
  Client* c = manager->findClientByLabel(label);
  if (c){
    if (!c->isOnDesktop(manager->currentDesktop()))
      manager->switchDesktop(c->desktop);
    if (c->isIconified())
      c->unIconify();
    else {
      manager->raiseClient(c);
      manager->activateClient(c);
    }
  }
}


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

void MyApp::doLogout(){
  KWM::raiseSoundEvent("Logout");
  saveSession();
  writeConfiguration();
  manager->cleanup();
  exit();
}

void MyApp::reConfigure(){
  getKApplication()->getConfig()->reparseConfiguration();
  readConfiguration();
}

void MyApp::showLogout(){
  ::showLogout();
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
  static Cursor kill_cursor = 0;
  int kc = XKeycodeToKeysym(qt_xdisplay(), key.keycode, 0);
  int km = key.state & (ControlMask | Mod1Mask | ShiftMask);


  // take care about minicli's grabbing 
  if (minicli && minicli->isVisible()){
      freeKeyboard(False);
      if( (kc == XK_F4)  && (km == Mod1Mask) )
	minicli->cleanup();
      return False;
  }
  // take care about klogout's grabbing 
  if (klogout && klogout->isVisible()){
    freeKeyboard(False);
    if( (kc == XK_F4)  && (km == Mod1Mask) )
      klogout->cleanup();
    return False;
  }

  // take care about ktasks's grabbing 
  if (ktask && ktask->isVisible()){
    freeKeyboard(False);
    if( (kc == XK_F4)  && (km == Mod1Mask) )
      ktask->cleanup();
    return False;
  }


  if (!control_grab){

    if( (kc == XK_Tab)  &&
	( km == (Mod1Mask | ShiftMask)
	  || km == (Mod1Mask)
	  )){
      freeKeyboard(False);
      if (!tab_grab){
	XGrabKeyboard(qt_xdisplay(),
		      qt_xrootwin(), False,
		      GrabModeAsync, GrabModeAsync,
		      CurrentTime);
	tab_grab = True;
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
	       !infoBoxClient->isOnDesktop(manager->currentDesktop()));
	       
      if (infoBoxClient && infoBoxClient->
	  isOnDesktop(manager->currentDesktop())){
	if (infoBoxClient->isIconified())
	  setInfoBoxText(QString("(")+infoBoxClient->label+")", 
			 infoBoxClient->window);
	else
	  setInfoBoxText(infoBoxClient->label,
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
//       QString s = desktopMenu->text(desktopMenu->idAt(infoBoxVirtualDesktop-1));
//       while (s.find('&',0) != -1)
//  	s.remove(s.find('&',0),1);
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

  if( (kc == XK_Escape)  && (km == Mod1Mask || km == ControlMask) ){
    freeKeyboard(False);
    showTask();
    return True;
  }    
  
  if( (kc == XK_F2)  && (km == Mod1Mask) ){
    freeKeyboard(False);
    show_minicli();
    return False;
  }    

  if( (kc == XK_F3)  && (km == Mod1Mask) ){
    freeKeyboard(False);
    if (manager->current())
      manager->current()->showOperations();
    return False;
  }

  if( (kc == XK_F4)  && (km == Mod1Mask) ){
    freeKeyboard(False);
    if (manager->current())
      manager->current()->closeClicked();
    return False;
  }    

  if( (kc == XK_F1)  && (km == ControlMask) ){
    freeKeyboard(False);
    manager->switchDesktop(1);
    return False;
  }
  if( (kc == XK_F2)  && (km == ControlMask) ){
    freeKeyboard(False);
    manager->switchDesktop(2);
    return False;
  }
  if( (kc == XK_F3)  && (km == ControlMask) ){
    freeKeyboard(False);
    manager->switchDesktop(3);
    return False;
  }
  if( (kc == XK_F4)  && (km == ControlMask) ){
    freeKeyboard(False);
    manager->switchDesktop(4);
    return False;
  }
  if( (kc == XK_F5)  && (km == ControlMask) ){
    freeKeyboard(False);
    manager->switchDesktop(5);
    return False;
  }
  if( (kc == XK_F6)  && (km == ControlMask) ){
    freeKeyboard(False);
    manager->switchDesktop(6);
    return False;
  }
  if( (kc == XK_F7)  && (km == ControlMask) ){
    freeKeyboard(False);
    manager->switchDesktop(7);
    return False;
  }
  if( (kc == XK_F8)  && (km == ControlMask) ){
    freeKeyboard(False);
    manager->switchDesktop(8);
    return False;
  }


  
  if( (kc == XK_Escape)  && (km == (Mod1Mask | ControlMask)) ){
    freeKeyboard(False);
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
	tab_grab = FALSE;
	if (infoBoxClient){
	  if (infoBoxClient->state == NormalState){
	    manager->raiseClient(infoBoxClient);
	    manager->activateClient(infoBoxClient);
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


bool MyApp::eventFilter( QObject *obj, QEvent * ev){
    static QPoint tmp_point(-10, -10);

    
    if (ev->type() == Event_MouseButtonPress
	|| ev->type() == Event_MouseButtonDblClick){
      if (obj != operations && obj != desktopMenu)
	tmp_point = QCursor::pos();
    }
    
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
	    ignore_release_on_this = NULL;
	    return True;
	  }
	  ignore_release_on_this = NULL;
	}
	if (QCursor::pos() == tmp_point){
	  tmp_point = QPoint(-10,-10);
	  return True;
	}
      }
    }
    return False;
}



bool MyApp::x11EventFilter( XEvent * ev){
  // do some KApp client messages.
  // we cannot call the KApplication::x11EventFilter always,
  // since the drag'n'drop protocoll will let us segfault.
  // A Windowmanager isn't really a usual xclient!
  if (ev->type == ClientMessage){
    if (ev->xclient.message_type == KDEChangePalette
	|| ev->xclient.message_type == KDEChangeGeneral){
      KApplication::x11EventFilter(ev);
      manager->repaintAll();
      return TRUE;
    }
  }
  
  switch (ev->type) {
  case KeyPress:
    return handleKeyPress(ev->xkey);
    break;
  case KeyRelease:
    handleKeyRelease(ev->xkey);
    return FALSE;
    break;
  case ButtonPress:
    {
      Client *c = manager->getClient(ev->xbutton.window);
      bool replay = true;
      if (c)
	c->stopAutoraise();
      if (c && ev->xbutton.window == c->window){
	if ((ev->xbutton.state & Mod1Mask) == Mod1Mask){
	  replay = false;
	  if  (ev->xbutton.button == Button1){
	    c->simple_move();
	  }
	  else if (ev->xbutton.button == Button2)
	    manager->raiseClient(c);
	  else c->simple_resize();

	}
	else if (!c->isActive()){
	  if  (ev->xbutton.button == Button1)
	    manager->raiseClient(c);
	  manager->activateClient(c);
	}
 	// unfreeze the passive grab which is active currently
	if (replay)
	  XAllowEvents(qt_xdisplay(), ReplayPointer, CurrentTime);
	else
	  XAllowEvents(qt_xdisplay(), SyncPointer, CurrentTime);
      }
    }
  break;
  case ButtonRelease:
    break;
  case CreateNotify:
    return TRUE;
    break;
  case MapRequest:
    manager->mapRequest(&ev->xmaprequest);
    break;
  case ConfigureRequest:
    manager->configureRequest(&ev->xconfigurerequest);
    break;
  case CirculateRequest:
    manager->circulateRequest(&ev->xcirculaterequest);
    break;
  case UnmapNotify:
    manager->unmapNotify(&ev->xunmap);
    if (ev->xunmap.window != ev->xunmap.event){
      return TRUE;
    }
    break;
  case DestroyNotify:
    manager->destroyNotify(&ev->xdestroywindow);
    if (ev->xdestroywindow.window != ev->xdestroywindow.event){
      return TRUE;
    }
    break;
  case ClientMessage:
    manager->clientMessage(ev);
    break;
  case ColormapNotify:
    manager->colormapNotify(&ev->xcolormap);
    break;
  case PropertyNotify:
    manager->propertyNotify(&ev->xproperty);
    break;
  case SelectionClear:
    break;
  case SelectionNotify:
    break;
  case SelectionRequest:
    break;
  case EnterNotify:
    if (!operations->isVisible())
      manager->enterNotify(&ev->xcrossing);
    break;
  case LeaveNotify:
    break;
  case ReparentNotify:
    return TRUE; //do not confuse Qt with these events...
    break;
  case MotionNotify:
    manager->motionNotify(&ev->xmotion);
    break;
  case ConfigureNotify:
    // this is because Qt cannot handle (usually does not need to)
    // SubstructureNotify events. 
    if (ev->xconfigure.window != ev->xconfigure.event){
      return TRUE;
    }
    break;
  case MapNotify:
    if (ev->xmap.window != ev->xmap.event){
      return TRUE;
    }
    break;
  case Expose:
    break;
  case FocusIn:
    break;
  case FocusOut:
    break;
  case MappingNotify:
    break;
  default:
    if (manager->shape && ev->type == manager->shape_event)
      manager->shapeNotify((XShapeEvent *)ev);
    break;
  }

  return FALSE;
}


void MyApp::handleOperationsPopup(int itemId){
  if (Client::operation_client)
    Client::operation_client->handleOperationsPopup(itemId);
}

void MyApp::handleDesktopPopup(int itemId){
  if (Client::operation_client)
    Client::operation_client->ontoDesktop(itemId);
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


