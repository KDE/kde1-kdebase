/*
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org>
 */
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbt.h>
#include <qchkbox.h>
#include <qlistbox.h>
#include <qcombo.h>
#include <kapp.h>
#include <kconfig.h>

#include "options.h"
#include "theme.h"
#include "global.h"
#include "groupdetails.h"

#include <assert.h>


//-----------------------------------------------------------------------------
Options::Options (QWidget * aParent, const char *aName, bool aInit)
  : OptionsInherited(aParent, aName)
{
  QLabel* lbl;
  QPushButton* btn;

  mGui = !aInit;
  if (!mGui)
  {
    return;
  }
  connect(theme, SIGNAL(changed()), SLOT(slotThemeChanged()));
  connect(theme, SIGNAL(apply()), SLOT(slotThemeApply()));

  mGrid = new QGridLayout(this, 11, 5, 10, 6);
  mGridRow = 0;

  lbl = new QLabel(i18n("Work on the following parts:"), this);
  lbl->setMinimumSize(lbl->sizeHint());
  mGrid->addMultiCellWidget(lbl, 0, 0, 0, 4);
  mGrid->setRowStretch(0, 3);
  mGridRow++;

  // The name of the entries and the name of the config groups must
  // be exactly the same. Otherwise the code in slotDetails()
  // will not be able to determine the translation for the
  // details dialog.
  mCbxColors = newLine("Colors", i18n("Colors"), &mStatColors);
  mCbxWindowBorder = newLine("Window Border", i18n("Window Border"), 
			     &mStatWindowBorder);
  mCbxWindowTitlebar = newLine("Window Titlebar", i18n("Window Titlebar"), 
			       &mStatWindowTitlebar);
  mCbxWallpapers = newLine("Wallpapers", i18n("Wallpapers"), &mStatWallpapers);
  mCbxPanel = newLine("Panel", i18n("Panel"), &mStatPanel);
  mCbxSounds = newLine("Sounds", i18n("Sounds"), &mStatSounds);
  mCbxIcons = newLine("Icons", i18n("Icons"), &mStatIcons);
  mCbxCleanupIcons = newLine("Cleanup Icons", i18n("Revert old icons"), 
			     &mStatCleanupIcons);
  mStatCleanupIcons->hide();

  btn = new QPushButton(i18n("Invert"), this);
  btn->setFixedSize(btn->sizeHint());
  connect(btn, SIGNAL(pressed()), SLOT(slotInvert()));
  mGrid->addWidget(btn, mGridRow++, 0);

  mGrid->setRowStretch(mGridRow, 1000);
  mGrid->setColStretch(0, 2);
  mGrid->setColStretch(1, 1);
  mGrid->setColStretch(2, 1);
  mGrid->setColStretch(3, 10);
  mGrid->activate();

  readConfig();
}


//-----------------------------------------------------------------------------
Options::~Options()
{
  writeConfig();
}


//-----------------------------------------------------------------------------
QCheckBox* Options::newLine(const char* aGroupName, const char* aText,
			    QLabel** aStatusPtr) 
{
  QCheckBox* cbx = new QCheckBox(aText, this);
  QPushButton* btnDetails;
  QLabel* lbl;

  cbx->setMinimumSize(cbx->sizeHint());
  cbx->setMaximumSize(32767, cbx->sizeHint().height()+5);
  connect(cbx, SIGNAL(clicked()), this, SLOT(slotCbxClicked()));
  mGrid->addMultiCellWidget(cbx, mGridRow, mGridRow, 0, 1);

  lbl = new QLabel(i18n("unknown"), this);
  lbl->setMinimumSize(lbl->sizeHint());
  lbl->setMaximumSize(32767, lbl->sizeHint().height()+5);
  mGrid->addWidget(lbl, mGridRow, 2);
  *aStatusPtr = lbl;

  btnDetails = new QPushButton("...", this, aGroupName);
  btnDetails->setFixedSize(btnDetails->sizeHint() - QSize(6,2));
  connect(btnDetails, SIGNAL(clicked()), this, SLOT(slotDetails()));
  mGrid->addWidget(btnDetails, mGridRow, 3);
  btnDetails->hide();

  mGridRow++;
  return cbx;
}


//-----------------------------------------------------------------------------
void Options::loadSettings()
{
  debug("Options::loadSettings() called");
}


//-----------------------------------------------------------------------------
void Options::applySettings()
{
  theme->instColors = mCbxColors->isChecked();
  theme->instWindowBorder = mCbxWindowBorder->isChecked();
  theme->instWindowTitlebar = mCbxWindowTitlebar->isChecked();
  theme->instWallpapers = mCbxWallpapers->isChecked();
  theme->instPanel = mCbxPanel->isChecked();
  theme->instSounds = mCbxSounds->isChecked();
  theme->instIcons = mCbxIcons->isChecked();
  theme->instCleanupIcons = mCbxCleanupIcons->isChecked();
}


//-----------------------------------------------------------------------------
void Options::slotInvert()
{
  mCbxColors->setChecked(!mCbxColors->isChecked());
  mCbxWindowBorder->setChecked(!mCbxWindowBorder->isChecked());
  mCbxWindowTitlebar->setChecked(!mCbxWindowTitlebar->isChecked());
  mCbxWallpapers->setChecked(!mCbxWallpapers->isChecked());
  mCbxPanel->setChecked(!mCbxPanel->isChecked());
  mCbxSounds->setChecked(!mCbxSounds->isChecked());
  mCbxIcons->setChecked(!mCbxIcons->isChecked());
  mCbxCleanupIcons->setChecked(!mCbxCleanupIcons->isChecked());
}


//-----------------------------------------------------------------------------
void Options::slotDetails()
{
  QString groupName = sender()->name();
  GroupDetails dlg(groupName);

  if (groupName.isEmpty())
  {
    warning("Empty group name ?!");
    return;
  }

  dlg.setCaption(i18n(groupName));
  dlg.exec();
}


//-----------------------------------------------------------------------------
void Options::slotCbxClicked()
{
}


//-----------------------------------------------------------------------------
void Options::slotThemeApply()
{
  applySettings();
}


//-----------------------------------------------------------------------------
void Options::slotThemeChanged()
{
  debug("Options::slotThemeChanged() called");
  updateStatus();
}


//-----------------------------------------------------------------------------
void Options::updateStatus(const char* aGroupName, QLabel* aLblStatus)
{
  const char* statusStr;

  assert(aGroupName!=0);
  assert(aLblStatus!=NULL);

  if (theme->hasGroup(aGroupName, true))
    statusStr = i18n("available");
  else statusStr = i18n("empty");

  aLblStatus->setText(statusStr);
  aLblStatus->setMinimumSize(aLblStatus->sizeHint());
}


//-----------------------------------------------------------------------------
void Options::updateStatus(void)
{
  updateStatus("Colors", mStatColors);
  updateStatus("Window Border", mStatWindowBorder);
  updateStatus("Window Titlebar", mStatWindowTitlebar);
  updateStatus("Wallpapers", mStatWallpapers);
  updateStatus("Panel", mStatPanel);
  updateStatus("Sounds", mStatSounds);
  updateStatus("Icons", mStatIcons);
}


//-----------------------------------------------------------------------------
void Options::writeConfig()
{
  KConfig* cfg = kapp->getConfig();

  cfg->setGroup("Options");
  cfg->writeEntry("panel", mCbxPanel->isChecked());
  cfg->writeEntry("icons", mCbxIcons->isChecked());
  cfg->writeEntry("cleanup-icons", mCbxCleanupIcons->isChecked());
  cfg->writeEntry("colors", mCbxColors->isChecked());
  cfg->writeEntry("window-border", mCbxWindowBorder->isChecked());
  cfg->writeEntry("window-titlebar", mCbxWindowTitlebar->isChecked());
  cfg->writeEntry("wallpapers", mCbxWallpapers->isChecked());
  cfg->writeEntry("sounds", mCbxSounds->isChecked());
}


//-----------------------------------------------------------------------------
void Options::readConfig()
{
  KConfig* cfg = kapp->getConfig();

  cfg->setGroup("Options");
  mCbxPanel->setChecked(cfg->readBoolEntry("panel", true));
  mCbxIcons->setChecked(cfg->readBoolEntry("icons", true));
  mCbxCleanupIcons->setChecked(cfg->readBoolEntry("cleanup-icons", true));
  mCbxColors->setChecked(cfg->readBoolEntry("colors", true));
  mCbxWindowBorder->setChecked(cfg->readBoolEntry("window-border", true));
  mCbxWindowTitlebar->setChecked(cfg->readBoolEntry("window-titlebar", true));
  mCbxWallpapers->setChecked(cfg->readBoolEntry("wallpapers", true));
  mCbxSounds->setChecked(cfg->readBoolEntry("sounds", true));
}

//-----------------------------------------------------------------------------
#include "options.moc"
