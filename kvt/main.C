// $ Id: $
//
// kvt. Part of the KDE project.
//
// Copyright (C) 1996 Matthias Ettrich
//
// (C) 1999 Leon Widdershoven
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
#include <qclipbrd.h>
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
#include <kcharsets.h>
#include <kfontdialog.h>
#include <qregexp.h>

#include "kvt_version.h"

#include "main.h"

extern "C" {
extern void *safemalloc(int, const char *identifier);
extern void get_token();
extern int handle_X_event(XEvent event, unsigned char);
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

extern int reg_font_handles[6];

extern char *xvt_name; // the name the program is run under
extern char *window_name;
extern char *icon_name;

extern keyset keys[];			/* keysets */

extern unsigned long	foreground;	/* foreground pixel value */
extern unsigned long	background;	/* background pixel value */
extern int rstyle;

extern int mouse_block;			/* block mouse input while popup */

extern int BackspaceSendsControlH;
extern int KeySetSend;

extern void kvt_set_fontnum(char *);
extern void kvt_set_menubar(int);
extern void kvt_set_scrollbar(int);
extern void kvt_set_size_increment(int, int);

extern void kvt_set_selection(char* s);
extern char* kvt_get_selection();

}

extern Display* display;
/* file descriptor for child process */
extern int comm_fd;
extern Window		main_win;
extern Window		vt_win;
extern char *fg_string_tmp, *bg_string_tmp;

kVt* kvt = 0;
OptionDialog *m_optiondialog = 0;
QString kvt_charclass;

QFont kvt_basefnt;
QFont kvt_fnt1;
QFont kvt_fnt2;
QFont kvt_fnt3;
QFont kvt_fnt4;
QFont kvt_fnt5;
QFont kvt_fnt6;
int qt_font_hack = 1;		/* use workaround for Qt font bug */

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

void kvt_set_selection(char* s){
  QApplication::clipboard()->setText(s);
}

char* kvt_get_selection(){
  return (char*) QApplication::clipboard()->text();
}

int kvt_find_font(const char *name, int ptsz, char *ret)
{
	char **fontNames;
	int    count, score = 99999, ptscore = 0, gfpts = 0;
	/* int pos
	 * char   buf[32];
	 */
	QString data;

	fontNames = XListFonts(QPaintDevice::x__Display(), (char*)name,
		1024, &count);
// printf("Query for [%s] size %d returned %d fonts\n", name, ptsz, count);
	score = 99999; ptscore = 0;
	for (int i = 0; i < count; ++i) {
		// find out point size from font name
		int sc1, fsz, fpts;
		const char *p = fontNames[i];
		int dashcnt = 7;
		while (*p && dashcnt) {
			if (*p == '-') dashcnt--;
			p++;
		}
		sscanf(p, "%d-%d-", &fsz, &fpts);
		sc1 = (fsz - ptsz) * 10;
		if (sc1 < 0) sc1 = -sc1 + 5;
// printf("Examine: [%s], size: %d, score: %d\n", fontNames[i], fsz, sc1);
		if (sc1 < score || (sc1 == score && ptsz > ptscore)) {
			ptscore = ptsz;
			score = sc1;
			gfpts = fpts;
			data = fontNames[i];
		}
	}
	XFreeFontNames(fontNames);
/*
	// now remove ptsize from font name and retry
	sprintf(buf, "-%d-", gfpts);
	pos = data.find(buf);
	if (pos >= 0) {
		data.replace(pos, strlen(buf), "-*-");
	}
	fontNames = XListFonts(QPaintDevice::x__Display(), (const char*)data,
		1024, &count);
// printf("Query for [%s] size %d returned %d fonts\n", (const char*) data, ptsz, count);
	for (int i = 0; i < count; ++i) {
		// find out point size from font name
		int sc1, fsz, fpts;
		const char *p = fontNames[i];
		int dashcnt = 7;
		while (*p && dashcnt) {
			if (*p == '-') dashcnt--;
			p++;
		}
		sscanf(p, "%d-%d-", &fsz, &fpts);
		sc1 = (fsz - ptsz) * 10;
		if (sc1 < 0) sc1 = -sc1 + 5;
// printf("Examine: [%s], size: %d, score: %d\n", fontNames[i], fsz, sc1);
		if (sc1 < score || (sc1 == score && ptsz > ptscore)) {
			ptscore = ptsz;
			score = sc1;
			data = fontNames[i];
		}
	}
	XFreeFontNames(fontNames);
*/
	strcpy(ret, (const char*) data);
// printf("Return [%s], score: %d\n", ret, score);
	return score;
}

int kvt_get_font_handle(QFont *fnt)
{
	if (qt_font_hack && strcmp(fnt->family(), "fixed") == 0
	    && (fnt->charSet() == QFont::ISO_8859_1
		|| fnt->charSet() == QFont::ISO_8859_2)) {
		// change font name
		char buf1[256], buf2[256], buf3[256], buf4[256];
		int chset = (fnt->charSet() == QFont::ISO_8859_1)?1:2;
		int score1, score2;

		sprintf(buf1, "-*-fixed-%s-r-semicondensed-*-*-*-*-*-*-*-"
			"iso8859-%d",
			(fnt->weight() >= QFont::Bold)?"bold":"medium",
			chset);
		sprintf(buf2, "-*-fixed-%s-r-normal-*-*-*-*-*-*-*-"
			"iso8859-%d",
			(fnt->weight() >= QFont::Bold)?"bold":"medium",
			chset);
		
		score1 = kvt_find_font(buf1, fnt->pointSize(), buf3);
		score2 = kvt_find_font(buf2, fnt->pointSize(), buf4);

		if (score1 <= score2) {
			fnt->setFamily(buf3);
		} else {
			fnt->setFamily(buf4);
		}
		fnt->setRawMode(true);
	}
	return fnt->handle();
}

void kvt_reinit_fonts(const QFont &base)
{
  int ptsz[6];

  ptsz[1] = kvt_fnt2.pointSize();
  ptsz[2] = kvt_fnt3.pointSize();
  ptsz[3] = kvt_fnt4.pointSize();
  ptsz[4] = kvt_fnt5.pointSize();
  ptsz[5] = kvt_fnt6.pointSize();

  kvt_basefnt = base;
  kvt_fnt1 = base;
  kvt_fnt2 = base;
  kvt_fnt3 = base;
  kvt_fnt4 = base;
  kvt_fnt5 = base;
  kvt_fnt6 = base;

  kvt_fnt2.setPointSize(ptsz[1]);
  kvt_fnt3.setPointSize(ptsz[2]);
  kvt_fnt4.setPointSize(ptsz[3]);
  kvt_fnt5.setPointSize(ptsz[4]);
  kvt_fnt6.setPointSize(ptsz[5]);

  // take font handles (they are actually fids)
  reg_font_handles[0] = kvt_get_font_handle(&kvt_fnt1);
  reg_font_handles[1] = kvt_get_font_handle(&kvt_fnt2);
  reg_font_handles[2] = kvt_get_font_handle(&kvt_fnt3);
  reg_font_handles[3] = kvt_get_font_handle(&kvt_fnt4);
  reg_font_handles[4] = kvt_get_font_handle(&kvt_fnt5);
  reg_font_handles[5] = kvt_get_font_handle(&kvt_fnt6);
}

class MyApp:public KApplication {
public:
  MyApp( int &argc, char **argv, const QString& rAppName );
  virtual bool x11EventFilter( XEvent * );
};

MyApp::MyApp(int &argc, char **argv , const QString& rAppName):
  KApplication(argc, argv, rAppName){
}

static MyApp* myapp = 0;

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
	int res = handle_X_event(*ev, 0);
	screen_refresh();
	if (res) return TRUE;
      }
    return FALSE;
  }

  // this hack is because the mentioned windows aren't in any widget.
  // setting the window back to main_win (a Qt window) is
  // necessary for Qt-grabbing when using the menubar. TODO. Matthias
  if ( ev->xany.window == main_win ||
       ev->xany.window == vt_win){

    if (ev->xany.type != MotionNotify || motion_allowed){
      int res = handle_X_event(*ev, 0);
      screen_refresh();
      if (res) return TRUE;
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

class KVT_QPopupMenu : public QPopupMenu {

public:
	virtual void show();
	virtual void hide();
};

void KVT_QPopupMenu::show()
{
	mouse_block = 1;
	QPopupMenu::show();
}

void KVT_QPopupMenu::hide()
{
	mouse_block = 0;
	QPopupMenu::hide();
}

OptionDialog::OptionDialog(QWidget *parent, const char *name)
  : QDialog( parent, name, TRUE )
{
  setCaption(name);
  QLabel *label_color, *label_class, *label_backspace, *label_keyset;

  label_color = new QLabel(i18n("choose type of color-mode"), this);
  label_color->setMinimumSize(label_color->sizeHint());
  colormode = new QComboBox(this);
  colormode->setMinimumSize(colormode->sizeHint());
  colormode->setFixedHeight(colormode->sizeHint().height());

  label_class = new QLabel(i18n("Add characters to word class"), this);
  label_class->setMinimumSize(label_class->sizeHint());
  chars = new QLineEdit(this);
  chars->setMinimumSize(chars->sizeHint());
  chars->setFixedHeight(chars->sizeHint().height());

  label_backspace = new QLabel(i18n("The backspace key will send a"), this);
  label_backspace->setMinimumSize(label_backspace->sizeHint());
  backspace = new QComboBox(this);
  backspace->setMinimumSize(backspace->sizeHint());
  backspace->setFixedHeight(backspace->sizeHint().height());

  label_keyset = new QLabel(i18n("Cursor key set"), this);
  label_keyset->setMinimumSize(label_keyset->sizeHint());
  keyset = new QComboBox(this);
  keyset->setMinimumSize(keyset->sizeHint());
  keyset->setFixedHeight(keyset->sizeHint().height());
  connect( keyset, SIGNAL(activated(int)), SLOT(update_bs(int)) );

  QPushButton *ok, *cancel;

  ok = new QPushButton( i18n("OK"), this );
  ok->setMinimumSize(ok->sizeHint());
  ok->setFixedHeight(ok->sizeHint().height());
  ok->setDefault(true);
  connect( ok, SIGNAL(clicked()), SLOT(accept()) );

  cancel = new QPushButton( i18n("Cancel"), this );
  cancel->setMinimumSize(cancel->sizeHint());
  cancel->setFixedHeight(cancel->sizeHint().height());
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

  QVBoxLayout *geom1;
  QGridLayout *geom2;
  QHBoxLayout *geom3;
  geom1 = new QVBoxLayout(this, 4);
  geom2 = new QGridLayout(4, 2);
  geom1->addLayout(geom2, 3);
  geom2->setColStretch(1,10);
  geom2->addWidget(label_color, 0, 0);
  geom2->addWidget(colormode, 0, 1);
  geom2->addWidget(label_class, 1, 0);
  geom2->addWidget(chars, 1, 1);
  geom2->addWidget(label_backspace, 2, 0);
  geom2->addWidget(backspace, 2, 1);
  geom2->addWidget(label_keyset, 3, 0);
  geom2->addWidget(keyset, 3, 1);
  geom3 = new QHBoxLayout();
  geom1->addLayout(geom3);
  geom3->addStretch();
  geom3->addWidget(ok);
  geom3->addStretch();
  geom3->addWidget(cancel);
  geom3->addStretch();
  resize(10, 10);
  geom1->activate();

  int i;
  for (i=0; color_mode_name[i]; i++) {
    colormode->insertItem(color_mode_name[i], i);
  }
  for (i=0; backspace_name[i]; i++) {
    backspace->insertItem(backspace_name[i], i);
  }
  for (i=0; keys[i].name; i++) {
    keyset->insertItem(keys[i].name, i);
  }
}

void OptionDialog::update_bs( int keyset ) {
  // update Backspace/delete according to current keyset
  if (keys[keyset].bs[0] == 8) backspace->setCurrentItem(1);
  else backspace->setCurrentItem(0);
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

kVt::kVt( KConfig* sessionconfig,  const QStrList& args,
	  QWidget *parent, const char *name )
  : QWidget( parent, name){

    kvtarguments = args;

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

    enableHotkeys = sessionconfig->readBoolEntry("Hotkeys", true);

    entry = sessionconfig->readEntry("charclass");
    if (!entry.isEmpty()) {
      kvt_charclass = entry;
      set_charclass(entry);
    } else {
      set_charclass("");
    }

    /* if bacspace=BS then backspace sends a ^H otherwise it will send a ^? */
    entry = sessionconfig->readEntry("backspace");
    BackspaceSendsControlH = !(entry && entry=="DEL");
    KeySetSend = sessionconfig->readNumEntry("keyset", 0);

    m_file = new QPopupMenu;
    CHECK_PTR( m_file );
    m_file->insertItem( i18n("&New Terminal"));
    m_file->insertSeparator();
    m_file->insertItem( i18n("E&xit") ,  this, SLOT(quit()) );
    connect(m_file, SIGNAL(activated(int)), SLOT(file_menu_activated(int)));

    m_scrollbar = new QPopupMenu;
    CHECK_PTR( m_scrollbar );
    m_scrollbar->insertItem( i18n("&Hide"));
    m_scrollbar->insertItem( i18n("&Left"));
    m_scrollbar->insertItem( i18n("&Right"));
    connect(m_scrollbar, SIGNAL(activated(int)), SLOT(scrollbar_menu_activated(int)));

    m_size = new QPopupMenu;
    CHECK_PTR( m_size );
    m_size->insertItem( i18n("&Normal"));
    m_size->insertItem( i18n("&Tiny"));
    m_size->insertItem( i18n("&Small"));
    m_size->insertItem( i18n("&Medium"));
    m_size->insertItem( i18n("&Large"));
    m_size->insertItem( i18n("&Huge"));
    connect(m_size, SIGNAL(activated(int)), SLOT(size_menu_activated(int)));

    m_dimen = new QPopupMenu;
    CHECK_PTR( m_dimen );
    for (int i=0; kvt_dimens[i].text; i++) {
      m_dimen->insertItem(kvt_dimens[i].text);
    }
    connect(m_dimen, SIGNAL(activated(int)), SLOT(dimen_menu_activated(int)));

    m_color = new QPopupMenu;
    CHECK_PTR( m_color );
    m_color->insertItem( i18n("&black/white"));
    m_color->insertItem( i18n("&white/black"));
    m_color->insertItem( i18n("&green/black"));
    m_color->insertItem( i18n("black/light&yellow"));
    m_color->insertItem( i18n("Linu&x Console"));

    colors =  	new QPopupMenu ();
    CHECK_PTR( colors );
    colors->insertItem(i18n("&Foreground Color"),
		       this, SLOT(select_foreground_color()));
    colors->insertItem(i18n("&Background Color"),
		       this, SLOT(select_background_color()));
    m_color->insertSeparator();
    m_color->insertItem(i18n("&Custom Colors"),colors);
    connect(m_color, SIGNAL(activated(int)), SLOT(color_menu_activated(int)));


    m_options = new KVT_QPopupMenu;
    CHECK_PTR( m_options );
    m_options->setCheckable(true);
    if (menubar_visible)
      m_options->insertItem( i18n("Hide &Menubar") );
    else
      m_options->insertItem( i18n("Show &Menubar") );
    m_options->insertItem( i18n("Secure &keyboard"));
    m_options->insertSeparator();
    m_options->insertItem( i18n("Scroll&bar"), m_scrollbar);
    m_options->insertItem( i18n("&Font size"), m_size);
    m_options->insertItem( i18n("&Color"), m_color);
    m_options->insertItem( i18n("&Size"), m_dimen);
    m_options->insertItem( i18n("Terminal...") );
    m_options->insertItem( i18n("Font...") );
    int id = m_options->insertItem( i18n("Enable &hotkeys") );
    m_options->setItemChecked(id, enableHotkeys);
    m_options->insertSeparator();
    m_options->insertItem( i18n("Save &Options"));

    m_options->installEventFilter( this );

    connect(m_options, SIGNAL(activated(int)),
	    SLOT(options_menu_activated(int)));


    QString at = KVT_VERSION;
    at += i18n("\n\n(C) 1996, 1997 Matthias Ettrich (ettrich@kde.org)\n"
	       "(C) 1997 M G Berberich (berberic@fmi.uni-passau.de)\n"
	       "(C) 1998 Ivan Schreter (schreter@kdk.sk)\n"
	       "(C) 1999 Leon Widdershoven (l.widdershoven@fz-juelich.de)\n\n"
	       "Terminal emulation for the KDE Desktop Environment\n"
	       "based on Robert Nation's rxvt-2.08");
		
    m_help = kapp->getHelpMenu(true, at.data());
    m_help->setAccel(0,0);
    menubar = new KMenuBar( this );
    CHECK_PTR( menubar );

    entry = sessionconfig->readEntry("kmenubar");
    if (!entry.isEmpty() && entry == "floating" ) {
      menubar->setMenuBarPos(KMenuBar::Floating);
      entry = sessionconfig->readEntry("kmenubargeometry");
      if (!entry.isEmpty()) {
	menubar->setGeometry(KWM::setProperties(menubar->winId(), entry));
      }
    }
    else if (!entry.isEmpty() && menubar->menuBarPos() == KMenuBar::Top && entry == "bottom") {
      menubar->setMenuBarPos(KMenuBar::Bottom);
    }
    
    connect(menubar, SIGNAL (moved(menuPosition)),
	    SLOT (menubarMoved()));
    toggleHotkeys();

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
    connect(KApplication::getKApplication(), SIGNAL(shutDown()),
	    SLOT(shutDown()));

    entry = sessionconfig->readEntry("geometry");
    if (!entry.isEmpty()) {
      setGeometry(KWM::setProperties(winId(), entry));
    }

    if (menubar_visible && menubar->menuBarPos() == KMenuBar::Floating)
	menubar->show();

    if (menubar_visible && menubar->menuBarPos() == KMenuBar::FloatingSystem)
	menubar->show();

    connect( QApplication::clipboard(), SIGNAL( dataChanged() ), 
	     SLOT( clipboard_changed() ));

}

void kVt::toggleHotkeys() {
  menubar->clear();
  QRegExp r("&");

  QString s = i18n("&File");
  if(!enableHotkeys)
    s.replace(r, "");
  menubar->insertItem( s.data(), m_file);

  s = i18n("&Options");
  if(!enableHotkeys)
    s.replace(r, "");
  menubar->insertItem( s.data(), m_options);

  menubar->insertSeparator();

  s = i18n("&Help");
  if(!enableHotkeys)
    s.replace(r, "");
  menubar->insertItem( s.data(), m_help);
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
  if (kvtarguments.count() > 0)
    config->writeEntry("kvtarguments", kvtarguments);
  config->writeEntry("defaultFont", kvt_basefnt);
  config->writeEntry("fontSize2", kvt_fnt2.pointSize());
  config->writeEntry("fontSize3", kvt_fnt3.pointSize());
  config->writeEntry("fontSize4", kvt_fnt4.pointSize());
  config->writeEntry("fontSize5", kvt_fnt5.pointSize());
  config->writeEntry("fontSize6", kvt_fnt6.pointSize());
  config->sync();
}

void kVt::shutDown(){  
  clean_exit(-1);
}

kVt::~kVt(){
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

  kvtconfig->writeEntry("keyset", KeySetSend);

  kvtconfig->writeEntry("Hotkeys", enableHotkeys);

  if (menubar->menuBarPos() == KMenuBar::Bottom)
    kvtconfig->writeEntry("kmenubar", "bottom");
  else
    kvtconfig->writeEntry("kmenubar", "top");

  kvtconfig->writeEntry("defaultFont", kvt_basefnt);
  kvtconfig->writeEntry("fontSize2", kvt_fnt2.pointSize());
  kvtconfig->writeEntry("fontSize3", kvt_fnt3.pointSize());
  kvtconfig->writeEntry("fontSize4", kvt_fnt4.pointSize());
  kvtconfig->writeEntry("fontSize5", kvt_fnt5.pointSize());
  kvtconfig->writeEntry("fontSize6", kvt_fnt6.pointSize());

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
    m_options->changeItem(i18n("Show &Menubar"), 0);
  }
  menubar_visible = b;
}

// only works for hiding!!
void kVt::setScrollbar(bool b){
  if (!b){
    scrollbar->hide();
    m_scrollbar->changeItem(i18n("&Show"), 0);
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
  if (menubar_visible &&  (menubar->menuBarPos() != KMenuBar::Floating
      || menubar->menuBarPos() != KMenuBar::FloatingSystem) ){
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


void kVt::resizeEvent( QResizeEvent * /*ev*/)
{
  doGeometry();
}

void kVt::options_menu_activated( int item){
  switch (item){
  case 0:
    // menubar
    if (menubar->isVisible()){
      // hide
      menubar_visible = FALSE;
      menubar->hide();
      if (menubar->menuBarPos() != KMenuBar::Floating &&
          menubar->menuBarPos() != KMenuBar::FloatingSystem )
	resize(width(), height()-menubar->height());
      m_options->changeItem(i18n("Show &Menubar"), item);
    }
    else {
      // show
      menubar_visible = TRUE;
      menubar->show();
      if (menubar->menuBarPos() != KMenuBar::Floating &&
          menubar->menuBarPos() != KMenuBar::FloatingSystem )
	resize(width(), height()+menubar->height());
      m_options->changeItem(i18n("Hide &Menubar"), item);
    }
    break;
  case 1:
    keyboard_secured = !keyboard_secured;
    if (keyboard_secured){
      m_options->changeItem(i18n("Unsecure &keyboard"), item);
      extract_colors(fg_string.data(), "red");
    }
    else {
      m_options->changeItem(i18n("Secure &keyboard"), item);
      extract_colors(fg_string.data(), bg_string.data());
    }
    scr_secure(); // also calls XClearwindow and scr_refresh
    break;

  case 7:
    m_optiondialog = new OptionDialog(this, i18n("Terminal Options"));
    m_optiondialog->colormode->setCurrentItem(get_color_mode());
    m_optiondialog->chars->setText(kvt_charclass);
    m_optiondialog->backspace->setCurrentItem(BackspaceSendsControlH);
    m_optiondialog->keyset->setCurrentItem(KeySetSend);
    if (m_optiondialog->exec()) {
      set_color_mode(m_optiondialog->colormode->currentItem());
      kvt_charclass = m_optiondialog->chars->text();
      set_charclass(kvt_charclass);
      scr_refresh(0,0,MyWinInfo.pwidth,MyWinInfo.pheight);
      BackspaceSendsControlH = m_optiondialog->backspace->currentItem();
      KeySetSend = m_optiondialog->keyset->currentItem();
    }
    delete m_optiondialog;
    break;
  case 8:
    // font options
    {
	QFont fnt = kvt_basefnt;
	KFontDialog::getFont(fnt);
	kvt_reinit_fonts(fnt);
	LoadNewFont();
	ResizeToVtWindow();
    }
    break;

  case 9:
    // hotkeys
    {
      enableHotkeys = !enableHotkeys;
      m_options->setItemChecked(9, enableHotkeys);
      toggleHotkeys();
    }

  case 11:
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
      m_scrollbar->changeItem(i18n("&Show"), item);
    }
    else {
      // show
      scrollbar_visible = TRUE;
      scrollbar->show();
      resize(width()+16, height());
      m_scrollbar->changeItem(i18n("&Hide"), item);
    }
    break;

  case 1: // left
      kvt_scrollbar = kvt_left;
      if (!scrollbar->isVisible()){
	scrollbar_menu_activated(0);
	break;
      }
      doGeometry();
      break;

  case 2: // right
    kvt_scrollbar = kvt_right;
    if (!scrollbar->isVisible()){
      scrollbar_menu_activated(0);
      break;
    }
    doGeometry();
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
//   if ( item == kvt_dimen)
//     return;
  kvt_dimen = item;
  //debug( "Resizing to %i, %i", kvt_dimens[item].x, kvt_dimens[item].y );
  // Hmm. I don't know why y needs -1, and x doesn't. But it works,
  // stty -a reports the correct size with -1, and not without.
  // If you DO know, mail me: l.widdershoven@fz-juelich.de
  kvt->ResizeToDimen(kvt_dimens[kvt_dimen].x, kvt_dimens[kvt_dimen].y-1);
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
    QMessageBox::message( i18n("Hint"),
			  i18n("New color settings will be displayed\nwhen the keyboard is unsecured.") );
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
	if (handle_X_event(stored_xevent_for_keys, ((QKeyEvent*)ev)->ascii()))
		return TRUE;
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
  if (new_pos == KMenuBar::Top || new_pos == KMenuBar::Flat){
    if (frame->height() == height())
      resize(width(), height()+menubar->height());
    else
      doGeometry();
  }
  else if (new_pos == KMenuBar::Bottom){
    if (frame->height() == height())
      resize(width(), height()+menubar->height());
    else
      doGeometry();
  }
  else if (new_pos == KMenuBar::Floating){
    if (frame->height() != height())
      resize(width(), height()-menubar->height());
  }
  else if (new_pos == KMenuBar::FloatingSystem) {
    if (frame->height() != height())
      resize(width(), height()-menubar->height());
  }
}

void kVt::doGeometry(){
  if (menubar_visible && menubar->menuBarPos() == KMenuBar::Top){
    menubar->setGeometry(0,0,width(), menubar->heightForWidth(width()));
    frame->setGeometry(0, menubar->height(), width(), height()-menubar->height());
  }
  else if (menubar_visible && menubar->menuBarPos() == KMenuBar::Flat){
    menubar->move (0,0);
    frame->setGeometry(0, menubar->height(), width(), height()-menubar->height());
  }
  else if (menubar_visible && menubar->menuBarPos() == KMenuBar::Bottom){
    menubar->setGeometry(0,height()-menubar->height(),width(), menubar->height());
    frame->setGeometry(0, 0, width(), height()-menubar->height());
  }
  else{ // menubar not visible or floating
      //    menubar->resize(width(), menubar->height());
    frame->setGeometry(0, 0, width(), height());
  }

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
}


void kVt::quit(){
  delete menubar;
  myapp->quit();
}

void kVt::clipboard_changed() {
  if ( !scr_has_focus() ) {
	scr_clear_selection();
	screen_refresh();
  }
} 

//---------------------------------------------------------------------------
// main
//---------------------------------------------------------------------------

#include "main.moc"

int main(int argc, char **argv)
{
  setlocale(LC_CTYPE, "");
  int i;
  // replace "-caption" with "-T"
  for (i=0; i<argc; i++) {
    if (QString("-caption")==argv[i]){
      argv[i][1]='T';
      argv[i][2]='\0';
    }
  }

  // find out if we need to turn off font hack
  for (i = 0; i < argc; ++i) {
	if (QString("-no_font_hack") == argv[i]) {
		qt_font_hack = 0;
	}
  }

  // first make an argument copy for a new kvt
  QStrList orgarg;
  for (i=0; i<argc; i++) {
    orgarg.append(argv[i]);
  }

  // cut off the command arguments for the terminal command
  char **com_argv = 0;
  for (i=0; i<argc; i++){
    if (strcmp(argv[i],"-e") == 0){
      com_argv = argv+i+1;
      argc = i;
      argv[i] = 0;
    }
  }

  // create the QT Application
  MyApp a( argc, argv, "kvt" );
  myapp = &a;

  // now create fonts for five resolutions from default fixed font
  // this ensures that we have correct charset for all fonts
  // TODO: make default font and font sizes configurable
  KConfig *cfg = kapp->getConfig();
  cfg->setGroup("kvt");
  QFont fntDef("fixed", 13);
  fntDef = cfg->readFontEntry("defaultFont", &fntDef);
  kvt_fnt2.setPointSize(cfg->readNumEntry("fontSize2", 7));
  kvt_fnt3.setPointSize(cfg->readNumEntry("fontSize3", 10));
  kvt_fnt4.setPointSize(cfg->readNumEntry("fontSize4", 14));
  kvt_fnt5.setPointSize(cfg->readNumEntry("fontSize5", 15));
  kvt_fnt6.setPointSize(cfg->readNumEntry("fontSize6", 20));
  kvt_reinit_fonts(fntDef);

  // this is for the original rxvt-code
  display = qt_xdisplay();

  // arguments after Qt
  QStrList arguments;
  for (i=0; i<argc; i++)
    arguments.append(argv[i]);
  if (com_argv){
    arguments.append("-e");
    for (i=0; com_argv[i]; i++) {
      arguments.append(com_argv[i]);
    }
  }


  KConfig* sessionconfig = a.getConfig();
  if (a.isRestored()){
    arguments.clear();
    sessionconfig = a.getSessionConfig();
    sessionconfig->setGroup("kvt");
    orgarg.clear();
    sessionconfig->readListEntry("kvtarguments", orgarg);
    arguments = orgarg;
    // everything has changed. Generate new args for this kvt
    argc = orgarg.count();
    argv = new char* [argc+1];
    for (i=0;i<(signed int)orgarg.count();i++)
      argv[i]=orgarg.at(i);
    argv[i]=0;

    // cut off the command arguments for the terminal command
    com_argv = 0;
    for (i=0; i<argc; i++){
      if (strcmp(argv[i],"-e") == 0){
	com_argv = argv+i+1;
	argc = i;
	argv[i] = 0;
      }
    }

    if (com_argv){
      orgarg.append("-e");
      for (i=0; com_argv[i]; i++) {
	orgarg.append(com_argv[i]);
      }
    }
  }

  // create args for a new kvt
  o_argc = orgarg.count();
  o_argv = new char* [o_argc+1];
  for (i=0;i<(signed int)orgarg.count();i++){
    o_argv[i] = orgarg.at(i);
  }
  o_argv[i] = 0;

  kvt = new kVt(sessionconfig, arguments);

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
    if ((s=strrchr(com_argv[0],'/'))!=0)
      s++;
    else
      s=com_argv[0];
    window_name = icon_name = s;
  }

  if (!init_display(argc, argv)) {
    fprintf(stderr, KVT_VERSION);
    fprintf(stderr, "\n\n");
    fprintf(stderr,
	    i18n("Copyright(C) 1996, 1997 Matthias Ettrich\n"
		 "Copyright(C) 1997 M G Berberich\n"
		 "Terminal emulation for the KDE Desktop Environment\n"
		 "based on Robert Nation's rxvt-2.08\n\n"));
    fprintf(stderr, i18n("Permitted arguments are:\n"));
    fprintf(stderr, i18n("-e <command> <arg> ...    execute command with ars - must be last argument\n"));
    fprintf(stderr, i18n("-display <name>           specify the display (server)\n"));
    fprintf(stderr, i18n("-vt_geometry <spec>       the initial vt window geometry\n"));
    fprintf(stderr, i18n("-print-pipe <name>        specify pipe for vt100 printer\n"));
    fprintf(stderr, i18n("-vt_bg <colour>           background color\n"));
    fprintf(stderr, i18n("-vt_fg <colour>           foreground color\n"));
    fprintf(stderr, i18n("-vt_font <fontname>       normal font\n"));
    fprintf(stderr, i18n("-no_font_hack             turn off Qt font bug workaround\n"));
    fprintf(stderr, i18n("-vt_size <size>           tiny, small, normal, large, huge\n"));
    fprintf(stderr, i18n("-linux                    set up kvt to mimic linux-console\n"));
    fprintf(stderr, i18n("-no_menubar               hide the menubar\n"));
    fprintf(stderr, i18n("-no_scrollbar             hide the scrollbar\n"));
    fprintf(stderr, i18n("-T <text>                 text in window titlebar \n"
			 "                          (Obsolete. Use -caption instead)\n"));
    fprintf(stderr, i18n("-tn <TERM>                Terminal type. Default is xterm.\n"));
    fprintf(stderr, i18n("-C                        Capture system console message\n"));
    fprintf(stderr, i18n("-n <text>                 name in icon or icon window\n"));
    fprintf(stderr, i18n("-7                        run in 7 bit mode\n"
			 "-8                        run in 8 bit mode\n"));
    fprintf(stderr, i18n("-ls                       initiate kvt's shell as a login shell\n"
			 "-ls-                      initiate kvt's shell as a non-login shell (default)\n"));
    fprintf(stderr, i18n("-meta <arg>               handle Meta with ESCAPE prefix, 8THBIT set, or ignore\n"));
    fprintf(stderr, i18n("-sl <number>              save number lines in scroll-back buffer\n"));
    fprintf(stderr, i18n("-pageup <keysym>          use hot key alt-keysym to scroll up through the buffer\n"
			 "-pagedown <keysym>        use hot key alt-keysym to scroll down through buffer\n"));
#ifdef GREEK_KBD
    fprintf(stderr, i18n(
	    "-grk9		greek kbd = ELOT 928 (default)\n"
	    "-grk4		greek kbd = IBM 437\n"));
#endif
    clean_exit(1);
  }

  kvt->do_some_stuff(sessionconfig);

  init_command(0 ,(unsigned char **)com_argv);

  QSocketNotifier sn( comm_fd, QSocketNotifier::Read );
  QObject::connect( &sn, SIGNAL(activated(int)),
		    kvt, SLOT(application_signal()) );

  if (!a.isRestored())
    kvt->ResizeToVtWindow();

  a.setMainWidget( kvt );
  a.setTopWidget( kvt );
  kvt->show();

  clean_exit(a.exec());  
}
