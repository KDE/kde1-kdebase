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


class Manager : public QObject {
Q_OBJECT
public:

  Manager();
  ~Manager(){};

  // events

  void configureRequest(XConfigureRequestEvent *e);
  void mapRequest(XMapRequestEvent *e);
  void circulateRequest(XCirculateRequestEvent */* e */ ){};

  void unmapNotify(XUnmapEvent *e);
  void destroyNotify(XDestroyWindowEvent *e);
  void clientMessage(XClientMessageEvent *e);
  void colormapNotify(XColormapEvent *e);
  void propertyNotify(XPropertyEvent *e);
  void shapeNotify(XShapeEvent *e);
  void enterNotify(XCrossingEvent *e);
    
  void randomPlacement(Client* c);
  
  void manage(Window w, bool mapped = False);

  void withdraw(Client* c);

  Client* getClient(Window w);
  Client* current();

  void addClient(Client* c);
  void activateClient(Client* c, bool set_revert = TRUE);
  void removeClient(Client* c);
  void raiseClient(Client* c);
  void lowerClient(Client* c);
  void closeClient(Client* c);
  void changedClient(Client* c);

  void noFocus();


  void setWindowState(Client *c, int state);
  void getWindowTrans(Client *c);

  void switchDesktop(int);
  int currentDesktop(){
    return current_desktop;
  }
  int number_of_desktops;

  void sendConfig(Client* c, bool emit_changed = TRUE);

  void cleanup();

  void repaintAll();

  GC gc;
  // shaped windows
  int shape;
  int shape_event;
  void setShape(Client* c);


  Client* nextClient(Client* c);
  Client* previousClient(Client* c);

  bool hasLabel(QString label_arg);

  Time timeStamp();

  // some commands
  void refreshScreen();
  void darkenScreen();
  void logout();

  QStrList* getSessionCommands();
  QStrList* getPseudoSessionClients();
  QStrList* getNoSessionClients();
  QStrList* getUnsavedDataClients();
  QStrList* getClientsOfDesktop(int desk);
  QStrList* getProxyHints();
  QStrList* getProxyProps();

  void setProxyData(QStrList* proxy_hints_arg, QStrList* proxy_props_arg);

  void killWindowAtPosition(int x, int y);

  Client* findClientByLabel(QString label);
  
  void iconifyTransientOf(Client* c);
  void unIconifyTransientOf(Client* c);

signals:
  void reConfigure();
  void showLogout();

private:
  QList <Client> clients; // creation order
  QList <Client> clients_sorted; // stack order
  QList <Client> clients_traversing; // focus order
					  
  QStrList* proxy_hints;
  QStrList* proxy_props;

  QStrList additional_commands;
  QStrList additional_machines;
  QStrList additional_proxy_hints;
  QStrList additional_proxy_props;
   

  void scanWins();

  // Colormaps
  Colormap default_colormap;
  void installColormap(Colormap cmap);
  void colormapFocus(Client *c);
  void getColormaps(Client *c);
  

  void getWMNormalHints(Client *c);
  void getWindowProtocols(Client *c);
  void getMwmHints(Client  *c);

  void gravitate(Client* c, bool invert);



  // internal tools
  int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p);
  QString getprop(Window w, Atom a);
  bool getSimpleProperty(Window w, Atom a, long &result);
  void setQRectProperty(Window w, Atom a, const QRect &rect);
  void sendClientMessage(Window w, Atom a, long x);

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

  Atom kwm_command;
  Atom kwm_activate_window;

  Atom kwm_running;


  int current_desktop;

  // Modules
  QList <Window> modules;
  void addModule(Window w);
  void removeModule(Window w);
  void sendToModules(Atom a, Window w);
  Window dock_module;
  QList <Window> dock_windows;
  void addDockWindow(Window w);
  void removeDockWindow(Window w);
  Atom kwm_module;
  Atom module_init;
  Atom module_desktop_change;
  Atom module_desktop_name_change;
  Atom module_desktop_number_change;
  Atom module_win_add;
  Atom module_win_remove;
  Atom module_win_change;
  Atom module_win_raise;
  Atom module_win_lower;
  Atom module_win_activate;
  Atom module_win_icon_change;
  Atom module_dockwin_add;
  Atom module_dockwin_remove;

  
  


};

#endif // MANAGER_H
