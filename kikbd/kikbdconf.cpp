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
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <qfileinf.h>
#include <qdir.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qdict.h>

#include <kconfig.h>
#include <kapp.h>
#include <kmsgbox.h>
#include <kiconloader.h> 

#include "kconfobjs.h"
#include "kikbdconf.h"
#include "kikbdconf.moc"

//=========================================================
//    data
//=========================================================
static const char* confMainGroup        = "International Keyboard";
static const char* confStartupGroup     = "StartUp";
static const char* confMapGroups[] = {
  "KeyboardMap",
  "ComposeMap1",
  "ComposeMap2",
  "ComposeMap3"
};
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
static const char* confStringBakColor   = "BackgroundColor";
static const char* confStringFont       = "Font";
static const char* confStringCustFont   = "CustomizeFont";
static const char* confStringAutoStart  = "AutoStart";
static const char* confStringDocking    = "Docking";  
static const char* confStringHotList    = "HotList";  
static const char* confStringCodes      = "Codes";  
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
  "Meta_R",
  "Alt_L",
  "Control_L",
  "Meta_L"
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
  gettext("Right Meta"),
  gettext("Left  Alt"),
  gettext("Left  Control"),
  gettext("Left  Meta"),
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
//   special widget
//=========================================================
class KiKbdCodesObject: public KConfigComboObject {
protected:
  KiKbdConfig* owner;
public:
  KiKbdCodesObject(KiKbdConfig* owner)
    :KConfigComboObject(confStringCodes, *((QString*)&owner->getCodes()),
			QStrList(), 0),
     owner(owner) {}
  virtual QWidget* createWidget(QWidget* parent=0L, const char* name=0L)
    {
      list = owner->availableMaps("codes");
      if(labels) delete labels;
      labels = new QStrList();
      for(unsigned i=0; i<list.count(); i++)
	labels->append(owner->getMap(list.at(i))->getLabel());
      // preppend x default
      list.insert(0, "");
      labels->insert(0, i18n("X default codes"));
      num = list.count();
      return KConfigComboObject::createWidget(parent, name);
    }
};

//=========================================================
//   config class
//=========================================================
KiKbdConfig::KiKbdConfig(bool readOnly)
  :KObjectConfig(UserFromSystemRc, 0L, readOnly)
{
  setVersion(1);
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
	<< new KConfigColorObject(confStringBakColor, bakColor)
	<< new KConfigFontObject(confStringFont,  font)
	<< new KiKbdCodesObject(this)
	<< setGroup(confStartupGroup)
	<< new KConfigBoolObject(confStringAutoStart, autoStart)
	<< new KConfigBoolObject(confStringDocking, docking)
	<< new KConfigComboObject(confStringAutoStartPlace, autoStartPlace,
				  autoStartPlaceLabels,
				  sizeof(autoStartPlaceLabels)
				  /sizeof(*autoStartPlaceLabels));
  connect(this, SIGNAL(newUserRcFile()), SLOT(newUserRc())); 
  connect(this, SIGNAL(wrongVersion(int)) , SLOT(wrongVersion(int))); 
  //CT connect(this, SIGNAL(newerVersion(float)) , SLOT(newerVersion(float))); 
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
  if(!KiKbdConfig::isConfigProgram()) 
  {
    if(KiKbdMsgBox::yesNo(gettext("%s\nDo you want to configure now?"),
			  msg))
      KiKbdConfig::startConfigProgram();
  } 
  else KiKbdMsgBox::warning(msg);
}
void KiKbdConfig::newUserRc()
{
  ask(gettext("You're using \"International Keyboard\" for the first time."
	      "\nI installed a number of default settings."));
}
//CT 17Jan1999 - rewritten
int doneCheck = FALSE;
void KiKbdConfig::wrongVersion(int how)
{
  if(!doneCheck) {
    QString tmp = "Your present configuration file uses a" ;
    if ( how < 0 )
      tmp = tmp + "n older ";
    else 
      tmp =tmp + " newer ";
    tmp = tmp +  "format than needed.\nSome settings may be incorrect.";
    ask(gettext(tmp));
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
bool KiKbdConfig::hasCompose()
{
  unsigned i;for(i=0; i<maps.count(); i++)
    if(getMap(maps.at(i))->getHasCompose()) return TRUE;
  return FALSE;
}
void KiKbdConfig::setDefaults()
{
  //CT setting autoStart to false by default
  autoStart = FALSE;
  keyboardBeep = hotList     = /*CT autoStart = */docking     = TRUE;
  autoMenu     = emuCapsLock = custFont  = saveClasses = FALSE;
  switchComb     = "Control_R+Shift_R";
  altSwitchComb  = "Alt_R";
  codes          = "";
  autoStartPlace = input = 0;
  forColor  = white;
  bakColor  = QColor(0, 128, 128);
  capsColor = QColor(128, 0, 128);
  altColor  = QColor(128, 128, 0);
  font      = QFont("Helvetica");
  maps.clear(); maps.append("en");
  markDataChanged();
}
bool KiKbdConfig::oneKeySwitch() const
{
  return (!switchComb.contains('+')) && (switchComb != "None");
}
QStrList KiKbdConfig::availableMaps(const char* subdir)
{
  static QDict<QStrList> *dict = 0L;

  QStrList* list;
  if(dict) {
    if((list = dict->find(subdir))) return *list;
  } else {
    dict = new QDict<QStrList>;
    dict->setAutoDelete(TRUE);
  }
  list = new QStrList();
  dict->insert(subdir, list);

  QStrList dirs;
  dirs.append(kapp->kde_datadir() + "/kikbd/"+subdir);
  dirs.append(kapp->localkdedir() + "/share/apps/kikbd/"+subdir);
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
	  if(*subdir) name = QString(subdir) + "/" + name;
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
void KiKbdConfig::startConfigProgram(int opt)
{
  int pid = 0;

  switch(opt) {
  case Config_Normal: 
    pid = fork();
    if (!pid) {
      execlp("kcmikbd", "kcmikbd", 0L);
      ::exit(1);
    } else {
      waitpid(pid, NULL, 0);
    }
    break;
  case Config_StartKikbd: execlp("kcmikbd", "kcmikbd", "-startkikbd", 0L);
  }
}
bool KiKbdConfig::isConfigProgram()
{
  return QString(kapp->argv()[0]).find("kikbd") == -1;
}

//=========================================================
// map configuration
//=========================================================
KiKbdMapConfig::KiKbdMapConfig(const char* nm):name(nm)
{
  KObjectConfig config(KObjectConfig::AppData, name + ".kimap");
  QStrList symList[4], codeList[4];
  config << config.setGroup(confMainGroup)
	 << new KConfigStringObject(confStringLabel, label)
	 << new KConfigStringObject(confStringComment, comment)
	 << new KConfigStringObject(confStringLanguage, language)
	 << new KConfigStringObject(confStringCharset, charset)
	 << new KConfigStringObject(confStringLocale, locale)
	 << new KConfigStrListObject(confStringAuthors , authors);
  unsigned g;for(g=0; g<4; g++) {
    config << config.setGroup(confMapGroups[g])
	   << new KConfigMatchKeysObject(QRegExp("^keysym[0-9]+$"), symList[g])
	   << new KConfigMatchKeysObject(QRegExp("^keycode[0-9]+$"), codeList[g])
	   << new KConfigStrListObject(confStringCaps, capssyms[g]);
  }
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
  for(g=0; g<4; g++) {
    /*--- parsing ---*/
    keysyms[g].setAutoDelete(TRUE);
    keycodes[g].setAutoDelete(TRUE);
    capssyms[g].setAutoDelete(TRUE);
    if(g < 1) hasAltKeys = FALSE;
    hasCompose = FALSE;
    if(g > 0 && (symList[g].count() > 0 || codeList[g].count() > 0))
      hasCompose = TRUE;
    if(hasCompose && g > 1 && (symList[g].count() < 1 && codeList[g].count() < 1))
      hasCompose = FALSE;
    if(hasCompose && g > 2 && (symList[g].count() < 1 && codeList[g].count() < 1))
      hasCompose = FALSE;
    /*--- parse key symbols ---*/
    unsigned i;for(i=0; i<symList[g].count(); i++) {
      QStrList *map = new QStrList;
      *map = KObjectConfig::separate(symList[g].at(i));
      keysyms[g].append(map);
      if(g < 1 && !hasAltKeys && map->count() > 3) hasAltKeys = TRUE;
      if(!hasAltKeys && map->count() > 3) hasAltKeys = TRUE;
    }
    /*--- parse key codes ---*/
    for(i=0; i<codeList[g].count(); i++) {
      QStrList *map = new QStrList;
      *map = KObjectConfig::separate(codeList[g].at(i));
      keycodes[g].append(map);
      if(g < 1 && !hasAltKeys && map->count() > 3) hasAltKeys = TRUE;
    }
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
  if(hasCompose) {
    unsigned g, c;
    for (g=c=0; g<4; g++) c += keysyms[g].count();
    num.setNum(c);
  }
  else num.setNum(keysyms[0].count());
  com += num + " ";
  com += i18n("symbols");
  if(hasCompose) {
    unsigned g, c;
    for (g=c=0; g<4; g++) c += keycodes[g].count();
    num.setNum(c);
  }
  else num.setNum(keycodes[0].count());
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
  //CT 18Jan1999 - temporary solution for the inversion thing 
  //      (where "Espagnol idioma" isn't right but "English language" is)
  //  label = language + " " + i18n("language");
  //  if(!charset.isEmpty()) label += ", " + charset + " " + i18n("charset");
  //  return label + ".";
  label = i18n("Language");
  label += ": ";
  label += language;
  label += ". ";
  label += i18n("Charset");
  label += ": ";
  if (charset.isEmpty())
    label += i18n("default");
  else
    label += charset;
  return label + ".";
  //CT don't ask me why the compiler had me to do this lame concatenation
  //   this way. Perhaps because it evaluates from left to right. I could
  //   have used QString::prepend()
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
  msg.sprintf(i18n(form), i18n(s1), i18n(s2));
  KMsgBox::message(0, i18n("International keyboard ERROR"), msg,
		   KMsgBox::EXCLAMATION);
  ::exit(1);
}
/**
   we use this to show dialog with error
*/
QString setMsg(const char* form, const char* s1, const char *s2)
{
  QString msg(128);
  msg.sprintf(i18n(form), i18n(s1), i18n(s2));
  return msg;
}
void KiKbdMsgBox::warning(const char* form, const char* s1, 
			  const char *s2)
{
  KMsgBox::message(0, i18n("International keyboard warning"),
		   setMsg(form, s1, s2),
		   KMsgBox::INFORMATION);
}

int KiKbdMsgBox::yesNo(const char* form, const char* s1, const char *s2)
{
  return KMsgBox::yesNo(0, i18n("International keyboard question"),
			setMsg(form, s1, s2)) == 1;
}
void KiKbdMsgBox::ask(const char* form, const char* s1, const char *s2)
{
  switch(KMsgBox::yesNoCancel(0, i18n("International keyboard question"),
			      setMsg(form, s1, s2),
			      KMsgBox::QUESTION, i18n("Configure"),
			      i18n("Continue"), i18n("Quit"))) {
  case 2: return;
  case 1: KiKbdConfig::startConfigProgram(KiKbdConfig::Config_StartKikbd);
  case 3: ::exit(0);
  }
}
void KiKbdMsgBox::askContinue(const char* form, const char* s1, const char *s2)
{
  QString msg = setMsg(form, s1, s2);
  msg += i18n("\nDo you want to continue?");
  if(KMsgBox::yesNo(0, i18n("International keyboard question"),
		    msg) == 1) return;
  ::exit(0);
}
