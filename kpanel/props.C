//
// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//



#include "kpanel.h"
#include "props.h"
#include "props.moc"
#include <klocale.h>

mySlider::mySlider (int minValue, int maxValue, int step, int value, 
		    Orientation orient, 
		    QLabel     * Label,
		    QWidget    * parent,
		    const char * name)
  : QSlider( minValue, maxValue, step, value, orient, parent, name )
{
  setSteps ( 125, 125 );
  label = Label;
}

void mySlider::setLabelText(int val)
{
  QString a;
  a.setNum(val);
  a += " ms";

  label->setText ( (const char*)a );
  
  if ( val == lastval )
    return;
}

void kPanel::writeInitStyle (KConfig* config, int size)
{
  switch (size){
  case tiny:  // tiny style
    {
      config->writeEntry("Style", "tiny");
      config->writeEntry("BoxWidth",26);
      config->writeEntry("BoxHeight",26);
      config->writeEntry("Margin",0);
      config->writeEntry("TaskbarButtonHorizontalSize",4);
      config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
      config->writeEntry("DesktopButtonRows",1);
      config->writeEntry("DateFont","*-times-medium-i-normal--12-*");
      break;
    }
  
  case normal:  // normal style
    {
      config->writeEntry("Style", "normal");
      config->writeEntry("BoxWidth",45);
      config->writeEntry("BoxHeight",45);
      config->writeEntry("Margin",0);
      config->writeEntry("TaskbarButtonHorizontalSize",4);
      config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
      config->writeEntry("DesktopButtonRows",2);
      config->writeEntry("DateFont","*-times-medium-i-normal--12-*");
      break;
    }

  case large:  // large style
    {
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
}
  
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

#define findMax(grp) \
do \
{ \
  for ( i = 0; i < 5; i++ ) { \
    QButton *tmpbtn = grp->find(i); \
    if ( ! tmpbtn ) break; \
    int mins = tmpbtn->sizeHint().width(); \
    if ( mins > maxw[i]) maxw[i] = mins; \
  } \
} while (0)  

#define initMax() for ( int j=0; j < 10; maxw[j++] = 0 )

#define STARTX 10
#define STARTY 15

void kPanel::configure_panel(){

  int maxw[10];
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

  initMax();

  bgrloc = new QButtonGroup(klocale->translate("Location"),  mw );
  rb = new QRadioButton( klocale->translate("Top"), bgrloc );
  rb->setChecked( (orientation == horizontal && position == top_left) );

  rb = new QRadioButton( klocale->translate("Left"), bgrloc );
  rb->setChecked( (orientation == vertical && position == top_left) );

  rb = new QRadioButton( klocale->translate("Bottom"), bgrloc );
  rb->setChecked( (orientation == horizontal && position == bottom_right) );

  findMax (bgrloc);

  // new group for the taskbar
  bgrta = new QButtonGroup(klocale->translate("Taskbar"),  mw );
  rb = new QRadioButton( klocale->translate("Hidden"), bgrta );
  rb->setChecked( (taskbar_position == hidden) );

  rb = new QRadioButton( klocale->translate("Top"), bgrta );
  rb->setChecked( (taskbar_position == top) );

  rb = new QRadioButton( klocale->translate("Bottom"), bgrta );
  rb->setChecked( (taskbar_position == bottom) );

  rb = new QRadioButton( klocale->translate("Top/Left"), bgrta );
  rb->setChecked( (taskbar_position == taskbar_top_left) );

  findMax (bgrta);

  int lastx = 0;
  for ( i = 0; i < 5; i++ ) {

    QButton *btn1, *btn2;
    if ( (btn1 = bgrloc->find(i)) != 0 ) {

      btn1->setGeometry(10 + lastx, 15, maxw[i] + 5, 25);
    }
    if ( (btn2 = bgrta->find(i)) != 0 ) {

      btn2->setGeometry(10 + lastx, 15, maxw[i] + 5, 25);
    }

    lastx += maxw[i] + 10;

    if ( ! btn1 && ! btn2 ) break;
  }

  bgrloc->setGeometry(10, 10, lastx , 45);
  bgrta->setGeometry(10, 65, lastx , 45);

  // size
  bgrl = new QButtonGroup(klocale->translate("Style"), mw);

  costy = new QComboBox(bgrl);
  costy->insertItem(klocale->translate("Tiny Style"));
  costy->insertItem(klocale->translate("Normal Style"));
  costy->insertItem(klocale->translate("Large Style"));

  costy->setGeometry(10, 20, costy->sizeHint().width() + 10, 25);
  bgrl->setGeometry(10, 120, lastx, 55);

  if (config->readEntry("Style") == "tiny")
    old_style = 0;
  else if (config->readEntry("Style") == "normal")
    old_style = 1;
  else if (config->readEntry("Style") == "large")
    old_style = 2;
    
  costy->setCurrentItem(old_style);

  tab->addTab(mw, klocale->translate("Panel"));

  mw = new QWidget(tab);

  // menu tool tips
  bgrm = new QButtonGroup(klocale->translate("Menu Tooltips"), mw);

  mttb = new QRadioButton( klocale->translate("On"), bgrm );
  mttb->setChecked( menu_tool_tips >= 0 );
  mttb->setGeometry(STARTX, STARTY, mttb->sizeHint().width(), mttb->sizeHint().height());

  rb = new QRadioButton( klocale->translate("Off"), bgrm );
  rb->setChecked( menu_tool_tips < 0 );
  rb->setGeometry (STARTX + mttb->width() + STARTX, STARTY, 
		   rb->sizeHint().width(), rb->sizeHint().height());

  QLabel 
    *mtts_start  = new QLabel("0", bgrm), 
    *mtts_actual = new QLabel("1000 ms", bgrm), 
    *mtts_end    = new QLabel("2000", bgrm);

  mtts = new mySlider (0, 2000, 250, // min, max, step
		       (menu_tool_tips >= 0) ? menu_tool_tips : 0, // actual value
		       QSlider::Horizontal, 
		       mtts_actual,
		       bgrm);

  mtts->setTickmarks    ( (QSlider::TickSetting)Below );
  mtts->setTickInterval ( 250 );
  mtts->setTracking     ( True );

  mtts->setGeometry(STARTX, 40 + mttb->height(), lastx - 20, 25);
  //  mtts->setEnabled(False);

  mtts_start->setGeometry(mtts->x(), 25 + mttb->height(), 20, 15);
  mtts_actual->
    setGeometry(mtts->x() + (mtts->width() / 2) - (mtts_actual->sizeHint().width()/2), 
		25 + mttb->height(), 
		mtts_actual->sizeHint().width(), 15);
  mtts_end->setGeometry(mtts->x() + mtts->width() - mtts_end->sizeHint().width(), 
			25 + mttb->height(), 
			mtts_end->sizeHint().width(), 15);


  bgrm->setGeometry(STARTX, STARTY, lastx, 90);

  connect(mtts, SIGNAL(valueChanged(int)), mtts, SLOT(setLabelText(int)));
  connect(mttb, SIGNAL(toggled(bool)), mtts, SLOT(setEnabled(bool)));

  
  // other settings
  bgro = new QButtonGroup(klocale->translate("Others"), mw );
  
  cbpf = new QCheckBox(klocale->translate("Personal First"), bgro);
  cbpf->setChecked(config->readEntry("PersonalFirst") == "on");
  
  cbah = new QCheckBox(klocale->translate("Auto Hide Panel"), bgro);
  cbah->setChecked(config->readEntry("AutoHide") == "on");
  
  cbaht = new QCheckBox(klocale->translate("Auto Hide Taskbar"), bgro);
  cbaht->setChecked(config->readEntry("AutoHideTaskbar") == "on");
  
  int lasty = 15;
  for ( i = 0; i < 10; i++ ) {
    
    QButton *btn = bgro->find(i);
    if ( ! btn ) break;
    
    int w = btn->sizeHint().width();
    btn->setGeometry(10, lasty, w + 10, 20);

    lasty += 25;
  }

  bgro->setGeometry(STARTX, STARTY + bgrm->height() + 10, lastx, lasty);
  
  // other settings
  bgrt = new QButtonGroup(klocale->translate("Clock"), mw );

  mttt = new QRadioButton( klocale->translate("24h"), bgrt );
  mttt->setChecked( !clockAmPm );
  mttt->setGeometry(STARTX, STARTY, mttt->sizeHint().width(), mttt->sizeHint().height());

  rb = new QRadioButton( klocale->translate("12h AM/PM"), bgrt );
  rb->setChecked( clockAmPm );
  rb->setGeometry (STARTX, STARTY + 10 + mttt->sizeHint().height(), 
		   rb->sizeHint().width(), rb->sizeHint().height());

  bgrt->setGeometry(STARTX, STARTY + bgrm->height() + bgro->height()+ 20, 
		    lastx, (rb->sizeHint().height() + 15)* 2);

  tab->addTab(mw, klocale->translate("Options"));

  mw = new QWidget(tab);

  QLabel *label;
  
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

  tab->resize(365, 385);
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

  
  config->writeEntry("MenuToolTips", mttb->isOn()?mtts->value():-1);

  config->writeEntry("PersonalFirst", cbpf->isChecked()?"on":"off");

  config->writeEntry("AutoHide", cbah->isChecked()?"on":"off");

  config->writeEntry("AutoHideTaskbar", cbaht->isChecked()?"on":"off");

  config->writeEntry("ClockAmPm", mttt->isOn()?"off":"on");


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

    writeInitStyle(config, costy->currentItem());
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
