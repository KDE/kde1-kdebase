#ifndef _kfmexec_h
#define _kfmexec_h

#include <qobject.h>
#include <qstring.h>
#include <qdialog.h>

#include "kfmjob.h"

/**
 * @author Torben Weis, weis@kde.org
 * @short Executes random URLs
 *
 * If you have a single URL and you want to run the default action, then just
 * create a KFMExec instance and call @ref #openURL.
 * The class tries to determine wether the URL is a directory or a HTML file
 * or something different. For directories and HTML a new KFM window
 * will be opened. For all the other files we try to find out the mime type
 * and run the default binding. If we have no success, the user is asked
 * what to do. There is no need for you to destroy KFMExec jobs. They
 * kill themselves after they finished their job. So dont use the pointer
 * to a KFMExec class after calling @ref #openURL. KFMExec shows a window
 * if it talks to the Internet to show the user that something is done.
 * For example @ref KRootWidget uses this class to execute URL links
 * lying around on the desktop.
 */
class KFMExec : public QObject
{
    Q_OBJECT
public:
    KFMExec();
    ~KFMExec();
    
    /**
     * Tells KFMExec to run the default action for the given URL.
     * After calling this function dont reference this instance any
     * more. It will destroy itself after its job is done.
     */
    void openURL( const char *_url );
    
    /**
     * This function handles some special stuff for local files. For example
     * it changes the URL, if the file is zipped or a tar file. The changed
     * URL is then returned. Sometimes the function can execute the URL
     * itself ( executables, mime-type with binding ). Then an empty string
     * is returned. This function is used by KFMExec and @ref KFMManager.
     */
    static QString openLocalURL( const char *_filename );

    /**
     * Internal function. Used to find out wether the job
     * of this instance is already finished. In this case
     * we can run some garbage collection ( e.g. delete this
     * instance.
     */
    bool isDone() { return bDone; }
    
public slots:
   /**
    * Called if the @ref #job raises an error.
    */
    void slotError( int, const char * );
   /**
    * Called when the @ref #job finishes.
    */
    void slotFinished( );
    /**
     * Called if the @ref #job found a directory entry.
     * In this case we can assume that we found a directory.
     */
    void slotNewDirEntry( KIODirectoryEntry * _entry );
    /**
     * Called if the @ref #job found some information about
     * the mime-type.
     *
     * @param _type may be somethinh like "text/html". It may be 0L
     *              if @ref #job does not know anything about the mime type.
     */
    void slotMimeType( const char *_type );

    /**
     * Called if the user presses the dialogs cancel button.
     */
    void slotCancel();
    
protected:
    void prepareToDie();
    
    /**
     * The URL we try to open.
     */
    QString tryURL;
    /**
     * The job we are using to get some informations about
     * @ref #tryURL.
     */
    KFMJob *job;
    /**
     * Set to tryURL if a list command works on it (@see #slotNewDirEntry)
     */
    QString dirURL;

    /**
     * Shows us wether out job is finished and wether we are
     * prepared to die.
     *
     * @see #isDone
     */
    bool bDone;
    /**
     * In some cases we show a dialog to tell the user that we
     * are doing something at least.
     */
    QDialog *dlg;

    static QList<KFMExec> *lstZombies;
};

#endif





