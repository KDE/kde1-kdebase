#include <stream.h>
#include <stdlib.h>
#include <qfileinf.h>
#include <qdir.h>
#include <kconfig.h>
#include <kapp.h>
#include <kmsgbox.h>
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
static const char* confStringLongComment= "Long Comment";
static const char* confStringLocale     = "Locale";
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
	  list.inSort(name);
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
  config.registerString(confStringLabel, label);
  config.registerString(confStringComment, comment);
  config.registerString(confStringLongComment, longcomment);
  config.registerString(confStringLocale, locale);
  config.setGroup(confMapsGroup);
  QStrList symList, codeList;
  config.registerObject(new KConfigMatchKeysObject(QRegExp("^keysym[0-9]+$"),
						   symList));
  config.registerObject(new KConfigMatchKeysObject(QRegExp("^keycode[0-9]+$"),
						   codeList));
  config.registerStrList(confStringCaps, capssyms);
  config.loadConfig();
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
const QString KiKbdMapConfig::getGoodLabel() const
{
  QString item(128);
  if (comment.isNull() || label.isNull())
   item = "default";
  else
   item.sprintf("%s (%s %s)", (const char*)comment,
               klocale->translate("Label"),
               (const char*)label);
  return item;
}
