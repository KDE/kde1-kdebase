/*
 * main.h. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <qpixmap.h>

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
  SHADED,
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

struct kwmOptions {

  WINDOW_MOVE_TYPE WindowMoveType;
  WINDOW_MOVE_TYPE WindowResizeType;
  FOCUS_POLICY FocusPolicy;
  TITLEBAR_LOOK TitlebarLook;
  bool ResizeAnimation;
  bool MaximizeOnlyVertically;
  int TitleAnimation;
  int AutoRaise;
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


#endif // OPTIONS_H

