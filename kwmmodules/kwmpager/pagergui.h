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

#ifndef pagergui_h
#define pagergui_h

#include <qframe.h>

class PagerGUI: public QObject
{
public:
    PagerGUI *getStyle(int _id);

protected:
    int id;
};


class PagerGUI1 : public PagerGUI, public QWidget
{
public:
    PagerGUI1();
};

class PagerGUI2 : public PagerGUI, public QFrame
{
public:
    PagerGUI2();
};

#endif


