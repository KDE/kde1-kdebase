/*
 * bg.h.  Part of the KDE Project.
 *
 * Copyright (C) 1997 Martin Jones
 *
 */

#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

#include <qcolor.h>
#include <qpixmap.h>

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
    void readSettings( const char *group );

    const QString &getName() const
	{ return name; }

    bool hasPixmap() const
	{ return hasPm; }
    bool isApplied() const
	{ return applied; }

protected:
    QPixmap *loadWallpaper();
    virtual void timerEvent( QTimerEvent * );

protected:
    enum { Tiled = 1, Centred, Scaled };
    enum { Flat = 1, Gradient };
    enum { Portrait = 1, Landscape };

    QString name;

    QString wallpaper;
    QColor  color1;
    QColor  color2;
    int     wpMode;
    int     gfMode;
    int     orMode;

    QPixmap *bgPixmap;
    bool    applied;

    bool    hasPm;
};

//----------------------------------------------------------------------------

#endif

