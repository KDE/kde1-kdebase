#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <qlist.h>
#include <qstring.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qapplication.h>

#include <kapp.h>
#include <kconfig.h>
#include <ktoolbar.h>
#include <kstatusbar.h>

#include <ktmainwindow.h>

#include "kwview.h"
#include "kwdoc.h"
#include "kguicommand.h"

class TopLevel : public KTMainWindow {
    Q_OBJECT
  public:

    TopLevel(KWriteDoc * = 0L, const char *path = 0L);
    ~TopLevel();
    void init(); //initialize caption, status and show

    virtual bool queryClose();
    virtual bool queryExit();

    void loadURL(const char *url, int flags = 0);
  protected:
    void setupEditWidget(KWriteDoc *);
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();

    KWrite *kWrite;
    int menuUndo, menuRedo;
    int menuVertical, menuShowTB, menuShowSB, menuShowPath;
//    int statusID, toolID, verticalID, indentID;
//    QPopupMenu *file, *help;
    KGuiCmdPopup *edit, *options;
    QPopupMenu *recentPopup, *hlPopup, *eolPopup;//, *popup;
    QStrList recentFiles;

    bool hideToolBar;
    bool hideStatusBar;
    bool showPath;

    QTimer *statusbarTimer;

  public slots:
    void openRecent(int);
    void newWindow();
    void newView();
    void closeWindow();
    void quitEditor();

   void keyDlg();
    void toggleStatusBar();
    void toggleToolBar();
    void togglePath();

    void helpSelected();

    void newCurPos();
    void newStatus();
    void statusMsg(const char *);
    void timeout();
    void newCaption();
    void newUndo();

    void dropAction(KDNDDropZone *);

    void showHighlight();
    void showEol();

    //config file functions
  public:
    //common config
    void readConfig(KConfig *);
    void writeConfig(KConfig *);
    //config file
    void readConfig();
  public slots:
    void writeConfig();
    //session management
  public:
    void restore(KConfig *,int);
  protected:
    virtual void readProperties(KConfig *);
    virtual void saveProperties(KConfig *);
    virtual void saveData(KConfig *);
};

#endif
