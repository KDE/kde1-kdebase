#ifndef _http_h
#define _http_h

#include "protocol.h"
#include "kio_errors.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>

#include <qdatetm.h>
#include <klocale.h>

class KProtocolHTTP :public KProtocol
{
    Q_OBJECT
protected:
    QString noProxyForStr;    
    int use_proxy;
    QString proxy_user;
    QString proxy_pass;
    QString post_data;
    QString Cookies;

// needed for language/charset support 
    KLocale *klocale;

    //#define HTTP_PROXY_NAME "someproxy.somewhere.org"
    //#define HTTP_PROXY_PORT 80
    
    struct sockaddr_in proxy_name;
    int connected, sock;
    long size;
    long bytesleft;
    long bytesRead;
    FILE *fsocket;

    // Jacek:
    // HTTP settings
    bool assumeHTML;
    QString languages;
    QString charsets;

    QString url;


    /**
     * Used to store the parameter given by a call to @ref #Open. We need this
     * for HTTP Redirection.
     *
     * @see #ProcessHeader
     * @see #Open
     */
    int currentMode;

    QTime currentTime;
    QTime startTime;
    
    void PrepareLanguageList(QString str);
    void PrepareCharsetList(QString str);
    int init_sockaddr(struct sockaddr_in *server_name, const char *hostname,int port);
    int ProcessHeader();
    int OpenHTTP(KURL *url, int mode,bool reload);
    
public:
    KProtocolHTTP();
    ~KProtocolHTTP();
    
    virtual int Open(KURL *url, int mode);
    virtual int ReOpen(KURL *url, int mode);
    virtual int Close();
    virtual int SetData(const char *_data);
    virtual int SetCookies(const char *_cookies);
    virtual long Read(void *buffer, long len);
    virtual long Size();
    virtual int atEOF();
};

#endif
