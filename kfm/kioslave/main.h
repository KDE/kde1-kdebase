#ifndef main_h
#define main_h

#include <qobject.h>
#include <qstring.h>

#include <kurl.h>

#include <stdlib.h>
#include <unistd.h>

#include "kioslave_ipc.h"
#include "protocol.h"

class KIOSlave : public QObject
{
    Q_OBJECT
public:
    KIOSlave( int _port );

    void terminate();
    
public slots:
    /// Copy a file
    /**
      Does not copy recursive on the local file system. Copies
      from HTTP are already supported.
      */
    void copy( const char *_src_url, const char *_dest_url, bool _overwriteExistingFiles );
    /// Delete a file
    /**
     If the _url ends with "/", then we assume that it is directory
     and use rmdir to delete the URL. Otherwise it is assumed that
     _url is a file.
     */
    void del( const char *_url );
    /// Mount a device
    void mount( bool _ro, const char *_fstype, const char* _dev, const char *_point );
    /// Unmount a directory
    void unmount( const char *_point );    
    /// Get a directory listing
    void list( const char* _url );
    /// Make a new directory
    void mkdir( const char *_url );
    /// Called if a job does not need this slave any more
    /**
      Used to clear up locked tar files and close locked ftp connections.
      */    
    void cleanUp();
    /// Tells the server about the PID.
    void getPID();
    
protected:
    void ProcessError(KProtocol *, const char *);

    /// Test an error log file
    /**
      When executing for example 'tar' or 'mount', the error output is written
      in a file. If the file is empty, en empty string will be returned. Otherwise
      the returned string will contain error information.
      The error log file is deleted any way by this function.
      */
    QString testLogFile( const char *_filename );
    
    KIOSlaveIPC* ipc;

    /// Locks a tgz file. This means it is unziped in /tmp
    /**
      Returns TRUE on success. If it returns FALSE, ipc->fatalError is already called.
      In this case just do a return.
      */
    bool lockTgz( const char *_tgz_file );
    /// Unlock a tgz file. This means zip it and write it back to its origin.
    /**
      Returns TRUE on success. If it returns FALSE, ipc->fatalError is already called.
      In this case just do a return.
      */
    bool unlockTgz();

    QString lockedTgzSource;
    QString lockedTgzTemp;
    bool lockedTgzModified;

    /// Locks a ftp connection. This means that the connection is established.
    /**
      Returns TRUE on success. If it returns FALSE, ipc->fatalError is already called.
      In this case just do a return.
      */
    bool lockFTP( const char *_host, int _port, const char* _login, const char *_passwd );
    /// Unlock the current ftp connection. This means the connection is closed.
    /**
      Returns TRUE on success. If it returns FALSE, ipc->fatalError is already called.
      In this case just do a return.
      */
    bool unlockFTP();

    QString lockedFTPHost;
    QString lockedFTPLogin;
    QString lockedFTPPasswd;
    int lockedFTPPort;

    /// The destination file we are writing to
    /**
      If the client gets a TERM signal, is closes and deletes the
      file mentioned here. It is usually the destination of a copy
      command. May be 0L.
      */
    FILE *copyDestFile;
    QString copyDestFileName;    
};

#endif
