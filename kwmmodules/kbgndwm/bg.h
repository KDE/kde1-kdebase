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

class KBackground
{
public:
    KBackground();
    ~KBackground();

    void apply();
    void readSettings( const char *group );

    const QString &getName() const
	{ return name; }

    bool hasPixmap()
	{ return hasPm; }

protected:
    QPixmap *loadWallpaper();

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

    bool    hasPm;
};

//----------------------------------------------------------------------------

#endif

