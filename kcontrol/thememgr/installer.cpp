/*
 * installer.cpp
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
#include <qstrlist.h>
#include <qfiledlg.h>
#include <qbttngrp.h>
#include <qframe.h>
#include <qgrpbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qchkbox.h>
#include <qfileinfo.h>
#include <qlistbox.h>
#include <qmultilinedit.h>
#include <qtooltip.h>
#include <kbuttonbox.h>
#include <kmsgbox.h>
#include <qmessagebox.h>
#include <kfiledialog.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "installer.h"
#include "themecreator.h"
#include "newthemedlg.h"

static bool sSettingTheme = false;


//-----------------------------------------------------------------------------
Installer::Installer (QWidget *aParent, const char *aName, bool aInit)
  : InstallerInherited(aParent, aName)
{
  KButtonBox* bbox;

  mTheme = 0;
  mGui = !aInit;
  if (!mGui)
  {
    return;
  }

  mEditing = false;

  connect(this, SIGNAL(changed(Theme *)), SLOT(slotThemeChanged(Theme *)));

  mGrid = new QGridLayout(this, 2, 3, 6, 6);
  mThemesList = new QListBox(this);
  connect(mThemesList, SIGNAL(highlighted(int)), SLOT(slotSetTheme(int)));
  mGrid->addWidget(mThemesList, 0, 0);

  mPreview = new QLabel(this);
  mPreview->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  mGrid->addWidget(mPreview, 0, 1);

  bbox = new KButtonBox(this, KButtonBox::VERTICAL, 0, 6);
  mGrid->addMultiCellWidget(bbox, 0, 1, 2, 2);

  mBtnAdd = bbox->addButton(i18n("Add..."));
  QToolTip::add(mBtnAdd, i18n("Add a new theme to the list."));
  connect(mBtnAdd, SIGNAL(clicked()), SLOT(slotAdd()));

  mBtnSave = bbox->addButton(i18n("Save"));
  QToolTip::add(mBtnSave, i18n("Save the Work directory"));
  connect(mBtnSave, SIGNAL(clicked()), SLOT(slotSave()));

  mBtnSaveAs = bbox->addButton(i18n("Save as..."));
  QToolTip::add(mBtnSaveAs, i18n("Save the Work directory as..."));
  connect(mBtnSaveAs, SIGNAL(clicked()), SLOT(slotSaveAs()));

  mBtnExtract = bbox->addButton(i18n("Create..."));
  QToolTip::add(mBtnExtract, i18n("Create a new theme from the current settings."));
  connect(mBtnExtract, SIGNAL(clicked()), SLOT(slotExtract()));

  mBtnRemove = bbox->addButton(i18n("Remove"));
  QToolTip::add(mBtnRemove, i18n("Remove the selected theme."));
  connect(mBtnRemove, SIGNAL(clicked()), SLOT(slotRemove()));

  mText = new QMultiLineEdit(this);
  mGrid->addMultiCellWidget(mText, 1,1, 0, 1);

  mGrid->setColStretch(0, 1);
  mGrid->setColStretch(1, 3);
  mGrid->setColStretch(2, 0);
  mGrid->setRowStretch(0, 3);
  mGrid->setRowStretch(1, 1);
  mGrid->activate();
  bbox->layout();
}


//-----------------------------------------------------------------------------
Installer::~Installer()
{
}

static bool pathToThemeName(QString &name)
{
    if (name.right(7) == ".tar.gz")
        name = name.left(name.length()-7);
    else if (name.right(4) == ".tgz")
        name = name.left(name.length()-4);
    else
        return false;
    return true;
}

static int findItem(QListBox *list, const QString aText)
{
  int id = list->count()-1;

  while (id >= 0)
  {
    if (list->text(id) == aText) return id;
    id--;
  }

  return -1;
}

static void removeFromList(QListBox *list, QString sName)
{
  QString name = sName.stripWhiteSpace();
  int it = findItem(list, (const char *) name);
  if (it >= 0) list->removeItem(it);
  it = findItem(list, (const char *) (name+" "));
  if (it >= 0) list->removeItem(it);
}

static void addToList(QListBox *list, QString name)
{
  sSettingTheme = true;
  // WABA: Horrible Hack to keep the "Default" theme on top.
  if ((name == "Default ") || (name == "Default"))
  {
    bool update = list->autoUpdate();
    list->setAutoUpdate( false );
    list->removeItem(0);
    list->insertItem(name, 0);
    list->setAutoUpdate( update );
    list->setCurrentItem(findItem(list, name)); 
    if ( update )
       list->repaint();
    sSettingTheme = false;
    return;
  }
  QString sDefault = list->text(0);
  bool update = list->autoUpdate();
  list->setAutoUpdate( false );
  list->removeItem(0);
  removeFromList(list, name);
  list->inSort(name);
  list->insertItem(sDefault, 0);
  list->setAutoUpdate( update );
  list->setCurrentItem(findItem(list, name)); 
  if ( update )
     list->repaint();
  sSettingTheme = false;
}

//-----------------------------------------------------------------------------
void Installer::readThemesList(void)
{
  QDir d(Theme::globalThemesDir(), 0, QDir::Name, QDir::Files|QDir::Dirs);
  QStrList* entryList;
  QString name;
  QString sDefault = "Default ";

  if (mTheme) {
    delete mTheme;
    mTheme = 0;
  }

//  d.setNameFilter("*.tar.gz");
  mThemesList->clear();

  // Read global themes
//  d.setPath(Theme::globalThemesDir());
  entryList = (QStrList*)d.entryList();
  if (entryList) for(name=entryList->first(); name; name=entryList->next())
  {
    if (name[0]=='.') continue;
    if (name=="CVS" || 
        name.right(8)==".themerc")
      continue;
    if (!pathToThemeName(name))
    {
      QString path = Theme::globalThemesDir()+"/"+name+"/"+name+".themerc";
      if (!d.exists(path))
         continue;
    }
    if (name == "Default")
      continue;
    // Dirty hack: the trailing space marks global themes ;-)
    removeFromList(mThemesList, name);
    mThemesList->inSort(name + " ");
  }

  // Read local themes
  d.setPath(Theme::themesDir());
  entryList = (QStrList*)d.entryList();
  if (entryList) for(name=entryList->first(); name; name=entryList->next())
  {
    if (name[0]=='.') continue;
    if (name=="CVS" || 
        name.right(8)==".themerc")
      continue;
    if (!pathToThemeName(name))
    {
      QString path = Theme::themesDir()+"/"+name+"/"+name+".themerc";
      if (!d.exists(path))
         continue;
    }
    if (name == "Default")
    {
      sDefault = name;
      continue;
    }
    removeFromList(mThemesList, name);
    mThemesList->inSort(name);
  }

  mThemesList->insertItem(sDefault, 0);
  mThemesList->setCurrentItem(0);
  slotSetTheme(0);
}


//-----------------------------------------------------------------------------
void Installer::loadSettings()
{
  debug("Installer::loadSettings() called");
}


//-----------------------------------------------------------------------------
void Installer::applySettings()
{
  debug("Installer::applySettings() called");
  if (!mTheme) return;
  mTheme->install();
}

void Installer::defaultSettings()
{
  mThemesList->setCurrentItem(0);
  slotSetTheme(0);
}


//-----------------------------------------------------------------------------
void Installer::slotExtract()
{
  ThemeCreator theme;

  NewThemeDlg dlg;

  if (!dlg.exec()) return;

  QString desc = dlg.description();
  theme.setDescription((const char *) desc);

  QString name = dlg.fileName();

  theme.setVersion("0.1");
  theme.setAuthor(dlg.author());
  theme.setEmail(dlg.email());
  theme.setHomepage(dlg.homepage());

  dlg.hide();

  if (!theme.create(name)) return;

  mEditing = true;

  mPreview->setText("");
  mText->setText("");
  theme.extract();
  theme.save(Theme::themesDir()+name+".tar.gz");
  addToList(mThemesList, name);
  slotSetTheme(mThemesList->currentItem());
  KMsgBox::message(this, kapp->name(), i18n("Theme created."),
	QMessageBox::Default | QMessageBox::Information );
}


//-----------------------------------------------------------------------------
void Installer::slotRemove()
{
  int cur = mThemesList->currentItem();
  QString cmd, themeFile;
  int rc;
  QFileInfo finfo;

  if (cur < 0) return;
  QString sName = mThemesList->text(cur);
  themeFile = Theme::themesDir() + sName;
  finfo.setFile(themeFile+".tar.gz");
  if (finfo.exists())
     themeFile += ".tar.gz";
  else {
     finfo.setFile(themeFile+".tgz");
     if (finfo.exists())
       themeFile += ".tgz";
     else {
       finfo.setFile(themeFile);
       if (!finfo.exists())
       {
           themeFile = 0;
           KMsgBox::message(this, kapp->name(), 
	     i18n("The "),
	     QMessageBox::Default | QMessageBox::Warning );
       }
     }
  }

  if (themeFile)
  {
    cmd.sprintf("rm -rf \"%s\"", (const char*)themeFile);
    rc = system(cmd);
    finfo.setFile(themeFile);
    if (rc || finfo.exists())
    {
      if (rc)
        warning(i18n("Failed to remove theme:\n%s\n(%s)"), 
               (const char*)themeFile, strerror(errno));
      else
        warning(i18n("Failed to remove theme %s"), (const char*)themeFile);
      return;
    }
  }
  else 
  {
    KMsgBox::message(this, kapp->name(), 
	     i18n("The file(s) of this theme could not be found!"),
	     QMessageBox::Default | QMessageBox::Warning );
  }
  // We have removed a local theme... Did it hide a global theme??
  bool bGlobalTheme = true;
  QFileInfo fi;
  fi.setFile(Theme::globalThemesDir()+sName+".tgz");
  if (!fi.exists())
  {
     fi.setFile(Theme::globalThemesDir()+sName+".tar.gz");
     if (!fi.exists())
     {
        fi.setFile(Theme::globalThemesDir()+sName);
        if (!fi.exists())
	   bGlobalTheme = false;
     }
  }
  if (bGlobalTheme)
  {
    addToList(mThemesList, sName+" "); // Add global theme to list
  }
  else
  {
    sSettingTheme = true;
    mThemesList->removeItem(cur);
    if (cur >= (int)mThemesList->count()) 
     cur = (int) mThemesList->count()-1;
    mThemesList->setCurrentItem(cur);
    sSettingTheme = false;
  }
  slotSetTheme(cur);
  if (bGlobalTheme)
     KMsgBox::message(this, kapp->name(), i18n("Theme removed locally.\n"
"Now using global version."),
	QMessageBox::Default | QMessageBox::Information );
  else
     KMsgBox::message(this, kapp->name(), i18n("Theme removed."),
	QMessageBox::Default | QMessageBox::Information );
}

//-----------------------------------------------------------------------------
void Installer::slotSetTheme(int id)
{
  bool enabled, isGlobal=false;

  if (sSettingTheme) return;

  if (mTheme) {
    delete mTheme;
    mTheme = 0;
    emit changed(mTheme);
  }

  if (id < 0)
  {
    mPreview->setText("");
    mText->setText("");
    enabled = false;
  }
  else
  {
    QString name = mThemesList->text(id);
    if (name.isEmpty()) return;

    isGlobal = (name[name.length()-1]==' ');
    QString path;
    if (isGlobal) path = Theme::globalThemesDir() + name.stripWhiteSpace();
    else path = Theme::themesDir() + name;

    mTheme = new Theme();
    enabled = mTheme->load(path+".tgz", name);
    if (!enabled)
    {
      enabled = mTheme->load(path+".tar.gz", name);
      if (!enabled)
      {
        enabled = mTheme->load(path, name);
        if (!enabled)
        {
          mPreview->setText(i18n("(no theme chosen)"));
          mText->setText("");
          delete mTheme;
          mTheme = 0;
        }
      }
    }
  }

  mBtnSave->setEnabled(enabled);
  mBtnSaveAs->setEnabled(enabled);
  mBtnRemove->setEnabled(enabled && !isGlobal);
  emit changed(mTheme);
}


//-----------------------------------------------------------------------------
void Installer::slotAdd()
{
  QString fname, fpath, cmd, theme;
  int i, rc;
  static QString path;
  if (path.isEmpty()) path = QDir::homeDirPath();

  KFileDialog dlg(path, "*gz", 0, 0, true, false);
  dlg.setCaption(i18n("Add Theme"));
  if (!dlg.exec()) return;

  path = dlg.dirPath();
  fpath = dlg.selectedFile();
  i = fpath.findRev('/');
  if (i >= 0) theme = fpath.mid(i+1, 1024);
  else theme = fpath;
  if (!pathToThemeName(theme))
  {
    warning(i18n("Wrong file.\n"
                 "The name of a theme file should end\n"
                 "either with '.tar.gz' or '.tgz'\n"));
    return;
  } 

  // Copy theme package into themes directory
  cmd.sprintf("cp %s %s", (const char*)fpath, 
	      (const char*)Theme::themesDir());
  rc = system(cmd);
  if (rc)
  {
    warning(i18n("Failed to copy theme %s\ninto themes directory %s"),
	    (const char*)fpath, (const char*)Theme::themesDir());
    return;
  }
 
  addToList(mThemesList, theme);
  slotSetTheme(mThemesList->currentItem());
  KMsgBox::message(this, kapp->name(), i18n("Theme added."),
	QMessageBox::Default | QMessageBox::Information );
}


//-----------------------------------------------------------------------------
void Installer::slotSaveAs()
{
  QString fname, fpath, cmd, themeFile, ext;
  bool isGlobal = false;
  static QString path;
  QFileInfo finfo;
  int cur;

  if (!mTheme) return;

  if (path.isEmpty()) path = QDir::homeDirPath();

  cur = mThemesList->currentItem();
  if (cur < 0) return;

  themeFile = mThemesList->text(cur);
  themeFile.detach();
  if (themeFile.isEmpty()) return;

  isGlobal = (themeFile[themeFile.length()-1]==' ');
  if (isGlobal) themeFile = themeFile.stripWhiteSpace();
  themeFile += ".tar.gz";
 
  ext = "*.tar.gz";

  KFileDialog dlg(path, ext, 0, 0, true, false);
  dlg.setCaption(i18n("Save Theme"));
  dlg.setSelection(themeFile);
  if (!dlg.exec()) return;

  path = dlg.dirPath();
  fpath = dlg.selectedFile();

  mTheme->save(fpath);
  KMsgBox::message(this, kapp->name(), i18n("Theme saved."),
	QMessageBox::Default | QMessageBox::Information );

}

//-----------------------------------------------------------------------------
void Installer::slotSave()
{
  QString name;
  bool isGlobal = false;
  int cur;

  if (!mTheme) return;

  cur = mThemesList->currentItem();
  if (cur < 0) return;

  name = mThemesList->text(cur);
  name.detach();
  if (name.isEmpty()) return;

  isGlobal = (name[name.length()-1]==' ');
  if (isGlobal) name = name.stripWhiteSpace();
 
  mTheme->save(Theme::themesDir()+name+".tar.gz");
  addToList(mThemesList, name);
  mBtnRemove->setEnabled(true);
  KMsgBox::message(this, kapp->name(), i18n("Theme saved."),
	QMessageBox::Default | QMessageBox::Information );
}


//-----------------------------------------------------------------------------
void Installer::slotThemeChanged(Theme *theme)
{
  if (!theme)
  {
     mText->setText("");
     mBtnSaveAs->setEnabled(false);
     mPreview->setText("");
  }
  else
  {
     mText->setText(theme->description()); 
     mBtnSaveAs->setEnabled(true);

     if (theme->preview().isNull())
        mPreview->setText(i18n("(no preview pixmap)"));
     else 
        mPreview->setPixmap(theme->preview());
     //mPreview->setFixedSize(theme->preview().size());
  }
}




//-----------------------------------------------------------------------------
#include "installer.moc"
