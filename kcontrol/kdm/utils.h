/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#ifndef __KDMUTILS_H__
#define __KDMUTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <qdir.h>

#include <qstring.h>
#include <qpushbt.h>
#include <qtooltip.h>
#include <qfileinf.h>
#include <qlabel.h>
#include <qgrpbox.h>
#include <qlayout.h>

#include <kapp.h>
#include <kcontrol.h>
#include <kiconloaderdialog.h>
#include <kmsgbox.h>
#include <ksimpleconfig.h>
#include <kimgio.h>

#define CONFIGFILE kapp->kde_configdir() + "/kdmrc"

#define PIXDIR kapp->kde_datadir()+"/kdm/pics/"
#define USERPIXDIR PIXDIR + "users/"


void semsplit( const QString& str, QStrList& result);


#endif


