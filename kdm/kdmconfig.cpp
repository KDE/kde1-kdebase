//                              -*- Mode: C++ -*- 
// Title            : kdmconfig.cpp
// 
// Description      : Config for kdm
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:53:49 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Sun Jun 29 19:24:26 1997
// Update Count     : 48
// Status           : Unknown, Use with caution!
// 

#include "kdmconfig.h"
#include "kdmview.h"
#include <qpixmap.h>
#include <kapp.h>
#include <pwd.h>
#include <sys/types.h>
#include <iostream.h>

// Func. for splitting ';' sep. lists.
static void semsplit( const QString& str, QStrList& result)
{
     //QStrList result;
     int i1 = 0, i2 = 0;
     while( ( i2 = str.find( ';', i1)) != -1) {
	  result.append( str.mid(i1,i2-i1));
	  i1 = i2 + 1;
     }
     if( i1 != (int)str.length()) {
	  result.append(str.mid(i1,str.length()));
     }
     //return result;
}     
     
KDMConfig::KDMConfig( const char* rc)
{
     kdedir = rc;
     initStream( QString(kdedir) + KDMRC);
     getConfig();
}

void
KDMConfig::initStream( const char* rc)
{
  /* kalle     cf = new QFile( rc);
     if( cf->exists()) {
	  cf->open( IO_ReadOnly);
	  cs = new QTextStream( cf);
     } else {
	  cs = NULL;
     }
	 */
}

KVItemList*
KDMConfig::getUsers( QString s, bool sorted = false)
{
     QString user_pix_dir( kdedir+"/share/apps/kdm/pics/users/");
     KVItemList* result = new KVItemList;
     QPixmap default_pix( user_pix_dir + "default.xpm");
     if( default_pix.isNull())
	  printf("Cant get default pixmap from \"%s\"\n", 
		 QString(user_pix_dir + "default.xpm").data());
     if( s.isNull()) {
	  QString  nu = kc->readEntry( "NoUsers");
	  QStrList no_users;
	  semsplit( nu, no_users);	  
	  struct passwd *ps;
#define CHECK_STRING( x) (x != 0 && x[0] != 0)
	  setpwent();
	  for( ps = getpwent(); ps ; ) {
	       if( CHECK_STRING(ps->pw_dir) &&
		   CHECK_STRING(ps->pw_shell) &&
		   CHECK_STRING(ps->pw_gecos) &&
		   ( no_users.contains( ps->pw_name) == 0)){
		    // we might have a real user, insert him/her
		    QPixmap p( user_pix_dir + QString(ps->pw_name) + ".xpm");
		    if( p.isNull())
			 p = default_pix;
		    if( sorted) 
			 result->inSort( new KDMViewItem( ps->pw_name, p));
		    else
			 result->append( new KDMViewItem( ps->pw_name, p));
	       }
	       ps = getpwent();
	  }
	  endpwent();
#undef CHECK_STRING
     } else {
	  QStrList sl;
	  semsplit( s, sl);
	  sl.setAutoDelete( true);
	  QStrListIterator it( sl);
	  for( ; it.current(); ++it) {
	       QPixmap p( user_pix_dir + it.current() + ".xpm");
	       if( p.isNull())
		    p = default_pix;
	       if( sorted) 
		    result->inSort( new KDMViewItem( it.current(),p));
	       else
		    result->append( new KDMViewItem( it.current(),p));
	  }
     }
     return result;
}

void
KDMConfig::getConfig()
{
  QString aFileName = QString( kdedir ) + KDMRC; // kalle
  kc = new KConfig( aFileName ); // kalle
     kc->setGroup( "KDM");

     // Read Entries
     QString normal_font = kc->readEntry( "StdFont");
     QString fail_font   = kc->readEntry( "FailFont");
     QString greet_font  = kc->readEntry( "GreetFont");

     QString greet_string   = kc->readEntry(             "GreetString");
     QString session_string = kc->readEntry(            "SessionTypes");
     QString logo_string    = kc->readEntry(              "LogoPixmap");
     if( kc->hasKey("ShutdownButton")) {
	  QString tmp       = kc->readEntry(       "ShutdownButton");
	  if( tmp == "All")
	       _shutdownButton = All;
	  else if( tmp == "RootOnly")
	       _shutdownButton = RootOnly;
	  else if( tmp == "ConsoleOnly")
	       _shutdownButton = ConsoleOnly;
	  else
	       _shutdownButton = KNone;
	  _shutdown         = new QString( kc->readEntry(   "Shutdown"));
	  if( _shutdown->isNull())
	       *_shutdown = "/sbin/halt";
	  _restart          = new QString( kc->readEntry(   "Restart"));
	  if( _restart->isNull())
	       *_restart = "/sbin/reboot";
     } else
	  _shutdownButton   = KNone;
     if( kc->hasKey( "GUIStyle")) {
	  if( kc->readEntry( "GUIStyle") == "Windows")
	       _style = WindowsStyle;
     } else {
	  _style = MotifStyle;
     }
     
     // Logo
     if( logo_string.isNull())
	  _logo = new QString( kdedir+KDMLOGO);
     else
	  _logo = new QString( logo_string);

     // Table of users
     bool sorted = kc->readNumEntry( "SortUsers", 1);
     if( kc->hasKey( "UserView") && kc->readNumEntry( "UserView")) {
	  if( kc->hasKey( "Users")) {
	       QString users = kc->readEntry( "Users");
	       /* make list of users from kdmrc */
	       _users = getUsers( users, sorted);
	  } else  {
	       _users = getUsers( QString(), sorted);
	  }
     } else {
	  /* no user view */
	  _users = NULL;
     }

     // Session Arguments:
     _sessionTypes = new QStrList;
     int i1 = 0, i2 = 0;
     while( ( i2 = session_string.find( ';', i1)) != -1) {
          _sessionTypes->append( 
	       qstrdup( session_string.mid( i1, i2-i1).data()));
          i1 = i2 + 1;
     }
     if( i1 != (int)session_string.length())
          _sessionTypes->append( 
               qstrdup( session_string.mid( i1, session_string.length())));
     if( _sessionTypes->count() == 0) {
          _sessionTypes->append( "kde");
          _sessionTypes->append( "failsafe");
     }

     // Greet String and fonts:
     QString longhostname(256);
     gethostname(longhostname.data(), 255);
     QString hostname;
     // Remove domainname, because it's generally
     // too long to look nice in the title:
     int dot = longhostname.find('.');
     if( dot != -1) hostname = longhostname.left( dot);
     else hostname = longhostname;

     if( !normal_font.isNull()) {
	  _normalFont = new QFont( normal_font.data());
	  _normalFont->setRawMode( true);
     } else 
	  _normalFont = new QFont;

     if( !fail_font.isNull()) {
       	  _failFont = new QFont( fail_font.data());
	  _failFont->setRawMode( true);
     } else {
          _failFont = new QFont( *_normalFont);
	  _failFont->setBold( true);
     }

     if( !greet_font.isNull()) {
	  _greetFont = new QFont( greet_font.data());
	  _greetFont->setRawMode( true);
     } else 
          _greetFont = new QFont( "times", 24, QFont::Black);
      
     if( greet_string.isNull())
          _greetString = new QString( hostname);
     else {
          QRegExp rx( "HOSTNAME");
          greet_string.replace( rx, hostname.data());
          _greetString = new QString( greet_string);
     }
} 

KDMConfig::~KDMConfig()
{
     delete _normalFont;
     delete _failFont;
     delete _greetFont;
     delete _greetString;
     delete _sessionTypes;
     delete kc;
     if( !cs) {
	  delete cs;
	  cf->close();
	  delete cf;
     }
}
