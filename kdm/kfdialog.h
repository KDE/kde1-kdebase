//                              -*- Mode: C++ -*- 
// Title            : kfdialog.h
// 
// Description      : Dialog class that handles input focus in absence of a wm
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:49:49 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Thu Jul 10 03:16:06 1997
// Update Count     : 4
// Status           : Unknown, Use with caution!
// 

#ifndef FDIALOG_H
#define FDIALOG_H

# include "kdm-config.h"

#include <qdialog.h>

class FDialog;

class FocusFilter : public QObject {
     Q_OBJECT
public:
     FocusFilter( QObject* _parent ); 
     ~FocusFilter();
     virtual bool eventFilter( QObject *qop, QEvent *e);
signals:
     void sig_show();
private:
     QObject *parent;
};

class FDialog : public QDialog {
     Q_OBJECT
public:
     FDialog( QWidget *parent = 0, const char* name = 0, 
	      bool modal = FALSE, WFlags f = 0);
     ~FDialog();
     int exec();
public slots:
     void take_focus();
protected:
     FocusFilter* ff;
};

#endif /* FDIALOG_H */
