    /*

    Configuration for kdm. Class KDMConfig
    $Id: kdmconfig.h,v 1.8 1999/01/28 14:10:01 kulow Exp $

    Copyright (C) 1997, 1998 Steffen Hansen
                             stefh@mip.ou.dk


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */
 

#ifndef KDMCONFIG_H
#define KDMCONFIG_H

# include "kdm-config.h"

#include <unistd.h>

#include <qstring.h>
#include <qstrlist.h>
#include <qregexp.h>
#include <qfont.h>
#include <qcolor.h>
#include <qfile.h>
#include <kconfig.h>

#include "kdmview.h"

class KDMConfig {
public:
     KDMConfig( );
     ~KDMConfig();
     
     QFont*          normalFont()      { return _normalFont;}
     QFont*          failFont()        { return _failFont;}
     QFont*          greetFont()       { return _greetFont;}
     QString*        greetString()     { return _greetString;}
     QStrList*       sessionTypes()    { return _sessionTypes;}
     int             shutdownButton()  { return _shutdownButton;}
     QString*        shutdown()        { return _shutdown;}
     QString*        restart()         { return _restart;}
     QString*        logo()            { return _logo;}
     KVItemList*     users()           { return _users;}
     GUIStyle        style()           { return _style;}
	// None is defined as a macro somewhere in an X header. GRRRR.
     enum { KNone, All, RootOnly, ConsoleOnly };
private:
     void           getConfig();
     KVItemList*    getUsers( QString s = 0, bool = false);
     KConfig*       kc;

     QFont*         _normalFont;
     QFont*         _failFont;
     QFont*         _greetFont;
     QString*       _greetString;
     QStrList*      _sessionTypes;
     int            _shutdownButton;
     QString*       _shutdown;
     QString*       _restart;
     QString*       _logo;
     KVItemList*    _users;
     GUIStyle       _style;
};

#endif /* KDMCONFIG_H */
