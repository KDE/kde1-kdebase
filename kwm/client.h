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
#include <qframe.h>
#include <qpushbt.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpopmenu.h>
#include <qtimer.h>

#include "options.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


enum {
  OP_MAXIMIZE = 5000,
  OP_RESTORE,
  OP_ICONIFY,
  OP_MOVE,
  OP_RESIZE,
  OP_CLOSE,
  OP_STICKY
};


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





class Client : public QFrame {

  Q_OBJECT
  
public:
  Client( Window w, QWidget *parent=0, const char *name=0 );
  ~Client();
   
  Window      window;
  
  Window      trans;
  long      decoration;
  bool      decoration_not_allowed;
  
  long getDecoration(){
    return decoration;
  }

  int         desktop;
  
  QRect geometry;

  void adjustSize();

  int         border;
  
  // for the unmaximize function
  QRect geometry_restore;


  QPoint old_cursor_pos;
  
  
  XSizeHints  size;
  
  int         state;
  bool         reparenting;
  bool   backing_store;

  // standard window manager protocols
  bool Pdeletewindow;
  bool Ptakefocus;
  bool Psaveyourself;
  
  bool isActive(){
    return is_active;
  }
  
  QString label;
  QString name;
  QString iconname;
  QString instance;
  QString klass;
  QString command;
  QString machine;

  
  Colormap    cmap;
  int         ncmapwins;
  Window      *cmapwins;
  Colormap    *wmcmaps;

  myPushButton   *buttons[6];
  myPushButton   *buttonMaximize;
  myPushButton   *buttonSticky;
  myPushButton   *buttonMenu;
  myPushButton * getNewButton( BUTTON_FUNCTIONS ); 

  // for the titlebar
  QRect title_rect;


  bool isOnDesktop(int desktop_arg){
    return desktop == desktop_arg || isSticky();
  }
  bool isSticky(){
    return buttonSticky->isOn();;
  }
  bool isMaximized(){
    return maximized;
  }
  bool isWithdrawn(){
    return state == WithdrawnState;
  }
  bool isIconified(){
    return iconified;
  }

  bool fixedSize();
   
  bool dragging_is_running();

  void setactive(bool on);
  void paintState(bool only_label = FALSE);

  void setLabel();

  void generateOperations();
  void showOperations();
  void setCursor();

  void handleOperationsPopup(int);
  void simple_move();
  void simple_resize();

  void reconfigure();
  void animateTitlebar();

  bool maximized;
  bool iconified;
  
  void unIconify(bool animation = True);
  void iconify(bool animation);
  void ontoDesktop(int new_desktop);
  void maximize(int mode = 0);
  void unMaximize();

  bool hidden_for_modules;

  void stopAutoraise();

 public slots:
   void iconify();
   void maximizeToggled(bool);
   void closeClicked();
   void stickyToggled(bool);
   void menuPressed();

signals:
  void clientClosed( int );
   
protected:
  void mousePressEvent( QMouseEvent * );
  void mouseReleaseEvent( QMouseEvent * );
  void mouseDoubleClickEvent( QMouseEvent * );
  void mouseMoveEvent( QMouseEvent * );
  void enterEvent( QEvent * );
  void leaveEvent( QEvent * );
  void paintEvent( QPaintEvent * );
  void resizeEvent( QResizeEvent * );

private:
  
  void generateButtons();
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

    void set_x_cursor(Cursor cur);
    Cursor current_cursor;
    

  int titlestring_offset_delta;
  int titlestring_offset;
  bool animation_is_active;

  bool is_active;
  
  bool autoraised_stopped;

private slots:
  void autoRaise();
    
};


QPixmap* loadIcon(const char* name);
void resizedrag(Client *c, int mode);
void movedrag(Client *c);
void draw_animation_rectangle(int x, int y, int dx, int dy, bool decorated, int o1, int o2);
void killSelect();


#endif // CLIENT_H
