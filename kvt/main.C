//
// kvt. Part of the KDE project.
//
// Copyright (C) 1996 Matthias Ettrich
//

#define STDC_HEADERS
#include <qapp.h>
#include <qpushbt.h>
#include <qbitmap.h>
#include <qfiledlg.h>
#include <qwindefs.h>
#include <qsocknot.h>
#include <qmsgbox.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// KDE includes
#include <Kconfig.h>
#include <kapp.h>


#include "main.h"

extern "C" {
extern void get_token();
extern void handle_X_event(XEvent event);
extern void rxvt_main(int argc,char **argv);
extern void refresh();
extern void extract_colors( char *fg_string, char *bg_string);

int run_command(unsigned char *,unsigned char **);

#include "rxvt.h"
#include "sbar.h"
#include "screen.h"
#include "xsetup.h"

extern struct sbar_info sbar;
extern WindowInfo MyWinInfo;
extern XSizeHints sizehints;

extern int font_num;
extern void LoadNewFont();

extern char* bg_string;
extern char* fg_string;

extern int rstyle;

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


kVt* kvt = NULL;

// the scrollbar hack
static int length = 0;
static int low = 0;
static int high = 0;

static XEvent stored_xevent_for_keys;

static const char* kvt_sizes[5] = {"tiny", "small", "medium", "large", "huge"};



int o_argc;
char ** o_argv;

void kvt_set_fontnum(char *s_arg){
  int i;
  QString s = s_arg;
  for (i=kvt_tiny; i<=kvt_huge; i++){
    if (s == kvt_sizes[i])
      font_num = i;
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
}


class MyApp:public KApplication {
public:
  MyApp( int &argc, char **argv, const QString& rAppName );
  virtual bool x11EventFilter( XEvent * );
};

MyApp::MyApp(int &argc, char **argv , const QString& rAppName):
  KApplication(argc, argv, rAppName){
}


bool MyApp::x11EventFilter( XEvent * ev){
//   printf("Qt: got one event %d for window %d\n", ev->type, ev->xany.window);

  static Bool motion_allowed = FALSE ;

  if (ev->xany.type == KeyPress || ev->xany.type == KeyRelease){
    stored_xevent_for_keys.xkey = ev->xkey;
    return False;
  }

   // send all focus events to the rxvt
  if (ev->xany.type == FocusIn || ev->xany.type == FocusOut){
    if (ev->xany.window == kvt->winId())
      {
	handle_X_event(*ev);
	// call get_token to do a screenrefresh if there are no more
	// character in the application and X-pipe. Matthias
	get_token();
	refresh();
      }
    return FALSE;
  }

  // this hack is because the mentioned windows aren't in any widget.
  // setting the window back to main_win (a Qt window) is 
  // necessary for Qt-grabbing when using the menubar. TODO. Matthias
  if ( ev->xany.window == main_win ||
       ev->xany.window == vt_win){
    if (ev->xany.type != MotionNotify || motion_allowed){
      handle_X_event(*ev);
      // call get_token to do a screenrefresh if there are no more
      // character in the application and X-pipe. Matthias
      if (ev->xany.type != Expose )
	get_token();
      else{
	refresh();
      }
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

kVt::kVt( QWidget *parent, const char *name )
  : QWidget( parent, name){
    int i;

    // set the default options
    menubar_visible = TRUE;
    scrollbar_visible = TRUE;
    kvt_scrollbar = kvt_right;
    kvt_size = kvt_medium;
    setting_to_vt_window = FALSE;

    // get the configutation
    kvtconfig = KApplication::getKApplication()->getConfig();
    kvtconfig->setGroup("kvt");

    QString entry;
    entry = kvtconfig->readEntry("menubar");
    if (entry && entry == "visible" )
      menubar_visible = TRUE;
    if (entry && entry == "invisible" )
      menubar_visible = FALSE;
    entry = kvtconfig->readEntry("scrollbar");
    if (entry && entry == "hidden" ){
      kvt_scrollbar = kvt_right;
      scrollbar_visible = FALSE;
    }
    if (entry && entry ==  "left" ){
      scrollbar_visible = TRUE;
      kvt_scrollbar = kvt_left;
    }
    if (entry && entry == "right" ){
      scrollbar_visible = TRUE;
      kvt_scrollbar = kvt_right;
    }
    entry = kvtconfig->readEntry("size");
    if (entry){
      kvt_set_fontnum(entry.data());
      kvt_size = (KvtSize) font_num;
    }

    entry = kvtconfig->readEntry("foreground");
    if (entry)
      fg_string = qstrdup(entry);

    entry = kvtconfig->readEntry("background");
    if (entry)
      bg_string = qstrdup(entry);


    m_file = new QPopupMenu;
    CHECK_PTR( m_file );
    m_file->insertItem( "New Terminal");
    m_file->insertSeparator();
    m_file->insertItem( "Exit" ,  qApp, SLOT(quit()) );
    connect(m_file, SIGNAL(activated(int)), SLOT(file_menu_activated(int)));

    m_scrollbar = new QPopupMenu;
    CHECK_PTR( m_scrollbar );
    m_scrollbar->insertItem( "Hide");
    m_scrollbar->insertItem( "Left");
    m_scrollbar->insertItem( "Right");
    connect(m_scrollbar, SIGNAL(activated(int)), SLOT(scrollbar_menu_activated(int)));

    m_size = new QPopupMenu;
    CHECK_PTR( m_size );
    m_size->insertItem( "Tiny");
    m_size->insertItem( "Small");
    m_size->insertItem( "Medium");
    m_size->insertItem( "Large");
    m_size->insertItem( "Huge");
    connect(m_size, SIGNAL(activated(int)), SLOT(size_menu_activated(int)));
    
    
    m_color = new QPopupMenu;
    CHECK_PTR( m_color );
    m_color->insertItem( "black/white");
    m_color->insertItem( "white/black");
    m_color->insertItem( "green/black");
    m_color->insertItem( "black/lightyellow");
    connect(m_color, SIGNAL(activated(int)), SLOT(color_menu_activated(int)));
    
    
    m_options = new QPopupMenu;
    CHECK_PTR( m_options );
    if (menubar_visible)
      m_options->insertItem( "Hide Menubar" );
    else
      m_options->insertItem( "Show Menubar" );
    m_options->insertItem( "Secure keyboard");
    m_options->insertSeparator();
    m_options->insertItem( "Scrollbar" , m_scrollbar);
    m_options->insertItem( "Size" , m_size);
    m_options->insertItem( "Color", m_color);
    m_options->insertSeparator();
    m_options->insertItem( "Save Options");

    m_options->installEventFilter( this );

    connect(m_options, SIGNAL(activated(int)), SLOT(options_menu_activated(int)));


    m_help = new QPopupMenu;
    CHECK_PTR( m_help );
    m_help->insertItem("About kvt");
    m_help->insertItem("Help");
    connect(m_help, SIGNAL(activated(int)), SLOT(help_menu_activated(int)));

    menubar = new QMenuBar( this );
    CHECK_PTR( menubar );
    menubar->insertItem( "File", m_file );
    menubar->insertItem( "Options", m_options);
    menubar->insertSeparator();
    menubar->insertItem( "Help", m_help);

    if (!menubar_visible)
      menubar->hide();
    

    frame = new QFrame( this );
    frame ->setFrameStyle( QFrame::WinPanel | QFrame::Sunken);

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
}

// only works for hiding!!
void kVt::setMenubar(bool b){
  if (!b){
    menubar->hide();
    m_options->changeItem("Show Menubar", 0);
  }
  menubar_visible = b;
}

// only works for hiding!!
void kVt::setScrollbar(bool b){
  if (!b){
    scrollbar->hide();
    m_scrollbar->changeItem("Show", 0);
    scrollbar_visible = b;
  }
}

void kVt::application_signal(){
  get_token();
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

  if (menubar_visible){
    frame->setGeometry(0, menubar->height(), width(), height()-menubar->height());
  }
  else{
    frame->setGeometry(0, 0, width(), height());
    menubar->setGeometry(0,0,0,0);
  }

   // a hack 
   if (setting_to_vt_window){
     if (frame->height()-4 != sizehints.height){
       resize(width(), height() + sizehints.height - frame->height() + 4);
       resize(width(), height() + sizehints.height - frame->height() + 4);
       return;
     }
   }

   if (scrollbar_visible) {
     switch (kvt_scrollbar){
     case kvt_right:
       rxvt->setGeometry(frame->x()+2, frame->y()+2,
			 frame->width()-4 - 16, frame->height()-4);
       scrollbar->setGeometry(rxvt->x() + rxvt->width(),
			      rxvt->y(),
			      16,
			      rxvt->height());
       break;
     case kvt_left:
       rxvt->setGeometry(frame->x()+2 + 16, frame->y()+2,
			 frame->width()-4 - 16, frame->height()-4);
       scrollbar->setGeometry(2,
			      rxvt->y(),
			      16,
			      rxvt->height());
       break;
     }
   }
   else {
     rxvt->setGeometry(frame->x()+2, frame->y()+2,
		       frame->width()-4, frame->height()-4);
     scrollbar->setGeometry(rxvt->x() + rxvt->width(),
			    rxvt->y(),
			    16,
			    rxvt->height());
   }
   
   if (ev){
    // redraw all
     // XClearWindow(display,vt_win);
  //    scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);
     //     refresh();
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
      resize(width(), height()-menubar->height());
      m_options->changeItem("Show Menubar", item);
    }
    else {
      // show 
      menubar_visible = TRUE;
      menubar->show();
      resize(width(), height()+menubar->height());
      m_options->changeItem("Hide Menubar", item);
    }
    break;
  case 1:
    keyboard_secured = !keyboard_secured;
    if (keyboard_secured){
      m_options->changeItem("Unsecure keyboard", item);
      extract_colors(fg_string, "red");
    }
    else {
      m_options->changeItem("Secure keyboard", item);
      extract_colors(fg_string, bg_string);
    }
    scr_secure(); // also calls XClearwindow and scr_refresh
    break;
    
  case 7:
    // save options
    {
      if (menubar_visible)
	kvtconfig->writeEntry("menubar", QString("visible"));
      else
	kvtconfig->writeEntry("menubar", QString("invisible"));

      if (scrollbar_visible){
	if (kvt_scrollbar == kvt_left)
	  kvtconfig->writeEntry("scrollbar", QString("left"));
	else
	  kvtconfig->writeEntry("scrollbar", QString("right"));
      }
      else
	kvtconfig->writeEntry("scrollbar", QString("hidden"));

      kvtconfig->writeEntry("size", QString(kvt_sizes[kvt_size]));

      kvtconfig->writeEntry("foreground", QString(fg_string));
      kvtconfig->writeEntry("background", QString(bg_string));
      
      kvtconfig->sync();
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
      m_scrollbar->changeItem("Show", item);
    }
    else {
      // show 
      scrollbar_visible = TRUE;
      scrollbar->show();
      resize(width()+16, height());
      m_scrollbar->changeItem("Hide", item);
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
    bg_string = "lightyellow";
    break;
  }
  if (!keyboard_secured){
    extract_colors(fg_string, bg_string);
    rstyle = 0;
    // redraw all
    XClearWindow(display,vt_win);
    scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);
  }else{
    QMessageBox::message( "Hint", "New color settings will be displayed \n when the keyboard is unsecured." );
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


void _invokeHtmlHelp(const char* filename){
  if ( fork	() == 0 )	
    {		
      QString path = "";
      char* kdedir = getenv("KDEDIR");
      if (kdedir)
	path.append(kdedir);
      else
	path.append("/usr/local/kde");
      path.append("/doc/HTML/");
      path.append(filename);
      execlp( "kdehelp", "kdehelp", path.data(), 0 );
      exit( 1 );
    }
}



void kVt::help_menu_activated(int item){
  switch (item){
  case 0:
    QMessageBox::message( "About kvt", "kvt-0.12\n\n(C) 1996, 1997 Matthias Ettrich (ettrich@kde.org)\n\nTerminal emulation for the KDE Desktop Environment\nbased on Robert Nation's rxvt-2.08");
    break;
  case 1:
    _invokeHtmlHelp("kvt.html");
    break;
  }
}

bool kVt::eventFilter( QObject *obj, QEvent * ev){
  static QPoint tmp_point;
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
	handle_X_event(stored_xevent_for_keys);
	get_token();
      }
    }
  }
  
  return FALSE;
}


#include "main.moc"


int main(int argc, char **argv){

  // first make an argument copy for a new kvt
  o_argc = argc;
  o_argv = new char*[o_argc + 2];
  int v;
  for (v=0; v<o_argc; v++) o_argv[v] = argv[v];
  o_argv[v]=NULL;

  // store the command arguments for the terminal command
  int commands=-1;
  for (v=0; v<argc; v++){
    if (strcmp(argv[v],"-e") == 0){
      argc = v;
      commands = v;
      argv[v] = NULL;
    }
    
  }

  MyApp a( argc, argv, "kvt" );

  // build an argument table for the rxvt
  int r_argc = 0;
  char ** r_argv = new char*[o_argc + 2];
  // first the args returned by Qt
  for (v=0; v<argc; v++) r_argv[v] = argv[v];

  // then the args for the terminal command
  if (commands > -1){
    while (o_argv[commands]){
      r_argv[v] = o_argv[commands];
      commands++;
      v++;
    }
  }
  r_argc = v;
  r_argv[r_argc] = NULL;
  //  a.setStyle(WindowsStyle);
  kvt = new kVt;
  a.setMainWidget( kvt );

  // this is for the original rxvt-code
  display = qt_xdisplay();

  rxvt_main(r_argc,r_argv);
   
  QSocketNotifier sn( comm_fd, QSocketNotifier::Read );
  QObject::connect( &sn, SIGNAL(activated(int)),
		    kvt, SLOT(application_signal()) );

  kvt->ResizeToVtWindow();
  kvt->show();
  return a.exec();

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


void kVt::scrolling( int value){
  MyWinInfo.offset =  length - value - (high - low);
  refresh();
}

