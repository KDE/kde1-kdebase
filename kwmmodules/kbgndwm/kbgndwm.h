/*
 * kbgndwm.h.  Part of the KDE Project.
 *
 * Copyright (C) 1997 Martin Jones
 *
 */

#ifndef __KBGNDWM_H__
#define __KBGNDWM_H__

//----------------------------------------------------------------------------

#include <qapp.h>

#include <stdlib.h>
#include <stdio.h>

#include <kwmmapp.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include "bg.h"

//----------------------------------------------------------------------------

#define MAX_DESKTOPS 8

//----------------------------------------------------------------------------

class KBGndManager: public QObject
{
    Q_OBJECT
public:
    KBGndManager( KWMModuleApplication * );
protected:
    void applyDesktop( int d );
    void cacheDesktop();
    void readSettings();

public slots:
    void desktopChange( int );
    void commandReceived( QString );

private:
    KWMModuleApplication* kwmmapp;

    KBackground *desktops;
    int current;
};

//----------------------------------------------------------------------------

#endif
