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
#include <stream.h>
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
#include "kikbd.moc.h"

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
  cout << "error = " << (int)e->error_code 
    //<< " for pid = " << getpid()
       << endl;
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
     pars command line options
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
     look for started kikbd
  */
  Window win = findKiKbdWindow();
  if(win)
    KiKbdMsgBox::error(gettext("KDE International Keyboard already "
			       "started on this display."));

  /**
     construct main widget
  */
  setMainWidget(button = new KiKbdButton());
  
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
  globalKeySyms.expandCodes(4);
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
  /** sheduling configuration load
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
    cerr << "Can not send command to running kikbd" << endl;
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
  KiKbdConfig kikbdConfig;
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
   In this function kikbd load it configuration from file
   This operation is dangaraus becouse a lot of delete, new
   and a big timeout for X events
*/
void KiKbdApplication::loadConfig()
{
  /**
     mark what we are loading configuration
  */
  static bool isInit = TRUE;
  inConfig = TRUE;

  /**
     restore modifiers
  */
  XSetModifierMapping(display, modifs);
  /**
     this is configurations object
  */
  KiKbdConfig kikbdConfig;
  /**
     adding class input
  */
  QStrList classInput;
  kikbdConfig << kikbdConfig.setGroup(confRunTimeGroup)
	      << new KConfigNumberedKeysObject(confClassBase, 0, 100,
					       classInput);
  kikbdConfig.loadConfig();

  /**
     set three palette: normal, with capslock, with alt
  */
  normalPalette = button->palette();
  capsPalette   = mkPalette(normalPalette.normal().foreground(),
			    kikbdConfig.getCapsColor());
  altPalette    = mkPalette(normalPalette.normal().foreground(),
			    kikbdConfig.getAltColor());
  normalPalette = mkPalette(kikbdConfig.getForColor(),
			    normalPalette.normal().background());
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
     configuration take a lot of time
     we have to care about pending events
  */
  //processEvents();

  /**
     here we are loading all used symbols in all maps
     also we are create popup menu and necasary connections
  */
  keyMaps.clear();
  menu->clear();
  int len = 0;
  QStrList maps = kikbdConfig.getMaps();
  unsigned i;for(i=0; i<maps.count(); i++) {
    KiKbdMapConfig *map = kikbdConfig.getMap(maps.at(i));
    keyMaps.append(new KeyMap(*map, globalKeySyms));
    menu->insertItem(map->getIcon(), map->getGoodLabel());
    /**
       we want to automaticaly adjust kikbd button size
       may be we need this optional in the future
    */
    int tlen = map->getLabel().length();
    if(len < tlen) len = tlen;
  }
  QFontMetrics font = button->fontMetrics();
  button->setMinimumSize(font.maxWidth()*len, 3*font.height()/2);
  mainWidget()->resize(button->minimumSize());
  menu->insertSeparator();
  menu->insertItem(i18n("Setup"));
  menu->insertItem(i18n("Quit"));
  //processEvents();

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
    QStrList altSwitchKeys = kikbdConfig.getAltSwitch();
    if(KeyTranslate::stringToSym(altSwitchKeys.at(0)) != NoSymbol) {
      /**
	 try to remove this key from modifier
      */
      if(!removeModifier(KeyTranslate::stringToSym(altSwitchKeys.at(0))))
	KiKbdMsgBox::warning(gettext("Can not remove %s from Modifiers.\n"),
			       altSwitchKeys.at(0));
      /**
	 try to add this key to mod3
      */
      altKey = KeyTranslate::stringToSym("Mode_switch");
      for(i=0; i < keyMaps.count(); i++) {
	if(keyMaps.at(i)->hasAltKeys()) {
	  keyMaps.current()->changeKeySym(altSwitchKeys.at(0),
					  "Mode_switch", 1);
	  if(!keyMaps.current()->changeKeySym(altSwitchKeys.at(0),
					      "Mode_switch", 0)) {
	    KiKbdMsgBox::warning(gettext("Can not set Mode Switch as %s"
					 " for keyboard %s.\nAlt symbols "
					 "disabled"),
		    altSwitchKeys.at(0), keyMaps.current()->getLabel());
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
     can we emulate without delete CAPSLOCK from modifiers?
  */
  isToggleCaps = FALSE;
  if(kikbdConfig.getEmuCapsLock()) {
    capsKey = KeyTranslate::stringToSym("Caps_Lock");
    if(!removeModifier(capsKey)) {
      KiKbdMsgBox::warning(gettext("Can not remove Caps Lock from Modifiers.\n"
				   "Caps Lock Emulation disabled"));
      capsKey = NoSymbol;
    }
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
       Take care about session menagment
    */
    topWidget = new QWidget();
    setTopWidget(topWidget);
    enableSessionManagement(TRUE);
    setWmCommand("");
    KWM::setUnsavedDataHint(topWidget->winId(), TRUE);
    /**
       Take events from desktop
    */
    keyMaps.at(0);
    XSelectInput(display, desktop()->winId(), 
		 SubstructureNotifyMask | FocusChangeMask);
    selectRecursivelyInput(desktop()->winId());
    /**
       Store default map. This for session menagment in Global
       and Window modes
    */
    keyMaps.at(0);
    if(input != KiKbdConfig::Input_Class) save();
  }
  setKeyMapTo((unsigned)-1);
  if(defmap) setKeyMapTo(defmap);

  /**
     try to restart according to config
  */
  if(isInit || (docked != kikbdConfig.getDocking())) {
    docked = kikbdConfig.getDocking();
    /**
       hide and recreate window
    */
    mainWidget()->hide();
    mainWidget()->recreate(0L, docked?0:(WStyle_Customize|WStyle_NoBorder),
			   QPoint(0, 0), FALSE);
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
  
  /**
     find good place
  */
  if(!docked) {
    if(isInit && !KWM::isKWMInitialized()) sleep(1);
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
     we chage behavior of Qt
     we have much more events
     we look for keyboard events from the world
     and only for foreign event of other types
  */
  /**
     event is for KApplication filter if:
     1)(it is not kikbd button OR the type is not key press/release) AND
     2)it not root window AND
     3)it can be find in Qt widgets AND
  */
  bool myWindow =  (e->xany.window != button->winId() 
		    || (e->type != KeyRelease && e->type != KeyPress))
    && (e->xany.window != desktop()->winId())
    && QWidget::find(e->xany.window);
  /**
     filter events
  */
  //if(e->type == ConfigureNotify)
  //cerr << windowClass(e->xany.window) << endl;
  if(!myWindow) {
    KeySym key;
    /**
       during configuration we ignory foreign events
    */
    if(inConfig) {return TRUE;}
    switch(e->type) {
      //case(UnmapNotify):
    case(MapNotify):
      /**
	 work around screen savers problem
      */
      if(windowClass(e->xany.window).contains(".kss") == 1)
	setKeyMapTo(0);
      break;
    case(CreateNotify): 
      /**
	 someone open the new window
	 we want keyboard events from it
      */
      //cout << form("new window %x", e->xcreatewindow.window) << endl;
      selectRecursivelyInput(e->xcreatewindow.window);
      break;
    case(DestroyNotify):
      /**
	 the window destroed
	 we have to forget about it
      */
      if(windowList.findWindow(e->xany.window))
	windowList.remove();
      break;
    case(KeyRelease):
      /**
	 when key released we have to do somethings
      */
      key = XLookupKeysym(&e->xkey, 0);
      if(key == capsKey) {
	toggleCaps(isToggleCaps?FALSE:TRUE);
	break;
      }
      if(key == firstKey || key == secondKey) {
	/**
	   may be we need to change map
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
      break;
    case(KeyPress):
      key = XLookupKeysym(&e->xkey, 0);
      if(key == firstKey || key == secondKey) {
	/**
	   we are worry about key press
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
   is this window a top level window
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
    is this window are the frame of window manager
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
   we adding this window to window cache
   and we whant some events from it
*/
void KiKbdApplication::selectWindowInput(Window win)
{
  //if(windowList.findWindow(win)) return;
  /**
     ignore own windows
  */
  if(QWidget::find(win)) return;

  /**
     clean any events for foreign windows
  */
  //if(!QWidget::find(win)) 
  XSelectInput(display, win, 0);

  XWindowAttributes wa;
  if(!XGetWindowAttributes(display, win, &wa)) return; // unknown error

  /**
     we need information on creation/deletion
  */
  wa.your_event_mask |= SubstructureNotifyMask;

  /**
     we request events only if
     1. the window already requested this kind of events
     2. the window is blocking the events from propagation
     3. this window is wm's one or top level one
     (for the case the application itself doesn't want these events)
      (... from xrus ...)
  */ 
  QString wclass = windowClass(win);
  int topLevel = isTopLevelWindow(win);
  //int wm       = isWmWindow(win);
  bool individ = topLevel && (wclass != "kwm");

  int needMask = (KeyPressMask | KeyReleaseMask);
  if(individ) {
    needMask |= FocusChangeMask;
  }

  wa.your_event_mask |= (wa.all_event_masks|wa.do_not_propagate_mask)
    & needMask;
  if(wa.your_event_mask & needMask) wa.your_event_mask |= needMask;
  //if(((wa.your_event_mask & needMask) != needMask)
  //&& (win==wa.root || topLevel || wm)) wa.your_event_mask |= needMask;

  /**
     init class list
  */
  WindowClassEntry entry(wclass);
  if(classList.find(&entry) == -1)
    classList.append(new WindowClassEntry(wclass));
  /**
     insert window into window list with class Id
  */
  if(!windowList.findWindow(win))
    windowList.append(new WindowEntry(win, keyMaps.at(), hotmap, classList.at()));
  else
    *(windowList.current()) = WindowEntry(win, keyMaps.at(), hotmap, 
					  classList.at());
  XSelectInput(display, win, wa.your_event_mask);
}
/**
   this did add window using window tree
*/
void KiKbdApplication::selectRecursivelyInput(Window win)
{
  Window   root, parent, *children;
  unsigned nchildren;
  
  if(!XQueryTree(display, win, &root, &parent, &children, &nchildren))
    return;

  unsigned i;for(i=0; i < nchildren; i++) {
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
    keyMaps.current()->toggle();
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
   change keyboard map to next in circle
*/
void KiKbdApplication::rotateKeyMap()
{
  unsigned next;
  if(hotList) next = keyMaps.at()==hotmap?0:hotmap;
  else next = (keyMaps.at()+1>=(int)keyMaps.count())?0:keyMaps.at()+1;
  setKeyMapTo(next);
}
void KiKbdApplication::toggleCaps(bool on)
{
  if(isToggleCaps == on) return;
  keyMaps.current()->toggleCaps(isToggleCaps = on);
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
   for a time)
   we need to know where mouse pointer is
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
   show poopum menu
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
    exit();
  } else if(i == (int)menu->count()-2) {
    system("kcmikbd&");
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

main(int argc, char** argv)
{
  qInstallMsgHandler(msgHandler);
  KiKbdApplication appl(argc, argv);
  appl.exec();
}
