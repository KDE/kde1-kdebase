// KIOSlave
// Server side implementation

#ifndef kioserver_h
#define kioserver_h

#include <qobject.h>
#include <qlist.h>
#include <qdict.h>
#include <qstring.h>

#include <kurl.h>

#include "kioserver_ipc.h"

//@Man: KIO Modes
//@{
/** Modes used for the static 'supports' functions of the KIOs */
/// Read from the KIO
#define KIO_Read 1
/// Write to the KIO
#define KIO_Write 2
/// Make a directory
#define KIO_MakeDir 4
/// Delete a file
#define KIO_Delete 8
/// Link a file
#define KIO_Link 16
/// List directory entries
#define KIO_List 32
/// Moves files
#define KIO_Move 64
//@}

class KIOServer;
class KIODirectoryEntry;

class KIODirectoryEntry
{
public:
    KIODirectoryEntry( const char *_name, bool _isDir, int _size = -1, const char * creationDate = 0L,
		       const char * _access = 0L, const char * _owner = 0L, const char *_group = 0L );
    
    KIODirectoryEntry( KIODirectoryEntry & _entry );
    
    bool isDir() { return bDir; }
    /**
     * @return TRUE if we are shure that the entry is a real file. Mention that it may
     *         be a link to some file never the less. But in the case of an link
     *         we can not be shure, so FALSE is returned.
     */
    bool isFile() { return ( access.left(1) == "-" ); }
    /**
     * @return the name of the entry. Mention that this is not a URL.
     */
    const char* getName() { return name.data(); }
    /**
     * @return an access string like you get it by "ls -l"
     */
    const char* getAccess() { return access.data(); }    
    const char* getOwner() { return owner.data(); }
    const char* getGroup() { return group.data(); }
    /**
     * @return an date string like you get it by "ls -l".
     *         Dont make too many assumptions about the format of this string.
     *         Yust display the returned string. Parsing on your own risk :-)
     */
    const char* getCreationDate() { return creationDate.data(); }
    int getSize() { return size; }

    bool mayRead( const char *_user );
    bool mayWrite( const char *_user );
    bool mayExec( const char *_user );
  
protected:
    bool bDir;
    QString name;
    int size;
    QString creationDate;
    QString access;
    QString owner;
    QString group;
};

typedef QList<KIODirectoryEntry> KIODirectory;

#include "kiojob.h"

class KIOServer : public KIOSlaveIPCServer
{
    Q_OBJECT
public:
    KIOServer();
    ~KIOServer();
    
    /// A Job is waiting for a slave
    /**
      If a job needs a slave, it calls this function. If a 
      slave is available ( after this call has returned ), the
      job will be told so.
      */
    void getSlave( KIOJob * _job );
    /// A Job does not need a slave any more.
    void freeSlave( KIOSlaveIPC * _slave );

    /**
     * Get information about a cached directory.
     *
     * @return 0L if the directory is not cached.
     */
    KIODirectory* getDirectory( const char *_url );
    /**
     * @return a pointer to a @ref KIODirectoryEntry for the given URL
     *         or 0L if no such entry exists.
     */
    KIODirectoryEntry* getDirectoryEntry( const char *_url );
    /// Return a pointer to the running KIOServer
    static KIOServer* getKIOServer() { return pKIOServer; }
    /// Get a new name for a link.
    /**
      If the user wants to make a link to an URL we need to make a valid file
      name from the URL because the link needs a name.
      */
    static QString getDestNameForLink( const char * _url );
    /// Tests wether the list of URLs are all together directories
    /**
      If the protocol of the URL is not "file", then this function
      depends an the convention, that directories have a trailing '/'
      in their URL.
      
      @return 1 if the URL is a directory, 0 if not and <0 if we are unshure.
      */
    static int isDir( QStrList & _urls );
    /// Tests wether _url is a directory or not
    /**
      If the protocol of the URL is not "file", then this function
      depends an the convention, that directories have a trailing '/'
      in their URL.

      @return 1 if the URL is a directory, 0 if not and <0 if we are unshure.
      */
    static int isDir( const char *_url );
    /**
     * Tests wether the list of URLs are all together the trash bin.
     * This function does not really make sense. It is just provided for
     * convenience.
     */
    static bool isTrash( QStrList & _urls );
    /**
     * Tests wether _url is the trash bin or not
     */
    static bool isTrash( const char *_url );
    static QString findDeviceMountPoint( const char *_device, const char *_file = "/etc/mtab" );
    /// Tells wether KIOServer supports an operation on an URL
    /**
      _mode is for example KIO_Read or any of those constants.
      */
    static bool supports( const char *_src_url, int _mode );
    /// Tells wether KIOServer supports an operation on some URLs
    /**
      An operation that is not supported by at least one of the URLs
      will cause this function to return FALSE.
      */
    static bool supports( QStrList & _urls, int _mode );
    /// Emit a notify signal for the given URL
    /**
      This function may be called by others to emit a notify signal.
      */
    static void sendNotify( const char *_url );
    /// Emit a notify signal after mount/unmount
    /**
      This function may be called by others to emit a mount notify signal.
      Every time a device is mounted unmounted, this signal is emitted. This
      causes some icons to change. A normal notify signal is emitted for
      the mount directory, too.
      */
    static void sendMountNotify();

    /**
     * Quotes a filename/URL like the shell ( bash ) would expect it.
     */
    static QString shellQuote( const char *_data );

    /**
     * Unquotes a filename/URL that is quoted like the shell ( bash ) would expect it.
     */
    static QString shellUnquote( const char *_data );

    /**
     * This function is used in @ref KfmView::slotDropEvent for example
     * to check wether some user tries to copy his home directory to his
     * Desktop, which is a sub directory of its home directory.
     *
     * @return true if '_src_url' does not contain '_dest_canonical'. 
     *
     * @param _src_url is a usual URL
     * @param _dest_canonical is a path on your local hadr drives that does not
     *                        contain any simlynks or a usual URL if the file this
     *                        URL points to is not a file on your local hard drives.
     */
    static bool testDirInclusion( const char *_src_url, const char *_dest_canonical );
    
    /**
     * Does nothing on protocols other then "file" or if there is a sub protocol
     * in the URL. The canonical URL does not contain any symlinks.
     */
    static QString canonicalURL( const char *_url );
    
public slots:
    /// If a new slave is created, this slot is called.
    /**
      This function connects 'slotDirEntry' and 'slotFlushDir' to the new slave
      and calls 'newSlave2' to do the rest.
      */
    void newSlave( KIOSlaveIPC * );
    void slotDirEntry( const char *_url, const char *_name, bool _isDir, int _size,
		   const char * creationDate, const char * _access,
		   const char * _owner, const char *_group );
    void slotFlushDir( const char *_url );
    void slotSlaveClosed( KIOSlaveIPC* );
    
signals:
    /// This notify is emitted each time a URL changed.
    /**
      Connect to this signal if you need to know when to 
      update your directory contents.
      */
    void notify( const char *_url );
    
    /// Tells that some device has been mounted/unmounted.
    /**
      Connect to this signal to get the icons associated with the devices
      updated.
      */
    void mountNotify();
    
protected:
    /// Handles the arrival of a new or freed slave
    void newSlave2( KIOSlaveIPC * );
    
    /// Internal function used to implement 'sendNotify' as static.
    void sendNotify2( const char *_url );

    /// Internal function used to implement 'sendMountNotify' as static.
    void sendMountNotify2();
    
    /// Start a new slave process
    void runNewSlave();
    
    /// List of all slaves without work
    QList<KIOSlaveIPC> freeSlaves;
    /// List of all jobs waiting for a slave
    QList<KIOJob> waitingJobs;

    /// List of all cached directories
    QDict<KIODirectory> dirList;

    /// A pointer to the running server
    /**
      Sometimes needed by static functions.
      */
    static KIOServer *pKIOServer;
};

#endif
