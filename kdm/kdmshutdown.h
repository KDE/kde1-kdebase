//                              -*- Mode: C++ -*- 
// Title            : kdmshutdown.h
// 
// Description      : Shutdown dialog. Class KDMShutdown
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:51:38 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Fri Jul 11 14:16:17 1997
// Update Count     : 6
// Status           : Unknown, Use with caution!
// 

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
