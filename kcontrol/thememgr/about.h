/*
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org>
 */
#ifndef ABOUT_H
#define ABOUT_H

#include <kcontrol.h>

class QLabel;
class QCheckBox;
class QComboBox;
class QPushButton;
class QBoxLayout;
class Theme;

#define AboutInherited KConfigWidget
class About : public KConfigWidget
{
  Q_OBJECT
public:
  About(QWidget *parent=0, const char* name=0, bool init=FALSE);
  ~About();

  virtual void loadSettings();
  virtual void applySettings();

public slots:
  virtual void slotThemeChanged( Theme *theme);

protected:
  QLabel *lblTheme, *lblAuthor, *lblVersion, *lblHomepage;
};

#endif /*ABOUT_H*/

