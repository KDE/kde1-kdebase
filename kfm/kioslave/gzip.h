#ifndef _gzip_h
#define _gzip_h

#include "subprotocol.h"
#include "slave.h"

class KProtocolGZIP :public KSubProtocol
{
    Q_OBJECT
private:
    KSlave Slave;
    int bEOF;

    // Used to pipe data between parent and gzip process
    char internalbuffer[1024];
    // Used to pipe data between parent and gzip process
    bool hasdata;
    // Used to pipe data between parent and gzip process
    long len;

public:
    KProtocolGZIP();
    ~KProtocolGZIP();

    bool HandleRefill();

    int OpenGZIP();
    int Open(KURL *url, int mode);
    int Close();
    long Read(void *buffer, long len);

    /**
     * Just returns an error.
     */
    int OpenDir( KURL * );
    
    long Size();
    
    int atEOF();
};

#endif
