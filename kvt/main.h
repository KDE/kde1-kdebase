//
// kvt. Part of the KDE project.
//
// Copyright (C) 1996 Matthias Ettrich
//

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
#include <kmenubar.h>

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
    kVt( QWidget *parent=0, const char *name=0 );
  // public because this need to be set from old rxvt-C-code
    QScrollBar* scrollbar;
  void ResizeToVtWindow();
  void setMenubar(bool);
  void setScrollbar(bool);
  void ResizeToDimen(int width, int height);

public slots:
    void application_signal();
    void options_menu_activated( int );
    void scrollbar_menu_activated( int );
    void size_menu_activated( int );
    void dimen_menu_activated(int);
    void color_menu_activated( int );
    void file_menu_activated(int);
    void help_menu_activated(int);
    void scrolling(int);
    void onDrop( KDNDDropZone* _zone );

    void menubarMoved();


protected:
    void    resizeEvent( QResizeEvent * );
    bool eventFilter( QObject *, QEvent * );

private:
    KMenuBar *menubar;
    QFrame *frame;
    QPopupMenu *m_file;
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

  // options
       
  KConfig* kvtconfig;

  Bool menubar_visible;
  Bool scrollbar_visible;
  KvtScrollbar kvt_scrollbar;
  KvtSize kvt_size;
  KDNDDropZone    *dropZone;
};

class CharClass : public QDialog {
  Q_OBJECT
public:
  QLineEdit *chars;

  CharClass(QWidget *parent, const char *name );
};




#endif // MAIN_H


