//                              -*- Mode: C++ -*- 
// Title            : kfdialog.h
// 
// Description      : Dialog class that handles input focus in absence of a wm
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:49:49 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Thu Nov 20 12:48:22 1997
// Update Count     : 8
// Status           : Unknown, Use with caution!
// 

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
