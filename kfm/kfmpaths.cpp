#include "kfmpaths.h"
#include <kconfig.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <kapp.h>

#include <unistd.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

QString* KFMPaths::desktopPath = 0L;
QString* KFMPaths::templatePath = 0L;
QString* KFMPaths::autostartPath = 0L;
QString* KFMPaths::trashPath = 0L;
QString* KFMPaths::cachePath = 0L;

void KFMPaths::initPaths() 
{
  if ( desktopPath == 0L )
    desktopPath = new QString;
  if ( templatePath == 0L )
    templatePath = new QString;
  if ( autostartPath == 0L )
    autostartPath = new QString;
  if ( trashPath == 0L )
    trashPath = new QString;
  if ( cachePath == 0L )
    cachePath = new QString;
  
  KConfig *config = kapp->getConfig();
  config->setGroup( "Paths" );

  // Desktop Path
  *desktopPath = QDir::homeDirPath() + "/Desktop/";
  *desktopPath = config->readEntry( "Desktop", *desktopPath);
  if ( desktopPath->right(1) != "/")
    *desktopPath += "/";
  *desktopPath = QDir::cleanDirPath( *desktopPath );
  
  // Templates Path
  *templatePath = *desktopPath + "Templates/";
  *templatePath = config->readEntry( "Templates" , *templatePath);
  if ( templatePath->right(1) != "/")
    *templatePath += "/";
  *templatePath = QDir::cleanDirPath( *templatePath );

  // Autostart Path
  *autostartPath = *desktopPath + "Autostart/";
  *autostartPath = config->readEntry( "Autostart" , *autostartPath);
  if ( autostartPath->right(1) != "/")
    *autostartPath += "/";
  *autostartPath = QDir::cleanDirPath( *autostartPath );

  // Trash Path
  *trashPath = *desktopPath + "Trash/";
  *trashPath = config->readEntry( "Trash" , *trashPath);
  if ( trashPath->right(1) != "/")
    *trashPath += "/";
  *trashPath = QDir::cleanDirPath( *trashPath );
  
  cachePath->sprintf(_PATH_TMP"/kfm-cache-%i", (int)getuid() );

  if (!QFileInfo(_PATH_TMP).isWritable())
  {
      QString s;
      s.sprintf("ERROR ! kfm needs write permissions to %s\n",_PATH_TMP);
      execlp("kfmwarn","kfmwarn",s.data(), 0);
      fprintf(stderr, s.data()); // in case kfmwarn didn't work
      exit( 1 );
  }
}

QString KFMPaths::DesktopPath()
{
  return *desktopPath;
}
 
QString KFMPaths::TemplatesPath()
{
  return *templatePath;
}

QString KFMPaths::AutostartPath()
{
  return *autostartPath;
}

QString KFMPaths::TrashPath()
{
  return *trashPath;
}

QString KFMPaths::CachePath()
{
  return *cachePath;
}
