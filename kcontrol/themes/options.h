/*
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org>
 */
#ifndef OPTIONS_H
#define OPTIONS_H

#include <kcontrol.h>

class QLabel;
class QCheckBox;
class QComboBox;
class QPushButton;
class QBoxLayout;
class QGridLayout;
class Theme;

#define OptionsInherited KConfigWidget
class Options : public KConfigWidget
{
  Q_OBJECT
public:
  Options(QWidget *parent=0, const char* name=0, bool init=FALSE);
  ~Options();

  virtual void loadSettings();
  virtual void applySettings();

  /** Update status information on available groups of current theme. */
  virtual void updateStatus(void);

public slots:
  void slotThemeChanged(Theme *theme);

protected slots:
  virtual void slotThemeApply();
  virtual void slotCbxClicked();
  virtual void slotDetails();
  virtual void slotInvert();
  virtual void slotClear();

protected:
  /** Creates a new options line */
  virtual QCheckBox* newLine(const char* groupName, const char* text,
			     QLabel** statusPtr);

  virtual void readConfig();
  virtual void writeConfig();

  virtual void updateStatus(const char* groupName, QLabel* status);

protected:
  QCheckBox *mCbxPanel, *mCbxIcons, *mCbxColors, *mCbxWindowBorder;
  QCheckBox *mCbxWindowTitlebar, *mCbxWallpapers, *mCbxSounds;
  QCheckBox *mCbxWindowButtonLayout, *mCbxGimmick, *mCbxKfm;
  QCheckBox *mCbxOverwrite, *mCbxKonsole;
  QLabel *mStatPanel, *mStatIcons, *mStatColors, *mStatWindowBorder;
  QLabel *mStatWindowTitlebar, *mStatWallpapers, *mStatSounds;
  QLabel *mStatWindowButtonLayout, *mStatGimmick, *mStatKfm, *mStatKonsole;
  QGridLayout *mGrid;
  bool mGui;
  int mGridRow;
  Theme *mTheme;
};

#endif /*OPTIONS_H*/

