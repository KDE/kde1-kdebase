#ifndef _icon_h
#define _icon_h

#include "protocol.h"
#include "slave.h"
#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>

class KProtocolICON : public KProtocol
{
    Q_OBJECT
private:
    FILE *f;
    QString xvfile;
    bool bDeleteFile;
    
public:
    KProtocolICON();
    ~KProtocolICON();

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
