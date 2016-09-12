    /*

    Dialog class that handles input focus in absence of a wm
    $Id: kfdialog.h,v 1.3 1998/09/04 15:33:03 bieker Exp $

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
 

#ifndef FDIALOG_H
#define FDIALOG_H

# include "kdm-config.h"

#include <qdialog.h>

class FDialog : public QDialog {
     Q_OBJECT
public:
     FDialog( QWidget *parent = 0, const char* name = 0, 
	      bool modal = FALSE, WFlags f = 0) 
       : QDialog( parent, name, modal, f) {}
     virtual int exec();
};

#endif /* FDIALOG_H */
