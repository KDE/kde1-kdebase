#ifndef kfmman_h
#define kfmman_h

class KFMManager;

#include "kfmview.h"
#include "htmlcache.h"
#include "kfmjob.h"

#include "popup.h"

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
    QString& getURL() { return url.isEmpty() ? tryURL : url; }

    /**
     * Open a new URL.
     */
    virtual bool openURL( const char *_url, bool _refresh = FALSE, int _x_offset = 0, int _y_offset = 0, const char *_data = 0L );
    /**
     * The user pressed the right mouse button over _url at point _p.
     *
     * @param _point is already in global coordinates.
     * @param _current_dir inidcates that we want to open the context menu
     *                     for the current directory. In this case we dont want
     *                     to offer 'delete' for example.
     */
    virtual void openPopupMenu( QStrList & _url, const QPoint &_p, bool _current_dir = false );
    /**
     * The user dropped the URLs _source over the URL _url at the point _p.
     * 
     * @param _nestedURLs indicates wether the URLs in '_zone' include a directory which
     *                    is equal to '_url' or at least a child of '_url'. In this case
     *                    only linking is allowed.
     */
    virtual void dropPopupMenu( KDNDDropZone *_zone, const char *_url, const QPoint *_p, bool _nestedURLs = false );

    /**
     * Stop the @ref #KFMJob that downloads the directory information
     */
    virtual void stop();

    /**
     * Is this a native HTML page ( this means that it was not KFMs job
     * to create this page from a list of @ref KIODirectoryEntry).
     */
    bool isHTML() const { return bHTML; }

    const char* getJobURL() { return jobURL.data(); }

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

    void slotError( int _kioerror, const char *_text );
    void slotFinished();
    void slotData( const char *_data, int _len );
    void slotMimeType( const char *_type );
    void slotInfo( const char *_text );
    void slotRedirection( const char *_url );
    void slotCookie( const char *_url, const char *_cookie_str );

    void setDefaultTextColors( const QColor& textc,const QColor& linkc,
			       const QColor& vlink);

    void setDefaultBGColor( const QColor& bgcolor );    
protected:
    /**
     * Writes the <body ...> tag for @ef #writeBeginning .
     * It has a look at .diretory files if available.
     */
    void writeBodyTag();
    void writeBeginning();
    void writeEntry( KIODirectoryEntry *_e );
    void writeEnd();
  
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

    /**
     * @param _maxwidth If no value is specified, then '_maxwidth' is set to
     *                  @ref #maxLabelWidth.
     */
    void writeWrapped( char *, int _maxwidth = -1 );

    /**
     * Produces the image tag to display the icon if the visual schnauzer
     * is turned on.
     */
    QString getVisualSchnauzerIconTag( const char *_url );

    /**
     * Sets the 'up url' in the kfmview.
     */
    void setUpURL();
    
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
     * This is the parameter passed to @ref #openURL. Sometimes we have to call
     * this function recursively with another URL, but this parameter should stay
     * the same. That is the reason why we have to store it here.
     */
    bool bReload;
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

    /*
     * The 'real' URL. e.g. after a redirection, or with 'index.html'
     * appended to it if HTML View is on.
     */
    QString jobURL;

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
    /**
     * The HTML page of the currently displayed page is stored here.
     * This is for example useful to checkin the page into the HTML cache
     * after it is entirely loaded.
     */
    QString pageBuffer;
    /**
     * If we know that we get the HTML stuff fast, we buffer the HTML code
     * and write it to the widget in one write command. This flag indicates
     * wether we want to do so.
     */
    bool bBufferPage;

    /**
     * If we call @ref KHTMLWidget::begin next time, then we get the
     * vertical offset from this variable. It is set by a call to
     * @ref #openURL.
     */
    int nextYOffset;
    /**
     * If we call @ref KHTMLWidget::begin next time, then we get the
     * horizontal offset from this variable. It is set by a call to
     * @ref #openURL.
     */
    int nextXOffset;  

    QString text_color;
    QString link_color;
    QString bg_color;
    QString vlink_color;
  
    // link and readonly overlay images
    static QString *link_overlay;
    static QString *ro_overlay;

    /**
     * The menu "New" in the popup menu.
     * Since the items of this menu are not connected themselves
     * we need a pointer to this menu to get information about the
     * selected menu item.
     */
    KNewMenu *menuNew;
    //-------- Sven's overlayed mime/app dirs end ---
    /**
     * Flag set by @ref #openURL if we read mime/applnk dir or
     * one of it's subdirs. Cleared in @ref #slotFinished or @ref #stop .
     */
    bool bindingDir;
    
    /**
     * Flag set by @ref #slotFinished Cleared in @ref #slotFinished or
     * @ref #stop . True if we do 2nd pass, false if 1st pass.
     */
    bool pass2;

    /**
     * List of duplicated list entries for overlay stuff. We must have
     * our own list. This is a pointer so we don't waste any memory -
     * changing mime/applnks is rare operation. List is autoDelete,
     * and has deep copies. Filled and tested for dups in @ref
     * #writeEntry , created in @ref #openURL , zeroed & deleted in
     * in @ref #slotFinished or @ref #stop .
     */
    QStrList *dupList;
    //-------- Sven's overlayed mime/app dirs start ---
};

#endif


