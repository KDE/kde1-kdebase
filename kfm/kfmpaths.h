/**
 * (c) Stephan Kulow
 * (c) Torben Weis
 */

#ifndef _KFMPATHS_H_
#define _KFMPATHS_H_

/* This is a little service class for KFM.
   It contains only static members and it
   contains the paths, that KFM needs.  */

#include <qstring.h>

class KFMPaths {

  static QString* desktopPath;
  static QString* templatePath;
  static QString* autostartPath;
  static QString* trashPath;

 public:
  // reads in all paths
  static void initPaths(); 

  // all this paths end with a '/'
  static QString DesktopPath();
  static QString TemplatesPath();
  static QString AutostartPath();
  static QString TrashPath();
};

#endif
