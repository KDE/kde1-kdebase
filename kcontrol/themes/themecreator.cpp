/*
 * themecreator.cpp
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

#include "themecreator.h"
#include <kapp.h>
#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>


//-----------------------------------------------------------------------------
ThemeCreator::ThemeCreator(): ThemeCreatorInherited()
{
}


//-----------------------------------------------------------------------------
ThemeCreator::~ThemeCreator()
{
}


//-----------------------------------------------------------------------------
bool ThemeCreator::create(const QString aThemeName)
{
  if (aThemeName.isEmpty()) return false;

  debug("Theme::create() started");

  clear();
  cleanupWorkDir();

  setName(aThemeName);
  mThemePath = workDir() + aThemeName + '/';
  if (mkdir(mThemePath, 0755))
  {
    warning(i18n("Failed to create directory %s: %s"),
	    (const char*)mThemePath, strerror(errno));
    return false;
  }

  mThemercFile = aThemeName + ".themerc";
  mPreviewFile = 0;
  mPreview.resize(0,0);

  if (mConfig)
  {
    delete mConfig; mConfig = 0;
  }

  // Create Read/Write config file
  mConfig = new KSimpleConfig( mThemePath + mThemercFile, false);

  setGroupGeneral();

  emit changed();
  return true;
}


//-----------------------------------------------------------------------------
bool ThemeCreator::extract(void)
{
  debug("Theme::extract() started");

  loadMappings();

  if (instPanel) extractGroup("Panel");
  if (instSounds) extractGroup("Sounds");
  if (instWindowBorder) extractGroup("Window Border");
  if (instWindowTitlebar) extractGroup("Window Titlebar");
  if (instWindowButtonLayout) extractGroup("Window Button Layout");
  if (instWindowGimmick) extractGroup("Window Gimmick");
  if (instWallpapers) extractGroup("Display");
  if (instColors) extractGroup("Colors");
  if (instIcons) extractIcons();

  debug("Theme::extract() done");

  mConfig->sync();
  saveSettings();

  return true;
}


//-----------------------------------------------------------------------------
int ThemeCreator::extractGroup(const char* aGroupName)
{
  QString value, cfgFile, cfgGroup, appDir, group, emptyValue, mapValue, str;
  QString oldCfgFile, key, cfgKey, cfgValue, themeValue, instCmd, baseDir;
  bool absPath, doCopyFile;
  KEntryIterator* it;
  KSimpleConfig* cfg = NULL;
  KEntryDictEntry* entry;
  int len, i, extracted = 0;
  const char* missing = 0;

  debug("*** beginning with %s", aGroupName);
  group = aGroupName;
  mConfig->setGroup(group);

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
printf("%s: [%s]\n", (const char*)cfgFile, (const char*)cfgGroup);
    // Process all mapping entries for the group
    it = mMappings->entryIterator(group);
    if (it) for (entry=it->toFirst(); entry; entry=it->operator++())
    {
      key = it->currentKey();
      if (stricmp(key.left(6),"Config")==0) continue;
      mapValue = QString(entry->aValue).stripWhiteSpace();
      len = mapValue.length();
      if (len>0 && mapValue[len-1]=='!')
      {
	doCopyFile = false;
	mapValue = mapValue.left(len-1);
      }
      else doCopyFile = true;

      // parse mapping
      i = mapValue.find(':');
      if (i >= 0)
      {
	cfgKey = mapValue.left(i);
	cfgValue = mapValue.mid(i+1, 1024);
      }
      else 
      {
	cfgKey = mapValue;
	cfgValue = 0;
      }
      if (cfgKey.isEmpty()) cfgKey = key;
      value = cfg->readEntry(cfgKey);

printf("Line 220: cfgKey = %s cfgValue = %s value =%s\n", 
(const char *) cfgKey, (const char *) cfgValue, (const char *) value);

      if (doCopyFile)
      {
printf("DoCopyFile!\n");
        if (!cfgValue.isEmpty()) {
// Add path!
          value =appDir+cfgValue;
        }
	if (!value.isEmpty())
	{
printf("extractFile! value =%s\n", (const char *) (value));
	  str = extractFile(value);
	  if (!str.isEmpty())
	  {
	    extracted++;
	    value = str;
	  }
	  value = fileOf(value);
	}
      }

      // Set config entry
      if (value == emptyValue) value = "";
      debug("%s=%s", (const char*)key, (const char*)value);
printf("Line 264: ey = %s value =%s\n", 
(const char *) key, (const char *) value);
      if (value.isEmpty()) {
         mConfig->deleteEntry(key, false);
printf("deleteEntry..\n");
      } else {
         mConfig->writeEntry(key, value);
printf("writeEntry..\n");
      }
    }

    if (!instCmd.isEmpty()) extractCmd(cfg, instCmd, extracted);
    group = mMappings->readEntry("ConfigNextGroup");
  }

  if (cfg)
  {
    debug("closing config file");
    cfg->sync();
    delete cfg;
  }

  debug("*** done with %s", aGroupName);
  return extracted;
}


//-----------------------------------------------------------------------------
void ThemeCreator::extractIcons(void)
{
}


//-----------------------------------------------------------------------------
void ThemeCreator::extractCmd(KSimpleConfig* aCfg, const QString& aCmd,
			      int aInstalled)
{
  QString grp = aCfg->group();
  QString value, cmd;

  cmd = aCmd.stripWhiteSpace();

  if (cmd == "winTitlebar")
  {
    if (aCfg->readEntry("TitlebarLook") != "pixmap")
    {
      mConfig->deleteEntry("TitlebarPixmapActive", false);
      mConfig->deleteEntry("TitlebarPixmapInactive", false);
    }
  }
  else if (cmd == "panelBack")
  {
    value = aCfg->readEntry("Position");
    if (stricmp(value,"right")==0 || stricmp(value,"left")==0)
    {
      value = mConfig->readEntry("background");
      debug("rotating %s", (const char*)value);
      rotateImage(mThemePath + value, 90);
    }
  }

  aCfg->setGroup(grp);
}


//-----------------------------------------------------------------------------
const QString ThemeCreator::extractFile(const QString& aFileName)
{
  QFileInfo finfo(aFileName);
  QFile srcFile, destFile;
  char buf[32768];
  QString fname, ext, str;
  int len, i, j, num;

  if (!finfo.exists() || !finfo.isFile())
  {
printf("File %s does not exist or is no file.", (const char*)aFileName);
    debug("File %s does not exist or is no file.", (const char*)aFileName);
    return 0;
  }

  fname = fileOf(aFileName);
  while (1) // find a unique filename in the work directory
  {
    finfo.setFile(mThemePath + fname);
    if (!finfo.exists()) break;
    i = fname.findRev('.');
    if (i >= 0) 
    {
      ext = fname.mid(i, 255);
      fname = fname.left(i);
    }
    else ext = 0;
    for (j=i-1, num=0; j>=0 && fname[j]>='0' && fname[j]<='9'; j--)
      num = num*10 + (int)(fname[j] - '0');
    j++;
    num++;
    fname[j] = '\0';
    str.sprintf("%s%d%s", (const char*)fname, num, (const char*)ext);
    fname = str;
  }

printf("Extracting %s to %s", (const char*)aFileName,
	(const char*)(mThemePath + fname));
  debug("Extracting %s to %s", (const char*)aFileName,
	(const char*)(mThemePath + fname));

  srcFile.setName(aFileName);
  if (!srcFile.open(IO_ReadOnly))
  {
    warning(i18n("Cannot open file %s for reading"), (const char*)aFileName);
    return 0;
  }

  destFile.setName(mThemePath + fname);
  if (!destFile.open(IO_WriteOnly))
  {
    warning(i18n("Cannot open file %s for writing"), 
	    (const char*)(mThemePath + fname));
    return 0;
  }

  while (!srcFile.atEnd())
  {
    len = srcFile.readBlock(buf, 32768);
    if (len <= 0) break;
    if (destFile.writeBlock(buf, len) != len)
    {
      warning(i18n("Write error to %s:\n%s"), 
	      (const char*)(mThemePath + fname), strerror(errno));
      return 0;
    }
  }

  srcFile.close();
  destFile.close();

  return fname;
}


//-----------------------------------------------------------------------------
void ThemeCreator::setGroupGeneral(void)
{
  KConfig* cfg = kapp->getConfig();

  cfg->setGroup("General");
  mConfig->setGroup("General");
  mConfig->writeEntry("description", mDescription);
  mConfig->writeEntry("Author", mAuthor);
  mConfig->writeEntry("Email", mEmail);
  mConfig->writeEntry("Homepage", mHomepage);
  mConfig->writeEntry("Version", mVersion);
}


//-----------------------------------------------------------------------------
#include "themecreator.moc"
