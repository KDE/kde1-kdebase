/*
 *   kwmpager - a pager for kwm (by Matthias Ettrich)
 *   Copyright (C) 1997  Stephan Kulow
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */   

#include "pagergui.h"

PagerGUI1::PagerGUI1() : QWidget(NULL, "kwmpager")
{
    id = 1;
}

PagerGUI2::PagerGUI2() : QFrame(NULL, "kwmpager", WStyle_Customize | 
						  WStyle_NoBorder | 
						  WStyle_Tool) 
{
    id = 2;
}

PagerGUI *PagerGUI::getStyle(int _id)
{
    if (_id == 1)
	return new PagerGUI1();
    else
	return new PagerGUI2();
}
