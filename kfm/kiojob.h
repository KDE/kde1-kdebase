#ifndef kiojob_h
#define kiojob_h

class KIOJob;
class KIOServer;
class KIODirectoryEntry;

#include <qlist.h>
#include <qdialog.h>
#include <qstring.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qdict.h>
#include <qlayout.h>

#include <kprogress.h>
#include <kurl.h>

#include "kstrlist.h"
#include "kioserver_ipc.h"

class KIOJob : public QObject
{
    Q_OBJECT
public:
    enum Jobs { JOB_COPY, JOB_MOVE, JOB_DELETE, JOB_MOUNT, JOB_UNMOUNT, JOB_LIST, JOB_MKDIR, JOB_LINK, JOB_GET };
    
    /**
     * Constructs a new job.
     * '_id' can be used to identify the job. Every signal transmitts this id,
     * so the client always knows which job sent him the signal.
     */
    KIOJob( int _id = 0 );
    ~KIOJob();
    
    void doIt( KIOSlaveIPC * _slave );

    /**
     * Turns the status window on or off.
     * By default the window is shown. You can not turn the window
     * off once it appeared.
     */
    void display( bool _display = FALSE ) { bDisplay = _display; }
    
    void copy( QStrList & _src_url_list, const char *_dest_dir_url );    
    void copy( const char *_src_url, const char *_dest_url );
    void get( const char *_url );
    void move( QStrList & _src_url_list, const char *_dest_dir_url );    
    void move( const char *_src_url, const char *_dest_url );
    void link( QStrList & _src_url_list, const char *_dest_dir_url );    
    void link( const char *_src_url, const char *_dest_url );
    void del( QStrList & _url_list );    
    void del( const char *_url );
    /**
     * Mounts a device.
     * '_point' is NOT an URL, it is just an usual path like "/cdrom".
     * You must only specify '_dev' and set the others to 0L, or you must
     * give values for every parameter.
     */
    void mount( bool _ro, const char *_fstype, const char* _dev, const char *_point );
    void unmount( const char *_point );
    /**
     * Gets a directory listing.
     * Connect to 'newDirEntry' to get the directory entries. You must expect that the
     * signal is emitte before this function returns ( if the entries are cached ) or
     * sometimes after the function returned.
     *
     * @param  _reload is TRUE if cached data should be deleted and the directory scanned again.
     * @param _bHTML tells wether we accept HTML code as a reesult. In this case
     *               the signal @ref #data will be emitted multiple times until
     *               the complete HTML code is transfered. For example the 'file'
     *               protocol sends HTML code if it finds some "index.html" file
     *               in the directory.
     */
    void list( const char *_url, bool _reload = FALSE, bool _bHTML = FALSE );
    void mkdir( const char *_url );    

    /**
     * Turns of the global notify signals.
     * If a client wants to handle the notifies by himself, he should call this
     * function and wait for notify signals of the job. Usually the server emits a
     * notify signal so that every client gets informed about every change.
     * To avoid for example, that the KRootWidget gets informed, you must turn of
     * the servers notify signals ( global notify ).
     */
    void noGlobalNotify() { globalNotify = FALSE; }

    /**
     * Deletes all running jobs and their slaves.
     * Do this only if you want to quit the application.
     */
    static void deleteAllJobs();

    /**
     * Allows the job to overwrite existing files during move and
     * copy commands.
     *
     * @see #overwriteExistingFiles
     */
    void KIOJob::setOverWriteExistingFiles( bool _o );

    /**
     * If this flag is set to TRUE, the instance deletes itself after
     * its job is finished. So you can start a KIOJob and dont have to care
     * about it any more. This flag is turned ON by default.
     */
    void setAutoDelete( bool _b ) { bAutoDelete = _b; }
    /**
     * @see #isAutoDelete
     */
    bool isAutoDelete() { return bAutoDelete; }

    /**
     * Call this function only if you received the signal @ref #errorFilter.
     * You can avoid further processing of this error with the help of this function.
     * 
     * @see #processError
     */
    void doNotProcessError() { bProcessError = false; }

    /**
     * If an error arrives @ref #fatalError is called and the error
     * is first emitted via the @ref errorFilter
     * signal. If as a result of this someone calls @ref #processError, then the
     * @ref #bProcessError flag is cleared to prevent calling this function
     * twice. If @ref #processError has not been called, then @ref #fatalError
     * calls it. One can use @ref #doNotProcessError to prevent @ref #fatalError
     * from doing so.
     */
    void processError( int _kioerror, const char* _url, int _errno );
  
    /*
     * hack to get static classes up and running even with C++-Compilers/
     * Systems where a constructor of a class element declared static
     * would never get called (Aix, Alpha,...). In the next versions these
     * elements should disappear.
     */
    static void InitStatic() {
    	passwordDict = new QDict<QString>;
    	jobList = new QList<KIOJob>;
    }

public slots:
    void slaveIsReady();
    void slaveProgress( int _percent );
    void cancel();
    void start( int _pid );
    void slotSlaveClosed( KIOSlaveIPC* );
    
    void slotData( IPCMemory _mem );
    /**
     * This slot is called if the kioslave produced some kind
     * of error. It emits the @ref #errorFilter signal and calls
     * @ref #processError afterwards, except a call to
     * @ref #doNotProcessError told us not to do so or if @ref processError
     * has already been called.
     */ 
    void fatalError( int _kioerror, const char* _url, int _errno );
    void msgResult( QWidget*, int );
    /**
     * Called if the user closes the rename dialog.
     * This dialog is shown if a file already exists but the user job tried to
     * overwrite it.
     */
    void msgResult2( QWidget*, int, const char*, const char* );
    
    void slotDirEntry( const char *_url, const char *_name, bool _isDir, int _size,
		       const char * _creationDate, const char * _access,
		       const char * _owner, const char *_group );
    void slotRedirection( const char *_url );
    void slotMimeType( const char *_type );
    void slotInfo( const char *_text );
    
signals:
    void data( const char *_data, int _len );
    void redirection( const char *_url );
    void mimeType( const char *_type );
    void info( const char *_text );
    
    /**
     * Used for the LIST command.
     * If a client starte a job with 'job->list( url )' he should first
     * connect to this slot to become informed about the progress of the job.
     * _entry may be 0L. So check this. If you get a 'finished' signal, the job will
     * emit no more 'newDirEntry' signals. The '_entry' will be deleted after control
     * gets back to KIOJob. So it is up to you to make a copy for your own.
     */
    void newDirEntry( int _id, KIODirectoryEntry * _entry );
    
    /**
     * Emitted if the job has finished.
     */
    void finished( int _id );

    /**
     * Emitted in @ref #processError. This signal is emitted AFTER the error has been
     * processed. It is just for information. If you want to see errors before they are
     * processed use @ref #errorFilter.
     *
     * @param _txt is generated by @ref #processError. It is NOT the debug string that is
     *             emitted by the kioslave.
     */
    void error( int _kioerror, const char *_txt );
    
    /**
     * This signal is emitted if an error arrives and before it is processed.
     * You may call @ref #processError if you receive this signal. Use
     * @ref #doNotProcessError if you want to process the error on your own.
     *
     * @param _text is the debug text generated by kioslave. Dont show this string
     *              to the user. It is just for debugging.
     */
    void errorFilter( int _kioerror, const char *_text, int _errno );
  
    /**
     * Notify signals indicate that some URLs contents has changed.
     * This signal is only emitted for directories. Use this signal only if
     *  you turned global notifies off. Usually you get notifies from KIOServer.
     */
    void notify( int _id, const char * _url );
    
    /**
     * Tells about the progress of this job in percent.
     *
     * @param _percent is between 0 and 100.
     */
    void progress( int _percent, int _bytesTransfered );
    
    /**
     * Tells about the amount of bytes read.
     * Not implemented yet.
     */
    void bytes( int _bytes );    

protected:  
    /**
     * Looks wether the '_url' specifies a user but not a password. If so the
     * function will ask the user for a password if we dont know it already.
     * The a complete URL us assembled and returned.
     *
     * @return a complete URL with password if needed.
     *
     * @see #passwordDict
     */
    QString KIOJob::completeURL( const char *_url );
    
    void done();
    /**
     * This one makes recursive copy possible.
     * This function assumes that cmSrcURLList and cmDestURLList are filled
     * correctly. It searches in cmSrcURLList for directories. If it finds one,
     * it recursively traverses these and adds those files to the 2 lists.
     * After that is tells the server that this job is waiting for a slave.
     */
    void copy();
    /**
     * This one makes recursive moves possible.
     * This function assumes that cmSrcURLList and cmDestURLList are filled
     * correctly. It searches in cmSrcURLList for directories. If it finds one,
     * it recursively traverses these and adds those files to the 2 lists.
     * After that is tells the server that this job is waiting for a slave.
     */
    void move();
    /**
     * This one makes recursive deletion possible.
     * This function assumes that mvDelURLList is filled correctly.
     * It searches in mvDelURLList for directories. If it finds one,
     * it recursively traverses these and adds those files to the list.
     * After that is tells the server that this job is waiting for a slave.
     */
    void del();
    /**
     * This function does the real linking stuff on local file systems.
     */
    void link();
    
    KIOSlaveIPC *slave;

    /**
     * Used for COPY command.
     */
    KStrList cmSrcURLList;
    KStrList cmDestURLList;
    /**
     * Used to store all directories in a move command.
     * When moving across device, we have to copy and delete. Directoies
     * are not copied themselves but only their files. But they have to
     * be deleted at the end. This list holds them. This list contains only
     * directories on the local file system. This list is used by the
     * del command to, to hold the URLs of all files/dirs that have to
     * be deleted.
     */
    KStrList mvDelURLList;
    
    /**
     * The contents of this list is inverted inserted in @ref #mvDelURLList.
     *
     * @see #move
     * @see #del
     */
    KStrList tmpDelURLList;
    KStrList tmpSrcURLList;
    KStrList tmpDestURLList;
    
    /**
     * Tells which source files should not be worked on.
     *
     * @see move
     */
    KStrList skipURLList;
    
    /**
     * Used to store infos for MOUNT / UNMOUNT command.
     */
    QString mntFSType;
    QString mntDev;
    QString mntPoint;
    bool mntReadOnly;

    /**
     * Used by the command MKDIR.
     */
    QString mkdirURL;
    
    /**
     * Used by DELETE/MOVE/COPY to store total count of files to process.
     */
    int cmCount;
        
    /**
     * Tells wether we already connected slots/signals to the slave.
     */
    bool started;

    /**
     * The URL, as used by the LIST command.
     */
    QString lstURL;
    
    KIOServer *server;

    /**
     * The action that is currently taking place.
     */
    Jobs action;

    /**
     * The dialog. May be 0L.
     */
    QDialog *dlg;
    /**
     * The dialogs progress bar. May be 0L.
     */
    KProgress *progressBar;
    /**
     * May be 0L.
     */
    QLabel *line1;
    /**
     * May be 0L.
     */
    QLabel *line2;
    /**
     * May be 0L.
     */
    QLabel *line3;
    /**
     * May be 0L.
     */
    QVBoxLayout *layout;
    
    /**
     * The modus when moving files.
     * If we want to move files across devices, we have to copy and then delete
     * them. This flag indicates wether we should delete a file next turn,
     * or copy the next one.
     */
    bool moveDelMode;

    /**
     * Flag indicated wether the slave had been told to clean up.
     * When the ob does not need the slave, the slave is told to clean up and
     * this flag is set. If the slave tells that he did the clean up, the dialog
     * window ( if any ) well be deleted and the job is deleted. But mention
     * that the cleanup may raise errors.
     */
    bool cleanedUp;

    /**
     * List of URLs to notify.
     * When a job is completed a notify signal is emitted for every URL in
     * this list.
     */
    KStrList notifyList;

    /**
     * The last error.
     * If 'fatalError' is called, this variable holds the error value. This is
     * used later by 'msgResult'.
     */
    int kioError;

    /** Flag for global notifies.
     * If this flag is FALSE, the server wont be told to emit notify signals.
     */
    bool globalNotify;

    /**
     * A unique ID.
     */
    int id;

    /**
     * A flag that shows wether the job should open a status window.
     */
    bool bDisplay;

    /**
     * List of all running jobs.
     */
    static QList<KIOJob> *jobList;

    /**
     * This flag is set to TRUE if existing files should be overwritten by copy/move jobs.
     */
    bool overwriteExistingFiles;
    
    /**
     * The last source we tried to copy/move.
     * This info is needed if the file already exists, to show the right URLs in the dialog.
     */
    QString lastSource;
    /**
     * The last destination of copy/move.
     * This info is needed if the file already exists, to show the right URLs in the dialog.
     */
    QString lastDest;

    /**
     * The keys for this dict look like "user@host".
     *
     * @see #completeURL
     */
    static QDict<QString> *passwordDict;

    /**
     * @see #setAutoDelete
     */
    bool bAutoDelete;
    /**
     * If we accept HTML as result of a @ref #list call, then this FLAG is set to TRUE.
     */
    bool bHTML;

    /**
     * @see #processError
     * @see #fatalError
     */
    bool bProcessError;
};

#include "kioserver.h"

#endif
