// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//

#include <stdio.h>
#include <unistd.h>

#include "kfm.h"
#include "pmenu.h"

#include <qapp.h>
#include <qmsgbox.h>
#include <qdrawutl.h>
#include <qpmcache.h>
#include <qmenudta.h>
#include <qkeycode.h>
#include <qbitmap.h>


myPopupMenu::myPopupMenu( QWidget *parent, const char *name )
  : QPopupMenu( parent, name ){
    setMouseTracking(TRUE);
}

int myPopupMenu::height(){
//   if (badSize){
//     QPoint p = pos();
//     move(-1000,-1000);
//     show();
//     hide();
//     move(p);
//   }
  return QPopupMenu::height();
}
int myPopupMenu::width(){
//   if (badSize){
//     QPoint p = pos();
//     move(-1000,-1000);
//     show();
//     hide();
//     move(p);
//   }
  return QPopupMenu::width();
}


