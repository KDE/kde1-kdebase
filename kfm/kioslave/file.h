#ifndef _file_h
#define _file_h

#include "protocol.h"
#include "kio_errors.h"

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

class KProtocolFILE :public KProtocol
{
    Q_OBJECT
private:
    long size;
    FILE *file;
    DIR *dp;
    QString path;
    bool allowHTML;
    
public:
    KProtocolFILE();
    ~KProtocolFILE();
    
    int Open(KURL *url, int mode);
    int Close();
    long Read(void *buffer, long len);
    long Write(void *buffer, long len);
    
    long Size();
    
    int atEOF();
    
    /** functions for directories **/
    int OpenDir(KURL *url);
    virtual bool isHTML() { return !( file == 0L ); }
    virtual void AllowHTML( bool _allow ) { allowHTML = _allow; }
    virtual void EmitData( KIOSlaveIPC *_ipc );
    KProtocolDirEntry *ReadDir();
    int CloseDir();
    int MkDir(KURL *url);

    /**
     * @return -1 if we dont know about permissions
     */
    virtual int GetPermissions( KURL &_u );
    /**
     * If '_perm' is -1, then we wont set any permissions here.
     */
    virtual void SetPermissions( KURL &_u, int _perm );      
};

#endif
