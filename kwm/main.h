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
#include "client.h"


class MyApp:public KApplication {
  Q_OBJECT
  public:
    MyApp( int &argc, char **argv, const QString& rAppName );
    virtual bool x11EventFilter( XEvent * );
    void saveSession();
    void restoreSession();

    void cleanup();

    QPopupMenu* operations;
    QPopupMenu* desktopMenu;

  public slots:
    void changeToClient(QString label);
    void changeToTaskClient(QString label);
    void doLogout();
    void reConfigure();
    void showLogout();

  protected:
    bool eventFilter( QObject *, QEvent * );
    void  timerEvent( QTimerEvent * );
    
  private slots:
    void handleOperationsPopup(int itemId);
    void handleDesktopPopup(int itemId);

  private:   
    void readConfiguration();
    void writeConfiguration();

    bool handleKeyPress(XKeyEvent);
    void handleKeyRelease(XKeyEvent);

};

	
void logout();

#endif /* MAIN_H */
