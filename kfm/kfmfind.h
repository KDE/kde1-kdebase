#ifndef KFMFIND_H
#define KFMFIND_H

#include "kurl.h"

#include <qlined.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qevent.h>

#include "kfmwin.h"

class KFindManager : public KFileManager
{
    Q_OBJECT
public:
    /*********************************************************
     * Create a manager. If '_out_file' is 0L or empty this function
     * will do nothing. Otherwise openDir is called with
     * _out_file as parameter.
     */
    KFindManager( KFileWindow *, KFileView * );
    ~KFindManager();

    /*********************************************************
     * Display search results. The information is in the file
     * _out_file. The parameter is no URL. It is an ordinary filename.
     */
    virtual void openDir( const char *_out_file );
    /*********************************************************
     * Display the message '_msg'.
     */
    virtual void message( const char *_msg );
};

class KFindWindow : public KFileWindow
{
    Q_OBJECT
public:
    /*********************************************************
     * Creates a new window with a search engine in the toolbar.
     * Open the window with a file view of the directory, given by
     * _dir. _dir may be any kind of URL.
     */
    KFindWindow( QWidget *parent=0, const char *name=0, const char *_url=0 );
    ~KFindWindow();

public slots:
    /*********************************************************
     * This function is called when the user presses the 'Do it'
     * button in the toolbar.
     */
    void slotStart();

protected:
    /*********************************************************
     * Override this function to initialize our own KFindManager.
     */
    virtual void initFileManagers();

    /*********************************************************
     * When we are waiting for a search result, this timer will bother
     * us every second. Then we can check wether the find process has
     * finished.
     */
    virtual void timerEvent( QTimerEvent * );

    /*********************************************************
     * Create our toolbar.
     */
    virtual void initToolBar();
    /*********************************************************
     * Create our menubar.
     */
    virtual void initMenu();

    /*********************************************************
     * KFileWindow shows a popup menu if the user drops something
     * over the background. This makes no sense here, so we
     * disable this feature.
     * TODO: Disable only if we show search results.
     */
    virtual void displayDropMenu( int _x ,int _y ) { }

    /*********************************************************
     * The url ( must be file:/... ) in which to start the
     * search.
     */
    QLineEdit *toolDir;
    /*********************************************************
     * The pattern we are searching for.
     */
    QLineEdit *toolName;
    /*********************************************************
     * Just a label.
     */
    QLabel *toolLabel1;
    /*********************************************************
     * Just a label.
     */
    QLabel *toolLabel2;
    /*********************************************************
     * The 'Do it' button, that starts the search.
     */
    QPushButton *toolStart;

    /*********************************************************
     * When we started a file process, we created a timer to watch
     * out for the result every second. This is the timers ID.
     */
    int timerID;
    /*********************************************************
     * When this file is created, the find process is finished.
     */
    QString tmpFile;
    /*********************************************************
     * The find process writes its results to this file.
     */
    QString outFile;

    /*********************************************************
     * The manager used to display search results.
     */
    KFindManager *findManager;
};

#endif
