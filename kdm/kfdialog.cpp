//                              -*- Mode: C++ -*- 
// Title            : kfdialog.cpp
// 
// Description      : Dialog class to handle input focus -- see headerfile
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:51:03 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Thu Jul 10 03:15:58 1997
// Update Count     : 4
// Status           : Unknown, Use with caution!
// 

#include "kfdialog.h"
#include <X11/Xlib.h>

FocusFilter::FocusFilter( QObject* _parent ) 
{ 
     parent = _parent;
     parent->installEventFilter( this);
     connect( this, SIGNAL(sig_show()), parent, SLOT( take_focus()));
}

FocusFilter::~FocusFilter() 
{ 
     parent->removeEventFilter( this);
}

bool 
FocusFilter::eventFilter( QObject *, QEvent *e)
{ 
     if( e->type() == Event_Show) {
	  emit( sig_show());
     }
     return FALSE;
}


FDialog::FDialog( QWidget *parent, const char* name, 
		  bool modal, WFlags f) 
     : QDialog( parent, name, modal, f) 
{ 
     ff = new FocusFilter( this);
}

int
FDialog::exec()
{
     setResult(0);
     show();
     if( isModal() && parentWidget() != 0)
	  parentWidget()->setActiveWindow();
     return result();
}

FDialog::~FDialog() 
{ 
     delete ff;
}

void
FDialog::take_focus()
{ 
     //setActiveWindow();
     QWidget *tlw = topLevelWidget();
     if ( tlw->isVisible() )
	  XSetInputFocus( qt_xdisplay(), tlw->winId(), 
			  RevertToPointerRoot, CurrentTime);
}

#include "kfdialog.moc"
