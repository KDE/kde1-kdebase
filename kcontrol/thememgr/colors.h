/*
 * colors.h
 *
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org> and
 *                    Roberto Alsina <ralsina@unl.edu.ar>
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
#ifndef COLORS_H
#define COLORS_H

#include <kcontrol.h>

class QGridLayout;
class QListBox;
class QPushButton;
class QLabel;
class WidgetCanvas;
class QComboBox;
class KColorButton;


#define ColorsInherited KConfigWidget
class Colors : public KConfigWidget
{
  Q_OBJECT
public:
  Colors(QWidget *parent=0, const char* name=0, bool init=FALSE);
  ~Colors();

  virtual void loadSettings();
  virtual void applySettings();

protected slots:
  virtual void slotThemeChanged();
  virtual void slotThemeApply();
  virtual void slotWidgetColor(int);
  virtual void slotSelectColor(const QColor &);
  // virtual void slotColorSelected(const QColor &);

protected:

private:
  bool mGui;
  QGridLayout *mGrid;
  WidgetCanvas *mCanvas;
  KColorButton *mBtnColor;
  QComboBox* mCbxColorNames;
  int mColorPushColor;
};

#endif /*COLORS_H*/

