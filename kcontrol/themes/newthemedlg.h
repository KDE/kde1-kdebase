/*
 * newthemedlg.h
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
#ifndef NEW_THEME_DLG_H
#define NEW_THEME_DLG_H

#include <qdialog.h>
#include <qlineedit.h>
#include <qmultilinedit.h>

class QLabel;
class QGridLayout;

#define NewThemeDlgInherited QDialog
class NewThemeDlg: public QDialog
{
public:
  NewThemeDlg();
  virtual ~NewThemeDlg();

  const char* fileName(void) const { return mEdtFilename->text(); }
  const char* description(void) const { return mEdtDesc->text(); }
  const char* author(void) const { return mEdtAuthor->text(); }
  const char* email(void) const { return mEdtEmail->text(); }
  const char* homepage(void) const { return mEdtHomepage->text(); }

protected:
  virtual QLineEdit* newLine(const char* lbl);
  virtual QMultiLineEdit* newMultiLine(const char* lbl);
  virtual void setValues(void);

protected:
  int mGridRow;
  QLineEdit *mEdtFilename; 
  QLineEdit *mEdtAuthor;
  QLineEdit *mEdtEmail;
  QLineEdit *mEdtHomepage;
  QMultiLineEdit *mEdtDesc;
  QGridLayout* mGrid;
};

#endif /*NEW_THEME_DLG_H*/
