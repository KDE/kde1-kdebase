/*
 * main.h. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <qpixmap.h>
#include <stdio.h>

#ifndef OPTIONS_H
#define OPTIONS_H

extern int BORDER;
#define BORDER_THIN 4
#define TITLEWINDOW_SEPARATION  2
#define BUTTON_SIZE    20
#define TITLEBAR_HEIGHT 20


enum WINDOW_MOVE_TYPE {
  TRANSPARENT,
  OPAQUE
};

enum FOCUS_POLICY {
  CLICK_TO_FOCUS,
  FOCUS_FOLLOWS_MOUSE,
  CLASSIC_FOCUS_FOLLOWS_MOUSE,
  CLASSIC_SLOPPY_FOCUS
};

enum TITLEBAR_LOOK{
  PLAIN,
  H_SHADED,
  V_SHADED,
  PIXMAP
};

enum BUTTON_FUNCTIONS {
 MAXIMIZE,
 ICONIFY,
 CLOSE,
 STICKY,
 MENU,
 NOFUNC,
 UNDEFINED
};


//CT 18jan98, 07mar98
enum PLACEMENT_POLICY {
  SMART_PLACEMENT,
  CASCADE_PLACEMENT,
  RANDOM_PLACEMENT,
  INTERACTIVE_PLACEMENT,
  MANUAL_PLACEMENT
};

enum ELECTRIC_BORDER_POINTER_WARP {
  FULL_WARP,
  MIDDLE_WARP,
  NO_WARP
};

enum ALT_TAB_MODE{
  KDE_STYLE,
  CDE_STYLE
};

enum ALIGN_TITLE {
  AT_LEFT,
  AT_MIDDLE,
  AT_RIGHT
};

struct kwmOptions {

  WINDOW_MOVE_TYPE WindowMoveType;
  WINDOW_MOVE_TYPE WindowResizeType;
  FOCUS_POLICY FocusPolicy;
  ALT_TAB_MODE AltTabMode;
  TITLEBAR_LOOK TitlebarLook;
  //CT 04Nov1998 - titlebar align
  ALIGN_TITLE alignTitle;
  //CT
  int ResizeAnimation;
  bool MaximizeOnlyVertically;
  int TitleAnimation;
  int AutoRaiseInterval;
  bool AutoRaise;
  int ElectricBorder;
  int ElectricBorderNumberOfPushes;
  ELECTRIC_BORDER_POINTER_WARP ElectricBorderPointerWarp;
  bool ControlTab;
  bool Button3Grab;
  //CT 18jan98, 07mar98, 17mar98
  PLACEMENT_POLICY Placement;
  int interactive_trigger;
  int BorderSnapZone;
  int WindowSnapZone;
  //CT ---

  const char* rstart;
  int titlebar_doubleclick_command;

  // mouse bindings
  int CommandActiveTitlebar1;
  int CommandActiveTitlebar2;
  int CommandActiveTitlebar3;
  int CommandInactiveTitlebar1;
  int CommandInactiveTitlebar2;
  int CommandInactiveTitlebar3;
  int CommandWindow1;
  int CommandWindow2;
  int CommandWindow3;
  int CommandAll1;
  int CommandAll2;
  int CommandAll3;


  bool TraverseAll;

  BUTTON_FUNCTIONS buttons[6];

  QPixmap* titlebarPixmapActive;
  QPixmap* titlebarPixmapInactive;

  bool ShapeMode;
  QPixmap* shapePixmapTop;
  QPixmap* shapePixmapLeft;
  QPixmap* shapePixmapBottom;
  QPixmap* shapePixmapRight;
  QPixmap* shapePixmapTopLeft;
  QPixmap* shapePixmapTopRight;
  QPixmap* shapePixmapBottomLeft;
  QPixmap* shapePixmapBottomRight;
  bool GimmickMode;
  int GimmickPositionX;
  int GimmickPositionY;
  int GimmickOffsetX;
  int GimmickOffsetY;
  QPixmap* gimmickPixmap;
};


extern kwmOptions options;

#define CLASSIC_FOCUS  (options.FocusPolicy == CLASSIC_FOCUS_FOLLOWS_MOUSE \
			|| options.FocusPolicy == CLASSIC_SLOPPY_FOCUS)


// #define DEBUG_EVENTS_ENABLED

#ifdef DEBUG_EVENTS_ENABLED
#define DEBUG_EVENTS(x,y) debug_events(x,y);
#define DEBUG_EVENTS2(x,y,z) debug_events(x,y,z);
     void debug_events(const char* s, long int l);
     void debug_events(const char* s, void* v, long int l=0);
#else
#define DEBUG_EVENTS(x,y)
#define DEBUG_EVENTS2(x,y,z)
#endif

#endif // OPTIONS_H

