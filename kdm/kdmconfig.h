//                              -*- Mode: C++ -*- 
// Title            : kdmconfig.h
// 
// Description      : Configuration for kdm. Class KDMConfig
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:53:09 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Mon Oct  6 14:22:03 1997
// Update Count     : 9
// Status           : Unknown, Use with caution!
// 

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
     KDMConfig( const char*);
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
     void           initStream( const char*);
     void           getConfig();
     KVItemList*    getUsers( QString s = NULL, bool = false);
     QString        kdedir;
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
