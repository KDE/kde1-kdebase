#include "kfmpaths.h"
#include <kconfig.h>
#include <qdir.h>
#include <kapp.h>

QString* KFMPaths::desktopPath = 0L;
QString* KFMPaths::templatePath = 0L;
QString* KFMPaths::autostartPath = 0L;
QString* KFMPaths::trashPath = 0L;

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
    
  KConfig *config = kapp->getConfig();
  config->setGroup( "Paths" );

  // Desktop Path
  *desktopPath = QDir::homeDirPath() + "/Desktop/";
  *desktopPath = config->readEntry( "Desktop", *desktopPath);
  if ( desktopPath->right(1) != "/")
    *desktopPath += "/";
  
  // Templates Path
  *templatePath = *desktopPath + "Templates/";
  *templatePath = config->readEntry( "Templates" , *templatePath);
  if ( templatePath->right(1) != "/")
    *templatePath += "/";

  // Autostart Path
  *autostartPath = *desktopPath + "Autostart/";
  *autostartPath = config->readEntry( "Autostart" , *autostartPath);
  if ( autostartPath->right(1) != "/")
    *autostartPath += "/";

  // Trash Path
  *trashPath = *desktopPath + "Trash/";
  *trashPath = config->readEntry( "Trash" , *trashPath);
  if ( autostartPath->right(1) != "/")
    *autostartPath += "/";

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
