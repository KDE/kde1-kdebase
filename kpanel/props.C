//
// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//



#include "kpanel.h"
#include <klocale.h>
#include <qmsgbox.h>

extern void execute(const char*);

  
void kPanel::writeOutConfiguration(){
   int i;
   QPushButton* tmp_push_button = 0;
   KConfig *config = KApplication::getKApplication()->getConfig();
   config->setGroup("kpanelButtons"); 
   QString s;
   QString b;
   float delta;
   float delta2;
   float left_side;
   float real_space;
   
   QStrList button;
   QStrList buttondelta;

   if (orientation == vertical)
     real_space = height() - bound_bottom_right + bound_top_left 
       - panel_button->y() - panel_button->height();
   else
     real_space = width() - bound_bottom_right + bound_top_left 
       - panel_button->x() - panel_button->width();

   for (i=0; i<nbuttons;i++){
     if (entries[i].popup == windowlist)
       s="windowlist";
     else if (entries[i].pmi){
       s=entries[i].pmi->getSaveName();
     }
     else
       s = "system";
     button.append(s);
     if (i>0)
       tmp_push_button = entries[i-1].button;
     else
       tmp_push_button = panel_button;
      
     if (orientation == vertical){
       delta = entries[i].button->y() - tmp_push_button->y() - tmp_push_button->height();
       left_side = entries[i].button->y() - bound_bottom_right;
     }
     else{
       delta = entries[i].button->x() - tmp_push_button->x() - tmp_push_button->width();
       left_side = entries[i].button->x() - bound_bottom_right;
     }

     if ((left_side >= 0 && left_side < delta) || (left_side < 0 && i == nbuttons-1)){
       if (left_side >= 0){
 	delta = left_side;
 	config->writeEntry("control", i-1);
       }
       else{
 	// there is no button on the right of the control_group,
 	// that means "i" is the lastbutton that is on the left
 	tmp_push_button = entries[i].button;
 	config->writeEntry("control", i);
       }
       if (orientation == vertical)
 	delta2 =bound_top_left - tmp_push_button->y() - tmp_push_button->height();
       else
 	delta2 = bound_top_left - tmp_push_button->x() - tmp_push_button->width();
       s.setNum(delta2*10000/real_space);
       config->writeEntry("controldelta", s);
     }

     s.setNum(delta*10000/real_space);
     buttondelta.append(s);
   }

   config->writeEntry("Buttons", button);
   config->writeEntry("ButtonDelta", buttondelta);
   config->sync();
};



void kPanel::parseMenus(){
    ready_for_event_loop = true;
    KConfig *config = KApplication::getKApplication()->getConfig();

    config->setGroup("KDE Desktop Entries");
    QString temp = QDir::homeDirPath() +"/.kde/share/applnk";
    QString personal = config->readEntry("PersonalPath", temp.data() );
    temp = KApplication::kde_appsdir();
    QString kde_apps = config->readEntry("Path", temp.data() );


    if (personalFirst){
      pmenu->parse(QDir(personal));
      p_pmenu = new PMenu;
      p_pmenu->setAltSort(foldersFirst);
      p_pmenu->parse(QDir(kde_apps));
      PMenuItem* pmi = new PMenuItem ;
      QFileInfo fi(personal);
      pmi->parse(&fi, p_pmenu);
      pmenu->add( new PMenuItem((EntryType) separator) );
      pmenu->add( pmi );
    }
    else {
      pmenu->parse(QDir(kde_apps));
      PMenu* tmp = new PMenu;
      tmp->setAltSort(foldersFirst);
      tmp->parse(QDir(personal));
      tmp->createMenu(new myPopupMenu, this);
      if (tmp->getQPopupMenu() && tmp->getQPopupMenu()->count()>0){
	p_pmenu = new PMenu;
	p_pmenu->setAltSort(foldersFirst);
	p_pmenu->parse(QDir(personal));
	PMenuItem* pmi = new PMenuItem ;
	QFileInfo fi(personal);
	pmi->parse(&fi, p_pmenu);
	pmenu->add( new PMenuItem((EntryType) separator) );
	pmenu->add( pmi );
      }
      delete tmp;
    }
    
    pmenu->add( new PMenuItem(separator) );
    pmenu_add = new PMenu(*pmenu);

    PMenu *panel_menu = new PMenu;
    panel_menu->add( new PMenuItem(add_but, klocale->translate("Add application"), 0, 0, pmenu_add,
				   0, 0, new myPopupMenu, false, 0, 
				   klocale->translate("Add an application or a submenu onto the panel")));
    panel_menu->add( new PMenuItem(prog_com, klocale->translate("Add windowlist"), 0, 0, 0, 
				   this, SLOT(add_windowlist()), 0, false, 0, 
				   klocale->translate("Add a windowlist menu onto the panel")) );
    panel_menu->add( new PMenuItem(prog_com, klocale->translate("Configure"), 0, 0, 0, 
				   this, SLOT(configurePanel()), 0, false, 0, 
				   klocale->translate("Configure panel")) );
    panel_menu->add( new PMenuItem(prog_com, klocale->translate("Edit Menus"), 0, 0, 0, 
				   this, SLOT(editMenus()), 0, false, 0, 
				   klocale->translate("Add or remove applications in the menu structure ")) );
    panel_menu->add( new PMenuItem(prog_com, klocale->translate("Restart"), 0, 0, 0, 
				   this, SLOT(restart()), 0, false, 0, 
				   klocale->translate("Restart panel")) );
    pmenu->add( new PMenuItem(submenu, klocale->translate("Panel"), 0, 0, panel_menu,
			      0, 0, new myPopupMenu) );
    pmenu->add( new PMenuItem(prog_com, klocale->translate("Lock Screen"), 0, 0, 0,
			      this, SLOT(call_klock()), 0, false, 0, 
			      klocale->translate("Lock screen")) );
    pmenu->add( new PMenuItem(prog_com, klocale->translate("Logout"), 0, 0, 0,
			      this, SLOT(ask_logout()), 0, false, 0, klocale->translate("Logout")) );

    pmenu->createMenu(new myPopupMenu, this);
    entries[0].popup = pmenu->getQPopupMenu();

    ready_for_event_loop = false;
    entries[0].button->setCursor(arrowCursor);
}


void kPanel::readInConfiguration(){

   KConfig *config = KApplication::getKApplication()->getConfig();
   config->setGroup("kpanelButtons");
 
   QStrList button;
   QStrList buttondelta;
   QString button_entry_value;

   config->readListEntry("Buttons", button);
   QString a;

   config->readListEntry("ButtonDelta", buttondelta);

   float x;
   float y;
   float real_space;
   if (orientation == vertical){
     real_space = height() - bound_bottom_right + bound_top_left 
       - panel_button->y() - panel_button->height();
     x = margin; y = panel_button->y() + panel_button->height();
   }
   else{
     real_space = width() - bound_bottom_right + bound_top_left 
       - panel_button->x() - panel_button->width();
     y = margin; x = panel_button->x() + panel_button->width();
   }
  
   int num = -1;
   float delta = 0;
   QString tmp;
    
   int control = 1;
   float controldelta = -1;
  
   if (config->hasKey("control"))
     control = config->readNumEntry("control");

   if (config->hasKey("controldelta"))
     controldelta = QString(config->readEntry("controldelta")).toFloat();
  
   if (controldelta >= 0){
     bound_top_left = 0;
     bound_bottom_right = 0;
   }

   while (num < 0 ||
	  !button_entry_value.isNull()){

     if (num>=0){
       if (button_entry_value == "system"){
 	// the unremovable kde_button!
 	if (orientation == horizontal){
 	  kde_button->move((int)(x + delta * real_space / 10000) , (int)y);
 	  x = kde_button->x() + kde_button->width();
 	}
 	else {
 	  kde_button->move((int)x, (int)(y + delta * real_space / 10000));
 	  y = kde_button->y() + kde_button->height();
 	}
       }
       else {

	 if (orientation == horizontal)
	   x += delta * real_space / 10000;
	 else 
	   y +=delta * real_space / 10000;

	 PMenuItem* pmi = 0;
	 
	 bool buttonAdded = false;
	 if (button_entry_value == "windowlist"){
	     addButtonInternal(0, (int)x, (int)y, "windowlist");
	     buttonAdded = true;
	 }
	 else {
	   pmi = pmenu->searchItem( button_entry_value);
	   if (pmi){
	     addButtonInternal(pmi, (int)x, (int)y);
	     buttonAdded = true;
	   }
	 }
	 
	 if (buttonAdded){
	   if (orientation == horizontal)
	     x += entries[nbuttons-1].button->width();
	   else 
	     y += entries[nbuttons-1].button->height();
	   
	 }
       }
     }

     if (num >= control && controldelta>=0){
       if (orientation == horizontal){
 	control_group->move((int)(x + controldelta * real_space / 10000), 
			    control_group->y());
 	if (control_group->x() + control_group->width() > width())
 	  control_group->move(width() - control_group->width(), (int)y);
 	bound_top_left = control_group->x();
 	bound_bottom_right = control_group->x() + control_group->width();
 	x = bound_bottom_right;
       }
       else {
 	control_group->move(control_group->x(), 
			    (int)(y  + controldelta * real_space / 10000));
 	if (control_group->y() + control_group->height() > height())
 	  control_group->move((int)x, height() - control_group->height());
 	bound_top_left = control_group->y();
 	bound_bottom_right = control_group->y() + control_group->height();
 	y = bound_bottom_right;
       }
       controldelta = -1;
     }


     num++;
     if (num == 0){
       button_entry_value = button.first();
       tmp = buttondelta.first();
       delta = tmp.isNull()?0:tmp.toFloat();
     }
     else {
       button_entry_value = button.next();
       tmp = buttondelta.next();
       delta = tmp.isNull()?0:tmp.toFloat();
     }
   }
  
}


void kPanel::configurePanel(){
  execute("kcmkpanel");
}

void kPanel::editMenus(){
  PMenuItem* pmi = pmenu->getMenuEditorItem();
  if (!pmi){
    QMessageBox::warning( 0, "Panel", 
			  klocale->translate("The menu editor is not installed."),
			  klocale->translate("Oops!"));
  } else {
    pmi->exec();
  }
}

