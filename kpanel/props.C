//
// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//



#include "kpanel.h"
#include <klocale.h>
#include <qmsgbox.h>
#include <kfm.h>

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
     // --sven: kdisknav button start --
     else if (entries[i].popup == kdisknav)
       s="kdisknav";
     // --sven: kdisknav button end --
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

   writeOutRecentList();

   config->sync();
};

//////////////////////////////////////////////////////////////////////////////
void kPanel::writeOutRecentList(bool sync)
{
  KConfig *config = KApplication::getKApplication()->getConfig();
   config->setGroup("kdisknav");

   config->writeEntry("RecentFolders", recent_folders);
   config->writeEntry("RecentFiles", recent_files);

   if (sync)
     config->sync();
}


void kPanel::parseMenus(){
    ready_for_event_loop = true;
    KConfig *config = KApplication::getKApplication()->getConfig();

    config->setGroup("KDE Desktop Entries");
    QString temp = KApplication::localkdedir() +"/share/applnk";
    QString personal = config->readEntry("PersonalPath", temp.data() );
    temp = KApplication::kde_appsdir();
    QString kde_apps = config->readEntry("Path", temp.data() );


    if (personalFirst){
      pmenu->parse(QDir(personal));
      personal_menu = pmenu;         // I need this for K-Button drops; chris
      global_menu = 0;
      personal_pmi = 0;
      p_pmenu = new PMenu;
      p_pmenu->setAltSort(foldersFirst);
      p_pmenu->parse(QDir(kde_apps));
      PMenuItem* pmi = new PMenuItem ;
      QFileInfo fi(kde_apps);
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
//if (tmp->getQPopupMenu()){
	p_pmenu = new PMenu;
	p_pmenu->setAltSort(foldersFirst);
	p_pmenu->parse(QDir(personal));
	personal_menu = p_pmenu;         // I need this for K-Button drops; chris
	global_menu = 0;
	personal_pmi = 0;
	PMenuItem* pmi = new PMenuItem ;
	QFileInfo fi(personal);
	pmi->parse(&fi, p_pmenu);
	pmenu->add( new PMenuItem((EntryType) separator) );
	pmenu->add( pmi );
      }
      else {
	p_pmenu = new PMenu;
	p_pmenu->setAltSort(foldersFirst);
	p_pmenu->parse(QDir(personal));
	personal_menu = p_pmenu;         // I need this for K-Button drops; chris
	global_menu = pmenu;
	personal_pmi = new PMenuItem ;
	QFileInfo fi(personal);
	personal_pmi->parse(&fi, personal_menu);
	personal_menu->createMenu(personal_pmi->getQPopupMenu(), this);
      }
      delete tmp;
    }

    pmenu_add = new PMenu(*pmenu);

    pmenu->add( new PMenuItem(separator) );

    PMenu *panel_menu = new PMenu;
    panel_menu->add( new PMenuItem(add_but, klocale->translate("Add Application"), 0, 0, pmenu_add,
				   0, 0, new myPopupMenu, false, 0,
				   klocale->translate("Add an application or a submenu onto the panel")));

    PMenuItem* pdisknav;
    PMenuItem* pwindowlist;

    // --sven: kdisknav button start --
    panel_menu->add( pdisknav = new PMenuItem(prog_com, klocale->translate("Add Disk Navigator"), 0, 0, 0,
				   this, SLOT(add_kdisknav()), 0, false, 0,
				   klocale->translate("Add a Disk Navigator menu onto the panel")) );

    // --sven: kdisknav button end --
    panel_menu->add( pwindowlist = new PMenuItem(prog_com, klocale->translate("Add Windowlist"), 0, 0, 0,
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

    PMenuItem* fileBrowser =
      new PMenuItem(submenu, klocale->translate("Disk Navigator"),
                    0, "kdisknav.xpm", new PFileMenu(true), 0, 0,
                    kdisknav /* instead of new myPopupMenu */ , false, 0, //sven changed
		    klocale->translate("Quickly navigate through the filesystem"));

    pmenu->add( fileBrowser );


    pmenu->add( new PMenuItem(submenu, klocale->translate("Panel"), 0, "mini-panel.xpm",
			      panel_menu, 0, 0, new myPopupMenu) );
    pmenu->add( new PMenuItem(prog_com, klocale->translate("Lock Screen"), 0, "mini-key.xpm", 0,
			      this, SLOT(call_klock()), 0, false, 0,
			      klocale->translate("Lock screen")) );
    pmenu->add( new PMenuItem(prog_com, klocale->translate("Logout"), 0, "mini-exit.xpm", 0,
			      this, SLOT(ask_logout()), 0, false, 0, klocale->translate("Logout")) );

    pmenu->createMenu(new myPopupMenu, this);
    PFileMenu::calculateMaxEntriesOnScreen(fileBrowser);

    panel_popup = panel_menu->getQPopupMenu();
    add_disknav_entry = pdisknav->getId();
    add_windowlist_entry = pwindowlist->getId();

    if (has_kdisknav_button)
      panel_popup->setItemChecked(add_disknav_entry, true);

    if (has_windowlist_button)
      panel_popup->setItemChecked(add_windowlist_entry, true);

    int i;
    for (i=0; i<nbuttons && entries[i].button!=kde_button; i++);
    entries[i].popup = pmenu->getQPopupMenu();
    kmenu = entries[i].popup;
    ready_for_event_loop = false;
    kde_button->setCursor(arrowCursor);
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
	     has_windowlist_button = true;
	 }
	 // --sven: kdisknav button start --
	 else if (button_entry_value == "kdisknav"){
	     addButtonInternal(0, (int)x, (int)y, "kdisknav");
	     buttonAdded = true;
	     has_kdisknav_button = true;
	 }
         // --sven: kdisknav button end --
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

   // kdisknav

   config->setGroup("kdisknav");

   if (config->hasKey("MaxRecentFoldersEntries"))
     max_recent_folders_entries =
       config->readEntry("MaxRecentFoldersEntries").toUInt();
   else {
     // config->writeEntry("MaxRecentFoldersEntries", 4);
     max_recent_folders_entries = 4;
   }

   if (config->hasKey("MaxRecentFilesEntries"))
     max_recent_files_entries =
       config->readEntry("MaxRecentFilesEntries").toUInt();
   else {
     // config->writeEntry("MaxRecentFilesEntries", 4);
     max_recent_files_entries = 4;
   }

   if (config->hasKey("MaxNavigableFolderEntries"))
     max_navigable_folder_entries =
       config->readEntry("MaxNavigableFolderEntries").toUInt();
   else {
     // config->writeEntry("MaxNavigableFolderEntries", 200);
     max_navigable_folder_entries = 200;
   }

   if (config->hasKey("ShowDotFiles"))
     show_dot_files = (config->readEntry("ShowDotFiles") == "on");
   else {
     // config->writeEntry("ShowDotFiles", "off");
     show_dot_files = false;
   }

   if (config->hasKey("IgnoreCase"))
     ignore_case = (config->readEntry("IgnoreCase") == "on");
   else {
     // config->writeEntry("IgnoreCase", "off");
     ignore_case = false;
   }

   if (config->hasKey("RecentFolders"))
     config->readListEntry("RecentFolders", recent_folders);

   if (config->hasKey("RecentFiles"))
     config->readListEntry("RecentFiles", recent_files);

   if (config->hasKey("ShowGlobalSection"))
     show_shared_section = (config->readEntry("ShowGlobalSection") == "on");
   else
     show_shared_section = true;

   if (config->hasKey("ShowLocalSection"))
     show_personal_section = (config->readEntry("ShowLocalSection") == "on");
   else {
     show_personal_section = true;
   }

   if (config->hasKey("ShowRecentSection"))
     show_recent_section = (config->readEntry("ShowRecentSection") == "on");
   else {
     show_recent_section = true;
   }

   if (config->hasKey("ShowOptionEntry"))
     show_option_entry = (config->readEntry("ShowOptionEntry") == "on");
   else {
     show_option_entry = true;
   }

}


void kPanel::configurePanel(){
  execute("kcmkpanel");
}

void kPanel::editMenus(){
    QString editor = findMenuEditor(kapp->kde_appsdir());
    if ( editor.isEmpty() ){
	QMessageBox::warning( 0, "Panel",
			      klocale->translate("The menu editor is not installed."),
			      klocale->translate("Oops!"));
    }
    else {
	KFM* kfm = new KFM;
	QString com = "file:";
	com.append(editor);
	kfm->exec(com, 0L);
	delete kfm;
    }
}

