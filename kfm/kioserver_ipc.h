// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#ifndef KIOSlaveIPC_h
#define KIOSlaveIPC_h
#include <qobject.h>
#include <ksock.h>
#include <ctype.h>
#include "ipc.h"

class KIOSlaveIPC : public QObject
{
    Q_OBJECT
public:
    KIOSlaveIPC( KSocket * );
    ~KIOSlaveIPC();

    void parse( char *_data, int _len );

signals:
	void hello();
private:
	void parse_hello( char *_data, int _len );
signals:
	void progress(int _percent);
private:
	void parse_progress( char *_data, int _len );
signals:
	void dirEntry(const char* _dir, const char* _name, bool _isDir, int _size, const char* _date, const char* _access, const char* _owner, const char* _group);
private:
	void parse_dirEntry( char *_data, int _len );
signals:
	void flushDir(const char* _dir);
private:
	void parse_flushDir( char *_data, int _len );
signals:
	void done();
private:
	void parse_done( char *_data, int _len );
signals:
	void fatalError(int _error, const char* _text, int _errno);
private:
	void parse_fatalError( char *_data, int _len );
signals:
	void setPID(int _pid);
private:
	void parse_setPID( char *_data, int _len );
public slots:
	void mount(bool _ro, const char* _fstype, const char* _dev, const char* _point);
public slots:
	void unmount(const char* _point);
public slots:
	void copy(const char* _from_url, const char* _to_url, bool _overwrite);
public slots:
	void del(const char* _url);
public slots:
	void mkdir(const char* _url);
public slots:
	void list(const char* _url);
public slots:
	void getPID();
public slots:
	void cleanUp();
public:
   int pid;

public slots:
   void closeEvent( KSocket* );
   void readEvent( KSocket* );
protected:
   KSocket* data_sock;
   char headerBuffer[11];
   int cHeader;
   bool bHeader;
   char *pBody;
   int cBody;
   int bodyLen;
};

class KIOSlaveIPCServer : public QObject
{
    Q_OBJECT
public:
    KIOSlaveIPCServer();
    ~KIOSlaveIPCServer();

    int getPort();

signals:
   void newClient( KIOSlaveIPC * );

public slots:
   virtual void slotAccept( KSocket* );

protected:
   KServerSocket* serv_sock;
};

#endif
