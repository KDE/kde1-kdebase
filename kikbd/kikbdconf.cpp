/*
   - 

  written 1998 by Alexander Budnik <budnik@linserv.jinr.ru>
  
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
#include <stream.h>
#include <stdlib.h>
#include <qfileinf.h>
#include <qdir.h>
#include <qcolor.h>

#include <kconfig.h>
#include <kapp.h>
#include <kmsgbox.h>
#include <kiconloader.h> 

#include "kikbdconf.h"
#include "kconfobjs.h"

//=========================================================
//    data
//=========================================================
static const char* confMainGroup        = "International Keyboard";
static const char* confStartupGroup     = "StartUp";
static const char* confMapsGroup        = "KeyboardMap";

static const char* confStringBeep       = "Beep";
static const char* confStringSwitch     = "Switch";
static const char* confStringAltSwitch  = "AltSwitch";
static const char* confStringAutoMenu   = "WorldMenu";
static const char* confStringEmuCapsLock= "EmulateCapsLock";
static const char* confStringSaveClasses= "SaveClasses";
static const char* confStringInput      = "Input";
static const char* confStringMap        = "Map";
static const char* confStringLabel      = "Label";
static const char* confStringComment    = "Comment";
static const char* confStringLanguage   = "Language";
static const char* confStringLocale     = "Locale";
static const char* confStringCharset    = "Charset";
static const char* confStringAuthors    = "Authors";
static const char* confStringCaps       = "CapsSymbols";
static const char* confStringCapsColor  = "CapsLockColor";
static const char* confStringAltColor   = "AltColor";
static const char* confStringForColor   = "ForegroundColor";
static const char* confStringFont       = "Font";
static const char* confStringCustFont   = "CustomizeFont";
static const char* confStringAutoStart  = "AutoStart";
static const char* confStringDocking    = "Docking";  
static const char* confStringHotList    = "HotList";  
static const char* confStringAutoStartPlace  = "AutoStartPlace";
static const char* swConfigString[] = {
  "None",
  "Alt_R",
  "Control_R",
  "Alt_R+Shift_R",
  "Control_R+Alt_R",
  "Control_R+Shift_R",
  "Alt_L+Shift_L",
  "Control_L+Alt_L",
  "Control_L+Shift_L",
  "Shift_L+Shift_R"
};
static const char* swConfigAltString[] = {
  "None",
  "Alt_R",
  "Control_R",
  "Alt_L",
  "Control_L"
};
static const char* inpConfigString[] = {
  "Global", "Window", "Class"
};
static const char* autoStartPlaceLabels[] = {
  "topleft",
  "topright",
  "botleft",
  "botright"
};
const QColor mapNormalColor = black;
const QColor mapUserColor   = darkBlue;
const QColor mapNoFileColor = darkRed;

//=========================================================
//   config class
//=========================================================
KiKbdConfig::KiKbdConfig():KObjectConfig(UserFromSystemRc)
{
  setGroup(confMainGroup);
  registerBool(confStringHotList    , hotList);
  registerBool(confStringBeep       , keyboardBeep);
  registerBool(confStringAutoMenu   , autoMenu);
  registerBool(confStringEmuCapsLock, emuCapsLock);
  registerBool(confStringCustFont   , custFont);
  registerBool(confStringSaveClasses, saveClasses);
  registerObject(new KConfigNumberedKeysObject(confStringMap, 0, 9, maps));
  registerObject(new KConfigComboObject(confStringSwitch, switchComb,
					swConfigString,
					sizeof(swConfigString)
					/sizeof(*swConfigString)));	
  registerObject(new KConfigComboObject(confStringAltSwitch, altSwitchComb,
					swConfigAltString,
					sizeof(swConfigAltString)
					/sizeof(*swConfigAltString)));	
  registerObject(new KConfigComboObject(confStringInput, input,
					inpConfigString,
					sizeof(inpConfigString)
					/sizeof(*inpConfigString)));
  registerColor(confStringCapsColor, capsColor);
  registerColor(confStringAltColor,  altColor);
  registerColor(confStringForColor,  forColor);
  registerFont(confStringFont,  font);
  maps.setAutoDelete(TRUE);
  allMaps.setAutoDelete(TRUE);
  setGroup(confStartupGroup);
  registerBool(confStringAutoStart, autoStart);
  registerBool(confStringDocking, docking);
  registerObject(new KConfigComboObject(confStringAutoStartPlace, 
					autoStartPlace,
					autoStartPlaceLabels,
					sizeof(autoStartPlaceLabels)
					/sizeof(*autoStartPlaceLabels)));
  connect(this, SIGNAL(newUserRcFile()), SLOT(newUserRc()));					  
}
void KiKbdConfig::newUserRc()
{
  message(klocale->translate("Your configuration is empty. Install system default."));
  if(QString(kapp->argv()[0]).find("kikbd") != -1) {
    if(KMsgBox::yesNo(0L, "kikbd", 
		      klocale->translate("Do you want to start Configuration?"))
       == 1)
      system("kcmikbd&");
  }
}
int KiKbdConfig::getInput()
{
  unsigned i;for(i=sizeof(inpConfigString)/sizeof(*inpConfigString); --i;)
    if(input == inpConfigString[i]) return i;
  return 0;
}
QStrList KiKbdConfig::getSwitch()
{
  return KConfigMatchKeysObject::separate(switchComb, '+');
}
QStrList KiKbdConfig::getAltSwitch()
{
  return KConfigMatchKeysObject::separate(altSwitchComb, '+');
}
QStrList KiKbdConfig::availableMaps()
{
  QStrList list, dirs;
  dirs.append(kapp->kde_datadir() + "/kikbd");
  dirs.append(kapp->localkdedir() + "/share/apps/kikbd");
  unsigned i;for(i=0; i<dirs.count(); i++)
    {
      QDir dir(dirs.at(i));  
      if(!dir.exists()) continue;
      QStrList entry = *dir.entryList("*.kimap",
				     QDir::Files | QDir::Readable,
				     QDir::Name | QDir::IgnoreCase);
      unsigned j;for(j=0; j<entry.count(); j++)
	{
	  QString name = entry.at(j);
	  name.resize(name.find(".")+1);
	  if(list.find(name) == -1) list.inSort(name);
	}
    }
  return list;
}
KiKbdMapConfig* KiKbdConfig::getMap(const char* name)
{
  unsigned i;for(i=0; i<allMaps.count(); i++)
    if(allMaps.at(i)->name == name) return allMaps.at(i);
  allMaps.append(new KiKbdMapConfig(name));
  return allMaps.current();
}
bool KiKbdConfig::hasAltKeys()
{
  unsigned i;for(i=0; i<maps.count(); i++)
    if(getMap(maps.at(i))->getHasAltKeys()) return TRUE;
  return FALSE;
}
bool KiKbdConfig::oneKeySwitch()
{
  return (!switchComb.contains('+')) && (switchComb != "None");
}
void KiKbdConfig::error(const char* form, const char* str,
			const char* str2)
{
  QString msg(128);
  msg.sprintf(form, str, str2);
  if(KMsgBox::yesNo(0, klocale->translate("kikbd configuration error"), msg) == 2) 
    ::exit(1);
}
void KiKbdConfig::message(const char* form, const char* str)
{
  QString msg(128);
  msg.sprintf(form, str);
  KMsgBox::message(0L, klocale->translate("kikbd configuration message"), msg);
}

//=========================================================
// map configuration
//=========================================================
KiKbdMapConfig::KiKbdMapConfig(const char* nm):name(nm)
{
  QString file = name + ".kimap";
  KObjectConfig config(KObjectConfig::AppData, file);
  config.setGroup(confMainGroup);
  config.registerString(confStringLabel   , label   );
  config.registerString(confStringComment , comment );
  config.registerString(confStringLanguage, language);
  config.registerString(confStringCharset , charset );
  config.registerString(confStringLocale  , locale  );
  config.registerStrList(confStringAuthors , authors);
  config.setGroup(confMapsGroup);
  QStrList symList, codeList;
  config.registerObject(new KConfigMatchKeysObject(QRegExp("^keysym[0-9]+$"),
						   symList));
  config.registerObject(new KConfigMatchKeysObject(QRegExp("^keycode[0-9]+$"),
						   codeList));
  config.registerStrList(confStringCaps, capssyms);
  userData = TRUE;
  noFile   = FALSE;
  connect(&config, SIGNAL(noUserDataFile(const char*)),
	  SLOT(noUserDataFile(const char*)));
  connect(&config, SIGNAL(noSystemDataFile(const char*)),
	  SLOT(noSystemDataFile(const char*)));
  config.loadConfig();
  /*--- check information ---*/
  if(comment.isNull() ) comment  = "";
  if(language.isNull()) language = "unknown";
  if(charset.isEmpty()) charset  = "unknown";
  if(label.isNull() || label == "")
    if(!locale.isNull()) label = locale; else label = name;
  if(locale.isNull() || locale == "")
    locale = klocale->translate("default");
  /*--- parsing ---*/
  keysyms.setAutoDelete(TRUE);
  keycodes.setAutoDelete(TRUE);
  capssyms.setAutoDelete(TRUE);
  hasAltKeys = FALSE;
  /*--- pars key symbols ---*/
  unsigned i;for(i=0; i<symList.count(); i++) {
    QStrList *map = new QStrList;
    *map = KConfigMatchKeysObject::separate(symList.at(i));
    keysyms.append(map);
    if(!hasAltKeys && map->count() > 3) hasAltKeys = TRUE;
  }
  /*--- pars key codes ---*/
  for(i=0; i<codeList.count(); i++) {
    QStrList *map = new QStrList;
    *map = KConfigMatchKeysObject::separate(codeList.at(i));
    keycodes.append(map);
    if(!hasAltKeys && map->count() > 3) hasAltKeys = TRUE;
  }
}
const QString KiKbdMapConfig::getInfo()
{
  QString com;
  // authors
  com = klocale->translate(authors.count()<2?"Author":"Authors");
  com += ":  ";
  com += authors.count()>0?authors.at(0):"Not specified";
  for(unsigned i = 1; i < authors.count(); i++) {
    com += ",  ";
    com += authors.at(i);
  }
  com += "\n\n";
  // description
  com += klocale->translate("Description:  Keyboard map for language \"");
  com += language + klocale->translate("\" with ");
  com += charset + " ";
  com += klocale->translate("charset");
  com += ".";
  if(!comment.isEmpty()) com += " " + comment;
  com += "\n\n";
  // source
  com += klocale->translate("Source:  ");
  com += noFile?klocale->translate("no file")
    :(userData?klocale->translate("user file")
    :klocale->translate("system file"));
  com += " \"" + name + ".kimap\"";
  com += "\n";
  // statistic
  QString num;
  com += klocale->translate("Statistic:  ");
  num.setNum(keysyms.count());
  com += num + " ";
  com += klocale->translate("symbols");
  num.setNum(keycodes.count());
  com += ", " + num + " ";
  com += klocale->translate("codes");
  if(hasAltKeys) {
    com += ", ";
    com += klocale->translate("alternative keys");
  }
  return com;
}
const QString KiKbdMapConfig::getGoodLabel() const
{
  QString item(128);
  item.sprintf("%s (%s %s)", (const char*)comment,
               klocale->translate("Label"),
               (const char*)label);
  if(userData) {
    item += klocale->translate("(User)");
  }
  return item;
}
const QColor KiKbdMapConfig::getColor() const
{
  return noFile?mapNoFileColor:(userData?mapUserColor:mapNormalColor);
}
const QPixmap KiKbdMapConfig::getIcon() const
{
  KIconLoader loader;
  QString file = kapp->kde_datadir()+"/kcmlocale/pics/flag_"
			 +locale+".gif";
  return loader.loadIcon(file);
}
