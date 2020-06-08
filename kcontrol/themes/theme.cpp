/*
 * theme.h
 *
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org>
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

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstrlist.h>
#include <kapp.h>
#include <qdir.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <kwm.h>
#include <kmsgbox.h>
#include <qwindowdefs.h>
#include <sys/stat.h>
#include <assert.h>

#include "theme.h"

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>


//-----------------------------------------------------------------------------
Theme::Theme(): ThemeInherited(0), mInstFiles(true)
{
  int len;

  initMetaObject();

  instOverwrite = false;

  mConfigDir = kapp->localconfigdir();
  len = mConfigDir.length();
  if (len > 0 && mConfigDir[len-1] != '/') mConfigDir += '/';

  mMappings = 0;
  mConfig = 0;
  loadMappings();

  // ensure that work directory exists
  mkdirhier(workDir());

  loadSettings();
}


//-----------------------------------------------------------------------------
Theme::~Theme()
{
  saveSettings();
  if (mMappings) delete mMappings;
  if (mConfig) {
      delete mConfig; mConfig = 0;
  }
}


//-----------------------------------------------------------------------------
void Theme::loadSettings(void)
{
  KConfig* cfg = kapp->getConfig();

  cfg->setGroup("Install");
  mRestartCmd = cfg->readEntry("restart-cmd",
			       "kill `pidof %s`; %s >/dev/null 2>&1 &");
  mInstIcons = cfg->readNumEntry("icons");
}


//-----------------------------------------------------------------------------
void Theme::saveSettings(void)
{
  KConfig* cfg = kapp->getConfig();

  cfg->setGroup("Install");
  cfg->writeEntry("icons", mInstIcons);
  cfg->sync();
}


//-----------------------------------------------------------------------------
void Theme::setDescription(const QString aDescription)
{
  mDescription = aDescription.copy();
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
void Theme::cleanupWorkDir(void)
{
  QString cmd;
  int rc;

  // cleanup work directory
  cmd.sprintf("rm -rf %s*", (const char*)workDir());
  rc = system(cmd);
  if (rc) warning(i18n("Error during cleanup of work directory: rc=%d\n%s"),
		  rc, (const char*)cmd);
}


//-----------------------------------------------------------------------------
bool Theme::load(const QString aPath, QString aName)
{
  QString cmd, str;
  QFileInfo finfo(aPath);
  int rc, num, i;

  debug("Theme::load()");

  if (!finfo.exists()) return false;

  clear();
  cleanupWorkDir();
  mName = aName.copy();

  if (finfo.isDir())
  {
    // The theme given is a directory. Copy files over into work dir.

    i = aPath.findRev('/');
    if (i >= 0) str = workDir() + aPath.mid(i, 1024);
    else str = workDir();

    cmd.resize(80+aPath.length()+str.length());
    cmd.sprintf("cp -r \"%s\" \"%s\"", (const char*)aPath,
		(const char*)str);
    debug(cmd);
    rc = system(cmd);
    if (rc)
    {
      warning(i18n("Failed to copy theme contents\nfrom %s\ninto %s"),
	      (const char*)aPath, (const char*)str);
      return false;
    }
  }
  else
  {
    // The theme given is a tar package. Unpack theme package.
    cmd.resize(80+workDir().length()+aPath.length());
    cmd.sprintf("cd \"%s\"; gzip -c -d \"%s\" | tar xf -", 
		(const char*)workDir(), (const char*)aPath);
    debug(cmd);
    rc = system(cmd);
    if (rc)
    {
      warning(i18n("Failed to unpack %s with\n%s"), 
	      (const char*)aPath,
	      (const char*)cmd);
      return false;
    }
  }

  // Let's see if the theme is stored in a subdirectory.
  QDir dir(workDir(), 0, QDir::Name, QDir::Files|QDir::Dirs);
  for (i=0, mThemePath=0, num=0; dir[i]!=0; i++)
  {
    if (dir[i][0]=='.') continue;
    finfo.setFile(workDir() + dir[i]);
    if (!finfo.isDir()) break;
    mThemePath = dir[i];
    num++;
  }
  if (num==1) mThemePath = workDir() + mThemePath + '/';
  else mThemePath = workDir();

  // Search for the themerc file
  dir.setNameFilter("*.themerc");
  dir.setPath(mThemePath);
  mThemercFile = dir[0];
  if (mThemercFile.isEmpty())
  {
    warning(i18n("Theme contains no .themerc file"));
    return false;
  }

  // Search for the preview image
  dir.setNameFilter("*.preview.*");
  mPreviewFile = dir[0];

  // Read .themerc file from disk
  resync();

  return true;
}


//-----------------------------------------------------------------------------
bool Theme::save(const QString aPath)
{
  QString cmd;
  QFile file;
  int rc;

  writeConfig();

  mConfig->sync();

  if (stricmp(aPath.right(4), ".tgz") == 0 ||
      stricmp(aPath.right(7), ".tar.gz") == 0)
  {
    cmd.resize(80+workDir().length()+aPath.length());
    cmd.sprintf("cd \"%s\";tar cf - *|gzip -c >\"%s\"",
		(const char*)workDir(), (const char*)aPath);
  }
  else
  {
    cmd.resize(80+workDir().length()+aPath.length()+aPath.length());
    cmd.sprintf("cd \"%s\"; rm -rf \"%s\"; cp -r * \"%s\"",
		(const char*)workDir(), (const char*)aPath,
		(const char*)aPath);
  }

  debug(cmd);
  rc = system(cmd);
  if (rc) {
      debug("Failed to save theme to:");
      debug(aPath);
      debug("with command");
      debug(cmd);
  }

  return (rc==0);
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
  QString cmd, dest, src;
  QFileInfo finfo;
  QFile srcFile, destFile;
  int len, i;
  char buf[32768];
  bool backupMade = false;

  if (aSrc.isEmpty()) return true;

  if (aDest[0] == '/') dest = aDest;
  else if (aDest[aDest.length()-1]=='/')
    dest = kapp->localkdedir() + aDest;
  else dest = QString(kapp->localkdedir()) + '/' + aDest;

  src = mThemePath + aSrc;

  finfo.setFile(src);
  if (!finfo.exists())
  {
//    debug("File %s is not in theme package.", (const char*)aSrc);
    return false;
  }

  finfo.setFile(dest);
  if (finfo.isDir())  // destination is a directory
  {
    len = dest.length();
    if (dest[len-1]=='/') dest[len-1] = '\0';
    i = src.findRev('/');
    dest = dest + '/' + src.mid(i+1,1024);
    finfo.setFile(dest);
  }

  if (!instOverwrite && finfo.exists()) // make backup copy
  {
    unlink(dest+'~');
    rename(dest, dest+'~');
    backupMade = true;
  }

  srcFile.setName(src);
  if (!srcFile.open(IO_ReadOnly))
  {
    warning(i18n("Cannot open file %s for reading"), (const char*)src);
    return false;
  }

  destFile.setName(dest);
  if (!destFile.open(IO_WriteOnly))
  {
    warning(i18n("Cannot open file %s for writing"), (const char*)dest);
    return false;
  }

  while (!srcFile.atEnd())
  {
    len = srcFile.readBlock(buf, 32768);
    if (len <= 0) break;
    if (destFile.writeBlock(buf, len) != len)
    {
      warning(i18n("Write error to %s:\n%s"), (const char*)dest,
	      strerror(errno));
      return false;
    }
  }

  srcFile.close();
  destFile.close();

  addInstFile(dest);
//  debug("Installed %s to %s %s", (const char*)src, (const char*)dest,
//	backupMade ? "(backup of existing file)" : "");

  return true;
}


//-----------------------------------------------------------------------------
int Theme::installGroup(const char* aGroupName)
{
  QString value, oldValue, cfgFile, cfgGroup, appDir, group, emptyValue;
  QString oldCfgFile, key, cfgKey, cfgValue, themeValue, instCmd, baseDir;
  QString preInstCmd;
  bool absPath = false, doInstall;
  KEntryIterator* it;
  KSimpleConfig* cfg = NULL;
  KEntryDictEntry* entry;
  int len, i, installed = 0;
  const char* missing = 0;

//  debug("*** beginning with %s", aGroupName);
  group = aGroupName;
  mConfig->setGroup(group);
  
  if (!instOverwrite) uninstallFiles(aGroupName);
  else readInstFileList(aGroupName);

  while (!group.isEmpty())
  {
    mMappings->setGroup(group);
//    debug("Mappings group [%s]", (const char*)group);

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
    preInstCmd = mMappings->readEntry("ConfigPreInstallCmd").stripWhiteSpace();

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
//	debug("closing config file");
	cfg->sync();
	delete cfg;
      }
//      debug("opening config file %s", (const char*)cfgFile);
      cfg = new KSimpleConfig(cfgFile);
      oldCfgFile = cfgFile;
    }

    // Set group in config file
    cfg->setGroup(cfgGroup);
//    debug("%s: [%s]", (const char*)cfgFile, (const char*)cfgGroup);

    // Execute pre-install command (if given)
    if (!preInstCmd.isEmpty()) preInstallCmd(cfg, preInstCmd);

    // Process all mapping entries for the group
    it = mMappings->entryIterator(group);
    if (it) for (entry=it->toFirst(); entry; entry=it->operator++())
    {
      key = it->currentKey();
      if (stricmp(key.left(6),"Config")==0) continue;
      value = QString(entry->aValue).stripWhiteSpace();
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

      themeValue = mConfig->readEntry(key);
      if (cfgValue.isEmpty()) cfgValue = themeValue;

      // Install file
      if (doInstall)
      {
	if (!themeValue.isEmpty())
	{
	  if (installFile(themeValue, appDir + cfgValue))
	    installed++;
	  else doInstall = false;
	}
      }

      // Determine config value
      if (cfgValue.isEmpty()) cfgValue = emptyValue;
      else if (doInstall && absPath) cfgValue = appDir + cfgValue;
 
      // Set config entry
//      debug("%s=%s", (const char*)cfgKey, (const char*)cfgValue);
      if (cfgKey != "-") {
          if (cfgValue.isEmpty()) {
              cfg->deleteEntry(cfgKey, false);
          } else {
              cfg->writeEntry(cfgKey, cfgValue);
          }
      }
      

    }

    if (!instCmd.isEmpty()) installCmd(cfg, instCmd, installed);
    group = mMappings->readEntry("ConfigNextGroup");
  }

  if (cfg)
  {
//    debug("closing config file");
    cfg->sync();
    delete cfg;
  }

  writeInstFileList(aGroupName);
//  debug("*** done with %s", aGroupName);
  return installed;
}


//-----------------------------------------------------------------------------
void Theme::preInstallCmd(KSimpleConfig* aCfg, const QString& aCmd)
{
  QString grp = aCfg->group();
  QString value, cmd;

  cmd = aCmd.stripWhiteSpace();

  if (cmd == "stretchBorders")
  {
    value = mConfig->readEntry("ShapePixmapBottom");
    if (!value.isEmpty()) stretchPixmap(mThemePath + value, false);
    value = mConfig->readEntry("ShapePixmapTop");
    if (!value.isEmpty()) stretchPixmap(mThemePath + value, false);
    value = mConfig->readEntry("ShapePixmapLeft");
    if (!value.isEmpty()) stretchPixmap(mThemePath + value, true);
    value = mConfig->readEntry("ShapePixmapRight");
    if (!value.isEmpty()) stretchPixmap(mThemePath + value, true);
  }
  else
  {
    warning(i18n("Unknown pre-install command `%s' in\n"
		 "theme.mappings file in group %s."), 
	    (const char*)aCmd, (const char*)mMappings->group());
  }
}


//-----------------------------------------------------------------------------
void Theme::installCmd(KSimpleConfig* aCfg, const QString& aCmd,
		       int aInstalled)
{
  QString grp = aCfg->group();
  QString value, cmd;
  bool flag;

  cmd = aCmd.stripWhiteSpace();

  if (cmd == "winShapeMode")
  {
    aCfg->writeEntry("ShapeMode", aInstalled ? "on" : "off");
  }
  else if (cmd == "winTitlebar")
  {
    if (mConfig->hasKey("TitlebarPixmapActive") || 
        mConfig->hasKey("TitlebarPixmapInactive")) {
      value = "pixmap";
    } else if (aCfg->readEntry("TitlebarLook") == "pixmap") {
      value = "shadedHorizontal";
    } else {
      value = 0;
    }
    if (!value.isEmpty()) {
      aCfg->writeEntry("TitlebarLook", value);
    }
  }
  else if (cmd == "winGimmickMode")
  {
    if (mConfig->hasKey("Pixmap")) value = "on";
    else value = "off";
    aCfg->writeEntry("GimmickMode", value);
  }
  /*CT 17Jan1999 no more needed - handled by the kpanel now 
  else if (cmd == "panelBack")
  {
    value = aCfg->readEntry("Position");
    if (stricmp(value,"right")==0 || stricmp(value,"left")==0)
    {
      value = aCfg->readEntry("BackgroundTexture");
      rotateImage(kapp->localkdedir()+"/share/apps/kpanel/pics/"+value, -90);
    }
  }
  */
  else if (cmd == "enableSounds")
  {
    aCfg->setGroup("GlobalConfiguration");
    aCfg->writeEntry("EnableSounds", aInstalled>0 ? "Yes" : "No");
  }
  else if (cmd == "setWallpaperMode")
  {
    value = aCfg->readEntry("wallpaper",0);
    aCfg->writeEntry("UseWallpaper", !value.isEmpty());
  }
  else if (cmd == "oneDesktopMode")
  {
    flag = mConfig->readBoolEntry("CommonDesktop", true);
    flag |= (aInstalled==1);
    aCfg->writeEntry("OneDesktopMode",  flag);
    if (flag) aCfg->writeEntry("DeskNum", 0);
  }
  else
  {
    warning(i18n("Unknown command `%s' in theme.mappings\n"
		 "file in group %s."), (const char*)aCmd,
	    (const char*)mMappings->group());
  }

  if (stricmp(aCfg->group(), grp)!=0) 
    aCfg->setGroup(grp);
}


//-----------------------------------------------------------------------------
void Theme::doCmdList(void)
{
  QString cmd, str, appName;
  bool kwmRestart = false;
  int rc;

  for (cmd=mCmdList.first(); !cmd.isNull(); cmd=mCmdList.next())
  {
    debug("do command:");
    debug((const char*)cmd);
    if (cmd.left(6) == "kwmcom")
    {
      cmd = cmd.mid(6,256).stripWhiteSpace();
      if (stricmp(cmd,"restart") == 0) kwmRestart = true;
      else KWM::sendKWMCommand(cmd);
    }
    else if (cmd.left(9) == "kfmclient1")
    {
      system(cmd);
    }
    else if (cmd == "applyColors")
    {
      colorSchemeApply();
      runKrdb();
    }
    else if (strncmp(cmd, "restart", 7) == 0)
    {
      appName = cmd.mid(7,256).stripWhiteSpace();
      str.sprintf(i18n("Restart %s to activate the new settings?"),
		  (const char*)appName);
      rc = KMsgBox::yesNo(NULL,i18n("Restart Application"), str,
			  KMsgBox::QUESTION|KMsgBox::DB_FIRST,
			  i18n("Yes"), i18n("No"));
      if (rc == 1)
      {
	str.sprintf(mRestartCmd, (const char*)appName, 
		    (const char*)appName);
	system(str);
      }
    }
  }

  if (kwmRestart) KWM::sendKWMCommand("restart");
  mCmdList.clear();
}


//-----------------------------------------------------------------------------
bool Theme::backupFile(const QString fname) const
{
  QFileInfo fi(fname);
  QString cmd;
  int rc;

  if (!fi.exists()) return false;

  unlink(fname + '~');
  cmd.sprintf("mv \"%s\" \"%s~\"", (const char*)fname,
	      (const char*)fname);
  rc = system(cmd);
  if (rc) warning(i18n("Cannot make backup copy of %s: mv returned %d"),
		  (const char*)fname, rc);
  return (rc==0);
}


//-----------------------------------------------------------------------------
int Theme::installIcons(void)
{
  KEntryIterator* it;
  KSimpleConfig* cfg = NULL;
  KEntryDictEntry* entry;
  QString key, value, mapval, fname, fpath, destName, icon, miniIcon;
  QString iconDir, miniIconDir, cmd, destNameMini, localShareDir;
  QStrList pathList(true);
  QStrList extraIcons;
  QFileInfo finfo;
  const char* path;
  int i, installed = 0;
  const char* groupName = "Icons";
  const char* groupNameExtra = "Extra Icons";
  bool wantRestart = false;

//  debug("*** beginning with %s", groupName);

  if (!instOverwrite)
  {
    uninstallFiles(groupName);
    if (mInstIcons > 0) wantRestart = true;
    mInstIcons = 0;
  }
  else readInstFileList(groupName);

  mConfig->setGroup(groupName);
  mMappings->setGroup(groupName);

  // Construct search path for kdelnk files
  pathList.append(kapp->localkdedir() + "/share/applnk");
  pathList.append(kapp->localkdedir() + "/share/mimelnk");
  pathList.append(kapp->kde_appsdir());
  pathList.append(kapp->kde_mimedir());

  mkdirhier("share/icons/mini");
  iconDir = kapp->localkdedir() + "/share/icons/";
  miniIconDir = kapp->localkdedir() + "/share/icons/mini/";
  localShareDir = kapp->localkdedir() + "/share/";

  // Process all mapping entries for the group
  it = mConfig->entryIterator(groupName);
  if (it) for (entry=it->toFirst(); entry; entry=it->operator++())
  {
    key = it->currentKey();
    if (stricmp(key.left(6),"Config")==0)
    {
      warning(i18n("Icon key name %s can not be used"), (const char*)key);
      continue;
    }
    value = entry->aValue;
    i = value.find(':');
    if (i > 0)
    {
      icon = value.left(i);
      miniIcon = value.mid(i+1, 1024);
    }
    else
    {
      icon = value;
      miniIcon = "mini-" + icon;
      iconToMiniIcon(mThemePath + icon, mThemePath + miniIcon);
    }

    // test if there is a 1:1 mapping in the mappings file
    destName = mMappings->readEntry(key,0);
    destNameMini = 0;

    // if not we have to search for the proper kdelnk file
    // and extract the icon name from there.
    if (destName.isEmpty())
    {
      fname = "/" + key + ".kdelnk";
      for (fpath=0, path=pathList.first(); path; path=pathList.next())
      {
	fpath = path + fname;
	finfo.setFile(fpath);
	if (finfo.exists()) break;
	fpath = 0;
      }
      if (!fpath.isEmpty())
      {
	cfg = new KSimpleConfig(fpath, true);
	cfg->setGroup("KDE Desktop Entry");
	destName = cfg->readEntry("Icon", 0);
	delete cfg;
      }
    }
    else
    {
      // test if the 1:1 mapping contains different names for icon
      // and mini icon
      i = destName.find(':');
      if (i >= 0)
      {
//	debug("mapping %s to...", (const char*)destName);
	destNameMini = destName.mid(i+1, 1024);
	destName = destName.left(i);
//	debug("   %s  %s", (const char*)destName, (const char*)destNameMini);
      }
    }

    if (destNameMini.isEmpty()) destNameMini = destName;

    // If we have still not found a destination icon name we will install
    // the icons with their current name
    if (destName.isEmpty())
    {
      if (icon.isEmpty()) continue;
      warning(i18n("No proper kdelnk file found for %s.\n"
		   "Installing icon(s) as %s"),
	      (const char*)key, (const char*)icon);
      destName = icon;
    }

    // install icons
    if (destName.find('/')>=0) 
    {
      mkdirhier(pathOf(destName),localShareDir);
      value = localShareDir + destName;
    }
    else value = iconDir + destName;
    if (installFile(icon, value)) installed++;

    if (destNameMini != "-")
    {
      if (destNameMini.find('/')>=0) 
      {
	mkdirhier(pathOf(destNameMini),localShareDir);
	value = localShareDir + destNameMini;
      }
      else value = miniIconDir + destNameMini;
      if (installFile(miniIcon, value)) installed++;
    }
  }

  // Look if there is anything to do after installation
  value = mMappings->readEntry("ConfigActivateCmd");
  if (!value.isEmpty() && (installed>0 || wantRestart) && 
      mCmdList.find(value) < 0) mCmdList.append(value);

  writeInstFileList(groupName);

  // Handle extra icons
  mConfig->setGroup(groupNameExtra);
  mMappings->setGroup(groupNameExtra);
  it = mConfig->entryIterator(groupNameExtra);
  if (it) for (entry=it->toFirst(); entry; entry=it->operator++())
  {
    key = it->currentKey();
    value = entry->aValue;
    i = value.find(':');
    if (i > 0)
    {
      icon = value.left(i);
      miniIcon = value.mid(i+1, 1024);
    }
    else
    {
      icon = value;
      miniIcon = "mini-" + icon;
      iconToMiniIcon(mThemePath + icon, mThemePath + miniIcon);
    }

    // Install icons
    value = iconDir + icon;
    if (installFile(icon, value)) installed++;

    value = miniIconDir + icon;
    if (installFile(miniIcon, value)) installed++;
  }

  writeInstFileList(groupName);
//  debug("*** done with %s (%d icons installed)", groupName, installed);

  mInstIcons += installed;
  return installed;
}


//-----------------------------------------------------------------------------
void Theme::addInstFile(const char* aFileName)
{
  if (aFileName && *aFileName && mInstFiles.find(aFileName) < 0)
    mInstFiles.append(strdup(aFileName));
}


//-----------------------------------------------------------------------------
void Theme::readInstFileList(const char* aGroupName)
{
  KConfig* cfg = kapp->getConfig();

  assert(aGroupName!=0);
  cfg->setGroup("Installed Files");
  mInstFiles.clear();
  cfg->readListEntry(aGroupName, mInstFiles, ':');
}


//-----------------------------------------------------------------------------
void Theme::writeInstFileList(const char* aGroupName)
{
  KConfig* cfg = kapp->getConfig();

  assert(aGroupName!=0);
  cfg->setGroup("Installed Files");
  cfg->writeEntry(aGroupName, mInstFiles, ':');
}


//-----------------------------------------------------------------------------
void Theme::uninstallFiles(const char* aGroupName)
{
  QString cmd, fname, value;
  QFileInfo finfo;
  bool reverted = false;
  int processed = 0;
  QStrList fileList;

//  debug("*** beginning uninstall of %s", aGroupName);

  readInstFileList(aGroupName);
  for (fname=mInstFiles.first(); !fname.isEmpty(); fname=mInstFiles.next())
  {
    reverted = false;
    if (unlink(fname)==0) reverted = true;

    if (reverted) 
    {
//      debug("uninstalled %s", (const char*)fname);
      processed++;
    }
  }
  mInstFiles.clear();
  writeInstFileList(aGroupName);

//  debug("*** done uninstall of %s", aGroupName);
}

void Theme::resync()
{
// Reread the .themerc file
  if (mConfig) {
      delete mConfig; mConfig = 0;
  }
  // read theme config file
  mConfig = new KSimpleConfig( mThemePath + mThemercFile, true);

  readConfig();
}

//-----------------------------------------------------------------------------
void Theme::install(void)
{
  debug("Theme::install() started");

  resync(); // Sync with the version on disk

  loadMappings();
  mCmdList.clear();

  if (instPanel) installGroup("Panel");
  if (instKonsole) installGroup("Konsole");
  if (instSounds) installGroup("Sounds");
  if (instWindowBorder) installGroup("Window Border");
  if (instWindowTitlebar) installGroup("Window Titlebar");
  if (instWindowButtonLayout) installGroup("Window Button Layout");
  if (instWindowGimmick) installGroup("Window Gimmick");
  if (instWallpapers) installGroup("Display");
  if (instKfm) installGroup("File Manager");
  if (instColors) installGroup("Colors");
  if (instIcons) installIcons();

  debug("*** executing command list");

  doCmdList();

  debug("Theme::install() done");
  saveSettings();
}


//-----------------------------------------------------------------------------
void Theme::readCurrent(void)
{
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

  mConfig->setGroup("General");
  mDescription = mConfig->readEntry("description", mName + " Theme");
  mVersion = mConfig->readEntry("Version");
  mAuthor = mConfig->readEntry("Author");
  mEmail = mConfig->readEntry("Email");
  mHomepage = mConfig->readEntry("Homepage");

  mConfig->setGroup("Colors");
  foregroundColor = readColorEntry(mConfig, "foreground", &col);
  backgroundColor = readColorEntry(mConfig, "background", &col);
  selectForegroundColor = readColorEntry(mConfig, "selectForeground", &col);
  selectBackgroundColor = readColorEntry(mConfig, "selectBackground", &col);
  activeForegroundColor = readColorEntry(mConfig, "activeForeground", &col);
  activeBackgroundColor = readColorEntry(mConfig, "activeBackground", &col);
  activeBlendColor = readColorEntry(mConfig, "activeBlend", &col);
  inactiveForegroundColor = readColorEntry(mConfig, "inactiveForeground", &col);
  inactiveBackgroundColor = readColorEntry(mConfig, "inactiveBackground", &col);
  inactiveBlendColor = readColorEntry(mConfig, "inactiveBlend", &col);
  windowForegroundColor = readColorEntry(mConfig, "windowForeground", &col);
  windowBackgroundColor = readColorEntry(mConfig, "windowBackground", &col);
  contrast = mConfig->readNumEntry("Contrast", 7);

  if (!mPreviewFile.isEmpty())
  {
    fname = mThemePath + mPreviewFile;
    if (!mPreview.load(fname))
    {
      warning(i18n("Failed to load preview image %s"), (const char*)fname);
      mPreview.resize(0,0);
    }
  }
  else mPreview.resize(0,0);
}


//-----------------------------------------------------------------------------
void Theme::writeConfig(void)
{
  debug("Theme::writeConfig() is broken");
  return;

  mConfig->setGroup("General");
  mConfig->writeEntry("description", mDescription);

#ifdef BROKEN
  mConfig->setGroup("Colors");
  writeColorEntry(mConfig, "BackgroundColor", backgroundColor);
  writeColorEntry(mConfig, "SelectColor", selectColor);
  writeColorEntry(mConfig, "TextColor", textColor);
  writeColorEntry(mConfig, "ActiveTitleTextColor", activeTextColor);
  writeColorEntry(mConfig, "InactiveTitleBarColor", inactiveTitleColor);
  writeColorEntry(mConfig, "ActiveTitleBarColor", activeTitleColor);
  writeColorEntry(mConfig, "InactiveTitleTextColor", inactiveTextColor);
  writeColorEntry(mConfig, "WindowTextColor", windowTextColor);
  writeColorEntry(mConfig, "WindowColor", windowColor);
  writeColorEntry(mConfig, "SelectTextColor", selectTextColor);
  mConfig->writeEntry("Contrast", contrast);
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
  if (!mConfig) return;

  KGroupIterator *it = mConfig->groupIterator();
  for (KEntryDict *grp = it->toFirst(); grp; )
  {
    QString groupName = it->currentKey();
    grp = it->operator++();
    mConfig->deleteGroup(groupName);
  }
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
  KGroupIterator* it = mConfig->groupIterator();
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
void Theme::stretchPixmap(const QString aFname, bool aStretchVert)
{
  QPixmap src, dest;
  QBitmap *srcMask, *destMask;
  int w, h, w2, h2;
  QPainter p;

  src.load(aFname);
  if (src.isNull()) return;

  w = src.width();
  h = src.height();

  if (aStretchVert)
  {
    w2 = w;
    for (h2=h; h2<64; h2=h2<<1)
      ;
  }
  else
  {
    h2 = h;
    for (w2=w; w2<64; w2=w2<<1)
      ;
  }

  dest = src;
  dest.resize(w2, h2);

  p.begin(&dest);
  p.drawTiledPixmap(0, 0, w2, h2, src);
  p.end();

  srcMask = (QBitmap*)src.mask();
  if (srcMask)
  {
    destMask = (QBitmap*)dest.mask();
    p.begin(destMask);
    p.drawTiledPixmap(0, 0, w2, h2, *srcMask);
    p.end();
  }

  dest.save(aFname, QPixmap::imageFormat(aFname));
}


//-----------------------------------------------------------------------------
void Theme::rotateImage(const QString aFname, int aAngle)
{
  QPixmap src, dest;
  QWMatrix mx;

  src.load(aFname);
  if (src.isNull()) return;

  mx.rotate(aAngle);
  dest = src.xForm(mx);

  dest.save(aFname, QPixmap::imageFormat(aFname));
}


//-----------------------------------------------------------------------------
void Theme::iconToMiniIcon(const QString aIcon, const QString aMiniIcon)
{
  QPixmap src, dest;
  QWMatrix mx;

  src.load(aIcon);
  if (src.isNull()) return;

  mx.scale(.5, .5);
  dest = src.xForm(mx);

  dest.save(aMiniIcon, QPixmap::imageFormat(aIcon));
}


//-----------------------------------------------------------------------------
void Theme::runKrdb(void) const
{
  KSimpleConfig cfg("kcmdisplayrc", true);

  cfg.setGroup("X11");
  if (cfg.readBoolEntry("useResourceManager", false))
    system("krdb");
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


//-----------------------------------------------------------------------------
const QString Theme::fileOf(const QString& aName) const
{
  int i = aName.findRev('/');
  if (i >= 0) return aName.mid(i+1, 1024);
  return aName;
}


//-----------------------------------------------------------------------------
const QString Theme::pathOf(const QString& aName) const
{
  int i = aName.findRev('/');
  if (i >= 0) return aName.left(i);
  return aName;
}


//-----------------------------------------------------------------------------
#include "theme.moc"
