#ifndef kfmman_h
#define kfmman_h

class KAbstractManager;
class KFileManager;
class KTarManager;
class KFtpManager;
class KHttpManager;
class KCgiManager;

#include "kfmview.h"
#include "htmlcache.h"
#include "cgi.h"

class QFontMetrics;

/**
 * For every protocol there is a manager. The manager is repsponsible
 * for creating HTML code and for actions on the URLs he supports.
 * The user may for example double click on a URL or press
 * the right mouse button.
 */
class KAbstractManager : public QObject
{
    Q_OBJECT
public:
    KAbstractManager( KfmView *_v );
    virtual ~KAbstractManager();
    
    /**
     * Display _url in the window.
     * If the URL opened causes a repaint of the KViewWindow, then the function
     * returns TRUE. If for example the URL could not be opened or if the URL
     * was an executable ( which means that the executable had been started )
     * or if a new window has been opened, the function will return FALSE.
     * openURL can open a new directory, start an executable or start for example
     * Netscape with '_url' as argument. If _refresh is TRUE, then the same
     * URL is already displayed. In this case some files changed and an update
     * has to be done.
     */
    virtual bool openURL( const char *, bool = FALSE ) { 
    	// Stephan: I'm not that sure, how to handle this
    	return true;
    }
  
    /// The user pressed the right mouse button over _url (or some selected URLs ) at point _p.
    /**
      You can not expect that you are the 'actualManager' of KFileWindow
      when this function is called. For example KHttpManager
      */
    virtual void openPopupMenu( QStrList & , const QPoint & ) { }
    /// The user dropped the URLs _source over the URL _url at the point _p.
    virtual void dropPopupMenu( KDNDDropZone *, const char *, const QPoint * ) { }

    /// The user pressed the stop button
    /**
      Usually this will interrupt a FTP or HTTP connection.
      */
    virtual void stop() { }
    
    /// Return the current URL, managed by this object
    QString& getURL() { return url; }

protected:
    /**
     * This fucntion is used to distinguish hard coded bindings and bindings belonging
     * to applications. The only thing we know is the bindings name '_txt'.
     *
     * @return TRUE if '_txt' is the name of a hard coded binding like "Copy", "Link" etc.
     *
     * @see KFileManager::slotPopupActivated
     */
    bool isBindingHardcoded( const char *_txt );

    /// The KFileWindows view.
    KfmView *view;

    /// The currently managed URL.
    QString url;

    /// The URLs belonging to the opened popupMenu
    KStrList popupFiles;

    /// The last opened popup menu.
    /**
      Delete the old one before creating a new one.
      */
    QPopupMenu *popupMenu;

    /// and the mouseposition when opening it (Matthias)
    QPoint popupMenuPosition;
    
    bool eventFilter(QObject *, QEvent *);

    /// the font metrics of the default HTML font that is used for icon labels
    QFontMetrics *labelFontMetrics;

    /// the maximum width of icon labels
    int maxLabelWidth;

    void writeWrapped(char *);          
};

/// Manager for the local file system
/**
  This manager is responsible for making HTML code out of the local file
  system and for interactions of the user with the URLs contained in
  the produced HTML code.
  */
class KFileManager : public KAbstractManager
{
    Q_OBJECT
public:
    /// Constructor
    /**
     If _view is 0L, then the manager is used by the root widget.
     In this case he has to open a new window every time.
     */
    KFileManager( KfmView *_view );
    /// Destructor
    virtual ~KFileManager();
    
    /// Display _url in the window.
    virtual bool openURL( const char *_url, bool _refresh = FALSE );
    /// Open the .kde.html file and display it
    /**
      If the directory we opened has a .kde.html or index.html file, then the
      file name of this html file is stored in '_filename'.
      The functions displays the HTML source in the 'KFileView' widget.
      It is usually called from 'openDir'.
      'refresh' has the same meaning like the refresh parameter in 'openURL'.
     */
    virtual void openDirHTML( const char *_filename, bool _refresh );
    /// The user pressed the right mouse button over _url at point _p.
    virtual void openPopupMenu( QStrList & _url, const QPoint & _point );
    /// The user dropped the URLs stored in _zone over the URL _dest at the point _p.
    virtual void dropPopupMenu( KDNDDropZone *_zone, const char *_dest, const QPoint *_p );

public slots:
    /// A item of the popup menu has been selected.
    void slotPopupActivated( int _id );

    /// Bound to a popup menu.
    /**
      If the user dropped files over a directory and has chosen the command
      'copy' from the popup menu, this function is called.
      'dropZone' knows which files were dropped and 'dropDestination' knows
      the directory in which to copy these files/directories.
      */
    void slotDropCopy();
    /// Bound to a popup menu.
    /**
      If the user dropped files over a directory and has chosen the command
      'move' from the popup menu, this function is called.
      'dropZone' knows which files were dropped and 'dropDestination' knows
      the directory in which to move these files/directories.
      */
    void slotDropMove();
    /// Bound to a popup menu.
    /**
      If the user dropped files over a directory and has chosen the command
      'link' from the popup menu, this function is called.
      'dropZone' knows which files were dropped and 'dropDestination' knows
      the directory in which to link these files/directories.
      */
    void slotDropLink();

protected:

    /// Destination of a drop that just happened.
    /**
      When a drop occured this is the URL over which the user released the mouse.
      This can be the directory the file view is showing right now or a file / directory 
      mentioned in the HTML code that is displayed in the file view widget.
      This is string is used by 'slotDropCopy' etc. to remeber the drops destination.
     */
    QString dropDestination;
    /// DropZone of the last drop
    /**
      When a drop occured and a popup menu is opened then this is the DropZone
      belonging to this drop. 'slotDropCopy' for example use this to get information
      about which files the user dropped.
      */
    KDNDDropZone *dropZone;
};

/// Manager for the HTTP protocol
/**
  Responsible for transfering HTML code from KIOManager to KViewWindow.
  */
class KHttpManager : public KAbstractManager
{
    Q_OBJECT
public:
    /// Constructor
    KHttpManager( KfmView *_view );
    /// Destructor
    virtual ~KHttpManager();

    /// Tell KIOJob to download the HTML page _url.
    virtual bool openURL( const char *_url, bool _refresh = FALSE );
    /// The user pressed the right mouse button over _url at point _p.
    virtual void openPopupMenu( QStrList &_url, const QPoint &_p );

    /// Stop the KIOJob that downloads the HTML code
    virtual void stop();

public slots: 
    /// A item of the popup menu has been selected.
    void slotPopupActivated( int _id );

    /// Called if the HTML document arrived.
    void slotShowHTML( const char *_url, const char *_filename );
    
    /**
     * Called if the job makes any progress.
     */
    void slotProgress( const char *_url, const char *_filename,
		       int _percent, int _bytes, float _rate, bool _stalled );
    
protected:
    //@Man: Variables
    //@{
    
    /// Flag that indicates wether we are downloading a HTML document right now
    bool running;

    /// The Cache
    HTMLCache *htmlCache;

    /**
     * Amount of bytes we alread read and wrote into the HTML widget.
     */
    int bytesRead;
    
    /**
     * This file handle is used to read the file, kioslave is writing to.
     */
    FILE *f;
    //@}
};

/// Manager for tar files.
/**
  Responsible for generating HTML code for the tar file.
  */
class KTarManager : public KAbstractManager
{
    Q_OBJECT
public:
    /// Constructor
    KTarManager( KfmView *_view );
    /// Destructor
    virtual ~KTarManager();

    /// Open a new URL
    virtual bool openURL( const char *_url, bool _refresh = FALSE );
    /// The user pressed the right mouse button over _url at point _p.
    virtual void openPopupMenu( QStrList & _url, const QPoint &_p );
    
public slots: 
    /// A item of the popup menu has been selected.
    void slotPopupActivated( int _id );

    /// Notify about new directory entries
    /**
      If a KIOJob is started with the command 'list', then this slot
      is called once for every file in the directory.
      */
    void slotNewDirEntry( int _id, KIODirectoryEntry * _entry );

    /// Shows the files listed in 'files'.
    /**
      This function produces the HTML output and starts the
      parsing process of the HTML code.
      */
    void slotShowFiles( int _id );

protected:

    /// List of all files in the directory
    /**
      This list is filled by 'slotNewDirEntry'
      */
    QList<KIODirectoryEntry> files;
};

/// Manager for the FTP protocol
/**
  Responsible for creating HTML code for a ftp directory.
  */
class KFtpManager : public KAbstractManager
{
    Q_OBJECT
public:
    /// Constructor
    KFtpManager( KfmView *_view );
    /// Destructor
    virtual ~KFtpManager();

    /// Open a new URL
    virtual bool openURL( const char *_url, bool _refresh = FALSE );
    /// The user pressed the right mouse button over _url at point _p.
    virtual void openPopupMenu( QStrList & _url, const QPoint &_p );
    /// The user dropped the URLs _source over the URL _url at the point _p.
    virtual void dropPopupMenu( KDNDDropZone *_zone, const char *_url, const QPoint *_p );

    /// ???
    void invalidateDirs();

    /// Stop the KIOJob that downloads the directory information
    virtual void stop();
    
public slots:
    /// A item of the popup menu has been selected.
    void slotPopupActivated( int _id );

    /// Notify about new directory entries
    /**
      If a KIOJob is started with the command 'list', then this slot
      is called once for every file in the directory.
      */
    void slotNewDirEntry( int _id, KIODirectoryEntry * _entry );

    /// Bound to a popup menu.
    /**
      If the user dropped files over a directory and has chosen the command
      'copy' from the popup menu, this function is called.
      'dropZone' knows which files were dropped and 'dropDestination' knows
      the directory in which to copy these files/directories.
      */
    void slotDropCopy();
    /// Bound to a popup menu.
    /**
      If the user dropped files over a directory and has chosen the command
      'move' from the popup menu, this function is called.
      'dropZone' knows which files were dropped and 'dropDestination' knows
      the directory in which to move these files/directories.
      */
    void slotDropMove();

    /// Shows the files listed in 'files'.
    /**
      This function produces the HTML output and starts the
      parsing process of the HTML code, once 'files' is totally
      filled up.
      */
    void slotShowFiles( int _id );

    /** 
     * Called from @ref #timer every 5 seconds to check wether the transfer
     * stalled.
     */
    void slotProgressTimeout();
    
    /**
     * Called from the @ref KIOJob if the job makes any progress.
     */
    void slotProgress( int _percent, int _bytesTransfered );
    
protected:
    void writeBeginning();
    void writeEntry( KIODirectoryEntry *_e );
    
    /// List of all files in the directory
    /**
      This list is filled by 'slotNewDirEntry'
      */
    QList<KIODirectoryEntry> files;

    /// Destination of a drop that just happened.
    /**
      When a drop occured this is the URL over which the user released the mouse.
      This can be the directory the file view is showing right now or a file / directory 
      mentioned in the HTML code that is displayed in the file view widget.
      This is string is used by 'slotDropCopy' etc. to remeber the drops destination.
     */
    QString dropDestination;
    /// DropZone of the last drop
    /**
      When a drop occured and a popup menu is opened then this is the DropZone
      belonging to this drop. 'slotDropCopy' for example use this to get information
      about which files the user dropped.
      */
    KDNDDropZone *dropZone;

    /// The job that is currently running
    KIOJob *job;

    /**
     * Time elapsed since job started.
     */
    QTime timer1;
    /**
     * Time elapsed since last bytes arrived.
     */
    QTime timer2;
    /**
     * Calls us every 5 seconds to check wether the transfer is stalled.
     */
    QTimer *timer;

    /**
     * The amount of bytes transfered right now.
     */
    int bytesTransfered;
    /**
     * The percent of the file that is already downloaded.
     */
    int percent;
};

/**
 * @short Manager for the local CGI protocol
 */
class KCgiManager : public KAbstractManager
{
    Q_OBJECT
public:
    KCgiManager( KfmView *_view );
    virtual ~KCgiManager();

    /**
     * Tell the CGI-Server to run the cgi script.
     *
     * @sse KCGI
     */
    virtual bool openURL( const char *_url, bool _refresh = FALSE );
    /**
     * The user pressed the right mouse button over _url at point _p.
     */
    virtual void openPopupMenu( QStrList &_url, const QPoint &_p );

    /**
     * Stop the CGI-Server that runs the script.
     *
     * @see KCGI
     */
    virtual void stop();

public slots: 
    /**
     * A item of the popup menu has been selected.
     */
    void slotPopupActivated( int _id );

    /**
     * Called if the HTML document arrived.
     */
    void slotShowHTML();
    
protected:
    /**
     * Flag that indicates wether we are waiting for a running script right now.
     */
    bool running;

    /**
     * This server is used to run local CGI programs.
     */
    KCGI *cgiServer;

    /**
     * The file used to save the output of the CGI script.
     */
    QString tmpFile;
};

#endif


