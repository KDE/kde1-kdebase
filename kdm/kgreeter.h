    /*

    Greeter module for xdm
    $Id: kgreeter.h,v 1.12.4.1 1999/06/29 22:13:36 steffen Exp $

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
 

#ifndef KGREETER_H
#define KGREETER_H

# include "kdm-config.h"

#define QT_CLEAN_NAMESPACE
#include <qglobal.h>

#include <X11/Xmd.h>
typedef unsigned char   UINT8;                  // 8 bit unsigned
typedef unsigned short  UINT16;                 // 16 bit unsigned
typedef unsigned int    UINT32;                 // 32 bit unsigned

#define WMRC ".wmrc"

#include <qwidcoll.h>
#include <qregexp.h>
#include <qstrlist.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpushbt.h>
#include <qlined.h>
#include <qcombo.h>
#include <qlayout.h>
#include <qmsgbox.h>
#include <kapp.h>
#include <kconfig.h>

#include "kfdialog.h"
#include "kdmshutdown.h"
#include "kdmconfig.h"

class KLoginLineEdit : public QLineEdit {
     Q_OBJECT
public:
     KLoginLineEdit( QWidget *parent = 0) : QLineEdit(parent) {}
signals:
     void lost_focus();
protected:
     void focusOutEvent( QFocusEvent *e);
};

class KGreeter : public QWidget {
     Q_OBJECT
public:
     KGreeter(QWidget *parent, const char *t);
     void ReturnPressed();
     void SetTimer();
public slots:
     void go_button_clicked();
     void cancel_button_clicked();
     void shutdown_button_clicked();
     void timerDone();
     void slot_user_name( int i);
     bool restrict();
     bool restrict_nologin();
     bool restrict_expired();
     bool restrict_nohome();
     bool restrict_time();
     void load_wm();
     void save_wm();
protected:
     void timerEvent( QTimerEvent * ) {};
private:
     QStrList       sessiontags;
     QTimer*        timer;
     KDMView*       user_view;
     QLabel*        pixLabel;
     QLabel*        loginLabel;
     QLabel*        passwdLabel;
     QLabel*        failedLabel;
     KLoginLineEdit*     loginEdit;
     QLineEdit*     passwdEdit; 
     QFrame*        separator;
     QPushButton*   goButton;
     QPushButton*   cancelButton;
     QPushButton*   shutdownButton;
     QComboBox*     sessionargBox;

     struct passwd *pwd;

#ifdef HAVE_LOGIN_CAP_H
     struct login_cap *lc;
#endif

#if USESHADOW
     struct spwd *swd;
#endif
};
#endif /* KGREETER_H */

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */
