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

public slots:
    void slotAuthorized( KFMClient * _client );
    virtual void slotAccept( KSocket *_sock );
    void slotOpenProperties( const char* _url );
    void slotRefreshDirectory( const char* _url );
    void slotOpenURL( const char* _url );
    void slotRefreshDesktop();
    void slotExec( const char *_url, const char *_binding );
    void slotMoveClients( const char *_src_urls, const char *_dest_url );
    void slotCopyClients( const char *_src_urls, const char *_dest_url );
    void slotAsk( int _x, int _y, const char *_src_urls, const char *_dest_url );
    void slotSortDesktop();
    void slotSelectRootIcons( int _x, int _y, int _w, int _h, bool _add );
};

class KFMClient : public KfmIpc
{
    Q_OBJECT
public:
    KFMClient( KSocket *_sock );
    
public slots:
    void slotAuth( const char *_password );
    void slotCopy( const char *_src_url, const char * _dest_url );
    void slotMove( const char *_src_urls, const char * _dest_url );
    /// A hack. I dont want to break compatibility yet
    void finished( int _id );
    
signals:
    void authorized( KFMClient *_this );
    
protected:
    KIOJob *job;
    bool bAuth;
    static QString password;
};

class KFMAsk : public QObject
{
    Q_OBJECT
public:
    KFMAsk( KFMServer* _server, int _x, int _y, const char *_src_urls, const char * _dest_url );
    ~KFMAsk();
    
public slots:
    void slotCopy();
    void slotMove();

protected:

    QPopupMenu *popupMenu;
    KFMServer* server;
    QString srcURLs;
    QString destURL;
};

#endif

