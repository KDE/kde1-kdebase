/*
 * krootwm.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 * Modified by Torben Weis, weis@kde.org
 * to add the "New" menu
 */

#include <qapp.h>
#include <qcursor.h>
#include <qlist.h>
#include <qstring.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <qwidget.h>
#include <qpopmenu.h>
#include <qstrlist.h>
#include <kwmmapp.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <klined.h>
#include "pmenu.h"

enum {
  RMB_HELP=100,
  RMB_DISPLAY_PROPERTIES,
  RMB_REFRESH_DESKTOP,
  RMB_ARRANGE_ICONS,
  RMB_LOCK_SCREEN,
  RMB_LOGOUT,
  RMB_EXECUTE,
  RMP_NEW
};

class KRootWm: public QObject {
  Q_OBJECT

public:
  KRootWm(KWMModuleApplication*);
  bool eventFilter( QObject *, QEvent * );

private:
  KWMModuleApplication* kwmmapp;

  GC gc;
  void draw_selection_rectangle(int x, int y, int dx, int dy);
  bool select_rectangle(int &x, int &y, int &dx, int &dy);
  
  void generateWindowlist(QPopupMenu* p);
  Window* callbacklist;

  QPopupMenu* mmb;
  QPopupMenu* rmb;
  QPopupMenu* menuNew;
    
  QStrList templatesList;
  QString desktopPath;
  QString templatePath;

  PMenu *pmenu;
      
private slots:
  void slotNewFile( int _id );
  void rmb_menu_activated(int);
  void mmb_menu_activated(int);
};


/**
 * @sort Asking for a single line of text
 * This class can be used to ask for a new filename or for
 * an URL.
 *
 * (c) Torben Weis, weis@kde.org 1997
 */
class DlgLineEntry : public QDialog
{
    Q_OBJECT
public:
    /**
     * Create a dialog that asks for a single line of text. _value is the initial
     * value of the line. _text appears as label on top of the entry box.
     */
    DlgLineEntry( const char *_text, const char *_value, QWidget *parent );
    ~DlgLineEntry();

    /**
     * @return the value the user entered
     */
    const char * getText() { return edit->text(); }
    
public slots:
    /**
     * The slot for clearing the edit widget
     */
    void slotClear();

protected:
    /**
     * The line edit widget
     */
    QLineEdit *edit;
};

