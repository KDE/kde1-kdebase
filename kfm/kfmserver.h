#ifndef kfmserver_h
#define kfmserver_h

class KFMClient;
class KFMServer;

#include <qobject.h>
#include <qpopmenu.h>

#include "kfmserver_ipc.h"
#include "kiojob.h"

class KFMServer : public KfmIpcServer
{
    Q_OBJECT
public:
    KFMServer();
    ~KFMServer() {}; // Stephan: the base class has an important destructor!

public slots:
    virtual void slotAccept( KSocket *_sock );
    void slotOpenProperties( const char* _url );
    void slotRefreshDirectory( const char* _url );
    void slotOpenURL( const char* _url );
    void slotRefreshDesktop();
    void slotConfigure();
    void slotExec( const char *_url, const char *_binding );
    void slotMoveClients( const char *_src_urls, const char *_dest_url );
    void slotCopyClients( const char *_src_urls, const char *_dest_url );
    void slotSortDesktop();
    void slotSelectRootIcons( int _x, int _y, int _w, int _h, bool _add );
};

class KFMClient : public KfmServIpc
{
    Q_OBJECT
public:
    KFMClient( KSocket *_sock, KFMServer *_server );
    
public slots:
    void slotAuth( const char *_password );
    void slotCopy( const char *_src_url, const char * _dest_url );
    void slotMove( const char *_src_urls, const char * _dest_url );
    void slotList( const char *_url );
    /// A hack. I dont want to break compatibility yet
    void finished( int _id );
    void newDirEntry( int _id, KIODirectoryEntry * _entry );
    void slotError( int _error, const char *_text );

protected:
    KIOJob *job;
    bool bAuth;
    static QString *password;
    KFMServer *server;
};

#endif

