/*
 * manager.h. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#ifndef MANAGER_H
#define MANAGER_H

#include <qapp.h>
#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qpixmap.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpopmenu.h>
#include <qchkbox.h>
#include <qtimer.h>
#include <qlist.h>
#include <qstrlist.h>

#include "client.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

#include <config.h>

class Manager : public QObject {
Q_OBJECT
public:

  Manager();
  ~Manager(){};

  // A X11 window manager has to react on a couple of XEvents. kwm
  // handles each kind of event in a dedicated function

  // a window wants to move/change size or change otherwise
  void configureRequest(XConfigureRequestEvent *e);
  // a window wants to be mapped
  void mapRequest(XMapRequestEvent *e);
  // circualte request: not needed and not handled
  void circulateRequest(XCirculateRequestEvent */* e */ ){};

  // a window wants to be unmapped
  void unmapNotify(XUnmapEvent *e);
  // a window has been destroyed, so we have to remove it from our
  // data structures.
  void destroyNotify(XDestroyWindowEvent *e);
  // client messages
  void clientMessage(XEvent *e);

  // color map issues
  void colormapNotify(XColormapEvent *e);
  // a property of one of the windows changed
  void propertyNotify(XPropertyEvent *e);
  // a window may be a shaped window
  void shapeNotify(XShapeEvent *e);
  // notification of reparenting events
  void reparentNotify(XReparentEvent* e);
  // notification of pointer movements
  void motionNotify(XMotionEvent* e);
  // the pointer entered one of our windows. We may have to activate a
  // window (when focus follows mouse) or handle electric borders.
  void enterNotify(XCrossingEvent *e);
  // the pointer left one of our windows. This will stop a possible
  // autoraise process.The brain-dead classic focus follows mouse policy
  // will also take the focus away from it.
  void leaveNotify(XCrossingEvent *e);


  // kwm supports different kinds of window placement. doPlacement
  // will select the appropriate method.
  void doPlacement(Client*);

  // some placement policies
  void randomPlacement(Client* c);
  //CT 18jan98
  void smartPlacement(Client* c);
  //CT 30jan98
  void cascadePlacement(Client* c, bool re_init);

  //CT 12mar98 - functions to rearrange the windows on the desk
  //   17mar98, 27May98 - magics: snap to border or to windows
  void deskUnclutter();
  void deskCascade();
  void snapIt(Client *c);



  // one of the central functions within kwm: manage a new window. If
  // mapped is true, then the window is already mapped. This happens
  // if kwm is started after some windows already appeard.
  void manage(Window w, bool mapped = False);

  // put the client in withdraw state (which means it is not managed
  // any longer)
  void withdraw(Client* c);

  // get a pointer to the Client object from an X11 window. The window
  // can be either the parent window (which is the kwm frame around a
  // window) or the client window itself. Returns 0 if no client can
  // be found.
  Client* getClient(Window w);

  // get a pointer to the Client object from the sizegrip
  Client* Manager::getClientFromSizegrip(Window w);

  // returns the current client (the client which has the focus) or 0
  // if no client has the focus.
  Client* current();

  // add a client into the manager큦 client lists
  void addClient(Client* c);
  // activate a client: Take care about visibility, floating submenus
  // and focus.
  void activateClient(Client* c, bool set_revert = true);
  // remove a client from the manager큦 client list
  void removeClient(Client* c);
  // raise a client. Take care about modal dialogs
  void raiseClient(Client* c);
  // lower a client. Take care about modal dialogs and kfm큦 root icons
  void lowerClient(Client* c);
  // close a client. clients with WM_DELETE_WINDOW protocol set
  // (=Pdeletewindow) are closed via wm_delete_window ClientMessage.
  // Others are destroyed.
  void closeClient(Client* c);
  // emits a signal to the kwm modules that a client has changed.
  void changedClient(Client* c);

  // checks whether the client is still existing
  bool hasClient(Client* c);


  // this is called if the current client loses focus. This can happen
  // if the window is destroyed, the client becomes unmapped or the
  // user has chosen the brain dead classic focus follows mouse policy
  // and the mouse pointer has left the window. noFocus may give the
  // focus to a window which had the focus before, or put the focus to
  // a dummy window if there큦 no window left on this desktop or the
  // user has chosen an archaic (=classic) desktop policy.
  void noFocus();
  // used by noFocus to put the X11 focus to a dummy window
  void focusToNull();
  // auxiliary function to put the focus to a client window
  void focusToClient(Client* c);

  // sets the state of a window (IconicState, NormalState, WithdrawnState)
  void setWindowState(Client *c, int state);
  // gets the transientfor hint of a client from the XServer and
  // stores it in Client->trans
  void getWindowTrans(Client *c);

  // switch to the specified virtual desktop
  void switchDesktop(int);

  // switch to another virtual desktop according to the specified direction.
  // Useful for keyboard shortcuts or electric borders.
  enum DesktopDirection {Up, Down, Left, Right};
  void moveDesktopInDirection(DesktopDirection d, Client* c = 0, bool move_pointer = true);

  // returns the number of the current desktop
  int currentDesktop(){
    return current_desktop;
  }

  // defines the number of virtual desktop. Can be from 1 up to 8
  int number_of_desktops;

  // Tells the XServer and the client itself to sync the X window with
  // the datas stored in kwm큦 client object. If emit_changed is true,
  // then all kwm modules are informed about that change.
  void sendConfig(Client* c, bool emit_changed = TRUE);

  // give all managed windows free and remove the kwm_running
  // property.
  void cleanup(bool kill=false);

  // repaint all windows. This is useful when we recieved a
  // KDEChangeGeneral or KDEChangePalette event.
  void repaintAll();

  // support for shaped windows

  // used to store the return values of
  // XShapeQueryExtension.
  // Necessary since shaped window are an extension to X
  int shape;
  int shape_event;

  // does the client need a shape combine mask around it?
  bool hasShape(Client* c);
  // put an appropriate  shape combine mask around the client
  void setShape(Client* c);


  // auxiliary functions to travers all clients according the focus
  // order. Usefull for kwm큦 Alt-tab feature.
  Client* nextClient(Client* c);
  Client* previousClient(Client* c);

  // auxiliary functions to travers all clients according the static
  // order. Usefull for the CDE-style Alt-tab feature.
  Client* nextStaticClient(Client* c);
  Client* previousStaticClient(Client* c);
  Client* topClientOnDesktop();

  // returns wether a client with such a label is existing. Useful to
  // determine wether a label has to be unified with <2>, <3>, <4>,
  // ...
  bool hasLabel(QString label_arg);

  // syncs the window manager with the X server and returns a current
  // timestamp.
  Time timeStamp();

  // some commands

  // repaint everything and even force the clients to repaint thereselfs.
  void refreshScreen();
  // put a grey veil over the screen. Useful to show a nice logout
  // dialog.
  void darkenScreen();
  // do the X11R4 session management: send a SAVE_YOURSELF message to
  // everybody who wants to know it and wait for the answers. Also
  // handles KDE큦 session management additions as well as pseudo
  // session management with the help of a build in proxy.  After
  // finishing this functions the manager will emit a showLogout()
  // signal.
  void processSaveYourself();

  // commands from clients which can do session management
  QStrList* getSessionCommands();
  // clients which  can only be restarted
  QStrList* getPseudoSessionClients();
  // clients which cannot do session management at all
  QStrList* getNoSessionClients();
  // kwm supports an unsaved data hint: these clients set this hint.
  QStrList* getUnsavedDataClients();
  // returns all clients which are on the specified desktop. Used to
  // build a list of clients in a listbox.
  QStrList* getClientsOfDesktop(int desk);
  // returns all hints of the session management proxy
  QStrList* getProxyHints();
  // returns all properties  of the session management proxy
  QStrList* getProxyProps();

  // sets all necessary properties for the session management
  // proxy. This function is called from main after reading the data
  // from the configuration file.
  void setProxyData(QStrList* proxy_hints_arg, QStrList* proxy_props_arg,
		    QStrList* proxy_ignore_arg);

  // kwm's nifty kill feature (accessible with Ctrl-Alt-Escape)
  void killWindowAtPosition(int x, int y);

  // usefull helper function
  Client* clientAtPosition(int x, int y);

    // returns the client with the specified label or 0 if there is no
  // such client.
  Client* findClientByLabel(QString label);

  // kwm usually iconifies all transient windows of a client if the
  // client itself is iconified. Same applies for desktop switching:
  // transient windows are supposed to be always on the desktop as the
  // main window.
  void iconifyTransientOf(Client* c);
  void unIconifyTransientOf(Client* c);
  void ontoDesktopTransientOf(Client* c);
  void stickyTransientOf(Client* c, bool sticky);

  // if a window loses focus, then all floating windows are
  // automatically iconified. These are floating toolbars or menubars
  // (except in mac-like style when the menubar really sets the
  // KWM::standaloneMenuBar decoration). We do not need a special
  // deIconifyFloatingOf function, since floating windows are also
  // transient windows. That means that unIconifyTransientOf already
  // does this job.
  void iconifyFloatingOf(Client* c);


  // kwm sort of supports floating menubars. They are raised when
  // the parent window gets the focus.
  void raiseMenubarsOf(Client* c);

  // send a sound event to the kwm sound module. Same function as in
  // KWM from libkdecore.
  void raiseSoundEvent(const QString &);

  // Electric Border Window management. Electric borders allow a user
  // to change the virtual desktop by moving the mouse pointer to the
  // borders. Technically this is done with input only windows. Since
  // electric borders can be switched on and off, we have these two
  // functions to create and destroy them.
  void createBorderWindows();
  void destroyBorderWindows();
  // this function is called when the user entered an electric border
  // with the mouse. It may switch to another virtual desktop
  void electricBorder(Window border);

  // the manager object also reads a few properties from the
  // rc-file. This is called when the users wants to reconfigure.
  void readConfiguration();

  // kills all modules and launches another window manager
  void launchOtherWindowManager(const char* other);


signals:
  // this signal is emitted if the manager recieves a configure
  // command (for example from "kwmcom configure")
  void reConfigure();

private:

  // a manger keeps several lists of the client objets. in different orders:
  QList <Client> clients; // creation order
  QList <Client> clients_sorted; // stack order
  QList <Client> clients_traversing; // focus order
					
  // these string lists are used for the session management proxy
  QStrList* proxy_hints;
  QStrList* proxy_props;
  QStrList* proxy_ignore;

  // string lists for the session manager.
  QStrList additional_commands;
  QStrList additional_machines;
  QStrList additional_proxy_hints;
  QStrList additional_proxy_props;

  // a list of window titles (regular expression) which will not be
  // mangaged. This is usefull if a client likes to manage these
  // windows manually. For example kpanel when swallowing
  // something. Thanks to doNotManage kwm will not produce any
  // unnecessary flicker.
  QStrList do_not_manage_titles;

  // kwm keeps a list of sound events which have been registered by
  // modules. This list will be passed to a sound module then.
  QStrList sound_events;


  // help functions in the init process of the manager: scans the
  // desktop to detect already mapped windows. Necessary if kwm
  // appears after some windows.
  void scanWins();

  // Colormap handling
  Colormap default_colormap;
  void installColormap(Colormap cmap);
  void colormapFocus(Client *c);
  void getColormaps(Client *c);

  // access functions to standard X11 window properties. The results
  // will be stored in attributes of the passed client object.
  void getWMNormalHints(Client *c);
  void getWindowProtocols(Client *c);
  // this is for MWM hints (Motif window manager).
  void getMwmHints(Client  *c);

  // handles gravitation according to the gravity of the window.
  void gravitate(Client* c, bool invert);



  // internal tools

  // low level function to access an X11 property
  int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p);
  // easier access to an X11 property
  QString getprop(Window w, Atom a);
  // xgetprop is like getprop but can handle 0-separated string
  // lists. Will replace \0 with a blank then. Necessary for the XA_WM_COMMAND
  // property.
  QString xgetprop(Window w, Atom a);
  // a better getrop used for icon names etc. It can also handle
  // compound stuff. Seems like some old and weird programs rely on that.
  QString getTextProperty (Window w, Atom a);
  // kwm internally sometimes uses simple property (long values)
  bool getSimpleProperty(Window w, Atom a, long &result, Atom type = 0);
  // kwm internally sometimes uses rectangle properties
  void setQRectProperty(Window w, Atom a, const QRect &rect);
  // sends a client message a x to the window w
  void sendClientMessage(Window w, Atom a, long x);

  //internal tool for smartPlacement --- CT 18jan98 ---
  void spGetOverlap(Client* c, int x, int y, int* overlap);


  // Atoms
  Atom         wm_state;
  Atom         wm_change_state;
  Atom         wm_protocols;
  Atom         wm_delete_window;
  Atom         wm_take_focus;
  Atom         wm_save_yourself;
  Atom         kwm_save_yourself;
  Atom         wm_client_leader;
  Atom         wm_client_machine;
  Atom         wm_colormap_windows;
  Atom         _motif_wm_hints;
  Atom   kwm_current_desktop;
  Atom   kwm_number_of_desktops;
  Atom   kwm_active_window;
  Atom   kwm_win_iconified;
  Atom   kwm_win_sticky;
  Atom   kwm_win_maximized;
  Atom   kwm_win_decoration;
  Atom   kwm_win_icon;
  Atom   kwm_win_desktop;
  Atom   kwm_win_frame_geometry;
  Atom   kwm_win_icon_geometry;

  Atom kwm_command;
  Atom kwm_do_not_manage;
  Atom kwm_keep_on_top;
  Atom kwm_activate_window;
  Atom kwm_maximize_window;

  Atom kwm_window_region_changed;

  Atom kwm_running;

  Atom kde_sound_event;
  Atom kde_register_sound_event;
  Atom kde_unregister_sound_event;

   Atom qt_sizegrip;
   Atom qt_sizegrip2;

  // used to store the current desktop internally
  int current_desktop;

  // Modules

  // a list of all modules
  QList <Window> modules;
  // adds a new module to the list.
  void addModule(Window w);
  // removes a module from the list. If w is not a module
  // communication window then it does nothing.
  void removeModule(Window w);
  // send a messages to all modules. Usually these messages (stored in
  // the atom a) are about a certain client. This client is passed as
  // c. You may sometimes also want to pass a window directly. Then
  // pass 0 as client and the window as w.  Clients can have
  // hidden_for_modules set. In this case sendToModules will ignore
  // your request.
  void sendToModules(Atom a, Client* c, Window w = None);

  // a dock_module is a special kwm module which will handle the
  // docking area. Usually this is kpanel. There can only be one
  // docking area at one time.
  Window dock_module;
  // the list of docking windows which are on the docking area at
  // present.
  QList <Window> dock_windows;
  // adds a new dock window to the docking area. Informs the
  // dock_module about the change.
  void addDockWindow(Window w);
  // removes a new dock window from the docking area. Informs the
  // dock_module about the change. If w is no dock window it does
  // nothing.
  void removeDockWindow(Window w);
  // the list of external unmanaged toplevels that wants to stay on top
  QList <Window> top_windows;
  // adds a top window
  void addTopWindow(Window w);
  // removes a top window
  void removeTopWindow(Window w);
  Atom kwm_module;
  Atom module_init;
  Atom module_initialized;
  Atom module_desktop_change;
  Atom module_desktop_name_change;
  Atom module_desktop_number_change;
  Atom module_win_add;
  Atom module_dialog_win_add;
  Atom module_win_remove;
  Atom module_win_change;
  Atom module_win_raise;
  Atom module_win_lower;
  Atom module_win_activate;
  Atom module_win_icon_change;
  Atom module_dockwin_add;
  Atom module_dockwin_remove;

  // focus follow mouse is somewhat tricky. kwm does not do it so
  // simple that the focus is always under the mouse. Instead the
  // focus is only moved around if the user explitely moves the mouse
  // pointer.  This allows users to use the advanced alt-tab feature
  // in all focus policies. Also the desktop switching is able to
  // restore the focus correctly.
  bool enable_focus_follow_mouse_activation;
  Client* delayed_focus_follow_mouse_client;

  // electric borders
  Window current_border;
  int current_border_push_count;
  Window left_border;
  Window top_border;
  Window right_border;
  Window bottom_border;
  // indicates wether electric borders are switched off. If false then
  // the above mentioned windows are invalid.
  bool have_border_windows;

  // electric borders (input only windows) have to be always on the
  // top. For that reason kwm calls this function always after some
  // windows have been raised.
  void raiseElectricBorders();

  // decoration tricks to be somewhat compatible with traditional window managers
  QStrList no_decoration_titles;
  QStrList tiny_decoration_titles;
  QStrList no_decoration_classes;
  QStrList tiny_decoration_classes;
  // same for supressing focus
  QStrList no_focus_titles;
  QStrList no_focus_classes;

  //CT 03Nov1998 sticky windows (titles and classes)
  QStrList sticky_titles;
  QStrList sticky_classes;
  //CT

  // apply the above mentioned global hints
  void doGlobalDecorationAndFocusHints(Client* c);

   // make all menubars fit to the current desktop region
  void updateMenuBars();

  // make the passed menubar fit to the current desktop region
  void updateMenuBar(Client* c);

    // make the maximized windows fit to the current desktop region
  void updateMaximizedWindows();

  // optimization for standalone menubars
  bool has_standalone_menubars;
};

// CC: new KDE Greyer widget

// The greyer widget is used to put a grey veil over the screen in the
// darkenScreen method.
class KGreyerWidget : public QWidget
{
 Q_OBJECT
 public:
  KGreyerWidget();

 protected:
  void paintEvent(QPaintEvent *e);
};


#endif // MANAGER_H
