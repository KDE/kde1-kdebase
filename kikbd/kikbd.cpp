/*
   - 

  written 1998 by Alexander Budnik <budnik@linserv.jinr.ru>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <stdlib.h>
#include <qaccel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qfile.h>
#include <qtstream.h>
#include <qlayout.h>

#include <kmsgbox.h>
#include <kwm.h>

#include "keytrans.h"
#include "kconfobjs.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "kikbd.h"
#include "kikbd.moc"

using namespace std;

KiKbdConfig *kikbdConfig = 0L;
//=========================================================
// constant definitions
//=========================================================
const char* confRunTimeGroup = "Run Time";
const char* confClassBase    = "class";

const int configDelay     = 50;
const int autoMenuDelay   = 250;
const int altSwitchDelay  = 200;

//=========================================================
//  signal handlers
//=========================================================
void emptyHandler(int sig)
{
  signal(sig, emptyHandler);
}
void exitHandler(int)
{
  ((KiKbdApplication*)kapp)->atExit();
  exit(0);
}
int (*oldXerrorHandler)(Display*, XErrorEvent*);
int xerrorHandler(Display* d, XErrorEvent* e)
{
  char errmsg[64] = {0};

  XGetErrorText(d, (int)e->error_code, errmsg, 64);
  cerr << "KiKbd: X11 error: " << errmsg << endl;
  if((int)e->error_code == BadWindow
     //     || (int)e->error_code == BadDrawable
     || (int)e->error_code == BadAccess)
    return -1;
  return oldXerrorHandler(d, e);
}

//=========================================================
//  application code
//=========================================================
KiKbdApplication::KiKbdApplication(int n, char**v)
  :KApplication(n, v)
{
  KiKbdReconfig = XInternAtom(display, "KDEiKbdReconfig", False );
  KiKbdRotate   = XInternAtom(display, "KDEiKbdRotate"  , False );
  KiKbdIdentity = XInternAtom(display, "KDEiKbdIdentity", False );

  /**
     parse command line options
  */
  int i;for(i=1; i<argc(); i++) {
    if(QString("-rotate") == argv()[i]) {
      sendCommand(KiKbdRotate);
      ::exit(0);
    }
    if(QString("-reconfig") == argv()[i]) {
      sendCommand(KiKbdReconfig);
      ::exit(0);
    }
  }

  /**
     construct main widget
  */
  setMainWidget(button = new KiKbdButton());


  // Disable global mouse tracking as this interferes with our
  // event mask for the root window, effectively disabling reception
  // of CreateNotify events for new windows. DB -- March 1999
  //
  QApplication::setGlobalMouseTracking(FALSE);

  /**
     look for started kikbd
  */
  Window win = findKiKbdWindow();
  if(win)
    KiKbdMsgBox::error(gettext("KDE International Keyboard already "
			       "started on this display."));

  /**
     create menu and connect signals
  */
  menu = new QPopupMenu();
  connect(button, SIGNAL(clicked())     , SLOT(rotateKeyMap()));
  connect(button, SIGNAL(showMenu())    , SLOT(showMenu()));
  connect(menu  , SIGNAL(activated(int)), SLOT(activateMenu(int)));
  connect(this  , SIGNAL(saveYourself()), SLOT(save()));
  connect(this  , SIGNAL(kdisplayPaletteChanged()), SLOT(setPalette()));

  /**
     init values for some variables
  */
  keyMaps.setAutoDelete(TRUE);
  /**
     read keyboard mapping and modifiers at startup
  */
  globalKeySyms.read();
  modifs = XGetModifierMapping(display);
  /**
     set handlers
  */
  signal(SIGHUP , exitHandler);
  signal(SIGINT , exitHandler);
  signal(SIGQUIT, exitHandler);
  signal(SIGTERM, exitHandler);
  signal(SIGUSR1, emptyHandler);
  signal(SIGUSR2, emptyHandler);
  oldXerrorHandler = XSetErrorHandler(xerrorHandler);
  /** scheduling configuration load
   */
  QTimer::singleShot(configDelay, this, SLOT(loadConfig()));
}
KiKbdApplication::~KiKbdApplication()
{
  atExit();
  XFreeModifiermap(modifs);
  delete menu;
}
void KiKbdApplication::sendCommand(Atom atom)
{
  Window win = findKiKbdWindow();
  if(!win) {
    cerr << "Cannot send command to running kikbd" << endl;
    return;
  }
  
  XEvent ev;
  ev.xclient.type         = ClientMessage;
  ev.xclient.display      = display;
  ev.xclient.window       = win;
  ev.xclient.message_type = atom;
  ev.xclient.format       = 32;
  XSendEvent(display, win, False, 0L, &ev);
  XFlush(display);
}
void KiKbdApplication::atExit()
{
  /**
     restore keyboard mapping
  */
  globalKeySyms.write();
  /**
     restore modifiers
  */
  XSetModifierMapping(display, modifs);
  /**
     save classes
  */
  save();
}
/**
   Save relations: window classes to keyboard maps
*/
void KiKbdApplication::save()
{
  QStrList list;
  if(!saveClasses || input != KiKbdConfig::Input_Class) {
    list.append(QString("ROOT,")+keyMaps.current()->getName()+","
		+ keyMaps.at(hotmap<(int)keyMaps.count()?hotmap:0)->getName()
		+ ",");
  } else {
    unsigned i;for(i=0; i<classList.count(); i++) {
      WindowClassEntry &entry = *classList.at(i);
      list.append(entry.name + "," + keyMaps.at(entry.map)->getName() + "," + 
		  keyMaps.at((int)keyMaps.count()>entry.hotmap
			     ?entry.hotmap:0)->getName() + ",");
    }
  }
  KiKbdConfig kikbdConfig(FALSE);
  kikbdConfig.loadConfig();
  kikbdConfig << kikbdConfig.setGroup(confRunTimeGroup)
	      << new KConfigNumberedKeysObject(confClassBase, 0, 
	      list.count(), list);
  kikbdConfig.saveConfig();
}

QPalette mkPalette(const QColor& fg, const QColor& bg) {
  QColorGroup cg(fg, bg, fg, fg, fg, fg, fg);
  return QPalette(cg, cg, cg);
}
/**
   In this function kikbd loads its configuration from file
   This operation is "dangerous" because it involves a lot of
   deletions, creations, and big timeouts for X events
*/
void KiKbdApplication::loadConfig()
{
  /**
     indicate that we are loading configuration
  */
  static bool isInit = TRUE;
  inConfig = TRUE;

  /**
     restore modifiers
  */
  XSetModifierMapping(display, modifs);
  /**
     this is the configurations object
  */
  KiKbdConfig kikbdConfig(FALSE);
  /**
     adding class input
  */
  QStrList classInput;
  kikbdConfig << kikbdConfig.setGroup(confRunTimeGroup)
	      << new KConfigNumberedKeysObject(confClassBase, 0, 100,
					       classInput);
  kikbdConfig.loadConfig();
  ::kikbdConfig = &kikbdConfig;

  /**
     set four palettes: normal, background, with capslock, with alt
  */
  capsPalette   = mkPalette(kikbdConfig.getForColor(),
			    kikbdConfig.getCapsColor());
  altPalette    = mkPalette(kikbdConfig.getForColor(),
			    kikbdConfig.getAltColor());
  normalPalette = mkPalette(kikbdConfig.getForColor(),
			    kikbdConfig.getBakColor());
  button->setPalette(normalPalette);

  /**
     read some settings
  */
  keyboardBeep = kikbdConfig.getKeyboardBeep();
  autoMenu     = kikbdConfig.getAutoMenu();
  input        = kikbdConfig.getInput();
  saveClasses  = kikbdConfig.getSaveClasses();
  hotList      = kikbdConfig.getHotList();
  hotmap       = 1;
  /**
     set font
  */
  button->setFont(kikbdConfig.getCustFont()?kikbdConfig.getFont():generalFont);

  /**
     configuration takes a lot of time, so
     we have to take care of pending events
  */
  //processEvents();

  /** Set initial x codes
   */
  KeySyms initSyms;
  if(kikbdConfig.getCodes() != "")
    if(kikbdConfig.getMap(kikbdConfig.getCodes())->getNoFile()) {
      KiKbdMsgBox::ask(gettext("X codes \"%s\" file does not exist.\n"
			       "Using X default codes."),
		       kikbdConfig.getCodes());
      initSyms = globalKeySyms;
    }
    else initSyms = kikbdConfig.getMap(kikbdConfig.getCodes());
  else initSyms = globalKeySyms;

  /** Expand to 4 symbols if has Alt keys and AltSwitch not equal "None"
      or to 2 symbols
   */
  if(kikbdConfig.hasAltKeys() && kikbdConfig.getAltSwitchRef() != "None")
    initSyms.expandCodes(4);
  else initSyms.expandCodes(2);
  /**
     here we are loading all used symbols in all maps,
     creating popup menu and necessary connections
  */
  keyMaps.clear();
  menu->clear();
  QRect flen;
  QStrList maps = kikbdConfig.getMaps();
  QFontMetrics font = button->fontMetrics();
  unsigned i;for(i=0; i<maps.count(); i++) {
    KiKbdMapConfig *map = kikbdConfig.getMap(maps.at(i));
    if(map->getNoFile()) {
      KiKbdMsgBox::ask(gettext("Keyboard map \"%s\" does not exists.\n"
			       "Skipping this map."), maps.at(i));
      continue;
    }
    keyMaps.append(new KeyMap(*map, initSyms));
    menu->insertItem(map->getIcon(), map->getGoodLabel());
    /**
       we want to automaticaly adjust kikbd button
       size (we may need this option in the future)
    */
    QRect tflen = font.boundingRect(map->getLabel());
    if(flen.width() < tflen.width()) flen = tflen;
  }
  button->setMinimumSize(3*flen.width()/2, 3*font.height()/2);
  mainWidget()->resize(button->minimumSize());
  menu->insertSeparator();
  menu->insertItem(i18n("Setup"));
  menu->insertItem(i18n("Quit"));
  //processEvents();

  // Compose keys support: only implemented for
  // Hellenic (Greek) keyboard -- DB January 1999
  // Reset compose keys & state machine vars for key groups
  //
  wasComp1Key = wasComp2Key = wasShift = rstGroup = FALSE;
  comp1Key = comp2Key = shiftL = shiftR = NoSymbol;
  group = 0;
  /**
     initialise up to two keys for switching from keyboard
  */
  isFirstKey = isSecondKey = FALSE;
  altKey = NoSymbol;
  QStrList switchKeys = kikbdConfig.getSwitch();
  firstKey = secondKey = KeyTranslate::stringToSym(switchKeys.at(0));
  if(switchKeys.count() >= 2)
    secondKey = KeyTranslate::stringToSym(switchKeys.at(1));
  if(kikbdConfig.oneKeySwitch()) {
    if(!removeModifier(firstKey))
      KiKbdMsgBox::warning(gettext("Can not remove %s from Modifiers.\n"),
			   switchKeys.at(0));
    if(!addModifier(firstKey, 5))
      KiKbdMsgBox::warning(gettext("Can not add %s to Modifier number 3.\n"),
			   switchKeys.at(0));    
  }
  if((!kikbdConfig.oneKeySwitch()) && kikbdConfig.hasAltKeys()) {
    QString altSwitchKey = kikbdConfig.getAltSwitchRef();
    if(KeyTranslate::stringToSym(altSwitchKey) != NoSymbol) {
      /**
	 try to remove this key from modifier
      */
      if(!removeModifier(KeyTranslate::stringToSym(altSwitchKey)))
	KiKbdMsgBox::warning(gettext("Can not remove %s from Modifiers.\n"),
			       altSwitchKey);
      /**
	 try to add this key to mod3
      */
      altKey = KeyTranslate::stringToSym("Mode_switch");
      for(i=0; i < keyMaps.count(); i++) {
	if(keyMaps.at(i)->hasAltKeys()) {
	  keyMaps.current()->changeKeySym(altSwitchKey,
					  "Mode_switch", 1, 0);
	  if(!keyMaps.current()->changeKeySym(altSwitchKey,
					      "Mode_switch", 0, 0)) {
	    KiKbdMsgBox::warning(gettext("Can not set Mode Switch as %s"
					 " for keyboard %s.\nAlt symbols "
					 "disabled"),
		    altSwitchKey, keyMaps.current()->getLabel());
	    altKey = NoSymbol;
	    break;
	  }
	}
      }  
    }
  }
  isToggleAlt = FALSE;

  /**
     we can emulate CAPSLOCK
     we need to remove CAPSLOCK key from modifiers
     can we emulate without deleting CAPSLOCK from modifiers?
  */
  isToggleCaps = FALSE;
  if(kikbdConfig.getEmuCapsLock()) {
    capsKey = KeyTranslate::stringToSym("Caps_Lock");
    if(!removeModifier(capsKey)) {
      KiKbdMsgBox::warning(gettext("Can't remove Caps Lock from Modifiers.\n"
				   "Caps Lock Emulation disabled"));
      capsKey = NoSymbol;
    }
  }

  // Compose keys support: only implemented for
  // Hellenic (Greek) keyboard -- DB January 1999
  // Initialize compose keys & state machine vars for key groups
  //
  if(kikbdConfig.hasCompose()) {
    comp1Key = KeyTranslate::stringToSym("Dacute_accent");
    comp2Key = KeyTranslate::stringToSym("Ddiaeresis");
    shiftL = KeyTranslate::stringToSym("Shift_L");
    shiftR = KeyTranslate::stringToSym("Shift_R");
  }

  /**
     some initial values
  */
  autoMenuRequestCount = altSwitchCount = 0;

  /**
     initialize class input
  */
  int defmap = 0;
  for(i=0; i<windowList.count(); i++) {
    windowList.at(i)->map = 0;
    windowList.at(i)->hotmap=1;
  }
  for(i=0; i<classList.count() ; i++) {
    classList.at(i)->map = 0;
    classList.at(i)->hotmap=1;
  }
  if(saveClasses && input == KiKbdConfig::Input_Class) {
    /**
       for all saved window classes
    */
    for(i=0; i<classInput.count(); i++) {
      QStrList cl = KObjectConfig::separate(classInput.at(i));
      /**
	 find class Id by name
      */
      unsigned j;for(j=0; j<keyMaps.count(); j++) {
	if(cl.at(1) == keyMaps.at(j)->getName()) {
	  /**
	     set window class Id
	  */
	  WindowClassEntry *entry = new WindowClassEntry(cl.at(0));
	  if(classList.find(entry)) classList.append(entry);
	  else delete entry;
	  classList.current()->map = j;
	  for(j=0; j<keyMaps.count(); j++) {
	    if(cl.at(2) == keyMaps.at(j)->getName()) {
	      classList.current()->hotmap = j;
	      break;
	    }
	  }
	  break;
	}
      }
    }
  } else {
    QStrList cl = KObjectConfig::separate(classInput.at(0));
    if(!strcmp(cl.at(0), "ROOT")) {
      unsigned j;for(j=0; j<keyMaps.count(); j++) {
	if(cl.at(1) == keyMaps.at(j)->getName()) defmap = j;
	if(cl.at(2) == keyMaps.at(j)->getName()) hotmap = j;
      }
    }
  }

  if(isInit) {
    /**
       Take care of session management
    */
    topWidget = new QWidget();
    setTopWidget(topWidget);
    enableSessionManagement(TRUE);
    setWmCommand("");
    KWM::setUnsavedDataHint(topWidget->winId(), TRUE);

    // Receive events from children of the desktop (root)
    // window; the desktop itself is excluded at this point
    //
    selectRecursivelyInput(qt_xrootwin());

    /**
       Store default map. This is for session managment in Global
       and Window modes
    */
    keyMaps.at(0);
    if(input != KiKbdConfig::Input_Class) save();
  }
  setKeyMapTo((unsigned)-1);
  if(defmap) setKeyMapTo(defmap);

  /**
     try restarting according to config
  */
  if(isInit || (docked != kikbdConfig.getDocking())) {
    docked = kikbdConfig.getDocking();
    /**
       hide and recreate window
    */
    mainWidget()->hide();
    mainWidget()->recreate(0L, docked?0:(WStyle_Customize|WStyle_NoBorder),
			   QPoint(3000, 3000), FALSE);
    /**
       set special property
    */
    long pdata = 1;
    XChangeProperty(display, mainWidget()->winId(), KiKbdIdentity,
		    KiKbdIdentity, 32,
		    PropModeReplace, (unsigned char *)&pdata, 1); 
    if(docked)
      KWM::setDockWindow(mainWidget()->winId());
    mainWidget()->show();
  }

  if (isInit) {
    // Now that out window has been recreated, indicate we
    // want events from the desktop (root) window as well
    //
    XSelectInput(qt_xdisplay(), qt_xrootwin(),
		 SubstructureNotifyMask | FocusChangeMask);
    XSync(qt_xdisplay(), False);
  }

  /**
     find a good place
  */
  if(!docked) {
    while (isInit && !KWM::isKWMInitialized()) sleep(1);
    if(KWM::isKWMInitialized()) {
      int geom = kikbdConfig.getAutoStartPlace();
      QRect rec = KWM::getWindowRegion(KWM::currentDesktop());
      int x = rec.x(), y = rec.y();
      switch(geom) {
      case KiKbdConfig::Place_TopRight    :
      case KiKbdConfig::Place_Bottom_Right:
	x = rec.x() + rec.width() - mainWidget()->width(); y = rec.y();
      }
      switch(geom) {
      case KiKbdConfig::Place_Bottom_Right:
      case KiKbdConfig::Place_BottomLeft  :
	y = rec.y() + rec.height() - mainWidget()->height();
      }
      mainWidget()->move(x, y);
    }
  }

  ::kikbdConfig = 0L;
  inConfig = (isInit = FALSE);
}
QString KiKbdApplication::windowClass(Window w)
{
  unsigned long nitems_ret, bytes_after_ret;
  unsigned char* prop;
  Atom     type_ret;
  int      format_ret;
  QString  ret = "";

  if(XGetWindowProperty(display, w, XA_WM_CLASS, 0L, 256L, 0, XA_STRING,
			&type_ret, &format_ret, &nitems_ret,
			&bytes_after_ret, &prop) == Success 
     && type_ret != None) {
    ret = (const char*)prop;
    XFree((char*)prop);
  }
  return ret;
}
bool KiKbdApplication::windowCheckProperty(Window win, Atom property)
{
  unsigned long nitems_ret, bytes_after_ret;
  unsigned char* prop;
  Atom     type_ret;
  int      format_ret;
  QString  ret = "";

  return XGetWindowProperty(display, win, property, 0L, 0L, 0,
			    AnyPropertyType, &type_ret, &format_ret, 
			    &nitems_ret, &bytes_after_ret, &prop) == Success
    && (type_ret != None);
}
Window KiKbdApplication::findKiKbdWindow(Window win)
{
  static short level = 0;
  if(level++ > 6) return level--, 0;

  if(!win) win = desktop()->winId();
  if(!windowCheckProperty(win, KiKbdIdentity)) {
    Window   root, parent, *children;
    unsigned nchildren, i;
    if(XQueryTree(display, win, &root, &parent,
		  &children, &nchildren)) {
      for(i=win=0; (i < nchildren) && !(win=findKiKbdWindow(children[i++])););
      XFree((char*)children);
    }
  }
  return level--, win;
}
bool KiKbdApplication::x11EventFilter(XEvent *e)
{
  /**
     We change Qt's default behavior:
     - we look for keyboard events from the world
     - and only for foreign events of other types
  */

  /**
     event is for KApplication filter if:
     1. (it is not kikbd button OR the type is not key press/release) AND
     2. it is not from the root (desktop) window AND
     3. it can't be found in Qt widgets
  */
  bool myWindow =
    (e->xany.window != button->winId() || (e->type != KeyRelease &&
					   e->type != KeyPress))
    && (e->xany.window != qt_xrootwin())
    && QWidget::find(e->xany.window);
  /**
     filter events
  */

  if(!myWindow) {
    KeySym key;
    /**
       during configuration we ignory foreign events
    */
    if (inConfig) return TRUE;

    switch(e->type) {
      //case(UnmapNotify):
    case(MapNotify):
      /**
	 work around screen savers' problem
      */
      if(windowClass(e->xany.window).contains(".kss") == 1)
	setKeyMapTo(0);
      break;

    case(CreateNotify): 
      /**
	 a new window was opened
	 we want keyboard events from it
      */
      //CT 07Jan1998 - temporary fix. The important winId of the newly
      //          created window is never passed to kikbd. So we reparse
      //          the whole shitty X tree.
      //CT 29Mar1999 - fixed by Dimitrios Bouras - he reported this was a Qt
      //	  bug
      selectRecursivelyInput(e->xcreatewindow.window);
      //selectRecursivelyInput(desktop()->winId());
      break;

    case(DestroyNotify):
      /**
	 the window was destroyed
	 we have to forget about it
      */
      if(windowList.findWindow(e->xany.window))
	windowList.remove();
      break;
      
    //CT 29Mar1999 - Dimitrios added this for fixing a Solaris problem
    case(MappingNotify):
      if (e->xmapping.request == MappingKeyboard)
	XRefreshKeyboardMapping(&e->xmapping);
      break;

    case(KeyRelease):
      /**
	 when key released we have to do a few things
      */
      key = XLookupKeysym(&e->xkey, 0);
      if(key == capsKey) {
	toggleCaps(isToggleCaps?FALSE:TRUE);
	break;
      }
      if(key == firstKey || key == secondKey) {
	/**
	   maybe we need to change map
	*/
	if((isFirstKey && key == secondKey) 
	   || (isSecondKey && key == firstKey)) {
	  rotateKeyMap();
	}
	if(key == firstKey ) isFirstKey = FALSE;
	if(key == secondKey) isSecondKey= FALSE;
      }
      if(key == altKey) {
	QTimer::singleShot(altSwitchDelay, this, SLOT(altSwitchTimer()));
      }
      if(rstGroup) {
	rstGroup = FALSE;
	group = 0;
	keyMapGroup(group);
	//cerr << "*group = " << group << endl;
      }
      break;

    case(KeyPress):
      key = XLookupKeysym(&e->xkey, 0);
      if(key == firstKey || key == secondKey) {
	/**
	   we only worry about key press
	   for the keymap switching keys
	*/
	if(key == firstKey ) isFirstKey = TRUE;
	if(key == secondKey) isSecondKey = TRUE;
	if(isFirstKey && isSecondKey && autoMenu) {
	  QTimer::singleShot(autoMenuDelay, this, SLOT(autoMenuRequest()));
	  autoMenuRequestCount++;
	}
      }
      if(key == altKey) {
	toggleAlt(TRUE);
	altSwitchCount++;
      }
      if(key == shiftL || key == shiftR)
	wasShift = TRUE;
      else
	wasShift = FALSE;
      if(((XKeyEvent *)e)->state & ShiftMask)
	key = XLookupKeysym(&e->xkey, 1);
      if(key == comp1Key || key == comp2Key) {
	if(key == comp1Key) {
	  wasComp1Key = TRUE;
	  if(wasComp2Key) group = 3;
	  else group = 1;
	}
	if(key == comp2Key) {
	  wasComp2Key = TRUE;
	  if(wasComp1Key) group = 3;
	  else group = 2;
	}
	keyMapGroup(group);
	//cerr << "*group = " << group << endl;
      }
      else {
	if (! wasShift && (wasComp1Key || wasComp2Key)) {
	  wasComp1Key = wasComp2Key = FALSE;
	  rstGroup = TRUE;
	}
      }
      break;

    case(FocusIn):
      /**
	 if we want to have map per window
	 we need to know when focus changed
      */
      switch(input) {
      case KiKbdConfig::Input_Window :
	if(windowList.findWindow(e->xany.window)) {
	  hotmap = windowList.current()->hotmap;
	  setKeyMapTo(windowList.current()->map, FALSE);
	}
	break;
      case KiKbdConfig::Input_Class :
	if(windowList.findWindow(e->xany.window)) {
	  hotmap = classList.at(windowList.current()->classId)->hotmap;
	  setKeyMapTo(classList.at(windowList.current()->classId)->map,
		      FALSE);
	}
	break;
      }
      break;
      //case(FocusOut): 
    }
    return TRUE;
  }
  /**
     client messages can tell about reconfiguration
  */
  if(!inConfig && e->type == ClientMessage) {
    if(e->xclient.message_type == KiKbdReconfig) {
      QTimer::singleShot(configDelay, this, SLOT(loadConfig()));
      return TRUE;
    } else if(e->xclient.message_type == KiKbdRotate) {
      QTimer::singleShot(configDelay, this, SLOT(rotateKeyMap()));
      return TRUE;
    }
  }
  /**
     this is default behavior of KApplication
     only our own events here
  */
  return KApplication::x11EventFilter(e);
}
/**
   check if window is a top level window
*/
int KiKbdApplication::isTopLevelWindow(Window win)
{
  Atom     type_ret;
  int      format_ret;
  unsigned long   nitems_ret;
  unsigned long   bytes_after_ret;
  unsigned char  *prop_ret;

  XGetWindowProperty(display, win, XA_WM_CLASS, 0L, 0L, 0, XA_STRING,
		     &type_ret,&format_ret,&nitems_ret,&bytes_after_ret,&prop_ret);
  return(type_ret!=None);
}
/** 
    is this window or any of its children a window manager frame?
*/
int KiKbdApplication::isWmWindow(Window win)
{
  Window   root1,parent,*children;
  unsigned children_num,i;
  int   is=0;

  if(XQueryTree(display, win, &root1, &parent, &children, &children_num))
    {
      is |= (parent==root1);
      for(i=0; !is && i<children_num; i++)
	{
	  is |= isTopLevelWindow(children[i]);
	}
      XFree(children);
    }
  return is;
}
/**
   add this window to window cache and
   indicate we want to receive events from it
*/
void KiKbdApplication::selectWindowInput(Window win)
{
  //if(windowList.findWindow(win)) return;

  // ignore QT widget windows; believe it or not,
  // this includes the desktop (root) window
  //
  if (QWidget::find(win))
    return;

  // clean any events for foreign windows
  //
  XSelectInput(qt_xdisplay(), win, NoEventMask);

  // Retrieve window attributes; we are interested in our
  // event notification masks with respect to this window
  //
  XWindowAttributes wa;
  if(!XGetWindowAttributes(display, win, &wa)) {
    cerr << "KiKbd: selectWindowInput: XGetWindowAttributes: "
	 << "error: window ID: " << hex << win << dec << endl;
    return;
  }

  // No matter what, we need info on creation
  // and deletion of this window's children
  //
  wa.your_event_mask |= SubstructureNotifyMask;

  // Collect some window properties
  //
  QString wclass = windowClass(win);
  int topLevel = isTopLevelWindow(win);

  // We request other events only if:
  // - some other window has already requested these events
  // - the window is blocking the events from propagation
  // 
  bool individ = topLevel && (wclass != "kwm");
  int needMask = (KeyPressMask | KeyReleaseMask);
  if (individ) needMask |= FocusChangeMask;
  wa.your_event_mask |= ((wa.all_event_masks|wa.do_not_propagate_mask)
			 & needMask);
  if (wa.your_event_mask & needMask) wa.your_event_mask |= needMask;

  /**
     init class list item
  */
  WindowClassEntry *entry = new WindowClassEntry(wclass);
  if(classList.find(entry) == -1)
    classList.append(entry);
  /**
     insert window into window list with class Id
  */
  if(!windowList.findWindow(win))
    windowList.append(new WindowEntry(win, keyMaps.at(), hotmap,
		      classList.at()));
  else
    *(windowList.current()) = WindowEntry(win, keyMaps.at(), hotmap, 
					  classList.at());
  XSelectInput(display, win, wa.your_event_mask);
}
/**
   add window using window tree
*/
void KiKbdApplication::selectRecursivelyInput(Window win)
{
  Window   root, parent, *children;
  unsigned nchildren;
  QString wclass = windowClass(win);

  if(!XQueryTree(display, win, &root, &parent, &children, &nchildren))
    return;

  // Ignore kpanel windows as they interfere with
  // the window creation monitoring mechanism -- DB March 1999
  //
  if (wclass == "kpanel")
    return;

  unsigned i; for (i=0; i<nchildren; i++) {
    selectRecursivelyInput(children[i]);
  }
  XFree((char *)children);
  selectWindowInput(win);
}
/**
   remove symbol from modifier
*/
bool KiKbdApplication::removeModifier(KeySym mod)
{ 
  /**
     get current
  */
  XModifierKeymap *modif = XGetModifierMapping(display);
  if(modif == 0L) return FALSE;
  /**
     look into it for changes
  */
  KeyCode code = XKeysymToKeycode(display, mod);
  bool changed = FALSE;
  int i;for(i=0; i<8*modif->max_keypermod; i++) {
    if(modif->modifiermap[i] == code) {
      if(modif->modifiermap[i]) {
	modif->modifiermap[i] = 0;
 	changed = TRUE;
      }
    }
  }
  if(changed && XSetModifierMapping(display, modif)!=MappingSuccess) {
    XFreeModifiermap(modif);
    return FALSE;
  } 
  XFreeModifiermap(modif);
  return TRUE;
}
/**
   add symbol to specified modifier
*/
bool KiKbdApplication::addModifier(KeySym sym, int mod)
{ 
  /**
     get current
  */
  XModifierKeymap *modif = XGetModifierMapping(display);
  if(modif == 0L) return FALSE;
  /**
     look into it for changes
  */
  KeyCode code = XKeysymToKeycode(display, sym);
  modif = XInsertModifiermapEntry(modif, code, mod);
  if(modif == 0L) return FALSE;

  if(XSetModifierMapping(display, modif)!=MappingSuccess) {
    XFreeModifiermap(modif);
    return FALSE;
  } 
  XFreeModifiermap(modif);
  return TRUE;
}
/**
   change keyboard group by number
*/
void KiKbdApplication::keyMapGroup(unsigned g)
{
  keyMaps.current()->toggle(g);
}
/**
   change keyboard map by number
*/
void KiKbdApplication::setKeyMapTo(unsigned i, bool ch)
{
  if(keyMaps.at() != (int)i || i == (unsigned)-1) {
    if(i == (unsigned)-1) i = 0;
    keyMaps.at(i);
    button->setText(keyMaps.current()->getLabel());
    QToolTip::remove(button);
    QToolTip::add(button, keyMaps.current()->getComment());
    if(keyboardBeep && !inConfig) beep();
    keyMaps.current()->toggle(0);
    group = 0;
    wasComp1Key = wasComp2Key = wasShift = rstGroup = FALSE;
  }

  if(ch) {
    if(i > 0) hotmap = i;
    /**
       find current window
    */
    Window win;
    int revert_to_return;
    XGetInputFocus(display, &win, &revert_to_return);
    windowList.findWindow(win);
    /**
       remember window settings
    */
    if(windowList.at() != -1) {
      windowList.current()->map = keyMaps.at();
      windowList.current()->hotmap = hotmap;
      classList.at(windowList.current()->classId)->map = keyMaps.at();
      classList.at(windowList.current()->classId)->hotmap = hotmap;
    }
  }
}
/**
   change keyboard map to the next one defined, in a circular fashion
*/
void KiKbdApplication::rotateKeyMap()
{
  unsigned next;
  if(hotList) next = keyMaps.at()==hotmap?0:hotmap;
  else 
    //next = (keyMaps.at()+1>=(int)keyMaps.count())?0:keyMaps.at()+1;
    //CT 05Jan1999 - arguably faster than above
    next = (keyMaps.at()+1) % (int)keyMaps.count(); 
  setKeyMapTo(next);
}
void KiKbdApplication::toggleCaps(bool on)
{
  if(isToggleCaps == on) return;
  keyMaps.current()->toggleCaps(isToggleCaps = on, group);
  if(isToggleCaps) button->setPalette(capsPalette);
  else button->setPalette(normalPalette);
}
void KiKbdApplication::toggleAlt(bool on)
{
  if(isToggleAlt == on) return;
  if((isToggleAlt=on)) button->setPalette(altPalette);
  else if(isToggleCaps) button->setPalette(capsPalette);
  else button->setPalette(normalPalette);
}
/**
   when we use global popup menu (by holding switch keys
   for some time) we need to know where mouse pointer is
*/
QPoint KiKbdApplication::getPointer()
{
  Window root_ret, win_ret;
  int    root_x, root_y, win_x, win_y;
  unsigned mask;
  XQueryPointer(display, desktop()->winId(), &root_ret, &win_ret,
		&root_x, &root_y, &win_x, &win_y, &mask);
  return QPoint(root_x, root_y);
}
void KiKbdApplication::altSwitchTimer()
{
  altSwitchCount--;
  if(!altSwitchCount) toggleAlt(FALSE);
}
void KiKbdApplication::setPalette()
{
  QPalette palette = mainWidget()->palette();
  normalPalette = mkPalette(normalPalette.normal().foreground(),
			    palette.normal().background());
  capsPalette   = mkPalette(palette.normal().foreground(),
			    capsPalette.normal().background());
  altPalette    = mkPalette(palette.normal().foreground(),
			    altPalette.normal().background());
  if((isToggleAlt)) button->setPalette(altPalette);
  else if(isToggleCaps) button->setPalette(capsPalette);
  else button->setPalette(normalPalette);
}
/**
   show popup menu
*/
void KiKbdApplication::showMenu()
{
  menu->popup(mainWidget()->mapToGlobal(mainWidget()->rect().center()));
}
void KiKbdApplication::autoMenuRequest()
{
  if(!--autoMenuRequestCount && isFirstKey && isSecondKey) {
    isFirstKey = isSecondKey = FALSE;
    menu->popup(getPointer());
  }
}
void KiKbdApplication::activateMenu(int i)
{
  if(i == (int)menu->count()-1) {
    //CT 20Jan1999 - added confirmation box
    bool autoStart = FALSE;
    switch (KMsgBox::yesNoCancel(0, i18n("Exiting International Keyboard"),
			      i18n("You're about to quit the \"International "
				   "Keyboard\" tool.\n Do you want to have it"
				   " autostarted at your next login?"),
			      KMsgBox::QUESTION, i18n("Yes"),
			      i18n("No"), i18n("Cancel"))) {
    case 3: return;
    case 1: autoStart = TRUE;  break;
    case 2: autoStart = FALSE; break;
    }
    KConfig config(kapp->localconfigdir() + "/" + kapp->appName() + "rc");
    config.setGroup("StartUp");
    config.writeEntry("AutoStart", autoStart);
    config.sync();
    //CT
    ::exit(0);
  } else if(i == (int)menu->count()-2) {
    KiKbdConfig::startConfigProgram();
  } else setKeyMapTo(i);
}
/**
   button class
*/
KiKbdButton::KiKbdButton(QWidget* parent)
  :QLabel(parent)
{
  setAlignment(AlignCenter);
}
void KiKbdButton::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == RightButton) emit showMenu();
  if(event->button() == LeftButton ) emit clicked();
}

/**
   Window List class
*/
WindowList::WindowList()
{
  setAutoDelete(TRUE);
}
int WindowList::compareItems(GCI item1, GCI item2)
{
  return ((WindowEntry*)item1)->window - ((WindowEntry*)item2)->window;
}
/**
   Window Class List class
*/
WindowClassList::WindowClassList()
{
  setAutoDelete(TRUE);
}
int WindowClassList::compareItems(GCI item1, GCI item2)
{
  return strcmp(((WindowClassEntry*)item1)->name,
		((WindowClassEntry*)item2)->name);
}


//=========================================================
//   Main Program
//=========================================================
void msgHandler(QtMsgType type, const char* msg)
{
  switch(type) {
  case QtWarningMsg:
  case QtDebugMsg:
    break;
  case QtFatalMsg:
    cerr << msg << endl;
    exit(0);
  }
}

int main(int argc, char** argv)
{
  qInstallMsgHandler(msgHandler);
  KiKbdApplication appl(argc, argv);
  appl.exec();
}
