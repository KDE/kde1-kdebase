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

class DocSaver : public QObject {
    Q_OBJECT
  public:
    DocSaver() : QObject() {};
  public slots:
    void saveYourself();
};


class TopLevel : public KTMainWindow {
    Q_OBJECT
  public:

    TopLevel(KWriteDoc * = 0);
    ~TopLevel();

    virtual void closeEvent(QCloseEvent *e);
    virtual bool queryExit();

    void loadURL(const char *url);
  protected:
    void setupEditWidget();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();

    KWrite *kWrite;
    KWriteDoc *kWriteDoc;
    int menuUndo, menuRedo;
    int menuVertical, menuShowTB, menuShowSB;
//    int statusID, toolID, verticalID, indentID;
    QPopupMenu *file, *edit, *options, *help;
    QPopupMenu *rightMouseButton, *colors, *recentPopup;
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

    void hlDlg();
    void newHl(int index);

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


    //config file functions
  public:
    void readConfig();
  public slots:
    void writeConfig();
    //session management
  public:
//    virtual void saveData(KConfig *);
    void readProperties(KConfig*);
    void restore(KConfig *,int);
    void saveProperties(KConfig*);
};

#endif
