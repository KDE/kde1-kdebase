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
    
    /**
     * @param _path is used do determine wether it is a gzipped tar file.
     *              To do so we just have a look at the extension.
     */
    int AttachTAR( const char *_command );
    int Open( KURL *url, int mode );
    int Close();
    long Read( void *buffer, long len );
    
    long Size();
    
    int atEOF();

private:
    bool HandleRefill();

    KSlave Slave;
    char *dirpathmem, *dirpath;
    FILE *dirfile;
    bool bEOF;

    // Used to pipe data between parent and tar process
    char internalbuffer[1024];
    // Used to pipe data between parent and tar process
    bool hasdata;
    // Used to pipe data between parent and tar process
    long len;

};

#endif
