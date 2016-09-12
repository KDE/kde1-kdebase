    /*

    $Id: kdmusers.h,v 1.2 1998/09/04 15:33:01 bieker Exp $

    Copyright (C) 1997, 1998 Steffen Hansen
                             stefh@mip.ou.dk


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */
 

# include "kdm-config.h"

#include "kdmview.h"

class KDMUserItem : public KDMViewItem {
public:
     setName( const char* name) { user_name = QString( name);}
     const char* name() { return user_name.data()} cont;
private:
     QString user_name;
};
