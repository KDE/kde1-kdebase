/*
 * bg.h.  Part of the KDE Project.
 *
 * Copyright (C) 1997 Martin Jones
 *               1998 Matej Koss
 *
 * $Id$
 *
 */

#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

#include <qcolor.h>
#include <qpixmap.h>
#include <qtimer.h>

//----------------------------------------------------------------------------

class KBackground : public QObject
{
  Q_OBJECT
public:
  KBackground();
  ~KBackground();

  void apply();
  // cancel an apply which has not yet been completed
  void cancel();
  void readSettings( int num, bool one, int desknum );

  const QString &getName() const
    { return name; }

  bool hasPixmap() const
    { return hasPm; }
  bool isApplied() const
    { return applied; }

  void setOneDesktop( bool one, int onedesk ) {
    oneDesktopMode = one;
    oneDesk = onedesk;
  }

  void setImmediately( const char* _wallpaper, int mode );

    void doRandomize(bool fromTimer = FALSE);
public slots:
  void randomize();

protected:
  QPixmap *loadWallpaper();
  virtual void timerEvent( QTimerEvent * );
  void setPixmapProperty( QPixmap * );

protected:
  enum { Tiled = 1,
	 Mirrored,
	 CenterTiled,
	 Centred,
	 CentredBrick,
	 CentredWarp,
	 CentredMaxpect,
	 SymmetricalTiled,
	 SymmetricalMirrored,
	 Scaled };
  enum { Flat = 1, Gradient, Pattern };
  enum { Portrait = 1, Landscape };

  QString name;

  QString wallpaper;
  QColor  color1;
  QColor  color2;
  int     wpMode;
  int     gfMode;
  int     orMode;

  QTimer *timerRandom;

  uint pattern[8];
  QPixmap *bgPixmap;
  bool    applied;

  bool    hasPm;
  bool bUseWallpaper;

  bool randomMode;
  int randomDesk;

  bool oneDesktopMode;
  int oneDesk;

  bool useDir;

  int desk;
};

//----------------------------------------------------------------------------

#endif
