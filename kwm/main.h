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
#include <kapp.h>
#include <qcolor.h>
#include "client.h"


class MyApp:public KApplication {
  Q_OBJECT
public:
  MyApp( int &argc, char **argv, const QString& rAppName );
  virtual bool x11EventFilter( XEvent * );

  // extend KApp (this should move to kapp some day in the future...
  QColor activeTitleBlend;
  QColor inactiveTitleBlend;

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

  // show the modal logout dialog
  void showLogout();
  
protected:
  bool eventFilter( QObject *, QEvent * );
  void  timerEvent( QTimerEvent * );
  
 private slots:
  // react on the operations QPopupMenu
  void handleOperation(int itemId);
  // react on the desktops QPopupMenu
  void handleDesktopPopup(int itemId);
  
private:   
  void readConfiguration();
  void writeConfiguration();
  
  // handle key press events of kwm´s global key grabs
  bool handleKeyPress(XKeyEvent);
  // handle key release events of kwm´s global key grabs
  void handleKeyRelease(XKeyEvent);
  
};


void logout();

#endif /* MAIN_H */
