//
// kvt. Part of the KDE project.
//
// Copyright (C) 1996 Matthias Ettrich
//
// (C) 1999 Leon Widdershoven

#ifndef MAIN_H
#define MAIN_H

#include <qwidget.h>
#include <qmenubar.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qfiledlg.h>
#include <qscrbar.h>
#include <qwindefs.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qcombo.h>
#include <qstrlist.h>
#include <kmenubar.h>
#include <klocale.h>

enum KvtScrollbar{kvt_right, kvt_left};
enum KvtSize{kvt_normal, kvt_tiny, kvt_small, kvt_medium, kvt_large, kvt_huge};
typedef struct {
  char *text;
  int x, y;
} Kvt_Dimen;

class kVt : public QWidget
{
    Q_OBJECT

public:
    kVt( KConfig* sessionconfig, const QStrList& args,
	 QWidget *parent=0, const char *name=0 );
   ~kVt();

  // public because this need to be set from old rxvt-C-code
    QScrollBar* scrollbar;
  void ResizeToVtWindow();
  void setMenubar(bool);
  void setScrollbar(bool);
  void ResizeToDimen(int width, int height);
  void do_some_stuff(KConfig* kvtconfig);//temporary (Matthias)
  void saveOptions(KConfig*);

public slots:
  void select_foreground_color();
  void select_background_color();
  void application_signal();
  void options_menu_activated( int );
  void scrollbar_menu_activated( int );
  void size_menu_activated( int );
  void dimen_menu_activated(int);
  void color_menu_activated( int );
  void file_menu_activated(int);
  void scrolling(int);
  void onDrop( KDNDDropZone* _zone );

  void menubarMoved();
  void quit();

  void saveYourself();
  void toggleHotkeys();
  void shutDown();

protected:
    void    resizeEvent( QResizeEvent * );
    bool eventFilter( QObject *, QEvent * );
	void styleChange( GUIStyle );

private:

  void doGeometry();
  KMenuBar *menubar;
  QFrame *frame;
  QPopupMenu *m_file;
  QPopupMenu *colors;
  QPopupMenu *m_options;
  QPopupMenu *m_scrollbar;
  QPopupMenu *m_size;
  QPopupMenu *m_dimen;
  QPopupMenu *m_color;
  QPopupMenu *m_help;
  QWidget *rxvt;
  // weird flags
  Bool setting_to_vt_window;
  Bool keyboard_secured;
  bool enableHotkeys;
  
  // options
  
  KConfig* kvtconfig;
  QStrList kvtarguments;

  QString fg_string;
  QString bg_string;
  Bool menubar_visible;
  Bool scrollbar_visible;
  KvtScrollbar kvt_scrollbar;
  KvtSize kvt_size;
  KDNDDropZone    *dropZone;
};

class OptionDialog : public QDialog {
  Q_OBJECT
public:
  QComboBox *colormode;
  QLineEdit *chars;
  QComboBox *backspace;
  QComboBox *keyset;

  OptionDialog(QWidget *parent, const char *name );

public slots:
  void update_bs(int);
  
private:
};


#endif // MAIN_H
