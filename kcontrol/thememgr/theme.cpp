/*
 * theme.h
 *
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org> and
 *                    Roberto Alsina <ralsina@unl.edu.ar>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qfile.h>
#include "theme.h"
#include <kapp.h>
#include <qdir.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <kwm.h>
#include <qwindowdefs.h>
#include <sys/stat.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>


//-----------------------------------------------------------------------------
Theme::Theme(const char* aName): ThemeInherited(aName) 
{
  int len;

  initMetaObject();
  mName = aName;
  mName.detach();

  setLocale();

  mConfigDir = kapp->localconfigdir();
  len = mConfigDir.length();
  if (len > 0 && mConfigDir[len-1] != '/') mConfigDir += '/';

  mMappings = NULL;
  loadMappings();

  // ensure that work directory exists
  mkdirhier(workDir());
}


//-----------------------------------------------------------------------------
Theme::~Theme()
{
  if (mMappings) delete mMappings;
}


//-----------------------------------------------------------------------------
void Theme::setDescription(const QString aDescription)
{
  mDescription = aDescription.copy();
}


//-----------------------------------------------------------------------------
void Theme::setName(const QString aName)
{
  int i;
  i = aName.findRev('/');
  if (i>0) mName = aName.mid(i+1, 1024);
  else mName = aName.copy();

  if (aName[0]=='/') mFileName = aName.copy();
  else mFileName = workDir() + aName;
}


//-----------------------------------------------------------------------------
const QString Theme::themesDir(void)
{
  static QString str;
  if (str.isEmpty())
    str = kapp->localkdedir() + "/share/apps/" + kapp->appName() + "/Themes/";
  return str;
}


//-----------------------------------------------------------------------------
const QString Theme::globalThemesDir(void)
{
  static QString str;
  if (str.isEmpty())
    str = kapp->kde_datadir() + '/' + kapp->appName() + "/Themes/";
  return str;
}


//-----------------------------------------------------------------------------
const QString Theme::workDir(void)
{
  static QString str;
  if (str.isEmpty())
    str = kapp->localkdedir() + "/share/apps/" + kapp->appName() + "/Work/";
  return str;
}


//-----------------------------------------------------------------------------
void Theme::loadMappings()
{
  QFile file;

  file.setName(kapp->localkdedir() + "/share/apps/" + kapp->appName() + 
	       "/theme.mappings");
  if (!file.exists())
  {
    file.setName("theme.mappings");
    if (!file.exists())
    {
      file.setName(kapp->kde_datadir() + "/" + kapp->appName() +
		   "/theme.mappings");
      if (!file.exists())
	fatal(i18n("Mappings file theme.mappings not found."));
    }
  }

  if (mMappings) delete mMappings;
  mMappings = new KSimpleConfig(file.name(), true);
}


//-----------------------------------------------------------------------------
void Theme::load(const QString aPath)
{
  QString cmd;
  QFile file;
  int rc;

  debug("Theme::load()");

  if (aPath) setName(aPath);
  file.setName(fileName() + ".themerc");

  clear();

  // cleanup work directory
  cmd.sprintf("rm %s*", (const char*)workDir());
  rc = system(cmd);
  if (rc) warning("Error during cleanup of work directory: rc=%d\n%s",
		  rc, (const char*)cmd);

  // unpack theme package
  cmd.sprintf("cd \"%s\"; gzip -c -d \"%s.tar.gz\" | tar xf -", 
	      (const char*)workDir(), (const char*)fileName());
  debug(cmd);
  rc = system(cmd);
  if (rc)
  {
    warning(i18n("Failed to unpack %s with\n%s"), 
	    (const char*)(fileName()+".tar.gz"),
	    (const char*)cmd);
    return;
  }

  // read theme config file
  file.open(IO_ReadOnly);
  parseOneConfigFile(file, NULL);
  file.close();

  readConfig();

  emit changed();
}


//-----------------------------------------------------------------------------
void Theme::save(const QString aPath)
{
  QFile file;
  if (aPath.isEmpty()) file.setName(fileName() + ".themerc");
  else file.setName(aPath);

  emit apply();
  writeConfig();
  writeConfigFile(file);
}


//-----------------------------------------------------------------------------
void Theme::removeFile(const QString& aName, const QString aDirName)
{
  if (aName.isEmpty()) return;

  if (aName[0] == '/' || aDirName.isEmpty())
    unlink(aName);
  else if (aDirName[aDirName.length()-1] == '/')
    unlink(aDirName + aName);
  else unlink(aDirName + '/' + aName);
}


//-----------------------------------------------------------------------------
bool Theme::installFile(const QString& aSrc, const QString& aDest)
{
  QString cmd, dest;
  int rc;

  if (aSrc.isEmpty()) return true;

  if (aDest[0] == '/') dest = aDest;
  else if (aDest[aDest.length()-1]=='/')
    dest = kapp->localkdedir() + aDest;
  else dest = QString(kapp->localkdedir()) + '/' + aDest;

  cmd.sprintf("cp \"%s/%s\" \"%s\"", (const char*)workDir(),
	      (const char*)aSrc, (const char*)dest);
  rc = system(cmd);
  if (rc) warning(i18n("Installing file %s: cp returned %d"),
		  (const char*)aSrc, rc);
  return (rc==0);
}


//-----------------------------------------------------------------------------
int Theme::installGroup(const char* aGroupName)
{
  QString value, oldValue, cfgFile, cfgGroup, appDir, group, emptyValue;
  QString oldCfgFile, key, cfgKey, cfgValue, themeValue, instCmd, baseDir;
  bool absPath = false, doInstall;
  KEntryIterator* it;
  KSimpleConfig* cfg = NULL;
  KEntryDictEntry* entry;
  int len, i, installed = 0;
  const char* missing = NULL;

  debug("*** beginning with %s", aGroupName);
  group = aGroupName;
  setGroup(group);

  while (!group.isEmpty())
  {
    mMappings->setGroup(group);
    debug("Mappings group [%s]", (const char*)group);

    // Read config settings
    value = mMappings->readEntry("ConfigFile");
    if (!value.isEmpty())
    {
      cfgFile = value.copy();
      if (cfgFile == "KDERC") cfgFile = QDir::homeDirPath() + "/.kderc";
      else if (cfgFile[0] != '/') cfgFile = mConfigDir + cfgFile;
    }
    value = mMappings->readEntry("ConfigGroup");
    if (!value.isEmpty()) cfgGroup = value.copy();
    value = mMappings->readEntry("ConfigAppDir");
    if (!value.isEmpty())
    {
      appDir = value.copy();
      if (appDir[0] != '/') baseDir = kapp->localkdedir() + "/share/";
      else baseDir = 0;

      mkdirhier(appDir, baseDir);
      appDir = baseDir + appDir;

      len = appDir.length();
      if (len > 0 && appDir[len-1]!='/') appDir += '/';
    }
    absPath = mMappings->readBoolEntry("ConfigAbsolutePaths", absPath);
    value = mMappings->readEntry("ConfigEmpty");
    if (!value.isEmpty()) emptyValue = value.copy();
    value = mMappings->readEntry("ConfigActivateCmd");
    if (!value.isEmpty() && mCmdList.find(value) < 0)
      mCmdList.append(value);
    instCmd = mMappings->readEntry("ConfigInstallCmd").stripWhiteSpace();

    // Some checks
    if (cfgFile.isEmpty()) missing = "ConfigFile";
    if (cfgGroup.isEmpty()) missing = "ConfigGroup";
    if (missing)
    {
      warning(i18n("Internal error in theme mappings\n"
		   "(file theme.mappings) in group %s:\n\n"
		   "Entry `%s' is missing or has no value."),
	      (const char*)group, missing);
      break;
    }

    // Open config file and sync/close old one
    if (oldCfgFile != cfgFile)
    {
      if (cfg)
      {
	debug("closing config file");
	cfg->sync();
	delete cfg;
      }
      debug("opening config file %s", (const char*)cfgFile);
      cfg = new KSimpleConfig(cfgFile);
      oldCfgFile = cfgFile;
    }

    // Set group in config file
    cfg->setGroup(cfgGroup);
    debug("%s: [%s]", (const char*)cfgFile, (const char*)cfgGroup);
    // Process all mapping entries for the group
    it = mMappings->entryIterator(group);
    for (entry=it->toFirst(); entry; entry=it->operator++())
    {
      key = it->currentKey();
      if (key.left(6)=="Config") continue;
      value = entry->aValue;
      len = value.length();
      if (len>0 && value[len-1]=='!')
      {
	doInstall = false;
	value = value.left(len-1);
      }
      else doInstall = true;

      // parse mapping
      i = value.find(':');
      if (i >= 0)
      {
	cfgKey = value.left(i);
	cfgValue = value.mid(i+1, 1024);
      }
      else 
      {
	cfgKey = value;
	cfgValue = 0;
      }
      if (cfgKey.isEmpty()) cfgKey = key;

      if (doInstall)
      {
	oldValue = cfg->readEntry(cfgKey);
	if (!oldValue.isEmpty() && oldValue==emptyValue)
	  oldValue = 0;
      }
      else oldValue = 0;

      themeValue = readEntry(key);
      if (cfgValue.isEmpty()) cfgValue = themeValue;

      // Install file
      if (doInstall)
      {
	if (!oldValue.isEmpty())
	{
	  debug("removeFile: %s%s", (const char*)appDir,
		(const char*)oldValue);
	  removeFile(oldValue, appDir);
	}
	if (!themeValue.isEmpty())
	{
	  debug("installFile: %s => %s", (const char*)themeValue,
		(const char*)(appDir + cfgValue));
	  installFile(themeValue, appDir + cfgValue);
	  installed++;
	}
      }

      // Set config entry
      if (cfgValue.isEmpty()) cfgValue = emptyValue;
      else if (absPath) cfgValue = appDir + cfgValue;
      debug("%s=%s", (const char*)cfgKey, (const char*)cfgValue);
      cfg->writeEntry(cfgKey, cfgValue);
    }

    if (!instCmd.isEmpty()) installCmd(cfg, instCmd, installed);
    group = mMappings->readEntry("ConfigNextGroup");
  }

  if (cfg)
  {
    debug("closing config file");
    cfg->sync();
    delete cfg;
  }

  debug("*** done with %s", aGroupName);
  return installed;
}


//-----------------------------------------------------------------------------
void Theme::installCmd(KSimpleConfig* aCfg, const QString& aCmd,
		       int aInstalled)
{
  QString grp = aCfg->group();
  QString value, cmd;

  cmd = aCmd.stripWhiteSpace();

  if (cmd == "winShapeMode")
  {
    aCfg->writeEntry("ShapeMode", aInstalled ? "on" : "off");
  }
  else if (cmd == "winTitlebar")
  {
    if (hasKey("TitlebarPixmapActive") || hasKey("TitlebarPixmapInactive"))
      value = "pixmap";
    else if (aCfg->readEntry("TitlebarLook") == "pixmap")
      value = "shadedHorizontal";
    else value = 0;
    if (!value.isEmpty()) aCfg->writeEntry("TitlebarLook", value);
  }
  else if (cmd == "enableSounds")
  {
    aCfg->setGroup("GlobalConfiguration");
    aCfg->writeEntry("EnableSounds", aInstalled>0 ? "Yes" : "No");
  }
  else if (cmd == "oneDesktopMode")
  {
    aCfg->writeEntry("OneDesktopMode", (aInstalled==1));
  }
  else
  {
    warning(i18n("Unknown command `%s' in theme.mappings\n"
		 "file in group %s."), (const char*)aCmd,
	    (const char*)mMappings->group());
  }

  aCfg->setGroup(grp);
}


//-----------------------------------------------------------------------------
void Theme::doCmdList(void)
{
  QString cmd;

  for (cmd=mCmdList.first(); !cmd.isNull(); cmd=mCmdList.next())
  {
    if (strncmp(cmd, "kwmcom", 6) == 0)
      KWM::sendKWMCommand(cmd.mid(6,256).stripWhiteSpace());
    else if (cmd == "applyColors")
      colorSchemeApply();
  }

  mCmdList.clear();
}


//-----------------------------------------------------------------------------
void Theme::install(void)
{
  debug("Theme::install()");

  loadMappings();
  mCmdList.clear();

  if (instPanel) installGroup("Panel");
  if (instSounds) installGroup("Sounds");
  if (instWindowBorder) installGroup("Window Border");
  if (instWindowTitlebar) installGroup("Window Titlebar");
  if (instWallpapers) installGroup("Display");
  if (instColors) installGroup("Colors");

  doCmdList();

#ifdef BROKEN
  installColors();
  installPanel();
  installDisplay();
  installWM();
  installSounds();

  KWM::sendKWMCommand("kbgwm_reconfigure");
  KWM::sendKWMCommand("configure");
  KWM::sendKWMCommand("kpanel:restart");
  KWM::sendKWMCommand("syssnd_restart");
  colorSchemeApply();

  sleep(1);
  system("xrefresh");
#endif

  debug("Theme::install() done");
}


//-----------------------------------------------------------------------------
void Theme::readCurrent(void)
{
  emit changed();
}


//-----------------------------------------------------------------------------
KConfig* Theme::openConfig(const QString aAppName) const
{
  KConfig* pConfig;
  QString aConfigName = kapp->localconfigdir();
  aConfigName += "/";
  aConfigName += aAppName;
  aConfigName += "rc";
  bool bSuccess;
  QString aGlobalAppConfigName = kapp->kde_configdir() + "/" + aAppName + "rc";

  debug(aConfigName);
  debug(aGlobalAppConfigName);
  QFile aConfigFile(aConfigName);

  // Open the application-specific config file. It will be created if
  // it does not exist yet.
  bSuccess = aConfigFile.open( IO_ReadWrite ); 
  if(!bSuccess)
  {
    // try to open at least read-only
    bSuccess = aConfigFile.open( IO_ReadOnly );
    if(!bSuccess)
    {
      // we didn't succeed to open an app-config file
      pConfig = new KConfig( aGlobalAppConfigName );
    }
    else
    {
      // we succeeded to open an app-config file read-only
      pConfig = new KConfig( aGlobalAppConfigName, aConfigName );
    }
  }
  else
  {
    // we succeeded to open an app-config file read-write
    pConfig = new KConfig( aGlobalAppConfigName, aConfigName );
  }

  return pConfig;
}


//-----------------------------------------------------------------------------
void Theme::readConfig(void)
{
  QString fname;
  QColor col;
  col.setRgb(192,192,192);

  setGroup("General");
  mDescription = readEntry("description", mName + " Theme");

  setGroup("Colors");
  foregroundColor = readColorEntry(this, "foreground", &col);
  backgroundColor = readColorEntry(this, "background", &col);
  selectForegroundColor = readColorEntry(this, "selectForeground", &col);
  selectBackgroundColor = readColorEntry(this, "selectBackground", &col);
  activeForegroundColor = readColorEntry(this, "activeForeground", &col);
  activeBackgroundColor = readColorEntry(this, "activeBackground", &col);
  activeBlendColor = readColorEntry(this, "activeBlend", &col);
  inactiveForegroundColor = readColorEntry(this, "inactiveForeground", &col);
  inactiveBackgroundColor = readColorEntry(this, "inactiveBackground", &col);
  inactiveBlendColor = readColorEntry(this, "inactiveBlend", &col);
  windowForegroundColor = readColorEntry(this, "windowForeground", &col);
  windowBackgroundColor = readColorEntry(this, "windowBackground", &col);
  contrast = readNumEntry("Contrast", 7);

  fname = fileName() + ".preview.jpg";
  if (!mPreview.load(fname))
  {
    warning(i18n("Failed to load preview image %s"), (const char*)fname);
    mPreview.resize(0,0);
  }
}


//-----------------------------------------------------------------------------
void Theme::writeConfig(void)
{
  debug("Theme::writeConfig() is broken");
  return;

  setGroup("General");
  writeEntry("description", mDescription);

#ifdef BROKEN
  setGroup("Colors");
  writeColorEntry(this, "BackgroundColor", backgroundColor);
  writeColorEntry(this, "SelectColor", selectColor);
  writeColorEntry(this, "TextColor", textColor);
  writeColorEntry(this, "ActiveTitleTextColor", activeTextColor);
  writeColorEntry(this, "InactiveTitleBarColor", inactiveTitleColor);
  writeColorEntry(this, "ActiveTitleBarColor", activeTitleColor);
  writeColorEntry(this, "InactiveTitleTextColor", inactiveTextColor);
  writeColorEntry(this, "WindowTextColor", windowTextColor);
  writeColorEntry(this, "WindowColor", windowColor);
  writeColorEntry(this, "SelectTextColor", selectTextColor);
  writeEntry("Contrast", contrast);
#endif
}


//-----------------------------------------------------------------------------
void Theme::writeColorEntry(KConfigBase* cfg, const char* aKey, 
			    const QColor& aColor)
{
  QString str(32);
  str.sprintf("#%02x%02x%02x", aColor.red(), aColor.green(), aColor.blue());
  cfg->writeEntry(aKey, str);
}


//-----------------------------------------------------------------------------
const QColor& Theme::readColorEntry(KConfigBase* cfg, const char* aKey,
				    const QColor* aDefault) const
{
  static QColor col;
  QString str = cfg->readEntry(aKey, 0);

  if (!str.isEmpty()) col.setNamedColor(str);
  else if (aDefault) col = *aDefault;

  return col;
}


//-----------------------------------------------------------------------------
void Theme::clear(void)
{
  deleteGroup("General");
  deleteGroup("Desktop");
  deleteGroup("Colors");
  deleteGroup("Window Border");
  deleteGroup("Window Titlebar");
  deleteGroup("Sounds");
  deleteGroup("Icons");
  deleteGroup("Panel");
  deleteGroup("Filemanager");
}


//-----------------------------------------------------------------------------
bool Theme::mkdirhier(const char* aDir, const char* aBaseDir)
{
  QDir dir;
  QString dirStr = aDir;
  dirStr.detach();
  const char* dirName;
  int oldMask = umask(077);

  if (aBaseDir) dir.cd(aBaseDir,true);
  else if (aDir[0]!='/') dir.cd(kapp->localkdedir());
  else dir.cd("/");

  for (dirName=strtok(dirStr.data(),"/"); dirName; dirName=strtok(0, "/"))
  {
    if (dirName[0]=='\0') continue;
    if (!dir.exists(dirName))
    {
      if (!dir.mkdir(dirName))
      {
	warning(i18n("Cannot create directory %s"), 
		(const char*)(dir.dirName() + dirName));
	umask(oldMask);
	return false;
      }
    }
    if (!dir.cd(dirName))
    {
      warning(i18n("Cannot enter directory %s"),
	      (const char*)(dir.dirName() + dirName));
      umask(oldMask);
      return false;
    }
  }

  umask(oldMask);
  return true;
}


//-----------------------------------------------------------------------------
bool Theme::hasGroup(const QString& aName, bool aNotEmpty)
{
  KGroupIterator* it = groupIterator();
  KEntryDict* grp;
  bool found = false;

  for (grp=it->toFirst(); grp; grp=it->operator++())
  {
    if (stricmp(it->currentKey(),aName)==0)
    {
      found = true;
      break;
    }
  }
  if (found && aNotEmpty) found = !grp->isEmpty();

  delete it;
  return found;
}


//-----------------------------------------------------------------------------
static int dropError(Display *, XErrorEvent *)
{
  return 0;
}


//-----------------------------------------------------------------------------
static int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p){
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;

  status = XGetWindowProperty(qt_xdisplay(), w, a, 0L, len, False, type, 
			      &real_type, &format, &n, &extra, p);
  if (status != Success || *p == 0) return -1;
  if (n == 0) XFree((char*) *p);
  return n;
}


//-----------------------------------------------------------------------------
static bool getSimpleProperty(Window w, Atom a, long &result){
  long *p = 0;

  if (_getprop(w, a, a, 1L, (unsigned char**)&p) <= 0){
    return FALSE;
  }

  result = p[0];
  XFree((char *) p);
  return TRUE;
}


//-----------------------------------------------------------------------------
void Theme::colorSchemeApply(void)
{
  XEvent ev;
  unsigned int i, nrootwins;
  Window dw1, dw2, *rootwins;
  int (*defaultHandler)(Display *, XErrorEvent *);
  Display* dpy = qt_xdisplay();
  Window root = RootWindow(dpy, DefaultScreen(dpy));
  Atom KDEChangePalette = XInternAtom(dpy, "KDEChangePalette", false);
	
  defaultHandler = XSetErrorHandler(dropError);
  XQueryTree(dpy, root, &dw1, &dw2, &rootwins, &nrootwins);
	
  // Matthias
  Atom a = XInternAtom(dpy, "KDE_DESKTOP_WINDOW", False);
  for (i = 0; i < nrootwins; i++) {
    long result = 0;
    getSimpleProperty(rootwins[i],a, result);
    if (result){
      ev.xclient.type = ClientMessage;
      ev.xclient.display = dpy;
      ev.xclient.window = rootwins[i];
      ev.xclient.message_type = KDEChangePalette;
      ev.xclient.format = 32;
      
      XSendEvent(dpy, rootwins[i] , False, 0L, &ev);
    }
  }
  
  XFlush(dpy);
  XSetErrorHandler(defaultHandler);
  
  XFree((char*)rootwins);
}




#ifdef BROKEN
//-----------------------------------------------------------------------------
void Theme::installSounds(void)
{
  bool enabled;

  enabled = installGroup("Sounds");

  cfg->setGroup("GlobalConfiguration");
  cfg->writeEntry("EnableSounds", enabled ? "Yes" : "No");
  cfg->sync();
  delete cfg;
}

//-----------------------------------------------------------------------------
void Theme::installWM(void)
{
  KSimpleConfig* cfg;
  QString key, str, name, value, oldValue, group, fname, dirname;
  int i;
  bool shapeMode = false;

  static const char* borderKeyTab[] =
  {
    "ShapePixmapTop", "ShapePixmapBottom", "ShapePixmapLeft", 
    "ShapePixmapRight", "ShapePixmapTopLeft", "ShapePixmapBottomLeft", 
    "ShapePixmapTopRight", "ShapePixmapBottomRight", 0
  };
  static const char* borderDestFileTab[] =
  {
    "wm_top.xpm", "wm_bottom.xpm", "wm_left.xpm",
    "wm_right.xpm", "wm_topleft.xpm", "wm_bottomleft.xpm", 
    "wm_topright.xpm", "wm_bottomright.xpm", 0
  };
  static const char* buttonKeyTab[] =
  {
    "TitlebarPixmapActive", "TitlebarPixmapInactive", "CloseButton",
    "MinimizeButton", "MaximizeButton", "MenuButton", "StickyButton", 0
  };
  static const char* buttonDestFileTab[] =
  {
    "activetitlebar.xpm", "inactivetitlebar.xpm", "close.xpm",
    "iconify.xpm", "maximize.xpm", "menu.xpm", "sticky.xpm", 0
  };

  if (!instWindowBorder && !instWindowTitlebar) return;

  dirname = kapp->localkdedir() + "/share/apps/kwm/pics/";
  if (!mkdirhier("share/apps/kwm/pics")) return;
  cfg = new KSimpleConfig(mConfigDir+"/kwmrc");

  cfg->setGroup("General");

  if (instWindowBorder)
  {
    debug("installing window border");
    setGroup("Window Border");
    for (i=0; borderKeyTab[i]; i++)
    {
      value = readEntry(borderKeyTab[i], 0);
      fname = dirname + borderDestFileTab[i];
      unlink(fname);
      if (!value.isEmpty())
      {
	installFile(value, fname);
	shapeMode = true;
      }
    }
    cfg->writeEntry("ShapeMode", shapeMode ? "on" : "off");
  }

  if (instWindowTitlebar)
  {
    debug("installing window titlebar");
    setGroup("Window Titlebar");
    for (i=0; buttonKeyTab[i]; i++)
    {
      value = readEntry(buttonKeyTab[i], 0);
      fname = dirname + buttonDestFileTab[i];
      unlink(fname);
      if (!value.isEmpty()) installFile(value, fname);
    }

    if (hasKey("TitlebarPixmapActive") || hasKey("TitlebarPixmapInactive"))
      value = "pixmap";
    else if (cfg->readEntry("TitlebarLook") == "pixmap")
      value = "shadedHorizontal";
    else value = 0;
    if (!value.isEmpty()) cfg->writeEntry("TitlebarLook", value);
  }

  cfg->sync();
  delete cfg;
}


//-----------------------------------------------------------------------------
void Theme::installDisplay(void)
{
  KSimpleConfig* cfg;
  QString key, str, name, value, group, fname;
  int i, num;

  if (!instWallpapers) return;
  debug("installing wallpapers");

  if (!mkdirhier("share/apps/kdisplay/color-scheme")) return;
  if (!mkdirhier("share/apps/kdisplay/pics")) return;

  setGroup("Display");
  num = readNumEntry("Desktops", 1);

  for (i=0; i<num; i++)
  {
    name.sprintf("%s/desktop%drc", (const char*)mConfigDir, i);
    cfg = new KSimpleConfig(name);
    group.sprintf("Desktop%d", i);
    cfg->setGroup(group);

    key.sprintf("Wallpaper%d", i);
    value = readEntry(key, 0);
    cfg->writeEntry("UseWallpaper", !value.isEmpty());
    if (!value.isEmpty())
    {
      fname = kapp->localkdedir() + "/share/apps/kdisplay/pics/" + value;
      cfg->writeEntry("Wallpaper", fname);
      installFile(value, fname);
    }
    else cfg->writeEntry("Wallpaper", "");

    key.sprintf("WallpaperMode%d", i);
    cfg->writeEntry("WallpaperMode", readEntry(key, 0));

    cfg->sync();
    delete cfg;
  }
}
#endif //BROKEN


//-----------------------------------------------------------------------------
#include "theme.moc"
