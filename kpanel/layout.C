// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//

#include "kpanel.h"
#include <qapp.h>
#include <qmsgbox.h>
#include <stdio.h>
#include <unistd.h>

void kPanel::layoutTaskbar(){
   myTaskButton* button;
   int n,w,d,d2,x,y;
   n = taskbar_buttons.count();
   int nr = numberOfTaskbarRows();
  
   if (taskbar_position == taskbar_top_left){
     y = 0;
     for (d=1; d <= number_of_desktops; d++){
       d2 = 0;
       for (button = taskbar_buttons.first(); button; button = taskbar_buttons.next()){
	 if (button->virtual_desktop == d){
	   if (d>1 && d2 == 0)
	     y += 2;
	   d2++;
	   button->setGeometry(0,y,taskbar_frame->width(), taskbar_height);
	   y += taskbar_height+1;
	   if (!button->isVisible())
	     button->show();
	 }
       }
     }
     taskbar_frame->resize(tbhs*taskbar_height, y);
     taskbar->resize(taskbar_frame->width(), taskbar_frame->height());
   }
   else {
     int r = 0;
     if (n>0) {
       w = (taskbar_frame->width()-((number_of_desktops-1)*4)) / 
	 ((n+((nr>1)?1:0))/nr);
       w--;
       if (w > tbhs*taskbar_height*3/2) w = tbhs*taskbar_height*3/2;
       x = 0;
       for (d=1; d <= number_of_desktops; d++){
	 d2 = 0;
	 for (button = taskbar_buttons.first(); button; button = taskbar_buttons.next()){
	   if (button->virtual_desktop == d){
	     if (d2 == 0 && d>1)
	       x += 4;
	     d2++;
	     button->setGeometry(x,taskbar_height * r,
				 w, taskbar_height);
	     n--;
	     x += w+1;
	     if (r<nr-1 && x > taskbar_frame->width() - (w+1)){
	       r++;
	       x = 0;
	     }
	     // aehem....
	     if (r < nr-1 && n == 1)
	       r++;
	     if (!button->isVisible())
	       button->show();
	   }
	 }
       }
     }
   }
}


int kPanel::numberOfTaskbarRows(){
  if (taskbar_position == taskbar_top_left)
    return 1;
  int n = taskbar_buttons.count();
  if (n == 0)
    return 1;
  if (taskbar_frame->width()-(number_of_desktops*4) < tbmhs*taskbar_height)
    return 1;

  int w = 0;
  int res = 0;
  do {
    res ++;
    w = (taskbar_frame->width()-(number_of_desktops*4)) / ((n+1)/res);
  } while (w < tbmhs*taskbar_height);
  return res;
}

void kPanel::reposition(int l){
  int i,i2,d;

  // brain dead. Don't ask me why I don't use a list, please....

  DesktopEntry tmp;

  /*

    i = 1;
    while ( i < nbuttons ) {
    
    if ( (orientation == vertical && entries[i-1].button->y() > entries[i].button->y()) ||
    (orientation == horizontal && entries[i-1].button->x() > entries[i].button->x())) {
    
    tmp = entries[i-1];
    entries[i-1] = entries[i];
    entries[i] = tmp;
    }
    i++;
    }
    */
  bool changed;
  do {
    changed = false;
    for (i = 0; i < (nbuttons - 1); i++) {

      if ( (orientation == vertical && entries[i].button->y() > entries[i+1].button->y()) ||
	   (orientation == horizontal && entries[i].button->x() > entries[i+1].button->x())) {
	tmp = entries[i];
	entries[i] = entries[i+1];
	entries[i+1] = tmp;
	changed = true;
      }
    }
  } while (changed); 

  // the first button is always the KDE button
//   if (orientation == vertical){
//     if (nbuttons > 1 && entries[0].button->y() > entries[1].button->y())
//       entries[0].button->move(entries[0].button->x(), 
// 			      entries[1].button->y()-entries[0].button->height());
//     if (entries[0].button->y()<panel_button->y() + panel_button->height()) 
// 	  entries[0].button->move(entries[0].button->x(),
// 				  panel_button->y() + panel_button->height());
//   }
//   else{
//     if (nbuttons > 1 && entries[0].button->x() > entries[1].button->x())
//       entries[0].button->move(entries[1].button->x() - entries[0].button->width(), 
// 			      entries[0].button->y()); 
//     if (entries[0].button->x()<panel_button->x() + panel_button->width()) 
// 	  entries[0].button->move(panel_button->x() + panel_button->width(),
// 				  entries[0].button->y());
//   }

  if (orientation == vertical){
    for (i=0; i<nbuttons-1; i++){
      d = entries[i].button->y() + entries[i].button->height() - entries[i+1].button->y();
      if (d>0){
	for (i2=i+1; 
	     i2<nbuttons && 
	       entries[i2].button->y() < entries[i2-1].button->y() +entries[i2-1].button->height(); 
	     i2++){
	  entries[i2].button->move(entries[i2].button->x(), entries[i2].button->y() + d);
	  if (entries[i2].button->y() > bound_top_left - entries[i2].button->height()&&
	      entries[i2].button->y() < bound_bottom_right
	      && height() - bound_bottom_right > entries[i2].button->height()
	      )
	    entries[i2].button->move(entries[i2].button->x(), bound_bottom_right);
	}
      } 
    }
    
    d = entries[nbuttons-1].button->y() + entries[nbuttons-1].button->height() - height();
    if (entries[nbuttons-1].button->y() < bound_bottom_right &&
	entries[nbuttons-1].button->y() + entries[nbuttons-1].button->height() > bound_top_left)
      d = entries[nbuttons-1].button->y() + entries[nbuttons-1].button->height() - bound_top_left;
    if ( d > 0) {
      entries[nbuttons-1].button->move(entries[nbuttons-1].button->x(), 
				entries[nbuttons-1].button->y() - d);
      for (i=nbuttons-2; 
	   i>=0 &&
	     entries[i].button->y() + entries[i].button->height() > entries[i+1].button->y(); 
	   i--){
	entries[i].button->move(entries[i].button->x(), entries[i].button->y() - d);
	if (entries[i].button->y() > bound_top_left - entries[i].button->height()&&
	    entries[i].button->y() < bound_bottom_right)
	  entries[i].button->move(entries[i].button->x(), bound_top_left - entries[i].button->height());
      }
      if (l>nbuttons&&nbuttons>1) {
	if (entries[nbuttons-1].button == kde_button){
	  entries[nbuttons-1].button = entries[nbuttons-2].button;
	  entries[nbuttons-2].button = kde_button;
	  kde_button->setGeometry(entries[nbuttons-1].button->geometry());
	}
	delete_button(entries[nbuttons-1].button);
	l=0;
      }
      reposition(l+1);
    }
  }
  else { // orientation == horizontal
 
    for (i=0; i<nbuttons-1; i++){
      d = entries[i].button->x() + entries[i].button->width() - entries[i+1].button->x();
      if (d>0){
	for (i2=i+1; 
	     i2<nbuttons && 
	       entries[i2].button->x() < entries[i2-1].button->x() +entries[i2-1].button->width(); 
	     i2++){
	  entries[i2].button->move(entries[i2].button->x() + d, entries[i2].button->y());
	  if (entries[i2].button->x() > bound_top_left - entries[i2].button->width()&&
	      entries[i2].button->x() < bound_bottom_right
	      && width() - bound_bottom_right > entries[i2].button->width()
	      )
	    entries[i2].button->move(bound_bottom_right, entries[i2].button->y());
	}
      } 
    }
    
    d = entries[nbuttons-1].button->x() + entries[nbuttons-1].button->width() - width();
    if (entries[nbuttons-1].button->x() < bound_bottom_right &&
	entries[nbuttons-1].button->x() + entries[nbuttons-1].button->width() > bound_top_left)
      d = entries[nbuttons-1].button->x() + entries[nbuttons-1].button->width() - bound_top_left;
    if ( d > 0) {
      entries[nbuttons-1].button->move(entries[nbuttons-1].button->x() - d, 
				entries[nbuttons-1].button->y());
      for (i=nbuttons-2; 
	   i>=0 &&
	     entries[i].button->x() + entries[i].button->width() > entries[i+1].button->x(); 
	   i--){
	entries[i].button->move(entries[i].button->x() - d, entries[i].button->y());
	if (entries[i].button->x() > bound_top_left - entries[i].button->width()&&
	    entries[i].button->x() < bound_bottom_right)
	  entries[i].button->move(bound_top_left - entries[i].button->width(), entries[i].button->y());
      }
      if (l>nbuttons&&nbuttons>1) {
	if (entries[nbuttons-1].button == kde_button){
	  entries[nbuttons-1].button = entries[nbuttons-2].button;
	  entries[nbuttons-2].button = kde_button;
	  kde_button->setGeometry(entries[nbuttons-1].button->geometry());
	}
	delete_button(entries[nbuttons-1].button);
	l=0;
      }
      reposition(l+1);
    }
  }

}

void kPanel::find_a_free_place(){
  int i;
  if (orientation == vertical)
    entries[nbuttons-1].button->setGeometry(margin, 
					    entries[nbuttons-2].button->y() 
					    +entries[nbuttons-2].button->height(),
					    box_width, box_height);
  else
    entries[nbuttons-1].button->setGeometry(
					    entries[nbuttons-2].button->x() 
					    +entries[nbuttons-2].button->width(),
					    margin,
					    box_width, box_height);

  // find a free place.
  for (i=1; i<nbuttons-1; i++){
    if (orientation == vertical){
      if (entries[i-1].button->y()+box_height <= entries[i].button->y()-box_height){
	entries[nbuttons-1].button->setGeometry(margin,
						entries[i-1].button->y()+box_height,
						box_width, box_height);
	return;
      }
    }
    else{   // orientation == horizontal
      if (entries[i-1].button->x()+box_width <= entries[i].button->x()-box_width){
	entries[nbuttons-1].button->setGeometry(entries[i-1].button->x()+box_width,
						margin, 
						box_width, box_height);
	return;
      }
    }
  }
}

void kPanel::check_button_bounds(QWidget* button){
  if (orientation == vertical){
    if (button->y() > bound_top_left - button->height()&&
	button->y() < bound_bottom_right)
      if ((2 * button->y() + button->height()
	  < bound_top_left + bound_bottom_right
	   && bound_top_left - panel_button->y() - panel_button->height()
	   > button->height())
	  || bound_bottom_right > height() - button->height())	
	button->move(button->x(), bound_top_left - button->height());
      else
	button->move(button->x(), bound_bottom_right);
  }
  else {
    if (button->x() > bound_top_left - button->width()&&
	button->x() < bound_bottom_right)
      if ((2 * button->x() + button->width()
	   < bound_top_left + bound_bottom_right
	   && bound_top_left - panel_button->x() - panel_button->width()
	   > button->width())
	  || bound_bottom_right > width() - button->width())
	button->move(bound_top_left - button->width(), button->y());
      else
	button->move(bound_bottom_right, button->y());
  }
}
