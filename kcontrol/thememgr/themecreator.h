/*
 * themecreator.h
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
#ifndef THEMECREATOR_H
#define THEMECREATOR_H

#include "theme.h"
#include <qfile.h>


/** Class of themes that can create a theme package from the current
 * Kde settings.
 */
#define ThemeCreatorInherited Theme
class ThemeCreator: public Theme
{
  Q_OBJECT
public:
  /** Construct a theme-creator object */
  ThemeCreator();
  virtual ~ThemeCreator();

  /** Create new, empty, theme with given name. */
  virtual bool create(const QString themeName);

  /** Extract theme groups from current Kde settings. */
  virtual bool extract(void);

protected:
  /** Extract items for given group. Returns number of extracted files. */
  virtual int extractGroup(const char* groupName);

  /** Extract icons. */
  virtual void extractIcons(void);

  /** Extract given file. Returns name of file in theme package.
      The file is renamed if there is already a file with the same
      name in the theme package. Returns NULL if aFile does not exist or
      is no file. */
  virtual const QString extractFile(const QString& aFile);

  /** Set information of "General" group. */
  virtual void setGroupGeneral(void);

  /** Do some special things */
  virtual void extractCmd(KSimpleConfig* aCfg, const QString& aCmd,
			  int aInstalled);
};

#endif /*THEMECREATOR_H*/

