//                              -*- Mode: C++ -*- 
// Title            : kfdialog.cpp
// 
// Description      : Dialog class to handle input focus -- see headerfile
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:51:03 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Thu Nov 20 12:49:23 1997
// Update Count     : 7
// Status           : Unknown, Use with caution!
// 

#include "kfdialog.h"
#include <X11/Xlib.h>

int
FDialog::exec()
{
     setResult(0);
     show();
     // Give focus back to parent:
     if( isModal() && parentWidget() != 0)
	  parentWidget()->setActiveWindow();
     return result();
}

#include "kfdialog.moc"
