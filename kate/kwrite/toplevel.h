#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <qlist.h>
#include <qstring.h>
#include <qpopmenu.h>
#include <qmenubar.h>
#include <qapp.h>

#include <kapp.h>
#include <kconfig.h>
#include <ktoolbar.h>
#include <kstatusbar.h>

#include <ktmainwindow.h>

#include "kwview.h"
#include "kwdoc.h"
/*
class DocSaver : public QObject {
    Q_OBJECT
  public:
    DocSaver() : QObject() {};
  public slots:
    void saveYourself();
};
*/

class TopLevel : public KTMainWindow {
    Q_OBJECT
  public:

    TopLevel(KWriteDoc * = 0);
    ~TopLevel();
    void init(); //initialize caption, status and show

//    virtual void closeEvent(QCloseEvent *e);
    virtual bool queryClose();

    void loadURL(const char *url, int flags = 0);
  protected:
    void setupEditWidget(KWriteDoc *);
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();

    KWrite *kWrite;
//    KWriteDoc *kWriteDoc;
    int menuUndo, menuRedo;
    int menuVertical, menuShowTB, menuShowSB;
//    int statusID, toolID, verticalID, indentID;
//    QPopupMenu *file, *edit, *options, *help;
    QPopupMenu *edit, *options, *recentPopup, *hlPopup, *popup;
    QStrList recentFiles;

    bool hideToolBar;
    bool hideStatusBar;

    QTimer *statusbarTimer;

//    void add_recent_file(const char*);
  public slots:
    void newWindow();
    void newView();
    void closeWindow();
    void quitEditor();

//    void hlDlg();
//    void newHl(int index);

    void toggleStatusBar();
    void toggleToolBar();

    void helpSelected();

    void newCurPos();
    void newStatus();
    void statusMsg(const char *);
    void timeout();
    void newCaption();
    void newUndo();

    void dropAction(KDNDDropZone *);

    void showHighlight();

    //config file functions
  public:
    void readConfig();
  public slots:
    void writeConfig();

    //session management
  public:
    void restore(KConfig *,int);
  protected:
    virtual void readProperties(KConfig*);
    virtual void saveProperties(KConfig*);
    virtual void saveData(KConfig *);
};

#endif
