#ifndef __MAINWIDGET_H__
#define __MAINWIDGET_H__

#include "helpwin.h"
#include "ktopwidget.h"
#include "kstatusbar.h"
#include "ktoolbar.h"

class KHelpMain : public KTopLevelWidget
{
    Q_OBJECT

public:
    KHelpMain(const char *name=NULL);
    virtual ~KHelpMain();

    int openURL( const char *URL, bool withHistory = true );
    void openNewWindow( const char *url );


public slots:
    void slotSaveSession();
    void slotEnableMenuItems();
    void slotNewWindow(const char *url);
    void slotCloneWindow();

    void slotUsingHelp();
    void slotAbout();


    void slotClose();
    void slotQuit();
    void slotOptionsGeneral();
    void slotOptionsToolbar();
    void slotOptionsStatusbar();
    void slotOptionsLocation();
    void slotOptionsSave();

    void slotSetStatusText(const char *text);
    void slotSetTitle(const char *_title);
    void slotSetLocation(const char *url);

    void slotBookmarkChanged(KBookmark *parent);

protected:
    void closeEvent (QCloseEvent *e);
    void resizeEvent(QResizeEvent *);

private:
    void createMenu();
    void createToolbar();
    void createStatusbar();
    void createLocationbar();
    void readConfig();

    void enableMenuItems();
    void fillBookmarkMenu(KBookmark *parent, QPopupMenu *menu, int &id);

private slots:

     void slotLocationEntered();

private:
    KHelpWindow *helpwin;

    KMenuBar *menu;
    KToolBar *toolbar;
    KToolBar *location;
    KStatusBar *statusbar;
    QPopupMenu *fileMenu;
    QPopupMenu *editMenu;
    QPopupMenu *gotoMenu;
    QPopupMenu *bookmarkMenu;
    QPopupMenu *optionsMenu;


    // menu ids
    int idClose;
    int idCopy;
    int idBack;
    int idForward;
    int idDir;
    int idTop;
    int idUp;
    int idPrev;
    int idNext;

    bool showToolBar;
    bool showStatusBar;
    bool showLocationBar;

    static QList<KHelpMain> helpWindowList;
    static KHelpOptionsDialog *optionsDialog;
};






#endif
