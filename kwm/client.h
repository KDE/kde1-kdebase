/*
 * client.h. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <qapp.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpopmenu.h>
#include <qtimer.h>
#include <kpixmap.h>

#include "options.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


// the possible operations from the window operations popup menu
enum {
  OP_MAXIMIZE = 5000,
  OP_RESTORE,
  OP_ICONIFY,
  OP_MOVE,
  OP_RESIZE,
  OP_CLOSE,
  OP_STICKY,
  OP_SHADE,
  OP_OPERATIONS
};


// a QPushButton subclass to allow flat buttons which raise when the
// mouse pointer is over them. Very similar to the toolbar buttons in
// ktoolbar.
class myPushButton: public QPushButton
{
  Q_OBJECT
public:
  myPushButton ( QWidget *parent=0, const char* name=0 );
  ~myPushButton () {}
  bool flat;
  int last_button;
protected:
  void enterEvent( QEvent * );
  void leaveEvent( QEvent * );
  void mousePressEvent( QMouseEvent *e);
  void mouseReleaseEvent( QMouseEvent *e);
  void mouseMoveEvent( QMouseEvent *e);
  void paint( QPainter *_painter );
  void drawButton( QPainter *p ){paint(p);}
  void drawButtonLabel( QPainter *p ){paint(p);}
};



class Client;


// the Client class encapsulates a window decoration. There´s one
// instance of Client for each managed window.
class Client : public QWidget {

  Q_OBJECT

public:
  Client( Window w, Window _sizegrip = None, QWidget *parent=0, const char *name=0 );
  ~Client();

  // the X11 handle of the managed window
  Window      window;

    // the Qt resize grip
    Window       sizegrip;

    // show the client. This means the decoration frame and the inner window
  void showClient();
  // hide the client. This means the decoration frame and the inner window
  void hideClient();

  //  the number of unmap events produced by us in hideClient.
  // These will be ignored in Manager::unmapNotify
  int unmap_events;



  // TODO decoration property handling must be cleaned-up!!
  Window      trans;
  Window      leader;
  long      decoration;
  bool wants_focus;
  bool is_menubar;

  // returns the clean decoration hint. Can be KWM::noDecoration,
  // KWM::tinyDecoration or KWM::normalDecoration
  long getDecoration(){
    return decoration;
  }

  // some windows do not want the focus
  bool wantsFocus(){
    return wants_focus;
  }

  // some windows set the menubar hint
  bool isMenuBar(){
    return is_menubar;
  }

  // the virtual desktop on which this client is located
  int         desktop;

  // the should-be geometry of this client. Will be applied to the
  // client itself in Manager::sendConfig.
  QRect geometry;

  // an X11 window manager handles a lot of size hints, like minimum
  // size or fixed increment values. adjustSize() will modify the
  // geometry to fullfil the criteria.
  void adjustSize();
  // the XSizeHints store all informations needed by adjustSize
  XSizeHints  size;


  // XWindows can specifiy window specific borders. More or less
  // ignored in kwm, but stored anyway.
  int         border;

  // The geometry of this client before a call of maximize. The
  // unmaximize function will restore the window to that geometry.
  QRect geometry_restore;

  // a help variable for window movement and resizing
  QPoint old_cursor_pos;



  // the window state( IconicState, NormalState, WithdrawnState)
  int         state;

  // stores the backing_store hint for the X Server.
  bool   backing_store;

  // standard window manager protocols
  bool Pdeletewindow; // does the window understands the DeleteWindow protocol?
  bool Ptakefocus;// does the window understands the TakeFocus protocol?
  bool Psaveyourself;// does the window understands the SaveYourself protocol?

  // returns wether the window has the focus
  bool isActive(){
    return is_active;
  }


  // a couple of string properties

  QString label; // the window label (shown in the titlebar)
  QString name; // the window name
  QString iconname; // the icon name
  QString instance; // the instance name
  QString klass; // the class name
  QString command; // XA_WM_COMMAND property (session management)
  QString machine; // WM_CLIENT_MACHINE property (session management)


  // some stuff for colormap handling
  Colormap    cmap;
  int         ncmapwins;
  Window      *cmapwins;
  Colormap    *wmcmaps;

  // pointers to the pushbuttons of the window decoration. May be 0.
  myPushButton   *buttons[6];

  // special pointers to some of the window operation buttons. These
  // can be located everywhere in the buttons array
  myPushButton   *buttonMaximize;
  myPushButton   *buttonSticky;
  myPushButton   *buttonMenu;

  // create a new pushbutton associated with the specified button
  // functions.
  myPushButton * getNewButton( BUTTON_FUNCTIONS );

  // the rectangle which represents the titelbar (where the window
  // label is drawn)
  QRect title_rect;


  // is the window on the specified desktop? Takes care about sticky
  // windows, too.
  bool isOnDesktop(int desktop_arg){
    return desktop == desktop_arg || isSticky();
  }
  // is the window maximized?
  bool isMaximized(){
    return maximized;
  }
  // is the window sticky?
  bool isSticky(){
    return sticky;
  }

  // is the window in withdrawn state?
  bool isWithdrawn(){
    return state == WithdrawnState;
  }
  // is the window in iconic state?
  bool isIconified(){
    return iconified;
  }
  // is the window shaded? (only titlebar visible)
  bool isShaded(){
    return shaded;
  }

    // is the window supposed to stay on top
  bool isStaysOnTop(){
      return stays_on_top;
  }

  // windows can have a fixed size, that means the window manager must
  // not resize them in any way.
  bool fixedSize();

  // indicates wether the user is currently dragging a window around.
  bool dragging_is_running();

  // activate or deactivate a window.
  void setactive(bool on);
  // Repaint the state of a window. Active or inactive windows differ
  // only in the look of the titlebar. If only_label is true then only
  // the label string is repainted. This is used for the titlebar
  // animation with animate = TRUE.
  void paintState(bool only_label = FALSE, bool colors_have_changed = FALSE, bool animate = FALSE);

  // generates a sensefull label property from the name, icon, klass
  // and instance. Also ensures that the label is unique by adding a
  // number in brackets after the name.
  void setLabel();

  // changes the global window operations popup to fit to this
  // client. This means activating/deactivating the items
  // appropriately and setting the right virual desktop.
  void generateOperations();

  //  a static pointer used to indicate which client defined the
  //  operation popup at present.
  static Client* operation_client;

  // shows the operations popup at the right place near the menu button
  void showOperations();

  // returns the id of an kwm command from a given string. (or -1 if
  // there is no such command)
  static int operationFromCommand(const QString &com);

  // handles a window operation
  void handleOperation(int);

  // set up everything to handle a move (used from Alt-LMB)
  void simple_move();
  // set up everything to handle a resize (used from Alt-RMB)
  void simple_resize();

  // the global options have changed => update the titlebar settings.
  void reconfigure();

  // if the titlestring is too large kwm can move it around (titlebar
  // animation). animateTilebar() is invoked from a timer and will
  // simply move it one single step further in this case.
  void animateTitlebar();

  // attributes to store the current state of the client
  bool maximized;
  int maximize_mode;
  bool sticky;
  bool iconified;
  bool shaded;
  bool shaped;
  bool stays_on_top;

  // unIconify this client. Takes care about floating or transient
  // windows. If animation is true, kwm may show some kind of
  // animation (if set by the options)
  void unIconify(bool animation = True);
  // iconify this client. Takes care about floating or transient
  // windows. If animation is true, kwm may show some kind of
  // animation (if set by the options)
  void iconify(bool animation = True);
  // move the client onto a new desktop. Will take floating and
  // transient windows with it.
  void ontoDesktop(int new_desktop);


  enum {horizontal = 1, vertical = 2, fullscreen = 3};
  // maximize this client. Mode can be horizontal, vertical or fullscreen.
  // Store the current geometry in geometry_restore
  void maximize(int mode = fullscreen, bool animate = True);
  // unmaximize this client. Geometry will be as it was before
  // (geometry_restore)
  void unMaximize();

  // shade or unshade this client. "shaded" means that only the
  // titlebar is visible.
  void toggleShade();

  // a flag to indicate wether this client is hidden for modules. If
  // true then kwm will never send a message to any module telling
  // them about this window.
  bool hidden_for_modules;

  // stops the autoraise timer for this client. If do_raise is TRUE then
  // the client might be raised for focus follows mouse policies *if*
  // the ClickRaise option is set.
  void stopAutoraise(bool do_raise = true);

  // returns the client itself it is not transient. If it is transient
  // it will return the main window recursively.
  Client* mainClient();

    // returns whether the client is only a dialog window
    //#### uses build-in iterator ot the manager's client list!!
  bool isDialog() {
      return mainClient() != this;
  }


 public slots:
 // these slots are connected to the buttons from the window
 // decorations.
   void iconifySlot();
   void maximizeToggled(bool);
   void closeClicked();
   void stickyToggled(bool);
   void menuPressed();
   void menuReleased();

protected:
  // Some of Qt´s syntactic events with which we will have to deal
  void mousePressEvent( QMouseEvent * );
  void mouseReleaseEvent( QMouseEvent * );
  void mouseDoubleClickEvent( QMouseEvent * );
  void mouseMoveEvent( QMouseEvent * );
  void enterEvent( QEvent * );
  void leaveEvent( QEvent * );
  void paintEvent( QPaintEvent * );
  void resizeEvent( QResizeEvent * );



private:

  // generate the window decoration buttons according to the settings in the options
  void generateButtons();
  // layout the window decoration buttons. Necessary after the window
  // changed its size, for example.
  void layoutButtons();

     // for dragging
    enum DRAGGING {
      dragging_enabled,
      dragging_smooth_wait,
      dragging_nope,
      dragging_runs,
      dragging_resize_wait,
      dragging_resize
    };
    DRAGGING dragging_state;
    int do_resize;

    int mouse_release_command; //hack

    // set the mouse pointer shape to the specified cursor. Also
    // stores the defined shaped in current_cursor.
    void set_x_cursor(Cursor cur);
    // current mouse pointer shape
    Cursor current_cursor;


  // if the titlestring is too large (titlestring_too_large == true)
  // kwm can do a nifty animation.
  int titlestring_offset_delta;
  int titlestring_offset;
  bool titlestring_too_large;
  int titlestring_delay;

  // used to store the activation state (do we have the focus?)
  // internally
  bool is_active;

  // do_close is used to fake some kind of doubleclick event with the
  // menu button.
  bool do_close;

  // used to indicate that somebody stopped auto raising via stopAutoraise().
  bool autoraised_stopped;

  // put a passive grab above the window for some button/modifier combinations.
  void doButtonGrab();

  // we called grabMouse() but cannot be sure that we really got the
  // grab! IMO this is a qt problem. Anyway this function solves it:
  // it will wait until we _really_ have the pointer grab.
  void ensurePointerGrab();

  // a buffer for the vertical shading
  KPixmap aShadepm;
  KPixmap iaShadepm;

  // the gimmick widget
  static QLabel* gimmick;
 public:
  static void createGimmick();
  void placeGimmick();
  static void hideGimmick();
  static Window gimmickWindow(){
    return gimmick?gimmick->winId():None;
  }

private slots:
  // this slot is connect with a singleshot timer when we are doing
  // auto raising (related to the focus follow mouse policy). If
  // autoraised_stopped is true it will do nothing.  It will also
  // check wether raising would cover a popup menu and avoid it in
  // such a case.
  void autoRaise();

};


// simple encapsulation of the KDE iconloader
QPixmap* loadIcon(const char* name);

// resize drag. Implemented in drag.C, mode specifies the corner or
// edge on which the user drags.
bool resizedrag(Client *c, int mode);
// move drag. Implemented in drag.C
bool movedrag(Client *c);
// used to draw that nifty animation rectangle. x,y,dx,dy specify the
// size of the window. If decorated is true than the function will
// also draw a skedge of a decorated window. In this case o1 and o2
// represent the left resp. right border of the titlebar rectangle.
void draw_animation_rectangle(int x, int y, int dx, int dy, bool decorated, int o1, int o2);

// shows a skull and lets the user select a window that will be killed
// with manager->killWindowAtPosition() later. Implemented in drag.C
void killSelect();


#endif // CLIENT_H
