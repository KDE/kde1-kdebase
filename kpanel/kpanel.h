//
// kpanel. Part of the KDE project.
//
// Copyright (C) 1996,97 Matthias Ettrich
//

#ifndef KPANEL_H
#define KPANEL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <qwidget.h>
#include <qmenubar.h>
#include <qlabel.h>
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qpaintd.h>
#include <qpalette.h>
#include <qscrbar.h>
#include <qpopmenu.h>
#include <qtabdlg.h>
#include <qfiledlg.h>
#include <qtooltip.h>
#include <qdatetm.h>
#include <qlined.h>
#include <qcombo.h>
#include <qlabel.h>
#include <qradiobt.h>
#include <qtimer.h>
#include <qchkbox.h>
#include <qslider.h>
#include <qdrawutl.h>
#include <qptrdict.h>


// KDE includes
#include <kconfig.h>
#include <kwmmapp.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <X11/Xlib.h>

#include "pmenu.h"

enum Orientation {horizontal, vertical};
enum Position {top_left, bottom_right};
enum TaskbarPosition {hidden, top, bottom, taskbar_top_left};

void restart_the_panel();
extern char* applications_path;
extern char* personal_applications_path;
extern char* personal_applications_path_name;
extern bool ready_for_event_loop;


enum {
  OP_MAXIMIZE = 5000,
  OP_RESTORE,
  OP_ICONIFY,
  OP_UNICONIFY,
  OP_MOVE,
  OP_RESIZE,
  OP_CLOSE,
  OP_STICKY,
  OP_ONTO_CURRENT_DESKTOP,
  OP_ICONIFY_OTHER_WINDOWS
};

enum StyleSize { tiny = 0, normal, large };



class myPushButton: public QPushButton
{
  Q_OBJECT
public:
  myPushButton ( QWidget *parent=0, const char* name=0);
  ~myPushButton ();
  bool flat;
  int last_button;
  static myPushButton* most_recent_pressed;
  Window swallowed_window;
protected:
  void enterEvent( QEvent * );
  void leaveEvent( QEvent * );
  void mousePressEvent( QMouseEvent *e);
  void mouseReleaseEvent( QMouseEvent *e);
  void mouseMoveEvent( QMouseEvent *e);
  void paint( QPainter *_painter );
  void drawButton( QPainter *p ){paint(p);}
  virtual void drawButtonLabel( QPainter *painter );
  bool never_flat;
  bool flat_means_down;
  bool draw_down;
};


class myFrame: public QFrame
{
  Q_OBJECT
public:
  myFrame( bool _autoHide, unsigned int delay, QWidget *parent=0, const char* name=0, WFlags f=0);
  ~myFrame(){};
  bool autoHidden;
  bool autoHide;
  int hide_delay;

signals:
  void showMe();
  void hideMe();

protected:
  void enterEvent( QEvent * );

 private slots:
 void hideTimerDone();

private:
  QTimer *hideTimer;
};



class myTaskButton: public myPushButton
{
  Q_OBJECT
public:
  myTaskButton ( QWidget *parent=0, const char* name=0 );
  ~myTaskButton();
  void setText(const char*);
  void setActive(bool value = TRUE);
  bool isActive() const;
  static void setNoActive();
  Window win;
  int virtual_desktop;
 protected:
  virtual void drawButtonLabel( QPainter *painter );
 private:
  QString s;
  static myTaskButton* active;
};


class DesktopEntry {
public:
  DesktopEntry();
  myPushButton* button;
  QPopupMenu* popup;
  PMenuItem* pmi;
  KDNDDropZone* drop_zone;

  QString swallow;
  Window swallowed;
  int app_id;
  QString identity;
  QPixmap* icon[4];
};



class kPanel : public QFrame
{
  Q_OBJECT
public:
  kPanel(KWMModuleApplication* kwmapp_arg,
	 QWidget *parent=0, const char *name=0 );
  ~kPanel() {}


  void show();
  void doGeometry (bool do_not_change_taskbar = FALSE);

  void showSystem();

 public slots:

 // kwm communication
  void kwmInit();
  void windowAdd(Window);
  void dialogWindowAdd(Window);
  void windowRemove(Window);
  void windowChange(Window);
  void windowActivate(Window);
  void windowIconChanged(Window);
  void windowRaise(Window);
  void kwmDesktopChange(int);
  void kwmDesktopNameChange(int, QString);
  void kwmDesktopNumberChange(int);
  void kwmCommandReceived(QString);
  void dockWindowAdd(Window);
  void dockWindowRemove(Window);

/*   void playSound(QString); */

  void windowlistActivated(int);

  void taskbarClicked(int);
  void taskbarPressed(int);

  void button_clicked();
  void button_pressed();

  void add_windowlist();
  // --sven: kdisknav button start --
  void add_kdisknav();
  // --sven: kdisknav button end --
  void ask_logout();
  void call_help();
  void call_klock();

  void showPanel();

  void hidePanelLeft ();
  void hidePanelRight();
  void showPanelFromLeft (bool smooth = TRUE);
  void showPanelFromRight(bool smooth = TRUE);

  void miniButtons (int); // sven
  void desktop_change(int);

  void standalonePanelButtonClicked();
  void standalonePanelButton2Clicked();

  void configurePanel();
  void editMenus();


  void restart();


  void slotDropEvent( KDNDDropZone *_zone );

  void addButton(PMenuItem* pmi);
  void showToolTip(QString s);

  void kdisplayPaletteChanged();

  void tipTimerDone();
  void tipSleepTimerDone();
  void hideTimerDone();

  // the autoHideTaskbar feature
  void showTaskbar();
  void hideTaskbar();


  QWidget * parentOfSwallowed(Window);

  void launchSwallowedApplications();
  void parseMenus();


protected:
    void    resizeEvent( QResizeEvent * );
    void enterEvent( QEvent *);
    void leaveEvent( QEvent * );
    void syncWindowRegions();

    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent( QMouseEvent * );

private slots:
    void slotUpdateClock();
    void slotSwallowedChildDied(KProcess *);

signals:

private:
  void     writeInitStyle(KConfig *config, int size);
  bool     initConfigFile(KConfig *config);
  void     createPixmap();

  QPixmap mBackTexture;
  KWMModuleApplication* kwmmapp;
  Window* callbacklist;

  DesktopEntry entries[100];
  int nbuttons;

  QFrame *control_group;
  QButtonGroup *desktopbar;
  QPushButton *exit_button;
  QPushButton *lock_button;
  QPushButton *panel_button;
  QPushButton *panel_button_standalone;
  QPushButton *panel_button2;
  QPushButton *panel_button_standalone2;
  QFrame *panel_button_frame_standalone;
  QFrame *panel_button_frame_standalone2;
  QPushButton *mock_button;

  QButtonGroup *taskbar;
  QList <myTaskButton> taskbar_buttons;
  myTaskButton* taskButtonFromWindow(Window);
  myFrame* taskbar_frame;
  int taskbar_height;

  QLabel *info_label;
  bool info_label_is_sleeping;
  QTimer *tipTimer;
  QTimer *tipSleepTimer;
  QTimer *hideTimer;
  QWidget* last_tip_widget;

  int bound_top_left;
  int bound_bottom_right;

  QPopupMenu *popup_item;

  PMenu* pmenu;
  PMenu* pmenu_add;
  PMenu* p_pmenu;

  int wId;
  QPopupMenu *taskbarPopup;

  QLabel *label_date;
  QFrame *dock_area;

  QWidget *moving_button;
  QPoint moving_button_offset;
// tu
  QPoint moving_button_oldpos;

  QWidget *wait_cursor_button;
  QLineEdit *edit_button; // used for the editable desktop buttons

  QPoint position_of_new_item;

  QWidget *button_to_be_modified;

  KDNDDropZone* drop_zone;


  // options
  Orientation orientation;
  Position position;
  TaskbarPosition taskbar_position;

  int box_width;
  int box_height;
  int margin;
  int dbhs;
  int dbrows;
  int number_of_desktops;

  int tbhs;
  int tbmhs;

  bool foldersFirst;
  bool personalFirst;
  bool autoHide;
  bool autoHidden;
  unsigned int autoHideDelay;
  int autoHideSpeed;

  bool autoHideTaskbar;
  unsigned int autoHideTaskbarDelay;
  int autoHideTaskbarSpeed;

  bool clockAmPm;
  bool clockBeats;

  int hide_show_animation;

  // tools
  QString findMenuEditor(const QString& folder);

  QPixmap create_arrow_pixmap(QPixmap pm);
  void arrow_on_pixmap(QPixmap* pm, ArrowType rt);

  void set_button_text(QButton* button, const char* s);
  int show_popup(QPopupMenu* popup, QWidget* button, bool isTaskButton = False);
  void init_popup(QPopupMenu* popup);
  void set_label_date();
  void delete_button(QWidget* button);
  void cleanup();
  void restore_editbutton( bool takeit ); // Stephan

  void addButtonInternal(PMenuItem* pmi, int x = -1, int y = -1, QString name = "");

  // swallowing stuff
  void swallowApplication(const char *s);

  //layout
  void layoutTaskbar();
  int numberOfTaskbarRows();
  void layoutDockArea();
  void reposition(int l = 0);
  void find_a_free_place();
  void check_button_bounds(QWidget* button);
  void reflow_buttons(QWidget* button);

  void load_and_set_some_fonts();

  // dialogs
  QTabDialog* tab;

  QLineEdit *led[8];
  QComboBox *costy;

  QButtonGroup
    *bgrloc, // panel location
    *bgrta,  // taskbar location
    *bgrl,   // style
    *bgro,   // others
    *bgrt,   // clock
    *bgrm;   // menu tool tips
  QCheckBox    *cbpf;
  QCheckBox    *cbah;
  QCheckBox    *cbaht;
  QLabel       *mtts_actual;
  QSlider      *mtts;
  QRadioButton *mttb;
  QRadioButton *mttt;





  int old_style;
  QSlider *sl_dbhs, *sl_nr_db;

  int menu_tool_tips;
  bool menu_tool_tips_disabled;

  // kdisknav

  friend class PFileMenu;

  QStrList recent_folders;
  QStrList recent_files;

  uint max_recent_folders_entries;
  uint max_recent_files_entries;
  int head_recent_id;
  uint max_navigable_folder_entries;

  bool show_dot_files;
  bool ignore_case;   // ignore case when sorting
  bool show_shared_section;
  bool show_personal_section;
  bool show_recent_section;
  bool show_option_entry;

  // development

  myPushButton* kde_button;
  QPopupMenu* kmenu;
  PMenu*      personal_menu;
  PMenu*      global_menu;
  PMenuItem*  personal_pmi;

  QPopupMenu* windows;
  QPopupMenu* windowlist;
  // --sven: kdisknav button start --
  myPopupMenu *kdisknav;
  // --sven: kdisknav button end --
  void generateWindowlist(QPopupMenu*);

  QPixmap load_pixmap(const char* name, bool is_folder = false);


  void writeOutConfiguration();
  void readInConfiguration();

  void showMiniPanel(); // sven
  void hideMiniPanel ();

  int currentDesktop;

  Bool panelHidden[8+1];
  Bool panelHiddenLeft[8+1];
  Bool panelCurrentlyHidden;
  Bool panelCurrentlyLeft;
  Bool miniPanelHidden;

  QFrame *miniPanelFrame;
  QButtonGroup *miniPanel;
  QPushButton *miniSystem;
  QPushButton *miniDesk;
  QPushButton *miniDiskNav;

  int clock_timer_id;

  bool initing;

  QStrList swallowed_applications;

  QRect taskbar_frame_geometry;

  QPixmap defaultPixmap;

  bool has_kdisknav_button;
  bool has_windowlist_button;
  int add_disknav_entry;
  int add_windowlist_entry;
  QPopupMenu* panel_popup;

};


#endif // KPANEL_H
