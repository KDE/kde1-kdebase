// $ Id: $
//
// kvt. Part of the KDE project.
//
// Copyright (C) 1996 Matthias Ettrich
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qapp.h>
#include <qpushbt.h>
#include <qbitmap.h>
#include <qfiledlg.h>
#include <qwindefs.h>
#include <qsocknot.h>
#include <qmsgbox.h>
#include <qlayout.h>
#include <qlined.h> 
#include <qbttngrp.h>
#include <qradiobt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <locale.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// KDE includes
#include <kconfig.h>
#include <kapp.h>
#include <kwm.h>
#include <kurl.h>
#include <kcolordlg.h>

#include "kvt_version.h"

#include "main.h"

extern "C" {
extern void *safemalloc(int, const char *identifier);
extern void get_token();
extern void handle_X_event(XEvent event, unsigned char);
extern void screen_refresh();
extern void extract_colors( char *fg_string, char *bg_string);
extern void clean_exit(int);

void rxvt_main(int argc,char **argv);

int run_command(unsigned char *,unsigned char **);

#include "rxvt.h"
#include "sbar.h"
#include "screen.h"
#include "xsetup.h"
#include "command.h"

extern struct sbar_info sbar;
extern WindowInfo MyWinInfo;
extern XSizeHints sizehints;

extern int font_num;
extern void LoadNewFont();

extern char *xvt_name; // the name the program is run under
extern char *window_name;
extern char *icon_name;


extern unsigned long	foreground;	/* foreground pixel value */
extern unsigned long	background;	/* background pixel value */
extern int rstyle;

extern int BackspaceSendsControlH;

extern void kvt_set_fontnum(char *);
extern void kvt_set_menubar(int);
extern void kvt_set_scrollbar(int);
extern void kvt_set_size_increment(int, int);
}

extern Display* display;
/* file descriptor for child process */
extern int comm_fd;
extern Window		main_win;
extern Window		vt_win;
extern char *fg_string_tmp, *bg_string_tmp;

kVt* kvt = NULL;
OptionDialog *m_optiondialog = 0;
QString kvt_charclass;

// the scrollbar hack
static int length = 0;
static int low = 0;
static int high = 0;

static XEvent stored_xevent_for_keys;

static const char* kvt_sizes[] = {
  "normal", 
  "tiny", 
  "small", 
  "medium", 
  "large", 
  "huge"
};

static Kvt_Dimen kvt_dimens[] = {
  { "80x24", 80, 24 },
  { "80x52", 80, 52 },
  { "96x24", 96, 24 },
  { "96x52", 96, 52 },
  { 0, 0, 0 }
};
static int kvt_dimen;

static const char* color_mode_name[] = {
  "ANSI",
  "Console",
  "KDE",
  0
};

static const char* backspace_name[] = {
  "Delete (^?)",
  "Backspace (^H)",
  0
};

int o_argc;
char ** o_argv;
char command_line[2048];

void kvt_set_fontnum(char *s_arg){
  int i;
  QString s = s_arg;
  for (i=kvt_normal; i<=kvt_huge; i++){
    if (s == kvt_sizes[i])
      font_num = i;
  }
}

void kvt_set_dimension(char *s_arg){
  int i;
  QString s = s_arg;
  for (i=0; kvt_dimens[i].text; i++){
    if (s == kvt_dimens[i].text) {
      MyWinInfo.cwidth = kvt_dimens[i].x;
      MyWinInfo.cwidth = kvt_dimens[i].y;
      kvt_dimen = i;
      break;
    }
  }
}

void kvt_set_menubar(int b){
    kvt->setMenubar((b!=0));
}

void kvt_set_scrollbar(int b){
    kvt->setScrollbar((b!=0));
}

void kvt_set_size_increment(int dx, int dy){
    kvt->setSizeIncrement(QSize(dx, dy));
    kvt->setMinimumSize(160,100);
}

class MyApp:public KApplication {
public:
  MyApp( int &argc, char **argv, const QString& rAppName );
  virtual bool x11EventFilter( XEvent * );
};

MyApp::MyApp(int &argc, char **argv , const QString& rAppName):
  KApplication(argc, argv, rAppName){
}

static MyApp* myapp = NULL;

bool MyApp::x11EventFilter( XEvent * ev){
//   printf("Qt: got one event %d for window %d\n", ev->type, ev->xany.window);

  static Bool motion_allowed = FALSE ;

  if (KApplication::x11EventFilter(ev))
    return TRUE;

  if (ev->xany.type == KeyPress || ev->xany.type == KeyRelease){
    stored_xevent_for_keys.xkey = ev->xkey;
    return False;
  }

   // send all focus events to the rxvt
  if (ev->xany.type == FocusIn || ev->xany.type == FocusOut){
    if (ev->xany.window == kvt->winId())
      {
	handle_X_event(*ev, 0);
	screen_refresh();
      }
    return FALSE;
  }

  // this hack is because the mentioned windows aren't in any widget.
  // setting the window back to main_win (a Qt window) is 
  // necessary for Qt-grabbing when using the menubar. TODO. Matthias
  if ( ev->xany.window == main_win ||
       ev->xany.window == vt_win){

    if (ev->xany.type != MotionNotify || motion_allowed){
      handle_X_event(*ev, 0);
      screen_refresh();
    }
    if (ev->xany.type == ButtonPress
	&& ev->xbutton.button == Button1)
      motion_allowed = TRUE;
    
    if (ev->xany.type == ButtonRelease
	&& ev->xbutton.button == Button1)
      motion_allowed = FALSE;

    ev->xany.window = main_win;
  }

  return FALSE;
}

OptionDialog::OptionDialog(QWidget *parent, const char *name)
  : QDialog( parent, name, TRUE )
{
  setCaption(name);
  QLabel *label_color, *label_class, *label_backspace;
  label_color = new QLabel(klocale->translate("choose type of color-mode"), this);
  colormode = new QComboBox(this);

  label_class = new QLabel(klocale->translate("Add characters to word class"), this);
  chars = new QLineEdit(this);

  label_backspace = new QLabel(klocale->translate("The backspace key will send a"), this);
  backspace = new QComboBox(this);
  
  QPushButton *ok, *cancel;
  ok = new QPushButton( klocale->translate("Ok"), this );
  connect( ok, SIGNAL(clicked()), SLOT(accept()) );
  cancel = new QPushButton( klocale->translate("Cancel"), this );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

  QBoxLayout *geom1, *geom3;
  QGridLayout *geom2;
  geom1 = new QBoxLayout(this, QBoxLayout::TopToBottom, 4);
  geom2 = new QGridLayout(3, 2); 
  geom1->addLayout(geom2, 3);
  geom2->setColStretch(0,2);
  geom2->setColStretch(1,1);
  geom2->addWidget(label_color, 0, 0);
  geom2->addWidget(colormode, 0, 1);
  geom2->addWidget(label_class, 1, 0);
  geom2->addWidget(chars, 1, 1);
  geom2->addWidget(label_backspace, 2, 0);
  geom2->addWidget(backspace, 2, 1);
  geom3 = new QBoxLayout(QBoxLayout::LeftToRight, 4);
  geom1->addLayout(geom3, 1);
  geom3->addStretch();
  geom3->addWidget(ok);
  ok->setDefault(true);
  geom3->addStretch();
  geom3->addWidget(cancel);
  geom3->addStretch();
  resize(400, 120);

  int i;
  for (i=0; color_mode_name[i]; i++) {
    colormode->insertItem(color_mode_name[i], i);
  }
  for (i=0; backspace_name[i]; i++) {
    backspace->insertItem(backspace_name[i], i);
  }
}

void sbar_init(void){
  sbar_show(100,0,100);
}

void sbar_show(int length_arg,int low_arg,int high_arg){
  if (length != length_arg ||
      low != low_arg ||
      high != high_arg){
    length = length_arg;
    low = low_arg;
    high = high_arg;
    kvt->scrollbar->setRange(0, length - (high - low));
    kvt->scrollbar->setSteps(1, high-low);
    kvt->scrollbar->setValue(length - high);
  }
}

void change_window_name(char *str){
  kvt->setCaption(str);
}

void change_icon_name(char *str){
  kvt->setIconText(str);
}

//---------------------------------------------------------------------------
// kVt
//---------------------------------------------------------------------------

kVt::kVt( KConfig* sessionconfig, QWidget *parent, const char *name )
  : QWidget( parent, name){

    // set the default options
    menubar_visible = TRUE;
    scrollbar_visible = TRUE;
    kvt_scrollbar = kvt_right;
    kvt_size = kvt_normal;
    setting_to_vt_window = FALSE;

    // get the configutation
    kvtconfig = KApplication::getKApplication()->getConfig();
    kvtconfig->setGroup("kvt");
    sessionconfig->setGroup("kvt");

    // Init DnD: Set up drop zone and drop handler
    dropZone = new KDNDDropZone( this, DndURL );
    connect( dropZone, SIGNAL( dropAction( KDNDDropZone* )), 
	     SLOT( onDrop( KDNDDropZone*)));
    

    QString entry;
    entry = sessionconfig->readEntry("menubar");
    if (!entry.isEmpty() && entry == "visible" )
      menubar_visible = TRUE;
    if (!entry.isEmpty() && entry == "invisible" )
      menubar_visible = FALSE;
    entry = sessionconfig->readEntry("scrollbar");
    if (!entry.isEmpty() && entry == "hidden" ){
      kvt_scrollbar = kvt_right;
      scrollbar_visible = FALSE;
    }
    if (!entry.isEmpty() && entry ==  "left" ){
      scrollbar_visible = TRUE;
      kvt_scrollbar = kvt_left;
    }
    if (!entry.isEmpty() && entry == "right" ){
      scrollbar_visible = TRUE;
      kvt_scrollbar = kvt_right;
    }
    entry = sessionconfig->readEntry("size");
    if (!entry.isEmpty()){
      kvt_set_fontnum(entry.data());
      kvt_size = (KvtSize) font_num;
    }

    entry = sessionconfig->readEntry("dimension");
    if (!entry.isEmpty()) {
      kvt_set_dimension(entry.data());
    }

    fg_string  = sessionconfig->readEntry("foreground","black");
    fg_string_tmp = fg_string.data();

    bg_string = sessionconfig->readEntry("background","white");
    bg_string_tmp = bg_string.data();

    entry = sessionconfig->readEntry("charclass");
    if (!entry.isEmpty()) {
      kvt_charclass = entry;
      set_charclass(entry);
    } else {
      set_charclass("");
    }

    /* if bacspace=BS then backspace sends a ^H otherwise it will send a ^? */
    entry = sessionconfig->readEntry("backspace");
    BackspaceSendsControlH = (entry && entry=="BS");

    m_file = new QPopupMenu;
    CHECK_PTR( m_file );
    m_file->insertItem( klocale->translate("&New Terminal"));
    m_file->insertSeparator();
    m_file->insertItem( klocale->translate("E&xit") ,  this, SLOT(quit()) );
    connect(m_file, SIGNAL(activated(int)), SLOT(file_menu_activated(int)));

    m_scrollbar = new QPopupMenu;
    CHECK_PTR( m_scrollbar );
    m_scrollbar->insertItem( klocale->translate("&Hide"));
    m_scrollbar->insertItem( klocale->translate("&Left"));
    m_scrollbar->insertItem( klocale->translate("&Right"));
    connect(m_scrollbar, SIGNAL(activated(int)), SLOT(scrollbar_menu_activated(int)));

    m_size = new QPopupMenu;
    CHECK_PTR( m_size );
    m_size->insertItem( klocale->translate("&Normal"));
    m_size->insertItem( klocale->translate("&Tiny"));
    m_size->insertItem( klocale->translate("&Small"));
    m_size->insertItem( klocale->translate("&Medium"));
    m_size->insertItem( klocale->translate("&Large"));
    m_size->insertItem( klocale->translate("&Huge"));
    connect(m_size, SIGNAL(activated(int)), SLOT(size_menu_activated(int)));

    m_dimen = new QPopupMenu;
    CHECK_PTR( m_dimen );
    for (int i=0; kvt_dimens[i].text; i++) {
      m_dimen->insertItem(kvt_dimens[i].text);
    }
    connect(m_dimen, SIGNAL(activated(int)), SLOT(dimen_menu_activated(int)));
    
    m_color = new QPopupMenu;
    CHECK_PTR( m_color );
    m_color->insertItem( klocale->translate("&black/white"));
    m_color->insertItem( klocale->translate("&white/black"));
    m_color->insertItem( klocale->translate("&green/black"));
    m_color->insertItem( klocale->translate("black/light&yellow"));
    m_color->insertItem( klocale->translate("Linu&x Console"));

    colors =  	new QPopupMenu ();
    CHECK_PTR( colors );
    colors->insertItem(klocale->translate("&Foreground Color"),
		       this, SLOT(select_foreground_color()));
    colors->insertItem(klocale->translate("&Background Color"),
		       this, SLOT(select_background_color()));
    m_color->insertSeparator();
    m_color->insertItem(klocale->translate("&Custom Colors"),colors);
    connect(m_color, SIGNAL(activated(int)), SLOT(color_menu_activated(int)));
    
    
    m_options = new QPopupMenu;
    CHECK_PTR( m_options );
    if (menubar_visible)
      m_options->insertItem( klocale->translate("Hide &Menubar") );
    else
      m_options->insertItem( klocale->translate("Show &Menubar") );
    m_options->insertItem( klocale->translate("Secure &keyboard"));
    m_options->insertSeparator();
    m_options->insertItem( klocale->translate("Scroll&bar"), m_scrollbar);
    m_options->insertItem( klocale->translate("&Font size"), m_size);
    m_options->insertItem( klocale->translate("&Color"), m_color);
    m_options->insertItem( klocale->translate("&Size"), m_dimen);
    m_options->insertItem( klocale->translate("Terminal...") );
    m_options->insertSeparator();
    m_options->insertItem( klocale->translate("Save &Options"));

    m_options->installEventFilter( this );

    connect(m_options, SIGNAL(activated(int)), 
	    SLOT(options_menu_activated(int)));


    QString at = KVT_VERSION;
    at += klocale->translate("\n\n(C) 1996, 1997 Matthias Ettrich (ettrich@kde.org)\n(C) 1997 M G Berberich (berberic@fmi.uni-passau.de)\n\nTerminal emulation for the KDE Desktop Environment\nbased on Robert Nation's rxvt-2.08");
		       
    m_help = kapp->getHelpMenu(true, at.data());

    menubar = new KMenuBar( this );
    CHECK_PTR( menubar );

    entry = sessionconfig->readEntry("kmenubar");
    if (!entry.isEmpty() && entry == "floating") {
      menubar->setMenuBarPos(KMenuBar::Floating);
      entry = sessionconfig->readEntry("kmenubargeometry");
      if (!entry.isEmpty()) {
	menubar->setGeometry(KWM::setProperties(menubar->winId(), entry));
      }
      menubar->show();
    }
    else if (!entry.isEmpty() && entry == "top") {
      menubar->setMenuBarPos(KMenuBar::Top);
    }
    else if (!entry.isEmpty() && entry == "bottom") {
      menubar->setMenuBarPos(KMenuBar::Bottom);
    }


    connect(menubar, SIGNAL (moved(menuPosition)),
	    SLOT (menubarMoved()));
    menubar->insertItem( klocale->translate("&File"), m_file );
    menubar->insertItem( klocale->translate("&Options"), m_options);
    menubar->insertSeparator();
    menubar->insertItem( klocale->translate("&Help"), m_help);

    if (!menubar_visible)
      menubar->hide();
    
    frame = new QFrame( this );
	if( style() == WindowsStyle )
      frame ->setFrameStyle( QFrame::WinPanel | QFrame::Sunken);
	else
	  frame ->setFrameStyle( QFrame::Panel | QFrame::Sunken);

    scrollbar = new QScrollBar(QScrollBar::Vertical, this );
    connect( scrollbar, SIGNAL(valueChanged(int)), SLOT(scrolling(int)) );

    if (!scrollbar_visible)
      scrollbar->hide();

    rxvt = new QWidget( this );
    main_win = rxvt->winId();
    rxvt->installEventFilter( this );

    installEventFilter( this );

    keyboard_secured = FALSE;
    
    setAcceptFocus( TRUE );
    
    KApplication::getKApplication()->setTopWidget(this);
    KApplication::getKApplication()->enableSessionManagement();
    KWM::setUnsavedDataHint(winId(), True);
    connect(KApplication::getKApplication(), SIGNAL(saveYourself()),
	    SLOT(saveYourself()));

    entry = sessionconfig->readEntry("geometry");
    if (!entry.isEmpty()) {
      setGeometry(KWM::setProperties(winId(), entry));
    }

}

void kVt::styleChange( GUIStyle ) {
  if( style() == WindowsStyle )
    frame ->setFrameStyle( QFrame::WinPanel | QFrame::Sunken);
  else
    frame ->setFrameStyle( QFrame::Panel | QFrame::Sunken); 
} 

void kVt::saveYourself(){
  KConfig* config = KApplication::getKApplication()->getSessionConfig();
  saveOptions(config);
  config->writeEntry("geometry", KWM::getProperties(winId()));
  if (menubar->menuBarPos() == KMenuBar::Floating){
    config->writeEntry("kmenubar", "floating");
    config->writeEntry("kmenubargeometry", 
			  KWM::getProperties(menubar->winId()));
  }
  config->sync();
}


void kVt::saveOptions(KConfig* kvtconfig){
  kvtconfig->setGroup("kvt");
  if (menubar_visible)
    kvtconfig->writeEntry("menubar", "visible");
  else
    kvtconfig->writeEntry("menubar", "invisible");
  
  if (scrollbar_visible){
    if (kvt_scrollbar == kvt_left)
      kvtconfig->writeEntry("scrollbar", "left");
    else
      kvtconfig->writeEntry("scrollbar", "right");
  }
  else
    kvtconfig->writeEntry("scrollbar", "hidden");
  
  kvtconfig->writeEntry("size", kvt_sizes[kvt_size]);
  
  kvtconfig->writeEntry("dimension", kvt_dimens[kvt_dimen].text);
  
  kvtconfig->writeEntry("foreground", fg_string);
  kvtconfig->writeEntry("background", bg_string);
  
  kvtconfig->writeEntry("charclass", kvt_charclass);
  
  kvtconfig->writeEntry("colormode", color_mode_name[get_color_mode()]);
  
  kvtconfig->writeEntry("backspace", BackspaceSendsControlH ? "BS" : "DEL");

  if (menubar->menuBarPos() == KMenuBar::Bottom)
    kvtconfig->writeEntry("kmenubar", "bottom");
  else
    kvtconfig->writeEntry("kmenubar", "top");
  kvtconfig->writeEntry("command_line", command_line);

  kvtconfig->sync();
}

void kVt::do_some_stuff(KConfig* kvtconfig) { //temporary (Matthias)
    QString entry;
    entry = kvtconfig->readEntry("colormode");
     if (!entry.isEmpty()) {
       if (entry == color_mode_name[COLOR_TYPE_ANSI])
 	init_color_mode(COLOR_TYPE_ANSI);
       if (entry == color_mode_name[COLOR_TYPE_Linux])
 	init_color_mode(COLOR_TYPE_Linux);      
       if (entry == color_mode_name[COLOR_TYPE_DEFAULT])
 	init_color_mode(COLOR_TYPE_DEFAULT);      
     } else {
       init_color_mode(COLOR_TYPE_DEFAULT);
     }
}

// only works for hiding!!
void kVt::setMenubar(bool b){
  if (!b){
    menubar->hide();
    m_options->changeItem(klocale->translate("Show &Menubar"), 0);
  }
  menubar_visible = b;
}

// only works for hiding!!
void kVt::setScrollbar(bool b){
  if (!b){
    scrollbar->hide();
    m_scrollbar->changeItem(klocale->translate("&Show"), 0);
    scrollbar_visible = b;
  }
}

void kVt::application_signal(){
  get_token();
}

void kVt::ResizeToDimen(int width, int height)
{
  MyWinInfo.cwidth = sizehints.width = width;
  MyWinInfo.cheight = sizehints.height = height;
  MyWinInfo.pwidth = MyWinInfo.cwidth*MyWinInfo.fwidth;
  MyWinInfo.pheight = MyWinInfo.cheight*MyWinInfo.fheight;
  sizehints.width = sizehints.width*sizehints.width_inc+sizehints.base_width;
  sizehints.height = sizehints.height*sizehints.height_inc+
  sizehints.base_height;
  scrn_reset();
  kvt->ResizeToVtWindow();
}

void kVt::ResizeToVtWindow(){
  setting_to_vt_window = TRUE;
  int menubar_height = menubar->height();
  if (menubar_visible){
    if (scrollbar_visible){
      resize(sizehints.width+4+16,
	     sizehints.height+4+menubar_height);
    }
    else
      resize(sizehints.width+4,
	     sizehints.height+4+menubar_height);
  }
  else{
    if (scrollbar_visible)
      resize(sizehints.width+4+16,
	     sizehints.height+4);
    else
      resize(sizehints.width+4,
	     sizehints.height+4);
  }
   setting_to_vt_window = FALSE;
}


void kVt::resizeEvent( QResizeEvent * ev)
{
  if (menubar_visible && menubar->menuBarPos() == KMenuBar::Top){
    menubar->setGeometry(0,0,width(), menubar->height());
    frame->setGeometry(0, menubar->height(), width(), height()-menubar->height());
  }
  else if (menubar_visible && menubar->menuBarPos() == KMenuBar::Bottom){
    menubar->setGeometry(0,height()-menubar->height(),width(), menubar->height());
    frame->setGeometry(0, 0, width(), height()-menubar->height());
  }
  else{ // menubar not visible or floating
    menubar->resize(width(), menubar->height());
    frame->setGeometry(0, 0, width(), height());
  }

  // a hack 
  //    if (setting_to_vt_window){
  //      if (frame->height()-4 != sizehints.height){
  //        resize(width(), height() + sizehints.height - frame->height() + 4);
  //        resize(width(), height() + sizehints.height - frame->height() + 4);
  //        return;
  //      }
  //    }
    
	int frameWidth = frame->lineWidth();
  
   if (scrollbar_visible) {
     switch (kvt_scrollbar){
     case kvt_right:
       rxvt->setGeometry(frame->x()+frameWidth, frame->y()+frameWidth,
			 frame->width()-2*frameWidth - 16, frame->height()-2*frameWidth);
       scrollbar->setGeometry(rxvt->x() + rxvt->width(),
			      rxvt->y(),
			      16,
			      rxvt->height());
       break;
     case kvt_left:
       rxvt->setGeometry(frame->x()+frameWidth + 16, frame->y()+frameWidth,
			 frame->width()-2*frameWidth - 16, frame->height()-2*frameWidth);
       scrollbar->setGeometry(frameWidth,
			      rxvt->y(),
			      16,
			      rxvt->height());
       break;
     }
   }
   else {
     rxvt->setGeometry(frame->x()+frameWidth, frame->y()+frameWidth,
		       frame->width()-2*frameWidth, frame->height()-2*frameWidth);
     scrollbar->setGeometry(rxvt->x() + rxvt->width(),
			    rxvt->y(),
			    16,
			    rxvt->height());
   }
   
   if (ev){
     // redraw all
     // XClearWindow(display,vt_win);
     //    scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);
     //     screen_refresh();
  }
}

void kVt::options_menu_activated( int item){
  switch (item){
  case 0:
    // menubar
    if (menubar->isVisible()){
      // hide 
      menubar_visible = FALSE;
      menubar->hide();
      if (menubar->menuBarPos() != KMenuBar::Floating)
	resize(width(), height()-menubar->height());
      m_options->changeItem(klocale->translate("Show &Menubar"), item);
    }
    else {
      // show 
      menubar_visible = TRUE;
      menubar->show();
      if (menubar->menuBarPos() != KMenuBar::Floating)
	resize(width(), height()+menubar->height());
      m_options->changeItem(klocale->translate("Hide &Menubar"), item);
    }
    break;
  case 1:
    keyboard_secured = !keyboard_secured;
    if (keyboard_secured){
      m_options->changeItem(klocale->translate("Unsecure &keyboard"), item);
      extract_colors(fg_string.data(), "red");
    }
    else {
      m_options->changeItem(klocale->translate("Secure &keyboard"), item);
      extract_colors(fg_string.data(), bg_string.data());
    }
    scr_secure(); // also calls XClearwindow and scr_refresh
    break;

  case 7:
    m_optiondialog = new OptionDialog(this, klocale->translate("Terminal Options"));
    m_optiondialog->colormode->setCurrentItem(get_color_mode());
    m_optiondialog->chars->setText(kvt_charclass);
    m_optiondialog->backspace->setCurrentItem(BackspaceSendsControlH);
    if (m_optiondialog->exec()) {
      set_color_mode(m_optiondialog->colormode->currentItem());
      kvt_charclass = m_optiondialog->chars->text();
      set_charclass(kvt_charclass);
      scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);
      BackspaceSendsControlH = m_optiondialog->backspace->currentItem();
    }
    delete m_optiondialog;
    break;
    
  case 9:
    // save options
    {
      saveOptions(kvtconfig);
    }
  }
}

void kVt::scrollbar_menu_activated( int item){
  switch (item){
  case 0: 
    if (scrollbar->isVisible()){
      // hide
      scrollbar_visible = FALSE;
      scrollbar->hide();
      resize(width()-16, height());
      m_scrollbar->changeItem(klocale->translate("&Show"), item);
    }
    else {
      // show 
      scrollbar_visible = TRUE;
      scrollbar->show();
      resize(width()+16, height());
      m_scrollbar->changeItem(klocale->translate("&Hide"), item);
    }
    break;
    
  case 1: // left
      kvt_scrollbar = kvt_left;
      if (!scrollbar->isVisible()){
	scrollbar_menu_activated(0);
	break;
      }
      resize(width(), height());
      break;
      
  case 2: // right
    kvt_scrollbar = kvt_right;
    if (!scrollbar->isVisible()){
      scrollbar_menu_activated(0);
      break;
    }
    resize(width(), height());
    break;
  }
}

void kVt::size_menu_activated( int item){
  if ( (KvtSize)item == kvt_size)
    return;
  font_num = item;
  kvt_size = (KvtSize) item;
  LoadNewFont();
  ResizeToVtWindow();
}

void kVt::dimen_menu_activated( int item){
  if ( item == kvt_dimen)
    return;
  kvt_dimen = item;
  kvt->ResizeToDimen(kvt_dimens[kvt_dimen].x, kvt_dimens[kvt_dimen].y);
}

void kVt::select_foreground_color(){

  QColor color(fg_string.data());
  
  if(KColorDialog::getColor(color) != QDialog::Accepted)
    return;
  
  fg_string.sprintf("#%02x%02x%02x",color.red(),color.green(),color.blue());

  //  printf("%s %s\n",fg_string.data(),bg_string.data());
  extract_colors(fg_string.data(), bg_string.data());

  // redraw all
  XClearWindow(display,vt_win);
  scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);
}


void kVt::select_background_color(){

  QColor color(bg_string.data());
  
  if(KColorDialog::getColor(color) != QDialog::Accepted)
    return;
  
  bg_string.sprintf("#%02x%02x%02x",color.red(),color.green(),color.blue());

  //  printf("%s %s\n",fg_string.data(),bg_string.data());
  extract_colors(fg_string.data(), bg_string.data());

  // redraw all
  XClearWindow(display,vt_win);
  scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);

}

void kVt::color_menu_activated( int item){
  switch (item){
  case 0: 
    fg_string = "black";
    bg_string = "white";
    break;
  case 1: 
    fg_string = "white";
    bg_string = "black";
    break;
  case 2: 
    fg_string = "green";
    bg_string = "black";
    break;
  case 3: 
    fg_string = "black";
    bg_string = "#FFFFDD";
    break;
  case 4:
    fg_string = "#b2b2b2";
    bg_string = "#000000";
    break;
  }
  if (!keyboard_secured){
    extract_colors(fg_string.data(), bg_string.data());
    // redraw all
    XClearWindow(display,vt_win);
    scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);
  }else{
    QMessageBox::message( klocale->translate("Hint"), 
			  klocale->translate("New color settings will be displayed\nwhen the keyboard is unsecured.") );
  }
}


void kVt::file_menu_activated(int item){
  switch (item){
  case 0: 
     if (fork()==0){
       execvp(o_argv[0], o_argv);
       exit(1);
     }
     //     signal(SIGCHLD,SIG_DFL);
  }
}


bool kVt::eventFilter( QObject *obj, QEvent * ev){
  static QPoint tmp_point(-10,-10);
  if (obj == m_options){
    if (ev->type() == Event_MouseButtonRelease){
      if (QCursor::pos() == tmp_point){
	tmp_point = QPoint(-10,-10);
	return TRUE;
      }
    }
  }
  if (obj == rxvt){
    switch (ev->type()){
    case Event_MouseButtonPress: case Event_MouseButtonDblClick:
      {
	QMouseEvent *e = (QMouseEvent*) ev;
	tmp_point = QCursor::pos();
	if (e->button() == RightButton){
	  m_options->popup(tmp_point);
	}
      }
      break;
    case Event_MouseButtonRelease: 
      tmp_point = QPoint(-10,-10);
      break;
    }
  }
  
  if (obj == this){
    switch (ev->type()){
    case Event_KeyPress: case Event_KeyRelease:
      {
	// well... aehh... but this works ;-)
	handle_X_event(stored_xevent_for_keys, ((QKeyEvent*)ev)->ascii());
      }
    }
  }
  
  return FALSE;
}

void kVt::scrolling( int value){
  MyWinInfo.offset =  length - value - (high - low);
  screen_refresh();
}

void kVt::onDrop( KDNDDropZone* _zone )
{
  QStrList strlist;
  KURL *url;
  QString str = "";
  int file_count = 0;
  char *p;

  strlist = _zone->getURLList();
  if (strlist.count()){
    p = strlist.first();
    while(p != 0) {
      url = new KURL( p );
      if(file_count++ > 0)
	str += " ";
      str += url->path();
      delete url;
      p = strlist.next();
    }
    send_string((unsigned char *)str.data(), str.length());
  }
}

void kVt::menubarMoved(){
  int new_pos = menubar->menuBarPos();
  if (new_pos == KMenuBar::Top){
    if (frame->height() == height())
      resize(width(), height()+menubar->height());
    else
      resize(width(), height());
  }
  else if (new_pos == KMenuBar::Bottom){
    if (frame->height() == height())
      resize(width(), height()+menubar->height());
    else
      resize(width(), height());
  }
  else if (new_pos == KMenuBar::Floating){
    if (frame->height() != height())
      resize(width(), height()-menubar->height());
  }
}

void kVt::quit(){
  delete menubar;
  myapp->quit();
}

//---------------------------------------------------------------------------
// main
//---------------------------------------------------------------------------

#include "main.moc"

int main(int argc, char **argv)
{
  setlocale(LC_CTYPE, "");
  
  // first make an argument copy for a new kvt
  o_argc = argc;
  o_argv = new char*[o_argc + 2];
  int i,i2;
  i2 = 0;
  for (i=0; i<o_argc; i++) {
    if (QString("-restore")==argv[i])
      i++;
    else {
      o_argv[i2] = argv[i];
      i2++;
    }
  }
  o_argv[i2]=NULL;

  // replace "-caption" with "-T" 
  for (i=0; i<argc; i++) {
    if (QString("-caption")==argv[i]){
      argv[i][1]='T';
      argv[i][2]='\0';
    }
  }
  


  // cut off the command arguments for the terminal command and set com_arg
  int commands = -1;
  int com_argc = -1;
  char **com_argv = 0;
  for (i=0; i<argc; i++){
    if (strcmp(argv[i],"-e") == 0){
      com_argv = argv+i+1;
      com_argc = argc-i-1;
      argc = i;
      commands = i;
      argv[i] = NULL;
    }
  }
  for(i=1; i<argc; i++) {
    strcat(command_line," ");
    strcat(command_line,argv[i]);
  }
  if(com_argc > 0) {
    strcat(command_line," -e");
    for(i=0; i<com_argc; i++) {
      strcat(command_line," ");
      strcat(command_line,com_argv[i]);
    }
  }
  // create the QT Application
  MyApp a( argc, argv, "kvt" );
  myapp = &a;

  // this is for the original rxvt-code
  display = qt_xdisplay();

  //  a.setStyle(WindowsStyle);


  KConfig* sessionconfig = a.getConfig();
  if (a.isRestored()){
    sessionconfig = a.getSessionConfig();
  // restore all command line arguments like "-sl 1000 -caption "Hallo World" -e ...." 
    {
      QString entry;
      sessionconfig->setGroup("kvt");
      entry = sessionconfig->readEntry("command_line");
     
      char *sp;
      char *spe;
      char exec_s[2048];
      
      int exec_found = 0;
      int string_argument = 0;
      
      argc = 0;
      argv = new char*[100];
      argv[argc] = NULL;

      if(!entry.isEmpty())
        strcpy(command_line, (const char*)entry);
	
      strcpy(exec_s, command_line);
      sp = exec_s;
                 
      while(*sp && !exec_found) 
      {
        while(*sp && *sp == ' ') 
          sp++;
        if(*sp && *sp == '"') {
          string_argument = 1;
          sp++;
        }
        spe = sp;

        if(string_argument) {
          while(*spe && *spe != '"') spe++;
          if(*spe == '"')
            *spe = ' ';
          else {
            fprintf(stderr, "Unmatched \" in command line\n%s\n  ...abort\n", command_line);
            exit(-1);
          }
          string_argument = 0;
        }
        else
          while(*spe && *spe != ' ') spe++;

        if(spe > sp )
        {
          if(*spe == ' ')
            *spe++ = 0;
          if(!strcmp(sp, "-e"))
            exec_found = 1; 
          else
          {               
            argv[argc++]=strdup(sp);
            argv[argc] = NULL;
          }
          sp = spe; 
        }
      }    
        
      // execute ? -------------------------------------

      if(exec_found)
      {       
        com_argv = new char*[100];
        com_argc = 0;
        com_argv[com_argc] = NULL;
        while(*sp) 
        {          
          while(*sp && *sp == ' ') 
            sp++;
          if(*sp && *sp == '"') {
            string_argument = 1;
            sp++;
          }
          spe = sp;

          if(string_argument) {
            while(*spe && *spe != '"') spe++;
            if(*spe == '"')
              *spe = ' ';
            else {
              fprintf(stderr, "Unmatched \" in command line\n%s\n  ...abort\n", command_line);
              exit(-1);
            }
            string_argument = 0;
          }
          else
            while(*spe && *spe != ' ') spe++;

          if(spe > sp )
          {
            if(*spe == ' ')
              *spe++ = 0;
            com_argv[com_argc++]=strdup(sp); 
            sp = spe;
          }
          com_argv[com_argc]=NULL;
        }
        if(!com_argc)
          delete com_argv;
      }
    }
  }

  kvt = new kVt(sessionconfig);
  a.setMainWidget( kvt );
  a.setTopWidget( kvt );

  if (!a.isRestored()){
    // a bisserl gehackt. [bmg]
    char buffer[60];
    sprintf(buffer, "%dx%d", kvt_dimens[kvt_dimen].x, kvt_dimens[kvt_dimen].y);
    set_geom_string(buffer);
    kvt_set_dimension(buffer);
  }
  
  // set the names
  xvt_name = window_name = icon_name = (char*) a.getCaption(); 
  
  char* s;
  if (com_argv){
    if ((s=strrchr(com_argv[0],'/'))!=NULL) 
      s++; 
    else 
      s=com_argv[0]; 
    window_name = icon_name = s; 
  }

  if (!init_display(argc, argv)) {
    fprintf(stderr, KVT_VERSION);
    fprintf(stderr, "\n\n");
    fprintf(stderr, klocale->translate(
	    "Copyright(C) 1996, 1997 Matthias Ettrich\n"
	    "Copyright(C) 1997 M G Berberich\n"
	    "Terminal emulation for the KDE Desktop Environment\n"
	    "based on Robert Nation's rxvt-2.08\n\n"
	    "Permitted arguments are:\n"
	    "-e <command> <arg> ...	execute command with ars - must be last argument\n"
	    "-display <name>	specify the display (server)\n"
	    "-vt_geometry <spec>	the initial vt window geometry\n"
	    "-print-pipe <name>	specify pipe for vt100 printer\n"
	    "-vt_bg <colour>		background color\n"
	    "-vt_fg <colour>		foreground color\n"
	    "-vt_font <fontname>	normal font\n"
	    "-vt_size <size>     	tiny, small, normal, large, huge\n"
	    "-linux			set up kvt to mimic linux-console\n"
	    "-no_menubar		hide the menubar\n"
	    "-no_scrollbar		hide the scrollbar\n"
	    "-T <text>		text in window titlebar \n"
	    "                   (Obsolete. Use -caption instead)\n"
	    "-tn <TERM>		Terminal type. Default is xterm.\n"
	    "-C			Capture system console message\n"
	    "-n <text>		name in icon or icon window\n"
	    "-7		        run in 7 bit mode\n"
	    "-8			run in 8 bit mode\n"
	    "-ls			initiate kvt's shell as a login shell\n"
	    "-ls-			initiate kvt's shell as a non-login shell (default)\n"
	    "-meta			handle Meta with ESCAPE prefix, 8THBIT set, or ignore\n"
	    "-sl <number>		save number lines in scroll-back buffer\n"
	    "-pageup <keysym>	use hot key alt-keysym to scroll up through the buffer\n"
	    "-pagedown <keysym>	use hot key alt-keysym to scroll down through buffer\n"));
#ifdef GREEK_KBD
    fprintf(stderr, klocale->translate(
	    "-grk9		greek kbd = ELOT 928 (default)\n"
	    "-grk4		greek kbd = IBM 437\n"));
#endif
    clean_exit(1);
  }

  kvt->do_some_stuff(sessionconfig);

  init_command(NULL ,(unsigned char **)com_argv);

  QSocketNotifier sn( comm_fd, QSocketNotifier::Read );
  QObject::connect( &sn, SIGNAL(activated(int)),
		    kvt, SLOT(application_signal()) );

  if (!a.isRestored())
    kvt->ResizeToVtWindow();

  kvt->show();

  return a.exec();
}
