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
#include <qintdict.h>

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
#include <qpixmap.h>

// --------- Sven's changes for macmode begin
class KMenuBar;
// --------- Sven's changes for macmode end;

enum {
  RMB_HELP=100,
  RMB_DISPLAY_PROPERTIES,
  RMB_REFRESH_DESKTOP,
  RMB_UNCLUTTER_WINDOWS,
  RMB_CASCADE_WINDOWS,
  RMB_ARRANGE_ICONS,
  RMB_LOCK_SCREEN,
  RMB_LOGOUT,
  RMB_EXECUTE,
  RMP_NEW,
  RMB_WMAKER_INFO,
  RMB_WMAKER_LEGAL,
  RMB_WMAKER_ARRANGE_ICONS,
  RMB_WMAKER_SHOW_ALL,
  RMB_WMAKER_HIDE_OTHER,
  RMB_WMAKER_RESTART,
  RMB_WMAKER_EXIT
};

class KRootWm: public QObject {
  Q_OBJECT

public:
  KRootWm(KWMModuleApplication*);
  bool eventFilter( QObject *, QEvent * );

public slots:
  void kwmCommandReceived(QString com);

private:
  KWMModuleApplication* kwmmapp;

  GC gc;
  void draw_selection_rectangle(int x, int y, int dx, int dy);
  bool select_rectangle(int &x, int &y, int &dx, int &dy);

  //void generateWindowlist(QPopupMenu* p); sven -  moved to slots
  Window* callbacklist;

  QPopupMenu* mmb;
  QPopupMenu* rmb;
  QPopupMenu* menuNew;
  QPopupMenu *bookmarks;

  QStrList templatesList;
  QString desktopPath;
  QString templatePath;

  bool kpanel_menu_on_left_button;

  void updateBookmarkMenu (void);
  void updateNewMenu (void);//David
  void buildMenubars(void);//CT
  void scanBookmarks( QPopupMenu*, const char *_dir );
  int bookmarkId;
  QIntDict<QString> bookmarkDict;

  // --------- Sven's changes for macmode begin
  bool macMode;
  KMenuBar *myMenuBar;
  QWidget *myMenuBarContainer;

  QPopupMenu *file;
  QPopupMenu *desk;
  QPopupMenu *help;
  // --------- Sven's changes for macmode end

  QPixmap defaultPixmap;
    
    bool wmakerMode;

private slots:
  void slotNewFile( int _id );
  void slotBookmarkSelected( int _id );

  void rmb_menu_activated(int);
  void mmb_menu_activated(int);
  // --------- Sven's changes for macmode begin
  void slotFocusChanged(Window);
  void generateWindowlist();
  // --------- Sven's changes for macmode end
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

