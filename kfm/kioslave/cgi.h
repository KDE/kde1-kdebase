#ifndef _cgi_h
#define _cgi_h

#include "http.h"

#include <stdio.h>
#include <stdlib.h>

class KProtocolCGI : public KProtocolHTTP
{
    Q_OBJECT
public:
    KProtocolCGI();
    ~KProtocolCGI();
    
    virtual int Open( KURL *url, int mode );
    virtual int Close();
    virtual long Read( void *buffer, long len );

protected:
};

#endif

