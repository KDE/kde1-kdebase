#include "kfmpaths.h"
#include <Kconfig.h>
#include <qdir.h>
#include <kapp.h>

QString KFMPaths::desktopPath;
QString KFMPaths::templatePath;
QString KFMPaths::autostartPath;
QString KFMPaths::trashPath;

void KFMPaths::initPaths() 
{
  KConfig *config = kapp->getConfig();
  config->setGroup( "Paths" );

  // Desktop Path
  desktopPath = QDir::homeDirPath() + "/Desktop/";
  desktopPath = config->readEntry( "Desktop", desktopPath);
  if ( desktopPath.right(1) != "/")
    desktopPath += "/";
  
  // Templates Path
  templatePath = desktopPath + "Templates/";
  templatePath = config->readEntry( "Templates" , templatePath);
  if ( templatePath.right(1) != "/")
    templatePath += "/";

  // Autostart Path
  autostartPath = desktopPath + "Autostart/";
  autostartPath = config->readEntry( "Autostart" , autostartPath);
  if ( autostartPath.right(1) != "/")
    autostartPath += "/";

  // Trash Path
  trashPath = desktopPath + "Trash/";
  trashPath = config->readEntry( "Trash" , trashPath);
  if ( autostartPath.right(1) != "/")
    autostartPath += "/";

  // To enable the user to edit the paths
  config->writeEntry("Desktop", desktopPath);
  config->writeEntry("Templates", templatePath);
  config->writeEntry("Autostart", autostartPath);
  config->writeEntry("Trash", trashPath);
  config->sync();
}

QString KFMPaths::DesktopPath()
{
  return desktopPath;
}
 
QString KFMPaths::TemplatesPath()
{
  return templatePath;
}

QString KFMPaths::AutostartPath()
{
  return autostartPath;
}

QString KFMPaths::TrashPath()
{
  return trashPath;
}
