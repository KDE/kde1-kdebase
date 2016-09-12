    /*

    Shutdown dialog. Class KDMShutdown
    $Id: kdmshutdown.h,v 1.3 1998/09/04 15:33:00 bieker Exp $

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
 

#ifndef KDMSHUTDOWN_H
#define KDMSHUTDOWN_H

#include "kdm-config.h"

#define QT_CLEAN_NAMESPACE
#include <qglobal.h>

#include <X11/Xmd.h>
typedef unsigned char   UINT8;                  // 8 bit unsigned
typedef unsigned short  UINT16;                 // 16 bit unsigned
typedef unsigned int    UINT32;                 // 32 bit unsigned

#include <stdlib.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qpushbt.h>
#include <qlined.h>
#include "kfdialog.h"

class KDMShutdown : public FDialog {
     Q_OBJECT
public:
     KDMShutdown( int mode, QWidget* _parent=0, const char* _name=0,
		  const char* _shutdown = "/sbin/halt", 
		  const char* _restart  = "/sbin/reboot");
private slots:
     void rb_clicked(int);
     void pw_entered();
     void bye_bye();
private:
     QLabel*       label;
     QButtonGroup* btGroup;
     QPushButton*  okButton;
     QPushButton*  cancelButton;
     QLineEdit*    pswdEdit;
     const char*   cur_action;
     const char*   shutdown;
     const char*   restart;
};

#endif /* KDMSHUTDOWN_H */
