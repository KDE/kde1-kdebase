//                              -*- Mode: C++ -*- 
// Title            : kgreeter.h
// 
// Description      : Greeter module for xdm. Class KGreeter.
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:48:08 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Sun Aug 16 19:17:31 1998
// Update Count     : 14
// Status           : Unknown, Use with caution!
// 

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

//class KGreeter : public QDialog {
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
     bool restrict_nologin();
     bool restrict_expired();
     bool restrict_nohome();
     void load_wm();
     void save_wm();
protected:
     void timerEvent( QTimerEvent * ) {};
private:
     QTimer*        timer;
     KDMView*       user_view;
     QLabel*        pixLabel;
     QLabel*        loginLabel;
     QLabel*        passwdLabel;
     QLabel*        failedLabel;
     QLineEdit*     loginEdit;
     QLineEdit*     passwdEdit; 
     QFrame*        separator;
     QPushButton*   goButton;
     QPushButton*   cancelButton;
     QPushButton*   shutdownButton;
     QComboBox*     sessionargBox;
};
#endif /* KGREETER_H */

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */
