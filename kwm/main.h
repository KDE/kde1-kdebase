/*
 * main.h. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#ifndef MAIN_H
#define MAIN_H
#include <qapp.h>
#include <qpopmenu.h>
#include <qfont.h> //CT 03Nov1998
#include <kapp.h>
#include <qcolor.h>
#include <kglobalaccel.h>
#include <kmenubar.h>
#include "client.h"

class MyApp:public KApplication {
  Q_OBJECT
public:
  MyApp( int &argc, char **argv, const QString& rAppName );
  virtual bool x11EventFilter( XEvent * );

  bool buttonPressEventFilter( XEvent * ev);

  // extend KApp (this should move to kapp some day in the future...
  QColor activeTitleBlend;
  QColor inactiveTitleBlend;
  QFont  tFont; //CT 03Nov1998

  // store the session in the kwm configuration file
  void saveSession();
  // restore the session from the kwm configuration file
  void restoreSession();

  // clean up everything and exit
  void cleanup();

  // a popup menu which contains all possible window operations. For
  // performance and memory saving issues all clients use this object.
  QPopupMenu* operations;
  // a popup menu which contains all virtual desktops. For performance
  // and memory saving issues all clients use this object.
  QPopupMenu* desktopMenu;


    KMenuBar* systemMenuBar;
    QWidget* systemMenuBarParent;

    void setupSystemMenuBar();
    void removeSystemMenuBar();
    void resetSystemMenuBar();
    void raiseSystemMenuBar();

  // possible mouse bindings
  enum {
    MouseRaise, MouseLower, MouseOperationsMenu, MouseToggleRaiseAndLower,
    MouseActivateAndRaise, MouseActivateAndLower, MouseActivate,
    MouseActivateRaiseAndPassClick, MouseActivateAndPassClick,
    MouseMove, MouseResize, MouseNothing
  };

 public slots:

 // put the focus on the window with the specified label.  Will switch
 // to the appropriate desktop and eventually deiconiy the window
 void changeToClient(QString label);

 // Same as changeToClient above, but you can also specify names of
 // virtual desktops as label argument. Window names therefore have to
 // start with three blanks. changeToTaskClient is used in the current
 // session manager.
  void changeToTaskClient(QString label);


  // process the logout: save session and exit.
  void doLogout();

  // reread the kwm configuration files
  void reConfigure();

  // the overall functions for the configurable keybindings
  void createKeybindings();

  // returns a mouse binding for a given string
  int mouseBinding(const char*);

  // execute one of the configurable mousebindings. Returns true it the
  // keyevent does not need to be further processed
  bool  executeMouseBinding(Client* c, int command);

  void myProcessEvents();

  public slots:

  void slotTaskManager();
  void slotKillWindowMode();
  void slotExecuteCommand();
  void slotWindowOperations();
  void slotWindowRaise();
  void slotWindowLower();
  void slotWindowClose();
  void slotWindowIconify();
  void slotWindowResize();
  void slotWindowMove();
  void slotWindowToggleSticky();
  void slotKMenu();

  void slotSwitchOneDesktopLeft();
  void slotSwitchOneDesktopRight();
  void slotSwitchOneDesktopUp();
  void slotSwitchOneDesktopDown();

  void slotSwitchDesktop1();
  void slotSwitchDesktop2();
  void slotSwitchDesktop3();
  void slotSwitchDesktop4();
  void slotSwitchDesktop5();
  void slotSwitchDesktop6();
  void slotSwitchDesktop7();
  void slotSwitchDesktop8();

protected:
  bool eventFilter( QObject *, QEvent * );
  void  timerEvent( QTimerEvent * );

 private slots:
  // react on the operations QPopupMenu
  void handleOperation(int itemId);
  // react on the desktops QPopupMenu
  void handleDesktopPopup(int itemId);
  // react on key events for the current client
  void doOperation(int itemId);

private:
  void readConfiguration();
  void writeConfiguration();

  KGlobalAccel* keys;

  // handle key press events of kwm´s global key grabs
  bool handleKeyPress(XKeyEvent);
  // handle key release events of kwm´s global key grabs
  void handleKeyRelease(XKeyEvent);

  bool process_events_mode;
  XEvent events[50];
  int events_count ;

  int fileSystemMenuId;

};


void logout();
bool focus_grabbed();
void showMinicli();
void showTask();
// Like manager->activateClient but also raises the window and sends a
// sound event.
void switchActivateClient(Client*, bool do_not_raise = false);

#endif /* MAIN_H */
