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
#include <qpainter.h>

#include <kconfig.h>
#include <kapp.h>
#include <kmsgbox.h>
#include <kiconloader.h> 

#include "kconfobjs.h"
#include "kikbdconf.h"
#include "kikbdconf.moc.h"

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
// GUI labels
static const char* inpLabels[] = {
  gettext("Global"), gettext("Window"), gettext("Class")
};
static const char* switchLabels[] = {
  gettext("(None)"),
  gettext("Right Alt"),
  gettext("Right Control"),
  gettext("Rights (Alt+Shift)") ,
  gettext("Rights (Ctrl+Alt)")  ,
  gettext("Rights (Ctrl+Shift)"),
  gettext("Lefts  (Alt+Shift)"),
  gettext("Lefts  (Ctrl+Alt)"),
  gettext("Lefts  (Ctrl+Shift)"),
  gettext("Both Shift's (Shift+Shift)")
};
static const char* altSwitchLabels[] = {
  gettext("(None)"),
  gettext("Right Alt"),
  gettext("Right Control"),
  gettext("Left  Alt"),
  gettext("Left  Control"),
};
static const char* autoStartPlaceLabels[] = {
  gettext("Top Left"),
  gettext("Top Right"),
  gettext("Bottom Left"),
  gettext("Bottom Right")
};

const QColor mapNormalColor = black;
const QColor mapUserColor   = darkBlue;
const QColor mapNoFileColor = darkRed;

//=========================================================
//   config class
//=========================================================
KiKbdConfig::KiKbdConfig():KObjectConfig(UserFromSystemRc)
{
  setVersion(1.0);
  *this << setGroup(confMainGroup)
	<< new KConfigBoolObject(confStringHotList, hotList)
	<< new KConfigBoolObject(confStringBeep, keyboardBeep)
	<< new KConfigBoolObject(confStringAutoMenu, autoMenu)
	<< new KConfigBoolObject(confStringEmuCapsLock, emuCapsLock)
	<< new KConfigBoolObject(confStringCustFont, custFont)
	<< new KConfigBoolObject(confStringSaveClasses, saveClasses)
	<< new KConfigNumberedKeysObject(confStringMap, 0, 9, maps)
	<< new KConfigComboObject(confStringSwitch, switchComb, swConfigString,
				  sizeof(swConfigString)
				  /sizeof(*swConfigString), switchLabels)
	<< new KConfigComboObject(confStringAltSwitch, altSwitchComb,
				  swConfigAltString, sizeof(swConfigAltString)
				  /sizeof(*swConfigAltString), altSwitchLabels)
	<< new KConfigComboObject(confStringInput, input, inpConfigString,
				  sizeof(inpConfigString)
				  /sizeof(*inpConfigString), inpLabels,
				  KConfigComboObject::ButtonGroup)
	<< new KConfigColorObject(confStringCapsColor, capsColor)
	<< new KConfigColorObject(confStringAltColor, altColor)
	<< new KConfigColorObject(confStringForColor, forColor)
	<< new KConfigFontObject(confStringFont,  font)
	<< setGroup(confStartupGroup)
	<< new KConfigBoolObject(confStringAutoStart, autoStart)
	<< new KConfigBoolObject(confStringDocking, docking)
	<< new KConfigComboObject(confStringAutoStartPlace, autoStartPlace,
				  autoStartPlaceLabels,
				  sizeof(autoStartPlaceLabels)
				  /sizeof(*autoStartPlaceLabels));
  connect(this, SIGNAL(newUserRcFile()), SLOT(newUserRc())); 
  connect(this, SIGNAL(olderVersion(float)) , SLOT(olderVersion(float))); 
  connect(this, SIGNAL(newerVersion(float)) , SLOT(newerVersion(float))); 
  maps.setAutoDelete(TRUE);
  allMaps.setAutoDelete(TRUE);
}
void KiKbdConfig::loadConfig()
{
  setDefaults();
  KObjectConfig::loadConfig();
}
void ask(const char* msg)
{
  if(QString(kapp->argv()[0]).find("kikbd") != -1) {
    if(KiKbdMsgBox::yesNo(gettext("%s\nDo you want to start "
				  "Configuration?"), msg)) {
      system("kcmikbd -startkikbd&");
      ::exit(0);
    }
  } else KiKbdMsgBox::warning(msg);
}
void KiKbdConfig::newUserRc()
{
  ask(gettext("Your configuration is empty. Install system default."));
}
int doneCheck = FALSE;
void KiKbdConfig::olderVersion(float)
{
  if(!doneCheck) {
    ask(gettext("Configuration file you have has older format when expected.\n"
		"Some settings may be incorrect."));
    doneCheck = TRUE;
  }
}
void KiKbdConfig::newerVersion(float)
{
  if(!doneCheck) {
    ask(gettext("Configuration file you have has newer format when expected.\n"
		"Some settings may be incorrect."));
    doneCheck = TRUE;
  }
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
void KiKbdConfig::setDefaults()
{
  keyboardBeep = hotList     = autoStart = docking     = TRUE;
  autoMenu     = emuCapsLock = custFont  = saveClasses = FALSE;
  switchComb     = "Control_R+Shift_R";
  altSwitchComb  = "Alt_R";
  autoStartPlace = input = 0;
  capsColor = QColor(0, 128, 128);
  altColor  = QColor(255, 255, 0);
  forColor  = black;
  font      = QFont("Helvetica");
  maps.clear(); maps.append("en");
}
bool KiKbdConfig::oneKeySwitch() const
{
  return (!switchComb.contains('+')) && (switchComb != "None");
}
QStrList KiKbdConfig::availableMaps()
{
  static QStrList *list = 0L;

  if(list) return *list;

  list = new QStrList();
  QStrList dirs;
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
	  if(list->find(name) == -1) list->inSort(name);
	}
    }
  return *list;
}
bool KiKbdConfig::readAutoStart()
{
  KObjectConfig config(AppRc);
  bool autoStart = FALSE;
  config << config.setGroup(confStartupGroup)
	 << new KConfigBoolObject(confStringAutoStart, autoStart);
  config.loadConfig();
  return autoStart;
}

//=========================================================
// map configuration
//=========================================================
KiKbdMapConfig::KiKbdMapConfig(const char* nm):name(nm)
{
  KObjectConfig config(KObjectConfig::AppData, name + ".kimap");
  QStrList symList, codeList;
  config << config.setGroup(confMainGroup)
	 << new KConfigStringObject(confStringLabel, label)
	 << new KConfigStringObject(confStringComment, comment)
	 << new KConfigStringObject(confStringLanguage, language)
	 << new KConfigStringObject(confStringCharset, charset)
	 << new KConfigStringObject(confStringLocale, locale)
	 << new KConfigStrListObject(confStringAuthors , authors)
	 << config.setGroup(confMapsGroup)
	 << new KConfigMatchKeysObject(QRegExp("^keysym[0-9]+$"), symList)
	 << new KConfigMatchKeysObject(QRegExp("^keycode[0-9]+$"), codeList)
	 << new KConfigStrListObject(confStringCaps, capssyms);
  userData = TRUE;
  noFile   = FALSE;
  connect(&config, SIGNAL(noUserDataFile(const char*)),
	  SLOT(noUserDataFile(const char*)));
  connect(&config, SIGNAL(noSystemDataFile(const char*)),
	  SLOT(noSystemDataFile(const char*)));
  config.loadConfig();
  /*--- check information ---*/
  if(comment.isNull() ) comment  = "";
  else {
    if(comment[comment.length()-1] != '.') comment += ".";
  }
  if(language.isNull()) language = gettext("unknown");
  //if(charset.isEmpty()) charset  = "unknown";
  if(label.isNull() || label == "")
    if(!locale.isNull()) label = locale; else label = name;
  if(locale.isNull() || locale == "") locale = i18n("default");
  /*--- parsing ---*/
  keysyms.setAutoDelete(TRUE);
  keycodes.setAutoDelete(TRUE);
  capssyms.setAutoDelete(TRUE);
  hasAltKeys = FALSE;
  /*--- pars key symbols ---*/
  unsigned i;for(i=0; i<symList.count(); i++) {
    QStrList *map = new QStrList;
    *map = KObjectConfig::separate(symList.at(i));
    keysyms.append(map);
    if(!hasAltKeys && map->count() > 3) hasAltKeys = TRUE;
  }
  /*--- pars key codes ---*/
  for(i=0; i<codeList.count(); i++) {
    QStrList *map = new QStrList;
    *map = KObjectConfig::separate(codeList.at(i));
    keycodes.append(map);
    if(!hasAltKeys && map->count() > 3) hasAltKeys = TRUE;
  }
}
const QString KiKbdMapConfig::getInfo() const
{
  QStrList authors(this->authors);
  QString com;
  // authors
  if(!authors.count()) com += i18n("Author:  Not specified");
  for(unsigned i = 0; i < authors.count() && i < 4; i++) {
    com += i18n("Author");
    if(authors.count()>1) com += QString("[") + (char)(i+'1') + "]";
    com += QString(":  ") + authors.at(i) + "\n";
  }
  com += "\n";
  // description
  com += i18n("Description:  ") + getGoodLabel() + " " 
    + comment + "\n\n";
  // source
  com += i18n("Source:  ");
  com += noFile?i18n("no file")
    :(userData?i18n("user file")
    :i18n("system file"));
  com += " \"" + name + ".kimap\"";
  com += "\n";
  // statistic
  QString num;
  com += i18n("Statistic:  ");
  num.setNum(keysyms.count());
  com += num + " ";
  com += i18n("symbols");
  num.setNum(keycodes.count());
  com += ", " + num + " ";
  com += i18n("codes");
  if(hasAltKeys) {
    com += ", ";
    com += i18n("alternative symbols");
  }
  return com;
}
const QString KiKbdMapConfig::getGoodLabel() const
{
  QString label;
  label = language + " " + i18n("language");
  if(!charset.isEmpty()) label += ", " + charset + " " + i18n("charset");
  return label + ".";
}
const QColor KiKbdMapConfig::getColor() const
{
  return noFile?mapNoFileColor:(userData?mapUserColor:mapNormalColor);
}
const QPixmap KiKbdMapConfig::getIcon() const
{
  static KIconLoader *loader = 0L;
  if(!loader) {
    loader = kapp->getIconLoader();
    loader->insertDirectory(0, kapp->kde_datadir()+"/kcmlocale/pics/");
  }

  QPainter p;
  QPixmap  pm(21, 14);
  QPixmap  flag(loader->loadIcon(QString("flag_")+locale+".gif", 21, 14));

  pm.fill(white);
  p.begin(&pm);
  p.fillRect(0, 0, 20, 13, gray);
  if(!flag.isNull()) p.drawPixmap(0, 0, flag);
  p.setPen(black), p.drawText(0, 0, 20, 13, AlignCenter, label);
  p.end();

  return pm;
}
//=========================================================
// message box
//=========================================================
/**
   we use this to show dialog with error
*/
void KiKbdMsgBox::error(const char* form, const char* s1, const char *s2)
{
  QString msg(128);
  msg.sprintf(form, i18n(s1), i18n(s2));
  KMsgBox::message(0, i18n("International keyboard ERROR"), msg,
		   KMsgBox::STOP);
  ::exit(1);
}
/**
   we use this to show dialog with error
*/
void KiKbdMsgBox::warning(const char* form, const char* s1, 
			  const char *s2)
{
  QString msg(128);
  msg.sprintf(form, i18n(s1), i18n(s2));
  KMsgBox::message(0, i18n("International keyboard warning"), msg);
}

int KiKbdMsgBox::yesNo(const char* form, const char* s1, const char *s2)
{
  QString msg(128);
  msg.sprintf(form, i18n(s1), i18n(s2));
  return KMsgBox::yesNo(0, i18n("International keyboard question"), msg) == 1;
}
