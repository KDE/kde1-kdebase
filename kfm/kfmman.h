#ifndef kfmman_h
#define kfmman_h

class KFMManager;

#include "kfmview.h"
#include "htmlcache.h"
#include "cgi.h"
#include "kfmjob.h"

class QFontMetrics;

class KFMManager : public QObject
{
    Q_OBJECT
public:
    KFMManager( KfmView *_v );
    virtual ~KFMManager();
    
    /**
     * @return the current URL, managed by this object
     */
    QString& getURL() { return url; }

    /**
     * Open a new URL.
     */
    virtual bool openURL( const char *_url, bool _refresh = FALSE );
    /**
     * The user pressed the right mouse button over _url at point _p.
     */
    virtual void openPopupMenu( QStrList & _url, const QPoint &_p );
    /**
     * The user dropped the URLs _source over the URL _url at the point _p.
     */
    virtual void dropPopupMenu( KDNDDropZone *_zone, const char *_url, const QPoint *_p );

    /**
     * Stop the @ref #KFMJob that downloads the directory information
     */
    virtual void stop();
    
public slots:
    /**
     * A item of the popup menu has been selected.
     */
    void slotPopupActivated( int _id );

    /**
     * Notify about new directory entries.
     * If a KIOJob is started with the command 'list', then this slot
     * is called once for every file in the directory.
     */
    void slotNewDirEntry( KIODirectoryEntry * _entry );

    void slotDropLink();
    /**
     * Bound to a popup menu.
     * If the user dropped files over a directory and has chosen the command
     * 'copy' from the popup menu, this function is called.
     * 'dropZone' knows which files were dropped and 'dropDestination' knows
     * the directory in which to copy these files/directories.
     */
    void slotDropCopy();
    /**
     * Bound to a popup menu.
     * If the user dropped files over a directory and has chosen the command
     * 'move' from the popup menu, this function is called.
     * 'dropZone' knows which files were dropped and 'dropDestination' knows
     * the directory in which to move these files/directories.
     */
    void slotDropMove();

    void slotError( const char *_text );
    void slotFinished();
    void slotData( const char *_data );
    void slotMimeType( const char *_type );
    void slotInfo( const char *_text );
    
protected:
    void writeBeginning();
    void writeEntry( KIODirectoryEntry *_e );

    /**
     * This fucntion is used to distinguish hard coded bindings and bindings belonging
     * to applications. The only thing we know is the bindings name '_txt'.
     *
     * @return TRUE if '_txt' is the name of a hard coded binding like "Copy", "Link" etc.
     *
     * @see KFileManager::slotPopupActivated
     */
    bool isBindingHardcoded( const char *_txt );

    /**
     * Used for the popup menu.
     */
    bool eventFilter( QObject *, QEvent * );

    void writeWrapped( char * );
    
    /**
     * List of all files in the directory.
     * is list is filled by 'slotNewDirEntry'
     */
    QList<KIODirectoryEntry> files;

    /**
     * Dstination of a drop that just happened.
     *  When a drop occured this is the URL over which the user released the mouse.
     * This can be the directory the file view is showing right now or a file / directory 
     * mentioned in the HTML code that is displayed in the file view widget.
     * This is string is used by 'slotDropCopy' etc. to remeber the drops destination.
     */
    QString dropDestination;
    /**
     * DropZone of the last drop.
     * When a drop occured and a popup menu is opened then this is the DropZone
     * belonging to this drop. 'slotDropCopy' for example use this to get information
     * about which files the user dropped.
     */
    KDNDDropZone *dropZone;

    /**
     * The job that is currently running.
     *
     * @see #tryURL
     */
    KFMJob *job;
    /**
     * The URL we are currenty trying to load.
     *
     * @see #job
     */
    QString tryURL;
    /**
     * This flag is set if the current @ref KAbstractManager::url is a native HTML page ( this means
     * that it was not KFMs job to create this page from a list of @ref KIODirectoryEntry.
     */
    bool bHTML;
    /**
     * Tells us wether the the current operartion has been finished.
     *
     * @see #job
     */
    bool bFinished;
    bool bHistoryStackLock;

    /**
     * The view this manager has to manage. 
     */
    KfmView *view;

    /**
     * The currently dispayed URL. This may differ from the URL we are trying to
     * load at a time.
     */
    QString url;

    /**
     * The URLs belonging to the opened popupMenu.
     */
    KStrList popupFiles;

    /**
     * The last opened popup menu.
     * Delete the old one before creating a new one.
     */
    QPopupMenu *popupMenu;

    /**
     * The mouseposition when opening the popup Menu.
     *
     * @see #popupMenu
     */
    QPoint popupMenuPosition;
    
    /**
     * The font metrics of the default HTML font that is used for icon labels.
     *
     * @see #writeWrapped
     */
    QFontMetrics *labelFontMetrics;
    /**
     * The maximum width of icon labels
     *
     * @see #writeWrapped
     */
    int maxLabelWidth;
};

#endif


