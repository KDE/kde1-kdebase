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

#include "utils.h"


// Func. for splitting ';' sep. lists.
void semsplit( const QString& str, QStrList& result)
{
     int i1 = 0, i2 = 0;
     while( ( i2 = str.find( ';', i1)) != -1) {
	  result.append( str.mid(i1,i2-i1));
	  i1 = i2 + 1;
     }
     if( i1 != (int)str.length()) {
	  result.append(str.mid(i1,str.length()));
     }
}     



