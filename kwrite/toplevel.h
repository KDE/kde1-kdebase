#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <qlist.h>
#include <qstring.h>
#include <qpopmenu.h>
#include <qmenubar.h>
#include <qapp.h>

//#include <kfm.h>
#include <kapp.h>
//#include <kurl.h>
#include <kconfig.h>
#include <ktoolbar.h>
#include <kstatusbar.h>

#include "kwmainwindow.h"

#include "kwview.h"
#include "kwdoc.h"



class TopLevel : public KWMainWindow {
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
    int statusID, toolID, indentID;
    QPopupMenu *file, *edit, *options, *help;
    QPopupMenu *rightMouseButton, *colors, *recentPopup;
    QStrList recentFiles;

    bool hideToolBar;
    bool hideStatusBar;

    //config functions
    void readConfig();
    void writeConfig();
    //session management
    virtual void saveData(KConfig *);
    void readProperties(KConfig*);
    void saveProperties(KConfig*);
  public:
    void restore(KConfig *,int);

//    void add_recent_file(const char*);
  public slots:
    void newWindow();
    void newView();
    void closeWindow();
    void quitEditor();

    void highlight();
    void newHl(int index);

    void toggleStatusBar();
    void toggleToolBar();

    void helpSelected();

    void newCurPos();
    void newStatus();
    void statusMsg(const char *);
    void newCaption();

    void dropAction(KDNDDropZone *);
};

#endif
