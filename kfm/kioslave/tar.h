#ifndef _tar_h
#define _tar_h

#include "subprotocol.h"
#include "slave.h"

class KProtocolTAR :public KSubProtocol
{
    Q_OBJECT
public:
    KProtocolTAR();
    ~KProtocolTAR();
    
    int OpenDir(KURL *url);
    KProtocolDirEntry *ReadDir();
    int CloseDir();
//     int MkDir(KURL *url); 
    
    /**
     * @param _path is used do determine whether it is a gzipped tar file.
     *              To do so we just have a look at the extension.
     */
    int AttachTAR( const char *_command );
    int Open( KURL *url, int mode );
    int Close();
    long Read( void *buffer, long len );
    
    long Size();
    
    int atEOF();
    int isGzip();

private:
    bool HandleRefill();

    /**
     * @return TRUE if the given directory is already in dirlist
     * (Path removed, leading slash removed, no trailing slash).
     */
    bool isDirStored( const char *_name );

    /**
     * @return tmp made relative to dirpath
     */
    QString makeRelative(QString tmp);

    KSlave Slave;
    /**
     * This variable is used in @ref #OpenDir only.
     */
    QString dirpath;
    /**
     * Stored after Open() is called, for Size()
     */
    QString openedUrl;
    
    FILE *dirfile;
    bool bEOF;

    // Used to pipe data between parent and tar process
    char internalbuffer[1024];
    // Used to pipe data between parent and tar process
    bool hasdata;
    // Used to pipe data between parent and tar process
    long len;

    QList<KProtocolDirEntry> dirlist;
};

#endif
