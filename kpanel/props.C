//
// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//



#include "kpanel.h"
#include <klocale.h>

void kPanel::write_out_configuration(){
   int i;
   QPushButton* tmp_push_button = NULL;
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


void kPanel::read_in_configuration(){

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
       if (num == 0){
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

	 PMenuItem* pmi = NULL;
	 
	 bool buttonAdded = FALSE;
	 if (button_entry_value == "windowlist"){
	     addButtonInternal(NULL, (int)x, (int)y, "windowlist");
	     buttonAdded = TRUE;
	 }
	 else {
	   pmi = pmenu->searchItem( button_entry_value);
	   if (pmi){
	     addButtonInternal(pmi, (int)x, (int)y);
	     buttonAdded = TRUE;
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



void kPanel::configure_panel(){
  int i;
  if (tab){
    for (i=0; i<8; i++)
      led[i]->setText(KWM::getDesktopName(i+1));
    tab->show();
    tab->raise();
    return;
  }
  
  KConfig *config = KApplication::getKApplication()->getConfig();
  config->setGroup("kpanel");

  tab = new QTabDialog(NULL, NULL, FALSE);
  tab->setCancelButton(klocale->translate("Cancel"));
  tab->setOKButton(klocale->translate("OK"));

  QWidget* mw = new QWidget(tab);
  QRadioButton *rb;

  QLabel *label;

  bgrloc = new QButtonGroup(klocale->translate("Location"),  mw );
  rb = new QRadioButton( klocale->translate("Top"), bgrloc );
  if (orientation == horizontal && position == top_left)
    rb->setChecked(TRUE);
  rb->setGeometry(10, 15, 65, 25);
  rb = new QRadioButton( klocale->translate("Left"), bgrloc );
  if (orientation == vertical && position == top_left)
    rb->setChecked(TRUE);
  rb->setGeometry(85, 15, 65, 25);
  rb = new QRadioButton( klocale->translate("Bottom"), bgrloc );
  if (orientation == horizontal && position == bottom_right)
    rb->setChecked(TRUE);
  rb->setGeometry(160, 15, 65, 25);
  bgrloc->setGeometry(10, 10, 320, 45);

  bgrta = new QButtonGroup(klocale->translate("Taskbar"),  mw );
  rb = new QRadioButton( klocale->translate("Hidden"), bgrta );
  if (taskbar_position == hidden)
    rb->setChecked(TRUE);
  rb->setGeometry(10, 15, 65, 25);
  rb = new QRadioButton( klocale->translate("Top"), bgrta );
  if (taskbar_position == top)
    rb->setChecked(TRUE);
  rb->setGeometry(85, 15, 65, 25);
  rb = new QRadioButton( klocale->translate("Bottom"), bgrta );
  if (taskbar_position == bottom)
    rb->setChecked(TRUE);
  rb->setGeometry(160, 15, 65, 25);
  rb = new QRadioButton( klocale->translate("Top/Left"), bgrta );
  if (taskbar_position == taskbar_top_left)
    rb->setChecked(TRUE);
  rb->setGeometry(235, 15, 75, 25);
  bgrta->setGeometry(10, 65, 320, 45);


  label = new QLabel(klocale->translate("Style:"), mw);
  label->setAlignment(AlignRight | AlignVCenter);
  label->setGeometry(10, 130, 50, 25);
  costy = new QComboBox(mw);
  costy->insertItem(klocale->translate("Tiny Style"));
  costy->insertItem(klocale->translate("Normal Style"));
  costy->insertItem(klocale->translate("Large Style"));
  costy->setGeometry(70,130, 120, 25);

  if (config->readEntry("Style") == "tiny")
    old_style = 0;
  else if (config->readEntry("Style") == "normal")
    old_style = 1;
  else if (config->readEntry("Style") == "large")
    old_style = 2;
    
  costy->setCurrentItem(old_style);

  cbtt = new QCheckBox(klocale->translate("Menu Tooltips"), mw);
  cbtt->setGeometry(220, 130, 110, 25);
  cbtt->setGeometry(220, 120, 110, 25);
  cbtt->setChecked(config->readNumEntry("MenuToolTips")>=0);
  
  cbpf = new QCheckBox(klocale->translate("Personal First"), mw);
  cbpf->setGeometry(220, 140, 110, 25);
  cbpf->setChecked(config->readEntry("PersonalFirst") == "on");

  
  tab->addTab(mw, klocale->translate("Panel"));

  mw = new QWidget(tab);
  
  for (i=0; i<8; i++){
    QString s;
    s.setNum(i+1);
    label = new QLabel(s, mw);
    label->setAlignment(AlignRight | AlignVCenter);
    led[i] = new QLineEdit(mw);
    led[i]->setText(KWM::getDesktopName(i+1));
    switch (i){
    case 0:
      label->setGeometry(10, 10, 20, 25);
      led[i]->setGeometry(40,10,120,25);
      break;
    case 1:
      label->setGeometry(10, 40, 20, 25);
      led[i]->setGeometry(40,40,120,25);
      break;
    case 2:
      label->setGeometry(10, 70, 20, 25);
      led[i]->setGeometry(40,70,120,25);
      break;
    case 3:
      label->setGeometry(10, 100, 20, 25);
      led[i]->setGeometry(40,100,120,25);
      break;
    case 4:
      label->setGeometry(170, 10, 20, 25);
      led[i]->setGeometry(200,10,120,25);
      break;
    case 5:
      led[i]->setGeometry(200,40,120,25);
      label->setGeometry(170, 40, 20, 25);
      break;
    case 6:
      label->setGeometry(170, 70, 20, 25);
      led[i]->setGeometry(200,70,120,25);
      break;
    case 7:
      label->setGeometry(170, 100, 20, 25);
      led[i]->setGeometry(200,100,120,25);
      break;
    }
  }

  for (i=number_of_desktops; i<8; i++)
    led[i]->setEnabled(False);

  sl_nr_db = new QSlider(1, 4, 1, number_of_desktops/2, QSlider::Horizontal, mw);
  label = new QLabel(klocale->translate("Visible"), mw);
  label->setAlignment(AlignRight | AlignVCenter);
  label->setGeometry(50, 130, 60, 25);
  sl_nr_db->setGeometry(120,134,120,16);
  connect(sl_nr_db, SIGNAL(valueChanged(int)), this, SLOT(slotsl_nr_db(int)));

  sl_dbhs = new QSlider(1, 6, 1, dbhs, QSlider::Horizontal, mw);
  label = new QLabel(klocale->translate("Width"), mw);
  label->setAlignment(AlignRight | AlignVCenter);
  label->setGeometry(50, 160, 60, 25);
  sl_dbhs->setGeometry(120,164,120,16);

  tab->addTab(mw, klocale->translate("Desktops"));

  connect( tab, SIGNAL( applyButtonPressed() ), this, SLOT( slotPropsApply() ) );
  connect( tab, SIGNAL( cancelButtonPressed() ), this, SLOT( slotPropsCancel() ) );

  tab->resize(360, 260);
  tab->setCaption(klocale->translate("KPanel Configuration"));
  tab->show();

}

void kPanel::slotPropsApply(){
  KConfig *config = KApplication::getKApplication()->getConfig();
  config->setGroup("kpanel");
  int i;
  for (i=0; i<8; i++){
    KWM::setDesktopName(i+1, led[i]->text());
  }
  KWM::setNumberOfDesktops(sl_nr_db->value()*2);
  QApplication::flushX();
  dbhs = sl_dbhs->value();
  config->writeEntry("DesktopButtonHorizontalSize", dbhs);

  
  if (cbtt->isChecked())
    config->writeEntry("MenuToolTips", 1000);
  else
    config->writeEntry("MenuToolTips", -1);

  config->writeEntry("PersonalFirst", cbpf->isChecked()?"on":"off");

  QButton* tmp_button;
  for (i=0; (tmp_button = bgrloc->find(i)); i++){
    if (tmp_button->isOn()){
      switch (i){
      case 0: config->writeEntry("Position", "top");break;
      case 1: config->writeEntry("Position", "left");break;
      case 2: config->writeEntry("Position", "bottom");break;
      case 3: config->writeEntry("Position", "right");break;
      }
    }
  }
  for (i=0; (tmp_button = bgrta->find(i)); i++){
    if (tmp_button->isOn()){
      switch (i){
      case 0: config->writeEntry("TaskbarPosition", "hidden");break;
      case 1: config->writeEntry("TaskbarPosition", "top");break;
      case 2: config->writeEntry("TaskbarPosition", "bottom");break;
      case 3: config->writeEntry("TaskbarPosition", "top_left");break;
      }
    }
  }

  if (old_style != costy->currentItem()){
    switch (costy->currentItem()){
    case 0: // tiny style
      config->writeEntry("Style", "tiny");
      config->writeEntry("BoxWidth",26);
      config->writeEntry("BoxHeight",26);
      config->writeEntry("Margin",0);
      config->writeEntry("TaskbarButtonHorizontalSize",4);
      config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
      config->writeEntry("DesktopButtonRows",1);
//       config->writeEntry("DateFont","*-helvetica-medium-r-normal--8-*");
      config->writeEntry("DateFont","*-times-medium-i-normal--12-*");
      break;
    case 1: // normal style
      config->writeEntry("Style", "normal");
      config->writeEntry("BoxWidth",45);
      config->writeEntry("BoxHeight",45);
      config->writeEntry("Margin",0);
      config->writeEntry("TaskbarButtonHorizontalSize",4);
      config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
      config->writeEntry("DesktopButtonRows",2);
      config->writeEntry("DateFont","*-times-medium-i-normal--12-*");
      break;
    case 2: // large style
      config->writeEntry("Style", "large");
      config->writeEntry("BoxWidth",47);
      config->writeEntry("BoxHeight",47);
      config->writeEntry("Margin",4);
      config->writeEntry("TaskbarButtonHorizontalSize",4);
      config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--14-*");
      config->writeEntry("DesktopButtonRows",2);
      config->writeEntry("DateFont","*-times-bold-i-normal--12-*");
      break;
    }
  }

  write_out_configuration();

  restart();
  delete tab;
  tab = NULL;
}
void kPanel::slotPropsCancel(){
  delete tab;
  tab = NULL;
}

void kPanel::slotsl_nr_db(int value){
  int i;
  for (i=0; i<8; i++){
    led[i]->setEnabled(i<value*2);
  }
}

